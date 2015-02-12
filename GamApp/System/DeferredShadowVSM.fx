#include "common.fx"

float4		g_ViewPos;

bool		g_IsPointLight;

struct OutputVS
{
	float4 posWVP			: POSITION;
	float3 posWV			: TEXCOORD0;
};


OutputVS VShader(float4 posL		: POSITION0)
{
	OutputVS outVS = (OutputVS)0;

	//��������Ķ���λ�ã��������硢�۲졢ͶӰ����任��
	outVS.posWVP = mul(posL, g_WorldViewProj);
	outVS.posWV = mul(posL, g_WorldView);

	return outVS;
}

float2 ComputeMoments(float Depth)
{
	float2 Moments;

	Moments.x = Depth;

	float dx = ddx(Depth);
	float dy = ddy(Depth);

	Moments.y = Depth*Depth + 0.25 * (dx*dx + dy*dy);

	return Moments;
}

float4 PShader(float3 posWV : TEXCOORD0) : COLOR
{
	//�����ɫ
	float4 Color = posWV.z;
	if (g_IsPointLight)
	{
		Color.x = length(posWV);
	}
	float2 monents = ComputeMoments(Color.x);
	Color = float4(monents, 0.0f, 1.0f);

	return Color;// float4(1.0f, 0.0f, 0.0f, 1.0f);
}

technique DeferredShadowVSM
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}