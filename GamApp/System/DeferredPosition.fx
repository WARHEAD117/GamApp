#include "common.fx"

float4		g_ViewPos;

struct OutputVS
{
	float4 posWVP			: POSITION;
	float4 posV				: TEXCOORD0;
	float4 posP				: TEXCOORD1;
};


OutputVS VShader(float4 posL		: POSITION0)
{
	OutputVS outVS = (OutputVS)0;

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	outVS.posWVP	= mul(posL, g_WorldViewProj);
	outVS.posV		= mul(posL, g_WorldView);
	outVS.posP		= outVS.posWVP;

	return outVS;
}


float4 PShader(float4 posV : TEXCOORD0,
				float4 posP : TEXCOORD1 ) : COLOR
{
	//输出颜色
	float DepthP = posP.z / posP.w;
	float4 Color = float4(posV.xyz, DepthP);
	return Color;// float4(1.0f, 0.0f, 0.0f, 1.0f);
}

technique DeferredPosition
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}