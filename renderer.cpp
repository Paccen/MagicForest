//=============================================================================
// レンダリング処理 [renderer.cpp]
//=============================================================================
#include "main.h"
#include "renderer.h"

//デバッグ用画面テキスト出力を有効にする
#define DEBUG_DISP_TEXTOUT
//シェーダーデバッグ設定を有効にする
//#define DEBUG_SHADER


//*********************************************************
// 構造体
//*********************************************************

// マテリアル用定数バッファ構造体
struct MATERIAL_CBUFFER //用于在图形渲染中存储材质（Material）相关的常量数据
{
	XMFLOAT4	Ambient;//存储环境光（Ambient Light）的颜色,环境光是场景中的全局光照，影响物体整体的亮度
	XMFLOAT4	Diffuse;//存储漫反射光（Diffuse Light）的颜色,漫反射光是从光源处发出，照射在物体表面上的散射光，决定了物体的基本颜色
	XMFLOAT4	Specular;//存储镜面光（Specular Light）的颜色,镜面光是高光部分的光照，决定了物体表面反射光的亮度和颜色
	XMFLOAT4	Emission;//存储自发光（Emission）的颜色,自发光是物体表面自身发出的光，类似于物体本身具有发光材质的效果
	float		Shininess;//存储材质的光滑度（Shininess）值,光滑度值越高，高光点越小而且更集中，反之则分散
	int			noTexSampling;//存储是否进行纹理采样的标志,
	float		Dummy[2];				// 16byte境界用   用于对齐结构体大小到16字节的边界，以满足硬件要求
};

// ライト用フラグ構造体
struct LIGHTFLAGS //用于在图形渲染中存储与光照相关的标志位和信息.
{
	int			Type;		//ライトタイプ（enum LIGHT_TYPE）//存储光照类型的标识,光照类型指定了光源的种类，如平行光、点光源、聚光灯等
	int         OnOff;		//ライトのオンorオフスイッチ//存储光照的开关状态，用于控制光照的启用或禁用
	int			Dummy[2];  //用于对齐结构体大小，确保结构体的大小是4的倍数
};

// ライト用定数バッファ構造体
struct LIGHT_CBUFFER //用于在图形渲染中存储与光照相关的常量信息
{
	XMFLOAT4	Direction[LIGHT_MAX];	// ライトの方向//表示一个光源的方向,指定了允许的最大光源数量
	XMFLOAT4	Position[LIGHT_MAX];	// ライトの位置//表示一个光源的位置,
	XMFLOAT4	Diffuse[LIGHT_MAX];		// 拡散光の色//表示一个光源的拡散光颜色。
	XMFLOAT4	Ambient[LIGHT_MAX];		// 環境光の色//表示一个光源的环境光颜色。
	XMFLOAT4	Attenuation[LIGHT_MAX];	// 減衰率//光源的衰减率参数。
	LIGHTFLAGS	Flags[LIGHT_MAX];		// ライト種別//包含光源类型和开关状态等信息。
	int			Enable;					// ライティング有効・無効フラグ//表示光照是否启用的标志
	int			Dummy[3];				// 16byte境界用//用于对齐结构体大小，确保结构体的大小是16的倍数
};

// フォグ用定数バッファ構造体
struct FOG_CBUFFER//用于在图形渲染中存储与雾（Fog）效果相关的常量信息
{
	XMFLOAT4	Fog;					// フォグ量//存储雾量的四维向量，可能包含雾的密度、距离等信息
	XMFLOAT4	FogColor;				// フォグの色//表示雾的颜色
	int			Enable;					// フォグ有効・無効フラグ//表示雾是否启用的标志
	float		Dummy[3];				// 16byte境界用//用于对齐结构体大小，确保结构体的大小是16的倍数
};

// 縁取り用バッファ
struct FUCHI//图形渲染中存储与描边效果相关的数据
{
	int			fuchi;//描边参数或标志
	int			fill[3];//一个包含3个整数的数组，可能用于占位或对齐结构体大小
};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
static void SetLightBuffer(void);


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static D3D_FEATURE_LEVEL       g_FeatureLevel = D3D_FEATURE_LEVEL_11_0;

static ID3D11Device*           g_D3DDevice = NULL;//Device对象用于创建和管理图形硬件资源以及控制渲染流水线，以及提供硬件抽象和优化接口
static ID3D11DeviceContext*    g_ImmediateContext = NULL; //1DeviceContext对象负责实际的绘制和渲染操作，执行渲染命令，配置状态和着色器等
static IDXGISwapChain*         g_SwapChain = NULL;
static ID3D11RenderTargetView* g_RenderTargetView = NULL;
static ID3D11DepthStencilView* g_DepthStencilView = NULL;



static ID3D11VertexShader*		g_VertexShader = NULL;
static ID3D11PixelShader*		g_PixelShader = NULL;
static ID3D11InputLayout*		g_VertexLayout = NULL;
static ID3D11Buffer*			g_WorldBuffer = NULL;
static ID3D11Buffer*			g_ViewBuffer = NULL;
static ID3D11Buffer*			g_ProjectionBuffer = NULL;
static ID3D11Buffer*			g_MaterialBuffer = NULL;
static ID3D11Buffer*			g_LightBuffer = NULL;
static ID3D11Buffer*			g_FogBuffer = NULL;
static ID3D11Buffer*			g_FuchiBuffer = NULL;
static ID3D11Buffer*			g_CameraBuffer = NULL;

static ID3D11DepthStencilState* g_DepthStateEnable;
static ID3D11DepthStencilState* g_DepthStateDisable;

static ID3D11BlendState*		g_BlendStateNone;
static ID3D11BlendState*		g_BlendStateAlphaBlend;
static ID3D11BlendState*		g_BlendStateAdd;
static ID3D11BlendState*		g_BlendStateSubtract;
static BLEND_MODE				g_BlendStateParam;


static ID3D11RasterizerState*	g_RasterStateCullOff;
static ID3D11RasterizerState*	g_RasterStateCullCW;
static ID3D11RasterizerState*	g_RasterStateCullCCW;


static MATERIAL_CBUFFER	g_Material;
static LIGHT_CBUFFER	g_Light;
static FOG_CBUFFER		g_Fog;

static FUCHI			g_Fuchi;


ID3D11Device* GetDevice( void )//当你调用 GetDevice() 函数时，它会返回当前已创建的 Direct3D 11 Device对象，可以用来进行各种图形渲染操作。
{
	return g_D3DDevice;
}


ID3D11DeviceContext* GetDeviceContext( void )//它会返回当前的 Direct3D 11 DeviceContext对象，你可以用来提交渲染命令、设置渲染状态等操作。
{
	return g_ImmediateContext;
}


void SetDepthEnable( BOOL Enable )//该函数用于设置深度测试(用于决定在绘制像素时是否应该考虑深度缓冲（Z 缓冲）来判断像素的可见性。)
{
	if( Enable )// OMSetDepthStencilState 方法将深度/模板状态设置为 g_DepthStateEnable。这意味着在接下来的渲染操作中将启用或关闭深度测试
		g_ImmediateContext->OMSetDepthStencilState( g_DepthStateEnable, NULL );
	else
		g_ImmediateContext->OMSetDepthStencilState( g_DepthStateDisable, NULL );

}


