//=============================================================================
// バレット処理 [bullet.cpp]
//=============================================================================

#include "bullet.h"
#include"main.h"
#include "enemy.h"
#include "collision.h"
#include "score.h"
#include "bg.h"
#include "effect.h"
#include "item.h"
#include "score.h"
#include "player.h"
#include<time.h>
#include <math.h>
#include<cmath>
//*****************************************************************************
// マクロ定義
//*****************************************************************************

#define TEXTURE_MAX	                     (4)    // テクスチャの数
#define M_PI         (3.14159265358979323846)   // pi


//AXE
#define TEXTURE_WIDTH_AXE               (80)    // キャラサイズ
#define TEXTURE_HEIGHT_AXE              (80)    // キャラサイズ
#define TEXTURE_PATTERN_DIVIDE_X_AXE    (4)     // アニメパターンのテクスチャ内分割数（X)
#define TEXTURE_PATTERN_DIVIDE_Y_AXE    (1)     // アニメパターンのテクスチャ内分割数（Y)
#define ANIM_PATTERN_NUM_AXE  (TEXTURE_PATTERN_DIVIDE_X_AXE*TEXTURE_PATTERN_DIVIDE_Y_AXE)	// アニメーションパターン数
#define ANIM_WAIT_AXE                   (5)     // アニメーションの切り替わるWait値
#define AXE_SPEED                      (6.0f)   // スピード
#define AXE_MAX                         (5)     //最大値


//BOOK
#define TEXTURE_WIDTH_BOOK              (53)    // キャラサイズ
#define TEXTURE_HEIGHT_BOOK             (53)    // キャラサイズ
#define BOOK_MAX                        (5)     // 最大値
#define BOOK_RADIUS                     (150)   // 回す半径
#define BOOK_ROTATION_SPEED             (0.03f) // 回すスピード


//FIRE
#define TEXTURE_WIDTH_FIRE              (150)   // キャラサイズ
#define TEXTURE_HEIGHT_FIRE				(100)   // キャラサイズ
#define FIRE_MAX                         (5)    // 最大値
#define FIREBALL_SPEED                   (4)    // スピード


//LIGHTING
#define TEXTURE_WIDTH_LIGHTING                  (80)   // キャラサイズ
#define TEXTURE_HEIGHT_LIGHTING                (240)   // キャラサイズ
#define TEXTURE_PATTERN_DIVIDE_X_LIGHTING       (3)    // アニメパターンのテクスチャ内分割数（X)
#define TEXTURE_PATTERN_DIVIDE_Y_LIGHTING       (3)	   // アニメパターンのテクスチャ内分割数（Y)
#define ANIM_PATTERN_NUM_LIGHTING         (TEXTURE_PATTERN_DIVIDE_X_LIGHTING*TEXTURE_PATTERN_DIVIDE_Y_LIGHTING)	// アニメーションパターン数
#define ANIM_WAIT_LIGHTING                      (5)    // アニメーションの切り替わるWait値
#define LIGHTING_MAX                            (5)    // 最大値


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報


static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/axe.png",  
	"data/TEXTURE/book.png", 
	"data/TEXTURE/fire.png", 
	"data/TEXTURE/lightning.png"
};

static BOOL		g_Load = FALSE;			// 初期化を行ったかのフラグ

//タイマー(攻撃頻度用)
static int g_LastAxeThrowTime = 0;
static int g_LastFireThrowTime = 0;
static int g_LastLightningThrowTime = 0;


//AXE
static BULLET	g_Axe[AXE_MAX];	


//BOOK
static BULLET	g_Book[BOOK_MAX];	
static float g_BookRotation = 0.0f;//回すスピード用グローバル変数

//FIRE
static BULLET	g_Fire[FIRE_MAX];

