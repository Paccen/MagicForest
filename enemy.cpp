//=============================================================================
// エネミー処理 [enemy.cpp]
//=============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "enemy.h"
#include "bg.h"
#include "player.h"
#include "fade.h"
#include "collision.h"
#include <cmath> 
#include <DirectXMath.h> 
#include "score.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH               (200/3)	// キャラサイズ
#define TEXTURE_HEIGHT              (200/3)	// キャラサイズ
#define TEXTURE_MAX                 (4)		// テクスチャの数

#define TEXTURE_PATTERN_DIVIDE_X    (3)		// アニメパターンのテクスチャ内分割数（X)
#define TEXTURE_PATTERN_DIVIDE_Y    (4)		// アニメパターンのテクスチャ内分割数（Y)
#define ANIM_PATTERN_NUM     (TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)  // アニメーションパターン数
#define ANIM_WAIT                   (4)		// アニメーションの切り替わるWait値

#define ELLIPSE_RADIUS_X     SCREEN_WIDTH   // 楕円形エネミーを生成するX半径
#define ELLIPSE_RADIUS_Y    SCREEN_HEIGHT   // 楕円形エネミーを生成するXY半径
#define GAME_TIME                   300     // ゲーム時間最大値


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;				            // 頂点情報
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char* g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/zombie.png",     
	"data/TEXTURE/ghost.png",      
	"data/TEXTURE/pumpkin.png",    
	"data/TEXTURE/sprite.png"
};

