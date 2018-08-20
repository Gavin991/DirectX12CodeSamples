Texture2D  g_MeshTexture;              // Color texture for mesh
float4x4 g_mWorld;    // Worldmatrix
float4x4 g_mWorldViewProjection;    // World * View * Projection matrix


float4 g_Emissive;
float4 g_LightDir;
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
};
;

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
   PS_INPUT output = (PS_INPUT)0;
   output.Pos = mul( float4(input.Pos,1), g_mWorldViewProjection);
   float3 vNormalWorldSpace = normalize( mul( input.Norm, (float3x3)g_mWorld ) );

    float fLighting = saturate( dot( vNormalWorldSpace, g_LightDir) );
    output.Diffuse.rgb = fLighting;
    output.Diffuse.a = 1.0f; 

   output.Tex = input.Tex;    
   return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{  
 float4 outputColor =  g_MeshTexture.Sample(MeshTextureSampler, input.Tex)* input.Diffuse;
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