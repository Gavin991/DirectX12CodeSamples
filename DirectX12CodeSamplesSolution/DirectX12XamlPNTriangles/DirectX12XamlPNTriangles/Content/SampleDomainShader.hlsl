// 存储用于构成几何图形的三个基本列优先矩阵的常量缓冲区。
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float3 lightDir;
};

struct DomainShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
	float3 norm: NORMAL;
};

struct DomainShaderOutput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
	float3 norm : NORMAL0;
	float3 EyeDir : TEXCOORD1;
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

/*DomainShader*/
[domain("tri")]
DomainShaderOutput main(ConstantOutputType input, float3 uvwCoord : SV_DomainLocation, const OutputPatch<DomainShaderInput, 3> patch)
{
	DomainShaderOutput Output;

	//--------------------------------------------
	// The barycentric coordinates
	float fU = uvwCoord.x;
	float fV = uvwCoord.y;
	float fW = uvwCoord.z;

	// Precompute squares and squares * 3 
	float fUU = fU * fU;
	float fVV = fV * fV;
	float fWW = fW * fW;
	float fUU3 = fUU * 3.0f;
	float fVV3 = fVV * 3.0f;
	float fWW3 = fWW * 3.0f;

	// Compute position from cubic control points and barycentric coords
	float3 f3Position = patch[0].pos * fWW * fW +
		patch[1].pos * fUU * fU +
		patch[2].pos * fVV * fV +
		input.f3B210 * fWW3 * fU +
		input.f3B120 * fW * fUU3 +
		input.f3B201 * fWW3 * fV +
		input.f3B021 * fUU3 * fV +
		input.f3B102 * fW * fVV3 +
		input.f3B012 * fU * fVV3 +
		input.f3B111 * 6.0f * fW * fU * fV;

	// Compute normal from quadratic control points and barycentric coords
	float3 f3Normal = patch[0].norm * fWW +
		patch[1].norm * fUU +
		patch[2].norm * fVV +
		input.f3N110 * fW * fU +
		input.f3N011 * fU * fV +
		input.f3N101 * fW * fV;
	// Normalize the interpolated normal    
	f3Normal = normalize(f3Normal);

	Output.pos.xyz = f3Position;
	Output.pos.w = 1;
	float4 pos = Output.pos;
	// Transform the vertex position into projected space.
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	Output.pos = pos;

	Output.uv = patch[0].uv*fW + patch[1].uv*fU + patch[2].uv*fV;

	Output.norm = f3Normal;
	Output.norm = normalize(mul(Output.norm, (float3x3)model));
	float fLighting = saturate(dot(Output.norm, lightDir));
	Output.color.rgb = fLighting;

	float3 kaopos = f3Position;
	float4 g_CameraPos = float4(0.0f, 0.7f, 1.5f, 0.0f);
	Output.EyeDir = mul(kaopos, (float3x3)model);
	Output.EyeDir = normalize(g_CameraPos.xyz - Output.EyeDir);
	//-------------------------------------------
	return Output;
}

