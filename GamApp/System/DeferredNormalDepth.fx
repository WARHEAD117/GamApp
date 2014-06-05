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
	float3 normalW			: NORMAL0;
	float4 toEyeDirW		: TEXCOORD0;
	float Depth		: TEXCOORD1;
};


OutputVS VShader(float4 posL		: POSITION0,
				 float3 normalL		: NORMAL0)  // Assumed to be unit length)
{
	OutputVS outVS = (OutputVS)0;

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	outVS.posWVP = mul(posL, g_WorldViewProj);
	float4 pos = mul(posL, g_World);
	pos = mul(posL, g_View);
	outVS.Depth = pos.z / (g_zFar - g_zNear);

	outVS.normalW = mul(normalL, g_World);

	outVS.toEyeDirW = g_ViewPos - mul(posL, g_World);
	return outVS;
}


float4 PShader(float3 NormalW			: NORMAL0,
				float4 ToEyeDirW		: TEXCOORD0,
				float Depth : TEXCOORD1) : COLOR
{
	NormalW = normalize(NormalW);
	ToEyeDirW = normalize(ToEyeDirW);

	float4 finalColor = float4(NormalW, Depth);
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