//=============================================================================
// ITEM処理[item.cpp]
//=============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "item.h"
#include "enemy.h"
#include "bg.h"
#include "player.h"
#include <stdlib.h>
#include <math.h>
#include"score.h"
#include"pause.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH                (50)			// 背景サイズ
#define TEXTURE_HEIGHT               (50)			// 背景サイズ
#define TEXTURE_MAX                  (2)            // テクスチャの数

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/item_HP.png",
	"data/TEXTURE/item_XP.png",

};

static BOOL	    g_Load = FALSE;		// 初期化を行ったかのフラグ
static ITEAM	g_Item[MAX_ITEM];

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitItem(void)
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


	// ITEM初期化
	for (int i = 0; i < MAX_ITEM; i++)
	{
		g_Item[i].w     = TEXTURE_WIDTH;
		g_Item[i].h     = TEXTURE_HEIGHT;
		g_Item[i].pos   = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Item[i].texNo = 0;                       //0=HEART  1=XP
		g_Item[i].use   = FALSE;
		g_Item[i].alpha = 0.0f;                    //透明度
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitItem(void)
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

void UpdateItem(void)
{
	PLAYER* player = GetPlayer();

	// ゲーム時間を得る
	int Cultime = GetTime();

	for (int i = 0; i < MAX_ITEM; i++)
	{
		if (g_Item[i].use == TRUE)
		{
			//２０秒内拾っていないアイテムを消す
			if (Cultime - g_Item[i].creationTime == 20)
			{
				g_Item[i].use = FALSE;
			}
			
			// エネミーとアイテムの距離を計算する
			float distance = sqrt(pow((player->pos.x - g_Item[i].pos.x), 2) + pow((player->pos.y - g_Item[i].pos.y), 2));

			// アイテムを拾う範囲最大値
			if (distance <= 150.0f)
			{
				// アイテムをプレイヤーに移動する
				float speed = 10.0f;
				g_Item[i].pos.x += (player->pos.x - g_Item[i].pos.x) / distance * speed;
				g_Item[i].pos.y += (player->pos.y - g_Item[i].pos.y) / distance * speed;
			}

			// アイテムが消える距離
			if (distance < 50.0f)
			{
				//　拾った！
				g_Item[i].use = FALSE;

				// ハートでHPを回復する
				if (g_Item[i].texNo == 0)
				{
					player->blood = 7;
				}
				else
				{
					// XPアップ
					player->xp += 1;

					// LEVELUPの判定
					if (player->xp == player->boom)
					{
						player->lv += 1;
						player->boom += 5;
						player->xp = 0;

						// LV２ごとにスキルアップできる
						if (player->lv % 2 == 0 && checkXpMAX())
						{
							// スキルアップ
							Switch(LEVELUP);
						}
					}
				}
				continue;
			}

			//アイテムをプレイヤーに移動して透明度処理
			if (g_Item[i].alpha < 1.0f)
			{
				g_Item[i].alpha += 0.01f;
				if (g_Item[i].alpha > 1.0f)
					g_Item[i].alpha = 1.0f;
			}
		}
	}
}


//=============================================================================
// 描画処理
//=============================================================================
void DrawItem(void)
{
	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1,&g_VertexBuffer,&stride,&offset);

	// マトリクス設定
	SetWorldViewProjection2D();

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	BG* bg = GetBG();

	// アイテムを描画
	for (int i = 0; i < MAX_ITEM; i++)
	{
		if (g_Item[i].use == TRUE)
		{
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Item[i].texNo]);

			//アイテムの位置やテクスチャー座標を反映
			float px = g_Item[i].pos.x - bg->pos.x;	// アイテムの表示位置X
			float py = g_Item[i].pos.y - bg->pos.y;	// アイテムの表示位置Y
			float pw = g_Item[i].w;		            // アイテムの表示幅
			float ph = g_Item[i].h;		            // アイテムの表示高さ


			// アニメーション用
			float tw = 1.0f;	                    // テクスチャの幅
			float th = 1.0f;	                    // テクスチャの高さ
			float tx = 0.0f;                        // テクスチャの左上X座標
			float ty = 0.0f;                        // テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteLTColor(g_VertexBuffer,
				px, py, pw, ph,
				tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, g_Item[i].alpha));  //透明度設定する

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);
		}


	}


}


// BG構造体の先頭アドレスを取得
ITEAM* GetItem(void)
{
	return &g_Item[0];
}

//ITEM生成する
int generateValue(void)
{
	//ランダムに生成する
	int randomValue = rand() % 100;
	if (randomValue <= 3)
	{
		//HP
		return 0;
	}
	else 
	{
		//XP
		return 1;
	}
}

//エネミー死んだらアイテムを生成する
void SetItem(float x, float y)
{
	for (int i = 0; i < MAX_ITEM; i++)
	{
		if (g_Item[i].use == FALSE)	
		{
			g_Item[i].use = TRUE;
			g_Item[i].pos.x = x -20;
			g_Item[i].pos.y = y -20;
			g_Item[i].texNo = generateValue();
			//アイテム生成する時間を得る
			g_Item[i].creationTime = GetTime();
			break;
		}
	}
}

