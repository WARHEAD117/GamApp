matrix		g_World;
matrix		g_View;
matrix		g_Proj;
matrix		g_ViewProj;
matrix		g_WorldViewProj;
matrix		g_mWorldInv;

texture		g_Texture;


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
	float4 Texture = tex2D(g_sampleTexture, TexCoord);

	//�����ɫ
	return Texture;// float4(1.0f, 0.0f, 0.0f, 1.0f);
}

technique DeferredDiffuse
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}