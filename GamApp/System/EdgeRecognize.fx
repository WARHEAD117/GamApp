#include "common.fx"

texture		g_NormalBuffer;
sampler2D g_sampleNormal =
sampler_state
{
	Texture = <g_NormalBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};


texture		g_PositionBuffer;
sampler2D g_samplePosition =
sampler_state
{
	Texture = <g_PositionBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

texture		g_MainColorBuffer;
sampler2D g_sampleMainColor =
sampler_state
{
	Texture = <g_MainColorBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

float g_normalFactor = 0.8f;
float g_depthFactor = 0.3f;

struct OutputVS
{
	float4 posWVP         : POSITION0;
	float2 TexCoord		: TEXCOORD0;
};


OutputVS VShader(float4 posL       : POSITION0,
				 float2 TexCoord : TEXCOORD0)
{
	OutputVS outVS = (OutputVS)0;

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	outVS.posWVP = mul(posL, g_WorldViewProj);

	outVS.TexCoord = TexCoord;

	return outVS;
}

float4 fliter(float3x3 _filter, sampler2D _image, float2 texCoord, float2 texSize)
{
	float2 filterOffset[3][3] = 
	{
		{ float2(-1.0f, -1.0f), float2( 0.0f, -1.0f), float2( 1.0f, -1.0f) },
		{ float2(-1.0f,  0.0f), float2( 0.0f,  0.0f), float2( 1.0f,  0.0f) },
		{ float2(-1.0f,  1.0f), float2( 0.0f,  1.0f), float2( 1.0f,  1.0f) }
	};

	float4 finalColor = float4(0, 0, 0, 0);

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			float2 newTexCoord = texCoord + float2(filterOffset[i][j].x / texSize.x, filterOffset[i][j].y / texSize.y);
				finalColor += tex2D(_image, newTexCoord) * _filter[i][j];
		}
	}

	return finalColor;
}

float4 SobelEdge(sampler2D _image, float2 texCoord, float2 texSize)
{
	float3x3 sobelX = float3x3(-1.0, -2.0, -1.0,
								0.0, 0.0, 0.0,
								1.0, 2.0, 1.0);
	float3x3 sobelY = float3x3(-1.0, 0.0, 1.0,
								-2.0, 0.0, 2.0,
								-1.0, 0.0, 1.0);
	float4 oColorX = fliter(sobelX, _image, texCoord, texSize);
	float4 oColorY = fliter(sobelY, _image, texCoord, texSize);
	
	float GX = 0.3 * oColorX.r + 0.59 * oColorX.g + 0.11 * oColorX.b;
	float GY = 0.3 * oColorY.r + 0.59 * oColorY.g + 0.11 * oColorY.b;

	GX = abs(GX);
	GY = abs(GY);

	if (GX + GY <= 0.86)
		return float4(1, 1, 1, 1.0f);
	float X = 1 - (GX + GY);
	return float4(X, X, X, 1);
	if (GX+GY > 0.86) //0.86
		return float4(0, 0, 0, 1.0f);
	else
		return float4(1, 1, 1, 1.0f);
}

float4 DiffEdge(sampler2D _image, float2 texCoord, float2 texSize)
{
	float3x3 sobelX = float3x3(0.0, 0.0, 0.0,
								0.0, -1.0, 1.0,
								0.0, 0.0, 0.0);
	float3x3 sobelY = float3x3(0.0, 0.0, 0.0,
								0.0, 1.0, 0.0,
								0.0, -1.0, 0.0);
	float4 oColorX = fliter(sobelX, _image, texCoord, texSize);
		float4 oColorY = fliter(sobelY, _image, texCoord, texSize);

		float GX = 0.3 * oColorX.r + 0.59 * oColorX.g + 0.11 * oColorX.b;
	float GY = 0.3 * oColorY.r + 0.59 * oColorY.g + 0.11 * oColorY.b;

	GX = abs(GX);
	GY = abs(GY);

	float X = 1 -( GX + GY);
	return float4(X, X, X, 1);
	if (GX + GY > 0.18)//0.18
		return float4(0, 0, 0, 1.0f);
	else
		return float4(1, 1, 1, 1.0f);
}

