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

texture		g_InkTex;
sampler2D g_sampleInkTex =
sampler_state
{
	Texture = <g_InkTex>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
	AddressU = Border;
	AddressV = Border;
	bordercolor = float4(1,1,1,1);
};

int minI = 0;
int maxI = 70;

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

float4 PShaderBlur(float2 TexCoord : TEXCOORD0) : COLOR
{
	return tex2D(g_sampleGrayscale, TexCoord);

	float4 color = GaussianBlur(g_ScreenWidth, g_ScreenHeight, g_sampleGrayscale, TexCoord);

	return color;
}

float4 Normal_Interval(float2 TexCoord)
{
	float3 normal = normalize(GetNormal(TexCoord, g_sampleNormal));

	float3 view = float3(0, 0, 1);

	float nv = dot(normal, -view);
	if (nv > 0 && nv < 0.45f)
		return float4(0, 0, 0, 0);
	else
		return float4(1, 1, 1, 1);
}

float4 Normal_Gray(float2 TexCoord)
{
	float depth = GetDepth(TexCoord, g_samplePosition);

	if (depth > 1000)
	{
		//normal = float3(1, 1, 1);
		return float4(1, 1, 1, 1);
	}

	float3 normal = GetNormal(TexCoord, g_sampleNormal);

	normal = normalize(normal);
		//normal = GetUnsharpMaskedNormal(TexCoord, g_sampleNormal);
	
	
	float3 light = float3(0, -0.3, 1);
	light = normalize(light);

	float nl = dot(normal, -light);
	
	//nl *= nl;
	nl *= 0.9;
	
	int color_int = nl * 255;
	if (color_int < maxI && color_int >= minI)
		return float4(nl, nl, nl, 1.0f);
	else if (color_int < minI)
		return float4(minI / 255.0f, minI / 255.0f, minI / 255.0f, 1.0f);
	else
		return float4(1, 1, 1, 1);

	return float4(nl, nl, nl, 1.0f);
}

float4 PShader(float2 TexCoord : TEXCOORD0) : COLOR
{
	float color = tex2D(g_sampleGrayscale, TexCoord).r;
	//if (color < 0.9f)
	//	return float4(color, color, color, 1.0f);
	//else
	//	return float4(1, 1, 1, 1.0f);
	
	int color_int = color * 255;
	if (color_int < maxI && color_int >= minI)
		return float4(color, color, color, 1.0f);
	else if (color_int < minI)
		return float4(minI / 255.0f, minI / 255.0f, minI / 255.0f, 1.0f);
	else
		return float4(1, 1, 1, 1);
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
	return tex2D(g_sampleMainColor, TexCoord);// *tex2D(g_sampleMainColor, TexCoord)*tex2D(g_sampleMainColor, TexCoord);
	//return tex2D(g_sampleGrayscale, TexCoord);
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
	float blendedColor = color3;
	//if (color4 <= 0.99)
	//	blendedColor = GetOverlap(color4, color5);

	//if (color3 <= 0.99)
	//	blendedColor = GetOverlap(color3, blendedColor);
	if (color2 <= 0.99)
		blendedColor = GetOverlap(color2, blendedColor);
	if (color1 <= 0.99)
		blendedColor = GetOverlap(color1, blendedColor);
		
	return float4(blendedColor, blendedColor, blendedColor, 1);
	
	float color = color1 * color2 * color3 * color4 *color5;

	float4 outcolor = float4(color, color, color, 1.0f);
	return outcolor;
}

