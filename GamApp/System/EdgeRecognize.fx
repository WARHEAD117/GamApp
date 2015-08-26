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

	if (depth - depthLeft > depth / 10.0f || depth - depthRight > depth / 10.0f || depth - depthUp > depth / 10.0f || depth - depthDown > depth / 10.0f)
	{
		
		return float4(1, 1, 1, 1);
	}

	float3 normalLeft = normalize(GetNormal(TexCoord + float2(-1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight), g_sampleNormal));
		//normalLeft = GetUnsharpMaskedNormal(TexCoord + float2(-1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight), g_sampleNormal);
	float3 normalRight = normalize(GetNormal(TexCoord + float2(1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight), g_sampleNormal));
		//normalRight = GetUnsharpMaskedNormal(TexCoord + float2(1.0f / g_ScreenWidth, -0.0f / g_ScreenHeight), g_sampleNormal);
	float3 normalUp = normalize(GetNormal(TexCoord + float2(-0.0f / g_ScreenWidth, -1.0f / g_ScreenHeight), g_sampleNormal));
		//normalUp = GetUnsharpMaskedNormal(TexCoord + float2(-0.0f / g_ScreenWidth, -1.0f / g_ScreenHeight), g_sampleNormal);
	float3 normalDown = normalize(GetNormal(TexCoord + float2(-0.0f / g_ScreenWidth, 1.0f / g_ScreenHeight), g_sampleNormal));
		//normalDown = GetUnsharpMaskedNormal(TexCoord + float2(-0.0f / g_ScreenWidth, 1.0f / g_ScreenHeight), g_sampleNormal);

	if (depthRight > 1000)
	{
		normalRight = float3(0, 0, -1);
	}
	if (depthDown > 1000)
	{
		normalDown = float3(0, 0, -1);
	}
	if (depthLeft > 1000)
	{
		normalLeft = float3(0, 0, -1);
	}
	if (depthUp > 1000)
	{
		normalUp = float3(0, 0, -1);
	}

	float dnx = dot(normal, normalRight);
	float dny = dot(normal, normalDown);
	float dnx2 = dot(normalLeft, normal);
	float dny2 = dot(normalUp, normal);
	dnx = min(dnx, dnx2);
	dny = min(dny, dny2);

	float N = sqrt(dnx*dnx + dny*dny);

	N = dnx + dny;
	N /= 2;
	//N = min(dny, dnx);
	//if (N >= 1)
	//	return float4(1, 1, 1, 1);
	return float4(N, N, N, 1);
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
	return Normal_Edge(TexCoord);
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