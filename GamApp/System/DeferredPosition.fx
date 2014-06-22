matrix		g_World;
matrix		g_View;
matrix		g_Proj;
matrix		g_WorldView;
matrix		g_ViewProj;
matrix		g_WorldViewProj;
matrix		g_mWorldInv;

float		g_zNear = 1.0f;
float		g_zFar = 1000.0f;

float4		g_ViewPos;

struct OutputVS
{
	float4 posWVP			: POSITION;
	float4 posV				: TEXCOORD0;
	float4 posP				: TEXCOORD1;
};


OutputVS VShader(float4 posL		: POSITION0)
{
	OutputVS outVS = (OutputVS)0;

	//��������Ķ���λ�ã��������硢�۲졢ͶӰ����任��
	outVS.posWVP	= mul(posL, g_WorldViewProj);
	outVS.posV		= mul(posL, g_WorldView);
	outVS.posP		= outVS.posWVP;

	return outVS;
}


float4 PShader(float4 posV : TEXCOORD0,
				float4 posP : TEXCOORD1 ) : COLOR
{
	//�����ɫ
	float DepthP = posP.z / posP.w;
	float4 Color = float4(posV.xyz, DepthP);
	return Color;// float4(1.0f, 0.0f, 0.0f, 1.0f);
}

technique DeferredPosition
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}