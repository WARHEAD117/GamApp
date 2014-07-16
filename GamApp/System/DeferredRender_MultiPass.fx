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

texture		g_ShadowBuffer;
texture		g_PointShadowBuffer;

texture		g_ShadowResult;

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
matrix g_ToViewDirMatrix;
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

sampler2D g_samplePosition =
sampler_state
{
	Texture = <g_PositionBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};


//----------------------------

sampler g_sampleShadow =
sampler_state
{
	Texture = <g_ShadowBuffer>;
	MinFilter = ANISOTROPIC;
	MagFilter = ANISOTROPIC;
	MipFilter = ANISOTROPIC;// ANISOTROPIC;
	AddressU = Clamp;
	AddressV = Clamp;
};

sampler2D g_sampleShadowResult =
sampler_state
{
	Texture = <g_ShadowResult>;
	MinFilter = GAUSSIANQUAD;
	MagFilter = GAUSSIANQUAD;
	MipFilter = GAUSSIANQUAD;// ANISOTROPIC;
	AddressU = Clamp;
	AddressV = Clamp;
}; 

texture g_LightBuffer;
sampler2D g_sampleLight =
sampler_state
{
	Texture = <g_LightBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
	AddressU = Clamp;
	AddressV = Clamp;
};

texture g_SpecularLightBuffer;
sampler2D g_sampleSpecularLight =
sampler_state
{
	Texture = <g_SpecularLightBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
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
	float4 viewDir		: TEXCOORD1;
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

	outVS.viewDir = mul(posL, g_ToViewDirMatrix);
	return outVS;
}

float3 GetNormal(in float2 uv)
{
	return normalize(tex2D(g_sampleNormal, uv).xyz * 2.0f - 1.0f);
}
/*
float3 GetPosition(in float2 uv)
{
	//使用positionBuffer来获取位置，精度较高，但是要占用三个通道，必须使用128位纹理，太大太慢
	return tex2D(g_samplePosition, uv).xyz;

	//使用投影深度重建位置信息，精度较低，误差在小数点后第二位出现，但是速度很好。
	float DepthP = tex2D(g_samplePosition, uv).w;

	// 从视口坐标中获取 x/w 和 y/w  
	float x = uv.x * 2.0f - 1.0f;
	float y = 1.0f - uv.y * 2.0f;
	//这里的z值是投影后的非线性深度
	float4 vProjectedPos = float4(x, y, DepthP, 1.0f);
	// 通过转置的投影矩阵进行转换到视图空间  
	float4 vPositionVS = mul(vProjectedPos, g_InverseProj);
	float3 vPositionVS3 = vPositionVS.xyz / vPositionVS.w;
	float q = g_zFar / (g_zFar - g_zNear);
	vPositionVS3.z = (g_zNear * q) / (q - DepthP);
	vPositionVS3.z = tex2D(g_samplePosition, uv).z;
	return vPositionVS3.xyz;
}
*/
float3 GetPosition(in float2 uv, in float4 viewDir)
{
	float DepthV = tex2D(g_samplePosition, uv).z;

	float3 pos = viewDir * ((DepthV) / viewDir.z);

	return pos;
}