float4 PShaderDiffusion(float2 TexCoord : TEXCOORD0) : COLOR
{
	//return float4(1, 0, 0, 1);
	float color = tex2D(g_sampleDiffusionSource, TexCoord).r;
	float avg = color;
	float outColor = color;

	if (color > 0.99)
	{
		//return float4(1, 0, 0, 1);
		float colorL = tex2D(g_sampleDiffusionSource, TexCoord + float2(-1, 0)*float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight)).r;
		float colorR = tex2D(g_sampleDiffusionSource, TexCoord + float2(1, 0)*float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight)).r;
		float colorU = tex2D(g_sampleDiffusionSource, TexCoord + float2(0, -1)*float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight)).r;
		float colorD = tex2D(g_sampleDiffusionSource, TexCoord + float2(0, 1)*float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight)).r;

		float colorLU = tex2D(g_sampleDiffusionSource, TexCoord + float2(-1, -1)*float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight)).r;
		float colorRU = tex2D(g_sampleDiffusionSource, TexCoord + float2(1, -1)*float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight)).r;
		float colorLD = tex2D(g_sampleDiffusionSource, TexCoord + float2(-1, 1)*float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight)).r;
		float colorRD = tex2D(g_sampleDiffusionSource, TexCoord + float2(1, 1)*float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight)).r;

		int InkQ_L = 255 - colorL * 255;
		int InkQ_R = 255 - colorR * 255;
		int InkQ_U = 255 - colorU * 255;
		int InkQ_D = 255 - colorD * 255;

		int InkQ_LU = 255 - colorLU * 255;
		int InkQ_RU = 255 - colorRU * 255;
		int InkQ_LD = 255 - colorLD * 255;
		int InkQ_RD = 255 - colorRD * 255;

		int sum = 0;
		int count = 0;
		
		if (InkQ_L > 0)
		{
			sum += InkQ_L; count++;
		}
		if (InkQ_R > 0)
		{
			sum += InkQ_R; count++;
		}
		if (InkQ_U > 0)
		{
			sum += InkQ_U; count++;
		}
		if (InkQ_D > 0)
		{
			sum += InkQ_D; count++;
		}
		/*
		if (InkQ_LU > 0)
		{
			sum += InkQ_LU; count++;
		}
		if (InkQ_RU > 0)
		{
			sum += InkQ_RU; count++;
		}
		if (InkQ_LD > 0)
		{
			sum += InkQ_LD; count++;
		}
		if (InkQ_RD > 0)
		{
			sum += InkQ_RD; count++;
		}
		*/
		if (count > 0)
		{
			//return float4(1, 0, 0, 1);
			avg = sum / count;
			float random = frac(TexCoord.x * count + TexCoord.y * avg + 1.17 * 1.86 * avg / 255) * 0.2f;
			random = 0;
			avg *= (0.98f - random);
			outColor = 1 - avg / 255.0f;
		}
	}
	//return float4(1, 0, 0, 1);
	return float4(outColor, outColor, outColor, 1.0f);
}

float4 PShaderEdgeBlur(float2 TexCoord : TEXCOORD0) : COLOR
{
	//return tex2D(g_sampleMainColor, TexCoord);
	//return float4(1, 0, 0, 1);
	float color = tex2D(g_sampleMainColor, TexCoord).r;
	if (color >= 0.99)
		return float4(color, color, color, 1);

	float2 offset = float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight);
	float colorL = tex2D(g_sampleMainColor, TexCoord + float2(-1, 0) * offset).r;
	float colorR = tex2D(g_sampleMainColor, TexCoord + float2(1, 0) * offset).r;
	float colorU = tex2D(g_sampleMainColor, TexCoord + float2(0, -1) * offset).r;
	float colorD = tex2D(g_sampleMainColor, TexCoord + float2(0, 1) * offset).r;
	float colorLU = tex2D(g_sampleMainColor, TexCoord + float2(-1, -1) * offset).r;
	float colorRU = tex2D(g_sampleMainColor, TexCoord + float2(1, -1) * offset).r;
	float colorLD = tex2D(g_sampleMainColor, TexCoord + float2(-1, 1) * offset).r;
	float colorRD = tex2D(g_sampleMainColor, TexCoord + float2(1, 1) * offset).r;

	float colorList[9] = { color, colorL, colorR, colorU, colorD, colorLU, colorRU, colorLD, colorRD };

	int count = 0;
	float sum = 0;
	for (int i = 0; i < 9; i++)
	{
		if (colorList[i] < 0.99)
		{
			count++;
			sum += colorList[i];
		}
	}
	if (count == 0)
		return float4(1, 1, 1, 1);
	float finalColor = 1.0f * sum / count;
	return float4(finalColor, finalColor, finalColor, 1.0f);

	return tex2D(g_sampleMainColor, TexCoord);
}

