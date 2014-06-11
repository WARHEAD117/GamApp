matrix		g_World;
matrix		g_View;
matrix		g_Proj;
matrix		g_ViewProj;
matrix		g_WorldViewProj;
matrix		g_mWorldInv;

float		g_zNear = 1.0f;
float		g_zFar = 1000.0f;

float4		g_ViewPos;

struct OutputVS
{
	float4 posWVP			: POSITION;
	float4 posView			: TEXCOORD0;
};


OutputVS VShader(float4 posL		: POSITION0)
{
	OutputVS outVS = (OutputVS)0;

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	outVS.posWVP = mul(posL, g_WorldViewProj);
	float4 pos = mul(posL, g_World);
	outVS.posView = mul(pos, g_View);

	return outVS;
}


float4 PShader(float4 posView : TEXCOORD0) : COLOR
{
	//输出颜色
	return posView;// float4(1.0f, 0.0f, 0.0f, 1.0f);
}

technique DeferredPosition
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}