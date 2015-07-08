#include "common.fx"

texture		g_NormalBuffer;
sampler2D g_sampleNormal =
sampler_state
{
	Texture = <g_NormalBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};


texture		g_PositionBuffer;
sampler2D g_samplePosition =
sampler_state
{
	Texture = <g_PositionBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

texture		g_MainColorBuffer;
sampler2D g_sampleMainColor =
sampler_state
{
	Texture = <g_MainColorBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

texture		g_Contour;
sampler2D g_sampleContour =
sampler_state
{
	Texture = <g_Contour>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

texture		g_Inside;
sampler2D g_sampleInside =
sampler_state
{
	Texture = <g_Inside>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

texture		g_BlurredInside;
sampler2D g_sampleBlurredInside =
sampler_state
{
	Texture = <g_BlurredInside>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};


texture		g_Background;
sampler2D g_sampleBackground =
sampler_state
{
	Texture = <g_Background>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

float g_AlphaFactor = 0.8f;

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

	tex2D(g_sampleNormal, float2(0, 0));
	return outVS;
}

float4 PShaderSynthesis(float2 TexCoord : TEXCOORD0) : COLOR
{
	float4 bgColor = tex2D(g_sampleBackground, TexCoord);

	float4 colorInside = tex2D(g_sampleInside, TexCoord);
	colorInside = GaussianBlur(g_ScreenWidth, g_ScreenHeight, g_sampleInside, TexCoord);

	float alphaFactor = g_AlphaFactor;
	float insideAlpha = colorInside.a * alphaFactor;

	//bgColor = float4(1, 1, 1, 1);
	float4 Inside = colorInside * insideAlpha + bgColor * (1 - insideAlpha);

	return Inside;
}

float4 PShaderBlur(float2 TexCoord : TEXCOORD0) : COLOR
{
	float4 colorInside = tex2D(g_sampleInside, TexCoord); 
	colorInside = GaussianBlur(g_ScreenWidth, g_ScreenHeight, g_sampleInside, TexCoord);
	
	return colorInside;
}

technique SumieSynthesis
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShaderSynthesis();
	}

	pass p1
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShaderBlur();
	}
}