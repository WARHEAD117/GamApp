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
	AddressU = Wrap;
	AddressV = Wrap;
};

texture		g_NormalMap;
sampler2D g_sampleNormalMap =
sampler_state
{
	Texture = <g_NormalMap>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU = Wrap;
	AddressV = Wrap;
};

texture		g_SpecularMap;
sampler2D g_sampleSpecularMap =
sampler_state
{
	Texture = <g_SpecularMap>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct OutputVS
{
	float4 posWVP			: POSITION;
	float3 normalV			: NORMAL;
	float4 tangentL			: TANGENT;
	float3 binormalL		: BINORMAL;
	float3 normalL			: TEXCOORD3;

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
				float4 tangentL : TANGENT,
				float3 binormalL : BINORMAL,
				float2 TexCoord : TEXCOORD0)
{
	OutputVS outVS = (OutputVS)0;

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	outVS.posWVP = mul(posL, g_WorldViewProj);
	

	//观察空间下的法线
	outVS.normalV = mul(normalL, g_WorldView);

	//观察空间下的副法线
	outVS.normalL = normalL;
	//观察空间下的切线
	outVS.tangentL = tangentL;
	//观察空间下的副法线
	outVS.binormalL = binormalL;

	outVS.posP = outVS.posWVP;
	outVS.posV = mul(posL, g_WorldView);
	outVS.TexCoord = TexCoord;

	return outVS;
}

OutputPS PShader(float3 NormalV		: NORMAL,
				float4 TangentL		: TANGENT,
				float3 BinormalL	: BINORMAL,
				float3 NormalL		: TEXCOORD3,
				float2 TexCoord		: TEXCOORD0,
				float4 posP			: TEXCOORD1,
				float4 posV			: TEXCOORD2)
{
	OutputPS PsOut;

	NormalV = normalize(NormalV);
	NormalL = normalize(NormalL);

	BinormalL = normalize(BinormalL);

	float3 TangentL3 = TangentL.xyz;
	
	//float tW = TangentL.w;
	//TangentL3 = normalize(TangentL3);
	//float3 BinormalL3 = cross(NormalL, TangentL3) *tW;
	//BinormalL3 = normalize(BinormalL3);

	float3x3 TBN = float3x3(TangentL3, BinormalL, NormalL);
	float3x3 toTangentSpace = transpose(TBN);

	float3 sampledNormalT = tex2D(g_sampleNormalMap, TexCoord).rgb;
	sampledNormalT = 2.0f * sampledNormalT - 1.0f;
	sampledNormalT = normalize(sampledNormalT);

	float3 sampledNormalV = sampledNormalT.x * TBN[0] + sampledNormalT.y * TBN[1] + sampledNormalT.z * TBN[2];

	sampledNormalV = mul(sampledNormalV, g_WorldView);
	sampledNormalV = normalize(sampledNormalV);
	sampledNormalV = (sampledNormalV + 1.0f) / 2.0f;

	//纹理采样
	float4 Texture = tex2D(g_sampleTexture, TexCoord);

	//投影后的非线性深度
	float DepthP = posP.z / posP.w;

	PsOut.diffuse = Texture;
	PsOut.normal = float4(sampledNormalV, 1.0f);
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