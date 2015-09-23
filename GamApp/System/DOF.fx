#include "common.fx"

static const int MAX_SAMPLES = 8;
float g_bokehAngle;

float g_aperture;
float g_focallength;
float g_planeinfocus;
float g_scale;

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

texture		g_ColorCoCBuffer;
sampler2D g_sampleColorCoC =
sampler_state
{
	Texture = <g_ColorCoCBuffer>;
	MinFilter = linear;
	MagFilter = linear;
	MipFilter = linear;
	AddressU = Border;
	AddressV = Border;
};

texture		g_ColorStep1;
sampler2D colorStep1Sampler =
sampler_state
{
	Texture = <g_ColorStep1>;
	MinFilter = linear;
	MagFilter = linear;
	MipFilter = linear;
	AddressU = Border;
	AddressV = Border;
};

texture		g_ColorStep2;
sampler2D colorStep2Sampler =
sampler_state
{
	Texture = <g_ColorStep2>;
	MinFilter = linear;
	MagFilter = linear;
	MipFilter = linear;
	AddressU = Border;
	AddressV = Border;
};

struct OutputVS
{
	float4 posWVP         : POSITION0;
	float2 TexCoord		: TEXCOORD0;
};

struct OffsetData
{
	half2 offsets[MAX_SAMPLES];
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

OffsetData MakeOffset(half angle)
{
	OffsetData offsetData;

	half aspectRatio = g_ScreenWidth / g_ScreenHeight;

	half radius = 0.5f;

	half2 pt = half2(radius * cos(angle), radius * sin(angle));

	pt.x /= aspectRatio;

	for (int i = 0; i < MAX_SAMPLES; i++)
	{
		half2 t = i / (MAX_SAMPLES - 1.0f);
			offsetData.offsets[i] = lerp(-pt, pt, t);
	}

	return offsetData;
}

float4 ComputeCoC(float2 TexCoord : TEXCOORD0) : COLOR
{
	float3 pos = GetPosition(TexCoord, g_samplePosition);
	float CoC = abs(g_aperture * (g_focallength * (pos.z - g_planeinfocus)) / (pos.z * (g_planeinfocus - g_focallength)));

	const half sensorHeight = 0.024f;

	//这里要压缩CoC，否则直接截取会导致残留的清晰区间过小（为什么？）
	const half percentOfSensor = CoC / sensorHeight * g_scale;
	
	half MaxCoC = 1.0f;

	half blurFactor = clamp(percentOfSensor, 0.0f, MaxCoC);

	half4 color = tex2D(g_sampleMainColor, TexCoord);

	return float4(color.xyz,blurFactor);
}

float4 DrawDof(float2 TexCoord, OffsetData offsetData)
{
	const half bleedingBias = 0.02f;
	const half bleedingMult = 30.0f;

	half centerPixelCoC = tex2D(g_sampleColorCoC, TexCoord).a;
	half centerDepth = GetPosition(TexCoord, g_samplePosition).z;

	half4 color = 0.0f;
		half totalWeight = 0.0f;

	for (int t = 0; t < MAX_SAMPLES; t++)
	{
		half2 offset = offsetData.offsets[t];

		half2 sampleCoords = TexCoord + offset * centerPixelCoC;

		half4 samplePixel = tex2D(g_sampleColorCoC, sampleCoords);
		half sampleDepth = GetPosition(sampleCoords, g_samplePosition).z;

		half weight = sampleDepth < centerDepth ? samplePixel.a * bleedingMult : 1.0f;
		weight = (centerPixelCoC > (samplePixel.a + bleedingBias)) ? weight : 1.0f;
		weight = saturate(weight);

		color += samplePixel * weight;
		totalWeight += weight;
	}

	return color / totalWeight;
	//return tex2D(g_sampleColorCoC, TexCoord);
	//return float4(centerDepth, centerDepth, centerDepth, 1.0f);
}

float4 DrawDoF_PS(float2 TexCoord : TEXCOORD0) : COLOR
{
	OffsetData offsetData = MakeOffset(g_bokehAngle);

	return DrawDof(TexCoord, offsetData);
}

float4 DrawDoF_Final(float2 TexCoord : TEXCOORD0) : COLOR
{
	half4 colorStep1 = tex2D(colorStep1Sampler, TexCoord);

	half4 colorStep2 = tex2D(colorStep2Sampler, TexCoord);

	return min(colorStep1, colorStep2);
	//return tex2D(g_sampleColorCoC, TexCoord);
	//return tex2D(g_sampleColorCoC, TexCoord).a * 10;
}

technique DOF
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 ComputeCoC();
	}

	pass p1
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 DrawDoF_PS();
	}

	pass p2
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 DrawDoF_Final();
	}
}