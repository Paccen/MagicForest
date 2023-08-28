//=============================================================================
// エネミー処理 [enemy.h]
//=============================================================================
#pragma once
#include "main.h"
#include "renderer.h"
#include "debugproc.h"
#include "sprite.h"
#include "player.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define ENEMY_MAX                    (100)  // エネミーのMax人数
#define TEXTURE_WIDTH				(200/3)	// キャラサイズ
#define TEXTURE_HEIGHT				(200/3)	// キャラサイズ
//*****************************************************************************
// 構造体定義
//*****************************************************************************

struct ENEMY
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
	int         blood;             //ブラッド
	int         attack;            //攻撃力

};


//エネミーの種類
enum
{
	ZOOBIE,
	GHOST,
	PUMPKIN,
	SPRITE
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitEnemy(void);
void UninitEnemy(void);
void UpdateEnemy(void);
void DrawEnemy(void);

ENEMY* GetEnemy(void);

void SpawnEnemiesAtRandomPosition(PLAYER* player, int enemyType, int count);


