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
	float4 Diffuse = tex2D(g_sampleDiffuse, TexCoord);
	float4 NormalDepth = tex2D(g_sampleNormalDepth, TexCoord);


	//DeferredRender
	//���㻷����
	float4 Ambient = float4(0.2f, 0.2f, 0.2f, 1.0f);  //���ʿ����Ϊ����ϵ��

	float4 NormalW = float4(normalize(NormalDepth.xyz), 1.0f);
	float4 ToEyeDirW = float4(0, 0, -1, 1.0f);
	float4 LightDir = float4(-1.0f, -1.0f, -1.0f,1.0f);
	//����������
	float DiffuseRatio = dot(-LightDir, NormalW);
	float4 DiffuseLight = float4(0.8f, 0.8f, 0.8f, 1.0f) * DiffuseRatio;

	//���㾵�淴��

	//����������
	float4 H = normalize(ToEyeDirW - LightDir);

	//Phong����
	//float4 Reflect = normalize(float4(g_LightDir.xyz - 2 * DiffuseRatio * NormalW, 1.0f));
	//float SpecularRatio = max(dot(Reflect, ToEyeDirW), 0);

	//Blinn-Phong����
	float SpecularRatio = max(dot(NormalW, H), 0);

	float4	Specular = pow(SpecularRatio, 12);
	//Specular = lightSpecular* (g_SpecularMaterial * Specular);

	//��Ϲ��պ�����
	float4 finalColor = DiffuseLight * Diffuse + Specular + Ambient*0.1f;
	//�����ɫ
	return finalColor;// float4(1.0f, 0.0f, 0.0f, 1.0f);

	float4 final = Diffuse + NormalDepth;
	//�����ɫ
	return final;// float4(1.0f, 0.0f, 0.0f, 1.0f);
}

technique DeferredRender
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}