//=============================================================================
// スプライト処理 [sprite.cpp]
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "sprite.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************



//=============================================================================
// 頂点データ設定
//=============================================================================

//默认中心点  颜色固定白色  
void SetSprite(ID3D11Buffer *buf, float X, float Y, float Width, float Height, float U, float V, float UW, float VH)
{
	// 定义一个 D3D11_MAPPED_SUBRESOURCE 结构体，用于映射缓冲区数据
	D3D11_MAPPED_SUBRESOURCE msr;
	
	//是用于在 DirectX 11 中映射一个缓冲区（Buffer）以便于读写数据的函数调用
	GetDeviceContext()->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	// 将映射后的数据转化为 VERTEX_3D 结构体数组
	VERTEX_3D *vertex = (VERTEX_3D*)msr.pData;

	float hw, hh;
	hw = Width * 0.5f;		// 计算精灵宽度的一半
	hh = Height * 0.5f;		//计算精灵高度的一半


	//精灵的位置是以屏幕中心点为基准进行设置的
	// 頂点０番（左上の頂点）
	vertex[0].Position = XMFLOAT3(X - hw, Y - hh, 0.0f);//精灵左上角坐标
	vertex[0].Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);//顶点的颜色属性
	vertex[0].TexCoord = XMFLOAT2(U, V);//纹理坐标

	// 頂点１番（右上の頂点）
	vertex[1].Position = XMFLOAT3(X + hw, Y - hh, 0.0f);
	vertex[1].Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].TexCoord = XMFLOAT2(U + UW, V);

	// 頂点２番（左下の頂点）
	vertex[2].Position = XMFLOAT3(X - hw, Y + hh, 0.0f);
	vertex[2].Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].TexCoord = XMFLOAT2(U, V + VH);

	// 頂点３番（右下の頂点）
	vertex[3].Position = XMFLOAT3(X + hw, Y + hh, 0.0f);
	vertex[3].Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(U + UW, V + VH);


	//// 解除缓冲区数据映射
	GetDeviceContext()->Unmap(buf, 0);

}

//默认左上角中心点  颜色固定白色  
void SetSpriteLeftTop(ID3D11Buffer *buf, float X, float Y, float Width, float Height, float U, float V, float UW, float VH)
{
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D *vertex = (VERTEX_3D*)msr.pData;

	// 左上を原点として設定するプログラム
	vertex[0].Position = XMFLOAT3(X, Y, 0.0f);
	vertex[0].Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[0].TexCoord = XMFLOAT2(U, V);

	vertex[1].Position = XMFLOAT3(X + Width, Y, 0.0f);
	vertex[1].Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].TexCoord = XMFLOAT2(U + UW, V);

	vertex[2].Position = XMFLOAT3(X, Y + Height, 0.0f);
	vertex[2].Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].TexCoord = XMFLOAT2(U, V + VH);

	vertex[3].Position = XMFLOAT3(X + Width, Y + Height, 0.0f);
	vertex[3].Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(U + UW, V + VH);



	GetDeviceContext()->Unmap(buf, 0);

}


//默认左上角中心点  颜色为传入的color  
void SetSpriteLTColor(ID3D11Buffer* buf,
	float X, float Y, float Width, float Height,
	float U, float V, float UW, float VH,
	XMFLOAT4 color)
{
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	// 左上を原点として設定するプログラム
	vertex[0].Position = XMFLOAT3(X, Y, 0.0f);
	vertex[0].Diffuse = color;// 设置顶点颜色时使用了传入的 color 参数
	vertex[0].TexCoord = XMFLOAT2(U, V);

	vertex[1].Position = XMFLOAT3(X + Width, Y, 0.0f);
	vertex[1].Diffuse = color;
	vertex[1].TexCoord = XMFLOAT2(U + UW, V);

	vertex[2].Position = XMFLOAT3(X, Y + Height, 0.0f);
	vertex[2].Diffuse = color;
	vertex[2].TexCoord = XMFLOAT2(U, V + VH);

	vertex[3].Position = XMFLOAT3(X + Width, Y + Height, 0.0f);
	vertex[3].Diffuse = color;
	vertex[3].TexCoord = XMFLOAT2(U + UW, V + VH);

	GetDeviceContext()->Unmap(buf, 0);

}


