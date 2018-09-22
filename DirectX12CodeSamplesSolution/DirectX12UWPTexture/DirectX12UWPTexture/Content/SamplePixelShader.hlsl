// 通过像素着色器传递的每个像素的颜色数据。
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
};

Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

// (内插)颜色数据的传递函数。
float4 main(PixelShaderInput input) : SV_TARGET
{
	//return float4(input.color, 1.0f);
	float4 textureColor;
	textureColor = shaderTexture.Sample(SampleType, input.uv) * float4(input.color, 1.0f);
	return textureColor;
}
