//=============================================================================
// スコア処理 [score.cpp]
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "score.h"
#include "sprite.h"
#
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(16*2)	// キャラサイズ
#define TEXTURE_HEIGHT				(32*2)	// キャラサイズ
#define TEXTURE_MAX                  (2)	// テクスチャの数
#define TEXTURE_PATTERN_DIVIDE       (10)   // アニメパターンのテクスチャ内分割数（X)

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/number.png",
	"data/TEXTURE/dot.png"
};

static bool						g_Use;						// true:使っている  false:未使用
static float					g_w, g_h;					// 幅と高さ
static XMFLOAT3					g_Pos;						// ポリゴンの座標
static int						g_TexNo;					// テクスチャ番号
static int						g_Score;					// スコア

static int                      elapsedTime;
DWORD                           g_StartTime;


#define COUNT_DOWN_MAX     15                
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitScore(void)
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


	// タイマーの初期化
	g_Use   =  true;
	g_w     =  TEXTURE_WIDTH;
	g_h     =  TEXTURE_HEIGHT;
	g_Pos   =  { SCREEN_WIDTH / 2,  TEXTURE_HEIGHT, 0.0f ,};
	g_TexNo =  0;
	g_StartTime = timeGetTime(); // 记录程序开始运行的时间戳

	// 数字の初期化
	g_Score = 0;

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitScore(void)
{
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

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateScore(void)
{
	//ゲーム時間を得る
	DWORD currentTime = timeGetTime();

	// 秒になる
    elapsedTime = (currentTime - g_StartTime) / 1000.0f; 


}

//=============================================================================
// 描画処理
//=============================================================================
void DrawScore(void)
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

	// 秒を描画する
	{
	    // テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);
		int number = elapsedTime % 60;
		for (int i = 0; i < SCORE_DIGIT; i++)
		{
			// 今回表示する桁の数字
			float x = (float)(number % 10);

			// 数字の位置やテクスチャー座標を反映
			float px = (g_Pos.x - g_w * i) + (3 * g_w);	// 数字の表示位置X
			float py = g_Pos.y + g_Pos.y / 2;			// 数字の表示位置Y
			float pw = g_w;				                // 数字の表示幅
			float ph = g_h;				                // 数字の表示高さ

			float tw = 1.0f / TEXTURE_PATTERN_DIVIDE;	// テクスチャの幅
			float th = 1.0f;		                    // テクスチャの高さ
			float tx = x * tw;			                // テクスチャの左上X座標
			float ty = 0.0f;			                // テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);

			// 次の桁へ
			number /= 10;
		}
	}

	//Dotテクスチャを描画する
	{
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		// Dotの位置やテクスチャー座標を反映
		float dot_px = (g_Pos.x - g_w * 2) + (2.5 * g_w);	// Dotの表示位置X
		float dot_py = g_Pos.y + g_Pos.y / 2;			    // Dotの表示位置Y
		float dot_pw = g_w;				                    // Dotの表示幅
		float dot_ph = g_h;				                    // Dotの表示高さ

		float dot_tw = 1.0f / 3.0f;		                    // テクスチャの幅
		float dot_th = 1.0f;		                        // テクスチャの高さ
		float dot_tx = 2.0f / 3.0f;			                // テクスチャの左上X座標
		float dot_ty = 0.0f;			                    // テクスチャの左上Y座標

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteColor(g_VertexBuffer, dot_px, dot_py, dot_pw, dot_ph, dot_tx, dot_ty, dot_tw, dot_th,
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);

	}

	// 分を描画する
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);
		int number = (elapsedTime/60);
		for (int i = 0; i < SCORE_DIGIT; i++)
		{
			// 今回表示する桁の数字
			float x = (float)(number % 10);

			// 分の位置やテクスチャー座標を反映
			float px = (g_Pos.x - g_w * i) - (1.0 * g_w);	//分の表示位置X
			float py = g_Pos.y + g_Pos.y/2;			        //分の表示位置Y
			float pw = g_w;				                    //分の表示幅
			float ph = g_h;				                    //分の表示高さ

			float tw = 1.0f / TEXTURE_PATTERN_DIVIDE;		// テクスチャの幅
			float th = 1.0f;		                        // テクスチャの高さ
			float tx = x * tw;			                    // テクスチャの左上X座標
			float ty = 0.0f;			                    // テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);

			// 次の桁へ
			number /= 10;
		}
	}
}





//今の時間を得る関数
int GetTime(void)
{
	return elapsedTime;
}