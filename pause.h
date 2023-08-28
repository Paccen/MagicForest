//=============================================================================
// [pause.h]
//=============================================================================
#pragma once
#include "main.h"
#include "renderer.h"
#include "debugproc.h"
#include "sprite.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
struct PAUSE
{
	XMFLOAT3	pos;		// ポリゴンの座標
	float		w, h;		// 幅と高さ
	int			texNo;		// 使用しているテクスチャ番号
	int         speed;      //スビート
	BOOL        use;        //使ってる？

};



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitPause(void);
void UninitPause(void);
void UpdatePause(int MENU_LEVELUP_START);
void DrawPause(int MENU_LEVELUP_START);

void SetXP(int x);
BOOL checkMax(int x);
BOOL checkXpMAX(void);