static BOOL		g_Load = FALSE;                                     // 初期化を行ったかのフラグ
static ENEMY	g_Enemy[TEXTURE_MAX][ENEMY_MAX];                    // エネミー構造体

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitEnemy(void)
{

	ID3D11Device* pDevice = GetDevice();
	PLAYER* player = GetPlayer();

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

	// エネミー構造体の初期化
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		for (int j = 0; j < ENEMY_MAX; j++)
		{
			g_Enemy[i][j].pos.x = 0;
			g_Enemy[i][j].pos.y = 0;
			g_Enemy[i][j].use = FALSE;
			g_Enemy[i][j].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
			g_Enemy[i][j].texNo = i;
			g_Enemy[i][j].countAnim = 0;
			g_Enemy[i][j].patternAnim = 0;
			g_Enemy[i][j].dir = CHAR_DIR_DOWN;					    // 下向きにしとくか
			g_Enemy[i][j].moving = FALSE;							// 移動中フラグ
			g_Enemy[i][j].patternAnim = g_Enemy[i][j].dir * TEXTURE_PATTERN_DIVIDE_X;

			//ZOMBIEの特別制定
			if (i == ZOOBIE)                                        
			{
				g_Enemy[i][j].blood = 2;                            //ブラッド
				g_Enemy[i][j].move = XMFLOAT3(1.0f, 0.0f, 0.0f);    //移動速度
				g_Enemy[i][j].attack = 1;                           //攻撃力
				g_Enemy[i][j].w = TEXTURE_WIDTH;                    //テクスチャのサイズ
				g_Enemy[i][j].h = TEXTURE_HEIGHT;                   //テクスチャのサイズ

			}

			//GHOSTの特別制定
			if (i == GHOST)                                         
			{
				g_Enemy[i][j].blood = 2;						    //ブラッド
				g_Enemy[i][j].move = XMFLOAT3(2.0f, 0.0f, 0.0f);    //移動速度
				g_Enemy[i][j].attack = 1;                           //攻撃力
				g_Enemy[i][j].w = TEXTURE_WIDTH + 20;				//テクスチャのサイズ
				g_Enemy[i][j].h = TEXTURE_HEIGHT + 20;				//テクスチャのサイズ

			}

			//PUMPKINの特別制定
			if (i == PUMPKIN)                                       
			{
				g_Enemy[i][j].blood = 3;                            //ブラッド
				g_Enemy[i][j].move = XMFLOAT3(1.0f, 0.0f, 0.0f);	//移動速度
				g_Enemy[i][j].attack = 1;							//攻撃力
				g_Enemy[i][j].w = TEXTURE_WIDTH + 30;				//テクスチャのサイズ
				g_Enemy[i][j].h = TEXTURE_HEIGHT + 30;				//テクスチャのサイズ

			}

			//SPRITEの特別制定
			if (i == SPRITE)                                        
			{
				g_Enemy[i][j].blood = 2;                            //ブラッド
				g_Enemy[i][j].move = XMFLOAT3(2.0f, 0.0f, 0.0f);	//移動速度	
				g_Enemy[i][j].attack = 2;							//攻撃力
				g_Enemy[i][j].w = TEXTURE_WIDTH + 10;				//テクスチャのサイズ
				g_Enemy[i][j].h = TEXTURE_HEIGHT + 10;				//テクスチャのサイズ

			}
			
		}
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitEnemy(void)
{
	if (g_Load == FALSE)
	{
		return;
	}

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
void UpdateEnemy(void)
{
	PLAYER* player = GetPlayer();
	BG* bg = GetBG();

	//ゲーム時間を得る
	int relNow = GetTime();

	//エネミー生成の頻度は５ｓ／１回
	if ((relNow % 5 == 0)  &&  (relNow  != 0))
	{

		//ゲーム時間とエネミー数量の関係
		int spawnCountForCurrentTime[] = 
		{
			static_cast<int>(ENEMY_MAX * 0.2f),
			relNow > GAME_TIME*0.25f ? static_cast<int>(ENEMY_MAX * 0.3f) : 0,
			relNow > GAME_TIME*0.5f  ? static_cast<int>(ENEMY_MAX * 0.6f) : 0,
			relNow > GAME_TIME*0.75f ? ENEMY_MAX : 0
		};

		for (int i = 0; i < 4; i++) 
		{
			if (spawnCountForCurrentTime[i] > 0)
			{
				//プレイヤーを中心に楕円形範囲内ランダムにエネミーを生成する
				SpawnEnemiesAtRandomPosition(player, i, spawnCountForCurrentTime[i]);
			}
		}
	}

	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		for (int j = 0; j < ENEMY_MAX; j++)
		{
			if (g_Enemy[i][j].use == TRUE)
			{
				//エネミーとプレイヤーの角度を計算
				XMVECTOR epos = XMLoadFloat3(&g_Enemy[i][j].pos);
				XMVECTOR vec = XMLoadFloat3(&player[0].pos) - epos;
				float angle = atan2f(vec.m128_f32[1], vec.m128_f32[0]);
				float speed = g_Enemy[i][j].move.x;

				//エネミーの移動処理
				g_Enemy[i][j].pos.x += cosf(angle) * speed;
				g_Enemy[i][j].pos.y += sinf(angle) * speed;

				// エネミーの向き処理
				if (fabs(sinf(angle)) > fabs(cosf(angle)))
				{
					if (sinf(angle) > 0)
						g_Enemy[i][j].dir = CHAR_DIR_DOWN;
					else
						g_Enemy[i][j].dir = CHAR_DIR_UP;
				}
				else
				{
					if (cosf(angle) > 0)
						g_Enemy[i][j].dir = CHAR_DIR_RIGHT;
					else
						g_Enemy[i][j].dir = CHAR_DIR_LEFT;
				}

				g_Enemy[i][j].moving = TRUE;


				// エネミーのアニメーション処理
				g_Enemy[i][j].countAnim += 1.0f;
				if (g_Enemy[i][j].countAnim > ANIM_WAIT)
				{
					g_Enemy[i][j].countAnim = 0.0f;
					g_Enemy[i][j].patternAnim = (g_Enemy[i][j].dir * TEXTURE_PATTERN_DIVIDE_X) +
						((g_Enemy[i][j].patternAnim + 1) % TEXTURE_PATTERN_DIVIDE_X);
				}

				// MAP外処理
				if (g_Enemy[i][j].pos.x < 0.0f)
				{
					g_Enemy[i][j].pos.x = 0.0f;
				}

				if (g_Enemy[i][j].pos.x > bg->w)
				{
					g_Enemy[i][j].pos.x = bg->w;
				}

				if (g_Enemy[i][j].pos.y < 0.0f)
				{
					g_Enemy[i][j].pos.y = 0.0f;
				}

				if (g_Enemy[i][j].pos.y > bg->h)
				{
					g_Enemy[i][j].pos.y = bg->h;
				}



				//エネミーのお互い当たり処理
				const float HALF_MIN_DISTANCE = TEXTURE_WIDTH * 0.5f;

				for (int k = 0; k < ENEMY_MAX; k++)
				{
					if (k != j && g_Enemy[i][k].use == TRUE)
					{
						XMVECTOR diff = XMLoadFloat3(&g_Enemy[i][j].pos) - XMLoadFloat3(&g_Enemy[i][k].pos);

						// エネミーのお互いの距離
						float distance = XMVectorGetX(XMVector2Length(diff));

						// 当たりました！位置を調整する
						if (distance < TEXTURE_WIDTH)
						{
							
							XMVECTOR normalizedDiff = XMVector2Normalize(diff);
							XMVECTOR newPos = XMLoadFloat3(&g_Enemy[i][k].pos) - normalizedDiff * (TEXTURE_WIDTH - distance) * 0.5f;
							XMStoreFloat3(&g_Enemy[i][k].pos, newPos);
						}
					}
				}
			}
		}
	}
}




//=============================================================================
// 描画処理
//=============================================================================
void DrawEnemy(void)
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


	BG* bg = GetBG();

	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		for (int j = 0; j < ENEMY_MAX; j++)
		{
			if (g_Enemy[i][j].use == TRUE)		
			{
				// テクスチャ設定
				GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[i]);

				//プレイヤーの位置やテクスチャー座標を反映
				float px = g_Enemy[i][j].pos.x - bg->pos.x;	// エネミーの表示位置X
				float py = g_Enemy[i][j].pos.y - bg->pos.y;	// エネミーの表示位置Y
				float pw = g_Enemy[i][j].w;		            // エネミーの表示幅
				float ph = g_Enemy[i][j].h;		            // エネミーの表示高さ

				 //アニメーション用
				float tw = 1.0f / TEXTURE_PATTERN_DIVIDE_X;	// テクスチャの幅
				float th = 1.0f / TEXTURE_PATTERN_DIVIDE_Y;	// テクスチャの高さ
				float tx = (float)(g_Enemy[i][j].patternAnim % TEXTURE_PATTERN_DIVIDE_X) * tw;	// テクスチャの左上X座標
				float ty = (float)(g_Enemy[i][j].patternAnim / TEXTURE_PATTERN_DIVIDE_X) * th;	// テクスチャの左上Y座標



				// １枚のポリゴンの頂点とテクスチャ座標を設定
				SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
					XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
					g_Enemy[i][j].rot.z);

				// ポリゴン描画
				GetDeviceContext()->Draw(4, 0);
			}
		}
	}


}


// Enemy構造体の先頭アドレスを取得
ENEMY* GetEnemy(void)
{
	return &g_Enemy[0][0];
}




//プレイヤーを中心に楕円形範囲内ランダムにエネミーを生成する
void SpawnEnemiesAtRandomPosition(PLAYER* player, int enemyType, int count)
{
	for (int i = 0; i < count; i++)
	{
		if (g_Enemy[enemyType][i].use)
		{
			continue;
		}

		g_Enemy[enemyType][i].use = TRUE;

		float angle = static_cast<float>(rand()) / RAND_MAX * DirectX::XM_2PI;
		float radiusX = ELLIPSE_RADIUS_X * (static_cast<float>(rand()) / RAND_MAX);
		float radiusY = ELLIPSE_RADIUS_Y * (static_cast<float>(rand()) / RAND_MAX);

		g_Enemy[enemyType][i].pos.x = player->pos.x + radiusX * cosf(angle);
		g_Enemy[enemyType][i].pos.y = player->pos.y + radiusY * sinf(angle);
	}
}