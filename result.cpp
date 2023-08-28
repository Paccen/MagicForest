//=============================================================================
// ���U���g��ʏ��� [result.cpp]
//=============================================================================
#include "result.h"
#include "input.h"
#include "score.h"
#include "fade.h"
#include "pause.h"
//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH			 	(SCREEN_WIDTH)  	// �w�i�T�C�Y
#define TEXTURE_HEIGHT			 	(SCREEN_HEIGHT)	    // �w�i�T�C�Y
#define TEXTURE_MAX				     	 (2)				// �e�N�X�`���̐�

#define TEXTURE_WIDTH_POINT              (80)
#define TEXTURE_HEIGHT_POINT             (80)

#define SOME_UPPER_VALUE_MENU         (SCREEN_HEIGHT / 2  + 100.0f)
#define SOME_LOWER_VALUE_MENU         (SCREEN_HEIGHT / 2  + 270.0f)
//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		        // ���_���
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char *g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/gameover.png",
	"data/TEXTURE/point.png",

};



static BOOL						g_Load = FALSE;
static PAUSE	                g_Ponit;
//=============================================================================
// ����������
//=============================================================================
HRESULT InitResult(void)
{
	ID3D11Device *pDevice = GetDevice();

	//�e�N�X�`������
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


	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	//LEVELUP�t���[���̏�����
	g_Ponit.w = TEXTURE_WIDTH_POINT;
	g_Ponit.h = TEXTURE_HEIGHT_POINT;
	g_Ponit.pos = XMFLOAT3(SCREEN_WIDTH / 2 - 400.0f, SCREEN_HEIGHT / 2 + 100.0f, 0.0f);
	g_Ponit.speed = 170;


	// BGM�Đ�


	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitResult(void)
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
// �X�V����
//=============================================================================
void UpdateResult(void)
{
	// POINT�̏���
	// �G�b�W����
	float upperBoundary = SOME_UPPER_VALUE_MENU; // ��G�b�W
	float lowerBoundary = SOME_LOWER_VALUE_MENU; // ���G�b�W

	//�|�C���g�̈ړ�
	if (GetKeyboardTrigger(DIK_DOWN))
	{
		g_Ponit.pos.y += g_Ponit.speed;
	}
	else if (GetKeyboardTrigger(DIK_UP))
	{
		g_Ponit.pos.y -= g_Ponit.speed;
	}

	// �G�b�W�O�`�F�b�N
	if (g_Ponit.pos.y > SOME_LOWER_VALUE_MENU)
	{
		g_Ponit.pos.y = SOME_UPPER_VALUE_MENU;
	}
	else if (g_Ponit.pos.y < SOME_UPPER_VALUE_MENU)
	{
		g_Ponit.pos.y = SOME_LOWER_VALUE_MENU;
	}

	//���Ɋm�F����
	if (g_Ponit.pos.y == SOME_UPPER_VALUE_MENU)
	{
		if (GetKeyboardTrigger(DIK_RETURN))
		{
			//�Q�[���̑���
			SetFade(FADE_OUT, MODE_TITLE);
		}
	}

	else if (g_Ponit.pos.y == SOME_LOWER_VALUE_MENU)
	{
		if (GetKeyboardTrigger(DIK_RETURN))
		{
			//�Q�[���̏I��
			PostQuitMessage(0);
		}
	}



}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawResult(void)
{
	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// �}�g���N�X�ݒ�
	SetWorldViewProjection2D();

	// �v���~�e�B�u�g�|���W�ݒ�
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �}�e���A���ݒ�
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	// �w�i��`��
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

	float px = 0.0f;	                // �\���ʒuX
	float py = 0.0f;	                // �\���ʒuY

	float pw = SCREEN_WIDTH;		    // �\����
	float ph = SCREEN_HEIGHT;		    // �\������
	float tw = 1.0f;    		        // �e�N�X�`���̕�
	float th = 1.0f;		            // �e�N�X�`���̍���
	float tx = 0.0f;	        	    // �e�N�X�`���̍���X���W
	float ty = 0.0f;			        // �e�N�X�`���̍���Y���W

	// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
	SetSpriteLTColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	// �|���S���`��
	GetDeviceContext()->Draw(4, 0);




	//POINT��`��
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

	// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
	SetSpriteLTColor(g_VertexBuffer,
		g_Ponit.pos.x, g_Ponit.pos.y, g_Ponit.w, g_Ponit.h,
		0.0f, 0.0f, 1.0f, 1.0f,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	// �|���S���`��
	GetDeviceContext()->Draw(4, 0);



}




