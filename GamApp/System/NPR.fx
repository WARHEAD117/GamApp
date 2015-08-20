#include "common.fx"

texture		g_MainColorBuffer;
sampler2D g_sampleMainColor =
sampler_state
{
	Texture = <g_MainColorBuffer>;
	MinFilter = point;
	MagFilter = point;
	MipFilter = point;
};

texture		g_NormalBuffer;
sampler2D g_sampleNormal =
sampler_state
{
	Texture = <g_NormalBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

texture		g_edgeBuffer;
sampler2D g_sampleEdge =
sampler_state
{
	Texture = <g_edgeBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

struct OutputVS
{
	float4 posWVP         : POSITION0;
	float2 TexCoord		: TEXCOORD0;
};


OutputVS VShader(float4 posL       : POSITION0,
				 float2 TexCoord : TEXCOORD0)
{
	OutputVS outVS = (OutputVS)0;

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	outVS.posWVP = mul(posL, g_WorldViewProj);

	outVS.TexCoord = TexCoord;

	return outVS;
}

float4 PShader(float2 TexCoord : TEXCOORD0) : COLOR
{
	float4 color = tex2D(g_sampleMainColor, TexCoord);

	float3 normal = normalize(GetNormal(TexCoord, g_sampleNormal));

	float3 lightVec = mul(float3(0, 1, 0), g_View);
	float result = dot(normal, normalize(lightVec));

	float outColor = 0;
	if (result < 0)
		outColor = 0.1f;
	if (result >= 0 && result < 0.3f)
		outColor = 0.3f;
	if (result >= 0.3 && result < 0.5f)
		outColor = 0.6f;
	if (result >= 0.5 && result < 1.0f)
		outColor = 0.9f;

	float4 edgeColor = tex2D(g_sampleEdge, TexCoord);
		edgeColor = GaussianBlur(g_ScreenWidth, g_ScreenHeight, g_sampleEdge, TexCoord);
		//return edgeColor;
	return edgeColor * outColor;
	if (edgeColor.r > 0.5f)
		return float4(outColor, outColor, outColor, 1.0f);
	else
		return float4(0, 0, 0, 0);
}

technique ColorChange
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}