
// Used to send per-vertex data to the vertex shader.
cbuffer ConstantPSBuffer
{
	float4 ambientColor;
	float4 specularColor;	
	float4 theOtherVar;
	//float  power;
	//bool enbleCubeMape;
	//bool enbleNormalMape;
};


// ͨ��������ɫ�����ݵ�ÿ�����ص���ɫ���ݡ�
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL0;
	float3 eyeDir : TEXCOORD1;
	float3 lightDir : TEXCOORD2;
	float3 tangent:TANGENT;
};


//Texture2D<float4> shaderTexture : register(t0);
TextureCube gCubeMap : register(t0);
Texture2D<float4> shaderTexture01 : register(t1);
Texture2D<float4> normalTexture : register(t2);

SamplerState SampleType : register(s0);

// (�ڲ�)��ɫ���ݵĴ��ݺ�����
float4 main(PixelShaderInput input) : SV_TARGET
{

	float2 uv = input.uv;			//�õ�uv��������
	float3 B = cross(input.normal, input.tangent);
	if (theOtherVar.w > 0)			//�Ƿ����Ӳ���ͼ����		
	{	
		float parallaxAmplitude = 0.05f;
		float parallaxOffset = 0.8f;
		//�����ߺ͸�����ת�����۲��߷���
		float2 camDir = mul(float2x3(input.tangent,B), input.eyeDir);
		float depth = normalTexture.Sample(SampleType, uv).a - parallaxOffset;	//�����߶�
		uv -= depth*parallaxAmplitude*camDir;				//��uv����ƫ��
	}


	float4 textureColor = shaderTexture01.Sample(SampleType, uv);

	if (theOtherVar.y>0)
	{
		//����ߵķ�������
		float3 refTexCoord = normalize(-reflect(normalize(input.eyeDir), normalize(input.normal)));
		float4 tex01 = gCubeMap.Sample(SampleType, refTexCoord);
		textureColor = lerp(textureColor, tex01, 0.7);
	}

	float3 normal = normalize(input.normal);
	if (theOtherVar.z>0)
	{
		float3 normalMapScale = { 1,1,1 };//{ -1,-1,1 }
		float3 bumpNorm = normalTexture.Sample(SampleType, uv);	//�Է�����ͼ���в���
		float3 normalComponent = 2 * (bumpNorm - 0.5);			//�Բ����Ľ�����н�ѹ
		bumpNorm = normalComponent*normalMapScale;		//�Խ�ѹ�Ľ������һ������

		/*float3 B = cross(input.normal, input.tangent);*/
		float3x3 TBN = float3x3(input.tangent, B, input.normal);
		normal = mul(bumpNorm, TBN);	//������ת�������߿ռ�
	}
	

	
	float3 lightDir = normalize(input.lightDir);//float3(-1.0, -1.0, -1.0)
	float3 viewDirection = normalize(-input.eyeDir);//float3(0.0f, -0.1f, 0.0f)
	float n_dot_1 = saturate(dot(lightDir, normal));//normal
	
	//R = 2 * (N.L) * N - L
	float3 reflectionVector = normalize(2 * n_dot_1 * normal - lightDir);
	//specular = R.V^n with gloss map in color texture's alpha channel	
	float3 specular = specularColor.rgb* min(pow(saturate(dot(reflectionVector, viewDirection)), theOtherVar.x), textureColor.w);//color.wpower
	
	float fLighting = saturate(dot(normal, lightDir));
	float3  diffuseColor= fLighting;
	
	float4 outputColor = (float4)0;
	outputColor.rgb = (diffuseColor + specular + ambientColor.xyz)* textureColor.xyz;//+ specular+ ambientColor.xyz
	outputColor.a = 1.0f;
	
	return outputColor;//outputColor

}
