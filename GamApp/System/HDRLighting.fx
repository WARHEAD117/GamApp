//-----------------------------------------------------------------------------
// Global constants
//-----------------------------------------------------------------------------
static const int    MAX_SAMPLES = 16;    // Maximum texture grabs
static const int    NUM_LIGHTS = 2;     // Scene lights 
static const float  BRIGHT_PASS_THRESHOLD = 5.0f;  // Threshold for BrightPass filter
static const float  BRIGHT_PASS_OFFSET = 10.0f; // Offset for BrightPass filter

// The per-color weighting to be used for luminance calculations in RGB order.
static const float3 LUMINANCE_VECTOR = float3(0.2125f, 0.7154f, 0.0721f);

// The per-color weighting to be used for blue shift under low light.
static const float3 BLUE_SHIFT_VECTOR = float3(1.05f, 0.97f, 1.27f);

#include "common.fx"


// Contains sampling offsets used by the techniques
float2 g_avSampleOffsets[MAX_SAMPLES];
float4 g_avSampleWeights[MAX_SAMPLES];

// Tone mapping variables
float  g_fMiddleGray;       // The middle gray key value
float  g_fWhiteCutoff;      // Lowest luminance which is mapped to white
float  g_fElapsedTime;      // Time in seconds since the last calculation

bool  g_bEnableBlueShift;   // Flag indicates if blue shift is performed
bool  g_bEnableToneMap;     // Flag indicates if tone mapping is performed

float  g_fBloomScale;       // Bloom process multiplier
float  g_fStarScale;        // Star process multiplier


texture	g_MainColorBuffer;
sampler2D g_sampleMainColor : register(s0) =
sampler_state
{
	Texture = <g_MainColorBuffer>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
};

texture	g_BloomTex;
sampler2D g_sampleBloomTex : register(s1) =
sampler_state
{
	Texture = <g_BloomTex>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
};

texture	g_StarTex;
sampler2D g_sampleStarTex : register(s2) =
sampler_state
{
	Texture = <g_StarTex>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
};

texture	g_LuminanceCur;
sampler2D g_sampleLuminanceCur : register(s3) =
sampler_state
{
	Texture = <g_LuminanceCur>;
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
	//color.r = 0.0f;
	return color;
}


float3 F(float3 x)
{
	const float A = 0.22f;
	const float B = 0.30f;
	const float C = 0.10f;
	const float D = 0.20f;
	const float E = 0.01f;
	const float F = 0.30f;

	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 Uncharted2ToneMapping(float3 color, float adapted_lum)
{
	const float WHITE = 11.2f;
	return F(1.6f * adapted_lum * color) / F(WHITE);
}


float3 ACESToneMapping(float3 color, float adapted_lum)
{
	const float A = 2.51f;
	const float B = 0.03f;
	const float C = 2.43f;
	const float D = 0.59f;
	const float E = 0.14f;

	color *= adapted_lum;
	return (color * (A * color + B)) / (color * (C * color + D) + E);
}

float3 CEToneMapping(float3 color, float adapted_lum)
{
	return 1 - exp(-adapted_lum * color);
}

technique HDRLighting
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}


//-----------------------------------------------------------------------------
// Name: DownScale4x4PS
// Type: Pixel shader                                      
// Desc: Scale the source texture down to 1/16 scale
//-----------------------------------------------------------------------------
float4 DownScale4x4PS
(
in float2 vScreenPosition : TEXCOORD0
) : COLOR
{

	float4 sample = 0.0f;

	for (int i = 0; i < 16; i++)
	{
		sample += tex2D(g_sampleMainColor, vScreenPosition + g_avSampleOffsets[i]);
	}

	return sample / 16;
}


//-----------------------------------------------------------------------------
// Name: DownScale4x4
// Type: Technique                                     
// Desc: Scale the source texture down to 1/16 scale
//-----------------------------------------------------------------------------
technique DownScale4x4
{
	pass P0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 DownScale4x4PS();
	}
}



