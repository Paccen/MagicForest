//=============================================================================
// プレイヤー処理 [player.cpp]
//=============================================================================
#include "player.h"
#include "input.h"
#include "bg.h"
#include "bullet.h"
#include "enemy.h"
#include "collision.h"
#include "score.h"
#include "item.h"
#include "fade.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH               (200/2)	         // キャラサイズ
#define TEXTURE_HEIGHT              (200/2)	         // キャラサイズ
#define TEXTURE_MAX                 (8)		         // テクスチャの数

#define TEXTURE_PATTERN_DIVIDE_X    (3)		         // アニメパターンのテクスチャ内分割数（X)
#define TEXTURE_PATTERN_DIVIDE_Y    (4)		         // アニメパターンのテクスチャ内分割数（Y)
#define ANIM_PATTERN_NUM   (TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// アニメーションパターン数
#define ANIM_WAIT                   (4)		         // アニメーションの切り替わるWait値



#define PLAYER_DISP_X          (SCREEN_WIDTH/2)      // プレイヤーの画面内配置座標
#define PLAYER_DISP_Y       (SCREEN_HEIGHT/2 + TEXTURE_HEIGHT)



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/char01.png",
	"data/TEXTURE/char01_flash.png",
	"data/TEXTURE/bar_white.png",
	"data/TEXTURE/xp.png",
	"data/TEXTURE/lv.png",
	"data/TEXTURE/lvnum.png",
	"data/TEXTURE/killed.png",
	"data/TEXTURE/number.png",
};


