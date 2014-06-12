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
	float3 toEyeDirW		: TEXCOORD0;
	float3 posWV			: TEXCOORD1;
};


OutputVS VShader(float4 posL		: POSITION0,
				 float3 normalL		: NORMAL0)  // Assumed to be unit length)
{
	OutputVS outVS = (OutputVS)0;

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	outVS.posWVP = mul(posL, g_WorldViewProj);
	float4 pos = mul(posL, g_World);
	pos = mul(pos, g_View);
	outVS.posWV = pos;

	float3 normalWorld = mul(normalL, g_World);
	outVS.normalW = mul(normalWorld, g_View);

	//outVS.normalW = mul(normalL, g_WorldViewProj);

	outVS.toEyeDirW = g_ViewPos.xyz - mul(posL, g_World).xyz;
	return outVS;
}


float4 PShader(float3 NormalW			: NORMAL0,
				float3 ToEyeDirW		: TEXCOORD0,
				float3 posWV : TEXCOORD1) : COLOR
{
	NormalW = normalize(NormalW);
	NormalW = (NormalW + float3(1.0f, 1.0f, 1.0f)) / 2;
	ToEyeDirW = normalize(ToEyeDirW);

	float Depth = posWV.z / g_zFar;
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