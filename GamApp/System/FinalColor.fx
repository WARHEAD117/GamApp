matrix		g_World;
matrix		g_View;
matrix		g_Proj;
matrix		g_ViewProj;
matrix		g_WorldViewProj;
matrix		g_mWorldInv;
matrix		g_InverseProj;

texture	g_FinalColorBuffer;
sampler2D g_sampleFinalColor =
sampler_state
{
	Texture = <g_FinalColorBuffer>;
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
	float4 Texture = tex2D(g_sampleFinalColor, TexCoord);
	
	return Texture;
}

technique FinalColor
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}