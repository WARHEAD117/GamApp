#include "common.fx"

static const int    MAX_SAMPLES = 16;    // Maximum texture grabs
float2		g_avSampleOffsets[MAX_SAMPLES];


int			g_mapWidth;
int			g_mapHeight;

float		g_intensity = 1;
float		g_scale = 1;
float		g_bias = 0;
float		g_sample_rad = 0.03f;

float		g_rad_scale = 0.3f;
float		g_rad_threshold = 4.0f;

texture		g_NormalBuffer;
texture		g_RandomNormal; 
texture		g_PositionBuffer;
texture		g_ViewDirBuffer;

sampler2D g_sampleNormal =
sampler_state
{
	Texture = <g_NormalBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

sampler2D g_sampleRandomNormal =
sampler_state
{
	Texture = <g_RandomNormal>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

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

texture		g_GiBuffer;
sampler2D g_sampleGi =
sampler_state
{
	Texture = <g_GiBuffer>;
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

float2 getRandom(in float2 uv)
{
	return normalize(tex2D(g_sampleRandomNormal, /*g_screen_size*/float2(g_ScreenWidth, g_ScreenHeight) * uv / /*random_size*/float2(64, 64)).xy * 2.0f - 1.0f);
}

//Maria SSAO
float doAmbientOcclusion(in float2 tcoord, in float2 uv, in float3 p, in float3 cnorm)
{
	float3 diff = GetPosition(tcoord + uv, g_samplePosition) - p;
	const float3 v = normalize(diff);
	const float d = length(diff)*g_scale;
	return max(0.0, dot(cnorm, v) - g_bias)*(1.0 / (1.0 + d))*g_intensity;
}

float4 doGI(in float2 tcoord, in float2 uv, in float3 p, in float3 cnorm)
{
	float3 pos = GetPosition(tcoord + uv, g_samplePosition);
		float3 diff = pos - p;
	const float3 v = normalize(diff);
	const float d = length(diff)*g_scale;
	float ao = max(0.0, dot(cnorm, v) - g_bias)*(1.0 / (1.0 + d))*g_intensity;

	float3 n = normalize(GetNormal(tcoord + uv, g_sampleNormal));
		float sameDir = dot(n, -v);// > 0 ? 1 : 0;
	//sameDir = max(sameDir, 0.0);

	float4 color = GetColor(tcoord + uv, g_sampleMainColor);

	return color * sameDir * ao * 2 * 3.14f;
}

float4 PShader(float2 TexCoord : TEXCOORD0) : COLOR
{
	const float2 vec[8] = { float2(1, 0), float2(-1, 0),float2(0, 1), float2(0, -1),
							float2(2, 0), float2(-2, 0), float2(0, 2), float2(0, -2) };
	
	//观察空间位置
	float3 p = GetPosition(TexCoord, g_samplePosition);

	//深度重建的位置会有误差，最远处的误差会导致背景变灰，所以要消除影响
	if (p.z > g_zFar)
		return float4(0, 0, 0, 0);

	//观察空间法线
	float3 n = GetNormal(TexCoord, g_sampleNormal);
	
	//随机法线
	float2 rand = getRandom(TexCoord);
	float4 giColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float invDepth = 1 - p.z / g_zFar;
	float rad = g_sample_rad / p.z;
	//float rad = g_sample_rad * (invDepth * invDepth) * g_rad_scale;
	rad = (p.z < g_rad_threshold) ? (g_sample_rad / p.z) : rad;

	//**SSAO Calculation**// 
	int iterations = 4;
	
	for (int j = 0; j < iterations; ++j)
	{
		float2 coord1 = reflect(vec[j], rand)*rad;
			
		float2 coord2 = float2(coord1.x*0.707 - coord1.y*0.707, coord1.x*0.707 + coord1.y*0.707);

		giColor += doGI(TexCoord, coord1*0.25, p, n);
		giColor += doGI(TexCoord, coord2*0.5, p, n);
		giColor += doGI(TexCoord, coord1*0.75, p, n);
		giColor += doGI(TexCoord, coord2, p, n);
	}

	giColor /= (float)iterations*4.0;
	
	return giColor;
	//**END**//  
	//Do stuff here with your occlusion value “ao”: modulate ambient lighting,  write it to a buffer for later //use, etc. 
	//return float4(1 - ao, 1 - ao, 1 - ao, 1.0f);
}

float4 DrawMain(float2 TexCoord : TEXCOORD0) : COLOR
{
	float4 GI = /*tex2D(g_sampleAo, TexCoord);*/ GaussianBlur(300, 200, g_sampleGi, TexCoord);
	float4 fianlColor = GI + tex2D(g_sampleMainColor, TexCoord);

	return fianlColor;
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

technique SSGI
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}

	pass p1
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 DrawMain();
	}

	pass bufferScale
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 DownScale4x4PS();
	}

}