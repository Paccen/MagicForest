//=============================================================================
// リザルト画面処理 [result.cpp]
//=============================================================================
#include "result.h"
#include "input.h"
#include "score.h"
#include "fade.h"
#include "pause.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH			 	(SCREEN_WIDTH)  	// 背景サイズ
#define TEXTURE_HEIGHT			 	(SCREEN_HEIGHT)	    // 背景サイズ
#define TEXTURE_MAX				     	 (2)				// テクスチャの数

#define TEXTURE_WIDTH_POINT              (80)
#define TEXTURE_HEIGHT_POINT             (80)

#define SOME_UPPER_VALUE_MENU         (SCREEN_HEIGHT / 2  + 100.0f)
#define SOME_LOWER_VALUE_MENU         (SCREEN_HEIGHT / 2  + 270.0f)
//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		        // 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/gameover.png",
	"data/TEXTURE/point.png",

};



static BOOL						g_Load = FALSE;
static PAUSE	                g_Ponit;
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitResult(void)
{
	ID3D11Device *pDevice = GetDevice();

	//テクスチャ生成
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TexturName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}


	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	//LEVELUPフレームの初期化
	g_Ponit.w = TEXTURE_WIDTH_POINT;
	g_Ponit.h = TEXTURE_HEIGHT_POINT;
	g_Ponit.pos = XMFLOAT3(SCREEN_WIDTH / 2 - 400.0f, SCREEN_HEIGHT / 2 + 100.0f, 0.0f);
	g_Ponit.speed = 170;


	// BGM再生


	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitResult(void)
{
	if (g_Load == FALSE) return;

	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		if (g_Texture[i])
		{
			g_Texture[i]->Release();
			g_Texture[i] = NULL;
		}
	}

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateResult(void)
{
	// POINTの処理
	// エッジ処理
	float upperBoundary = SOME_UPPER_VALUE_MENU; // 上エッジ
	float lowerBoundary = SOME_LOWER_VALUE_MENU; // 下エッジ

	//ポイントの移動
	if (GetKeyboardTrigger(DIK_DOWN))
	{
		g_Ponit.pos.y += g_Ponit.speed;
	}
	else if (GetKeyboardTrigger(DIK_UP))
	{
		g_Ponit.pos.y -= g_Ponit.speed;
	}

	// エッジ外チェック
	if (g_Ponit.pos.y > SOME_LOWER_VALUE_MENU)
	{
		g_Ponit.pos.y = SOME_UPPER_VALUE_MENU;
	}
	else if (g_Ponit.pos.y < SOME_UPPER_VALUE_MENU)
	{
		g_Ponit.pos.y = SOME_LOWER_VALUE_MENU;
	}

	//次に確認する
	if (g_Ponit.pos.y == SOME_UPPER_VALUE_MENU)
	{
		if (GetKeyboardTrigger(DIK_RETURN))
		{
			//ゲームの続き
			SetFade(FADE_OUT, MODE_TITLE);
		}
	}

	else if (g_Ponit.pos.y == SOME_LOWER_VALUE_MENU)
	{
		if (GetKeyboardTrigger(DIK_RETURN))
		{
			//ゲームの終了
			PostQuitMessage(0);
		}
	}



}

//=============================================================================
// 描画処理
//=============================================================================
void DrawResult(void)
{
	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// マトリクス設定
	SetWorldViewProjection2D();

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	// 背景を描画
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

	float px = 0.0f;	                // 表示位置X
	float py = 0.0f;	                // 表示位置Y

	float pw = SCREEN_WIDTH;		    // 表示幅
	float ph = SCREEN_HEIGHT;		    // 表示高さ
	float tw = 1.0f;    		        // テクスチャの幅
	float th = 1.0f;		            // テクスチャの高さ
	float tx = 0.0f;	        	    // テクスチャの左上X座標
	float ty = 0.0f;			        // テクスチャの左上Y座標

	// １枚のポリゴンの頂点とテクスチャ座標を設定
	SetSpriteLTColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	// ポリゴン描画
	GetDeviceContext()->Draw(4, 0);




	//POINTを描画
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

	// １枚のポリゴンの頂点とテクスチャ座標を設定
	SetSpriteLTColor(g_VertexBuffer,
		g_Ponit.pos.x, g_Ponit.pos.y, g_Ponit.w, g_Ponit.h,
		0.0f, 0.0f, 1.0f, 1.0f,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	// ポリゴン描画
	GetDeviceContext()->Draw(4, 0);



}




