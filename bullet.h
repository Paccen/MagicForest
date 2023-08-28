//=============================================================================
// バレット処理 [bullet.h]
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"
#include "sprite.h"
#include <random>  // 随机数库

//*****************************************************************************
// マクロ定義
//*****************************************************************************



// バレット構造体
struct BULLET
{
	BOOL				use;                // true:使っている  false:未使用
	float				w, h;               // 幅と高さ
	XMFLOAT3			pos;                // バレットの座標
	XMFLOAT3			rot;				// バレットの回転量
	XMFLOAT3			move;				// バレットの移動量
	int					countAnim;			// アニメーションカウント
	int					patternAnim;		// アニメーションパターンナンバー
	int					texNo;				// 何番目のテクスチャーを使用するのか
	float               angle;              // FIREの角度
	BOOL                animationCompleted; // LIGHTINGのアニメーション頻度をコントロールする
};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitBullet(void);
void UninitBullet(void);
void UpdateBullet(void);
void DrawBullet(void);


//LEVELUP後スキルアップ用
void SetAxe(void);
void SetBook(void);
void SetFire(void);
void SetLighting(void);

