// 用作顶点着色器输入的每个顶点的数据。
struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR0;
};

// 通过像素着色器传递的每个像素的颜色数据。
struct VertexShaderOutput
{
	//float4 pos : SV_POSITION;
	//float3 color : COLOR0;
	float3 pos : POSITION;
	float3 color : COLOR0;
};

// 用于在 GPU 上执行顶点处理的简单着色器。
VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	//float4 pos = float4(input.pos, 1.0f);
	//// 将顶点位置转换为投影空间。
	//pos = mul(pos, model);
	//pos = mul(pos, view);
	//pos = mul(pos, projection);
	//output.pos = pos;
	output.pos = input.pos;
	// 不加修改地传递颜色。
	output.color = input.color;
	return output;
}
