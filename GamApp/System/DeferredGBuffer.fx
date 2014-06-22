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
	float3 normalV			: NORMAL;
	float2 TexCoord			: TEXCOORD0;
	float4 posP				: TEXCOORD1;
	float4 posV				: TEXCOORD2;
};


struct OutputPS
{
	float4 diffuse			: COLOR0;
	float4 normal			: COLOR1;
	float4 position			: COLOR2;
};

OutputVS VShader(float4 posL		: POSITION,
				float3 normalL : NORMAL,
				float2 TexCoord : TEXCOORD0)
{
	OutputVS outVS = (OutputVS)0;

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	outVS.posWVP = mul(posL, g_WorldViewProj);
	

	//观察空间下的法线
	outVS.normalV = mul(normalL, g_WorldView);

	outVS.posP = outVS.posWVP;
	outVS.posV = mul(posL, g_WorldView);
	outVS.TexCoord = TexCoord;

	return outVS;
}

OutputPS PShader(float3 NormalV		: NORMAL,
				float2 TexCoord		: TEXCOORD0,
				float4 posP			: TEXCOORD1,
				float4 posV		: TEXCOORD2)
{
	OutputPS PsOut;

	NormalV = (normalize(NormalV) + float3(1.0f, 1.0f, 1.0f)) / 2;

	//纹理采样
	float4 Texture = tex2D(g_sampleTexture, TexCoord);

	//投影后的非线性深度
	float DepthP = posP.z / posP.w;

	PsOut.diffuse = Texture;
	PsOut.normal = float4(NormalV, 1.0f);
	PsOut.position = float4(posV.xyz, DepthP);
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