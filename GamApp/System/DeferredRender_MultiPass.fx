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
texture		g_ShadowBuffer;
texture		g_PositionBuffer;

float		g_zNear = 1.0f;
float		g_zFar = 1000.0f;

int			g_ShadowMapSize = 512;
float		g_ShadowBias = 0.2f;

float		g_MinVariance = 0.02;
float		g_Amount = 1.0f;

bool		g_bUseShadow;
float4		g_LightDir;
float4		g_LightColor;
float4		g_AmbientColor;

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

/*
sampler2D g_samplePosition =
sampler_state
{
	Texture = <g_PositionBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};
*/

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

matrix g_ShadowView;
matrix g_ShadowProj;
matrix g_invView;

//----------------------------
struct OutputVS
{
	float4 posWVP         : POSITION0;
	float2 TexCoord		: TEXCOORD0;
};


OutputVS VShader(float4 posL       : POSITION0,
				 float2 TexCoord : TEXCOORD0)
{
	OutputVS outVS = (OutputVS)0;

	//��������Ķ���λ�ã��������硢�۲졢ͶӰ����任��
	outVS.posWVP = mul(posL, g_WorldViewProj);

	outVS.TexCoord = TexCoord;

	return outVS;
}


float3 GetPosition(in float2 uv)
{
	//ʹ��positionBuffer����ȡλ�ã����Ƚϸߣ�������
	//return tex2D(g_samplePosition, uv).xyz;

	//ʹ������ؽ�λ����Ϣ�����Ƚϵͣ������С�����ڶ�λ���֣������ٶȺܺ�
	//�������ͼ����
	float4 NormalDepth = tex2D(g_sampleNormalDepth, uv);

	// ���ӿ������л�ȡ x/w �� y/w  
	float x = uv.x * 2.0f - 1;
	float y = (1 - uv.y) * 2.0f - 1.0f;
	//�����zֵ��ͶӰ��ķ��������
	float4 vProjectedPos = float4(x, y, NormalDepth.w, 1.0f);
	// ͨ��ת�õ�ͶӰ�������ת������ͼ�ռ�  
	float4 vPositionVS = mul(vProjectedPos, g_InverseProj);
	float3 vPositionVS3 = vPositionVS.xyz / vPositionVS.w;
	return vPositionVS3.xyz;
}



float4 AmbientPass(float2 TexCoord : TEXCOORD0) : COLOR
{
	//���㻷����
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

float4 GaussianBlur(int mapWidth, int mapHeight, float2 texCoords)
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
	color = tex2D(g_sampleShadow, texCoords + float2(-2 * stepU, -2 * stepV)) * (weights[0]);
	color += tex2D(g_sampleShadow, texCoords + float2(-1 * stepU, -2 * stepV)) * (weights[1]);
	color += tex2D(g_sampleShadow, texCoords + float2(0 * stepU, -2 * stepV)) * (weights[2]);
	color += tex2D(g_sampleShadow, texCoords + float2(1 * stepU, -2 * stepV)) * (weights[1]);
	color += tex2D(g_sampleShadow, texCoords + float2(2 * stepU, -2 * stepV)) * (weights[0]);

	color += tex2D(g_sampleShadow, texCoords + float2(-2 * stepU, -1 * stepV)) * (weights[1]);
	color += tex2D(g_sampleShadow, texCoords + float2(-1 * stepU, -1 * stepV)) * (weights[3]);
	color += tex2D(g_sampleShadow, texCoords + float2(0 * stepU, -1 * stepV)) * (weights[4]);
	color += tex2D(g_sampleShadow, texCoords + float2(1 * stepU, -1 * stepV)) * (weights[3]);
	color += tex2D(g_sampleShadow, texCoords + float2(2 * stepU, -1 * stepV)) * (weights[1]);

	color += tex2D(g_sampleShadow, texCoords + float2(-2 * stepU, 0 * stepV)) * (weights[2]);
	color += tex2D(g_sampleShadow, texCoords + float2(-1 * stepU, 0 * stepV)) * (weights[4]);
	color += tex2D(g_sampleShadow, texCoords + float2(0 * stepU, 0 * stepV)) * (weights[5]);
	color += tex2D(g_sampleShadow, texCoords + float2(1 * stepU, 0 * stepV)) * (weights[4]);
	color += tex2D(g_sampleShadow, texCoords + float2(2 * stepU, 0 * stepV)) * (weights[2]);

	color += tex2D(g_sampleShadow, texCoords + float2(-2 * stepU, 1 * stepV)) * (weights[1]);
	color += tex2D(g_sampleShadow, texCoords + float2(-1 * stepU, 1 * stepV)) * (weights[3]);
	color += tex2D(g_sampleShadow, texCoords + float2(0 * stepU, 1 * stepV)) * (weights[4]);
	color += tex2D(g_sampleShadow, texCoords + float2(1 * stepU, 1 * stepV)) * (weights[3]);
	color += tex2D(g_sampleShadow, texCoords + float2(2 * stepU, 1 * stepV)) * (weights[1]);

	color += tex2D(g_sampleShadow, texCoords + float2(-2 * stepU, 2 * stepV)) * (weights[0]);
	color += tex2D(g_sampleShadow, texCoords + float2(-1 * stepU, 2 * stepV)) * (weights[1]);
	color += tex2D(g_sampleShadow, texCoords + float2(0 * stepU, 2 * stepV)) * (weights[2]);
	color += tex2D(g_sampleShadow, texCoords + float2(1 * stepU, 2 * stepV)) * (weights[1]);
	color += tex2D(g_sampleShadow, texCoords + float2(2 * stepU, 2 * stepV)) * (weights[0]);

	return color;
}


