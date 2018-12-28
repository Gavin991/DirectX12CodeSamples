// 通过像素着色器传递的每个像素的颜色数据。
struct PlanePixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
};

Texture2D shaderTexture_plane : register(t0);
SamplerState SampleType_plane : register(s0);

// (内插)颜色数据的传递函数。
float4 main(PlanePixelShaderInput input) : SV_TARGET
{
	//return float4(input.color, 1.0f);
	float4 textureColor;
	textureColor = shaderTexture_plane.Sample(SampleType_plane, input.uv) * float4(input.color, 1.0f);
	return textureColor;
}
