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
	float3 normalWV			: NORMAL;
	float4 posP				: TEXCOORD0;
};


OutputVS VShader(float4 posL		: POSITION,
				 float3 normalL		: NORMAL)  // Assumed to be unit length)
{
	OutputVS outVS = (OutputVS)0;

	//��������Ķ���λ�ã��������硢�۲졢ͶӰ����任��
	outVS.posWVP = mul(posL, g_WorldViewProj);
	outVS.posP = outVS.posWVP;

	//�۲�ռ��µķ���
	outVS.normalWV = mul(normalL, g_WorldView);

	return outVS;
}


float4 PShader(float3 NormalWV			: NORMAL,
				float4 posP				: TEXCOORD0) : COLOR
{
	NormalWV = normalize(NormalWV);
	NormalWV = (NormalWV + float3(1.0f, 1.0f, 1.0f)) / 2;

	//ͶӰ��ķ��������
	float Depth = posP.z / posP.w;

	float4 finalColor = float4(NormalWV, Depth);
	//�����ɫ
	return finalColor;// float4(1.0f, 0.0f, 0.0f, 1.0f);
}

technique DeferredNormalDepth
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}