matrix		g_World;
matrix		g_View;
matrix		g_Proj;
matrix		g_ViewProj;
matrix		g_mWorldInv;

float4		g_LightDir;
float4		g_ViewPos;

texture		g_Texture;

float4 lightDiffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
float4 lightAmbient = float4(1.0f, 1.0f, 1.0f, 1.0f);
float4 lightSpecular = float4(0.5f, 0.5f, 0.5f, 1.0f);

float4 materialAmbient = float4(1.0f, 1.0f, 1.0f, 1.0f);
float4 materialDiffuse = float4(0.5f, 0.5f, 0.5f, 1.0f);
float4 materialSpecular = float4(1.0f, 1.0f, 1.0f, 1.0f);

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
	float4 posH         : POSITION0;
	float2 TexCoord		: TEXCOORD0;
	float2 normal		: NORMAL0;
	float4 toEyeDir		: TEXCOORD1;
};


OutputVS VShader(float4 posL       : POSITION0,
	float3 normalL : NORMAL0,  // Assumed to be unit length
	float2 TexCoord : TEXCOORD0)
{
	OutputVS outVS = (OutputVS)0;

	//��������Ķ���λ�ã��������硢�۲졢ͶӰ����任��
	// Transform to homogeneous clip space.
	//matrix matWVP = mul(g_ViewProj, g_World);
	outVS.posH = mul(posL, g_ViewProj);

	// Pass on texture coordinates to be interpolated
	// in rasterization.
	outVS.TexCoord = TexCoord;
	outVS.normal = normalL;
	outVS.toEyeDir = g_LightDir - posL;
	return outVS;
}


float4 PShader(float4 posH			:	POSITION0,
	float2 TexCoord : TEXCOORD0,
	float3 Normal : NORMAL0,
	float4 ToEyeDir : TEXCOORD1) : COLOR
{

	//�������
	float4 Texture = tex2D(g_sampleTexture, TexCoord);



	//���㻷����
	float4 Ambient = lightAmbient * materialAmbient;  //���ʿ����Ϊ����ϵ��

	//����������
	float DiffuseRatio = dot(g_LightDir, Normal);
	float4 Diffuse = lightDiffuse * (materialDiffuse * DiffuseRatio);

	Normal = normalize(Normal);
	float4 LightDir = normalize(g_LightDir);
	//���㾵�淴��
	float4 Reflect = float4(normalize(LightDir.xyz - 2 * DiffuseRatio * Normal), 1.0f);
	float SpecularRatio = dot(Reflect, ToEyeDir);
	float4 Specular = lightSpecular* (materialSpecular * SpecularRatio);

	//Specular = normalize(Specular); 
	//Diffuse = normalize(Diffuse);
	//��Ϲ��պ�����
	float4 finalColor = Texture * (Ambient *0.3f + Diffuse);// + Specular;// ;
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