float4 LightPass(float2 TexCoord : TEXCOORD0) : COLOR
{
	float4 NormalDepth = tex2D(g_sampleNormalDepth, TexCoord);
	float3 NormalV = normalize(NormalDepth.xyz * 2.0f - 1.0f);

	float3 pos = GetPosition(TexCoord);
	if (pos.z > g_zFar - 0.1)
		return float4(0, 0, 0, 1);

	float3 ToEyeDirV = normalize(pos * -1.0f);

	float4 DiffuseLight = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 SpecularLight = float4(0.0f, 0.0f, 0.0f, 1.0f);
	
	float3 toLight = g_LightDir.xyz;
	//float4 toLightV4 = mul(float4(toLight,1.0f), g_View);
	//toLight = toLightV4.xyz / toLightV4.w;

	toLight = normalize(-toLight);
	//����������
	float DiffuseRatio = max(dot(toLight, NormalV), 0);
	DiffuseLight += g_LightColor * float4(1.0f, 1.0f, 1.0f, 1.0f) * DiffuseRatio;

	//����������
	float3 H = normalize(toLight + ToEyeDirV);

	//Phong����
	//float4 Reflect = normalize(float4(g_LightDir.xyz - 2 * DiffuseRatio * NormalW, 1.0f));
	//float SpecularRatio = max(dot(Reflect, ToEyeDirW), 0);

	//Blinn-Phong����
	float SpecularRatio = max(dot(NormalV, H), 0);
	float PoweredSpecular = pow(SpecularRatio, 24);
	SpecularLight += g_LightColor * PoweredSpecular;

	float shadowFinal = 1.0f;
	if (g_bUseShadow)
	{
		//Shadow
		float4 worldPos = mul(float4(pos, 1.0f), g_invView);
		float4 lightViewPos = mul(worldPos, g_ShadowView);
		float4 lightProjPos = mul(lightViewPos, g_ShadowProj);
		float lightU = (lightProjPos.x / lightProjPos.w + 1.0f) / 2.0f;
		float lightV = (1.0f - lightProjPos.y / lightProjPos.w) / 2.0f;

		float2 ShadowTexCoord = float2(lightU, lightV);

		//float2 Moments = tex2D(g_sampleShadow, ShadowTexCoord);
		float2 Moments = GaussianBlur(g_ShadowMapSize, g_ShadowMapSize, ShadowTexCoord);

		shadowFinal = ChebyshevUpperBound(Moments, lightViewPos.z);
	}

	//��Ϲ��պ�����
	float4 finalColor = DiffuseLight +SpecularLight;
	return finalColor * shadowFinal;
}