//-----------------------------------------------------------------------------
// Name: BrightPassFilterPS
// Type: Pixel shader                                      
// Desc: Perform a high-pass filter on the source texture
//-----------------------------------------------------------------------------
float4 BrightPassFilterPS
(
in float2 vScreenPosition : TEXCOORD0
) : COLOR
{
	float4 vSample = tex2D(g_sampleMainColor, vScreenPosition);
	float  fAdaptedLum = tex2D(g_sampleLuminanceCur, float2(0.5f, 0.5f));

	// Determine what the pixel's value will be after tone-mapping occurs
	vSample.rgb *= g_fMiddleGray / (fAdaptedLum + 0.001f);

	// Subtract out dark pixels
	vSample.rgb -= BRIGHT_PASS_THRESHOLD;

	// Clamp to 0
	vSample = max(vSample, 0.0f);

	// Map the resulting value into the 0 to 1 range. Higher values for
	// BRIGHT_PASS_OFFSET will isolate lights from illuminated scene 
	// objects.
	vSample.rgb /= (BRIGHT_PASS_OFFSET + vSample);

	//vSample.rgb = ACESToneMapping(vSample.rgb, fAdaptedLum);
	//vSample.rgb = Uncharted2ToneMapping(vSample.rgb, fAdaptedLum + 0.001f);
	//vSample.rgb = CEToneMapping(vSample.rgb, fAdaptedLum + 0.001f);

	return vSample;
}


//-----------------------------------------------------------------------------
// Name: BrightPassFilter
// Type: Technique                                     
// Desc: Perform a high-pass filter on the source texture
//-----------------------------------------------------------------------------
technique BrightPassFilter
{
	pass P0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 BrightPassFilterPS();
	}
}




//-----------------------------------------------------------------------------
// Name: GaussBlur5x5PS
// Type: Pixel shader                                      
// Desc: Simulate a 5x5 kernel gaussian blur by sampling the 12 points closest
//       to the center point.
//-----------------------------------------------------------------------------
float4 GaussBlur5x5PS
(
in float2 vScreenPosition : TEXCOORD0
) : COLOR
{

	float4 sample = 0.0f;

	for (int i = 0; i < 12; i++)
	{
		sample += g_avSampleWeights[i] * tex2D(g_sampleMainColor, vScreenPosition + g_avSampleOffsets[i]);
	}

	return sample;
}


//-----------------------------------------------------------------------------
// Name: -
// Type: Technique                                     
// Desc: Simulate a 5x5 kernel gaussian blur by sampling the 12 points closest
//       to the center point.
//-----------------------------------------------------------------------------
technique GaussBlur5x5
{
	pass P0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 GaussBlur5x5PS();
	}
}



//-----------------------------------------------------------------------------
// Name: DownScale2x2PS
// Type: Pixel shader                                      
// Desc: Scale the source texture down to 1/4 scale
//-----------------------------------------------------------------------------
float4 DownScale2x2PS
(
in float2 vScreenPosition : TEXCOORD0
) : COLOR
{

	float4 sample = 0.0f;

	for (int i = 0; i < 4; i++)
	{
		sample += tex2D(g_sampleMainColor, vScreenPosition + g_avSampleOffsets[i]);
	}

	return sample / 4;
}

//-----------------------------------------------------------------------------
// Name: DownScale2x2
// Type: Technique                                     
// Desc: Scale the source texture down to 1/4 scale
//-----------------------------------------------------------------------------
technique DownScale2x2
{
	pass P0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 DownScale2x2PS();
	}
}


//-----------------------------------------------------------------------------
// Name: SampleLumInitial
// Type: Pixel shader                                      
// Desc: Sample the luminance of the source image using a kernal of sample
//       points, and return a scaled image containing the log() of averages
//-----------------------------------------------------------------------------
float4 SampleLumInitial
(
in float2 vScreenPosition : TEXCOORD0
) : COLOR
{
	float3 vSample = 0.0f;
	float  fLogLumSum = 0.0f;

	for (int iSample = 0; iSample < 9; iSample++)
	{
		// Compute the sum of log(luminance) throughout the sample points
		vSample = tex2D(g_sampleMainColor, vScreenPosition + g_avSampleOffsets[iSample]);
		fLogLumSum += log(dot(vSample, LUMINANCE_VECTOR) + 0.0001f);
	}

	// Divide the sum to complete the average
	fLogLumSum /= 9;

	return float4(fLogLumSum, fLogLumSum, fLogLumSum, 1.0f);
}




