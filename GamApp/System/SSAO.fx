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
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
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

float3 GetPosition(in float2 uv)
{
	//使用positionBuffer来获取位置，精度较高，但是慢
	//return tex2D(g_samplePosition, uv).xyz;

	//使用深度重建位置信息，精度较低，误差在小数点后第二位出现，但是速度很好
	//法线深度图采样
	float4 NormalDepth = tex2D(g_sampleNormalDepth, uv);

	// 从视口坐标中获取 x/w 和 y/w  
	float x = uv.x * 2.0f - 1;
	float y = (1 - uv.y) * 2.0f - 1.0f;
	//这里的z值是投影后的非线性深度
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

//Maria SSAO
float doAmbientOcclusion(in float2 tcoord, in float2 uv, in float3 p, in float3 cnorm)
{
	float3 diff = GetPosition(tcoord + uv) - p;
	const float3 v = normalize(diff);
	const float d = length(diff)*g_scale;
	return max(0.0, dot(cnorm, v) - g_bias)*(1.0 / (1.0 + d))*g_intensity;
}

float4 PShader(float2 TexCoord : TEXCOORD0) : COLOR
{
	const float2 vec[4] = { float2(1, 0), float2(-1, 0),float2(0, 1), float2(0, -1) };
	
	//观察空间位置
	float3 p = GetPosition(TexCoord);

	//深度重建的位置会有误差，最远处的误差会导致背景变灰，所以要消除影响
	if (p.z > g_zFar - 0.1)
		return float4(1, 1, 1, 1);

	//观察空间法线
	float3 n = getNormal(TexCoord);
	
	//随机法线
	float2 rand = getRandom(TexCoord);
	float ao = 0.0f;
	float rad = g_sample_rad / p.z;

	//**SSAO Calculation**// 
	int iterations = 4;
	
	for (int j = 0; j < iterations; ++j)
	{
		float2 coord1 = reflect(vec[j], rand)*rad;
			
		float2 coord2 = float2(coord1.x*0.707 - coord1.y*0.707, coord1.x*0.707 + coord1.y*0.707);

		ao += doAmbientOcclusion(TexCoord, coord1*0.25, p, n);
		ao += doAmbientOcclusion(TexCoord, coord2*0.5, p, n);
		ao += doAmbientOcclusion(TexCoord, coord1*0.75, p, n);
		ao += doAmbientOcclusion(TexCoord, coord2, p, n);
	}

	ao /= (float)iterations*4.0;
	//**END**//  
	//Do stuff here with your occlusion value “ao”: modulate ambient lighting,  write it to a buffer for later //use, etc. 

	return float4(1 - ao, 1 - ao, 1 - ao, 1.0f);
}

technique SSAO
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}