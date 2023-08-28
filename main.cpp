﻿//=============================================================================
// メイン処理 [main.cpp]
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "debugproc.h"
#include "input.h"
#include"item.h"
#include "title.h"
#include "bg.h"
#include "player.h"
#include "enemy.h"
#include "bullet.h"
#include "score.h"
#include "result.h"
#include "sound.h"
#include "fade.h"
#include"pause.h"
#include "effect.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define CLASS_NAME		"GAME"			    // ウインドウのクラス名
#define WINDOW_NAME		"MagicForest"		// ウインドウのキャプション名
#define COUNT_DOWN_MIN     0        
//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow);
void Uninit(void);
void Update(void);
void Draw(void);


//*****************************************************************************
// グローバル変数:
//*****************************************************************************
long g_MouseX = 0;
long g_MouseY = 0;
int countdown;

int g_bIsPaused = START;
#ifdef _DEBUG
int		g_CountFPS;							// FPSカウンタ
char	g_DebugStr[2048] = WINDOW_NAME;		// デバッグ文字表示用

#endif

int	g_Mode = MODE_TITLE;					// 起動時の画面を設定

BOOL g_LoadGame = FALSE;					// NewGame


//=============================================================================
// メイン関数
//=============================================================================
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);	// 無くても良いけど、警告が出る（未使用宣言）
	UNREFERENCED_PARAMETER(lpCmdLine);		// 無くても良いけど、警告が出る（未使用宣言）


	countdown = COUNT_DOWN_MIN * 60 * 50 * 1.21; // 50为1秒(15MIN * 60S * 50 * 系数)

	// 時間計測用
	DWORD dwExecLastTime;
	DWORD dwFPSLastTime;
	DWORD dwCurrentTime;
	DWORD dwFrameCount;

	WNDCLASSEX	wcex = {
		sizeof(WNDCLASSEX),
		CS_CLASSDC,
		WndProc,
		0,
		0,
		hInstance,
		NULL,
		LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		NULL,
		CLASS_NAME,
		NULL
	};
	HWND		hWnd;
	MSG			msg;
	
	// ウィンドウクラスの登録
	RegisterClassEx(&wcex);

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// ウィンドウの作成
	hWnd = CreateWindow(CLASS_NAME,
		WINDOW_NAME,
		WS_POPUP,
		CW_USEDEFAULT,									// ウィンドウの左座標
		CW_USEDEFAULT,								    // ウィンドウの上座標
		screenWidth,									// ウィンドウ横幅
		screenHeight,	                                // ウィンドウ縦幅
		NULL,
		NULL,
		hInstance,
		NULL);

	// ウィンドウモードかフルスクリーンモードかの処理
	BOOL mode = TRUE;


	// 初期化処理(ウィンドウを作成してから行う)
	if(FAILED(Init(hInstance, hWnd, mode)))
	{
		return -1;
	}

	// フレームカウント初期化
	timeBeginPeriod(1);	// 分解能を設定
	dwExecLastTime = dwFPSLastTime = timeGetTime();	// システム時刻をミリ秒単位で取得
	dwCurrentTime = dwFrameCount = 0;

	// ウインドウの表示(初期化処理の後に呼ばないと駄目)
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	
	// メッセージループ
	while(1)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{// PostQuitMessage()が呼ばれたらループ終了
				break;
			}
			else
			{
				// メッセージの翻訳と送出
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
        }
		else
		{
			dwCurrentTime = timeGetTime();

			if ((dwCurrentTime - dwFPSLastTime) >= 1000)	// 1秒ごとに実行
			{
#ifdef _DEBUG
				g_CountFPS = dwFrameCount;
#endif
				dwFPSLastTime = dwCurrentTime;				// FPSを測定した時刻を保存
				dwFrameCount = 0;							// カウントをクリア
			}

			if ((dwCurrentTime - dwExecLastTime) >= (1000 / 60))	// 1/60秒ごとに実行
			{
				dwExecLastTime = dwCurrentTime;	// 処理した時刻を保存

#ifdef _DEBUG	// デバッグ版の時だけFPSを表示する
				wsprintf(g_DebugStr, WINDOW_NAME);
				wsprintf(&g_DebugStr[strlen(g_DebugStr)], " FPS:%d", g_CountFPS);
#endif

			if (countdown > 0)
			{
                countdown--; 
#ifdef _DEBUG
                wsprintf(&g_DebugStr[strlen(g_DebugStr)], " Countdown:%d", countdown);
                SetWindowText(hWnd, g_DebugStr);
#endif
            } 
			else
			{
               
            }
				Update();			// 更新処理
				Draw();				// 描画処理



#ifdef _DEBUG	// デバッグ版の時だけ表示する
				wsprintf(&g_DebugStr[strlen(g_DebugStr)], " MX:%d MY:%d", GetMousePosX(), GetMousePosY());
				SetWindowText(hWnd, g_DebugStr);
#endif

				dwFrameCount++;
			}
		}
	}

	timeEndPeriod(1);				// 分解能を戻す

	// ウィンドウクラスの登録を解除
	UnregisterClass(CLASS_NAME, wcex.hInstance);

	// 終了処理
	Uninit();

	return (int)msg.wParam;
}