void SetBlendState(BLEND_MODE bm)//用于设置混合（Blend）状态（混合是图形渲染中的另一个重要技术，用于在将新像素颜色与背景颜色组合时控制其透明度和效果。）
{                                //通过设置不同的混合模式，可以实现各种渲染效果，例如透明度、颜色叠加等。
	
	g_BlendStateParam = bm;//static BLEND_MODE  g_BlendStateParam；用于指示要设置的混合模式

	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };//用于设置混合因子。混合因子用于调整源颜色和目标颜色的混合比例。在这里，所有分量初始值都为0，意味着没有混合因子的调整

	switch (g_BlendStateParam)// 根据传入的混合模式参数选择不同的混合模式
	{
	case BLEND_MODE_NONE:// 设置混合状态为无混合模式，直接使用新像素颜色覆盖背景
		g_ImmediateContext->OMSetBlendState(g_BlendStateNone, blendFactor, 0xffffffff);
		break;
	case BLEND_MODE_ALPHABLEND://设置混合状态为Alpha混合模式，用于透明度混合
		g_ImmediateContext->OMSetBlendState(g_BlendStateAlphaBlend, blendFactor, 0xffffffff);
		break;
	case BLEND_MODE_ADD: // 设置混合状态为加法混合模式，用于颜色叠加效果
		g_ImmediateContext->OMSetBlendState(g_BlendStateAdd, blendFactor, 0xffffffff);
		break;
	case BLEND_MODE_SUBTRACT:// 设置混合状态为减法混合模式，用于颜色减法效果
		g_ImmediateContext->OMSetBlendState(g_BlendStateSubtract, blendFactor, 0xffffffff);
		break;
	}
}

void SetCullingMode(CULL_MODE cm)//设置三角形剔除，三角形剔除是图形渲染中的一个技术，用于排除不可见的三角形，从而提高渲染性能
{
	switch (cm)//用于指示要设置的剔除模式
	{
	case CULL_MODE_NONE://表示不进行三角形剔除
		g_ImmediateContext->RSSetState(g_RasterStateCullOff);
		break;
	case CULL_MODE_FRONT://这个对象表示剔除正面朝向的三角形。
		g_ImmediateContext->RSSetState(g_RasterStateCullCW);
		break;
	case CULL_MODE_BACK://表示剔除背面朝向的三角形。
		g_ImmediateContext->RSSetState(g_RasterStateCullCCW);
		break;
	}
}

void SetAlphaTestEnable(BOOL flag)  //设置 alpha 测试和混合状态的函数
//Alpha 测试是一种用于确定像素是否应该被绘制的技术，而混合是用于控制像素颜色的技术，以实现不同的透明度和混合效果
{
	D3D11_BLEND_DESC blendDesc;//这个结构体用于描述混合状态的设置。它包含了一组成员变量，用于指定混合状态的各个参数
	ZeroMemory(&blendDesc, sizeof(blendDesc));//用于初始化内存，以确保结构体中的各个成员变量都被正确地初始化为零。

	if (flag)//// 根据 flag 设置是否启用 Alpha 覆盖
		blendDesc.AlphaToCoverageEnable = TRUE;
	else
		blendDesc.AlphaToCoverageEnable = FALSE;

	// 关闭独立混合（Independent Blend）模式
	blendDesc.IndependentBlendEnable = FALSE;

	// 启用混合（Blend）模式
	blendDesc.RenderTarget[0].BlendEnable = TRUE;

	//  // 根据 g_BlendStateParam 选择不同的混合模式
	switch (g_BlendStateParam)
	{
	case BLEND_MODE_NONE:
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		break;
	case BLEND_MODE_ALPHABLEND:
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		break;
	case BLEND_MODE_ADD:
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		break;
	case BLEND_MODE_SUBTRACT:
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		break;
	}
	// 创建混合状态对象
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ID3D11BlendState* blendState = NULL;
	g_D3DDevice->CreateBlendState(&blendDesc, &blendState);

	// 设置渲染目标的混合状态
	g_ImmediateContext->OMSetBlendState(blendState, blendFactor, 0xffffffff);

	if (blendState != NULL)
		blendState->Release();
}

//设置二维渲染的世界（World）、视图（View）和投影（Projection）矩阵的函数，用于定义渲染的几何变换和相机视角。
void SetWorldViewProjection2D( void )
{

	// 设置世界矩阵为单位矩阵，并将其传递给 g_WorldBuffer
	XMMATRIX world;
	world = XMMatrixTranspose(XMMatrixIdentity());
	GetDeviceContext()->UpdateSubresource(g_WorldBuffer, 0, NULL, &world, 0, 0);

	// 设置视图矩阵为单位矩阵，并将其传递给 g_ViewBuffer
	XMMATRIX view;
	view = XMMatrixTranspose(XMMatrixIdentity());
	GetDeviceContext()->UpdateSubresource(g_ViewBuffer, 0, NULL, &view, 0, 0);

	// 设置正交投影矩阵，并将其传递给 g_ProjectionBuffer
	//该矩阵将3D空间映射到2D屏幕空间。
	XMMATRIX worldViewProjection;
	worldViewProjection = XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f);// 创建一个正交投影矩阵
	worldViewProjection = XMMatrixTranspose(worldViewProjection);//转置正交投影矩阵
	GetDeviceContext()->UpdateSubresource(g_ProjectionBuffer, 0, NULL, &worldViewProjection, 0, 0);
}

//设置世界矩阵的函数。用于定义物体在世界坐标系中的位置、旋转和缩放
void SetWorldMatrix( XMMATRIX *WorldMatrix )
{
	XMMATRIX world;//声明一个名为 world 的 XMMATRIX 类型的变量，用于存储最终的世界矩阵。
	world = *WorldMatrix;//首先将传入的世界矩阵数据复制给一个临时变量 world
	world = XMMatrixTranspose(world);//XMMatrixTranspose 函数将 world 矩阵转置，并将转置后的矩阵数据更新到相应的缓冲区中，以备在渲染时使用

	//这是一个用于传递世界矩阵的缓冲区。这个函数调用确保在渲染时使用了正确的世界矩阵。
	//UpdateSubresource用于将数据从内存中的源位置复制到显存中的目标位置，通常用于更新常量缓冲区
	GetDeviceContext()->UpdateSubresource(g_WorldBuffer, 0, NULL, &world, 0, 0);
}

void SetViewMatrix(XMMATRIX *ViewMatrix )
{
	XMMATRIX view;
	view = *ViewMatrix;
	view = XMMatrixTranspose(view);

	GetDeviceContext()->UpdateSubresource(g_ViewBuffer, 0, NULL, &view, 0, 0);
}

void SetProjectionMatrix( XMMATRIX *ProjectionMatrix )
{
	XMMATRIX projection;
	projection = *ProjectionMatrix;
	projection = XMMatrixTranspose(projection);

	GetDeviceContext()->UpdateSubresource(g_ProjectionBuffer, 0, NULL, &projection, 0, 0);
}


//用于设置材质，材质是描述物体外观属性的数据，如漫射颜色、环境颜色、高光颜色、发光颜色等
void SetMaterial( MATERIAL material )
{
	// 将传入的材质属性赋值给全局变量 g_Material
	g_Material.Diffuse = material.Diffuse;//将传入的材质的漫射颜色赋值给全局变量 
	g_Material.Ambient = material.Ambient;//将传入的材质的环境颜色赋值给全局变量 
	g_Material.Specular = material.Specular;//将传入的材质的高光颜色赋值给全局变量
	g_Material.Emission = material.Emission;//将传入的材质的发光颜色赋值给全局变量
	g_Material.Shininess = material.Shininess;//将传入的材质的高光强度赋值给全局变量
	g_Material.noTexSampling = material.noTexSampling;//将传入的材质的纹理采样标志赋值给全局变量

	// 将更新后的 g_Material 数据更新到材质缓冲区 g_MaterialBuffer
	GetDeviceContext()->UpdateSubresource( g_MaterialBuffer, 0, NULL, &g_Material, 0, 0 );
}


//将光照信息更新到光照缓冲区的函数
void SetLightBuffer(void)
{
	GetDeviceContext()->UpdateSubresource(g_LightBuffer, 0, NULL, &g_Light, 0, 0);
}

