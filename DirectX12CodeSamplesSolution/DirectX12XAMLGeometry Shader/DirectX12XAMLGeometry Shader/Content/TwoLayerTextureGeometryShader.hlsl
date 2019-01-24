
// 存储用于构成几何图形的三个基本列优先矩阵的常量缓冲区。
cbuffer GeometryShaderConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float explode;
	float3 lightDir;
	float3 cameraPos;
};


// 通过像素着色器传递的每个像素的颜色数据。
struct VertexShaderOutput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL0;
	float3 EyeDir : TEXCOORD1;
	float2 uv02 : TEXCOORD2;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL0;
	float3 EyeDir : TEXCOORD1;
	float2 uv02 : TEXCOORD2;
};

[maxvertexcount(12)]
void main(
	triangle VertexShaderOutput input[3] : SV_POSITION,
	inout TriangleStream< GSOutput > output
)
{

	//float Explode = 0.55;
	// Calculate the face normal
	//
	float3 faceEdgeA = input[1].pos - input[0].pos;
	float3 faceEdgeB = input[2].pos - input[0].pos;
	float3 faceNormal = normalize(cross(faceEdgeA, faceEdgeB));
	float3 ExplodeAmt = faceNormal*explode;


	////
	//// Calculate the face center
	////
	//float3 centerPos = (input[0].pos.xyz + input[1].pos.xyz + input[2].pos.xyz) / 3.0;
	//float2 centerTex = (input[0].uv + input[1].uv + input[2].uv) / 3.0;
	//centerPos += faceNormal*explode;

	float fLighting=0;
	//
	// Output the pyramid
	//
	for (int i = 0; i<3; i++)
	{
		GSOutput element=(GSOutput)0;

		element.pos = input[i].pos + float4(ExplodeAmt, 0);
		element.pos = mul(element.pos, view);
		element.pos = mul(element.pos, projection);
		//element.color =  input[i].color;
		element.uv = input[i].uv;
		element.normal = input[i].normal;

		fLighting = saturate(dot(element.normal, lightDir));
		element.color.rgb = fLighting ;// saturate(fLighting + input.color);

		element.EyeDir = input[i].EyeDir;
		element.uv02 = input[i].uv02;

		output.Append(element);

		int iNext = (i + 1) % 3;
		element.pos = input[iNext].pos + float4(ExplodeAmt, 0);
		element.pos = mul(element.pos, view);
		element.pos = mul(element.pos, projection);
		//element.color = input[iNext].color;
		element.uv = input[iNext].uv;
		element.normal = input[iNext].normal;
		
		fLighting = saturate(dot(element.normal, lightDir));
		element.color.rgb = fLighting;// saturate(fLighting + input.color);

		element.EyeDir = input[iNext].EyeDir;
		element.uv02 = input[iNext].uv02;

		output.Append(element);

		iNext = (i + 2) % 3;
		element.pos = input[iNext].pos + float4(ExplodeAmt, 0);
		element.pos = mul(element.pos, view);
		element.pos = mul(element.pos, projection);
		//element.color = input[iNext].color;
		element.uv = input[iNext].uv;
		element.normal = input[iNext].normal;

		fLighting = saturate(dot(element.normal, lightDir));
		element.color.rgb = fLighting;// saturate(fLighting + input.color);

		element.EyeDir = input[iNext].EyeDir;
		element.uv02 = input[iNext].uv02;

		output.Append(element);

		//element.pos = float4(centerPos, 1) + float4(ExplodeAmt, 0);
		//element.pos = mul(element.pos, view);
		//element.pos = mul(element.pos, projection);
		//element.normal = faceNormal;
		//element.uv = centerTex;
		//output.Append(element);

		output.RestartStrip();
	}

	//for (int j = 2; j >= 0; j--)
	//{
	//	GSOutput element=(GSOutput)0;;
	//	element.pos = input[j].pos + float4(ExplodeAmt, 0);
	//	element.pos = mul(element.pos, view);
	//	element.pos = mul(element.pos, projection);
	//	//element.color = input[j].color;
	//	element.uv = input[j].uv;
	//	element.normal = input[j].normal;

	//	fLighting = saturate(dot(element.normal, lightDir));
	//	element.color.rgb = fLighting;// saturate(fLighting + input.color);

	//	element.EyeDir = input[j].EyeDir;
	//	element.uv02 = input[j].uv02;

	//	output.Append(element);
	//}
	//output.RestartStrip();

	//-------------------------------------------------------------------




	//----------------------------------------------------------------
	//for (uint i = 0; i < 3; i++)
	//{
	//	GSOutput element;
	//	element.pos = input[i].pos;
	//	element.color =  input[i].color;
	//	element.uv = input[i].uv;
	//	element.normal = input[i].normal;
	//	element.EyeDir = input[i].EyeDir;
	//	element.uv02 = input[i].uv02;
	//	output.Append(element);
	//}
}