float GetDepth(in float2 uv)
{
	float DepthV = tex2D(g_samplePosition, uv).z;

	return DepthV;
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
	//裁减掉背光的像素
	float NL = dot(toLight, normal);
	clip(NL);
	//计算漫反射光照
	float DiffuseRatio = max(NL, 0.0);
	DiffuseLight = lightColor * DiffuseRatio;

	//Blinn-Phong光照，已经是基于物理的了
	//计算半角向量
	float3 H = normalize(toLight + toEye);

	//遮挡项
	float lh = dot(toLight, H);
	float G = 1 / (lh*lh);

	float shininess = 50.05f;
	float SpecularRatio = max(dot(normal, H), 0.0);
	//对Blinn-Phong的高光归一化，保证反射的能量守恒
	float PoweredSpecular = pow(SpecularRatio, shininess) * (shininess + 2.0f) / 8.0f;

	SpecularLight = lightColor * PoweredSpecular * DiffuseRatio * G;
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
			
		float2 Moments = texCUBE(g_sampleShadow, normalize(worldToObj.xyz));

			
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

//光照结果的MRT输出
struct OutputPS
{
	float4 diffuseLight		: COLOR0;
	float4 specularLight	: COLOR1;
};

OutputPS DirectionLightPass(float4 posWVP : TEXCOORD0, float4 viewDir : TEXCOORD1)
{
	OutputPS outPs;
	outPs.diffuseLight = 0;
	outPs.specularLight = 0;

	float lightU = (posWVP.x / posWVP.w + 1.0f) / 2.0f + 0.5f / g_ScreenWidth;
	float lightV = (1.0f - posWVP.y / posWVP.w) / 2.0f + 0.5f / g_ScreenHeight;
	float2 TexCoord = float2(lightU, lightV);
	float3 Normal = GetNormal(TexCoord);

	float3 pos = GetPosition(TexCoord, viewDir);

	//加入灯光体和stencil剔除后之后就可以省略掉这个判断了
	if (pos.z > g_zFar)
		return outPs;

	float3 ToEyeDirV = normalize(pos * -1.0f);
	float3 toLight = normalize(-g_LightDir.xyz);

	float4 DiffuseLight = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 SpecularLight = float4(0.0f, 0.0f, 0.0f, 0.0f);

	LightFunc(Normal, toLight, ToEyeDirV, g_LightColor, DiffuseLight, SpecularLight);

	float shadowFinal = 1.0f;
	//shadowFinal = ShadowFunc(g_bUseShadow, pos);

	//阴影图采样
	shadowFinal = tex2D(g_sampleShadowResult, TexCoord);

	//MRT输出光照结果
	outPs.diffuseLight = DiffuseLight * shadowFinal.x;
	outPs.specularLight = SpecularLight * shadowFinal.x;
	return outPs;
}

OutputPS PointLightPass(float4 posWVP : TEXCOORD0, float4 viewDir : TEXCOORD1)
{
	OutputPS outPs;
	outPs.diffuseLight = 0;
	outPs.specularLight = 0;

	float lightU = (posWVP.x / posWVP.w + 1.0f) / 2.0f + 0.5f / g_ScreenWidth;
	float lightV = (1.0f - posWVP.y / posWVP.w) / 2.0f + 0.5f / g_ScreenHeight;
	float2 TexCoord = float2(lightU, lightV);

	float3 Normal = GetNormal(TexCoord);

	float3 pos = GetPosition(TexCoord, viewDir);
	
	//加入灯光体和stencil剔除后之后就可以省略掉这个判断了
	if (pos.z > g_zFar)
		return outPs;

	float3 ToEyeDirV = normalize(pos * -1.0f);

	float4 DiffuseLight = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 SpecularLight = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 toLight = g_LightPos.xyz - pos;
	float toLightDistance = length(toLight);
	
	//加入灯光体和stencil剔除后之后就可以省略掉这个判断了
	if (toLightDistance > g_LightRange)
		return outPs;

	float disRange = clamp(toLightDistance / g_LightRange, 0.0f, 1.0f);


	float attenuation = (1.0f - disRange) / (g_LightAttenuation.x + g_LightAttenuation.y * disRange + g_LightAttenuation.z * disRange * disRange + 0.000001f);
	//attenuation = 1 - dot(toLight / g_LightRange, toLight / g_LightRange);

	float4 lightColor = attenuation * g_LightColor;

	toLight = normalize(toLight);
	LightFunc(Normal, toLight, ToEyeDirV, lightColor, DiffuseLight, SpecularLight);

	float shadowFinal = 1.0f;
	//shadowFinal = PointShadowFunc(g_bUseShadow, pos);

	//阴影图采样，因为点光源分辨率小，所以模糊一次
	//shadowFinal = tex2D(g_sampleShadowResult, TexCoord);
	shadowFinal = GaussianBlur(g_ScreenWidth, g_ScreenHeight, g_sampleShadowResult, TexCoord);

	//MRT输出光照结果
	outPs.diffuseLight = DiffuseLight * shadowFinal.x;
	outPs.specularLight = SpecularLight * shadowFinal.x;
	return outPs;
}

OutputPS SpotLightPass(float4 posWVP : TEXCOORD0, float4 viewDir : TEXCOORD1)
{
	OutputPS outPs;
	outPs.diffuseLight = 0;
	outPs.specularLight = 0;

	float lightU = (posWVP.x / posWVP.w + 1.0f) / 2.0f + 0.5f / g_ScreenWidth;
	float lightV = (1.0f - posWVP.y / posWVP.w) / 2.0f + 0.5f / g_ScreenHeight;
	float2 TexCoord = float2(lightU, lightV);

	float3 Normal = GetNormal(TexCoord);

	float3 pos = GetPosition(TexCoord, viewDir);
	
	//加入灯光体和stencil剔除后之后就可以省略掉这个判断了
	if (pos.z > g_zFar)
		return outPs;

	float3 ToEyeDirV = normalize(pos * -1.0f);

	float4 DiffuseLight = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 SpecularLight = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 toLight = g_LightPos.xyz - pos;

	float toLightDistance = length(toLight);
	
	float disRange = clamp(toLightDistance / g_LightRange, 0.0f, 1.0f);

	float attenuation = (1.0f - disRange) / (g_LightAttenuation.x + g_LightAttenuation.y * disRange + g_LightAttenuation.z * disRange * disRange + 0.000001f);
	//不使用衰减参数，直接衰减到边缘
	//attenuation = 1 - dot(toLight / g_LightRange, toLight / g_LightRange);
	
	toLight = normalize(toLight);

	//光源到像素的向量与光源方向的夹角
	float cosAlpha = dot(-toLight, g_LightDir);
	//内锥角的一半的cos
	float cosTheta = g_LightCosAngle.y;
	//外锥角的一半的cos
	float cosPhi = g_LightCosAngle.x;

	//聚光灯外角以外和范围以外都没有光照(加入灯光体之后就可以省略掉这个判断了)
	if (toLightDistance > g_LightRange || cosAlpha < cosPhi)
		return outPs;
	
	float falloff = 1.0f;
	if (cosAlpha < cosTheta)
	{
		falloff = (cosAlpha - cosPhi) / (cosTheta - cosPhi);
	}
	//下面的公式可以简化掉if，但是看起来没有效率提升
	//falloff = 1 - (cosAlpha < cosTheta)* (1 - (cosAlpha - cosPhi) / (cosTheta - cosPhi));
	
	//灯光falloff的指数衰减
	//TODO：

	float4 lightColor = attenuation * g_LightColor * falloff;

	LightFunc(Normal, toLight, ToEyeDirV, lightColor, DiffuseLight, SpecularLight);

	float shadowFinal = 1.0f;
	//shadowFinal = ShadowFunc(g_bUseShadow, pos);
	//阴影图采样
	shadowFinal = tex2D(g_sampleShadowResult, TexCoord);

	//MRT输出光照结果
	outPs.diffuseLight = DiffuseLight * shadowFinal.x;
	outPs.specularLight = SpecularLight * shadowFinal.x;
	return outPs;
}

float4 ShadowPass(float4 posWVP : TEXCOORD0, float4 viewDir : TEXCOORD1) : COLOR
{
	//return float4(0.5, 0.0f, 0.0f, 1.0f);

	float lightU = (posWVP.x / posWVP.w + 1.0f) / 2.0f + 0.5f / g_ScreenWidth;
	float lightV = (1.0f - posWVP.y / posWVP.w) / 2.0f + 0.5f / g_ScreenHeight;
	float2 TexCoord = float2(lightU, lightV);
	float3 pos = GetPosition(TexCoord, viewDir);


	float shadowFinal = 1.0f;
	shadowFinal = ShadowFunc(g_bUseShadow, pos);

	//输出阴影结果
	float4 finalColor = float4(shadowFinal, shadowFinal, shadowFinal, shadowFinal);
	return finalColor;
}

float4 PointShadowPass(float4 posWVP : TEXCOORD0, float4 viewDir : TEXCOORD1) : COLOR
{
	//return float4(0.5, 0.0f, 0.0f, 1.0f);

	float lightU = (posWVP.x / posWVP.w + 1.0f) / 2.0f + 0.5f / g_ScreenWidth;
	float lightV = (1.0f - posWVP.y / posWVP.w) / 2.0f + 0.5f / g_ScreenHeight;
	float2 TexCoord = float2(lightU, lightV);

	float3 pos = GetPosition(TexCoord, viewDir);


	float shadowFinal = 1.0f;
	shadowFinal = PointShadowFunc(g_bUseShadow, pos);

	//输出阴影结果
	float4 finalColor = float4(shadowFinal, shadowFinal, shadowFinal, shadowFinal);
	return finalColor;
}

float4 AmbientPass(float2 TexCoord : TEXCOORD0) : COLOR
{
	//计算环境光
	float4 Ambient = g_AmbientColor;

	//高亮天空盒（实际上应该用HDR天空盒）
	Ambient = GetDepth(TexCoord) > g_zFar ? float4(1,1,1,1): Ambient;

	return Ambient;
}

float4 ShadingPass(float2 TexCoord : TEXCOORD0) : COLOR
{
	//采样光照结果
	float4 DiffuseLightResult = tex2D(g_sampleLight, TexCoord);
	float4 SpecularLightResult = tex2D(g_sampleSpecularLight, TexCoord);

	float4 diffuseLight = float4(DiffuseLightResult.xyz, 1.0f);
	float4 specularLight = float4(SpecularLightResult.xyz, 1.0f);

	//纹理采样
	float4 DiffuseBuffer = tex2D(g_sampleDiffuse, TexCoord);
	//纹理颜色
	float4 Texture = float4(DiffuseBuffer.rgb, 1.0f);
	//高光强度
	float  Specularintensity = DiffuseBuffer.a;

	//计算最终光照
	return Texture * diffuseLight + specularLight * Specularintensity;
}

float4 DebugPass(float2 TexCoord : TEXCOORD0) : COLOR
{
	//DIffuse
	float4 Texture = tex2D(g_sampleDiffuse, TexCoord);
	//ShadowMap
	float4 Shadow = tex2D(g_sampleShadow, TexCoord);
	//Normal
	float4 Normal = tex2D(g_sampleNormal, TexCoord);

	return Texture;
	//return float4(Shadow.x / 100, Shadow.x / 100, Shadow.x / 100, 1.0f);
	//return float4(GetNormal(TexCoord),1.0f);
	return Normal;
}

technique DeferredRender
{
	pass p0 //渲染灯光
	{
		vertexShader = compile vs_3_0 VShaderLightVolume();
		pixelShader = compile ps_3_0 DirectionLightPass();
		AlphaBlendEnable = true;                        //设置渲染状态        
		SrcBlend = ONE;
		DestBlend = ONE;
	}

	pass p1 //PointLight
	{
		vertexShader = compile vs_3_0 VShaderLightVolume();
		pixelShader = compile ps_3_0 PointLightPass();
		AlphaBlendEnable = true;                        //设置渲染状态        
		SrcBlend = ONE;
		DestBlend = ONE;
	}

	pass p2 //SpotLight
	{
		vertexShader = compile vs_3_0 VShaderLightVolume();
		pixelShader = compile ps_3_0 SpotLightPass();
		AlphaBlendEnable = true;                        //设置渲染状态        
		SrcBlend = ONE;
		DestBlend = ONE;
	}

	pass p3 //DirShadow
	{
		vertexShader = compile vs_3_0 VShaderLightVolume();
		pixelShader = compile ps_3_0 ShadowPass();
		AlphaBlendEnable = false;
	}

	pass p4 //PointShadow
	{
		vertexShader = compile vs_3_0 VShaderLightVolume();
		pixelShader = compile ps_3_0 PointShadowPass();
		AlphaBlendEnable = false;
	}

	pass p5 //添加环境光
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 AmbientPass();
		AlphaBlendEnable = true;                        //设置渲染状态        
		SrcBlend = ONE;
		DestBlend = ONE;
	}
	pass p6 //着色
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 ShadingPass();
		AlphaBlendEnable = false;                        //设置渲染状态        
		//SrcBlend = ZERO;
		//DestBlend = SrcColor;
	}
	pass p7 //DEBUG PASS
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 DebugPass();
		//pixelShader = compile ps_3_0 AmbientPass();
		//pixelShader = compile ps_3_0 DiffusePass();
		AlphaBlendEnable = true;                        //设置渲染状态        
		SrcBlend = ONE;
		DestBlend = ZERO;
	}
	
}