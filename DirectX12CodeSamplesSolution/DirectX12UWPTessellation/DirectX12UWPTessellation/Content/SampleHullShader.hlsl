
//从VertexShader输出的
struct HullShaderInput
{
	float3 pos:POSITION;
	float3 color : COLOR0;
};


struct HullShaderOutput
{
	float3 pos:POSITION;
	float3 color : COLOR0;
};

struct ConstantOutputType
{
	float edges[3] : SV_TessFactor;
	float inside : SV_InsideTessFactor;
};

ConstantOutputType ColorPathConstantFunction(InputPatch<HullShaderInput, 3> inputPatch, uint patchId : SV_PrimitiveID)
{
	ConstantOutputType outa;
	//设置三角形三条边的曲面细分因子
	outa.edges[0] = 15;
	outa.edges[1] = 15;
	outa.edges[2] = 15;
	//设置三角形里面的曲面细分因子
	outa.inside = 15;
	return outa;
}

[domain("tri")]    //三角形
[partitioning("integer")]  //整数
[outputtopology("triangle_cw")]  //顺时针
[outputcontrolpoints(3)]  //三个控制点
[patchconstantfunc("ColorPathConstantFunction")]    // 补丁常量缓存函数名
HullShaderOutput main(InputPatch<HullShaderInput, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
	HullShaderOutput outa;
	outa.pos = patch[pointId].pos;	
	outa.color = patch[pointId].color;
	return outa;
}

