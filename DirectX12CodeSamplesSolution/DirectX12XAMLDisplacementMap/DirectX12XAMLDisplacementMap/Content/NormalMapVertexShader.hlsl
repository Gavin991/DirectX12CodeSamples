// 存储用于构成几何图形的三个基本列优先矩阵的常量缓冲区。
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float3 lightPos;//lightDir
	float3 cameraPos;
};

// 用作顶点着色器输入的每个顶点的数据。
struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
	float3 norm: NORMAL;
	float3 tangent: TANGENT;
};

// 通过像素着色器传递的每个像素的颜色数据。
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


// 用于在 GPU 上执行顶点处理的简单着色器。
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
	// 将顶点位置转换为投影空间。
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	//存储纹理UV信息
	output.uv = input.uv;
	//把顶点的法线转换到世界坐标系中
	output.normal = normalize(mul(input.norm, (float3x3)model));
	//把局部的顶点坐标转换到世界坐标系中
	float3 worldPos = mul(input.pos, (float3x3)model);	
	//每个顶点指向光源的向量													
	output.lightDir = normalize(lightPos - worldPos);	
	//每个顶点指向观察者相机的向量
	output.eyeDir = normalize(cameraPos - worldPos);
	//把顶点的切线转换到世界坐标系中
	output.tangent = normalize(mul(input.tangent, (float3x3)model));

	return output;
}
