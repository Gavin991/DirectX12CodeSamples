Texture2D  g_MeshTexture;              // Color texture for mesh
float4x4 g_mWorld;    // World matrix
float4x4 g_mView;    //  View  matrix
float4x4 g_mProjection;    //Projection matrix


float4 g_Ambient;
float4 g_LightDir;
float4 g_CameraPos;
float4 g_Specular;
float  g_Power;
//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------
SamplerState MeshTextureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 Norm: NORMAL;           //normal
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 Diffuse : COLOR0;
	float2 Tex : TEXCOORD0;
        float3 Normal : NORMAL;
	float3 EyeDir : TEXCOORD1;
};
;

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
   PS_INPUT output = (PS_INPUT)0;

    output.Pos= mul(float4(input.Pos,1), g_mWorld );
    output.Pos= mul( output.Pos, g_mView );
    output.Pos= mul( output.Pos, g_mProjection );

    output.Normal=normalize( mul( input.Norm, (float3x3)g_mWorld ) );
   // float3 vNormalWorldSpace = normalize( mul( input.Norm, (float3x3)g_mWorld ) );

    //float fLighting = saturate( dot( vNormalWorldSpace, g_LightDir) );
    float fLighting = saturate( dot( output.Normal, g_LightDir) );
    output.Diffuse.rgb = fLighting;
    output.Diffuse.a = 1.0f; 

    output.EyeDir=mul( input.Pos, (float3x3)g_mWorld );
    output.EyeDir=normalize(g_CameraPos.xyz -output.EyeDir);
 
   output.Tex = input.Tex;    
   return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{  
float4 outputColor=(float4)0;
float4 color =  g_MeshTexture.Sample(MeshTextureSampler, input.Tex);

float3 normal = normalize(input.Normal);
float3 lightDirection = normalize(g_LightDir);
float3 viewDirection= normalize(input.EyeDir);
float n_dot_1 = dot(lightDirection, normal);
//R = 2 * (N.L) * N - L
float3 reflectionVector = normalize(2 * n_dot_1 * normal - lightDirection);
//specular = R.V^n with gloss map in color texture's alpha channel
float3 specular = g_Specular.rgb * g_Specular.a * min(pow(saturate(dot(reflectionVector, viewDirection)), g_Power), color.w);//color.w

outputColor.rgb=(g_Ambient.rgb +input.Diffuse.rgb+specular)* color  ;

outputColor.a = 1;
return outputColor ;
}

//--------------------------------------------------------------------------------------
// Renders scene to render target using D3D11 Techniques
//--------------------------------------------------------------------------------------
technique11 RenderCubeWithTexture
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0,VS()));
		SetPixelShader(CompileShader(ps_4_0,PS()));
	}
}