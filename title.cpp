//=============================================================================
// TITLE処理 [title.cpp]
//=============================================================================
#include "title.h"
#include "main.h"
#include "title.h"
#include "input.h"
#include "fade.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				SCREEN_WIDTH		     // 背景サイズ
#define TEXTURE_HEIGHT				SCREEN_HEIGHT		     // 背景サイズ
#define TEXTURE_MAX					    (3)                  // テクスチャの数

#define SOME_UPPER_VALUE_MENU       SCREEN_HEIGHT / 2+ 30.0f //上エッジ
#define SOME_LOWER_VALUE_MENU       SCREEN_HEIGHT / 2+ 210.0f//下エッジ
 
#define TEXTURE_WIDTH_POINT             (80)			     // ポイントサイズ
#define TEXTURE_HEIGHT_POINT            (80)			     // ポイントサイズ

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;		                    // 頂点情報
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報




static char* g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/title.png",
	"data/TEXTURE/point.png",
	"data/TEXTURE/guide.jpg"
};

static TITLE	g_PonitMenu;
static BOOL	    g_Load = FALSE;		                                 // 初期化を行ったかのフラグ
static TITLE	g_Title;
static int      g_mode = M_TITLE;
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitTitle(void)
{

	ID3D11Device* pDevice = GetDevice();

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


	// TITLE MENUの初期化
	g_Title.w = TEXTURE_WIDTH;
	g_Title.h = TEXTURE_HEIGHT;
	g_Title.pos = XMFLOAT3(0.0f, 0.0f, 0.0f);

	// TITLE POINTの初期化
	g_PonitMenu.w = TEXTURE_WIDTH_POINT;
	g_PonitMenu.h = TEXTURE_HEIGHT_POINT;
	g_PonitMenu.pos = XMFLOAT3(SCREEN_WIDTH / 2 -800.0f, SCREEN_HEIGHT / 2+ 30.0f, 0.0f);
	g_PonitMenu.speed = 180;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitTitle(void)
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
void UpdateTitle(void)
{
	if (g_mode == M_TITLE)
	{
		// エッジ
		float upperBoundary = SOME_UPPER_VALUE_MENU; // 上エッジ
		float lowerBoundary = SOME_LOWER_VALUE_MENU; // 下エッジ

		//ポイントの移動
		if (GetKeyboardTrigger(DIK_DOWN))
		{
			g_PonitMenu.pos.y += g_PonitMenu.speed;
		}
		else if (GetKeyboardTrigger(DIK_UP))
		{
			g_PonitMenu.pos.y -= g_PonitMenu.speed;
		}

		// エッジ外チェック
		if (g_PonitMenu.pos.y > SOME_LOWER_VALUE_MENU)
		{
			g_PonitMenu.pos.y = SOME_UPPER_VALUE_MENU;
		}
		else if (g_PonitMenu.pos.y < SOME_UPPER_VALUE_MENU)
		{
			g_PonitMenu.pos.y = SOME_LOWER_VALUE_MENU;
		}

		//次に確認する
		if (g_PonitMenu.pos.y == SOME_UPPER_VALUE_MENU)
		{
			if (GetKeyboardTrigger(DIK_RETURN))
			{
				//GUIDE MODE IN
				g_mode = M_GUIDE;
			}
		}
		else if (g_PonitMenu.pos.y == SOME_LOWER_VALUE_MENU)
		{
			if (GetKeyboardTrigger(DIK_RETURN))
			{
				//ゲームの終了
				PostQuitMessage(0);
			}
		}
	}


	if (g_mode == M_GUIDE)
	{
		//ゲームの続き
		if (GetKeyboardTrigger(DIK_RETURN))
		{
			SetFade(FADE_OUT, MODE_GAME);
			g_mode = M_TITLE;
		}
	}




	
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawTitle(void)
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

	if (g_mode == M_TITLE)
	{
		// TITLEの背景を描画
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteLTColor(g_VertexBuffer,
			0 - g_Title.pos.x, 0 - g_Title.pos.y, g_Title.w, g_Title.h,
			1.0f, 1.0f, 1.0f, 1.0f,
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);

		//MENUのPOINTを描画
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteLTColor(g_VertexBuffer,
			g_PonitMenu.pos.x, g_PonitMenu.pos.y, g_PonitMenu.w, g_PonitMenu.h,
			0.0f, 0.0f, 1.0f, 1.0f,
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}


	if (g_mode == M_GUIDE)
	{
		// GUIDEの背景を描画
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);

		float px = SCREEN_WIDTH  / 2;	    // GUIDEの表示位置X
		float py = SCREEN_HEIGHT / 2;	    // GUIDEの表示位置Y

		float pw = SCREEN_WIDTH;				// GUIDEの表示幅
		float ph = SCREEN_HEIGHT;				// GUIDEの表示高さ
		float tw = 1.0f;    		    // テクスチャの幅
		float th = 1.0f;		        // テクスチャの高さ
		float tx = 0.0f;	        	// テクスチャの左上X座標
		float ty = 0.0f;			    // テクスチャの左上Y座標

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}
	

}