//默认最左边的一半为中心点  颜色为传入的color,增加旋转量rotationAngle

void SetSpriteLTColorRota(ID3D11Buffer* buf,
	float X, float Y, float Width, float Height,
	float U, float V, float UW, float VH,
	XMFLOAT4 color, float rotationAngle)
{
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	// 计算旋转矩阵
	XMMATRIX rotationMatrix = XMMatrixRotationZ(rotationAngle);
	XMVECTOR center = XMVectorSet(X + Width * 0.5f, Y + Height * 0.5f, 0.0f, 1.0f);

	// 用旋转矩阵旋转每个顶点，并确保围绕中心点旋转
	auto RotateVertex = [&](float x, float y) {
		XMVECTOR point = XMVectorSet(x, y, 0.0f, 1.0f);
		point -= center;
		point = XMVector3Transform(point, rotationMatrix);
		return point + center;
	};

	XMVECTOR rotatedPos = RotateVertex(X, Y);
	vertex[0].Position = XMFLOAT3(XMVectorGetX(rotatedPos), XMVectorGetY(rotatedPos), 0.0f);
	vertex[0].Diffuse = color;
	vertex[0].TexCoord = XMFLOAT2(U, V);

	rotatedPos = RotateVertex(X + Width, Y);
	vertex[1].Position = XMFLOAT3(XMVectorGetX(rotatedPos), XMVectorGetY(rotatedPos), 0.0f);
	vertex[1].Diffuse = color;
	vertex[1].TexCoord = XMFLOAT2(U + UW, V);

	rotatedPos = RotateVertex(X, Y + Height);
	vertex[2].Position = XMFLOAT3(XMVectorGetX(rotatedPos), XMVectorGetY(rotatedPos), 0.0f);
	vertex[2].Diffuse = color;
	vertex[2].TexCoord = XMFLOAT2(U, V + VH);

	rotatedPos = RotateVertex(X + Width, Y + Height);
	vertex[3].Position = XMFLOAT3(XMVectorGetX(rotatedPos), XMVectorGetY(rotatedPos), 0.0f);
	vertex[3].Diffuse = color;
	vertex[3].TexCoord = XMFLOAT2(U + UW, V + VH);

	GetDeviceContext()->Unmap(buf, 0);
}





//默认中心点  颜色为传入的color  
void SetSpriteColor(ID3D11Buffer *buf, float X, float Y, float Width, float Height,
		float U, float V, float UW, float VH,
		XMFLOAT4 color)
{
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D *vertex = (VERTEX_3D*)msr.pData;

	float hw, hh;
	hw = Width * 0.5f;	
	hh = Height * 0.5f;		

	// 指定された座標を中心に設定するプログラム

	// 頂点０番（左上の頂点）
	vertex[0].Position = XMFLOAT3(X - hw, Y - hh, 0.0f);
	vertex[0].Diffuse  = color;
	vertex[0].TexCoord = XMFLOAT2(U, V);

	// 頂点１番（右上の頂点）
	vertex[1].Position = XMFLOAT3(X + hw, Y - hh, 0.0f);
	vertex[1].Diffuse  = color;
	vertex[1].TexCoord = XMFLOAT2(U + UW, V);

	// 頂点２番（左下の頂点）
	vertex[2].Position = XMFLOAT3(X - hw, Y + hh, 0.0f);
	vertex[2].Diffuse  = color;
	vertex[2].TexCoord = XMFLOAT2(U, V + VH);

	// 頂点３番（右下の頂点）
	vertex[3].Position = XMFLOAT3(X + hw, Y + hh, 0.0f);
	vertex[3].Diffuse  = color;
	vertex[3].TexCoord = XMFLOAT2(U + UW, V + VH);

	GetDeviceContext()->Unmap(buf, 0);

}



