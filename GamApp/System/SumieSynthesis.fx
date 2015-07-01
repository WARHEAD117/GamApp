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

texture		g_GrayscaleBuffer;
sampler2D g_sampleGrayscale =
sampler_state
{
	Texture = <g_GrayscaleBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

texture		g_DiffusionSource;
sampler2D g_sampleDiffusionSource =
sampler_state
{
	Texture = <g_DiffusionSource>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;

};
texture		SA1;
sampler2D g_sampleSA1 =
sampler_state
{
	Texture = <SA1>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};
texture		SA2;
sampler2D g_sampleSA2 =
sampler_state
{
	Texture = <SA2>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};
texture		SA3;
sampler2D g_sampleSA3 =
sampler_state
{
	Texture = <SA3>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};
texture		SA4;
sampler2D g_sampleSA4 =
sampler_state
{
	Texture = <SA4>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};
texture		SA5;
sampler2D g_sampleSA5 =
sampler_state
{
	Texture = <SA5>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

texture		g_InkCloud;
sampler2D g_sampleInkCloud =
sampler_state
{
	Texture = <g_InkCloud>;
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

int minI = 0;
int maxI = 70;

int g_baseTexSize = 32;
int g_maxTexSize = 20;


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


float4 fliter(float3x3 _filter, sampler2D _image, float2 texCoord, float2 texSize)
{
	float2 filterOffset[3][3] = 
	{
		{ float2(-1.0f, -1.0f), float2( 0.0f, -1.0f), float2( 1.0f, -1.0f) },
		{ float2(-1.0f,  0.0f), float2( 0.0f,  0.0f), float2( 1.0f,  0.0f) },
		{ float2(-1.0f,  1.0f), float2( 0.0f,  1.0f), float2( 1.0f,  1.0f) }
	};

	float4 finalColor = float4(0, 0, 0, 0);

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			float2 newTexCoord = texCoord + float2(filterOffset[i][j].x / texSize.x, filterOffset[i][j].y / texSize.y);
				finalColor += tex2D(_image, newTexCoord) * _filter[i][j];
		}
	}

	return finalColor;
}

float4 PShaderGrayscale(float2 TexCoord : TEXCOORD0) : COLOR
{
	float4 color = tex2D(g_sampleMainColor, TexCoord);

	float depth = GetDepth(TexCoord, g_samplePosition);

	if (depth > 1000)
	{
		//normal = float3(1, 1, 1);
		return float4(1, 1, 1, 1);
	}

	float3 normal = GetNormal(TexCoord, g_sampleNormal);

	normal = normalize(normal);

	float3 light = float3(0, -0.1, 1);
	light = normalize(light);

	float nl = dot(normal, -light);

	nl = pow(nl, 1.5);
	//nl *= 0.9;

	return float4(nl, nl, nl, 1.0f);
}

float GetOverlap(float dark, float light)
{
	float tempLight = max(dark, light);
	float tempDark = min(dark, light);
	dark = tempDark;
	light = tempLight;
	int d_stroke = (light - dark) * 255;
	int d_overlap = (
		40 * pow(d_stroke / 100, 3) -
		60 * pow(d_stroke / 100, 2) + 20
		) * (1 - light);
	float overlap = dark - d_overlap / 255.0f;
	return overlap;
}

float4 PShaderBlend(float2 TexCoord : TEXCOORD0) : COLOR
{	
	return float4(1, 1, 1, 1);
	//return tex2D(g_sampleMainColor, TexCoord);// *tex2D(g_sampleMainColor, TexCoord)*tex2D(g_sampleMainColor, TexCoord);

	//return tex2D(g_sampleGrayscale, TexCoord);
	float2 cloudTc = float2(g_ScreenWidth, g_ScreenHeight) * TexCoord / float2(256, 256);
	float cloud = tex2D(g_sampleInkCloud, cloudTc).r;
	float outC = tex2D(g_sampleSA1, TexCoord).r;
	outC = clamp(outC, 0.1, 1);
	//return float4(outC, outC, outC, 1) * 8 * cloud;
	//return tex2D(g_sampleSA1, TexCoord);
	//return tex2D(g_sampleSA2, TexCoord);
	//return tex2D(g_sampleSA3, TexCoord);
	//return tex2D(g_sampleSA4, TexCoord);
	//return tex2D(g_sampleSA5, TexCoord);

	float color1 = tex2D(g_sampleSA1, TexCoord).r;
		//color1 = GaussianBlur(g_ScreenWidth, g_ScreenHeight, g_sampleSA1, TexCoord).r;
	float color2 = tex2D(g_sampleSA2, TexCoord).r;
		//color2 = GaussianBlur(g_ScreenWidth, g_ScreenHeight, g_sampleSA2, TexCoord).r;
	float color3 = tex2D(g_sampleSA3, TexCoord).r;
		//color3 = GaussianBlur(g_ScreenWidth, g_ScreenHeight, g_sampleSA3, TexCoord).r;
	float color4 = tex2D(g_sampleSA4, TexCoord).r;
		//color4 = GaussianBlur(g_ScreenWidth, g_ScreenHeight, g_sampleSA4, TexCoord).r;
	float color5 = tex2D(g_sampleSA5, TexCoord).r;
		//color5 = GaussianBlur(g_ScreenWidth, g_ScreenHeight, g_sampleSA5, TexCoord).r;
	//float4 color = tex2D(g_sampleMainColor, TexCoord);

	//return float4(color1, color1, color1, 1);
	//if (color1 >= 0.99)
	//	return float4(1, 0, 0, 1);
	//else
	//	return float4(0, 1, 0, 1);
	float blendedColor = color2;
	//if (color4 <= 0.99)
	//	blendedColor = GetOverlap(color4, color5);

	//if (color3 <= 0.99)
	//	blendedColor = GetOverlap(color3, blendedColor);
	//if (color2 <= 0.99)
	//	blendedColor = GetOverlap(color2, blendedColor);
	//if (color1 <= 0.99)
	//	blendedColor = GetOverlap(color1, blendedColor);
		
	return float4(blendedColor, blendedColor, blendedColor, 1);
	
	float color = color1 * color2 * color3 * color4 *color5;

	float4 outcolor = float4(color, color, color, 1.0f);
	return outcolor;
}

float4 PShaderSynthesis(float2 TexCoord : TEXCOORD0) : COLOR
{
	float4 colorContour = tex2D(g_sampleContour, TexCoord);
	float4 colorInside = tex2D(g_sampleInside, TexCoord);
	colorInside = GaussianBlur(g_ScreenWidth, g_ScreenHeight, g_sampleInside, TexCoord);

	float4 Inside = colorInside * colorInside.a + float4(1, 1, 1, 1) * (1 - colorInside.a);

	float alpha = colorContour.r;
	if (alpha < 0.9)
		alpha /= 2;

	float4 InsideAndContour = Inside * alpha + colorContour *(1 - alpha);
		return InsideAndContour;
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

	pass p2
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShaderBlur();
	}
}