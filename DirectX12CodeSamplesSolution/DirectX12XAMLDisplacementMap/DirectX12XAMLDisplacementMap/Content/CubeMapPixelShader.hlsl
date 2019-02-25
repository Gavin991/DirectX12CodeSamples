// ͨ��������ɫ�����ݵ�ÿ�����ص���ɫ���ݡ�
struct CubeMapPixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
	float3 PosL : POSITION;
};

//Texture2D shaderTexture_plane : register(t0);
TextureCube gCubeMap : register(t0);
SamplerState SampleType_plane : register(s0);

// (�ڲ�)��ɫ���ݵĴ��ݺ�����
float4 main(CubeMapPixelShaderInput input) : SV_TARGET
{
	//return float4(input.color, 1.0f);
	float4 textureColor;
	return textureColor = gCubeMap.Sample(SampleType_plane, input.PosL);
	//textureColor = shaderTexture_plane.Sample(SampleType_plane, input.uv) * float4(input.color, 1.0f);
	//return textureColor;
}
