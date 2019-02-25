// �洢���ڹ��ɼ���ͼ�ε��������������Ⱦ���ĳ�����������
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float3 lightDir;
	float3 cameraPos;
};

// ����������ɫ�������ÿ����������ݡ�
struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
	float3 norm: NORMAL;
};

// ͨ��������ɫ�����ݵ�ÿ�����ص���ɫ���ݡ�
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;	
	float3 normal : NORMAL0;
	float3 EyeDir : TEXCOORD1;
	float3 PosL : POSITION;
};

// ������ GPU ��ִ�ж��㴦��ļ���ɫ����
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

	// ������λ��ת��ΪͶӰ�ռ䡣
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;


	//// Store the texture coordinates for the pixel shader.
	output.uv = input.uv;

	output.normal = normalize(mul(input.norm, (float3x3)model));		
	float fLighting = saturate(dot(output.normal, lightDir));	
	output.color.rgb = fLighting ;// saturate(fLighting + input.color);

	//float4 g_CameraPos = float4(0.0f, 0.7f, 1.5f, 0.0f);
	output.EyeDir = mul(input.pos, (float3x3)model);
	output.EyeDir = normalize(cameraPos - output.EyeDir);

	// Use local vertex position as cubemap lookup vector.
	output.PosL = input.pos;

	return output;
}