static BOOL		g_Load = FALSE;			// 初期化を行ったかのフラグ
static PLAYER	g_Player[PLAYER_MAX];	// プレイヤー構造体


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitPlayer(void)
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


	// プレイヤー構造体の初期化
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		g_Player[i].use = TRUE;
		g_Player[i].pos = XMFLOAT3(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0.0f);	// 中心点から表示
		g_Player[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Player[i].w = TEXTURE_WIDTH;
		g_Player[i].h = TEXTURE_HEIGHT;
		g_Player[i].texNo = 0;
		g_Player[i].countAnim = 0;
		g_Player[i].patternAnim = 0;
		g_Player[i].move = XMFLOAT3(4.0f, 0.0f, 0.0f);		                    // 移動量(4,6)
		g_Player[i].dir = CHAR_DIR_DOWN;					                    // 下向きにしとくか
		g_Player[i].moving = FALSE;							                    // 移動中フラグ
		g_Player[i].patternAnim = g_Player[i].dir * TEXTURE_PATTERN_DIVIDE_X;
		g_Player[i].normalColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);             //デフォルト座標値
		g_Player[i].blood = 7;                                                  //ブラッド
		g_Player[i].flashing = FALSE;                                           //フラッシュ用
		g_Player[i].flashStartTime = 0;
		g_Player[i].xp = 0;                                                     //XP
		g_Player[i].lv = 0;                                                     //LV
		g_Player[i].boom = 5;                                                   //LEVEL UP値
		g_Player[i].killed = 0;                                                 //撃破数
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitPlayer(void)
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
void UpdatePlayer(void)
{
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		if (g_Player[i].use == TRUE)
		{
			// 地形との当たり判定用に座標のバックアップを取っておく
			XMFLOAT3 pos_old = g_Player[i].pos;

			// アニメーション  
			if (g_Player[i].moving == TRUE)
			{
				g_Player[i].countAnim += 1.0f;
				if (g_Player[i].countAnim > ANIM_WAIT)
				{
					g_Player[i].countAnim = 0.0f;
					// パターンの切り替え
					g_Player[i].patternAnim = (g_Player[i].dir * TEXTURE_PATTERN_DIVIDE_X) + ((g_Player[i].patternAnim + 1) % TEXTURE_PATTERN_DIVIDE_X);
				}
			}

			// キー入力で移動 
			float speed = g_Player[i].move.x;

			g_Player[i].moving = FALSE;

			if (GetKeyboardPress(DIK_C))       //加速度
			{
				speed *= 1.3;
			}
			if (GetKeyboardPress(DIK_DOWN))
			{
				g_Player[i].pos.y += speed;
				g_Player[i].dir = CHAR_DIR_DOWN;
				g_Player[i].moving = TRUE;
			}
			else if (GetKeyboardPress(DIK_UP))
			{
				g_Player[i].pos.y -= speed;
				g_Player[i].dir = CHAR_DIR_UP;
				g_Player[i].moving = TRUE;
			}
			if (GetKeyboardPress(DIK_RIGHT))
			{
				g_Player[i].pos.x += speed;
				g_Player[i].dir = CHAR_DIR_RIGHT;
				g_Player[i].moving = TRUE;
			}
			else if (GetKeyboardPress(DIK_LEFT))
			{
				g_Player[i].pos.x -= speed;
				g_Player[i].dir = CHAR_DIR_LEFT;
				g_Player[i].moving = TRUE;
			}


			// MAP外チェック
			BG* bg = GetBG();

			if (g_Player[i].pos.x < 0.0f)
			{
				g_Player[i].pos.x = 0.0f;
			}

			if (g_Player[i].pos.x > bg->w)
			{
				g_Player[i].pos.x = bg->w;
			}

			if (g_Player[i].pos.y < 0.0f)
			{
				g_Player[i].pos.y = 0.0f;
			}

			if (g_Player[i].pos.y > bg->h)
			{
				g_Player[i].pos.y = bg->h;
			}


			// プレイヤーの立ち位置からMAPのスクロール座標を計算する
			bg->pos.x = g_Player[i].pos.x - PLAYER_DISP_X;
			if (bg->pos.x < 0) bg->pos.x = 0;
			if (bg->pos.x > bg->w - SCREEN_WIDTH) bg->pos.x = bg->w - SCREEN_WIDTH;

			bg->pos.y = g_Player[i].pos.y - PLAYER_DISP_Y;
			if (bg->pos.y < 0) bg->pos.y = 0;
			if (bg->pos.y > bg->h - SCREEN_HEIGHT) bg->pos.y = bg->h - SCREEN_HEIGHT;


			//当たり判定をする
			ENEMY(*enemy)[ENEMY_MAX] = (ENEMY(*)[ENEMY_MAX])GetEnemy();
			for (int j = 0; j < 4; j++)
			{
				for (int k = 0; k < ENEMY_MAX; k++)
				{

					// 生きてるエネミーと当たり判定をする
					if (enemy[j][k].use == TRUE)
					{
						if (!g_Player[i].flashing)
						{
							//当たり判定関数で
							BOOL ans = CollisionBB(g_Player[i].pos, g_Player[i].w, g_Player[i].h,
								enemy[j][k].pos, enemy[j][k].w, enemy[j][k].h);

							// 当たっている？
							if (ans == TRUE)
							{

								g_Player[i].blood -= enemy[j][k].attack;

								if (g_Player[i].blood == 0)
								{
									SetFade(FADE_OUT, MODE_RESULT);
								}

								//無敵状態
								g_Player[i].flashing = TRUE;
								g_Player[i].flashStartTime = timeGetTime(); 


								 // エネミーの当たったら、後退する
								float backwardDistance = 35.0f; 

								// プレイヤーの向きを応じで後退方向計算する
								switch (g_Player[i].dir)
								{
								case CHAR_DIR_DOWN:
									g_Player[i].pos.y -= backwardDistance;
									break;
								case CHAR_DIR_UP:
									g_Player[i].pos.y += backwardDistance;
									break;
								case CHAR_DIR_RIGHT:
									g_Player[i].pos.x -= backwardDistance;
									break;
								case CHAR_DIR_LEFT:
									g_Player[i].pos.x += backwardDistance;
									break;

								}
							}
						}
					}
				}


				//　無敵状態の時間を計算する
				if (g_Player[i].flashing && (timeGetTime() - g_Player[i].flashStartTime >= 800))
				{
					g_Player[i].flashing = FALSE; 
				}


			}
		}
	}
}



