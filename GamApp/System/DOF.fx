matrix		g_World;
matrix		g_View;
matrix		g_Proj;
matrix		g_ViewProj;
matrix		g_WorldViewProj;
matrix		g_mWorldInv;
matrix		g_InverseProj;

static const int MAX_SAMPLES = 16;
// Contains sampling offsets used by the techniques
float2 g_avSampleOffsets[MAX_SAMPLES];
float4 g_avSampleWeights[MAX_SAMPLES];

float		g_zNear = 1.0f;
float		g_zFar = 100.0f;

int g_ScreenWidth;
int g_ScreenHeight;

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

texture		g_CoCBuffer;
sampler2D g_sampleCoC =
sampler_state
{
	Texture = <g_CoCBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

texture		g_scaledBuffer;
sampler2D g_sampleScaledBuffer =
sampler_state
{
	Texture = <g_scaledBuffer>;
	MinFilter = linear;
	MagFilter = linear;
	MipFilter = linear;
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

float3 GetPosition(in float2 uv)
{
	//使用positionBuffer来获取位置，精度较高，但是要占用三个通道
	return tex2D(g_samplePosition, uv).xyz;

	//使用投影深度重建位置信息，精度较低，误差在小数点后第二位出现，但是速度很好。但是为了能精确还原，必须使用128位纹理，太大太慢
	float DepthP = tex2D(g_samplePosition, uv).w;

	// 从视口坐标中获取 x/w 和 y/w  
	float x = uv.x * 2.0f - 1;
	float y = (1 - uv.y) * 2.0f - 1.0f;
	//这里的z值是投影后的非线性深度
	float4 vProjectedPos = float4(x, y, DepthP, 1.0f);
	// 通过转置的投影矩阵进行转换到视图空间  
	float4 vPositionVS = mul(vProjectedPos, g_InverseProj);
	float3 vPositionVS3 = vPositionVS.xyz / vPositionVS.w;
	return vPositionVS3.xyz;
}


float4 ComputeCoC(float2 TexCoord : TEXCOORD0) : COLOR
{
	float3 pos = GetPosition(TexCoord);
	float CoC = abs(g_aperture * (g_focallength * (pos.z - g_planeinfocus)) / (pos.z * (g_planeinfocus - g_focallength)));
	float maxCoC = g_aperture * (g_focallength / (g_planeinfocus - g_focallength));
	//CoC = saturate(CoC * g_scale);
	return float4(CoC / maxCoC, CoC / maxCoC, CoC / maxCoC, 1.0f);
}


float4 GaussianBlur(int mapWidth, int mapHeight, sampler2D texSampler, float2 texCoords, float stepLength)
{
	float weights[6] = { 0.00078633, 0.00655965, 0.01330373, 0.05472157, 0.11098164, 0.22508352 };

	float4 color;
	float stepU = stepLength / mapWidth;
	float stepV = stepLength / mapHeight;

	//0,1,2,1,0
	//1,3,4,3,1
	//2,4,5,4,2
	//1,3,4,3,1
	//0,1,2,1,0
	color = tex2D(texSampler, texCoords + float2(-2 * stepU, -2 * stepV)) * (weights[0]);
	color += tex2D(texSampler, texCoords + float2(-1 * stepU, -2 * stepV)) * (weights[1]);
	color += tex2D(texSampler, texCoords + float2(0 * stepU, -2 * stepV)) * (weights[2]);
	color += tex2D(texSampler, texCoords + float2(1 * stepU, -2 * stepV)) * (weights[1]);
	color += tex2D(texSampler, texCoords + float2(2 * stepU, -2 * stepV)) * (weights[0]);

	color += tex2D(texSampler, texCoords + float2(-2 * stepU, -1 * stepV)) * (weights[1]);
	color += tex2D(texSampler, texCoords + float2(-1 * stepU, -1 * stepV)) * (weights[3]);
	color += tex2D(texSampler, texCoords + float2(0 * stepU, -1 * stepV)) * (weights[4]);
	color += tex2D(texSampler, texCoords + float2(1 * stepU, -1 * stepV)) * (weights[3]);
	color += tex2D(texSampler, texCoords + float2(2 * stepU, -1 * stepV)) * (weights[1]);

	color += tex2D(texSampler, texCoords + float2(-2 * stepU, 0 * stepV)) * (weights[2]);
	color += tex2D(texSampler, texCoords + float2(-1 * stepU, 0 * stepV)) * (weights[4]);
	color += tex2D(texSampler, texCoords + float2(0 * stepU, 0 * stepV)) * (weights[5]);
	color += tex2D(texSampler, texCoords + float2(1 * stepU, 0 * stepV)) * (weights[4]);
	color += tex2D(texSampler, texCoords + float2(2 * stepU, 0 * stepV)) * (weights[2]);

	color += tex2D(texSampler, texCoords + float2(-2 * stepU, 1 * stepV)) * (weights[1]);
	color += tex2D(texSampler, texCoords + float2(-1 * stepU, 1 * stepV)) * (weights[3]);
	color += tex2D(texSampler, texCoords + float2(0 * stepU, 1 * stepV)) * (weights[4]);
	color += tex2D(texSampler, texCoords + float2(1 * stepU, 1 * stepV)) * (weights[3]);
	color += tex2D(texSampler, texCoords + float2(2 * stepU, 1 * stepV)) * (weights[1]);

	color += tex2D(texSampler, texCoords + float2(-2 * stepU, 2 * stepV)) * (weights[0]);
	color += tex2D(texSampler, texCoords + float2(-1 * stepU, 2 * stepV)) * (weights[1]);
	color += tex2D(texSampler, texCoords + float2(0 * stepU, 2 * stepV)) * (weights[2]);
	color += tex2D(texSampler, texCoords + float2(1 * stepU, 2 * stepV)) * (weights[1]);
	color += tex2D(texSampler, texCoords + float2(2 * stepU, 2 * stepV)) * (weights[0]);

	return color;
}

float4 GaussBlur5x5PS(sampler2D texSampler, float2 texCoords, float stepLength)
{

	float4 sample = 0.0f;

	for (int i = 0; i < 12; i++)
	{
		float w = tex2D(g_sampleCoC, texCoords + stepLength * g_avSampleOffsets[i]).x - tex2D(g_sampleCoC, texCoords).x;
		if (abs(w)*1000 <2.0)
			sample += g_avSampleWeights[i] * tex2D(texSampler, texCoords + stepLength * g_avSampleOffsets[i]);
		else
			sample += g_avSampleWeights[i] * tex2D(texSampler, texCoords);
	}

	return sample;
}

float4 Blur3x3(sampler2D texSampler, int mapWidth, int mapHeight, float2 texCoords)
{
	float stepU = 1.0f / mapWidth;
	float stepV = 1.0f / mapHeight;

	float4 sample = 0.0f;

	sample += tex2D(texSampler, texCoords + float2(-stepU, -stepV)) * 1.0f /16.0f;
	sample += tex2D(texSampler, texCoords + float2(-stepU, 0)) * 2.0f / 16.0f;
	sample += tex2D(texSampler, texCoords + float2(-stepU, stepV)) * 1.0f / 16.0f;
	sample += tex2D(texSampler, texCoords + float2(0, -stepV)) * 2.0f / 16.0f;
	sample += tex2D(texSampler, texCoords + float2(0, 0)) * 4.0f / 16.0f;
	sample += tex2D(texSampler, texCoords + float2(0, stepV)) * 2.0f / 16.0f;
	sample += tex2D(texSampler, texCoords + float2(stepU, -stepV)) * 1.0f / 16.0f;
	sample += tex2D(texSampler, texCoords + float2(stepU, 0)) * 2.0f / 16.0f;
	sample += tex2D(texSampler, texCoords + float2(stepU, stepV)) * 1.0f / 16.0f;


	return sample;
}


float4 DrawDoF(float2 TexCoord : TEXCOORD0) : COLOR
{
	//float CoC = tex2D(g_sampleCoC, TexCoord);
	float CoC = Blur3x3(g_sampleCoC, g_ScreenWidth, g_ScreenHeight, TexCoord);
	float4 mainColor = tex2D(g_sampleMainColor, TexCoord);

	float radius = clamp(CoC, 0.0f, 5.0f);
	//float4 bluredColor = GaussBlur5x5PS(g_sampleMainColor, TexCoord, radius);
	float4 bluredColor = GaussianBlur(g_ScreenWidth, g_ScreenHeight, g_sampleScaledBuffer, TexCoord, radius);

	float radius_0to1 = saturate(radius);
	float4 finalColor = mainColor * (1 - radius_0to1) + bluredColor * radius_0to1;
	//return float4(CoC, CoC, CoC, 1.0f);
	return finalColor;
}

float4 DownScale4x4(float2 TexCoord : TEXCOORD0) : COLOR
{
	float4 sample = 0.0f;

	for (int i = 0; i < 16; i++)
	{
		sample += tex2D(g_sampleMainColor, TexCoord + g_avSampleOffsets[i]);
	}

	return sample / 16;
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
		pixelShader = compile ps_3_0 DrawDoF();
	}

	pass p2
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 DownScale4x4();
	}
}