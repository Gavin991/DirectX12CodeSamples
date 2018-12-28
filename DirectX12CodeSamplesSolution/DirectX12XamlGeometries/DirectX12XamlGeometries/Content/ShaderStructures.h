#pragma once

namespace DirectX12XamlGeometries
{
	// 用于向顶点着色器发送 MVP 矩阵的常量缓冲区。
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
		DirectX::XMFLOAT3   lightDirction;
		DirectX::XMFLOAT3   cameraPosition;		
	};

	// 用于向顶点着色器发送每个顶点的数据。
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT3 normal;
	};

	//用于向像素着色器发生的数据
	struct ConstantPSBuffer
	{
		DirectX::XMFLOAT4 ambientColor;
		DirectX::XMFLOAT4 specularColor;
		DirectX::XMFLOAT3 lightDirection;	
		float power;
	};

	// 用于向顶点着色器发送 MVP 矩阵的常量缓冲区。
	struct ModelViewProjectionConstantBufferForPlane
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	// 用于向顶点着色器发送每个顶点的数据。
	struct VertexPositionColorForPlane
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
		DirectX::XMFLOAT2 uv;
	};
}