#pragma once

namespace DirectX12UWPLight
{
	// 用于向顶点着色器发送 MVP 矩阵的常量缓冲区。
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
		DirectX::XMFLOAT3   lightDirction;
	};

	// 用于向顶点着色器发送每个顶点的数据。
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT3 normal;
	};

	// Used to send per-vertex data to the vertex shader.
	struct ConstantPSBuffer
	{
		DirectX::XMFLOAT4 ambientColor;
		DirectX::XMFLOAT4 specularColor;
		DirectX::XMFLOAT3 lightDirection;	
		float power;
	};
}