float3 GetUnsharpMaskedNormal(float2 texCoords, sampler2D texSampler)
{
	float3 oriNormal = GetNormal(texCoords, texSampler);

	float weights[6] = { 0.00078633, 0.00655965, 0.01330373, 0.05472157, 0.11098164, 0.22508352 };

	float3 normal;
	float stepU = 1.0f / g_ScreenWidth;
	float stepV = 1.0f / g_ScreenHeight;

	//0,1,2,1,0
	//1,3,4,3,1
	//2,4,5,4,2
	//1,3,4,3,1
	//0,1,2,1,0
	normal = GetNormal(texSampler, texCoords + float2(-2 * stepU, -2 * stepV)) * (weights[0]);
	normal += GetNormal(texSampler, texCoords + float2(-1 * stepU, -2 * stepV)) * (weights[1]);
	normal += GetNormal(texSampler, texCoords + float2(0 * stepU, -2 * stepV)) * (weights[2]);
	normal += GetNormal(texSampler, texCoords + float2(1 * stepU, -2 * stepV)) * (weights[1]);
	normal += GetNormal(texSampler, texCoords + float2(2 * stepU, -2 * stepV)) * (weights[0]);

	normal += GetNormal(texSampler, texCoords + float2(-2 * stepU, -1 * stepV)) * (weights[1]);
	normal += GetNormal(texSampler, texCoords + float2(-1 * stepU, -1 * stepV)) * (weights[3]);
	normal += GetNormal(texSampler, texCoords + float2(0 * stepU, -1 * stepV)) * (weights[4]);
	normal += GetNormal(texSampler, texCoords + float2(1 * stepU, -1 * stepV)) * (weights[3]);
	normal += GetNormal(texSampler, texCoords + float2(2 * stepU, -1 * stepV)) * (weights[1]);

	normal += GetNormal(texSampler, texCoords + float2(-2 * stepU, 0 * stepV)) * (weights[2]);
	normal += GetNormal(texSampler, texCoords + float2(-1 * stepU, 0 * stepV)) * (weights[4]);
	normal += GetNormal(texSampler, texCoords + float2(0 * stepU, 0 * stepV)) * (weights[5]);
	normal += GetNormal(texSampler, texCoords + float2(1 * stepU, 0 * stepV)) * (weights[4]);
	normal += GetNormal(texSampler, texCoords + float2(2 * stepU, 0 * stepV)) * (weights[2]);

	normal += GetNormal(texSampler, texCoords + float2(-2 * stepU, 1 * stepV)) * (weights[1]);
	normal += GetNormal(texSampler, texCoords + float2(-1 * stepU, 1 * stepV)) * (weights[3]);
	normal += GetNormal(texSampler, texCoords + float2(0 * stepU, 1 * stepV)) * (weights[4]);
	normal += GetNormal(texSampler, texCoords + float2(1 * stepU, 1 * stepV)) * (weights[3]);
	normal += GetNormal(texSampler, texCoords + float2(2 * stepU, 1 * stepV)) * (weights[1]);

	normal += GetNormal(texSampler, texCoords + float2(-2 * stepU, 2 * stepV)) * (weights[0]);
	normal += GetNormal(texSampler, texCoords + float2(-1 * stepU, 2 * stepV)) * (weights[1]);
	normal += GetNormal(texSampler, texCoords + float2(0 * stepU, 2 * stepV)) * (weights[2]);
	normal += GetNormal(texSampler, texCoords + float2(1 * stepU, 2 * stepV)) * (weights[1]);
	normal += GetNormal(texSampler, texCoords + float2(2 * stepU, 2 * stepV)) * (weights[0]);
	float3 blurredNormal = normalize(normal);

	float3 unsharpedMaskedNormal = oriNormal + 5*(oriNormal - blurredNormal);
	float3 importance = oriNormal - blurredNormal;

	if (importance.z < 0)
		return oriNormal;
	return normalize(unsharpedMaskedNormal);
}