//设置光照是否启用（开关）并将其更新到光照缓冲区的函数
void SetLightEnable(BOOL flag)
{
	// フラグを更新する
	g_Light.Enable = flag;//// 更新光照启用标志

	SetLightBuffer();// // 将更新后的光照信息更新到光照缓冲区
}


//设置光源信息并将其更新到光照缓冲区的函数。在图形渲染中，光源是影响场景光照效果的重要因素，包括光源位置、方向、颜色、强度等
void SetLight(int index, LIGHT* pLight)
{
	// 将传入的光源信息赋值给全局变量 g_Light 的对应索引处
	g_Light.Position[index] = XMFLOAT4(pLight->Position.x, pLight->Position.y, pLight->Position.z, 0.0f);//将传入的光源的位置信息赋值给全局变量 g_Light 中对应索引位置的 Position 成员，并将 z 分量设置为 0
	g_Light.Direction[index] = XMFLOAT4(pLight->Direction.x, pLight->Direction.y, pLight->Direction.z, 0.0f);//将传入的光源的方向信息赋值给全局变量 g_Light 中对应索引位置的 Direction 成员，并将 z 分量设置为 0。
	g_Light.Diffuse[index] = pLight->Diffuse;//将传入的光源的漫射颜色赋值给全局变量
	g_Light.Ambient[index] = pLight->Ambient;//将传入的光源的环境颜色赋值给全局变量
	g_Light.Flags[index].Type = pLight->Type;//将传入的光源的类型赋值给全局变量 g_Light
	g_Light.Flags[index].OnOff = pLight->Enable;//将传入的光源的启用标志赋值给全局变量
	g_Light.Attenuation[index].x = pLight->Attenuation;//将传入的光源的衰减因子赋值给全局变量


	// 将更新后的光照信息更新到光照缓冲区
	SetLightBuffer();
}


//这段代码是用于将雾（Fog）信息更新到雾缓冲区的函数
void SetFogBuffer(void)
{
	GetDeviceContext()->UpdateSubresource(g_FogBuffer, 0, NULL, &g_Fog, 0, 0);
}

//这段代码是用于设置雾是否启用（开关）并将其更新到雾缓冲区的函数
void SetFogEnable(BOOL flag)
{
	// フラグを更新する
	g_Fog.Enable = flag;

	SetFogBuffer();
}

//用于设置雾（Fog）的相关属性并将其更新到雾缓冲区的函数
void SetFog(FOG* pFog)
{
	g_Fog.Fog.x = pFog->FogStart;
	g_Fog.Fog.y = pFog->FogEnd;
	g_Fog.FogColor = pFog->FogColor;

	SetFogBuffer();
}

//设置腐蚀（Fuchi）属性并将其更新到腐蚀缓冲区的函数
void SetFuchi(int flag)
{
	g_Fuchi.fuchi = flag;
	GetDeviceContext()->UpdateSubresource(g_FuchiBuffer, 0, NULL, &g_Fuchi, 0, 0);
}

//设置着色器中的摄像机位置信息并将其更新到摄像机缓冲区的函数
//在图形渲染中，摄像机的位置是决定视点和视角的关键因素，它影响着物体的可见性和相对位置
void SetShaderCamera(XMFLOAT3 pos)
{
	// 创建一个临时的 XMFLOAT4 变量，并将传入的位置信息转换为四维向量
	XMFLOAT4 tmp = XMFLOAT4( pos.x, pos.y, pos.z, 0.0f );

	// 将更新后的摄像机位置信息更新到摄像机缓冲区 g_CameraBuffer
	GetDeviceContext()->UpdateSubresource(g_CameraBuffer, 0, NULL, &tmp, 0, 0);
}



//=============================================================================
// 初期化処理
//=============================================================================

