
Texture2D gInput            : register(t0);
RWTexture2D<float4> gOutput : register(u0);

#define cMult 0.0001002707309736288
#define aSubtract 0.2727272727272727
float4 randGrieu(float4 t)
{
	float a = t.x + t.z*cMult + aSubtract - floor(t.x);
	a *= a;
	float b = t.y + a;
	b -= floor(b);
	float c = t.z + b;
	c -= floor(c);
	float d = c;
	a += c*cMult + aSubtract - floor(a);
	a *= a;
	b += a;
	b -= floor(b);
	c += b;
	c -= floor(c);
	return float4(a, b, c, d);
}

 float colorBlend(float scale, float dest, float src)
{
	return (scale * dest + (1.0 - scale) * src);

}

 void writeToPixel(int x, int y, float4 colour)
 {
	 uint index = (x + y * 1024);

	 int ired = (int)(clamp(colour.r, 0, 1) * 255);
	 int igreen = (int)(clamp(colour.g, 0, 1) * 255) << 8;
	 int iblue = (int)(clamp(colour.b, 0, 1) * 255) << 16;
	 int ialpha = (int)(clamp(colour.a, 0, 1) * 255) << 24;

	 float2 curIndex;
	 curIndex.x = x;
	 curIndex.y = y;

	 float4 curCol;
	 // curCol.r;
	 gOutput[curIndex] = ired + igreen + iblue + ialpha;
 }

[numthreads(32, 32,1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	
	float4 pixel = gInput[DTid.xy];

	//子業-----start
	pixel.rgb = pixel.r * 0.3 + pixel.g * 0.59 + pixel.b * 0.11;
	pixel.a = 1;
	//子業-----end

	//burn-----start
	pixel.rgb = pixel.rgb*1.6f;	
	//burn-----end
	//float4 randomcolor = randGrieu(pixel);

	gOutput[DTid.xy] = pixel;

	int x = DTid.x;
	int y = DTid.y;
	float2 curIndex;

	GroupMemoryBarrierWithGroupSync();

	int tempx = x % 256;
	int tempy = y % 256;
	//if (dot(pixel.rgb, float3(1, 1, 1)) > 2.999)
	if (tempx == tempy&&tempx==0)
	{
		for (float alpha = 0; alpha<360; alpha += 1)
		{
			curIndex.x = x + cos(alpha) * 15;
			curIndex.y = y + sin(alpha) * 15;
			gOutput[curIndex] = float4(0.2, 0, 0.2, 1.0);
		
		}
	}


	//gOutput[DTid.xy] = pixel;
}