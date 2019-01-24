
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
};


//Texture2D<float4> shaderTexture : register(t0);
TextureCube gCubeMap : register(t0);
Texture2D<float4> shaderTexture01 : register(t1);

SamplerState SampleType : register(s0);

// (内插)颜色数据的传递函数。
float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 textureColor;

float etaRatio = 0.5f;
//求光线的折射后向量
float3 refTexCoord = normalize(-refract(normalize(input.EyeDir), normalize(input.normal), etaRatio));

float4 tex01 = gCubeMap.Sample(SampleType, refTexCoord); //input.PosL //shaderTexture.Sample(SampleType, input.uv);// *float4(input.color, 1.0f);//
float4 tex02 = shaderTexture01.Sample(SampleType, input.uv);

textureColor = lerp(tex01, tex02, 0.2);

float3 normal = normalize(input.normal);
float3 lightDir = normalize(lightDirection);//float3(-1.0, -1.0, -1.0)
float3 viewDirection = normalize(-input.EyeDir);//float3(0.0f, -0.1f, 0.0f)
float n_dot_1 = saturate(dot(lightDir, normal));

//R = 2 * (N.L) * N - L
float3 reflectionVector = normalize(2 * n_dot_1 * normal - lightDir);
//specular = R.V^n with gloss map in color texture's alpha channel	
float3 specular = specularColor.rgb* min(pow(saturate(dot(reflectionVector, viewDirection)), power), textureColor.w);//color.w

float4 outputColor = (float4)0;
outputColor.rgb = (input.color + ambientColor.xyz + specular)* textureColor.xyz;
outputColor.a = 1.0f;
return outputColor;
//return float4(input.color, 1.0f);
}