//初始化渲染器的函数 InitRenderer，用于创建设备、交换链、上下文以及各种渲染所需的资源和状态
HRESULT InitRenderer(HINSTANCE hInstance, HWND hWnd, BOOL bWindow)
{
	HRESULT hr = S_OK;

	// デバイス、スワップチェーン、コンテキスト生成
	//初始化渲染器时配置 DirectX 设备，交换链和呈现上下文
	DWORD deviceFlags = 0;//用于设备创建的标志位。在这里初始化为0，表示不使用任何特殊标志。
	DXGI_SWAP_CHAIN_DESC sd;//这是用于配置交换链的结构体
	ZeroMemory( &sd, sizeof( sd ) );//函数将 sd 结构体的内存初始化为零，以确保所有字段都具有适当的默认值。
	sd.BufferCount = 1;//设置交换链的后台缓冲数量为1。
	sd.BufferDesc.Width = SCREEN_WIDTH;// 设置后台缓冲的宽度和高度，通常是屏幕的分辨率。
	sd.BufferDesc.Height = SCREEN_HEIGHT;// 设置后台缓冲的宽度和高度，通常是屏幕的分辨率。
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;// 设置后台缓冲的像素格式，这里使用的是 32 位的 RGBA 格式。
	sd.BufferDesc.RefreshRate.Numerator = 60;//设置屏幕刷新率，这里是 60Hz。
	sd.BufferDesc.RefreshRate.Denominator = 1;//设置屏幕刷新率，这里是 1秒。
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//设置后台缓冲的用途，这里表示后台缓冲将用于呈现图像。
	sd.OutputWindow = hWnd;//设置交换链的输出窗口句柄，这里使用传入的 hWnd 参数。
	sd.SampleDesc.Count = 1;//设置抗锯齿的样本数量为1，质量为0，表示不使用抗锯齿。
	sd.SampleDesc.Quality = 0;//设置抗锯齿的样本数量为1，质量为0，表示不使用抗锯齿。
	sd.Windowed = bWindow;//根据传入的 bWindow 参数，设置窗口模式（TRUE）还是全屏模式（FALSE）。


	//デバッグ文字出力用設定
#if defined(_DEBUG) && defined(DEBUG_DISP_TEXTOUT)    //代码使用条件编译来检查是否定义了宏 _DEBUG 和 DEBUG_DISP_TEXTOUT
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;//如果满足条件，将交换链的像素格式sd.BufferDesc.Format设置为 DXGI_FORMAT_B8G8R8A8_UNORM，
	                                                  //这是一种常见的无符号整数像素格式，其中分别代表蓝色、绿色、红色和Alpha通道。
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE; //同时设置 sd.Flags 为 DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE，表示创建一个与GDI兼容的交换链，这通常用于在渲染中显示文本或图像。
	deviceFlags = D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT;//这将启用D3D11设备的调试模式，并支持BGRA像素格式，以适应前面设置的像素格式。
#endif


	//！！！！函数创建了一个D3D11设备、交换链和设备上下文！！！！
	hr = D3D11CreateDeviceAndSwapChain( NULL,                      //表示没有已存在的设备。
										D3D_DRIVER_TYPE_HARDWARE,  //指定使用硬件加速的驱动类型
										NULL,                      //表示没有指定任何软件层级
										deviceFlags,               //之前已经设置的设备标志，用于指定创建设备的选项，包括调试模式和像素格式支持。
										NULL,                      //表示没有指定特性级别，将会使用最高级别。
										0,                         //表示没有指定多线程支持。
										D3D11_SDK_VERSION,         //表示使用的D3D SDK版本。
										&sd,                       //指向一个 DXGI_SWAP_CHAIN_DESC 结构体，其中包含交换链的描述。
										&g_SwapChain,              //用于返回创建的交换链对象
										&g_D3DDevice,              //用于返回创建的D3D设备对象。
										&g_FeatureLevel,           //用于返回实际支持的D3D特性级别。
										&g_ImmediateContext );     //用于返回创建的设备上下文对象。
	if( FAILED( hr ) )   // 调用失败（返回的 hr 不是 S_OK），则函数返回 hr，表示初始化渲染器失败。
		return hr;

	Sleep(100);//函数让程序暂停执行 100 毫秒。这可能是为了确保设备和交换链完全创建，然后继续执行后续操作



	//デバッグ文字出力用設定
#if defined(_DEBUG) && defined(DEBUG_DISP_TEXTOUT)//在定义了 _DEBUG 和 DEBUG_DISP_TEXTOUT 宏的情况下
	//首先尝试重新调整交换链的缓冲区大小，以适应新的窗口尺寸，并启用 GDI 兼容模式。这可能是为了支持在调试时在窗口中绘制文本
	hr = g_SwapChain->ResizeBuffers(0, SCREEN_WIDTH, SCREEN_HEIGHT, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE); 
	if (FAILED(hr))//如果调整缓冲区大小失败（返回的 hr 不是 S_OK），则函数返回 hr，表示无法进行调整。
		return hr;
#endif

	// レンダーターゲットビュー生成、設定
	//获取交换链的后备缓冲区
	ID3D11Texture2D* pBackBuffer = NULL;

	//通过调用 g_SwapChain->GetBuffer 函数。这将返回一个 ID3D11Texture2D 接口的指针，该接口表示后备缓冲区
	g_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );

	//使用后备缓冲区创建一个渲染目标视图（RenderTargetView），通过调用 g_D3DDevice->CreateRenderTargetView 函数。
	g_D3DDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_RenderTargetView );

	//释放后备缓冲区的引用，以便后续操作不再使用它，通过调用 pBackBuffer->Release()。
	pBackBuffer->Release();

	//再次通过 Sleep(100) 函数暂停程序执行 100 毫秒。这可能是为了确保渲染目标视图完全创建，然后继续执行后续操作。
	Sleep(100);


	//ステンシル用テクスチャー作成
	//这段代码创建一个用于深度和模板测试的纹理，具有与交换链缓冲区相同的尺寸和抗锯齿设置。它将在后续的渲染过程中用作深度模板缓冲区。
	ID3D11Texture2D* depthTexture = NULL;//创建一个名为 depthTexture 的指向 ID3D11Texture2D 接口的指针，初始值为 NULL。
	D3D11_TEXTURE2D_DESC td;//创建一个名为 td 的 D3D11_TEXTURE2D_DESC 结构体，用于描述纹理的属性。
	ZeroMemory( &td, sizeof(td) );// 将 td 结构体的内存清零，以确保所有成员都被初始化。
	td.Width			= sd.BufferDesc.Width;//设置纹理的宽度为交换链缓冲区的宽度。
	td.Height			= sd.BufferDesc.Height;// 设置纹理的高度为交换链缓冲区的高度。
	td.MipLevels		= 1;//设置纹理的细化级别数量为 1，表示不使用多级纹理。
	td.ArraySize		= 1;//设置纹理的数组大小为 1，表示不使用纹理数组。
	td.Format			= DXGI_FORMAT_D24_UNORM_S8_UINT;//设置纹理的格式为深度和模板数据，使用 DXGI_FORMAT_D24_UNORM_S8_UINT 格式。
	td.SampleDesc		= sd.SampleDesc;//设置纹理的采样描述为与交换链缓冲区相同的采样描述，用于指定抗锯齿的采样方式。
	td.Usage			= D3D11_USAGE_DEFAULT;//设置纹理的使用方式为默认，表示它将用于 GPU 访问。
	td.BindFlags		= D3D11_BIND_DEPTH_STENCIL;//设置纹理的绑定标志为深度和模板缓冲区，表示纹理将用作深度和模板测试的缓冲区。
    td.CPUAccessFlags	= 0;//设置纹理的 CPU 访问标志为 0，表示 CPU 不需要访问纹理数据。
    td.MiscFlags		= 0;//设置纹理的其他标志为 

	//使用描述结构体 td 创建纹理对象，通过调用 g_D3DDevice->CreateTexture2D 函数。depthTexture 指针将指向创建的深度模板纹理。
	g_D3DDevice->CreateTexture2D( &td, NULL, &depthTexture );


	
	//ステンシルターゲット作成
	//这段代码用于创建深度模板视图，并将其绑定为渲染目标，以便在渲染过程中执行深度和模板测试
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;//创建一个名为 dsvd 的 D3D11_DEPTH_STENCIL_VIEW_DESC 结构体，用于描述深度模板视图的属性。
	ZeroMemory( &dsvd, sizeof(dsvd) );// 将 dsvd 结构体的内存清零，以确保所有成员都被初始化。
	dsvd.Format			= td.Format;//设置深度模板视图的格式为与之前创建的深度模板纹理相同的格式。
	dsvd.ViewDimension	= D3D11_DSV_DIMENSION_TEXTURE2D;//设置深度模板视图的维度为 D3D11_DSV_DIMENSION_TEXTURE2D，表示它是一个二维纹理视图。
	dsvd.Flags			= 0;//设置深度模板视图的标志为 0。
	//使用描述结构体 dsvd 创建深度模板视图对象，通过调用 g_D3DDevice->CreateDepthStencilView 函数。g_DepthStencilView 指针将指向创建的深度模板视图。
	g_D3DDevice->CreateDepthStencilView( depthTexture, &dsvd, &g_DepthStencilView );

	//将渲染目标设置为渲染目标视图（g_RenderTargetView）和深度模板视图（g_DepthStencilView）。这将在后续的渲染过程中使用这些视图进行渲染操作，包括深度和模板测试。
	g_ImmediateContext->OMSetRenderTargets( 1, &g_RenderTargetView, g_DepthStencilView );



	// ビューポート設定
	//目的是在渲染之前设置渲染视口，确定渲染的区域和深度范围。渲染视口在每次渲染帧时都需要进行设置，以确保正确的投影和视图转换
	D3D11_VIEWPORT vp;//创建一个名为 vp 的 D3D11_VIEWPORT 结构体，用于描述渲染视口的属性。
	vp.Width = (FLOAT)SCREEN_WIDTH;// 设置视口的宽度为屏幕宽度（在此处使用 SCREEN_WIDTH 变量）。
	vp.Height = (FLOAT)SCREEN_HEIGHT;//设置视口的高度为屏幕高度（在此处使用 SCREEN_HEIGHT 变量）。
	vp.MinDepth = 0.0f;//设置视口的最小深度值为 0.0，表示深度范围的最近处。
	vp.MaxDepth = 1.0f;//设置视口的最大深度值为 1.0，表示深度范围的最远处。
	vp.TopLeftX = 0;//设置视口左上角的 X 坐标为 0。
	vp.TopLeftY = 0;//设置视口左上角的 Y 坐标为 0。
	//使用渲染上下文的 RSSetViewports 函数将定义好的渲染视口应用于渲染管线。在这里，1 表示视口数量，&vp 是指向 vp 结构体的指针。
	g_ImmediateContext->RSSetViewports( 1, &vp );



	// ラスタライザステート作成
	//用于创建和配置光栅化器状态，光栅化器状态定义了如何在渲染管线中处理几何形状并将其映射到屏幕上。
	D3D11_RASTERIZER_DESC rd; //创建一个名为 rd 的 D3D11_RASTERIZER_DESC 结构体，用于描述光栅化器状态的属性。
	ZeroMemory( &rd, sizeof( rd ) );//使用 ZeroMemory 函数将 rd 结构体的内存清零，以确保没有未初始化的值。
	rd.FillMode = D3D11_FILL_SOLID;// 设置填充模式为实心填充，即通过填充多边形的内部来渲染它。
	rd.CullMode = D3D11_CULL_NONE; //设置剔除模式为无剔除，这将禁用剔除（Culling），即渲染所有多边形，而不考虑它们的朝向。
	rd.DepthClipEnable = TRUE; //启用深度剪裁，这将确保在渲染时只显示相机前方的物体。
	rd.MultisampleEnable = FALSE; //禁用多重采样，这将影响渲染图像的平滑度。
	g_D3DDevice->CreateRasterizerState( &rd, &g_RasterStateCullOff);//使用设备的 CreateRasterizerState 函数来根据 rd 结构体的描述创建光栅化器状态，并将其保存在名为 g_RasterStateCullOff 的变量中。

	//接下来，代码分别创建了两个额外的光栅化器状态 g_RasterStateCullCW 和 g_RasterStateCullCCW，它们分别启用了正面剔除（Clockwise Culling）和背面剔除（Counter-Clockwise Culling）。
	rd.CullMode = D3D11_CULL_FRONT;
	g_D3DDevice->CreateRasterizerState(&rd, &g_RasterStateCullCW);

	rd.CullMode = D3D11_CULL_BACK;
	g_D3DDevice->CreateRasterizerState(&rd, &g_RasterStateCullCCW);



	// カリングモード設定（CCW）
	//这段代码用于设置光栅化器的剔除模式，以控制渲染管线如何对多边形进行剔除
	SetCullingMode(CULL_MODE_BACK);//这个函数的作用是设置光栅化器的剔除模式为背面剔除



	// ブレンドステートの作成
	//用于创建并配置不同的混合状态（Blend State），以控制渲染目标（RenderTarget）上的像素混合操作。混合状态可以用于实现透明度、颜色叠加等效果
	D3D11_BLEND_DESC blendDesc;//用于描述 Direct3D 11 中混合状态的结构体blendDesc
	ZeroMemory( &blendDesc, sizeof( blendDesc ) );//使用 ZeroMemory 函数将其初始化为零

	//设置了一个名为 g_BlendStateAlphaBlend 的混合状态，其中渲染目标的颜色混合方式为标准的 Alpha 混合。这种混合方式通常用于绘制半透明物体，以正确处理透明度效果
	blendDesc.AlphaToCoverageEnable = FALSE;//关闭 Alpha To Coverage 功能，该功能在进行 Alpha 抗锯齿时使用。
	blendDesc.IndependentBlendEnable = FALSE;//关闭独立渲染目标的混合设置。
	blendDesc.RenderTarget[0].BlendEnable = TRUE;// 打开混合功能。
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;//源像素使用 Alpha 值作为混合因子。
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;//目标像素使用反转的 Alpha 值作为混合因子。
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;//使用加法操作进行混合。
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;//源像素的 Alpha 值使用 1。
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;//目标像素的 Alpha 值使用 0。
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;// 使用加法操作混合 Alpha 值。
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;//启用写入所有颜色通道
	g_D3DDevice->CreateBlendState( &blendDesc, &g_BlendStateAlphaBlend );//最后通过 g_D3DDevice->CreateBlendState 创建混合状态


	//这段代码设置了一个名为 g_BlendStateNone 的混合状态，其中渲染目标的颜色不进行混合，绘制的物体将直接覆盖到渲染目标上，不会受到之前已存在的颜色影响。
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	g_D3DDevice->CreateBlendState(&blendDesc, &g_BlendStateNone);

	//这段代码设置了一个名为 g_BlendStateAdd 的混合状态，用于执行颜色的加法混合。绘制时会执行颜色的加法混合，将绘制的颜色与目标颜色相加。这
	//可以用于实现一些特殊的视觉效果，比如光照的累加效果
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	g_D3DDevice->CreateBlendState(&blendDesc, &g_BlendStateAdd);

	//这段代码设置了一个名为 g_BlendStateSubtract 的混合状态，用于执行颜色的反向减法混合
	//绘制时会执行颜色的反向减法混合，将绘制的颜色从目标颜色中减去。这可以用于实现一些特殊的效果，比如实现颜色逐渐消失的动画效果。
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	g_D3DDevice->CreateBlendState(&blendDesc, &g_BlendStateSubtract);


	// アルファブレンド設定
	//设定为BLEND_MODE_ALPHABLEND混合模式，如上
	SetBlendState(BLEND_MODE_ALPHABLEND);




	// 深度ステンシルステート作成
	//用于创建深度和模板测试状态（Depth-Stencil State）的操作
	//深度和模板测试是在进行3D渲染时用于控制对象渲染顺序以及深度信息和模板信息的测试和写入。
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;// 是用于描述深度和模板测试状态的结构体类型
	ZeroMemory( &depthStencilDesc, sizeof( depthStencilDesc ) );

	depthStencilDesc.DepthEnable = TRUE;//指示是否启用深度测试。如果启用，将根据深度值对像素进行测试。
	depthStencilDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;//指示在通过深度测试后是否要写入深度缓冲区
	                                                            //D3D11_DEPTH_WRITE_MASK_ALL 表示所有像素都将写入深度缓冲区
	//指定用于深度测试的比较函数。在这里，D3D11_COMPARISON_LESS_EQUAL 表示只有当像素的深度值小于或等于当前深度缓冲区的值时，才会通过深度测试
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;//指示是否启用模板测试。在这里设置为 FALSE，表示不使用模板测试。

	//函数来创建一个深度测试状态对象（ID3D11DepthStencilState），使用 depthStencilDesc 结构体来配置深度测试的行为。
	g_D3DDevice->CreateDepthStencilState( &depthStencilDesc, &g_DepthStateEnable );//深度有効ステート

	//depthStencilDesc.DepthEnable = FALSE;


	depthStencilDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ZERO;
	g_D3DDevice->CreateDepthStencilState( &depthStencilDesc, &g_DepthStateDisable );//深度無効ステート//这个状态对象将禁用深度写入

	// 深度ステンシルステート設定
	SetDepthEnable(TRUE);



	// サンプラーステート設定
	//设置像素着色器中的采样器状态，以控制纹理采样的方式。以下是对每个参数的解释：
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory( &samplerDesc, sizeof( samplerDesc ) );

	//用于纹理采样过滤的方法。在这里，D3D11_FILTER_ANISOTROPIC 表示使用各向异性过滤，这是一种高质量的纹理过滤方法，可在纹理被缩小的情况下提供更好的细节保留。
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; //指定纹理坐标超出[0, 1] 范围时的处理方式。
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;//指定纹理坐标超出[0, 1] 范围时的处理方式。
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;//指定纹理坐标超出[0, 1] 范围时的处理方式。
	//D3D11_TEXTURE_ADDRESS_WRAP 表示超出范围时纹理坐标会被循环包裹。

	samplerDesc.MipLODBias = 0;//Mipmap 层级偏移值，用于控制使用哪个 Mipmap 层级进行采样。
	samplerDesc.MaxAnisotropy = 16;//各向异性过滤的最大各向异性值。值越大，过滤效果越好但性能可能会受到影响。
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;// 用于比较深度或模板值的比较函数，D3D11_COMPARISON_ALWAYS 表示始终通过比较
	samplerDesc.MinLOD = 0;//指定要采样的 Mipmap 层级范围。
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;//指定要采样的 Mipmap 层级范围。

	//用于创建采样器状态对象，并将该采样器状态对象绑定到像素着色器管道的采样器槽位上。
	ID3D11SamplerState* samplerState = NULL; //首先声明了一个指向 ID3D11SamplerState 对象的指针 samplerState，并将其初始化为 NULL
	// 使用设备对象 g_D3DDevice 的 CreateSamplerState 方法来创建采样器状态对象。将之前设置好的 samplerDesc 结构体传递给函数，该结构体描述了采样器状态的各种属性。
	//该函数会创建一个采样器状态对象并将其存储在 samplerState 指针所指向的内存位置。
	g_D3DDevice->CreateSamplerState( &samplerDesc, &samplerState );

	//将创建的采样器状态对象绑定到像素着色器管道的采样器槽位上。PSSetSamplers 函数的第一个参数是槽位索引（在这里是 0），第二个参数是要绑定的采样器状态对象数量（在这里是 1），
	//第三个参数是存储了采样器状态对象地址的数组。
	g_ImmediateContext->PSSetSamplers( 0, 1, &samplerState );




	// 頂点シェーダコンパイル・生成
	//用于编译和生成顶点着色器（Vertex Shader）
	ID3DBlob* pErrorBlob;//声明一个指向 ID3DBlob 对象的指针 pErrorBlob，用于接收编译错误信息（如果有）。
	ID3DBlob* pVSBlob = NULL;//声明一个指向 ID3DBlob 对象的指针 pVSBlob，用于接收编译后的顶点着色器代码。
	DWORD shFlag = D3DCOMPILE_ENABLE_STRICTNESS;//声明一个 DWORD 类型的变量 shFlag，并将其初始化为 D3DCOMPILE_ENABLE_STRICTNESS，这表示在编译过程中启用严格模式，将会检查代码中的错误。
	
