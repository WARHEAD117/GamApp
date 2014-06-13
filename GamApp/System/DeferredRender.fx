matrix		g_World;
matrix		g_View;
matrix		g_Proj;
matrix		g_ViewProj;
matrix		g_WorldViewProj;
matrix		g_mWorldInv;
matrix		g_InverseProj;

texture		g_DiffuseBuffer;
texture		g_NormalDepthBuffer;
texture		g_AOBuffer;

float		g_zNear = 1.0f;
float		g_zFar = 1000.0f;

sampler2D g_sampleDiffuse =
sampler_state
{
	Texture = <g_DiffuseBuffer>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
};

sampler2D g_sampleNormalDepth =
sampler_state
{
	Texture = <g_NormalDepthBuffer>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
};

sampler2D g_sampleAO =
sampler_state
{
	Texture = <g_AOBuffer>;
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

float4 PShader(float2 TexCoord : TEXCOORD0) : COLOR
{
	//纹理采样
	float4 Texture = tex2D(g_sampleDiffuse, TexCoord);
	float4 NormalDepth = tex2D(g_sampleNormalDepth, TexCoord);
	float3 NormalV = normalize(NormalDepth.xyz * 2.0f - 1.0f);

	//DeferredRender
	//计算环境光
	float4 Ambient = float4(0.2f, 0.2f, 0.2f, 1.0f);  //材质可理解为反射系数

	float3 pos = GetPosition(TexCoord);
	if (pos.z > g_zFar - 0.1)
		return float4(0, 0, 0, 1);

	float3 ToEyeDirV = normalize(pos * -1.0f);
	float3 LightDir = float3(-1.0f, -1.0f, -1.0f);
	//float4 LIGHTDIR[4] = { float4(-1.0f, -1.0f, -1.0f, 1.0f), float4(-1.0f, -1.0f, 1.0f, 1.0f), float4(-1.0f, 1.0f, -1.0f, 1.0f), float4(-1.0f, 1.0f, 1.0f, 1.0f) };
	//float4 lightSpecular[4] = { float4(1.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 1.0f, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f) };

	float3 LIGHTDIR1 = float3(-2.0f, -0.0f, -0.0f);
	float4 lightSpecular1 = float4(0.3f, 0.0f, 0.0f, 1.0f);

	float3 LIGHTDIR2 = float3(-2.0f, 2.0f, -2.0f);
	float4 lightSpecular2 = float4(0.0f, 0.3f, 0.0f, 1.0f);

	float3 LIGHTDIR3 = float3(-2.0f, -2.0f, 2.0f);
	float4 lightSpecular3 = float4(0.0f, 0.0f, 0.3f, 1.0f);

	float3 LIGHTDIR4 = float3(0.0f, 0.0f, 1.0f);
	float4 lightSpecular4 = float4(0.3f, 0.3f, 0.3f, 1.0f);

	float4 DiffuseLight = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4	Specular = float4(0.0f, 0.0f, 0.0f, 1.0f);
	for (int i = 0; i < 4; i++)
	//int i = 3;
	{
		float3 L;
		float4 LS;
		if (i == 0)
		{
			L = LIGHTDIR1;
			LS = lightSpecular1;
		}
		if (i == 1)
		{
			L = LIGHTDIR2;
			LS = lightSpecular2;
		}
		if (i == 2)
		{
			L = LIGHTDIR3;
			LS = lightSpecular3;
		}
		if (i == 3)
		{
			L = LIGHTDIR4;
			LS = lightSpecular4;
		}

		L = normalize(-L);
		//计算漫反射
		float DiffuseRatio = max(dot(L, NormalV), 0);
		DiffuseLight += LS * float4(1.0f, 1.0f, 1.0f, 1.0f) * DiffuseRatio;

		//计算镜面反射

		//计算半角向量
		float3 H = normalize(L + ToEyeDirV);

		//Phong光照
		//float4 Reflect = normalize(float4(g_LightDir.xyz - 2 * DiffuseRatio * NormalW, 1.0f));
		//float SpecularRatio = max(dot(Reflect, ToEyeDirW), 0);

		//Blinn-Phong光照
		float SpecularRatio = max(dot(NormalV, H), 0);
		float PoweredSpecular = pow(SpecularRatio, 24);
		Specular += LS * PoweredSpecular;

	}

	float4 AO = tex2D(g_sampleAO, TexCoord);
	//混合光照和纹理
	float4 finalColor = AO * (Ambient * Texture + DiffuseLight *Texture + Specular);
	//输出颜色
	//return AO;
	return finalColor;// float4(1.0f, 0.0f, 0.0f, 1.0f);
}

technique DeferredRender
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}