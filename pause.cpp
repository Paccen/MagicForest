//=============================================================================
// [pause.cpp]
//=============================================================================

#include "pause.h"
#include "main.h"
#include "input.h"
#include "bullet.h"
#include "sound.h"
#include <windows.h>

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define XP_MAX                              (5)              //ブレット最大値
#define XP_KIND                             (4)              //ブレット種類

#define TEXTURE_WIDTH                       (480)             // キャラサイズ		
#define TEXTURE_HEIGHT                      (640)            // キャラサイズ
#define TEXTURE_MAX                         (5)              // テクスチャの数

#define TEXTURE_WIDTH_POINT                 (80)             // キャラサイズ(POINT)
#define TEXTURE_HEIGHT_POINT                (80)             // キャラサイズ(POINT)

#define TEXTURE_WIDTH_XP                    (80)             // キャラサイズ(XP)
#define TEXTURE_HEIGHT_XP                   (80)             // キャラサイズ(XP)

#define SOME_UPPER_VALUE_MENU  (SCREEN_HEIGHT / 2 - 40.0f)   //フレームの上エッジMENU
#define SOME_LOWER_VALUE_MENU  (SCREEN_HEIGHT / 2 + 80.0f)   //フレームの下エッジMENU

#define SOME_UPPER_VALUE_LEVELUP (SCREEN_HEIGHT / 2 - 60.0f) //フレームの上エッジMENU
#define SOME_LOWER_VALUE_LEVELUP (SCREEN_HEIGHT / 2 + 195.0f)//フレームの上エッジMENU


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;                         // 頂点情報
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char* g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/menu.png",
	"data/TEXTURE/levelup.png",
	"data/TEXTURE/point.png",
	"data/TEXTURE/xp2.png",

};


