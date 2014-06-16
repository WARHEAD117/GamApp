matrix		g_World;
matrix		g_View;
matrix		g_Proj;
matrix		g_WorldView;
matrix		g_ViewProj;
matrix		g_WorldViewProj;
matrix		g_mWorldInv;

float		g_zNear = 1.0f;
float		g_zFar = 1000.0f;

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

technique DeferredNormalDepth
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}