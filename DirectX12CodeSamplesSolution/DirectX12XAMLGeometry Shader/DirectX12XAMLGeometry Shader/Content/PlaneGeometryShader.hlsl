// 通过像素着色器传递的每个像素的颜色数据。
struct  PlaneVertexShaderOutput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
};

[maxvertexcount(12)]
void main(
	triangle PlaneVertexShaderOutput input[3] : SV_POSITION,
	inout TriangleStream< GSOutput > output
)
{

	//
	// 将一个三角形分裂成三个三角形，即没有v3v4v5的三角形
	//       v1
	//       /\
	//      /  \
    //   v3/____\v4
    //    /\xxxx/\
    //   /  \xx/  \
    //  /____\/____\
    // v0    v5    v2

	
	GSOutput vertexes[6];
	int i;
	[unroll]
	for (i = 0; i < 3; ++i)
	{
		vertexes[i] = input[i];
		vertexes[i + 3].color = (input[i].color + input[(i + 1) % 3].color) / 2.0f;
		vertexes[i + 3].pos = (input[i].pos + input[(i + 1) % 3].pos) / 2.0f;
		vertexes[i + 3].uv = (input[i].uv + input[(i + 1) % 3].uv) / 2.0f;
	}

	[unroll]
	for (i = 0; i < 3; ++i)
	{
		output.Append(vertexes[i]);
		output.Append(vertexes[3 + i]);
		output.Append(vertexes[(i + 2) % 3 + 3]);
		output.RestartStrip();
	}

	output.Append(vertexes[3]);
	output.Append(vertexes[4]);
	output.Append(vertexes[5]);
	//output.RestartStrip();

	//for (uint i = 0; i < 3; i++)
	//{
	//	GSOutput element;
	//	element.pos = input[i].pos;
	//	element.color = input[i].color;
	//	element.uv = input[i].uv;
	//	output.Append(element);
	//}

////-----------------------------------------------------------------
//	float4 p[6];
//	p[0] = input[0].pos;
//	p[1] = 0.5f*(input[0].pos + input[1].pos);
//	p[2] = 0.5f*(input[2].pos + input[0].pos);
//	p[3] = 0.5f*(input[1].pos + input[2].pos);
//	p[4] = input[2].pos;
//	p[5] = input[1].pos;
//
//	float3 c[6];
//	c[0] = input[0].color;
//	c[1] = 0.5f*(input[0].color + input[1].color);
//	c[2] = 0.5f*(input[2].color + input[0].color);
//	c[3] = 0.5f*(input[1].color + input[2].color);
//	c[4] = input[2].color;
//	c[5] = input[1].color;
//
//	float2 uv[6];
//	uv[0] = input[0].uv;
//	uv[1] = 0.5f*(input[0].uv + input[1].uv);
//	uv[2] = 0.5f*(input[2].uv + input[0].uv);
//	uv[3] = 0.5f*(input[1].uv + input[2].uv);
//	uv[4] = input[2].uv;
//	uv[5] = input[1].uv;
//
//	// 画分割后的小三角形bcd
//	for (int i = 0; i < 5; ++i)
//	{	
//		GSOutput element;
//		element.pos = p[i];
//		element.color =  c[i];
//		element.uv = uv[i];		
//		output.Append(element);
//	}
//
//	output.RestartStrip();
//
//	GSOutput element01;
//	element01.pos = p[1];
//	element01.color = c[1];
//	element01.uv = uv[1];
//	output.Append(element01);
//
//	GSOutput element02;
//	element02.pos = p[5];
//	element02.color = c[5];
//	element02.uv = uv[5];	
//	output.Append(element02);
//
//	GSOutput element03;
//	element03.pos = p[3];
//	element03.color = c[3];
//	element03.uv = uv[3];	
//	output.Append(element03);
//-------------------------------------------------
	//for (uint i = 0; i < 3; i++)
	//{
	//	GSOutput element;
	//	element.pos = input[i].pos;
	//	element.color = input[i].color;
	//	element.uv = input[i].uv;
	//	output.Append(element);
	//}
}