float4 Normal_Edge3(float2 TexCoord)
{

	float depth = GetDepth(TexCoord, g_samplePosition);

	float depthLeft = GetDepth(TexCoord + float2(-1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight), g_samplePosition);
	float depthRight = GetDepth(TexCoord + float2(1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight), g_samplePosition);
	float depthUp = GetDepth(TexCoord + float2(-0.0f / g_ScreenWidth, -1.0f / g_ScreenHeight), g_samplePosition);
	float depthDown = GetDepth(TexCoord + float2(-0.0f / g_ScreenWidth, 1.0f / g_ScreenHeight), g_samplePosition);

	float3 normal = normalize(GetNormal(TexCoord, g_sampleNormal));
		//normal = GetUnsharpMaskedNormal(TexCoord, g_sampleNormal);

		//return float4(normal, 1.0f);

	if (depth > 1000)
	{
		normal = float3(0, 0, 1);
		return float4(1, 1, 1, 1);
	}



	bool isEdge = false;

	bool isUpEdge = false;
	bool isDownEdge = false;
	bool isLeftEdge = false;
	bool isRightEdge = false;

	float deltaD_Inner = depth / 10.0f;
	float deltaD_Outer = depth / 20.0f;


	if (depth - depthLeft > deltaD_Outer || depth - depthRight > deltaD_Outer || depth - depthUp > deltaD_Outer || depth - depthDown > deltaD_Outer)
	{
		return float4(1, 1, 1, 1);
	}

	if (depthUp - depth  > deltaD_Inner)
	{
		isEdge = true;
		isUpEdge = true;
	}
	if (depthDown - depth  > deltaD_Inner)
	{
		isEdge = true;
		isDownEdge = true;
	}
	if (depthLeft - depth > deltaD_Inner)
	{
		isEdge = true;
		isLeftEdge = true;
	}
	if (depthRight - depth  > deltaD_Inner)
	{
		isEdge = true;
		isRightEdge = true;
	}

	float3 normalLeft = normalize(GetNormal(TexCoord + float2(-1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight), g_sampleNormal));
		//normalLeft = GetUnsharpMaskedNormal(TexCoord + float2(-1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight), g_sampleNormal);
		float3 normalRight = normalize(GetNormal(TexCoord + float2(1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight), g_sampleNormal));
		//normalRight = GetUnsharpMaskedNormal(TexCoord + float2(1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight), g_sampleNormal);
		float3 normalUp = normalize(GetNormal(TexCoord + float2(-0.0f / g_ScreenWidth, -1.0f / g_ScreenHeight), g_sampleNormal));
		//normalUp = GetUnsharpMaskedNormal(TexCoord + float2(-0.0f / g_ScreenWidth, -1.0f / g_ScreenHeight), g_sampleNormal);
		float3 normalDown = normalize(GetNormal(TexCoord + float2(-0.0f / g_ScreenWidth, 1.0f / g_ScreenHeight), g_sampleNormal));
		//normalDown = GetUnsharpMaskedNormal(TexCoord + float2(-0.0f / g_ScreenWidth, 1.0f / g_ScreenHeight), g_sampleNormal);

		float3 normalProjX = normalize(float3(normal.x, 0, normal.z));
		float3 normalProjY = normalize(float3(0, normal.y, normal.z));
		float3 normalLProj = normalize(float3(normalLeft.x, 0, normalLeft.z));
		float3 normalRProj = normalize(float3(normalRight.x, 0, normalRight.z));
		float3 normalUProj = normalize(float3(0, normalUp.y, normalUp.z));
		float3 normalDProj = normalize(float3(0, normalDown.y, normalDown.z));

	float dnR = abs(dot(normal, normalRight));
	dnR = acos(dot(normalProjX, normalRProj)) / 3.14;
	if (isRightEdge) dnR = 10;
	float dnD = abs(dot(normal, normalDown));
	dnD = acos(dot(normalProjY, normalDProj)) / 3.14;
	if (isDownEdge) dnD = 10;
	float dnL = abs(dot(normalLeft, normal));
	dnL = acos(dot(normalLeft, normalProjX)) / 3.14;
	if (isLeftEdge) dnL = 10;
	float dnU = abs(dot(normalUp, normal));
	dnU = acos(dot(normalUProj, normalProjY)) / 3.14;
	if (isUpEdge) dnU = 10;

	if (isRightEdge) dnR = dnL;
	if (isDownEdge) dnD = dnU;
	if (isLeftEdge) dnL = dnR;
	if (isUpEdge) dnU = dnD;

	float res_dnx = 0.5 * (dnR + dnL);
	float res_dny = 0.5 * (dnD + dnU);

	float N = res_dnx + res_dny;
	N /= 2;


	if (isEdge)
	{
		if (isRightEdge && !isDownEdge && !isLeftEdge && !isUpEdge) N = (dnD + dnL + dnU) / 3;
		if (!isRightEdge && isDownEdge && !isLeftEdge && !isUpEdge) N = (dnR + dnL + dnU) / 3;
		if (!isRightEdge && !isDownEdge && isLeftEdge && !isUpEdge) N = (dnR + dnD + dnU) / 3;
		if (!isRightEdge && !isDownEdge && !isLeftEdge && isUpEdge) N = (dnR + dnD + dnL) / 3;

		if (!isRightEdge && !isDownEdge && isLeftEdge && isUpEdge) N = (dnR + dnD) * 0.5;
		if (!isRightEdge && isDownEdge && !isLeftEdge && isUpEdge) N = (dnR + dnL) * 0.5;
		if (!isRightEdge && isDownEdge && isLeftEdge && !isUpEdge) N = (dnR + dnU) * 0.5;
		if (isRightEdge && !isDownEdge && !isLeftEdge && isUpEdge) N = (dnD + dnL) * 0.5;
		if (isRightEdge && !isDownEdge && isLeftEdge && !isUpEdge) N = (dnD + dnU) * 0.5;
		if (isRightEdge && isDownEdge && !isLeftEdge && !isUpEdge) N = (dnL + dnU) * 0.5;

		if (!isRightEdge && isDownEdge && isLeftEdge && isUpEdge) N = dnR;
		if (isRightEdge && !isDownEdge && isLeftEdge && isUpEdge) N = dnD;
		if (isRightEdge && isDownEdge && !isLeftEdge && isUpEdge) N = dnL;
		if (isRightEdge && isDownEdge && isLeftEdge && !isUpEdge) N = dnU;

		if (isRightEdge && isDownEdge&& isLeftEdge&& isUpEdge)
			N = 1;
		else
			N = N;
	}
	else
	{

		N = ((dnR + dnL) + (dnU + dnD)) / 4;
		N = N;
	}



	if (false)
	{
		if (res_dnx >= 1 && res_dny < 1) N = res_dny;
		if (res_dnx < 1 && res_dny >= 1) N = res_dnx;
		if (res_dnx >= 1 && res_dny >= 1) N = 1;
		if (res_dnx < 1 && res_dny < 1) N = N;
	}

	float alpha = 1;
	if (isEdge)
		alpha = 0;
	//N = min(dny, dnx);
	//if (N >= 1)
	//	return float4(1, 1, 1, 1);
	N = 1-50*N;
	return float4(N, N, N, alpha);
}


