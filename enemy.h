//=============================================================================
// �G�l�~�[���� [enemy.h]
//=============================================================================
#pragma once
#include "main.h"
#include "renderer.h"
#include "debugproc.h"
#include "sprite.h"
#include "player.h"
//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define ENEMY_MAX                    (100)  // �G�l�~�[��Max�l��
#define TEXTURE_WIDTH				(200/3)	// �L�����T�C�Y
#define TEXTURE_HEIGHT				(200/3)	// �L�����T�C�Y
//*****************************************************************************
// �\���̒�`
//*****************************************************************************

struct ENEMY
{
	XMFLOAT3	pos;			// �|���S���̍��W
	XMFLOAT3	rot;			// �|���S���̉�]��
	BOOL		use;			// true:�g���Ă���  false:���g�p
	float		w, h;			// ���ƍ���
	float		countAnim;		// �A�j���[�V�����J�E���g
	int			patternAnim;	// �A�j���[�V�����p�^�[���i���o�[
	int			texNo;			// �e�N�X�`���ԍ�
	int			dir;			// �����i0:�� 1:�E 2:�� 3:���j
	BOOL		moving;			// �ړ����t���O
	XMFLOAT3	move;			// �ړ����x
	int         blood;             //�u���b�h
	int         attack;            //�U����

};


//�G�l�~�[�̎��
enum
{
	ZOOBIE,
	GHOST,
	PUMPKIN,
	SPRITE
};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitEnemy(void);
void UninitEnemy(void);
void UpdateEnemy(void);
void DrawEnemy(void);

ENEMY* GetEnemy(void);

void SpawnEnemiesAtRandomPosition(PLAYER* player, int enemyType, int count);


