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
#define		LIGHTCOUNT 32
float4		g_LightDirArray[LIGHTCOUNT];
float4		g_LightColorArray[LIGHTCOUNT];
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
	MipFilter = ANISOTROPIC;
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
	
	for (int i = 0; i < LIGHTCOUNT; i++)
	{
		float3 toLight = g_LightDirArray[i];
		//float4 toLightV4 = mul(float4(toLight,1.0f), g_View);
		//toLight = toLightV4.xyz / toLightV4.w;

		toLight = normalize(-toLight);
		//����������
		float DiffuseRatio = max(dot(toLight, NormalV), 0);
		DiffuseLight += g_LightColorArray[i] * float4(1.0f, 1.0f, 1.0f, 1.0f) * DiffuseRatio;

		//����������
		float3 H = normalize(toLight + ToEyeDirV);

		//Phong����
		//float4 Reflect = normalize(float4(g_LightDir.xyz - 2 * DiffuseRatio * NormalW, 1.0f));
		//float SpecularRatio = max(dot(Reflect, ToEyeDirW), 0);

		//Blinn-Phong����
		float SpecularRatio = max(dot(NormalV, H), 0);
		float PoweredSpecular = pow(SpecularRatio, 24);
		SpecularLight += g_LightColorArray[i] * PoweredSpecular;
	}

	


	//��Ϲ��պ�����
	float4 finalColor = DiffuseLight +SpecularLight;
	return finalColor;
}

float4 DiffusePass(float2 TexCoord : TEXCOORD0) : COLOR
{
	//�������
	float4 Texture = tex2D(g_sampleDiffuse, TexCoord);

	//AO
	float4 AO = tex2D(g_sampleAO, TexCoord);

	return Texture * AO;
}

float ChebyshevUpperBound(float2 Moments, float t)
{
	float g_MinVariance = 0.02f;

	float p = (t <= Moments.x);

	float Variance = Moments.y - (Moments.x*Moments.x);
	Variance = max(Variance, g_MinVariance);

	float d = t - Moments.x;
	float p_max = Variance / (Variance + d*d);

	return max(p, p_max);
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

	float2 Moments = tex2D(g_sampleShadow, ShadowTexCoord);
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
	//return float4(Shadow.x / 100, Shadow.x / 100, Shadow.x / 100, 1.0f);
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
	pass p1 //��Ⱦ��Ӱ
	{
		pixelShader = compile ps_3_0 ShadowVSMPass();
		AlphaBlendEnable = true;                        //������Ⱦ״̬        
		SrcBlend = ZERO;
		DestBlend = SrcColor;
	}
	pass p2 //��ӻ�����
	{
		pixelShader = compile ps_3_0 AmbientPass();
		AlphaBlendEnable = true;                        //������Ⱦ״̬        
		SrcBlend = ONE;
		DestBlend = ONE;
	}
	pass p3 //����AO
	{
		pixelShader = compile ps_3_0 DiffusePass();
		AlphaBlendEnable = true;                        //������Ⱦ״̬        
		SrcBlend = ZERO;
		DestBlend = SrcColor;
	}
	pass p4 //DEBUG PASS
	{
		//pixelShader = compile ps_3_0 DebugPass();
		pixelShader = compile ps_3_0 ShadowVSMPass();
		//pixelShader = compile ps_3_0 AmbientPass();
		//pixelShader = compile ps_3_0 DiffusePass();
		AlphaBlendEnable = true;                        //������Ⱦ״̬        
		SrcBlend = ONE;
		DestBlend = ZERO;
	}
	
}