int g_switch = 1;

float4 Normal_Edge2(float2 TexCoord)
{
	int neighborNum = 4;
	//neighborNum = 8;

	float2 offset = float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight);

	float depthN[9];

	depthN[0] = GetDepth(TexCoord, g_samplePosition);

	depthN[1] = GetDepth(TexCoord + offset * float2( 0,-1), g_samplePosition); //Up
	depthN[2] = GetDepth(TexCoord + offset * float2( 1, 0), g_samplePosition); //Right
	depthN[3] = GetDepth(TexCoord + offset * float2( 0, 1), g_samplePosition); //Down
	depthN[4] = GetDepth(TexCoord + offset * float2(-1, 0), g_samplePosition); //Left

	depthN[5] = GetDepth(TexCoord + offset * float2( 1,-1), g_samplePosition); //RightUp
	depthN[6] = GetDepth(TexCoord + offset * float2( 1, 1), g_samplePosition); //RightDown
	depthN[7] = GetDepth(TexCoord + offset * float2(-1, 1), g_samplePosition); //LeftDown
	depthN[8] = GetDepth(TexCoord + offset * float2(-1,-1), g_samplePosition); //LeftUp

	float3 normalR[9];

	normalR[0] = normalize(GetNormal(g_sampleNormal, TexCoord));

	normalR[1] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2( 0,-1))); //Up
	normalR[2] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2( 1, 0))); //Right
	normalR[3] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2( 0, 1))); //Down
	normalR[4] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(-1, 0))); //Left

	normalR[5] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2( 1,-1))); //RightUp
	normalR[6] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2( 1, 1))); //RIghtDown
	normalR[7] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(-1, 1))); //LeftDown
	normalR[8] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(-1,-1))); //LeftUp

	float depth = depthN[0];

	float deltaD_Inner = (depth / 20.0f)*(depth / 20.0f);
	float deltaD_Outer = depth / 20.0f;

	for (int i = 1; i<neighborNum + 1; i++)
	{
		if (depth - depthN[i] > deltaD_Outer)
		{
			return float4(1, 1, 1, 1);
		}
	}
	
	if (depth > 1000)
	{
		return float4(1, 1, 1, 1);
	}

	bool isEdge = false;
	bool edgeList[9] = { false, false, false, false, false, false, false, false, false };
	for (int i = 1; i< neighborNum + 1; i++)
	{
		if (depthN[i] - depth  > deltaD_Inner)
		{
			isEdge = true;
			edgeList[i] = true;
		}
	}

	float laplace = 4 * depth - (depthN[1] + depthN[2] + depthN[3] + depthN[4]);
	//if (laplace)
	//laplace = -laplace;
	//if (laplace < 0.0f)
	//	laplace = 0;
	//laplace = 1 - laplace;

	//return float4(laplace, laplace, laplace, 1);

	if (g_switch == 1)
	{
		if (laplace < -0.4)
			isEdge = true;
		else
			isEdge = false;

	}

	//if (isEdge)
	//	return float4(0, 0, 0, 1);
	//else
	//	return float4(1, 1, 1, 1);

	//return float4(maxD, maxD, maxD, 1);


	float3 vecXY = normalize(float3(1, 1, 0));
	float3 vecYX = normalize(float3(1, -1, 0));

	float3 normalProjX = normalize(float3(normalR[0].x, 0, normalR[0].z));
	float3 normalProjY = normalize(float3(0, normalR[0].y, normalR[0].z));

	float3 normalProjXY = dot(normalR[0], vecXY) * vecXY + float3(0, 0, normalR[0].z);
	float3 normalProjYX = dot(normalR[0], vecYX) * vecYX + float3(0, 0, normalR[0].z);

	float3 normalUProj = normalize(float3(0, normalR[1].y, normalR[1].z));
	float3 normalRProj = normalize(float3(normalR[2].x, 0, normalR[2].z));
	float3 normalDProj = normalize(float3(0, normalR[3].y, normalR[3].z));
	float3 normalLProj = normalize(float3(normalR[4].x, 0, normalR[4].z));

	float3 normalRUProjXY = dot(normalR[5], vecXY) * vecXY + float3(0, 0, normalR[5].z);
	float3 normalRDProjYX = dot(normalR[6], vecYX) * vecYX + float3(0, 0, normalR[6].z);
	float3 normalLDProjXY = dot(normalR[7], vecXY) * vecXY + float3(0, 0, normalR[7].z);
	float3 normalLUProjYX = dot(normalR[8], vecYX) * vecYX + float3(0, 0, normalR[8].z);

	float du = dot(normalUProj, normalProjY);
	float dr = dot(normalProjX, normalRProj);
	float dd = dot(normalProjY, normalDProj);
	float dl = dot(normalLProj, normalProjX);

	float dru = dot(normalRUProjXY, normalProjXY);
	float drd = dot(normalProjYX, normalRDProjYX);
	float dld = dot(normalProjXY, normalLDProjXY);
	float dlu = dot(normalLUProjYX, normalProjYX);

	dl = acos(dl);
	dr = acos(dr);
	du = acos(du);
	dd = acos(dd);
	dru = acos(dru);
	drd = acos(drd);
	dld = acos(dld);
	dlu = acos(dlu);
	float dmax = (max(max(max(dl, dr), du), dd));
	float dmin = (min(min(min(dl, dr), du), dd));

	float d[9] = {0, du, dr, dd, dl, dru, drd, dld, dlu };
	dmax = -1000;
	dmin = 1000;
	for (int i = 1; i < neighborNum + 1; i++)
	{
		if (edgeList[i] != true)
		{
			dmax = max(d[i], dmax);
			dmin = min(d[i], dmin);
		}
	}
	
	dmax = (dmax + dmin) / 2;
	//dmax = dmax * dmin;
	float N = 1 - dmax / 3.14 * 0.8;

	

	N = N*N*N ;
	if (isEdge)
	{
		N *= 0.5;
		//if (N > 0.6) N = 0.6;
	}
	float alpha = 1;
	if (isEdge)
		alpha = 0;
	return float4(N, N, N, alpha);
}