//-----------------------------------------------------------------------------
// Name: SampleLumIterative
// Type: Pixel shader                                      
// Desc: Scale down the luminance texture by blending sample points
//-----------------------------------------------------------------------------
float4 SampleLumIterative
(
in float2 vScreenPosition : TEXCOORD0
) : COLOR
{
	float fResampleSum = 0.0f;

	for (int iSample = 0; iSample < 16; iSample++)
	{
		// Compute the sum of luminance throughout the sample points
		fResampleSum += tex2D(g_sampleMainColor, vScreenPosition + g_avSampleOffsets[iSample]);
	}

	// Divide the sum to complete the average
	fResampleSum /= 16;

	return float4(fResampleSum, fResampleSum, fResampleSum, 1.0f);
}




//-----------------------------------------------------------------------------
// Name: SampleLumFinal
// Type: Pixel shader                                      
// Desc: Extract the average luminance of the image by completing the averaging
//       and taking the exp() of the result
//-----------------------------------------------------------------------------
float4 SampleLumFinal
(
in float2 vScreenPosition : TEXCOORD0
) : COLOR
{
	float fResampleSum = 0.0f;

	for (int iSample = 0; iSample < 16; iSample++)
	{
		// Compute the sum of luminance throughout the sample points
		fResampleSum += tex2D(g_sampleMainColor, float2(0.5f,0.5f) + g_avSampleOffsets[iSample]);
	}

	// Divide the sum to complete the average, and perform an exp() to complete
	// the average luminance calculation
	fResampleSum = exp(fResampleSum / 16);

	return float4(fResampleSum, fResampleSum, fResampleSum, 1.0f);
}

//-----------------------------------------------------------------------------
// Name: SampleAvgLum
// Type: Technique                                     
// Desc: Takes the HDR Scene texture as input and starts the process of 
//       determining the average luminance by converting to grayscale, taking
//       the log(), and scaling the image to a single pixel by averaging sample 
//       points.
//-----------------------------------------------------------------------------
technique SampleAvgLum
{
	pass P0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 SampleLumInitial();
	}
}




//-----------------------------------------------------------------------------
// Name: ResampleAvgLum
// Type: Technique                                     
// Desc: Continue to scale down the luminance texture
//-----------------------------------------------------------------------------
technique ResampleAvgLum
{
	pass P0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 SampleLumIterative();
	}
}




//-----------------------------------------------------------------------------
// Name: ResampleAvgLumExp
// Type: Technique                                     
// Desc: Sample the texture to a single pixel and perform an exp() to complete
//       the evalutation
//-----------------------------------------------------------------------------
technique ResampleAvgLumExp
{
	pass P0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 SampleLumFinal();
	}
}





//-----------------------------------------------------------------------------
// Name: CalculateAdaptedLumPS
// Type: Pixel shader                                      
// Desc: Calculate the luminance that the camera is current adapted to, using
//       the most recented adaptation level, the current scene luminance, and
//       the time elapsed since last calculated
//-----------------------------------------------------------------------------
float4 CalculateAdaptedLumPS
(
in float2 vScreenPosition : TEXCOORD0
) : COLOR
{
	float fAdaptedLum = tex2D(g_sampleMainColor, float2(0.5f, 0.5f));
	float fCurrentLum = tex2D(g_sampleLuminanceCur, float2(0.5f, 0.5f));

	// The user's adapted luminance level is simulated by closing the gap between
	// adapted luminance and current luminance by 2% every frame, based on a
	// 30 fps rate. This is not an accurate model of human adaptation, which can
	// take longer than half an hour.
	float fNewAdaptation = fAdaptedLum + (fCurrentLum - fAdaptedLum) * (1 - pow(0.90f, 30 * g_fElapsedTime));
	return float4(fNewAdaptation, fNewAdaptation, fNewAdaptation, 1.0f);
}