//这个函数是用于设置带颜色和旋转的精灵的位置、颜色、纹理坐标以及旋转角度
void SetSpriteColorRotation(ID3D11Buffer *buf, float X, float Y, float Width, float Height,
	float U, float V, float UW, float VH,
	XMFLOAT4 Color, float Rot)
{

	// 定义一个 D3D11_MAPPED_SUBRESOURCE 结构体，用于映射缓冲区数据
	D3D11_MAPPED_SUBRESOURCE msr;

	//是用于在 DirectX 11 中映射一个缓冲区（Buffer）以便于读写数据的函数调用
	GetDeviceContext()->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	// 将映射后的数据转化为 VERTEX_3D 结构体数组
	VERTEX_3D *vertex = (VERTEX_3D*)msr.pData;



	float hw, hh;
	hw = Width * 0.5f;		// 计算精灵的宽度的一半
	hh = Height * 0.5f;		// 计算精灵的高度的一半



	// 计算从中心点到顶点的基础角度
	float BaseAngle = atan2f(hh, hw);			// 计算从精灵中心点到其左上角顶点的角度
	XMVECTOR temp = { hw, hh, 0.0f, 0.0f };
	temp = XMVector2Length(temp);				// 计算中心点到顶点的距离

	//从 temp 向量中提取距离值，并将其存储在名为 Radius 的变量中。这将被用作后续旋转计算的半径值，以确保顶点在固定距离内旋转。
	float Radius = 0.0f;
	XMStoreFloat(&Radius, temp);

	// 这段代码用于根据旋转角度调整精灵的四个顶点的位置。它使用三角函数（cos 和 sin）来计算旋转后的顶点位置

	//这组代码计算了左上角的顶点的新位置。它使用了精灵的中心点 X 和 Y，通过减去旋转后的半径 Radius 与中心点到左上角顶点角度 的余弦和正弦值的乘积来计算新的 x
	//和 y 坐标。最后，它将新的坐标设置给顶点数组中的第一个顶点
	float x = X - cosf(BaseAngle + Rot) * Radius;
	float y = Y - sinf(BaseAngle + Rot) * Radius;
	vertex[0].Position = XMFLOAT3(x, y, 0.0f);


	//这组代码计算了右上角的顶点的新位置。它与前面的计算类似，不同之处在于计算右上角的坐标。
	x = X + cosf(BaseAngle - Rot) * Radius;
	y = Y - sinf(BaseAngle - Rot) * Radius;
	vertex[1].Position = XMFLOAT3(x, y, 0.0f);


	//这组代码计算了左下角的顶点的新位置。它与前面的计算类似，不同之处在于计算左下角的坐标。
	x = X - cosf(BaseAngle - Rot) * Radius;
	y = Y + sinf(BaseAngle - Rot) * Radius;
	vertex[2].Position = XMFLOAT3(x, y, 0.0f);


	//这组代码计算了右下角的顶点的新位置。它与前面的计算类似，不同之处在于计算右下角的坐标。
	x = X + cosf(BaseAngle + Rot) * Radius;
	y = Y + sinf(BaseAngle + Rot) * Radius;
	vertex[3].Position = XMFLOAT3(x, y, 0.0f);


	// // 设置顶点的颜色
	vertex[0].Diffuse = Color;
	vertex[1].Diffuse = Color;
	vertex[2].Diffuse = Color;
	vertex[3].Diffuse = Color;


	//// 设置顶点的纹理坐标
	vertex[0].TexCoord = XMFLOAT2(U, V);
	vertex[1].TexCoord = XMFLOAT2(U + UW, V);
	vertex[2].TexCoord = XMFLOAT2(U, V + VH);
	vertex[3].TexCoord = XMFLOAT2(U + UW, V + VH);

	GetDeviceContext()->Unmap(buf, 0);

}