float4 Normal_Edge(float2 TexCoord)
{

	float depth = GetDepth(TexCoord, g_samplePosition);

	float depthLeft = GetDepth(TexCoord + float2(-1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight), g_samplePosition);
	float depthRight = GetDepth(TexCoord + float2(1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight), g_samplePosition);
	float depthUp = GetDepth(TexCoord + float2(-0.0f / g_ScreenWidth, -1.0f / g_ScreenHeight), g_samplePosition);
	float depthDown = GetDepth(TexCoord + float2(-0.0f / g_ScreenWidth, 1.0f / g_ScreenHeight), g_samplePosition);

	float3 normal = normalize(GetNormal(TexCoord, g_sampleNormal));
		//normal = GetUnsharpMaskedNormal(TexCoord, g_sampleNormal);

	//return float4(normal, 1.0f);

	if (depth > 1000)
	{
		normal = float3(0, 0, 1);
		return float4(1, 1, 1, 1);
	}

	

	bool isEdge = false;

	bool isUpEdge = false;
	bool isDownEdge = false;
	bool isLeftEdge = false;
	bool isRightEdge = false;

	float deltaD_Inner = depth / 10.0f;
	float deltaD_Outer = depth / 20.0f;


	if (depth - depthLeft > deltaD_Outer || depth - depthRight > deltaD_Outer || depth - depthUp > deltaD_Outer || depth - depthDown > deltaD_Outer)
	{
		return float4(1, 1, 1, 1);
	}

	if (depthUp - depth  > deltaD_Inner)
	{
		isEdge = true;
		isUpEdge = true;
	}
	if (depthDown - depth  > deltaD_Inner)
	{
		isEdge = true;
		isDownEdge = true;
	}
	if (depthLeft - depth > deltaD_Inner)
	{
		isEdge = true;
		isLeftEdge = true;
	}
	if (depthRight - depth  > deltaD_Inner)
	{
		isEdge = true;
		isRightEdge = true;
	}

	float3 normalLeft = normalize(GetNormal(TexCoord + float2(-1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight), g_sampleNormal));
		//normalLeft = GetUnsharpMaskedNormal(TexCoord + float2(-1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight), g_sampleNormal);
	float3 normalRight = normalize(GetNormal(TexCoord + float2(1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight), g_sampleNormal));
		//normalRight = GetUnsharpMaskedNormal(TexCoord + float2(1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight), g_sampleNormal);
	float3 normalUp = normalize(GetNormal(TexCoord + float2(-0.0f / g_ScreenWidth, -1.0f / g_ScreenHeight), g_sampleNormal));
		//normalUp = GetUnsharpMaskedNormal(TexCoord + float2(-0.0f / g_ScreenWidth, -1.0f / g_ScreenHeight), g_sampleNormal);
	float3 normalDown = normalize(GetNormal(TexCoord + float2(-0.0f / g_ScreenWidth, 1.0f / g_ScreenHeight), g_sampleNormal));
		//normalDown = GetUnsharpMaskedNormal(TexCoord + float2(-0.0f / g_ScreenWidth, 1.0f / g_ScreenHeight), g_sampleNormal);

	float dnR = abs(dot(normal, normalRight));
	if (isRightEdge) dnR = 10;
	float dnD = abs(dot(normal, normalDown));
	if (isDownEdge) dnD = 10;
	float dnL = abs(dot(normalLeft, normal));
	if (isLeftEdge) dnL = 10;
	float dnU = abs(dot(normalUp, normal));
	if (isUpEdge) dnU = 10;

	if (isRightEdge) dnR = dnL;
	if (isDownEdge) dnD = dnU;
	if (isLeftEdge) dnL = dnR;
	if (isUpEdge) dnU = dnD;

	float res_dnx = 0.5 * (dnR + dnL);
	float res_dny = 0.5 * (dnD + dnU);

	float N = res_dnx + res_dny;
	N /= 2;

	
	if (isEdge)
	{
		if (isRightEdge && !isDownEdge && !isLeftEdge && !isUpEdge) N = (dnD + dnL + dnU) / 3;
		if (!isRightEdge && isDownEdge && !isLeftEdge && !isUpEdge) N = (dnR + dnL + dnU) / 3;
		if (!isRightEdge && !isDownEdge && isLeftEdge && !isUpEdge) N = (dnR + dnD + dnU) / 3;
		if (!isRightEdge && !isDownEdge && !isLeftEdge && isUpEdge) N = (dnR + dnD + dnL) / 3;

		if (!isRightEdge && !isDownEdge && isLeftEdge && isUpEdge) N = (dnR + dnD) * 0.5;
		if (!isRightEdge && isDownEdge && !isLeftEdge && isUpEdge) N = (dnR + dnL) * 0.5;
		if (!isRightEdge && isDownEdge && isLeftEdge && !isUpEdge) N = (dnR + dnU) * 0.5;
		if (isRightEdge && !isDownEdge && !isLeftEdge && isUpEdge) N = (dnD + dnL) * 0.5;
		if (isRightEdge && !isDownEdge && isLeftEdge && !isUpEdge) N = (dnD + dnU) * 0.5;
		if (isRightEdge && isDownEdge && !isLeftEdge && !isUpEdge) N = (dnL + dnU) * 0.5;

		if (!isRightEdge && isDownEdge && isLeftEdge && isUpEdge) N = dnR;
		if (isRightEdge && !isDownEdge && isLeftEdge && isUpEdge) N = dnD;
		if (isRightEdge && isDownEdge && !isLeftEdge && isUpEdge) N = dnL;
		if (isRightEdge && isDownEdge && isLeftEdge && !isUpEdge) N = dnU;

		if (isRightEdge && isDownEdge&& isLeftEdge&& isUpEdge)
			N = 1;
		else 
			N = N*N*N*0.4;
	}
	else
	{
		
		N = ((dnR+dnL) + (dnU+dnD))/4;
		N = N*N*N;
	}

	

	if (false)
	{
		if (res_dnx >= 1 && res_dny < 1) N = res_dny;
		if (res_dnx < 1 && res_dny >= 1) N = res_dnx;
		if (res_dnx >= 1 && res_dny >= 1) N = 1;
		if (res_dnx < 1 && res_dny < 1) N = N*N*N*0.4;
	}
	
	float alpha = 1;
	if (isEdge)
		alpha = 0;
	//N = min(dny, dnx);
	//if (N >= 1)
	//	return float4(1, 1, 1, 1);
	return float4(N, N, N, alpha);
}