//编译了一个顶点着色器并创建了一个输入布局
#if defined(_DEBUG) && defined(DEBUG_SHADER)
	shFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION; //设置了编译着色器时的编译标志（flags）
	//标志告诉编译器生成用于调试的附加信息，这将允许您在调试时查看源代码与汇编代码之间的映射，以及进行源代码级别的调试
	// 标志告诉编译器跳过对着色器代码的优化步骤，这对于在调试过程中更好地查看代码执行过程很有用。
#endif

	hr = D3DX11CompileFromFile( //从 HLSL 文件中编译顶点着色器代码
		"shader.hlsl",//: HLSL 文件的路径
		NULL,// 在此参数位置可以提供一个用于指定宏定义的 D3DInclude 接口，以便在编译期间包含其他文件
		NULL,//在此参数位置可以提供用于指定一组编译标志的 D3D_SHADER_MACRO 数组
		"VertexShaderPolygon",// HLSL 着色器代码中的入口函数名称，这是顶点着色器函数的名称。
		"vs_4_0",//指定编译目标版本，这里是 DirectX 11 的顶点着色器目标版本
		shFlag,//编译着色器时使用的编译标志，此处设置了调试和跳过优化
		0,//指定编译器效果。在此示例中未使用
		NULL,//在此参数位置可以提供一个 ID3DInclude 接口，用于自定义 HLSL 文件的包含。在此示例中未使用。
		&pVSBlob,//输出参数，用于接收编译后的着色器字节码。
		&pErrorBlob,//输出参数，用于接收编译错误信息（如果有错误的话）。
		NULL );//在此参数位置可以提供一个 ID3DX11ThreadPump 接口，用于多线程编译

	if( FAILED(hr) )
	{
		//这段代码检查编译顶点着色器是否失败，并在失败时弹出一个消息框显示编译错误信息。如果 hr 表示编译失败
		MessageBox( NULL , (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR );
	}
	//创建顶点着色器对象并将其编译后的字节码加载到对象中
	//pVSBlob->GetBufferPointer(): 这是顶点着色器的编译后字节码的指针。pVSBlob 是一个 ID3DBlob* 对象，通过 GetBufferPointer() 方法获取字节码指针
	//pVSBlob->GetBufferSize(): 这是顶点着色器的编译后字节码的大小。通过 GetBufferSize() 方法获取字节码大小。
	//这是一个用于与其他接口对象关联的参数，对于顶点着色器来说，不需要与其他对象关联，所以传入 NULL。
	//这是一个用于存储创建的顶点着色器对象的指针。函数会将创建的顶点着色器对象的地址存储在这个变量中。
	g_D3DDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_VertexShader );


	// 入力レイアウト生成
	//创建输入布局，定义了顶点数据在顶点着色器中的布局。
	D3D11_INPUT_ELEMENT_DESC layout[] =          //这是一个数组，其中包含了每个顶点属性的描述。每个元素都包含了属性名、索引、数据格式、输入槽、字节偏移等信息
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//1.属性名称为 "POSITION"，在 HLSL 顶点着色器中使用这个名称来引用这个属性。
		//2.属性的索引，通常为 0。
		//3.属性的数据格式，这里是 3 个浮点数，表示 x、y 和 z 坐标。
		//4.输入槽，通常为 0。
		//5.为字节偏移，D3D11_APPEND_ALIGNED_ELEMENT表示按字节对齐添加属性
		//6.输入数据类型.表示该属性每个顶点提供一次数据。
		//7.输入数据步长，通常为 0。
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	UINT numElements = ARRAYSIZE( layout );// 这里计算了输入布局数组中元素的数量，以便传递给 CreateInputLayout 函数。


	g_D3DDevice->CreateInputLayout(
		layout,//结构体数组的指针,每个结构体描述了一个顶点属性的布局信息，例如位置、法线、颜色等。
		numElements,//这是布局中顶点属性元素的数量，即数组 layout 中的元素个数
		pVSBlob->GetBufferPointer(),//这是指向顶点着色器代码字节码的指针通过调用 pVSBlob->GetBufferPointer() 获取。
		pVSBlob->GetBufferSize(),//这是顶点着色器代码字节码的大小，通过调用 pVSBlob->GetBufferSize() 获取。
		&g_VertexLayout );//: 这是用于接收创建的输入布局对象的指针。g_VertexLayout 是在代码中定义的变量，它将在创建输入布局后被赋值。

	pVSBlob->Release();//释放之前加载的顶点着色器字节码，因为已经不再需要了。





	// ピクセルシェーダコンパイル・生成
	//编译和生成像素着色器（Pixel Shader）
	ID3DBlob* pPSBlob = NULL;//这里声明了一个指向 ID3DBlob 接口的指针 pPSBlob，用于存储编译后的像素着色器代码字节码。
	//
	hr = D3DX11CompileFromFile        //函数来从文件中编译像素着色器代码
	( "shader.hlsl",//HLSL 文件的路径。
		NULL, //这是一个可选的 ID3DXInclude 接口指针，用于处理包含其他文件的指令。
		NULL,// 这是一个可选的 LPCSTR 类型的宏定义列表，可以在编译过程中定义预处理宏
		"PixelShaderPolygon",//这是 HLSL 文件中的像素着色器入口函数的名称。编译器将从这个函数开始分析和编译代码。
		"ps_4_0", // 这是像素着色器的目标编译版本。在这里，使用的是 DirectX 11 的像素着色器版本 4.0。
		shFlag,//这是编译标志，可能包括调试和优化选项。在之前的代码中，您可能已经根据需要设置了调试和优化选项。
		0,//这是编译标志的辅助标志，用于进一步控制编译过程。在这里，设置为 0 表示没有额外的辅助标志。
		NULL,//这是用于包含其他文件的 ID3DX11ThreadPump 接口指针，通常用于多线程编译。
		&pPSBlob,//这是用于存储编译后像素着色器代码字节码的 ID3DBlob 指针。
		&pErrorBlob,//这是用于存储编译错误信息的 ID3DBlob 指针。如果编译过程中出现错误，您可以通过此参数获取错误信息
		NULL //这是一个可选的函数指针，用于在编译时处理异步线程回调
	);

	//首先检查了像素着色器代码的编译是否失败，如果失败则弹出一个消息框显示错误信息
	if( FAILED(hr) )
	{
		MessageBox( NULL , (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR );
	}

	//创建了像素着色器对象，并使用之前编译的字节码来初始化着色器
	g_D3DDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShader );
	
	// 释放了之前分配的 ID3DBlob 对象
	pPSBlob->Release();






	// 定数バッファ生成
	//创建一个常量缓冲区（Constant Buffer），用于将常量数据传递给着色器。常量缓冲区用于存储在渲染过程中不会发生变化的数据，例如变换矩阵、材质属性等。
	D3D11_BUFFER_DESC hBufferDesc;

	hBufferDesc.ByteWidth = sizeof(XMMATRIX);//缓冲区的大小，以字节为单位。这里使用 sizeof(XMMATRIX) 表示缓冲区的大小与 XMMATRIX 类型的大小相同
	hBufferDesc.Usage = D3D11_USAGE_DEFAULT;//缓冲区的用途，这里使用 D3D11_USAGE_DEFAULT 表示缓冲区将被用于默认的渲染操作。
	hBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;//缓冲区的绑定标志，这里使用 D3D11_BIND_CONSTANT_BUFFER 表示缓冲区将被绑定为常量缓冲区。
	hBufferDesc.CPUAccessFlags = 0;//控制 CPU 访问缓冲区的方式，这里设置为 0 表示 CPU 无法直接访问缓冲区。
	hBufferDesc.MiscFlags = 0;// 其他标志，这里设置为 0。
	hBufferDesc.StructureByteStride = sizeof(float);//缓冲区中每个元素的大小（以字节为单位）。这里设置为 sizeof(float)。



	//ワールドマトリクス
	//创建一个用于存储世界变换矩阵的常量缓冲区，并将该缓冲区绑定到顶点着色器和像素着色器中，以便在渲染过程中使用。
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_WorldBuffer);//创建一个常量缓冲区对象，用于存储世界变换矩阵。
																 //&hBufferDesc 是用于描述缓冲区属性的 D3D11_BUFFER_DESC 结构体。
																//NULL 参数表示不需要初始化数据。
																//&g_WorldBuffer 是一个存储缓冲区指针的变量，将在创建成功后指向新创建的常量缓冲区对象。
	//这行代码将刚刚创建的世界变换矩阵的常量缓冲区绑定到顶点着色器（Vertex Shader）中的指定槽位。
	g_ImmediateContext->VSSetConstantBuffers(0, 1, &g_WorldBuffer);
	//VSSetConstantBuffers 函数用于设置顶点着色器中的常量缓冲区。
	//0 表示槽位索引，这里表示绑定到槽位 0
	//1 表示要绑定的缓冲区数量。
	//&g_WorldBuffer 是要绑定的常量缓冲区对象。
	

	//这行代码将同样的世界变换矩阵的常量缓冲区绑定到像素着色器（Pixel Shader）中的指定槽位。
	g_ImmediateContext->PSSetConstantBuffers(0, 1, &g_WorldBuffer);
	//参数的含义与上述顶点着色器的绑定相同。


	//ビューマトリクス
	//同上（创建绑定操作）
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_ViewBuffer);
	g_ImmediateContext->VSSetConstantBuffers(1, 1, &g_ViewBuffer);
	g_ImmediateContext->PSSetConstantBuffers(1, 1, &g_ViewBuffer);




	//プロジェクションマトリクス
	//同上（创建绑定操作）
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_ProjectionBuffer);
	g_ImmediateContext->VSSetConstantBuffers(2, 1, &g_ProjectionBuffer);
	g_ImmediateContext->PSSetConstantBuffers(2, 1, &g_ProjectionBuffer);




	//マテリアル情報
	//同上（创建绑定操作）
	hBufferDesc.ByteWidth = sizeof(MATERIAL_CBUFFER);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_MaterialBuffer);
	g_ImmediateContext->VSSetConstantBuffers(3, 1, &g_MaterialBuffer);
	g_ImmediateContext->PSSetConstantBuffers(3, 1, &g_MaterialBuffer);



	//ライト情報
	//同上（创建绑定操作）
	hBufferDesc.ByteWidth = sizeof(LIGHT_CBUFFER);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_LightBuffer);
	g_ImmediateContext->VSSetConstantBuffers(4, 1, &g_LightBuffer);
	g_ImmediateContext->PSSetConstantBuffers(4, 1, &g_LightBuffer);



	//フォグ情報
	//同上（创建绑定操作）
	hBufferDesc.ByteWidth = sizeof(FOG_CBUFFER);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_FogBuffer);
	g_ImmediateContext->VSSetConstantBuffers(5, 1, &g_FogBuffer);
	g_ImmediateContext->PSSetConstantBuffers(5, 1, &g_FogBuffer);



	//縁取り
	//同上（创建绑定操作）
	hBufferDesc.ByteWidth = sizeof(FUCHI);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_FuchiBuffer);
	g_ImmediateContext->VSSetConstantBuffers(6, 1, &g_FuchiBuffer);
	g_ImmediateContext->PSSetConstantBuffers(6, 1, &g_FuchiBuffer);



	//カメラ
	//同上（创建绑定操作）
	hBufferDesc.ByteWidth = sizeof(XMFLOAT4);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_CameraBuffer);
	g_ImmediateContext->VSSetConstantBuffers(7, 1, &g_CameraBuffer);
	g_ImmediateContext->PSSetConstantBuffers(7, 1, &g_CameraBuffer);




	// 入力レイアウト設定
	//设置输入布局（Input Layout）以告诉图形渲染管线如何解释顶点数据。
	g_ImmediateContext->IASetInputLayout( g_VertexLayout );

	//IASetInputLayout 函数将输入布局设置为指定的布局对象。g_ImmediateContext 是设备上下文对象。
	//g_VertexLayout 是之前创建的输入布局对象，它定义了顶点数据的格式、语义和排列方式。



	// シェーダ設定
	//同上（创建绑定操作）
	g_ImmediateContext->VSSetShader( g_VertexShader, NULL, 0 );
	g_ImmediateContext->PSSetShader( g_PixelShader, NULL, 0 );




	//ライト初期化
	//这段代码用于初始化光照属性，并将光照数据传递给常量缓冲区供着色器使用。
	ZeroMemory(&g_Light, sizeof(LIGHT_CBUFFER));//变量的内存内容全部设置为零

	//这行代码设置光照方向，XMFLOAT4 存储了4个浮点数，表示方向向量。这个方向向量(1.0f, -1.0f, 1.0f) 可能代表了一个斜向上的光源方向。
	g_Light.Direction[0] = XMFLOAT4(1.0f, -1.0f, 1.0f, 0.0f);

	//XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);设置漫反射光的颜色。Diffuse 表示从光源发出并被物体表面反射的光线。这个颜色向量(0.9f, 0.9f, 0.9f) 可能表示了一个白色的漫反射光。
	g_Light.Diffuse[0] = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);

	//这行代码设置环境光的颜色
	//Ambient 表示在整个场景中均匀分布的光，不受光源位置和方向影响。这个颜色向量 (0.1f, 0.1f, 0.1f) 可能表示了一个较暗的灰色环境光
	g_Light.Ambient[0] = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	
	//设置光源的类型为定向光源，根据此设置，着色器可以根据光源类型进行不同的光照计算
	g_Light.Flags[0].Type = LIGHT_TYPE_DIRECTIONAL;

	//这个函数调用的目的是将光照数据写入到常量缓冲区，以便在着色器中使用。它可能是一个自定义的函数，用于更新常量缓冲区中的光照数据。
	SetLightBuffer();




	//マテリアル初期化
	//初始化材质属性并将材质数据传递给常量缓冲区供着色器使用
	MATERIAL material;//定义了一个名为 material 的结构体变量，该结构体可能包含描述材质属性的各个成员变量，例如漫反射颜色、环境颜色等。
	ZeroMemory(&material, sizeof(material));//变量的内存内容全部设置为零

	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);//设置材质的漫反射颜色(1.0f, 1.0f, 1.0f, 1.0f) 可能表示了一个白色的漫反射颜色。
	material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);//表示物体表面在环境光下的颜色，(1.0f, 1.0f, 1.0f, 1.0f) 也可能表示了一个白色的环境颜色。

	SetMaterial(material);//这个函数调用的目的是将材质数据写入到常量缓冲区，以便在着色器中使用。它可能是一个自定义的函数，用于更新常量缓冲区中的材质数据

	return S_OK;
}


