// 存储用于构成几何图形的三个基本列优先矩阵的常量缓冲区。
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

struct DomainShaderInput
{
	float3 pos:POSITION;
	float3 color : COLOR0;
};

struct DomainShaderOutput
{
	float4 pos:SV_POSITION;
	float3 color : COLOR0;
};

struct ConstantOutputType
{
	float edges[3] : SV_TessFactor;
	float inside : SV_InsideTessFactor;
};

/*DomainShader*/
[domain("tri")]
DomainShaderOutput main(ConstantOutputType input, float3 uvwCoord : SV_DomainLocation, const OutputPatch<DomainShaderInput, 3> patch)
{
	DomainShaderOutput Output;
	Output.pos.xyz = patch[0].pos*uvwCoord.x + patch[1].pos*uvwCoord.y + patch[2].pos*uvwCoord.z;
	Output.pos.w = 1;
	float4 pos = Output.pos;
	// Transform the vertex position into projected space.
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	Output.pos = pos;
	Output.color = patch[0].color*uvwCoord.x + patch[1].color*uvwCoord.y + patch[2].color*uvwCoord.z;	
	return Output;
}

