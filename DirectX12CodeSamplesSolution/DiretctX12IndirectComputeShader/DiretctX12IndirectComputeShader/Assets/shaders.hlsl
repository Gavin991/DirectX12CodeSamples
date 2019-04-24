//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

cbuffer SceneConstantBuffer : register(b0)
{
	float4 velocity;
	float4 offset;
	float4 color;
	float4x4 projection;
};

// 用作顶点着色器输入的每个顶点的数据。
struct VertexShaderInput
{
	float3 pos : POSITION;	
	float2 uv : TEXCOORD;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

PSInput VSMain(VertexShaderInput input)//float4 position : POSITION
{
	PSInput result;
	float4 pos = float4(input.pos, 1.0f);
	result.position = mul(pos + offset, projection);

	float intensity = saturate((4.0f - result.position.z) / 2.0f);
	result.color = float4(color.xyz * intensity, 1.0f);

	// Store the texture coordinates for the pixel shader.
	result.uv = input.uv;
	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{	
	float4 textureColor = shaderTexture.Sample(SampleType, input.uv)*input.color;
	return textureColor;
}
