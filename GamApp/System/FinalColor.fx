#include "common.fx"

texture	g_FinalColorBuffer;
sampler2D g_sampleFinalColor =
sampler_state
{
	Texture = <g_FinalColorBuffer>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
};

struct OutputVS
{
	float4 posWVP         : POSITION0;
	float2 TexCoord		: TEXCOORD0;
};


OutputVS VShader(float4 posL       : POSITION0,
				 float2 TexCoord : TEXCOORD0)
{
	OutputVS outVS = (OutputVS)0;

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	outVS.posWVP = mul(posL, g_WorldViewProj);

	outVS.TexCoord = TexCoord;

	return outVS;
}

float4 PShader(float2 TexCoord : TEXCOORD0) : COLOR
{
	//纹理采样
	float4 Texture = tex2D(g_sampleFinalColor, TexCoord);
	
	return Texture;
}

technique FinalColor
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}