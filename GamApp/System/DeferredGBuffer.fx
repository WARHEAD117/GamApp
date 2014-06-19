matrix		g_World;
matrix		g_View;
matrix		g_Proj;
matrix		g_ViewProj;
matrix		g_WorldView;
matrix		g_WorldViewProj;
matrix		g_mWorldInv;

texture		g_Texture;


sampler2D g_sampleTexture =
sampler_state
{
	Texture = <g_Texture>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
};

struct OutputVS
{
	float4 posWVP			: POSITION;
	float3 normalWV			: NORMAL;
	float2 TexCoord			: TEXCOORD0;
	float4 posP				: TEXCOORD1;
};


struct OutputPS
{
	float4 diffuse			: COLOR0;
	float4 normalDepth		: COLOR1;
};

OutputVS VShader(float4 posL		: POSITION,
				float3 normalL : NORMAL,
				float2 TexCoord : TEXCOORD0)
{
	OutputVS outVS = (OutputVS)0;

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	outVS.posWVP = mul(posL, g_WorldViewProj);
	outVS.posP = outVS.posWVP;

	//观察空间下的法线
	outVS.normalWV = mul(normalL, g_WorldView);
	outVS.TexCoord = TexCoord;

	return outVS;
}

OutputPS PShader(float3 NormalWV		: NORMAL,
				float2 TexCoord		: TEXCOORD0,
				float4 posP			: TEXCOORD1)
{
	OutputPS PsOut;

	NormalWV = normalize(NormalWV);
	NormalWV = (NormalWV + float3(1.0f, 1.0f, 1.0f)) / 2;

	//纹理采样
	float4 Texture = tex2D(g_sampleTexture, TexCoord);

	//投影后的非线性深度
	float Depth = posP.z / posP.w;
	float4 normalDepth = float4(NormalWV, Depth);

	PsOut.diffuse = Texture;
	PsOut.normalDepth = normalDepth;

	return PsOut;
}

technique DeferredGBuffer
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}