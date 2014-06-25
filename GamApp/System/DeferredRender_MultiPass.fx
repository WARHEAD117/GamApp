matrix		g_World;
matrix		g_View;
matrix		g_Proj;
matrix		g_ViewProj;
matrix		g_WorldViewProj;
matrix		g_mWorldInv;
matrix		g_InverseProj;

texture		g_DiffuseBuffer;
texture		g_NormalBuffer;
texture		g_PositionBuffer;

texture		g_AOBuffer;

texture		g_ShadowBuffer;
texture		g_PointShadowBuffer;

float		g_zNear = 1.0f;
float		g_zFar = 1000.0f;

int			g_ShadowMapSize = 512;
float		g_ShadowBias = 0.2f;

float		g_MinVariance = 0.02;
float		g_Amount = 1.0f;

float4		g_LightDir;
float4		g_LightPos;
float		g_LightRange;
float4		g_LightCosAngle;
float4		g_LightAttenuation;

int g_ScreenWidth;
int g_ScreenHeight;
//lightVolume================
matrix g_LightVolumeWVP;
//===========================

bool		g_bUseShadow;
float4		g_LightColor;
float4		g_AmbientColor;

sampler2D g_sampleDiffuse =
sampler_state
{
	Texture = <g_DiffuseBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

sampler2D g_sampleNormal =
sampler_state
{
	Texture = <g_NormalBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

sampler2D g_sampleAO =
sampler_state
{
	Texture = <g_AOBuffer>;
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


//----------------------------

sampler2D g_sampleShadow =
sampler_state
{
	Texture = <g_ShadowBuffer>;
	MinFilter = ANISOTROPIC;
	MagFilter = ANISOTROPIC;
	MipFilter = ANISOTROPIC;// ANISOTROPIC;
	AddressU = Clamp;
	AddressV = Clamp;
};

sampler g_samplePointShadow =
sampler_state
{
	Texture = <g_PointShadowBuffer>;
	MinFilter = ANISOTROPIC;
	MagFilter = ANISOTROPIC;
	MipFilter = ANISOTROPIC;// ANISOTROPIC;
	AddressU = Clamp;
	AddressV = Clamp;
};

matrix g_ShadowView;
matrix g_ShadowProj;
matrix g_invView;

//----------------------------
struct OutputVS
{
	float4 posWVP         : POSITION0;
	float2 TexCoord		: TEXCOORD0;
};
struct OutputVS2
{
	float4 posWVP         : POSITION0;
	float4 TexCoord		: TEXCOORD0;
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

OutputVS2 VShaderLightVolume(float4 posL       : POSITION0)
{
	OutputVS2 outVS = (OutputVS2)0;

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	outVS.posWVP = mul(posL, g_LightVolumeWVP);

	outVS.TexCoord = outVS.posWVP;

	return outVS;
}

float3 GetNormal(in float2 uv)
{
	return normalize(tex2D(g_sampleNormal, uv).xyz * 2.0f - 1.0f);
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



float4 AmbientPass(float2 TexCoord : TEXCOORD0) : COLOR
{
	//计算环境光
	float4 Ambient = g_AmbientColor;

	return Ambient;
}


float linstep(float min, float max, float v)
{
	return clamp((v - min) / (max - min), 0, 1);
}

float ReduceLightBleeding(float p_max, float Amount)
{
	return linstep(Amount, 1, p_max);
}

float ChebyshevUpperBound(float2 Moments, float t)
{
	float p = (t <= Moments.x);

	float Variance = Moments.y - (Moments.x*Moments.x);
	Variance = max(Variance, g_MinVariance);

	float d = t - Moments.x;
	float p_max = Variance / (Variance + d*d);

	p_max = ReduceLightBleeding(p_max, g_Amount);

	return max(p, p_max);
}

float4 GaussianBlur(int mapWidth, int mapHeight, sampler2D texSampler, float2 texCoords)
{
	float weights[6] = { 0.00078633, 0.00655965, 0.01330373, 0.05472157, 0.11098164, 0.22508352 };

	float4 color;
	float stepU = 1.0f / mapWidth;
	float stepV = 1.0f / mapHeight;

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

void LightFunc(float3 normal, float3 toLight, float3 toEye, float4 lightColor, inout float4 DiffuseLight, inout float4 SpecularLight)
{
	//计算漫反射
	float DiffuseRatio = max(dot(toLight, normal), 0);
	DiffuseLight += lightColor * DiffuseRatio;

	//Blinn-Phong光照
	//计算半角向量
	float3 H = normalize(toLight + toEye);

	float SpecularRatio = max(dot(normal, H), 0);
	float PoweredSpecular = pow(SpecularRatio, 24);
	SpecularLight += lightColor * PoweredSpecular;
}

float ShadowFunc(bool useShadow, float3 objViewPos)
{
	float shadowContribute = 1.0f;
	if (useShadow)
	{
		//Shadow
		float4 worldPos = mul(float4(objViewPos, 1.0f), g_invView);
		float4 lightViewPos = mul(worldPos, g_ShadowView);
		float4 lightProjPos = mul(lightViewPos, g_ShadowProj);
		float lightU = (lightProjPos.x / lightProjPos.w + 1.0f) / 2.0f;
		float lightV = (1.0f - lightProjPos.y / lightProjPos.w) / 2.0f;

		float2 ShadowTexCoord = float2(lightU, lightV);

		//float2 Moments = tex2D(g_sampleShadow, ShadowTexCoord);
		float2 Moments = GaussianBlur(g_ShadowMapSize, g_ShadowMapSize, g_sampleShadow, ShadowTexCoord);

		shadowContribute = ChebyshevUpperBound(Moments, lightViewPos.z);
	}
	return shadowContribute;
}

float PointShadowFunc(bool useShadow, float3 objViewPos)
{
	float shadowContribute = 1.0f;
	if (useShadow)
	{
		//Shadow
		float3 toObj = objViewPos - g_LightPos.xyz;

		float4 worldToObj = mul(float4(toObj, 0.0f), g_invView);

		//float2 Moments = tex2D(g_sampleShadow, ShadowTexCoord);
		//float2 Moments = GaussianBlur(g_ShadowMapSize, g_ShadowMapSize, g_sampleShadow, ShadowTexCoord);
			
		float2 Moments = texCUBE(g_samplePointShadow, normalize(worldToObj.xyz));

			
		shadowContribute = ChebyshevUpperBound(Moments, length(worldToObj.xyz));
	}
	return shadowContribute;
}

float PCFShadow(bool useShadow, float3 objViewPos)
{
	float shadowContribute = 1.0f;
	if (useShadow)
	{
		//Shadow
		float4 worldPos = mul(float4(objViewPos, 1.0f), g_invView);
		float4 lightViewPos = mul(worldPos, g_ShadowView);
		float4 lightProjPos = mul(lightViewPos, g_ShadowProj);
		float lightU = (lightProjPos.x / lightProjPos.w + 1.0f) / 2.0f;
		float lightV = (1.0f - lightProjPos.y / lightProjPos.w) / 2.0f;

		int SHADOWMAP_SIZE = g_ShadowMapSize;
		float2 ShadowTexCoord = float2(lightU, lightV);
		float2 texturePos = ShadowTexCoord * SHADOWMAP_SIZE;
		float2 lerps = frac(texturePos);

		float shadowBias = g_ShadowBias;

		float shadowVal[4];
		shadowVal[0] = (tex2D(g_sampleShadow, ShadowTexCoord) + shadowBias < lightViewPos.z / lightViewPos.w) ? 0.0f : 1.0f;
		shadowVal[1] = (tex2D(g_sampleShadow, ShadowTexCoord + float2(1.0f / SHADOWMAP_SIZE, 0.0f)) + shadowBias < lightViewPos.z / lightViewPos.w) ? 0.0f : 1.0f;
		shadowVal[2] = (tex2D(g_sampleShadow, ShadowTexCoord + float2(0.0f, 1.0f / SHADOWMAP_SIZE)) + shadowBias < lightViewPos.z / lightViewPos.w) ? 0.0f : 1.0f;
		shadowVal[3] = (tex2D(g_sampleShadow, ShadowTexCoord + float2(1.0f / SHADOWMAP_SIZE, 1.0f / SHADOWMAP_SIZE)) + shadowBias < lightViewPos.z / lightViewPos.w) ? 0.0f : 1.0f;

		shadowContribute = lerp(lerp(shadowVal[0], shadowVal[1], lerps.x), lerp(shadowVal[2], shadowVal[3], lerps.x), lerps.y);

	}
	return shadowContribute;
}

float4 DirectionLightPass(float2 TexCoord : TEXCOORD0) : COLOR
{
	float3 Normal = GetNormal(TexCoord);

	float3 pos = GetPosition(TexCoord);

	if (pos.z > g_zFar - 0.1)
		return float4(0, 0, 0, 1);

	float3 ToEyeDirV = normalize(pos * -1.0f);
	float3 toLight = normalize(-g_LightDir.xyz);


	float4 DiffuseLight = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 SpecularLight = float4(0.0f, 0.0f, 0.0f, 1.0f);

	LightFunc(Normal, toLight, ToEyeDirV, g_LightColor, DiffuseLight, SpecularLight);

	float shadowFinal = 1.0f;
	shadowFinal = ShadowFunc(g_bUseShadow, pos);

	//混合光照和纹理
	float4 finalColor = DiffuseLight +SpecularLight;
	return finalColor * shadowFinal;
}

float4 PointLightPass(float4 posWVP : TEXCOORD0) : COLOR
{
	//return float4(0.0, 0.5f, 0.0f, 1.0f);

	float lightU = (posWVP.x / posWVP.w + 1.0f) / 2.0f + 0.5f / g_ScreenWidth;
	float lightV = (1.0f - posWVP.y / posWVP.w) / 2.0f + 0.5f / g_ScreenHeight;
	float2 TexCoord = float2(lightU, lightV);

	float3 Normal = GetNormal(TexCoord);

	float3 pos = GetPosition(TexCoord);
	
	//加入灯光体之后就可以省略掉这个判断了
	//if (pos.z > g_zFar - 0.1)
	//	return float4(0, 0, 0, 1);

	float3 ToEyeDirV = normalize(pos * -1.0f);

	float4 DiffuseLight = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 SpecularLight = float4(0.0f, 0.0f, 0.0f, 1.0f);

	float3 toLight = g_LightPos.xyz - pos;
	float toLightDistance = length(toLight);
	
	//加入灯光体之后就可以省略掉这个判断了
	//if (toLightDistance > g_LightRange)
	//	return float4(0.0f, 0.0f, 0.0f, 1.0f);

	float disRange = clamp(toLightDistance / g_LightRange, 0.0f, 1.0f);


	float attenuation = (1.0f - disRange) / (g_LightAttenuation.x + g_LightAttenuation.y * disRange + g_LightAttenuation.z * disRange * disRange);
	//attenuation = 1 - dot(toLight / g_LightRange, toLight / g_LightRange);

	float4 lightColor = attenuation * g_LightColor;

	toLight = normalize(toLight);
	LightFunc(Normal, toLight, ToEyeDirV, lightColor, DiffuseLight, SpecularLight);

	float shadowFinal = 1.0f;
	shadowFinal = PointShadowFunc(g_bUseShadow, pos);

	//混合光照和纹理
	float4 finalColor = DiffuseLight + SpecularLight;
		return finalColor * shadowFinal;
}

float4 SpotLightPass(float4 posWVP : TEXCOORD0) : COLOR
{
	//return float4(0.5, 0.0f, 0.0f, 1.0f);

	float lightU = (posWVP.x / posWVP.w + 1.0f) / 2.0f + 0.5f / g_ScreenWidth;
	float lightV = (1.0f - posWVP.y / posWVP.w) / 2.0f + 0.5f / g_ScreenHeight;
	float2 TexCoord = float2(lightU, lightV);

	float3 Normal = GetNormal(TexCoord);

	float3 pos = GetPosition(TexCoord);
	
	//if (pos.z > g_zFar - 0.1)
	//	return float4(0, 0, 0, 1);

	float3 ToEyeDirV = normalize(pos * -1.0f);

	float4 DiffuseLight = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 SpecularLight = float4(0.0f, 0.0f, 0.0f, 1.0f);

	float3 toLight = g_LightPos.xyz - pos;
	float toLightDistance = length(toLight);
	
	float disRange = clamp(toLightDistance / g_LightRange, 0.0f, 1.0f);

	float attenuation = (1.0f - disRange) / (g_LightAttenuation.x + g_LightAttenuation.y * disRange + g_LightAttenuation.z * disRange * disRange);
	//attenuation = 1 - dot(toLight / g_LightRange, toLight / g_LightRange);
	
	toLight = normalize(toLight);

	//光源到像素的向量与光源方向的夹角
	float cosAlpha = dot(-toLight, g_LightDir);
	//内锥角的一半的cos
	float cosTheta = g_LightCosAngle.y;
	//外锥角的一半的cos
	float cosPhi = g_LightCosAngle.x;

	//聚光灯外角以外和范围以外都没有光照(加入灯光体之后就可以省略掉这个判断了)
	if (/*toLightDistance > g_LightRange || */cosAlpha < cosPhi)
		return float4(0.0f, 0.0f, 0.0f, 1.0f);
	
	float falloff = 1.0f;
	if (cosAlpha < cosTheta)
	{
		falloff = (cosAlpha - cosPhi) / (cosTheta - cosPhi);
	}
	//下面的公式可以简化掉if，但是看起来没有效率提升
	//falloff = 1 - (cosAlpha < cosTheta)* (1 - (cosAlpha - cosPhi) / (cosTheta - cosPhi));
	
	
	float4 lightColor = attenuation * g_LightColor * falloff;

	LightFunc(Normal, toLight, ToEyeDirV, lightColor, DiffuseLight, SpecularLight);

	float shadowFinal = 1.0f;
	shadowFinal = ShadowFunc(g_bUseShadow, pos);

	//混合光照和纹理
	float4 finalColor = DiffuseLight + SpecularLight;
	return finalColor * shadowFinal;
}

float4 DiffusePass(float2 TexCoord : TEXCOORD0) : COLOR
{
	//纹理采样
	float4 Texture = tex2D(g_sampleDiffuse, TexCoord);

	//AO
	float4 AO = tex2D(g_sampleAO, TexCoord);

	return Texture * AO;
}

float4 DebugPass(float2 TexCoord : TEXCOORD0) : COLOR
{
	//DIffuse
	float4 Texture = tex2D(g_sampleDiffuse, TexCoord);
	//AO
	float4 AO = tex2D(g_sampleAO, TexCoord);
	//ShadowMap
	float4 Shadow = tex2D(g_sampleShadow, TexCoord);
	//Normal
	float4 Normal = tex2D(g_sampleNormal, TexCoord);

	//return Texture;
	//return AO;
	return float4(Shadow.x / 100, Shadow.x / 100, Shadow.x / 100, 1.0f);
	return Normal;
}

technique DeferredRender
{
	pass p0 //渲染灯光
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 DirectionLightPass();
		AlphaBlendEnable = true;                        //设置渲染状态        
		SrcBlend = ONE;
		DestBlend = ONE;
	}
	pass p1 //添加环境光
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 AmbientPass();
		AlphaBlendEnable = true;                        //设置渲染状态        
		SrcBlend = ONE;
		DestBlend = ONE;
	}
	pass p2 //纹理及AO
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 DiffusePass();
		AlphaBlendEnable = true;                        //设置渲染状态        
		SrcBlend = ZERO;
		DestBlend = SrcColor;
	}
	pass p3 //DEBUG PASS
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 DebugPass();
		//pixelShader = compile ps_3_0 AmbientPass();
		//pixelShader = compile ps_3_0 DiffusePass();
		AlphaBlendEnable = true;                        //设置渲染状态        
		SrcBlend = ONE;
		DestBlend = ZERO;
	}
	pass p4 //PointLight
	{
		vertexShader = compile vs_3_0 VShaderLightVolume();
		pixelShader = compile ps_3_0 PointLightPass();
		AlphaBlendEnable = true;                        //设置渲染状态        
		SrcBlend = ONE;
		DestBlend = ONE;
	}
	pass p5 //SpotLight
	{
		vertexShader = compile vs_3_0 VShaderLightVolume();
		pixelShader = compile ps_3_0 SpotLightPass();
		AlphaBlendEnable = true;                        //设置渲染状态        
		SrcBlend = ONE;
		DestBlend = ONE;
	}
	
}