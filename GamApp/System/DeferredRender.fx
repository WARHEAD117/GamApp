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

//----------------------------

sampler2D g_sampleShadow =
sampler_state
{
	Texture = <g_ShadowBuffer>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
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

float4 PShader(float2 TexCoord : TEXCOORD0) : COLOR
{
	//��������
	float4 Texture = tex2D(g_sampleDiffuse, TexCoord);
	float4 NormalDepth = tex2D(g_sampleNormalDepth, TexCoord);
	float3 NormalV = normalize(NormalDepth.xyz * 2.0f - 1.0f);

	//DeferredRender
	//���㻷����
	float4 Ambient = float4(0.2f, 0.2f, 0.2f, 1.0f);  //���ʿ�����Ϊ����ϵ��

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

	float3 LIGHTDIR4 = float3(1.0f, -1.0f, 0.0f);
	float4 lightSpecular4 = float4(0.3f, 0.3f, 0.3f, 1.0f);

	float4 DiffuseLight = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4	Specular = float4(0.0f, 0.0f, 0.0f, 1.0f);
	//for (int i = 0; i < 4; i++)
	int i = 3;
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
		//����������
		float DiffuseRatio = max(dot(L, NormalV), 0);
		DiffuseLight += LS * float4(1.0f, 1.0f, 1.0f, 1.0f) * DiffuseRatio;

		//���㾵�淴��

		//����������
		float3 H = normalize(L + ToEyeDirV);

		//Phong����
		//float4 Reflect = normalize(float4(g_LightDir.xyz - 2 * DiffuseRatio * NormalW, 1.0f));
		//float SpecularRatio = max(dot(Reflect, ToEyeDirW), 0);

		//Blinn-Phong����
		float SpecularRatio = max(dot(NormalV, H), 0);
		float PoweredSpecular = pow(SpecularRatio, 24);
		Specular += LS * PoweredSpecular;

	}

	float4 AO = tex2D(g_sampleAO, TexCoord);
	

	//Shadow
	float4 worldPos = mul(float4(pos, 1.0f), g_invView);
	float4 lightViewPos = mul(worldPos, g_ShadowView);
	float4 lightProjPos = mul(lightViewPos, g_ShadowProj);
	float lightU = (lightProjPos.x / lightProjPos.w + 1.0f) / 2.0f;
	float lightV = (1.0f - lightProjPos.y / lightProjPos.w) / 2.0f;
	float4 shadow = tex2D(g_sampleShadow, float2(lightU, lightV));
	
	float4 shadow1 = tex2D(g_sampleShadow, float2(lightU - 1 / 700.0f, lightV));
	float4 shadow2 = tex2D(g_sampleShadow, float2(lightU + 1 / 700.0f, lightV));
	float4 shadow3 = tex2D(g_sampleShadow, float2(lightU, lightV - 1 / 700.0f));
	float4 shadow4 = tex2D(g_sampleShadow, float2(lightU, lightV + 1 / 700.0f));
	//return float4(float2(lightU, lightV),0.0f,1.0f);

	//return float4(shadow.x, 0.0f,0.0f,1.0f);

	//float4 tempShadow = tex2D(g_sampleShadow, TexCoord);
	//return float4(tempShadow.x, 0.0f,0.0f,1.0f);

	//return lightProjPos;
	//return  float4(lightProjPos.x, lightProjPos.y, 0.0f, 1.0f);

	float shadowStrength = 1.0f;
	if (lightProjPos.z / lightProjPos.w > shadow.r + 0.003f)
		shadowStrength = 0.1f;

	//��Ϲ��պ�����
	float4 finalColor = AO * Ambient * Texture + (DiffuseLight *Texture + Specular)*shadowStrength;
	//return float4(NormalDepth.w, 0.0f, 0.0f, 1.0f);
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