//=============================================================================
// 終了処理
//=============================================================================

//这段代码用于释放和清理Direct3D 11渲染器所使用的资源和对象。在结束应用程序或者重新初始化渲染器之前，
//应该调用这个函数来确保已释放分配的资源，避免内存泄漏
void UninitRenderer(void)
{
	// オブジェクト解放
	if (g_DepthStateEnable)		g_DepthStateEnable->Release();//释放深度启用状态的资源。
	if (g_DepthStateDisable)	g_DepthStateDisable->Release();//释放深度禁用状态的资源。
	if (g_BlendStateNone)		g_BlendStateNone->Release();//释放无混合状态的资源。
	if (g_BlendStateAlphaBlend)	g_BlendStateAlphaBlend->Release();//释放Alpha混合状态的资源。
	if (g_BlendStateAdd)		g_BlendStateAdd->Release();//释放加法混合状态的资源。
	if (g_BlendStateSubtract)	g_BlendStateSubtract->Release();//释放减法混合状态的资源。
	if (g_RasterStateCullOff)	g_RasterStateCullOff->Release();//释放关闭背面剔除状态的资源。
	if (g_RasterStateCullCW)	g_RasterStateCullCW->Release();//释放顺时针背面剔除状态的资源。
	if (g_RasterStateCullCCW)	g_RasterStateCullCCW->Release();//释放逆时针背面剔除状态的资源。

	if (g_WorldBuffer)			g_WorldBuffer->Release();//释放世界矩阵缓冲区的资源。
	if (g_ViewBuffer)			g_ViewBuffer->Release();//释放视图矩阵缓冲区的资源。
	if (g_ProjectionBuffer)		g_ProjectionBuffer->Release();//释放投影矩阵缓冲区的资源。
	if (g_MaterialBuffer)		g_MaterialBuffer->Release();//释放材质缓冲区的资源。
	if (g_LightBuffer)			g_LightBuffer->Release();//释放光源缓冲区的资源。
	if (g_FogBuffer)			g_FogBuffer->Release();//释放雾效缓冲区的资源。


	if (g_VertexLayout)			g_VertexLayout->Release();//释放顶点输入布局的资源。
	if (g_VertexShader)			g_VertexShader->Release();//释放顶点着色器的资源。
	if (g_PixelShader)			g_PixelShader->Release();//释放像素着色器的资源。

	if (g_ImmediateContext)		g_ImmediateContext->ClearState();//清除设备上下文的状态。
	if (g_RenderTargetView)		g_RenderTargetView->Release();//释放渲染目标视图的资源。
	if (g_SwapChain)			g_SwapChain->Release();//释放交换链的资源。
	if (g_ImmediateContext)		g_ImmediateContext->Release();//释放设备上下文的资源。
	if (g_D3DDevice)			g_D3DDevice->Release();//释放Direct3D设备的资源。
}