float4 DiffusePass(float2 TexCoord : TEXCOORD0) : COLOR
{
	//��������
	float4 Texture = tex2D(g_sampleDiffuse, TexCoord);

	//AO
	float4 AO = tex2D(g_sampleAO, TexCoord);

	return Texture * AO;
}

float4 ShadowVSMPass(float2 TexCoord : TEXCOORD0) : COLOR
{
	//Shadow
	float3 pos = GetPosition(TexCoord);
	float4 worldPos = mul(float4(pos, 1.0f), g_invView);
	float4 lightViewPos = mul(worldPos, g_ShadowView);
	float4 lightProjPos = mul(lightViewPos, g_ShadowProj);
	float lightU = (lightProjPos.x / lightProjPos.w + 1.0f) / 2.0f;
	float lightV = (1.0f - lightProjPos.y / lightProjPos.w) / 2.0f;

	float2 ShadowTexCoord = float2(lightU, lightV);

	//float2 Moments = tex2D(g_sampleShadow, ShadowTexCoord);
	float2 Moments = GaussianBlur(g_ShadowMapSize, g_ShadowMapSize, ShadowTexCoord);

	float shadowFinal = ChebyshevUpperBound(Moments, lightViewPos.z);

	return float4(shadowFinal, shadowFinal, shadowFinal, 1.0f);
}

float4 ShadowPass(float2 TexCoord : TEXCOORD0) : COLOR
{
	//Shadow
	float3 pos = GetPosition(TexCoord);
	float4 worldPos = mul(float4(pos, 1.0f), g_invView);
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

	float shadowFinal = lerp(lerp(shadowVal[0], shadowVal[1], lerps.x), lerp(shadowVal[2], shadowVal[3], lerps.x), lerps.y);

	return float4(shadowFinal, shadowFinal, shadowFinal,1.0f);
}

float4 DebugPass(float2 TexCoord : TEXCOORD0) : COLOR
{
	//DIffuse
	float4 Texture = tex2D(g_sampleDiffuse, TexCoord);
	//AO
	float4 AO = tex2D(g_sampleAO, TexCoord);
	//ShadowMap
	float4 Shadow = tex2D(g_sampleShadow, TexCoord);
	//NormalDepth
	float4 NormalDepth = tex2D(g_sampleNormalDepth, TexCoord);

	//return Texture;
	//return AO;
	return float4(Shadow.x / 100, Shadow.x / 100, Shadow.x / 100, 1.0f);
	//return NormalDepth;
	return float4(NormalDepth.w, NormalDepth.w, NormalDepth.w,1.0f);
}

technique DeferredRender
{
	pass p0 //��Ⱦ�ƹ�
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 LightPass();
		AlphaBlendEnable = true;                        //������Ⱦ״̬        
		SrcBlend = ONE;
		DestBlend = ONE;
	}
	pass p1 //���ӻ�����
	{
		pixelShader = compile ps_3_0 AmbientPass();
		AlphaBlendEnable = true;                        //������Ⱦ״̬        
		SrcBlend = ONE;
		DestBlend = ONE;
	}
	pass p2 //������AO
	{
		pixelShader = compile ps_3_0 DiffusePass();
		AlphaBlendEnable = true;                        //������Ⱦ״̬        
		SrcBlend = ZERO;
		DestBlend = SrcColor;
	}
	pass p3 //DEBUG PASS
	{
		pixelShader = compile ps_3_0 DebugPass();
		//pixelShader = compile ps_3_0 ShadowVSMPass();
		//pixelShader = compile ps_3_0 AmbientPass();
		//pixelShader = compile ps_3_0 DiffusePass();
		AlphaBlendEnable = true;                        //������Ⱦ״̬        
		SrcBlend = ONE;
		DestBlend = ZERO;
	}
	
}