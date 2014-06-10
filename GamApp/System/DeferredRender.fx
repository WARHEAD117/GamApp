matrix		g_World;
matrix		g_View;
matrix		g_Proj;
matrix		g_ViewProj;
matrix		g_WorldViewProj;
matrix		g_mWorldInv;

texture		g_DiffuseBuffer;
texture		g_NormalDepthBuffer;

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


float4 PShader(float2 TexCoord : TEXCOORD0) : COLOR
{
	//�������
	float4 Texture = tex2D(g_sampleDiffuse, TexCoord);
	float4 NormalDepth = tex2D(g_sampleNormalDepth, TexCoord);


	//DeferredRender
	//���㻷����
	float4 Ambient = float4(0.2f, 0.2f, 0.2f, 1.0f);  //���ʿ����Ϊ����ϵ��

	float3 NormalW = normalize(NormalDepth.xyz);
	float3 ToEyeDirW = normalize(float3(1, 0, 0));
	float3 LightDir = float3(-1.0f, -1.0f, -1.0f);
	//float4 LIGHTDIR[4] = { float4(-1.0f, -1.0f, -1.0f, 1.0f), float4(-1.0f, -1.0f, 1.0f, 1.0f), float4(-1.0f, 1.0f, -1.0f, 1.0f), float4(-1.0f, 1.0f, 1.0f, 1.0f) };
	//float4 lightSpecular[4] = { float4(1.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 1.0f, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f) };

	float3 LIGHTDIR1 = float3(-2.0f, -0.0f, -0.0f);
	float4 lightSpecular1 = float4(1.0f, 0.0f, 0.0f, 1.0f);

	float3 LIGHTDIR2 = float3(-2.0f, 2.0f, -2.0f);
	float4 lightSpecular2 = float4(0.0f, 1.0f, 0.0f, 1.0f);

	float3 LIGHTDIR3 = float3(-2.0f, -2.0f, 2.0f);
	float4 lightSpecular3 = float4(0.0f, 0.0f, 1.0f, 1.0f);

	float3 LIGHTDIR4 = float3(0.0f, -0.0f, -1.0f);
	float4 lightSpecular4 = float4(1.0f, 1.0f, 1.0f, 1.0f);

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
		//����������
		float DiffuseRatio = max(dot(L, NormalW), 0);
		DiffuseLight += LS * float4(1.0f, 1.0f, 1.0f, 1.0f) * DiffuseRatio;

		//���㾵�淴��

		//����������
		float3 H = normalize(L + ToEyeDirW);

		//Phong����
		//float4 Reflect = normalize(float4(g_LightDir.xyz - 2 * DiffuseRatio * NormalW, 1.0f));
		//float SpecularRatio = max(dot(Reflect, ToEyeDirW), 0);

		//Blinn-Phong����
		float SpecularRatio = max(dot(NormalW, H), 0);
		float PoweredSpecular = pow(SpecularRatio, 6);
		Specular += LS * PoweredSpecular;

	}

	//��Ϲ��պ�����
	float4 finalColor = Ambient * Texture +DiffuseLight *Texture;// DiffuseLight *Texture + Specular + ;
	//�����ɫ
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