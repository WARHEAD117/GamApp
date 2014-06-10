matrix		g_World;
matrix		g_View;
matrix		g_Proj;
matrix		g_ViewProj;
matrix		g_WorldViewProj;
matrix		g_mWorldInv;
matrix		g_InverseProj;

float		g_zNear = 1.0f;
float		g_zFar = 1000.0f;

float4		aoParam = float4(0.1, 0.1, 5.0, 15.0);

texture		g_NormalDepthBuffer;
texture		g_RandomNormal;

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

float doAO(float2 uv, float2 rand, float2 radius, float3 pos, int i)
{
	float intensity = aoParam.z;
	float scale = aoParam.w;

	//float Dis = pow(length(pos), 2);

	float2 randUV = uv + rand*radius;// *Dis;
		float4 NormalDepth = tex2D(g_sampleNormalDepth, randUV);

	// 从视口坐标中获取 x/w 和 y/w  
	float x = randUV.x * 2 - 1;
	float y = (1 - randUV.y) * 2 - 1;
	float4 vProjectedPos = float4(x, y, NormalDepth.w, 1.0f);
		// 通过转置的投影矩阵进行转换到视图空间  
		float4 vPositionVS = mul(vProjectedPos, g_InverseProj);
		float3 ViewPos = vPositionVS.xyz / vPositionVS.w;
		float3 samplePos = ViewPos;

		float3 diff = samplePos - pos;
		float3 v = normalize(samplePos - pos);
		float D = length(diff) / pos.z * 100;// *scale;

	

	float3 normal = normalize(NormalDepth.xyz* 2.0f - 1.0f);

		
		
	float ao = max(0.0, dot(normal, v) - 0.1f)* (1.0f / (1.0f + D)) * intensity;
	return ao;
}

float4 PShader(float2 TexCoord : TEXCOORD0) : COLOR
{
	//纹理采样
	float4 NormalDepth = tex2D(g_sampleNormalDepth, TexCoord);
	
	// 从视口坐标中获取 x/w 和 y/w  
	float x = TexCoord.x * 2 - 1;
	float y = (1 - TexCoord.y) * 2 - 1;
	float4 vProjectedPos = float4(x, y, NormalDepth.w, 1.0f);
	// 通过转置的投影矩阵进行转换到视图空间  
	float4 vPositionVS = mul(vProjectedPos, g_InverseProj);
	float3 ViewPos = vPositionVS.xyz / vPositionVS.w;
	float3 pos = ViewPos;

	float dis = g_zFar - g_zNear;

	float2 vec[8] = { float2(1, 1), float2(1, -1), float2(-1, 1), float2(-1, -1), float2(0, 1.4f), float2(0, -1.4f), float2(1.4f, 0), float2(-1.4f, 0) };
	float radius1 = aoParam.x;
	float radius2 = aoParam.y;
	float intensity = aoParam.z;

	float d1 = radius1 / NormalDepth.w / dis;
	float d2 = radius2 / NormalDepth.w / dis;

	float4 normal = tex2D(g_sampleRandomNormal, TexCoord * float2(800,600) / float2(256, 256));
	float2 rand = normalize(normal);
	rand = rand * 2 - 1;

	float ao = 0;

	for (int i = 0; i<8; i++)
	{
		ao += doAO(TexCoord, rand, vec[i] * d1, pos, i);
	}

	for (int i = 0; i<8; i++)
	{
		ao += doAO(TexCoord, rand, vec[i] * d2, pos, i);
	}

	ao = ao / 16;

	if (ao < 0)
		ao = 0;

	float finalAO = ao;
	float4 finalColor = float4(finalAO, finalAO, finalAO, 1.0f);

	return finalColor;// float4(1.0f, 0.0f, 0.0f, 1.0f);
}

technique SSAO
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}