//-----------------------------------------------------------------------------
// Name: CalculateAdaptedLum
// Type: Technique                                     
// Desc: Determines the level of the user's simulated light adaptation level
//       using the last adapted level, the current scene luminance, and the
//       time since last calculation
//-----------------------------------------------------------------------------
technique CalculateAdaptedLum
{
	pass P0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 CalculateAdaptedLumPS();
	}
}

//-----------------------------------------------------------------------------
// Name: FinalScenePassPS
// Type: Pixel shader                                      
// Desc: Perform blue shift, tone map the scene, and add post-processed light
//       effects
//-----------------------------------------------------------------------------
float4 FinalScenePassPS
(
in float2 vScreenPosition : TEXCOORD0
) : COLOR
{
	float4 vSample = tex2D(g_sampleMainColor, vScreenPosition);
	float4 vBloom = tex2D(g_sampleBloomTex, vScreenPosition);
	float4 vStar = tex2D(g_sampleStarTex, vScreenPosition);
	float fAdaptedLum = tex2D(g_sampleLuminanceCur, float2(0.5f, 0.5f));

	// For very low light conditions, the rods will dominate the perception
	// of light, and therefore color will be desaturated and shifted
	// towards blue.
	if (g_bEnableBlueShift)
	{
		// Define a linear blending from -1.5 to 2.6 (log scale) which
		// determines the lerp amount for blue shift
		float fBlueShiftCoefficient = 1.0f - (fAdaptedLum + 1.5) / 4.1;
		fBlueShiftCoefficient = saturate(fBlueShiftCoefficient);

		// Lerp between current color and blue, desaturated copy
		float3 vRodColor = dot((float3)vSample, LUMINANCE_VECTOR) * BLUE_SHIFT_VECTOR;
			vSample.rgb = lerp((float3)vSample, vRodColor, fBlueShiftCoefficient);
	}


	// Map the high range of color values into a range appropriate for
	// display, taking into account the user's adaptation level, and selected
	// values for for middle gray and white cutoff.
	if (g_bEnableToneMap)
	{
		//vSample.rgb *= g_fMiddleGray / (fAdaptedLum + 0.001f);
		//vSample.rgb /= (1.0f + vSample);

		//vSample.rgb = ACESToneMapping(vSample.rgb, 1 / (fAdaptedLum + 0.001f));
		vSample.rgb = Uncharted2ToneMapping(vSample.rgb, 1 / (fAdaptedLum + 0.001f));
		//vSample.rgb = CEToneMapping(vSample.rgb, fAdaptedLum + 0.001f);
	}

	// Add the star and bloom post processing effects
	//vSample += g_fStarScale * vStar;
	vSample += g_fBloomScale * vBloom;
	//return float4(vStar.x, vStar.x, vStar.x, 1.0f);
	return vSample;
}


//-----------------------------------------------------------------------------
// Name: FinalScenePass
// Type: Technique                                     
// Desc: Minimally transform and texture the incoming geometry
//-----------------------------------------------------------------------------
technique FinalScenePass
{
	pass P0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 FinalScenePassPS();
	}
}




//-----------------------------------------------------------------------------
// Name: BloomPS
// Type: Pixel shader                                      
// Desc: Blur the source image along one axis using a gaussian
//       distribution. Since gaussian blurs are separable, this shader is called 
//       twice; first along the horizontal axis, then along the vertical axis.
//-----------------------------------------------------------------------------
float4 BloomPS
(
in float2 vScreenPosition : TEXCOORD0
) : COLOR
{

	float4 vSample = 0.0f;
	float4 vColor = 0.0f;

	float2 vSamplePosition;

	// Perform a one-directional gaussian blur
	for (int iSample = 0; iSample < 15; iSample++)
	{
		vSamplePosition = vScreenPosition + g_avSampleOffsets[iSample];
		vColor = tex2D(g_sampleMainColor, vSamplePosition);
		vSample += g_avSampleWeights[iSample] * vColor;
	}

	return vSample;
}


//-----------------------------------------------------------------------------
// Name: Bloom
// Type: Technique                                     
// Desc: Performs a single horizontal or vertical pass of the blooming filter
//-----------------------------------------------------------------------------
technique Bloom
{
	pass P0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 BloomPS();
	}

}