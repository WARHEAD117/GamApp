#include "common.fx"

static const float3 LUMINANCE_VECTOR = float3(0.2125f, 0.7154f, 0.0721f);

texture		g_MainColorBuffer;
sampler2D g_sampleMainColor =
sampler_state
{
	Texture = <g_MainColorBuffer>;
	MinFilter = linear;
	MagFilter = linear;
	MipFilter = linear;
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

float getLum(float3 color)
{
	return dot(color, LUMINANCE_VECTOR);
}

int BayerPatton[4][4] = {{ 0, 8, 2, 10 }, {12,4,14,6}, {3,11,1,9}, {15,7,13,5}};

float4 PShader(float2 TexCoord : TEXCOORD0) : COLOR
{
	int indexX = fmod(floor(TexCoord.x * g_ScreenWidth), 4);
	int indexY = fmod(floor(TexCoord.y * g_ScreenHeight), 4);

	float4 color = tex2D(g_sampleMainColor, TexCoord);
	float lum = getLum(color.xyz);
	int resampledColor = lum / 1 * 255;
	bool isWhite = resampledColor > BayerPatton[indexX][indexY] * 16 + 8;
	if (isWhite)
		return float4(1, 1, 1, 0);
	else
		return float4(0, 0, 0, 0);
}

technique Dither_Halftoning
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}