//=============================================================================
// プロシージャ
//=============================================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		switch(wParam)
		{

		case VK_ESCAPE: 
			if (g_bIsPaused == MENU)
			{
				g_bIsPaused = START;
			}
			if (g_bIsPaused == START)
			{
				g_bIsPaused = MENU;
			}
			break;
		}
		break;

	case WM_MOUSEMOVE:
		g_MouseX = LOWORD(lParam);
		g_MouseY = HIWORD(lParam);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow)
{
	// 描画の初期化
	InitRenderer(hInstance, hWnd, bWindow);

	// カメラの初期化
	InitCamera();

	// ライトを有効化
	SetLightEnable(FALSE);

	// 背面ポリゴンをカリング
	SetCullingMode(CULL_MODE_BACK);

	// 入力処理の初期化
	InitInput(hInstance, hWnd);

	// サウンド処理の初期化
	InitSound(hWnd);

	// フェード処理の初期化
	InitFade();


	// 最初のモードをセット
	SetMode(g_Mode);

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void Uninit(void)
{
	// 終了のモードをセット
	SetMode(MODE_MAX);

	// フェードの終了処理
	UninitFade();

	// サウンドの終了処理
	UninitSound();

	// 入力の終了処理
	UninitInput();

	// カメラの終了処理
	UninitCamera();

	// レンダラーの終了処理
	UninitRenderer();
}

//=============================================================================
// 更新処理
//=============================================================================
void Update(void)
{
	if (g_bIsPaused == LEVELUP || g_bIsPaused == MENU)
	{
		if (g_bIsPaused == LEVELUP)
		{
			UpdatePause(LEVELUP);
		}

		if (g_bIsPaused == MENU)
		{
			UpdatePause(MENU);
		}


		// 入力の更新処理
		UpdateInput();

		return;
	}
		
	UpdateInput();

	// カメラ更新
	UpdateCamera();

	// モードによって処理を分ける
	switch (g_Mode)
	{
	case MODE_TITLE:		// タイトル画面の更新
		UpdateTitle();
		break;

	case MODE_GAME:			// ゲーム画面の更新
		UpdateBG();
		UpdatePlayer();
		UpdateEnemy();
		UpdateItem();
		UpdateBullet();
		UpdateEffect();
		UpdateScore();
		break;

	case MODE_RESULT:		// リザルト画面の更新
		UpdateResult();
		break;
	}

	UpdateFade();			// フェードの更新処理
}

//=============================================================================
// 描画処理
//=============================================================================
void Draw(void)
{
	// バックバッファクリア
	Clear();

	SetCamera();

	// 2Dの物を描画する処理
	SetViewPort(TYPE_FULL_SCREEN);

	// Z比較なし
	SetDepthEnable(FALSE);

	// ライティングを無効
	SetLightEnable(FALSE);


	// モードによって処理を分ける
	switch (g_Mode)
	{
	case MODE_TITLE:		// タイトル画面の描画
		DrawTitle();
		break;

	case MODE_GAME:			// ゲーム画面の描画
		DrawBG();
		DrawBullet();
		DrawItem();		// 重なる順番を意識してね
		DrawEnemy();
		DrawPlayer();
		DrawEffect();
		DrawScore();
		break;

	case MODE_RESULT:		// リザルト画面の描画
		DrawResult();
		break;
	}


	DrawFade();				// フェード画面の描画


	if (g_bIsPaused == LEVELUP)
	{
		DrawPause(LEVELUP);
	}

	if (g_bIsPaused == MENU)
	{
		DrawPause(MENU);
	}



#ifdef _DEBUG
	// デバッグ表示
	DrawDebugProc();
#endif

	// バックバッファ、フロントバッファ入れ替え
	Present();
}


long GetMousePosX(void)
{
	return g_MouseX;
}


long GetMousePosY(void)
{
	return g_MouseY;
}


#ifdef _DEBUG
char* GetDebugStr(void)
{
	return g_DebugStr;
}
#endif



//=============================================================================
// モードの設定
//=============================================================================
void SetMode(int mode)
{
	// モードを変える前に全部メモリを解放しちゃう
	StopSound();			// まず曲を止める

	// モードを変える前に全部メモリを解放しちゃう

	// タイトル画面の終了処理
	UninitTitle();

	// BGの終了処理
	UninitBG();

	// プレイヤーの終了処理
	UninitPlayer();

	// エネミーの終了処理
	UninitEnemy();

	// バレットの終了処理
	UninitBullet();

	// スコアの終了処理
	UninitScore();

	// リザルトの終了処理
	UninitResult();

	// エフェクトの終了処理
	UninitEffect();

	//
	UninitItem();

	//
	UninitPause();

	g_Mode = mode;	// 次のモードをセットしている

	switch (g_Mode)
	{
	case MODE_TITLE:
		// タイトル画面の初期化
		InitTitle();
		PlaySound(SOUND_LABEL_BGM_maou);
		break;

	case MODE_GAME:
		// ゲーム画面の初期化
		InitBG();
		InitPlayer();
		InitEnemy();
		InitItem();
		InitBullet();
		InitEffect();
		InitScore();
		InitPause();
		PlaySound(SOUND_LABEL_BGM_sample001);
		break;

	case MODE_RESULT:
		InitResult();
		PlaySound(SOUND_LABEL_BGM_sample002);
		break;

	case MODE_MAX:
		break;
	}
}

//=============================================================================
// モードの取得
//=============================================================================
int GetMode(void)
{
	return g_Mode;
}


//=============================================================================
// ニューゲームかロードゲームかをセットする
//=============================================================================
void SetLoadGame(BOOL flg)
{
	g_LoadGame = flg;
}

int* GetCountDown(void)
{
	return &countdown;
}

void Switch(int MENU_LEVELUP_START_GUIDE)
{
	g_bIsPaused = MENU_LEVELUP_START_GUIDE;
}
