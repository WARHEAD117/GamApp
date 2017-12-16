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

texture		g_DiffuseBuffer;
sampler2D g_sampleDiffuse =
sampler_state
{
	Texture = <g_DiffuseBuffer>;
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

texture		g_Inside2;
sampler2D g_sampleInside2 =
sampler_state
{
	Texture = <g_Inside2>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

texture		g_InputTex;
sampler2D g_sampleInput =
sampler_state
{
	Texture = <g_InputTex>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

texture		g_Bloomed;
sampler2D g_sampleBloomed =
sampler_state
{
	Texture = <g_Bloomed>;
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

texture		g_Src;
sampler2D g_sampleSrc =
sampler_state
{
	Texture = <g_Src>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

texture		g_Src2;
sampler2D g_sampleSrc2 =
sampler_state
{
	Texture = <g_Src2>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};
texture		g_Src3;
sampler2D g_sampleSrc3 =
sampler_state
{
	Texture = <g_Src3>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};
texture		g_Src4;
sampler2D g_sampleSrc4 =
sampler_state
{
	Texture = <g_Src4>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};
texture		g_Src5;
sampler2D g_sampleSrc5 =
sampler_state
{
	Texture = <g_Src5>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

texture		g_FP_LF;
sampler2D g_sample_FP_LF =
sampler_state
{
	Texture = <g_FP_LF>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

float g_AlphaFactor = 0.8f;

matrix g_LastView;

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

float GetOverlap(float dark, float light)
{
	int d_stroke = (light - dark) * 255;
	int d_overlap = (
		40 * pow(d_stroke / 100, 3) -
		60 * pow(d_stroke / 100, 2) + 20
		) * (1 - light);
	float overlap = d_overlap / 255.0f;
	return overlap;
}

float4 PShaderSynthesis(float2 TexCoord : TEXCOORD0) : COLOR
{
	float4 bgColor = tex2D(g_sampleBackground, TexCoord);

	float4 colorInside = tex2D(g_sampleInside, TexCoord);
	colorInside = GaussianBlur(g_ScreenWidth, g_ScreenHeight, g_sampleInside, TexCoord);

	float4 colorInside2 = tex2D(g_sampleInside2, TexCoord);
	colorInside2 = GaussianBlur(g_ScreenWidth, g_ScreenHeight, g_sampleInside2, TexCoord);

	float insideAlpha = colorInside.a;// *g_AlphaFactor;
	float insideAlpha2 = colorInside2.a *0.5;// *g_AlphaFactor;

	float4 bloomColor = tex2D(g_sampleBloomed, TexCoord);
	bloomColor.rgb = 0;

	float bloomFactor = 1 -insideAlpha;
	insideAlpha = bloomFactor * bloomColor.a + (1-bloomFactor)* insideAlpha;
	
	insideAlpha = insideAlpha *g_AlphaFactor;

	float4 brushColor = float4(0, 0, 0, 0);
	float4 Inside = brushColor * insideAlpha + bgColor * (1 - insideAlpha);

	
	insideAlpha2 = insideAlpha2 * (1 - colorInside.r);
	//if (colorInside.r <= 0)
	Inside = brushColor * insideAlpha2 + Inside * (1 - insideAlpha2);

	//return colorInside;
	return bloomColor + Inside;
}

float4 PShaderBlur(float2 TexCoord : TEXCOORD0) : COLOR
{
	float4 colorInside = tex2D(g_sampleInside, TexCoord); 
	float4 colorInside2 = tex2D(g_sampleInside2, TexCoord);
	colorInside = GaussianBlur(g_ScreenWidth, g_ScreenHeight, g_sampleInside, TexCoord);
	colorInside2 = GaussianBlur(g_ScreenWidth, g_ScreenHeight, g_sampleInside2, TexCoord);
	//colorInside.a = max(colorInside.a, colorInside2.a);
	return colorInside;
}

float4 PShaderSrc(float2 TexCoord : TEXCOORD0) : COLOR
{
	float4 colorSrc = tex2D(g_sampleSrc, TexCoord);
	//return float4(1, 0, 1, 0);
	return colorSrc;
}

float4 Diffusion(sampler2D texSampler, float2 TexCoord, float2 screenSize)
{
	//return float4(1, 0, 0, 1);
	float color = tex2D(texSampler, TexCoord).r;
	float avg = color;
	float outColor = color;

	if (color > 0.99)
	{
		//return float4(1, 0, 0, 1);
		float colorL = tex2D(texSampler, TexCoord + float2(-1, 0)*float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight)).r;
		float colorR = tex2D(texSampler, TexCoord + float2(1, 0)*float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight)).r;
		float colorU = tex2D(texSampler, TexCoord + float2(0, -1)*float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight)).r;
		float colorD = tex2D(texSampler, TexCoord + float2(0, 1)*float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight)).r;

		float colorLU = tex2D(texSampler, TexCoord + float2(-1, -1)*float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight)).r;
		float colorRU = tex2D(texSampler, TexCoord + float2(1, -1)*float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight)).r;
		float colorLD = tex2D(texSampler, TexCoord + float2(-1, 1)*float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight)).r;
		float colorRD = tex2D(texSampler, TexCoord + float2(1, 1)*float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight)).r;

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
			avg *= (0.95f - random);
			outColor = 1 - avg / 255.0f;
		}
	}
	//return float4(1, 0, 0, 1);
	return float4(outColor, outColor, outColor, 1.0f);
}