float4 Normal_Depth_Edge(float2 TexCoord)
{
	
		float depth = GetDepth(TexCoord, g_samplePosition);

	float depthLeft = GetDepth(TexCoord + float2(-1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight), g_samplePosition);
	float depthRight = GetDepth(TexCoord + float2(1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight), g_samplePosition);
	float depthUp = GetDepth(TexCoord + float2(-0.0f / g_ScreenWidth, -1.0f / g_ScreenHeight), g_samplePosition);
	float depthDown = GetDepth(TexCoord + float2(-0.0f / g_ScreenWidth, 1.0f / g_ScreenHeight), g_samplePosition);

	float3 normal = normalize(GetNormal(TexCoord, g_sampleNormal));
	if (depth > 1000)
	{
		normal = float3(0,0,-1);
	}
	//float3 normalLeft = normalize(GetNormal(TexCoord + float2(-1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight), g_sampleNormal));
	float3 normalRight = normalize(GetNormal(TexCoord + float2(1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight), g_sampleNormal));
	//float3 normalUp = normalize(GetNormal(TexCoord + float2(-0.0f / g_ScreenWidth, -1.0f / g_ScreenHeight), g_sampleNormal));
	float3 normalDown = normalize(GetNormal(TexCoord + float2(-0.0f / g_ScreenWidth, 1.0f / g_ScreenHeight), g_sampleNormal));

	if (depthRight > 1000)
	{
		normalRight = float3(0, 0, -1);
	}
	if (depthDown > 1000)
	{
		normalDown = float3(0, 0, -1);
	}

	float dnx = dot(normal, normalRight);
	float dny = dot(normal, normalDown);

	

	float dDepthx_L = (depth - depthLeft);// / abs(pos.x - posLeft.x);
	float dDepthx_R = (depthRight - depth);// / abs(posRight.x - pos.x);
	float dDepthy_U = (depth - depthUp);// / abs(posUp.y - pos.y);
	float dDepthy_D = (depthDown - depth);// / abs(pos.y - posDown.y);

	float d_X = abs(dDepthx_R - dDepthx_L);
	float d_Y = abs(dDepthy_U - dDepthy_D);

	if (abs(dDepthx_R) > g_depthFactor || abs(dDepthy_D) > g_depthFactor || dnx < g_normalFactor || dny < g_normalFactor)
		//if (d_X > g_depthFactor || d_Y > g_depthFactor || dnx < g_normalFactor || dny < g_normalFactor)
		return float4(0, 0, 0, 0);
	else
		return float4(1, 1, 1, 1);
}

