// �洢���ڹ��ɼ���ͼ�ε��������������Ⱦ���ĳ�����������
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float3 lightPos;//lightDir
	float3 cameraPos;
};

// ����������ɫ�������ÿ����������ݡ�
struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
	float3 norm: NORMAL;
	float3 tangent: TANGENT;
};

// ͨ��������ɫ�����ݵ�ÿ�����ص���ɫ���ݡ�
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL0;
	float3 eyeDir : TEXCOORD1;
	float3 lightDir : TEXCOORD2;
	float3 tangent:TANGENT;
};


Texture2D<float4> normalTexture : register(t2);

SamplerState SampleType : register(s0);


// ������ GPU ��ִ�ж��㴦��ļ���ɫ����
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	
	float3 vWorldPos = input.pos;
	// Sample normal and height map
	float4 vNormalHeight = normalTexture.SampleLevel(SampleType, input.uv, 0);
	// Displace vertex along normal
	/*vWorldPos += -1*input.norm * (0.2f * (vNormalHeight.w - 8.0));*/
	vWorldPos += 1 * input.norm * (0.05f * (vNormalHeight.w - 1.0));

	float4 pos = float4(vWorldPos, 1.0f);
	// ������λ��ת��ΪͶӰ�ռ䡣
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	//�洢����UV��Ϣ
	output.uv = input.uv;
	//�Ѷ���ķ���ת������������ϵ��
	output.normal = normalize(mul(input.norm, (float3x3)model));
	//�Ѿֲ��Ķ�������ת������������ϵ��
	float3 worldPos = mul(input.pos, (float3x3)model);	
	//ÿ������ָ���Դ������													
	output.lightDir = normalize(lightPos - worldPos);	
	//ÿ������ָ��۲������������
	output.eyeDir = normalize(cameraPos - worldPos);
	//�Ѷ��������ת������������ϵ��
	output.tangent = normalize(mul(input.tangent, (float3x3)model));

	return output;
}
