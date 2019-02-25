// �洢���ڹ��ɼ���ͼ�ε��������������Ⱦ���ĳ�����������
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

// ����������ɫ�������ÿ����������ݡ�
struct CubeMapVertexShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
};

// ͨ��������ɫ�����ݵ�ÿ�����ص���ɫ���ݡ�
struct  CubeMapPixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
	float3 PosL : POSITION;
};

// ������ GPU ��ִ�ж��㴦��ļ���ɫ����
CubeMapPixelShaderInput main(CubeMapVertexShaderInput input)
{
	CubeMapPixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

	// ������λ��ת��ΪͶӰ�ռ䡣
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	// �����޸ĵش�����ɫ��
	output.color = input.color;

	// Store the texture coordinates for the pixel shader.
	output.uv = input.uv;

	// Use local vertex position as cubemap lookup vector.
	output.PosL = input.pos;

	return output;
}
