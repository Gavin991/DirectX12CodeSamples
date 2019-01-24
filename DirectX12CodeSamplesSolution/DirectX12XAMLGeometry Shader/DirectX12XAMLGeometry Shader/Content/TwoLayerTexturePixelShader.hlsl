
// Used to send per-vertex data to the vertex shader.
cbuffer ConstantPSBuffer
{
	float4 ambientColor;
	float4 specularColor;
	float3 lightDirection;
	float  power;	
};


// 通过像素着色器传递的每个像素的颜色数据。
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL0;
	float3 EyeDir : TEXCOORD1;
	float2 uv02 : TEXCOORD2;
};


Texture2D<float4> shaderTexture : register(t0);
Texture2D<float4> shaderTexture01 : register(t1);
SamplerState SampleType : register(s0);

// (内插)颜色数据的传递函数。
float4 main(PixelShaderInput input) : SV_TARGET
{
//	// Calculate lighting assuming light color is <1,1,1,1>
//	float fLighting = saturate(dot(input.normal, lightDirection));
//
//// Load the diffuse texture and multiply by the lighting amount
//float4 cDiffuse = shaderTexture.Sample(SampleType, input.uv) * ambientColor;
//cDiffuse.a = 1;
//
//// return diffuse
//return cDiffuse;

	float4 textureColor;

	float4 tex01 = shaderTexture.Sample(SampleType, input.uv);
	float4 tex02 =  shaderTexture01.Sample(SampleType, input.uv02);
	textureColor=lerp(tex01, tex02, tex02.a);

	float3 normal = normalize(input.normal);
	float3 lightDir = normalize(lightDirection);
	float3 viewDirection = normalize(-input.EyeDir);
	float n_dot_1 = saturate( dot(lightDir, normal));
	
	float3 reflectionVector = normalize(2 * n_dot_1 * normal - lightDir);	
	float3 specular = specularColor.rgb* min(pow(saturate(dot(reflectionVector, viewDirection)), power), textureColor.w);//color.w

	float4 outputColor = (float4)0;
	outputColor.rgb = ( input.color + ambientColor +specular)* textureColor;
	return outputColor;
	
}