//LIGHTING
static BULLET	g_Lighting[LIGHTING_MAX];


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitBullet(void)
{
	srand((unsigned)time(NULL));
	g_LastAxeThrowTime = GetTime();
	g_LastFireThrowTime = GetTime();
	g_LastLightningThrowTime = GetTime();

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


	//AXEの初期化
	for (int i = 0; i < AXE_MAX; i++)
	{

		g_Axe[i].use   = FALSE;
		g_Axe[i].w     = TEXTURE_WIDTH_AXE;
		g_Axe[i].h     = TEXTURE_HEIGHT_AXE;
		g_Axe[i].pos   = XMFLOAT3(3000.0f, 0.0f, 1.0f);
		g_Axe[i].rot   = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Axe[i].texNo = 0;

		g_Axe[i].countAnim = 0;
		g_Axe[i].patternAnim = 0;

	}
	

	//BOOKの初期化
	for (int i = 0; i < BOOK_MAX; i++)
	{

		g_Book[i].use   = FALSE;
		g_Book[i].w     = TEXTURE_WIDTH_BOOK;
		g_Book[i].h     = TEXTURE_HEIGHT_BOOK;
		g_Book[i].pos   = XMFLOAT3(3000.0f, 0.0f, 1.0f);   
		g_Book[i].rot   = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Book[i].texNo = 1;
	
	}
	//初め持ってるスキル
	g_Book[0].use = TRUE;


	//FIREの初期化     
	for (int i = 0; i < FIRE_MAX; i++)
	{

		g_Fire[i].use   = FALSE;
		g_Fire[i].w     = TEXTURE_WIDTH_FIRE;
		g_Fire[i].h     = TEXTURE_HEIGHT_FIRE;
		g_Fire[i].pos   = XMFLOAT3(3000.0f, 0.0f, 1.0f);
		g_Fire[i].rot   = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Fire[i].texNo = 2;
		g_Fire[i].angle = 0.0f;

	}


	// LIGHTINGの初期化
	for (int i = 0; i < LIGHTING_MAX; i++)
	{
		g_Lighting[i].use         = FALSE;
		g_Lighting[i].w           = TEXTURE_WIDTH_LIGHTING;
		g_Lighting[i].h           = TEXTURE_HEIGHT_LIGHTING;
		g_Lighting[i].pos         = XMFLOAT3(3000.0f, 0.0f, 1.0f);
		g_Lighting[i].rot         = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Lighting[i].texNo       = 3;
		g_Lighting[i].countAnim   = 0;
		g_Lighting[i].patternAnim = 0;


	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitBullet(void)
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

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateBullet(void)
{
	PLAYER* player = GetPlayer();


	// **********************************　　AXEの更新　　************************************ 
	//ゲーム時間を得る
	int currentTime_AXE = GetTime();

	//攻撃の頻度は３ｓ／１回
	if (currentTime_AXE - g_LastAxeThrowTime >= 3)
	{

		//ランダムに初期化する
		for (int i = 0; i < AXE_MAX; i++)
		{
			g_Axe[i].pos = player->pos;
			g_Axe[i].move.x = rand() % 25 - 12;
			g_Axe[i].move.y = -(rand() % 10);
		}

		//タイマーを更新する
		g_LastAxeThrowTime = currentTime_AXE;
		
	}

	for (int i = 0; i < AXE_MAX; i++)
	{

		// このバレットが使われている？
		if (g_Axe[i].use == TRUE)	
		{
			// アニメーション用  
			g_Axe[i].countAnim++;
			if ((g_Axe[i].countAnim % ANIM_WAIT_AXE) == 0)
			{
				// パターンの切り替え
				g_Axe[i].patternAnim = (g_Axe[i].patternAnim + 1) % ANIM_PATTERN_NUM_AXE;
			}


			// パラボラ（放物線）で移動する
			g_Axe[i].move.y += 0.1f;  // 重力加速度

			//移動を更新する
			XMVECTOR pos = XMLoadFloat3(&g_Axe[i].pos);
			XMVECTOR move = XMLoadFloat3(&g_Axe[i].move);
			pos += move;
			XMStoreFloat3(&g_Axe[i].pos, pos);

			// エネミーの当たり判定処理
			{
				ENEMY(*enemy)[ENEMY_MAX] = (ENEMY(*)[ENEMY_MAX])GetEnemy();
				for (int j = 0; j < 4; j++)
				{
					for (int k = 0; k < ENEMY_MAX; k++)
					{
						if (enemy[j][k].use == TRUE)
						{
							//当たり判定関数で
							BOOL ans = CollisionBB(g_Axe[i].pos, g_Axe[i].w, g_Axe[i].h,
								enemy[j][k].pos, enemy[j][k].w, enemy[j][k].h);

							// 当たっている？
							if (ans == TRUE)
							{

								// 当たった時の処理
								enemy[j][k].use = FALSE;

								//撃破カウンター
								player->killed += 1;

								// エフェクト発生
								SetEffect(enemy[j][k].pos.x, enemy[j][k].pos.y, 30);

								//アイテム生成する
								SetItem(enemy[j][k].pos.x, enemy[j][k].pos.y);
								

							}
							

						}
					}
				}
			}

		}
	}


	// **********************************　　BOOKの更新　　************************************ 
	//回す処理
	g_BookRotation += BOOK_ROTATION_SPEED;
	if (g_BookRotation > 2 * M_PI)
	{
		g_BookRotation -= 2 * M_PI;
	}

	//使用中のBOOKを計算する
	int booksInUse = 0;
	for (int i = 0; i < BOOK_MAX; i++)
	{
		if (g_Book[i].use == TRUE)
		{
			booksInUse++;
		}
	}

	//使用中のBOOKに応じて、BOOKとBOOKの角度を計算する
	float angleDifference;
	if (booksInUse > 0)
	{
		angleDifference = 2 * M_PI / booksInUse;
	}
	else
	{
		angleDifference = 0;
	}

	
	int bookIndex = 0;
	for (int i = 0; i < BOOK_MAX; i++)
	{
		// BOOKの位置を計算して更新する
		if (g_Book[i].use == TRUE)
		{
			float angle = angleDifference * bookIndex + g_BookRotation;
			g_Book[i].pos = XMFLOAT3(player->pos.x + BOOK_RADIUS * cosf(angle) - 25, player->pos.y + BOOK_RADIUS * sinf(angle)- 25 , 1.0f);
			bookIndex++;
		}

		// エネミーの当たり判定処理
		ENEMY(*enemy)[ENEMY_MAX] = (ENEMY(*)[ENEMY_MAX])GetEnemy();
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < ENEMY_MAX; k++)
			{
				if (enemy[j][k].use == TRUE)
				{
					//当たり判定関数で
					BOOL ans = CollisionBB(g_Book[i].pos, g_Book[i].w, g_Book[i].h,
						enemy[j][k].pos, enemy[j][k].w, enemy[j][k].h);

					// 当たっている？
					if (ans == TRUE)
					{
							// 当たった時の処理
							enemy[j][k].use = FALSE;

							//撃破カウンター
							player->killed += 1;

							// エフェクト発生
							SetEffect(enemy[j][k].pos.x, enemy[j][k].pos.y, 30);

							//アイテム生成する
							SetItem(enemy[j][k].pos.x, enemy[j][k].pos.y);

					}
				}
			}
		}
	}


	// **********************************　　FIREの更新　　************************************ 
	//ゲーム時間を得る
	int currentTime_FIRE = GetTime();

	//攻撃の頻度は３ｓ／１回
	if (currentTime_FIRE - g_LastFireThrowTime >= 3)
	{
		for (int i = 0; i < FIRE_MAX; i++)
		{
			if (g_Fire[i].use == TRUE)
			{
				g_Fire[i].pos = player->pos;

				//プレイヤーを中心にランダムに角度を生成する
				g_Fire[i].angle = (float)(rand()) / RAND_MAX * 2.0f * XM_PI;

				//位置を計算する
				g_Fire[i].move.x = FIREBALL_SPEED * cosf(g_Fire[i].angle); 
				g_Fire[i].move.y = FIREBALL_SPEED * sinf(g_Fire[i].angle); 
			}
			
			g_LastFireThrowTime = currentTime_FIRE;
		}	

	}

	for (int i = 0; i < FIRE_MAX; i++)
	{

		// 使われている？
		if (g_Fire[i].use == TRUE)	
		{
			//位置を更新する
			XMVECTOR pos = XMLoadFloat3(&g_Fire[i].pos);
			XMVECTOR move = XMLoadFloat3(&g_Fire[i].move);
			pos += move;
			XMStoreFloat3(&g_Fire[i].pos, pos);
		}

		// エネミーの当たり判定処理
		ENEMY(*enemy)[ENEMY_MAX] = (ENEMY(*)[ENEMY_MAX])GetEnemy();
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < ENEMY_MAX; k++)
			{
				if (enemy[j][k].use == TRUE)
				{
					//当たり判定関数で
					BOOL ans = CollisionBB(g_Fire[i].pos, g_Fire[i].w, g_Fire[i].h,
						enemy[j][k].pos, enemy[j][k].w, enemy[j][k].h);

					// 当たっている？
					if (ans == TRUE)
					{
						// 当たった時の処理
						enemy[j][k].use = FALSE;

						//撃破カウンター
						player->killed += 1;

						// エフェクト発生
						SetEffect(enemy[j][k].pos.x, enemy[j][k].pos.y, 30);

						//アイテム生成する
						SetItem(enemy[j][k].pos.x, enemy[j][k].pos.y);
					}
				}
			}
		}
	}

	// **********************************　　FIREの更新　　************************************
	//ゲーム時間を得る
	int currentTime_LIGHTNING = GetTime();

	if (currentTime_LIGHTNING - g_LastLightningThrowTime >= 3)
	{
		for (int i = 0; i < LIGHTING_MAX; i++)
		{
			if (g_Lighting[i].use == TRUE)
			{
				// ランダムに位置を初期化する
				int randomX = (rand() % 1600) - 800;
				int randomY = (rand() % 800) - 400;

				//プレイヤーの周りに生成する
				g_Lighting[i].pos.x = randomX + player[0].pos.x;
				g_Lighting[i].pos.y = randomY + player[0].pos.y;
				g_Lighting[i].animationCompleted = FALSE;
			}
		}

		g_LastLightningThrowTime = currentTime_LIGHTNING;
	}

	// LIGHTINGのアニメーションを更新する
	for (int i = 0; i < LIGHTING_MAX; i++)
	{
		//アニメーションは一回だけを制定する
		if (g_Lighting[i].use == TRUE && g_Lighting[i].animationCompleted == FALSE)
		{
			g_Lighting[i].countAnim++;
			if ((g_Lighting[i].countAnim % ANIM_WAIT_AXE) == 0)
			{
				// パターンの切り替え
				g_Lighting[i].patternAnim = (g_Lighting[i].patternAnim + 1) % ANIM_PATTERN_NUM_LIGHTING;
				if (g_Lighting[i].patternAnim == 0) 
				{
					g_Lighting[i].animationCompleted = TRUE;
				}

			}

			// エネミーの当たり判定処理
			ENEMY(*enemy)[ENEMY_MAX] = (ENEMY(*)[ENEMY_MAX])GetEnemy();
			for (int j = 0; j < 4; j++)
			{
				for (int k = 0; k < ENEMY_MAX; k++)
				{
					if (enemy[j][k].use == TRUE)
					{
						//当たり判定関数で
						BOOL ans = CollisionBB(g_Lighting[i].pos, g_Lighting[i].w, g_Lighting[i].w,
							enemy[j][k].pos, enemy[j][k].w, enemy[j][k].h);

						// 当たっている？
						if (ans == TRUE)
						{
							// 当たった時の処理
							enemy[j][k].use = FALSE;

							//撃破カウンター
							player->killed += 1;

							// エフェクト発生
							SetEffect(enemy[j][k].pos.x, enemy[j][k].pos.y, 30);

							//アイテム生成する
							SetItem(enemy[j][k].pos.x, enemy[j][k].pos.y);

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
void DrawBullet(void)
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


	// **********************************　AXEの描画　************************************
	for (int i = 0; i < AXE_MAX; i++)
	{
		if (g_Axe[i].use == TRUE)
		{									
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

			//バレットの位置やテクスチャー座標を反映
			float px = g_Axe[i].pos.x - bg->pos.x;	// 表示位置X
			float py = g_Axe[i].pos.y - bg->pos.y;	// 表示位置Y
			float pw = g_Axe[i].w;		            // 表示幅
			float ph = g_Axe[i].h;		            // 表示高さ

			float tw = 1.0f / TEXTURE_PATTERN_DIVIDE_X_AXE;	                                // テクスチャの幅
			float th = 1.0f / TEXTURE_PATTERN_DIVIDE_Y_AXE;                                 // テクスチャの高さ
			float tx = (float)(g_Axe[i].patternAnim % TEXTURE_PATTERN_DIVIDE_X_AXE) * tw;   // テクスチャの左上X座標
			float ty = (float)(g_Axe[i].patternAnim / TEXTURE_PATTERN_DIVIDE_X_AXE) * th;   // テクスチャの左上Y座標

			// 1枚のポリゴンの頂点とテクスチャ座標を設定
			// 回りある
			SetSpriteColorRotation(g_VertexBuffer, 
				px, py, pw, ph, 
				tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Axe[i].rot.z);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);

		}
	}



	// **********************************　BOOKの描画　************************************
	for (int i = 0; i < BOOK_MAX; i++)
	{
		if (g_Book[i].use == TRUE)		
		{									
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);
			float g_px = g_Book[i].pos.x - bg->pos.x ;		// 表示位置X
			float g_py = g_Book[i].pos.y - bg->pos.y;		// 表示位置Y
			float g_pw = g_Book[i].w;	                    // 表示幅
			float g_ph = g_Book[i].h;                       // 表示高さ

			float g_tw = 1.0f;	                            // テクスチャの幅
			float g_th = 1.0f;	                            // テクスチャの高さ
			float g_tx = 0.0f;                              // テクスチャの左上X座標
			float g_ty = 0.0f;                              // テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteLTColor(g_VertexBuffer,
				g_px, g_py, g_pw, g_ph,
				g_tx, g_ty, g_tw, g_th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);

		}
	}


	// **********************************　FIREの描画　************************************
	for (int i = 0; i < FIRE_MAX; i++)
	{
		if (g_Fire[i].use == TRUE )
		{
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);
			float g_px = g_Fire[i].pos.x - bg->pos.x - 25.0f;    // 表示位置X
			float g_py = g_Fire[i].pos.y - bg->pos.y - 25.0f;    // 表示位置Y
			float g_pw = g_Fire[i].w;                            // 表示幅
			float g_ph = g_Fire[i].h;                            // 表示高さ

			float g_tw = 1.0f;                                   // テクスチャの幅
			float g_th = 1.0f;                                   // テクスチャの高さ
			float g_tx = 0.0f;                                   // テクスチャの左上X座標
			float g_ty = 0.0f;                                   // テクスチャの左上Y座標
			float rotationAngle = g_Fire[i].angle;

			SetSpriteLTColorRota(g_VertexBuffer,
				g_px, g_py, g_pw, g_ph,
				g_tx, g_ty, g_tw, g_th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), rotationAngle);

			GetDeviceContext()->Draw(4, 0);
		}
	}



	// **********************************　LIGHTINGの描画　************************************
	for (int i = 0; i < LIGHTING_MAX; i++)
	{
		if (g_Lighting[i].use == TRUE && g_Lighting[i].animationCompleted == FALSE)
		{								
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[3]);

			//バレットの位置やテクスチャー座標を反映
			float px = g_Lighting[i].pos.x - bg->pos.x;             // 表示位置X
			float py = g_Lighting[i].pos.y - bg->pos.y;	            // 表示位置Y
			float pw = g_Lighting[i].w;					            // 表示幅
			float ph = g_Lighting[i].h;					            // 表示高さ

			float tw = 1.0f / TEXTURE_PATTERN_DIVIDE_X_LIGHTING;	                                 // テクスチャの幅
			float th = 1.0f / TEXTURE_PATTERN_DIVIDE_Y_LIGHTING;                                     // テクスチャの高さ
			float tx = (float)(g_Lighting[i].patternAnim % TEXTURE_PATTERN_DIVIDE_X_LIGHTING) * tw;  // テクスチャの左上X座標
			float ty = (float)(g_Lighting[i].patternAnim / TEXTURE_PATTERN_DIVIDE_X_LIGHTING) * th;  // テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColorRotation(g_VertexBuffer, 
				px, py, pw, ph, 
				tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Lighting[i].rot.z);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);

		}
	}

























}


//=============================================================================
// バレット構造体の先頭アドレスを取得
//=============================================================================
BULLET *GetBullet(void)
{
	return &g_Axe[0];
}



void SetAxe(void)
{
	for (int i = 0; i < AXE_MAX; i++)
	{
		if (g_Axe[i].use == FALSE)
		{
			g_Axe[i].use = TRUE;
			break;
		}
	}
}

void SetBook(void)
{
	for (int i = 0; i < BOOK_MAX; i++)
	{
		if (g_Book[i].use == FALSE)
		{
			g_Book[i].use = TRUE;
			break;
		}
	}
}

void SetFire(void)
{
	for (int i = 0; i < FIRE_MAX; i++)
	{
		if (g_Fire[i].use == FALSE)
		{
			g_Fire[i].use = TRUE;
			break;
		}
	}
}

void SetLighting(void)
{
	for (int i = 0; i < LIGHTING_MAX; i++)
	{
		if (g_Lighting[i].use == FALSE)
		{
			g_Lighting[i].use = TRUE;
			break;
		}
	}
}

