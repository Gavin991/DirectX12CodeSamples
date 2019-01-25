
//从VertexShader输出的
struct HullShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
	float3 norm: NORMAL;
};


struct HullShaderOutput
{
	float3 pos : POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
	float3 norm: NORMAL;
};

struct ConstantOutputType
{
	float edges[3] : SV_TessFactor;
	float inside : SV_InsideTessFactor;

	// Geometry cubic generated control points
	float3 f3B210    : POSITION3;
	float3 f3B120    : POSITION4;
	float3 f3B021    : POSITION5;
	float3 f3B012    : POSITION6;
	float3 f3B102    : POSITION7;
	float3 f3B201    : POSITION8;
	float3 f3B111    : CENTER;

	// Normal quadratic generated control points
	float3 f3N110    : NORMAL3;
	float3 f3N011    : NORMAL4;
	float3 f3N101    : NORMAL5;
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


	//--------------------------------------
	// Assign Positions
	float3 f3B003 = inputPatch[0].pos;
	float3 f3B030 = inputPatch[1].pos;
	float3 f3B300 = inputPatch[2].pos;
	// And Normals
	float3 f3N002 = inputPatch[0].norm;
	float3 f3N020 = inputPatch[1].norm;
	float3 f3N200 = inputPatch[2].norm;


	// Compute the cubic geometry control points
	// Edge control points
	outa.f3B210 = ((2.0f * f3B003) + f3B030 - (dot((f3B030 - f3B003), f3N002) * f3N002)) / 3.0f;
	outa.f3B120 = ((2.0f * f3B030) + f3B003 - (dot((f3B003 - f3B030), f3N020) * f3N020)) / 3.0f;
	outa.f3B021 = ((2.0f * f3B030) + f3B300 - (dot((f3B300 - f3B030), f3N020) * f3N020)) / 3.0f;
	outa.f3B012 = ((2.0f * f3B300) + f3B030 - (dot((f3B030 - f3B300), f3N200) * f3N200)) / 3.0f;
	outa.f3B102 = ((2.0f * f3B300) + f3B003 - (dot((f3B003 - f3B300), f3N200) * f3N200)) / 3.0f;
	outa.f3B201 = ((2.0f * f3B003) + f3B300 - (dot((f3B300 - f3B003), f3N002) * f3N002)) / 3.0f;
	// Center control point
	float3 f3E = (outa.f3B210 + outa.f3B120 + outa.f3B021 + outa.f3B012 + outa.f3B102 + outa.f3B201) / 6.0f;
	float3 f3V = (f3B003 + f3B030 + f3B300) / 3.0f;
	outa.f3B111 = f3E + ((f3E - f3V) / 2.0f);

	// Compute the quadratic normal control points, and rotate into world space
	float fV12 = 2.0f * dot(f3B030 - f3B003, f3N002 + f3N020) / dot(f3B030 - f3B003, f3B030 - f3B003);
	outa.f3N110 = normalize(f3N002 + f3N020 - fV12 * (f3B030 - f3B003));
	float fV23 = 2.0f * dot(f3B300 - f3B030, f3N020 + f3N200) / dot(f3B300 - f3B030, f3B300 - f3B030);
	outa.f3N011 = normalize(f3N020 + f3N200 - fV23 * (f3B300 - f3B030));
	float fV31 = 2.0f * dot(f3B003 - f3B300, f3N200 + f3N002) / dot(f3B003 - f3B300, f3B003 - f3B300);
	outa.f3N101 = normalize(f3N200 + f3N002 - fV31 * (f3B003 - f3B300));

	//-----------------------------------------

	return outa;
}

[domain("tri")]    //三角形
[partitioning("fractional_odd")]  //整数//integer
[outputtopology("triangle_cw")]  //顺时针
[outputcontrolpoints(3)]  //三个控制点
[patchconstantfunc("ColorPathConstantFunction")]    // 补丁常量缓存函数名
HullShaderOutput main(InputPatch<HullShaderInput, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
	HullShaderOutput outa;
	outa.pos = patch[pointId].pos;	
	outa.color = patch[pointId].color;
	outa.uv = patch[pointId].uv;
	outa.norm = patch[pointId].norm;
	return outa;
}

