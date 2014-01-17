matrix		g_World;
matrix		g_View;
matrix		g_Proj;
matrix		g_ViewProj;
matrix		g_WorldViewProj;
matrix		g_mWorldInv;

float4		g_LightDir;
float4		g_ViewPos;

texture		g_Texture;

float4 lightDiffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
float4 lightAmbient = float4(1.0f, 1.0f, 1.0f, 1.0f);
float4 lightSpecular = float4(1.0f, 1.0f, 1.0f, 1.0f);

float4 g_AmbientMaterial = float4(1.0f, 1.0f, 1.0f, 1.0f);
float4 g_DiffuseMaterial = float4(1.0f, 1.0f, 1.0f, 1.0f);
float4 g_SpecularMaterial = float4(1.0f, 1.0f, 1.0f, 1.0f);
float g_SpecularPower = 1.0f;

sampler2D g_sampleTexture =
sampler_state
{
	Texture = <g_Texture>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
};

struct OutputVS
{
	float4 posWVP         : POSITION0;
	float2 TexCoord		: TEXCOORD0;
	float3 normalW		: NORMAL0;
	float4 toEyeDirW		: TEXCOORD1;
};


OutputVS VShader(float4 posL       : POSITION0,
	float3 normalL : NORMAL0,  // Assumed to be unit length
	float2 TexCoord : TEXCOORD0)
{
	OutputVS outVS = (OutputVS)0;

	//��������Ķ���λ�ã��������硢�۲졢ͶӰ����任��
	outVS.posWVP = mul(posL, g_WorldViewProj);
	outVS.normalW = mul(normalL, g_World);

	outVS.TexCoord = TexCoord;
	outVS.toEyeDirW = g_ViewPos - mul(posL, g_World);
	return outVS;
}


float4 PShader(float4 posWVP			:	POSITION0,
	float2 TexCoord : TEXCOORD0,
	float3 NormalW : NORMAL0,
	float4 ToEyeDirW : TEXCOORD1) : COLOR
{
	//�������
	float4 Texture = tex2D(g_sampleTexture, TexCoord);


	//���㻷����
	float4 Ambient = lightAmbient * g_AmbientMaterial;  //���ʿ����Ϊ����ϵ��

	NormalW = normalize(NormalW);
	ToEyeDirW = normalize(ToEyeDirW);
	//����������
	float DiffuseRatio = dot(-g_LightDir, NormalW);
	float4 Diffuse = lightDiffuse * (g_DiffuseMaterial * DiffuseRatio);

	//���㾵�淴��

	//����������
	float4 H = normalize(ToEyeDirW - g_LightDir);

	float4 V = ToEyeDirW;

	float4 Specular = float4(0.0, 0.0, 0.0, 0.0);

	bool back = (dot(V, float4(NormalW, 1.0f))>0) && (dot(g_LightDir, float4(NormalW, 1.0f)));

	float Ks = 0.8;
	float shininess = 6;
	if (back)
	{
		float4 T = float4(normalize(cross(NormalW, V)),1.0f);   // ���㶥�������� 

		float a = dot(-g_LightDir, T);

		float b = dot(V, T);

		float c = sqrt(1 - pow(a, 2.0))* sqrt(1 - pow(b, 2.0)) - a*b; // ���� Bank BRDF ϵ�� 

		float brdf = Ks* pow(c, shininess);

		Specular = brdf *Diffuse;// *lightColor;
	}

	//��Ϲ��պ�����
	float4 finalColor = Texture * Diffuse + Specular + Ambient*0.1f;
		//�����ɫ
	return finalColor;
}

technique CommonDiffuse
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}