//=============================================================================
// プレイヤー処理 [player.h]
//=============================================================================
#pragma once
#include "main.h"
#include "renderer.h"
#include "debugproc.h"
#include "sprite.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define PLAYER_MAX			(1)		// プレイヤーのMax人数

//向き
enum
{
	CHAR_DIR_DOWN,
	CHAR_DIR_LEFT,
	CHAR_DIR_RIGHT,
	CHAR_DIR_UP,

	CHAR_DIR_MAX
};

//*****************************************************************************
// 構造体定義
//*****************************************************************************

struct PLAYER
{
	XMFLOAT3	pos;			// ポリゴンの座標
	XMFLOAT3	rot;			// ポリゴンの回転量
	BOOL		use;			// true:使っている  false:未使用
	float		w, h;			// 幅と高さ
	float		countAnim;		// アニメーションカウント
	int			patternAnim;	// アニメーションパターンナンバー
	int			texNo;			// テクスチャ番号
	int			dir;			// 向き（0:上 1:右 2:下 3:左）
	BOOL		moving;			// 移動中フラグ
	XMFLOAT3	move;			// 移動速度
	XMFLOAT4    normalColor;    // デフォルト位置値
	int         blood;          // ブラッド
	int         xp;
	int         lv;
	int         boom;           // LEVELUP値
	int         killed;         // 撃破数
	BOOL        flashing;       // フラッシュ判定
	DWORD       flashStartTime; // フラッシュ時間

};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitPlayer(void);
void UninitPlayer(void);
void UpdatePlayer(void);
void DrawPlayer(void);

PLAYER* GetPlayer(void);