float4 PShaderSrcAdd(float2 TexCoord : TEXCOORD0) : COLOR
{
	float4 colorSrc = tex2D(g_sampleSrc, TexCoord);
	float4 colorSrc2 = tex2D(g_sampleSrc2, TexCoord);
	float4 colorSrc3 = tex2D(g_sampleSrc3, TexCoord);
	float4 colorSrc4 = tex2D(g_sampleSrc2, TexCoord);
	float4 colorSrc5 = tex2D(g_sampleSrc3, TexCoord);

	float4 final = colorSrc*0.0 + colorSrc2*0.2442 + colorSrc3*0.40262 + colorSrc4*0.2442 + colorSrc5*0.0545;
	//float4 blur = GaussianBlur(g_ScreenWidth, g_ScreenHeight, g_sampleSrc, TexCoord);
	//blur = Diffusion(g_sampleSrc, TexCoord, float2(g_ScreenWidth, g_ScreenHeight));
	//final = colorSrc*0.2 + colorSrc2*0.2 + colorSrc3*0.2 + colorSrc4*0.2 + colorSrc5*0.2;
	//float4 outColor = final.x;
	//	outColor = blur.x > final.x ? final : blur;
	//return outColor;
	return final;

	float d = colorSrc2.r - final.r;
	if (d != 0) d = 1;
	return float4(abs(d), abs(d), abs(d), 0);
	return colorSrc;
}

float4 PShaderGetDarkPart(float2 TexCoord : TEXCOORD0) : COLOR
{
	float4 colorSrc = tex2D(g_sampleInside, TexCoord);
	if (colorSrc.a > 0.2)
	{
		return colorSrc;
	}
	return float4(0, 0, 0, 0);
	/*
	if (colorSrc.a > 0.8)
	{
		float darkness = 1 - colorSrc.a;
		return float4(darkness, darkness, darkness, 1);
	}
	return float4(1, 1, 1, 1);
	*/
}

#define SAMPLE_COUNT 30

// 偏移数组
float2 g_SampleOffsets[SAMPLE_COUNT];
// 权重数组
float g_SampleWeights[SAMPLE_COUNT];

float4 PShaderInputBlur(float2 TexCoord : TEXCOORD0) : COLOR
{
	float4 c = 0;

	float depthCenter = tex2D(g_samplePosition, TexCoord).r;
	// 按偏移及权重数组叠加周围颜色值到该像素
	// 相对原理，即可理解为该像素颜色按特定权重发散到周围偏移像素
	float totalWeight = 0;
	for (int i = 0; i < SAMPLE_COUNT; i++)
	{
		float depthSample = tex2D(g_samplePosition, TexCoord + g_SampleOffsets[i]).r;

		float diffuseSample = tex2D(g_sampleDiffuse, TexCoord + g_SampleOffsets[i]).b;

		float weight = g_SampleWeights[i];
		/*
		float deltaD_Inner = depthCenter / 10.0f;
		if (depthCenter - depthSample >deltaD_Inner && depthSample < 50)
			weight = 0;
		else if (depthCenter - depthSample >deltaD_Inner && (depthSample >= 50 && depthSample < 100))
			weight = g_SampleWeights[i] * ((depthSample - 50.0f) / 50.0f);
			*/

		weight = g_SampleWeights[i] * (1 - diffuseSample);
		c += tex2D(g_sampleInput, TexCoord + g_SampleOffsets[i]) * weight;
		totalWeight += weight;
	}
	c = c / totalWeight;
	return c;
}

