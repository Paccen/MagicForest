//=============================================================================
// BG処理 [item.h]
//=============================================================================
#pragma once
#include "main.h"
#include "renderer.h"
#include "debugproc.h"
#include "sprite.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_ITEM         ENEMY_MAX * 10 



struct ITEAM
{
	XMFLOAT3     pos;		// ポリゴンの座標
	float        w, h;		// 幅と高さ
	int          texNo;		// 使用しているテクスチャ番号
	float        alpha;
	BOOL         use;
	int      creationTime;  // アイテム生成する時間を得るため
};




//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitItem(void);
void UninitItem(void);
void UpdateItem(void);
void DrawItem(void);

ITEAM* GetItem(void);
int generateValue(void);
void SetItem(float x, float y);