static BOOL	    g_Load = FALSE;		
static PAUSE	g_Menu;
static PAUSE	g_Up;
static PAUSE	g_PonitMenu;
static PAUSE	g_PonitUp;
static PAUSE	g_Xp[XP_KIND][XP_MAX];

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitPause(void)
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

	//MENUフレームの初期化
	g_Menu.w = TEXTURE_WIDTH;
	g_Menu.h = TEXTURE_HEIGHT;
	g_Menu.pos = XMFLOAT3(SCREEN_WIDTH/2 - TEXTURE_WIDTH/2, SCREEN_HEIGHT/2- TEXTURE_HEIGHT/2, 0.0f);
	g_Menu.texNo = 0;

	//LEVELUPフレームの初期化
	g_PonitUp.w = TEXTURE_WIDTH_POINT;
	g_PonitUp.h = TEXTURE_HEIGHT_POINT;
	g_PonitUp.pos = XMFLOAT3(SCREEN_WIDTH / 2 - TEXTURE_WIDTH / 2 + 20.0f, SCREEN_HEIGHT / 2 - 60.0f, 0.0f);
	g_PonitUp.texNo = 3;
	g_PonitUp.speed = 85;

	//LEVELUPフレームポイントの初期化
	g_Up.w = TEXTURE_WIDTH;
	g_Up.h = TEXTURE_HEIGHT;
	g_Up.pos = XMFLOAT3(SCREEN_WIDTH / 2 - TEXTURE_WIDTH / 2, SCREEN_HEIGHT / 2 - TEXTURE_HEIGHT / 2, 0.0f);
	g_Up.texNo = 1;

	//MENUフレームポイントの初期化
	g_PonitMenu.w = TEXTURE_WIDTH_POINT;
	g_PonitMenu.h = TEXTURE_HEIGHT_POINT;
	g_PonitMenu.pos = XMFLOAT3(SCREEN_WIDTH / 2 - TEXTURE_WIDTH / 2 + 10.0f, SCREEN_HEIGHT / 2 - 40.0f, 0.0f);
	g_PonitMenu.texNo = 2;
	g_PonitMenu.speed = 120;

	//XPテクスチャの初期化
	for (int i = 0; i < XP_KIND; i++)
	{
		for (int j = 0; j < XP_MAX; j++)
		{
			g_Xp[i][j].w = TEXTURE_WIDTH_XP;
			g_Xp[i][j].h = TEXTURE_HEIGHT_XP;
			g_Xp[i][j].texNo = 3;
			g_Xp[i][j].use = FALSE;
			g_Xp[i][j].pos = XMFLOAT3(SCREEN_WIDTH / 2 - TEXTURE_WIDTH / 2 + 180.0f + j*50, 
				SCREEN_HEIGHT / 2 - 60.0f+i*90, 
				0.0f);
		}
	}

	//初め持ってるスキル
	g_Xp[1][0].use = TRUE;

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitPause(void)
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
void UpdatePause(int MENU_LEVELUP_START)
{

	// MENU POINTの処理
	if (MENU_LEVELUP_START == MENU)
	{
		// エッジ処理
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
				//ゲームの続き
				Switch(START);
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

	// LEVEL UP
	if (MENU_LEVELUP_START == LEVELUP)
	{
		float upperBoundary = SOME_UPPER_VALUE_LEVELUP; // 上エッジ
		float lowerBoundary = SOME_LOWER_VALUE_LEVELUP; // 下エッジ

		//ポイントの移動
		if (GetKeyboardTrigger(DIK_DOWN))
		{
			g_PonitUp.pos.y += g_PonitUp.speed;
		}
		else if (GetKeyboardTrigger(DIK_UP))
		{
			g_PonitUp.pos.y -= g_PonitUp.speed;
		}

		//  エッジ外チェック
		if (g_PonitUp.pos.y > SOME_LOWER_VALUE_LEVELUP)
		{
			g_PonitUp.pos.y = SOME_UPPER_VALUE_LEVELUP;
		}
		else if (g_PonitUp.pos.y < SOME_UPPER_VALUE_LEVELUP)
		{
			g_PonitUp.pos.y = SOME_LOWER_VALUE_LEVELUP;
		}

		// XP++
		for (int i = 0; i < XP_KIND; i++)
		{
			if (g_PonitUp.pos.y == SOME_UPPER_VALUE_LEVELUP + g_PonitUp.speed * i) 
			{
				if (GetKeyboardTrigger(DIK_RETURN))
				{
					//FIREスキルアップ
					if (i == 0)
					{
						if (checkMax(i))
						{
							SetFire();
						}
						else
						{
							continue;
						}
						
					}
					//BOOKスキルアップ
					if (i == 1)
					{
						if (checkMax(i))
						{
							SetBook();
						}
						else
						{
							continue;
						}
					}
					//AXEスキルアップ
					if (i == 2)
					{
						if (checkMax(i))
						{
							SetAxe();
						}
						else
						{
							continue;
						}
					}
					//LIGHTINGスキルアップ
					if (i == 3)
					{
						if (checkMax(i))
						{
							SetLighting();
						}
						else
						{
							continue;
						}
					}

					//ディスプレイ
					SetXP(i);

					//ゲームの続き
					Switch(START);
				}
			}
		}
	}
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawPause(int MENU_LEVELUP_START_GUIDE)
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

	
	//MENUモデル
	if (MENU_LEVELUP_START_GUIDE == MENU)
	{
		// MENUの背景を描画
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteLTColor(g_VertexBuffer,
			g_Menu.pos.x, g_Menu.pos.y, g_Menu.w, g_Menu.h,
			0.0f, 0.0f, 1.0f, 1.0f,
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);



		//MENUのPOINTを描画
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteLTColor(g_VertexBuffer,
			g_PonitMenu.pos.x, g_PonitMenu.pos.y, g_PonitMenu.w, g_PonitMenu.h,
			0.0f, 0.0f, 1.0f, 1.0f,
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);

	}


	 //  LEVELUPモデル
	if (MENU_LEVELUP_START_GUIDE == LEVELUP)
	{
		// MENUの背景を描画
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteLTColor(g_VertexBuffer,
			g_Up.pos.x, g_Up.pos.y, g_Up.w, g_Up.h,
			0.0f, 0.0f, 1.0f, 1.0f,
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);

		//MENUのPOINTを描画
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSpriteLTColor(g_VertexBuffer,
			g_PonitUp.pos.x, g_PonitUp.pos.y, g_PonitUp.w, g_PonitUp.h,
			0.0f, 0.0f, 1.0f, 1.0f,
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);

		// XPを描画
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[3]);

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				if (g_Xp[i][j].use == TRUE)
				{
					// １枚のポリゴンの頂点とテクスチャ座標を設定
					SetSpriteLTColor(g_VertexBuffer,
						g_Xp[i][j].pos.x, g_Xp[i][j].pos.y, g_Xp[i][j].w, g_Xp[i][j].h,
						0.0f, 0.0f, 1.0f, 1.0f,
						XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

					// ポリゴン描画
					GetDeviceContext()->Draw(4, 0);

				}
			}
		}
	}
}


//スキルアップ用
void SetXP(int x)
{
	for (int i = 0; i < XP_MAX; i++)
	{
		if (g_Xp[x][i].use == FALSE)
		{
			g_Xp[x][i].use = TRUE;
			break;
		}
	}
}

//スキル最大値なら
BOOL checkMax(int x)
{
	if (g_Xp[x][4].use == TRUE)
	{
		return FALSE;
	}

	else
	{
		return TRUE;
	}
}

//スキル全然最大値
BOOL checkXpMAX(void)
{

	for (int i = 0; i < XP_KIND; i++)
	{
		if (g_Xp[i][4].use == FALSE)
		{
			return TRUE;
		}
	}

	return FALSE;


}