float4 PShaderFP(float2 TexCoord : TEXCOORD0) : COLOR
{ 
	float4 colorSrc = tex2D(g_sampleSrc, TexCoord);

	float moveMark = tex2D(g_sampleNormal, TexCoord).a;
	if (moveMark < 0.5)
		colorSrc = float4(1, 1, 1, 1);

	float4 colorSrc_LF = tex2D(g_sample_FP_LF, TexCoord);
	
	colorSrc = float4(1, 1, 1, 1) - colorSrc;
	//colorSrc = colorSrc *0.99f;
	colorSrc = float4(1, 1, 1, 1) - colorSrc;

	colorSrc_LF = float4(1, 1, 1, 1) - colorSrc_LF;
	colorSrc_LF = colorSrc_LF *0.9f;
	colorSrc_LF = float4(1, 1, 1, 1) - colorSrc_LF;

	//return float4(1, 0, 1, 0);
	return colorSrc * colorSrc_LF;
}

float4 PShaderDiffusion(float2 TexCoord : TEXCOORD0) : COLOR
{
	float4 colorSrc = tex2D(g_sampleSrc, TexCoord);
	float4 blur = GaussianBlur(g_ScreenWidth, g_ScreenHeight, g_sampleSrc, TexCoord);




	//blur = Diffusion(g_sampleSrc, TexCoord, float2(g_ScreenWidth, g_ScreenHeight));
	//final = colorSrc*0.2 + colorSrc2*0.2 + colorSrc3*0.2 + colorSrc4*0.2 + colorSrc5*0.2;
	//float4 outColor = final.x;
	//	outColor = blur.x > final.x ? final : blur;
	//return outColor;
	return blur;
}

float4 PShaderSynthesis2(float2 TexCoord : TEXCOORD0) : COLOR
{
	float4 final = tex2D(g_sampleSrc, TexCoord);

	float4 diffusion = tex2D(g_sampleSrc2, TexCoord);


	float4 outColor = diffusion * final;

	float moveMark = tex2D(g_sampleNormal, TexCoord).a;
	if (moveMark < 0.5)
		outColor = diffusion * final;
	else
		outColor = final;


	//outColor = float4(min(diffusion.x, final.x), min(diffusion.y, final.y), min(diffusion.z, final.z), 1);
	return outColor;
}

float4 PShaderDrawBack(float2 TexCoord : TEXCOORD0) : COLOR
{
	float4 colorSrc = tex2D(g_sampleSrc, TexCoord);
	return colorSrc;
}

float2 GetPosUV(float3 pos)
{

	float u = g_zNear * pos.x / pos.z / (g_zNear * g_ViewAngle_half_tan * g_ViewAspect);
	float v = g_zNear * pos.y / pos.z / (g_zNear * g_ViewAngle_half_tan);

	u = 0.5 * u + 0.5;
	v = 0.5 - 0.5 * v;

	return float2(u, v);
}

bool g_remap = true;

float4 PShaderTemporal(float2 TexCoord : TEXCOORD0) : COLOR
{
	float4 colorSrc = tex2D(g_sampleSrc, TexCoord);

	float2 UV = TexCoord;
	if (g_remap)
	{

		float3 pos = GetPosition(TexCoord, g_samplePosition);
			float4 posW = mul(float4(pos, 1.0f), g_invView);
			float4 lastPosV = mul(posW, g_LastView);
			UV = GetPosUV(lastPosV.xyz);
			//lastUV = TexCoord;
	}

	float4 historyColor = tex2D(g_sampleSrc2, UV);


	float4 minColor, maxColor;
	minColor = colorSrc;
	maxColor = colorSrc; 
	float2 fullResStep = float2(1.0 / g_ScreenWidth, 1.0 / g_ScreenHeight);
	if (true)
	{
		minColor = colorSrc;
		maxColor = colorSrc;
		int num = 2;
		for (int i = 0; i < num; i++)
		{
			for (int j = 0; j < num; j++)
			{
				float2 offset = float2(i - num / 2, j - num / 2);
				offset *= fullResStep;
				float4 curColor = tex2D(g_sampleSrc, TexCoord + offset);

				maxColor = max(maxColor, curColor);
				minColor = min(minColor, curColor);
			}
		}
		historyColor = clamp(historyColor, minColor, maxColor);
	}

	float4 final = lerp(historyColor, colorSrc, 0.05f);

		return final;
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
		pixelShader = compile ps_3_0 PShaderSrc();
	}

	pass p3
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShaderSrcAdd();
	}

	pass p4
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShaderGetDarkPart();
	}

	pass p5
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShaderInputBlur();
	}

	pass p6
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShaderFP();
	}

	pass p7
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShaderDiffusion();
	}

	pass p8
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShaderSynthesis2();
	}
	pass p9
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShaderDrawBack();
	}
	pass p10
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShaderTemporal();
	}
}