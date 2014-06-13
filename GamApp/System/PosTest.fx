matrix		g_World;
matrix		g_View;
matrix		g_Proj;
matrix		g_ViewProj;
matrix		g_WorldViewProj;
matrix		g_mWorldInv;
matrix		g_InverseProj;

float		g_zNear = 1.0f;
float		g_zFar = 100.0f;

float		g_intensity = 1;
float		g_scale = 1;
float		g_bias = 0;
float		g_sample_rad = 0.03;

texture		g_NormalDepthBuffer;
texture		g_RandomNormal; 
texture		g_PositionBuffer;

sampler2D g_sampleNormalDepth =
sampler_state
{
	Texture = <g_NormalDepthBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

sampler2D g_sampleRandomNormal =
sampler_state
{
	Texture = <g_RandomNormal>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
};

sampler2D g_samplePosition =
sampler_state
{
	Texture = <g_PositionBuffer>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
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



float3 getPosition2(in float2 uv)
{
	return tex2D(g_samplePosition, uv).xyz;
}

float3 getPosition1(in float2 uv)
{
	//纹理采样
	float4 NormalDepth = tex2D(g_sampleNormalDepth, uv);

	// 从视口坐标中获取 x/w 和 y/w  
	float x = uv.x * 2.0f - 1;
	float y = (1 - uv.y) * 2.0f - 1.0f;
	float4 vProjectedPos = float4(x, y, NormalDepth.w, 1.0f);
	// 通过转置的投影矩阵进行转换到视图空间  
	float4 vPositionVS = mul(vProjectedPos, g_InverseProj);
	float3 vPositionVS3 = vPositionVS.xyz / vPositionVS.w;
	return vPositionVS3.xyz;
}

float3 getNormal(in float2 uv)
{
	return normalize(tex2D(g_sampleNormalDepth, uv).xyz * 2.0f - 1.0f);
}

float2 getRandom(in float2 uv)
{
	return normalize(tex2D(g_sampleRandomNormal, /*g_screen_size*/float2(800,600) * uv / /*random_size*/float2(256,256)).xy * 2.0f - 1.0f);
}

float4 PShader(float2 TexCoord : TEXCOORD0) : COLOR
{ 
	float z1 = getPosition1(TexCoord).z;
	float z2 = getPosition2(TexCoord).z;

	float x1 = getPosition1(TexCoord).x;
	float x2 = getPosition2(TexCoord).x;

	float y1 = getPosition1(TexCoord).y;
	float y2 = getPosition2(TexCoord).y;

	float3 p1 = getPosition2(TexCoord);
	float3 p2 = getPosition2(TexCoord);

	//return float4(p1, 1.0f);
	return float4(p2, 1.0f);

	//return tex2D(g_samplePosition, TexCoord);
	float dz = abs(z1 - z2);
	float dx = abs(x1 - x2);
	float dy = abs(y1 - y2);

	if (dx >= 0.03)
		return float4(1, 0, 0, 1.0f);
	else
		return float4(0, 0, 1, 1.0f);

	return float4(dy, dy, dy, 1.0f);
}

technique SSAO
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}