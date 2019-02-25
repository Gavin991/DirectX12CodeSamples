
// Used to send per-vertex data to the vertex shader.
cbuffer ConstantPSBuffer
{
	float4 ambientColor;
	float4 specularColor;
	//float3 lightDirection;
	float  power;
};


// ͨ��������ɫ�����ݵ�ÿ�����ص���ɫ���ݡ�
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL0;
	float3 eyeDir : TEXCOORD1;
	float3 lightDir : TEXCOORD2;
};


//Texture2D<float4> shaderTexture : register(t0);
TextureCube gCubeMap : register(t0);
Texture2D<float4> shaderTexture01 : register(t1);

SamplerState SampleType : register(s0);

// (�ڲ�)��ɫ���ݵĴ��ݺ�����
float4 main(PixelShaderInput input) : SV_TARGET
{
float4 textureColor;

//����ߵķ�������
float3 refTexCoord = normalize(-reflect(normalize(input.eyeDir), normalize(input.normal)));
float4 reflectColor = gCubeMap.Sample(SampleType, refTexCoord); //input.PosL //shaderTexture.Sample(SampleType, input.uv);// *float4(input.color, 1.0f);//

float etaRatioRed = 0.4f;
float etaRatioGreen = 0.5f;
float etaRatioBlue = 0.6f;

//ɫɢ�Ļ���ӳ�����
//��ɫ�����������
float3 RedTexCoord = normalize(-refract(normalize(input.eyeDir), normalize(input.normal), etaRatioRed));
//��ɫ�����������
float3 GreenTexCoord = normalize(-refract(normalize(input.eyeDir), normalize(input.normal), etaRatioGreen));
//��ɫ�����������
float3 BlueTexCoord = normalize(-refract(normalize(input.eyeDir), normalize(input.normal), etaRatioBlue));

float4 refractColor;
//���ζԸ�����ɫ�������в���
refractColor.r = gCubeMap.Sample(SampleType, RedTexCoord).r;
refractColor.g = gCubeMap.Sample(SampleType, GreenTexCoord).g;
refractColor.b = gCubeMap.Sample(SampleType, BlueTexCoord).b;
refractColor.a = 1;		//alphaֵ����Ϊ��͸��

float4 tex01 = lerp(reflectColor, refractColor, 0.7f);		//�����������ɫɢ�������

float4 tex02 = shaderTexture01.Sample(SampleType, input.uv);

textureColor = lerp(tex01, tex02, 0.2);

float3 normal = normalize(input.normal);
float3 lightDir = normalize(input.lightDir);//float3(-1.0, -1.0, -1.0)
float3 viewDirection = normalize(-input.eyeDir);//float3(0.0f, -0.1f, 0.0f)
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
