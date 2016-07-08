#include "common.fx"

float4		g_ViewPos;

struct OutputVS
{
	float4 posWVP			: POSITION;
	float3 normalWV			: NORMAL;
	float4 posP				: TEXCOORD0;
};


OutputVS VShader(float4 posL		: POSITION,
				 float3 normalL		: NORMAL)  // Assumed to be unit length)
{
	OutputVS outVS = (OutputVS)0;

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	outVS.posWVP = mul(posL, g_WorldViewProj);
	//outVS.posP = mul(posL, g_WorldView);
	outVS.posP = outVS.posWVP;

	//观察空间下的法线
	outVS.normalWV = mul(normalL, g_WorldView);

	return outVS;
}


float4 PShader(float3 NormalWV			: NORMAL,
				float4 posP				: TEXCOORD0) : COLOR
{
	NormalWV = normalize(NormalWV);
	NormalWV = (NormalWV + float3(1.0f, 1.0f, 1.0f)) / 2;

	//投影后的非线性深度
	float Depth = posP.z / posP.w;

	float4 finalColor = float4(NormalWV, Depth);
	//输出颜色
	return finalColor;// float4(1.0f, 0.0f, 0.0f, 1.0f);
}

technique DeferredNormal
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}