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



float levelColor(float inputColor)
{	
	float level0 = 0.0f;
	float level1 = 0.2f;
	float level2 = 0.4f;
	float level3 = 0.6f;
	float level4 = 0.8f;
	float level5 = 1.0f;
	
	if(inputColor < level1)
	{
		inputColor = level0;
	}
	if(inputColor > level1 && inputColor < level2)
	{
		inputColor = level1;
	}
	if(inputColor > level2 && inputColor < level3)
	{
		inputColor = level2;
	}
	if(inputColor > level3 && inputColor < level4)
	{
		inputColor = level3;
	}	
	if(inputColor > level4 && inputColor < level5)
	{
		inputColor = level4;
	}
	return inputColor;
}

float levelColor_4Level(float inputColor)
{	
	float level0 = 0.0f;
	float level1 = 0.25f;
	float level2 = 0.5f;
	float level3 = 0.75f;
	float level4 = 1.0f;
	
	if(inputColor < level1)
	{
		inputColor = level0;
	}
	if(inputColor > level1 && inputColor < level2)
	{
		inputColor = level1;
	}
	if(inputColor > level2 && inputColor < level3)
	{
		inputColor = level2;
	}
	if(inputColor > level3 && inputColor < level4)
	{
		inputColor = level3;
	}
	
	return inputColor;
}

float4 PShader(float2 TexCoord : TEXCOORD0) : COLOR
{
	float4 color = tex2D(g_sampleMainColor, TexCoord);

	//反転
	if(false)
	{
		color.r = 1 - color.r;
		color.g = 1 - color.g;
		color.b = 1 - color.b;
	}
	
	//階段化-5level
	if(false)
	{
		color.r = levelColor(color.r);
		color.g = levelColor(color.g);
		color.b = levelColor(color.b);
	}
	
	//階段化-4level
	{
		color.r = levelColor_4Level(color.r);
		color.g = levelColor_4Level(color.g);
		color.b = levelColor_4Level(color.b);
	}
	
	return color;
}

technique ColorChange
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}