//=============================================================================
// バックバッファクリア
//=============================================================================
//这段代码实现了清除渲染目标视图（也称为“后备缓冲”）和深度/模板视图的操作，以准备进行新一帧的渲染。
void Clear(void)
{
	// バックバッファクリア
	float ClearColor[4] = { 0.3f, 0.3f, 0.3f, 1.0f };//定义一个浮点数组，用于表示要清除的颜色值，每个分量表示R、G、B和Alpha通道。这里使用灰色作为清除颜色。
	g_ImmediateContext->ClearRenderTargetView( g_RenderTargetView, ClearColor );//清除渲染目标视图，将后备缓冲的内容设为指定的颜色。这里使用之前定义的ClearColor作为清除颜色。
	g_ImmediateContext->ClearDepthStencilView( g_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	//清除深度/模板视图，将深度缓冲和模板缓冲的内容设为指定的值。第一个参数是要清除的深度/模板视图，第二个参数D3D11_CLEAR_DEPTH表示要清除深度缓冲，
	//第三个参数1.0f表示要将深度值设为1.0，第四个参数0表示要将模板值设为0。
}


//=============================================================================
// プレゼント
//=============================================================================
//将渲染结果呈现到屏幕上的操作
void Present(void)
{
	//调用Present方法将渲染结果呈现到屏幕上。第一个参数0表示不使用垂直同步（垂直同步可以防止屏幕撕裂，但可能会导致性能下降），
	//第二个参数0表示不使用任何标志。
	g_SwapChain->Present( 0, 0 );
}


//=============================================================================
// デバッグ用テキスト出力
//=============================================================================
//这段代码用于在调试模式下，在屏幕上输出文本信息。
void DebugTextOut(char* text, int x, int y)
{
//这是一个预处理条件，只有在定义了_DEBUG和DEBUG_DISP_TEXTOUT的情况下才会执行其中的代码。这样可以确保只有在调试模式下才会进行文本输出。
#if defined(_DEBUG) && defined(DEBUG_DISP_TEXTOUT)
	HRESULT hr;//声明一个HRESULT类型的变量，用于存储函数调用的结果

	//バックバッファからサーフェスを取得する
	IDXGISurface1* pBackSurface = NULL;//声明一个指向IDXGISurface1接口的指针变量pBackSurface，用于获取后备缓冲区的表面。

	//使用GetBuffer方法获取后备缓冲区的表面，并将结果存储在pBackSurface中
	hr = g_SwapChain->GetBuffer(0, __uuidof(IDXGISurface1), (void**)&pBackSurface);


	if (SUCCEEDED(hr))//如果获取后备缓冲区表面成功，则执行以下代码块
	{
		//取得したサーフェスからデバイスコンテキストを取得する
		HDC hdc;//声明一个HDC类型的变量，用于存储设备上下文句柄。
		hr = pBackSurface->GetDC(FALSE, &hdc);//使用GetDC方法获取与后备缓冲区表面关联的设备上下文句柄。

		if (SUCCEEDED(hr))//如果获取设备上下文句柄成功，则执行以下代码块
		{
			//设置文本的颜色为白色。
			SetTextColor(hdc, RGB(255, 255, 255));
			//设置背景模式为透明，这样文本的背景将不会被填充。
			SetBkMode(hdc, TRANSPARENT);

			RECT rect;//声明一个RECT结构体，用于指定文本输出的矩形区域。
			rect.left = 0;//设置矩形区域的左边界为0
			rect.top = 0;//设置矩形区域的上边界为0。
			rect.right = SCREEN_WIDTH;//设置矩形区域的右边界为屏幕宽度。
			rect.bottom = SCREEN_HEIGHT;//设置矩形区域的下边界为屏幕高度

			//テキスト出力
			//使用DrawText函数在指定矩形区域内绘制文本。
			DrawText(hdc, text, (int)strlen(text), &rect, DT_LEFT);

			//デバイスコンテキストを解放する
			//释放设备上下文句柄。
			pBackSurface->ReleaseDC(NULL);
		}
		//サーフェスを解放する
		//释放后备缓冲区表面。
		pBackSurface->Release();

		//レンダリングターゲットがリセットされるのでセットしなおす
		//重新设置渲染目标，以防止渲染目标在绘制文本后被重置。
		g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetView, g_DepthStencilView);

		//通过这个DebugTextOut函数，您可以在调试模式下将文本信息输出到屏幕上，帮助您进行调试和测试。
	}
#endif
}
