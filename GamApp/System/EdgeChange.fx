#include "common.fx"

texture		g_MainColorBuffer;
sampler2D g_sampleMainColor =
sampler_state
{
	Texture = <g_MainColorBuffer>;
	MinFilter = linear;
	MagFilter = linear;
	MipFilter = linear;
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
	float4 color = tex2D(g_sampleMainColor, TexCoord);

	float4 colorL = tex2D(g_sampleMainColor, TexCoord + float2(-1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight));
	float4 colorR = tex2D(g_sampleMainColor, TexCoord + float2(1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight));
	float4 colorU = tex2D(g_sampleMainColor, TexCoord + float2(-0.0f / g_ScreenWidth, -1.0f / g_ScreenHeight));
	float4 colorD = tex2D(g_sampleMainColor, TexCoord + float2(-0.0f / g_ScreenWidth, 1.0f / g_ScreenHeight));
	/*
	float4 colorL2 = tex2D(g_sampleMainColor, TexCoord + float2(-2.0f / g_ScreenWidth, -0.0f / g_ScreenHeight));
	float4 colorR2 = tex2D(g_sampleMainColor, TexCoord + float2(2.0f / g_ScreenWidth, -0.0f / g_ScreenHeight));
	float4 colorU2 = tex2D(g_sampleMainColor, TexCoord + float2(-0.0f / g_ScreenWidth, -2.0f / g_ScreenHeight));
	float4 colorD2 = tex2D(g_sampleMainColor, TexCoord + float2(-0.0f / g_ScreenWidth, 2.0f / g_ScreenHeight));
	*/
	float minG = min(min(min(colorL.x, colorR.x), colorU.x), colorD.x);

	bool isEdge[4] = { false, false, false, false };
	float nextColor[4] = { colorL.x, colorR.x, colorU.x, colorD.x };
	isEdge[0] = colorL.x < 1;
	isEdge[1] = colorR.x < 1;
	isEdge[2] = colorU.x < 1;
	isEdge[3] = colorD.x < 1;

	float finalG = 0;
	int count = 0;
	for (int i = 0; i < 4; i++)
	{
		if (nextColor[i] < 1)
		{
			count++;
			finalG += nextColor[i];
		}
	}

	finalG /= count;

	if (colorL.x < 1 || colorR.x < 1 || colorU.x < 1 || colorD.x < 1 /*|| colorL2.x < 0.2 || colorR2.x < 0.2 || colorU2.x < 0.2 || colorD2.x < 0.2*/)
		return float4(finalG, finalG, finalG, 1.0f);
	return color;
}

technique ColorChange
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}