//=============================================================================
// 描画処理
//=============================================================================
void DrawPlayer(void)
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

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		//プレイヤーを描画
		if (g_Player[i].use == TRUE)
		{

			 // 無敵状態確認
            if (g_Player[i].flashing)
            {
                DWORD currentTime = timeGetTime();
                DWORD flashDuration = 10; // フラッシュ时间（ミリ秒）

                //　交番に描画する
                int texIndex = (currentTime - g_Player[i].flashStartTime) / flashDuration % 2;
                GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[texIndex]);
            }
            else
            {
                // デフォルト状態値
                GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Player[i].texNo]);
            }


			//プレイヤーの位置やテクスチャー座標を反映
			float px = g_Player[i].pos.x - bg->pos.x;	// プレイヤーの表示位置X
			float py = g_Player[i].pos.y - bg->pos.y;	// プレイヤーの表示位置Y
			float pw = g_Player[i].w;                   // プレイヤーの表示幅
			float ph = g_Player[i].h;                   // プレイヤーの表示高さ


			// アニメーション用
			float tw = 1.0f / TEXTURE_PATTERN_DIVIDE_X;	// テクスチャの幅
			float th = 1.0f / TEXTURE_PATTERN_DIVIDE_Y;	// テクスチャの高さ
			float tx = (float)(g_Player[i].patternAnim % TEXTURE_PATTERN_DIVIDE_X) * tw;	// テクスチャの左上X座標
			float ty = (float)(g_Player[i].patternAnim / TEXTURE_PATTERN_DIVIDE_X) * th;	// テクスチャの左上Y座標


			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				g_Player[i].normalColor,
				g_Player[i].rot.z);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);


			// ゲージ(红)を描画
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);
			float g_px = g_Player[i].pos.x - bg->pos.x - TEXTURE_WIDTH /2  + 20;		// ゲージの表示位置X
			float g_py = g_Player[i].pos.y - bg->pos.y + TEXTURE_HEIGHT/2  + 10;		// ゲージの表示位置Y
			float g_pw = g_Player[i].blood * 10.0f;                                     // ゲージの表示幅
			float g_ph = 10.0f;                                                         // ゲージの表示高さ

			float g_tw = 1.0f;	// テクスチャの幅
			float g_th = 1.0f;	// テクスチャの高さ
			float g_tx = 0.0f;	// テクスチャの左上X座標
			float g_ty = 0.0f;	// テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteLTColor(g_VertexBuffer,
				g_px, g_py, g_pw, g_ph,
				g_tx, g_ty, g_tw, g_th,
				XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);


			// ゲージ(黑）
			const float BLOOD_DROP_WIDTH = 10.0f;
			const int MAX_BLOOD_DROPS = 7;

			int currentBloodDrops = g_Player[i].blood;
			for (int drop = currentBloodDrops; drop < MAX_BLOOD_DROPS; drop++)
			{
				// ブラッド計算する
				float g_px_lost = g_px + drop * BLOOD_DROP_WIDTH;
				float g_py_lost = g_py;
				float g_ph_lost = g_ph;
				float g_pw_lost = BLOOD_DROP_WIDTH;
				float g_tw_lost = 1.0f;
				float g_th_lost = 1.0f;
				float g_tx_lost = 0.0f;
				float g_ty_lost = 0.0f;

				// １枚のポリゴンの頂点とテクスチャ座標を設定
				SetSpriteLTColor(g_VertexBuffer,
					g_px_lost, g_py_lost, g_pw_lost, g_ph_lost,
					g_tx_lost, g_ty_lost, g_tw_lost, g_th_lost,
					XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f));       // RGB(0,0,0) 黒い

				// ポリゴン描画
				GetDeviceContext()->Draw(4, 0);
			}


			// XPフレームの描画を描画
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[3]);
			float xp_px = 0.0f;		            // ゲージの表示位置X
			float xp_py = 0.0f;		            // ゲージの表示位置Y
			float xp_pw = SCREEN_WIDTH - 8;		// ゲージの表示幅
			float xp_ph = 45.0f;		        // ゲージの表示高さ

			float xp_tw = 1.0f;	                // テクスチャの幅
			float xp_th = 1.0f;	                // テクスチャの高さ
			float xp_tx = 0.0f;	                // テクスチャの左上X座標
			float xp_ty = 0.0f;	                // テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteLTColor(g_VertexBuffer,
				xp_px, xp_py, xp_pw, xp_ph,
				xp_tx, xp_ty, xp_tw, xp_th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);



			// XP内容塗り(blue)を描画
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);
			float xpb_px = 4.0f;		// ゲージの表示位置X
			float xpb_py = 4.0f;		// ゲージの表示位置Y
			float xpb_pw = ((SCREEN_WIDTH - 8)/ g_Player[i].boom) * g_Player[i].xp;		// ゲージの表示幅
			float xpb_ph = 37.0f;		// ゲージの表示高さ

			float xpb_tw = 1.0f;	    // テクスチャの幅
			float xpb_th = 1.0f;	    // テクスチャの高さ
			float xpb_tx = 0.0f;	    // テクスチャの左上X座標
			float xpb_ty = 0.0f;	    // テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteLTColor(g_VertexBuffer,
				xpb_px, xpb_py, xpb_pw, xpb_ph,
				xpb_tx, xpb_ty, xpb_tw, xpb_th,
				XMFLOAT4(18 / 255.0f, 113 / 255.0f, 142 / 255.0f, 1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);



			//LVテクスチャを描画
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[4]);
			float lv_px = 4.0f;		// ゲージの表示位置X
			float lv_py = 4.0f;		// ゲージの表示位置Y

			float lv_pw = 40.0f;	// ゲージの表示幅
			float lv_ph = 40.0f;	// ゲージの表示高さ
			float lv_tw = 1.0f;	    // テクスチャの幅
			float lv_th = 1.0f;	    // テクスチャの高さ
			float lv_tx = 0.0f;	    // テクスチャの左上X座標
			float lv_ty = 0.0f;	    // テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteLTColor(g_VertexBuffer,
				lv_px, lv_py, lv_pw, lv_ph,
				lv_tx, lv_ty, lv_tw, lv_th,
				XMFLOAT4(1.0f,1.0f,1.0f,0.5f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);


			//数字テクスチャを描画(LV)
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[5]);
			int number = g_Player[i].lv;

			// 術を計算
			int digits = 0;
			int tempNumber = number;
			do 
			{
				digits++;
				tempNumber /= 10;
			} 
			while (tempNumber > 0);

			for (int h = 0; h < digits; h++)
			{
				// 今回表示する桁の数字
				float x = (float)(number % 10);

				// スコアの位置やテクスチャー座標を反映
				float lvn_px = 90.0f - h * 25.0f;	// スコアの表示位置X
				float lvn_py = 22.0f;			    // スコアの表示位置Y

				float lvn_pw = 25.0f;				// スコアの表示幅
				float lvn_ph = 30.0f;				// スコアの表示高さ
				float lvn_tw = 1.0f / 10;		    // テクスチャの幅
				float lvn_th = 1.0f;		        // テクスチャの高さ
				float lvn_tx = x * lvn_tw;			// テクスチャの左上X座標
				float lvn_ty = 0.0f;			    // テクスチャの左上Y座標

				// １枚のポリゴンの頂点とテクスチャ座標を設定
				SetSpriteColor(g_VertexBuffer, lvn_px, lvn_py, lvn_pw, lvn_ph, lvn_tx, lvn_ty, lvn_tw, lvn_th,
					XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f));

				// ポリゴン描画
				GetDeviceContext()->Draw(4, 0);

				// 次の桁へ
				number /= 10;
			}


			//KILLEDテクスチャを描画
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[6]);

			float killed_px = 10.0f;		// ゲージの表示位置X
			float killed_py = 60.0f;		// ゲージの表示位置Y

			float killed_pw = 60.0f;		// ゲージの表示幅
			float killed_ph = 60.0f;		// ゲージの表示高さ
			float killed_tw = 1.0f;         // テクスチャの幅
			float killed_th = 1.0f;	        // テクスチャの高さ
			float killed_tx = 0.0f;	        // テクスチャの左上X座標
			float killed_ty = 0.0f;	        // テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteLTColor(g_VertexBuffer,
				killed_px, killed_py, killed_pw, killed_ph,
				killed_tx, killed_ty, killed_tw, killed_th,
				XMFLOAT4(1.0f,1.0f,1.0f,1.0f));

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);




			//数字テクスチャを描画(KILLED)
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[7]);
			int number_n = g_Player[i].killed;

			// 術を計算
			int digits_n = 0;
			int tempNumber_n = number_n;
			do
			{
				digits_n++;
				tempNumber_n /= 10;
			} 
			while (tempNumber_n > 0);

			for (int r = 0; r < digits_n; r++) 
			{
				// 今回表示する桁の数字
				float x = (float)(number_n % 10);

				// スコアの位置やテクスチャー座標を反映
				float killedn_px = 160.0f - r * 25.0f;	// スコアの表示位置X
				float killedn_py = 100.0f;			    // スコアの表示位置Y

				float killedn_pw = 25.0f;				// スコアの表示幅
				float killedn_ph = 30.0f;				// スコアの表示高さ
				float killedn_tw = 1.0f / 10;		    // テクスチャの幅
				float killedn_th = 1.0f;		        // テクスチャの高さ
				float killedn_tx = x * killedn_tw;		// テクスチャの左上X座標
				float killedn_ty = 0.0f;			    // テクスチャの左上Y座標

				// １枚のポリゴンの頂点とテクスチャ座標を設定
				SetSpriteColor(g_VertexBuffer, killedn_px, killedn_py, killedn_pw, killedn_ph, killedn_tx, killedn_ty, killedn_tw, killedn_th,
					XMFLOAT4(1.0f, 0.0f, 0.0f, 0.5f));

				// ポリゴン描画
				GetDeviceContext()->Draw(4, 0);

				// 次の桁へ
				number_n /= 10;
			}
		}
	}
}


// Player構造体の先頭アドレスを取得
PLAYER* GetPlayer(void)
{
	return &g_Player[0];
}