float4 Normal_Interval(float2 TexCoord)
{
	float3 normal = normalize(GetNormal(TexCoord, g_sampleNormal));
	normal = GetUnsharpMaskedNormal(TexCoord, g_sampleNormal);

	float3 view = float3(0, 0, 1);

	float nv = dot(normal, -view);
	if (nv > 0 && nv < 0.45f)
		return float4(0, 0, 0, 0);
	else
		return float4(1, 1, 1, 1);
}

float4 Normal_Gray(float2 TexCoord)
{
	float depth = GetDepth(TexCoord, g_samplePosition);

	if (depth > 1000)
	{
		//normal = float3(1, 1, 1);
		return float4(1, 1, 1, 1);
	}

	float3 normal = GetNormal(TexCoord, g_sampleNormal);

	normal = normalize(normal);
		//normal = GetUnsharpMaskedNormal(TexCoord, g_sampleNormal);
	
	
	float3 light = float3(0, -0.3, 1);

	float nl = dot(normal, -light);
	
	nl *= nl;
	nl *= 0.3;

	return float4(nl, nl, nl, 1.0f);
}


float4 PShader(float2 TexCoord : TEXCOORD0) : COLOR
{

	//return float4(GetUnsharpMaskedNormal(TexCoord, g_sampleNormal), 1.0f);
	
	float4 color = tex2D(g_sampleMainColor, TexCoord);

	//return DiffEdge(g_sampleMainColor, TexCoord, float2(g_ScreenWidth, g_ScreenHeight));
	//return SobelEdge(g_sampleMainColor, TexCoord, float2(g_ScreenWidth, g_ScreenHeight));
	//return Normal_Gray(TexCoord);
	//return Normal_Interval(TexCoord);
	if (g_switch == 1)
		return Normal_Edge2(TexCoord);
	else
		return Normal_Edge2(TexCoord);
	//return Normal_Depth_Edge(TexCoord);
	//============================================================================
	float3x3 sobel = float3x3(-0.5, -1.0, 0.0,
							  -1.0,  0.0, 1.0,
							  -0.0,  1.0, 0.5);
	float4 oColor = fliter(sobel, g_sampleMainColor, TexCoord, float2(g_ScreenWidth, g_ScreenHeight));
	float G = 0.3 * oColor.r + 0.59 * oColor.g + 0.11 * oColor.b;
	G = abs(G);
	if (G > g_depthFactor)
		return float4(0, 0, 0, 1.0f);
	else
		return float4(1,1,1,1.0f);
	//=============================================================================

	

	return color;
}

technique ColorChange
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShader();
	}
}