struct P_OutVS
{
	float4 posWVP         : POSITION0;
	float2 TexCoord		  : TEXCOORD0;
	float  psize		  : PSIZE;
	float4 color		  : COLOR0;
};

P_OutVS VShaderParticle(float4 posL       : POSITION0,
	float2 TexCoord : TEXCOORD0)
{
	P_OutVS outVS = (P_OutVS)0;

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	//outVS.posWVP = mul(float4(0.5f, 0.5f, 0, 1), g_Proj);
	//outVS.posWVP = float4(2*TexCoord.x-1, 1-2*TexCoord.y, 0, 1);
	outVS.posWVP = float4( - 2, -2, -2, 1);
	//outVS.TexCoord = TexCoord;
	
	outVS.psize = 0;
	float4 color = tex2Dlod(g_sampleMainColor, float4(TexCoord.x, TexCoord.y, 0, 0));
	if (color.r < 0.99f)
	{
		outVS.psize = 22 * (1 - color.r);
		outVS.posWVP = float4(2 * TexCoord.x - 1, 1 - 2 * TexCoord.y, 0, 1);
	}
	
	outVS.color = color;
	outVS.color = float4(TexCoord, 0, 0);
	return outVS;
}

float4 PShaderParticle(float2 TexCoord : TEXCOORD0,
	float4 color : COLOR0) : COLOR
{ 
	float3 normal = normalize(GetNormal(g_sampleNormal, color.xy));
	//return float4(normal, 0.0f);
	//float3 normal3 = normalize(normal.xyz);

	float cosA = normal.x;
	float A = 3.141592653 - acos(cosA);
	if (normal.y < 0)
		A = 3.141592653 + acos(cosA);
	//return normal;

	float2x2 rotationM = float2x2(float2(cos(A), -sin(A)), float2(sin(A), cos(A)));
	TexCoord = mul(TexCoord - float2(0.5,0.5), rotationM) + float2(0.5,0.5);
	//TexCoord = saturate(TexCoord);
	float4 brush = tex2D(g_sampleInkTex, TexCoord);
		//brush.rgb = float3(0.5, 0.5, 0.5);
	brush.a = 1;

	if (brush.r > 0.59f)
	{
		brush = float4(1.0f, 1.0f, 1.0f, 0.0f);
	}
	else
		brush = float4(0.3f, 0.3f, 0.3f, 0.5f);
	//brush.a = brush.r;


	if (TexCoord.x < 0 || TexCoord.x > 1 || TexCoord.y < 0 || TexCoord.y > 1)
		brush = float4(1.0f, 1.0f, 1.0f, 0.0f);

	if (brush.r > 0.99f)
		brush = float4(1.0f, 1.0f, 1.0f, 0.0f);
	
	//brush = float4(0.1,0.1,0.1,0.5f);
	return brush;
	//return float4(1, 0, 0, 1);
	//return tex2D(g_sampleMainColor, TexCoord);
}

technique ColorChange
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShaderGrayscale();
	}
	
	pass p1
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}

	pass p2
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShaderBlend();
	}

	pass p3
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShaderBlur();
	}

	pass p4
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShaderDiffusion();
	}

	pass p5
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShaderEdgeBlur();

		AlphaBlendEnable = false;
	}

	pass p6
	{
		vertexShader = compile vs_3_0 VShaderParticle();
		pixelShader = compile ps_3_0 PShaderParticle();

		AlphaBlendEnable = true;
		SrcBlend = SRCALPHA;
		DestBlend = INVSRCALPHA;
	}

}