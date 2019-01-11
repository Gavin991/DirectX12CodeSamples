// 存储用于构成几何图形的三个基本列优先矩阵的常量缓冲区。
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float3 lightDir;
	float3 cameraPos;
};

// 用作顶点着色器输入的每个顶点的数据。
struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
	float3 norm: NORMAL;
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

// 用于在 GPU 上执行顶点处理的简单着色器。
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

	// 将顶点位置转换为投影空间。
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;



	//// Store the texture coordinates for the pixel shader.
	output.uv = input.uv;

	output.normal = normalize(mul(input.norm, (float3x3)model));
	float fLighting = saturate(dot(output.normal, lightDir));
	output.color.rgb = fLighting;// saturate(fLighting + input.color);

								 //float4 g_CameraPos = float4(0.0f, 0.7f, 1.5f, 0.0f);
	output.EyeDir = mul(input.pos, (float3x3)model);
	output.EyeDir = normalize(cameraPos - output.EyeDir);

	return output;
}
