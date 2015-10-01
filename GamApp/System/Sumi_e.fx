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

texture		g_DiffuseBuffer;
sampler2D g_sampleDiffuse =
sampler_state
{
	Texture = <g_DiffuseBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

texture		g_GrayscaleBuffer;
sampler2D g_sampleGrayscale =
sampler_state
{
	Texture = <g_GrayscaleBuffer>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

texture		g_DiffusionSource;
sampler2D g_sampleDiffusionSource =
sampler_state
{
	Texture = <g_DiffusionSource>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;

};

texture		SA;
sampler2D g_sampleSA =
sampler_state
{
	Texture = <SA>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

texture		g_InkTex;
sampler2D g_sampleInkTex =
sampler_state
{
	Texture = <g_InkTex>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
	AddressU = Border;
	AddressV = Border;
	bordercolor = float4(1,1,1,1);
};

texture		g_InkTex1;
sampler2D g_sampleInkTex1 =
sampler_state
{
	Texture = <g_InkTex1>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
	AddressU = Border;
	AddressV = Border;
	bordercolor = float4(1, 1, 1, 1);
};

texture		g_InkTex2;
sampler2D g_sampleInkTex2 =
sampler_state
{
	Texture = <g_InkTex2>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
	AddressU = Border;
	AddressV = Border;
	bordercolor = float4(1, 1, 1, 1);
};

texture		g_InkTex3;
sampler2D g_sampleInkTex3 =
sampler_state
{
	Texture = <g_InkTex3>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
	AddressU = Border;
	AddressV = Border;
	bordercolor = float4(1, 1, 1, 1);
};

texture		g_InkTex4;
sampler2D g_sampleInkTex4 =
sampler_state
{
	Texture = <g_InkTex4>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
	AddressU = Border;
	AddressV = Border;
	bordercolor = float4(1, 1, 1, 1);
};

texture		g_InkCloud;
sampler2D g_sampleInkCloud =
sampler_state
{
	Texture = <g_InkCloud>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

texture		g_JudgeTex;
sampler2D g_sampleJudgeTex =
sampler_state
{
	Texture = <g_JudgeTex>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
};

int minI = 0;
int maxI = 70;

int g_baseTexSize = 32;
int g_maxTexSize = 20;
int g_minTexSize = 2;

float g_colorFactor = 0.3;

int g_baseInsideTexSize = 0;//46
int g_maxInsideTexSize = 0;//33
int g_minInsideTexSize = 0;//2

float g_SizeFactor = 0;//1
float g_alphaTestFactor = 0;//0.79

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

	tex2D(g_sampleNormal, float2(0, 0));
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

float4 PShaderGrayscale(float2 TexCoord : TEXCOORD0) : COLOR
{ 
	//return float4(1, 1, 1, 1.0f);
	
	float4 color = tex2D(g_sampleMainColor, TexCoord);

	float depth = GetDepth(TexCoord, g_samplePosition);
	
	if (depth > 1000)
	{
		//normal = float3(1, 1, 1);
		return float4(1, 1, 1, 1);
	}

	float3 normal = GetNormal(TexCoord, g_sampleNormal);

	normal = normalize(normal);

	float3 light = float3(0, 0, 1);
	light = normalize(light);

	float3 pos = GetPosition(TexCoord, g_samplePosition);
	pos = normalize(pos);
	light = pos;

	float nl = dot(normal, -light);

	nl = pow(nl, 1.5);
	//nl *= 0.9;

	return float4(nl, nl, nl, 1.0f);
}

float4 PShaderBlur(float2 TexCoord : TEXCOORD0) : COLOR
{
	return tex2D(g_sampleGrayscale, TexCoord);

	float4 color = GaussianBlur(g_ScreenWidth, g_ScreenHeight, g_sampleGrayscale, TexCoord);

	return color;
}

float4 PSAreaSplit(float2 TexCoord : TEXCOORD0) : COLOR
{ 
	float color = tex2D(g_sampleGrayscale, TexCoord).r;
	//if (color < 0.9f)
	//	return float4(color, color, color, 1.0f);
	//else
	//	return float4(1, 1, 1, 1.0f);
	
	int color_int = color * 255;
	if (color_int < maxI && color_int >= minI)
	{
		if (color_int > maxI - 25)
		{
			//return float4(0.5, 0.5, 0.5, 1);
			float fadeColor = lerp(maxI - 25, 255, (color_int - maxI + 25) / 25.0f);
			return float4(fadeColor / 255.0f, fadeColor / 255.0f, fadeColor / 255.0f, 1.0f);
		}
		return float4(color, color, color, 1.0f); //float4(minI / 255.0f, minI / 255.0f, minI / 255.0f, 1.0f); //
	}
	else if (color_int < minI)
		return float4(minI / 255.0f, minI / 255.0f, minI / 255.0f, 1.0f);
	else
		return float4(1, 1, 1, 1);
}

float4 PShaderEdgeBlur(float2 TexCoord : TEXCOORD0) : COLOR
{
	//return tex2D(g_sampleMainColor, TexCoord);
	//return float4(1, 0, 0, 1);
	
	float4 color4 = tex2D(g_sampleMainColor, TexCoord);
	float color = color4.r;
	if (color >= 0.99)
		return float4(color, color, color, 1);

	float2 offset = float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight);


	float colorL = tex2D(g_sampleMainColor, TexCoord + float2(-1, 0) * offset).r;
	float colorR = tex2D(g_sampleMainColor, TexCoord + float2(1, 0) * offset).r;
	float colorU = tex2D(g_sampleMainColor, TexCoord + float2(0, -1) * offset).r;
	float colorD = tex2D(g_sampleMainColor, TexCoord + float2(0, 1) * offset).r;
	float colorLU = tex2D(g_sampleMainColor, TexCoord + float2(-1, -1) * offset).r;
	float colorRU = tex2D(g_sampleMainColor, TexCoord + float2(1, -1) * offset).r;
	float colorLD = tex2D(g_sampleMainColor, TexCoord + float2(-1, 1) * offset).r;
	float colorRD = tex2D(g_sampleMainColor, TexCoord + float2(1, 1) * offset).r;

	float colorList[9] = { color, colorL, colorR, colorU, colorD, colorLU, colorRU, colorLD, colorRD };

	if (color >= 0.89)
		return color4;

	int count = 0;
	float sum = 0;
	for (int i = 0; i < 9; i++)
	{
		if (colorList[i] < 0.89)
		{
			count++;
			sum += colorList[i];
		}
	}
	if (count <= 1)
		return color4;
	float finalColor = 1.0f * sum / count;
	return float4(finalColor, finalColor, finalColor, color4.a);
}

float4 PShaderBlend(float2 TexCoord : TEXCOORD0) : COLOR
{
	return tex2D(g_sampleMainColor, TexCoord);

	float4 color[9];
	float2 offset = float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight);
	//中，上，右上，右，右下，下，左下，左，左上
	color[0] = tex2D(g_sampleMainColor, TexCoord);
	color[1] = tex2D(g_sampleMainColor, TexCoord + offset * float2(0, -1));
	color[2] = tex2D(g_sampleMainColor, TexCoord + offset * float2(1, -1));
	color[3] = tex2D(g_sampleMainColor, TexCoord + offset * float2(1,  0));
	color[4] = tex2D(g_sampleMainColor, TexCoord + offset * float2(1,  1));
	color[5] = tex2D(g_sampleMainColor, TexCoord + offset * float2(0,  1));
	color[6] = tex2D(g_sampleMainColor, TexCoord + offset * float2(-1, 1));
	color[7] = tex2D(g_sampleMainColor, TexCoord + offset * float2(-1, 0));
	color[8] = tex2D(g_sampleMainColor, TexCoord + offset * float2(-1,-1));

	float3 normal[9];
	normal[0] = normalize(GetNormal(g_sampleNormal, TexCoord));
	normal[1] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(0, -1)));
	normal[2] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(1, -1)));
	normal[3] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(1,  0)));
	normal[4] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(1,  1)));
	normal[5] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(0,  1)));
	normal[6] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(-1, 1)));
	normal[7] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(-1, 0)));
	normal[8] = normalize(GetNormal(g_sampleNormal, TexCoord + offset * float2(-1,-1)));

	//return float4(color[1].a, color[1].a, color[1].a, 1);

	int countN = 0;
	float judge = 0;

	if (color[0].a < 0.5)
	{
		for (int i = 1; i < 9; i++)
		{
			if (color[i].a < 0.5)
			{
				countN++;
				judge += abs(dot(normal[i], normal[0]));

			}
		}
	}
	judge = judge / countN;
	judge = judge*judge;

	return float4(judge, judge, judge, 1);

	if ((color[1].a < 0.5 || color[3].a < 0.5 || color[5].a < 0.5 || color[7].a < 0.5) && color[0].a > 0.5)
	{
		//return float4(color[1].a, color[1].a, color[1].a, 1);
		return float4(normal[0], 1);
	}
	//return float4(color[1].a, color[1].a, color[1].a, 1);
	return tex2D(g_sampleMainColor, TexCoord);

	for (int i = 0; i < 9; i++)
	{
		//if (color[i] > 0.3f && color())
	}

	if (color[0].r > 0.3f && color[0].r - color[1].r >= 0.3f  && color[0].r - color[3].r <= 0.3f)
	{
		return float4(1.0f, 0, 0, 1);
	}


	return tex2D(g_sampleMainColor, TexCoord);
	//return float4(1, 1, 0, 1);
}

struct P_OutVS
{
	float4 posWVP         : POSITION0;
	float2 TexCoord		  : TEXCOORD0;
	float  psize		  : PSIZE;
	float4 texC			  : COLOR0;
	float4 thickness	  : COLOR1;
};

P_OutVS VShaderParticle(float4 posL       : POSITION0,
	float2 TexCoord : TEXCOORD0)
{
	P_OutVS outVS = (P_OutVS)0;

	//最终输出的顶点位置（经过世界、观察、投影矩阵变换）
	//outVS.posWVP = mul(float4(0.5f, 0.5f, 0, 1), g_Proj);
	//outVS.posWVP = float4(2*TexCoord.x-1, 1-2*TexCoord.y, 0, 1);
	outVS.posWVP = float4( - 2, -2, -2, 1);
	//outVS.TexCoord = TexCoord;
	
	outVS.psize = 0;

	//DiffuseMap的纹理采样
	float4 diffuseColor = tex2Dlod(g_sampleDiffuse, float4(TexCoord.x, TexCoord.y, 0, 0)) * float4(2,2,2,1);
	float4 color = tex2Dlod(g_sampleMainColor, float4(TexCoord.x, TexCoord.y, 0, 0));

	float thickness = 1 - color.r;
	outVS.thickness = float4(thickness, thickness, thickness, thickness);
	outVS.texC = float4(TexCoord, 0, 0);

	float size = 0;
	size = thickness * g_baseTexSize;
	
	//size = clamp(size, 0, g_maxTexSize);
	float depth = tex2Dlod(g_samplePosition, float4(TexCoord.x, TexCoord.y, 0, 0));
	//将深度转化为0-1区间内的值，深度越大，invDepth越小
	float invDepth = 1 - depth / g_zFar;
	//也就是说深度越大，计算出的纹理大小越小，而最大值不会超过g_maxTexSize*sizeFactor，最小不会小于g_minTexSize
	float sizeFactor = 0.8;
	float scaledSize = 0;

	//if (TexCoord.x > 0.5)
	{
		float scale = 1.0f * g_zNear / depth * diffuseColor;
		
		//距离是_的时候，更改大小调整的曲线
		float limit = 17;
		float b = 0.009;
		if (depth <= limit)
		{
			scale = (g_zNear - limit * b) / (limit * limit * limit) * depth * depth + b;
			scale = scale * diffuseColor;
		}
		
		scaledSize = size * scale;

		//scaledSize = clamp(scaledSize, g_minTexSize * diffuseColor, g_maxTexSize);
		scaledSize = max(scaledSize, g_minTexSize * diffuseColor);

	}

	if (scaledSize >= 1 && thickness > 0.0f)
	{
		outVS.posWVP = float4(2 * TexCoord.x - 1, 1 - 2 * TexCoord.y, 0, 1);
		outVS.psize = scaledSize;
	}

	return outVS;
}

float4 PShaderParticle(float2 TexCoord : TEXCOORD0,
	float4 thickness : COLOR1,
	float4 color : COLOR0) : COLOR
{
	float3 normal = normalize(GetNormal(g_sampleNormal, color.xy));
	//return float4(normal, 1.0f);

	float projectXY = sqrt(normal.x * normal.x + normal.y * normal.y);
	float cosA = normal.x / projectXY;
	float A = acos(cosA);
	if (normal.y < 0)
		A = -acos(cosA);

	//return normal;

	//HLSL内部声明的矩阵是列矩阵
	//vec(A,B) mat|m00,m01|
	//------------|m10,m11|
	//矩阵中储存的的数据是row0(m00,m10),row1(m01,m11)
	//这样mul计算的时候就可以这样计算
	//x=dp(vec,row0),y=dp(vec,row1),dp是笛卡尔积
	//但是在C++代码中储存的还是行向量，因此在SetMatrix的时候是会自动进行转置的
	//所以如果在hlsl直接声明矩阵，或者没有通过SetMatrix传入的话，就需要改变左右位置
	//所以这里写成mul(矩阵，向量)
	float2x2 rotationM = float2x2(float2(cos(A), -sin(A)), float2(sin(A), cos(A)));
	//TexCoord = mul(TexCoord - float2(0.5,0.5), rotationM) + float2(0.5,0.5);
	TexCoord = mul(rotationM, TexCoord - float2(0.5, 0.5)) + float2(0.5, 0.5);
	
	//扭曲纹理，初步代码，未完成
	float2x2 tranS = float2x2(float2(1, 0.2* sin(TexCoord.y*3.141592653) / TexCoord.y), float2(0, 1));
		//TexCoord = mul(TexCoord, tranS);
		//TexCoord = mul(tranS, TexCoord);

		//TexCoord = saturate(TexCoord);
		float4 brush = float4(1, 1, 1, 1);

		float4 brush0 = tex2D(g_sampleInkTex, TexCoord);
		float4 brush1 = tex2D(g_sampleInkTex1, TexCoord);
		float4 brush2 = tex2D(g_sampleInkTex2, TexCoord);
		float4 brush3 = tex2D(g_sampleInkTex3, TexCoord);
		float4 brush4 = tex2D(g_sampleInkTex4, TexCoord);

		brush = brush2;
	if (thickness.x <= 0.5f)
	{
		float tmp = thickness.x / 0.5f;
		brush = lerp(brush4, brush3, 2 * tmp - tmp*tmp); //y = 2x-x^2
	}
	else if (thickness.x < 1)
	{
		brush = lerp(brush3, brush2, pow((thickness.x - 0.5f), 2) / pow((1 - 0.5f), 2)); //
	}
	//brush = lerp(brush4, brush3, size.x / g_maxTexSize);
		//brush.rgb = float3(0.5, 0.5, 0.5);
	brush.a = 1;

	float alpha = 0.3;
	if (thickness.x < 1 && thickness.x > 0.5f)
		alpha = thickness.x / 2;
	if (thickness.x <= 0.5f)
		alpha = thickness.x / 3;

	float colorFactor = g_colorFactor;
	if (brush.r > 0.59f)
	{
		brush = float4(1.0f, 1.0f, 1.0f, 0.0f);
	}
	else
	{
		brush = float4(colorFactor, colorFactor, colorFactor, alpha);
		//brush = float4(normal, 0.5f);
	}
		
	if (TexCoord.x < 0 || TexCoord.x > 1 || TexCoord.y < 0 || TexCoord.y > 1)
		brush = float4(1.0f, 1.0f, 1.0f, 0.0f);

	if (brush.r > 0.99f)
		brush = float4(1.0f, 1.0f, 1.0f, 0.0f);
	
	return brush;
}

float g_baseColorScale = 0;
float g_maxColorScale = 0;

struct PInside_OutVS
{
	float4 posWVP         : POSITION0;
	float2 TexCoord		  : TEXCOORD0;
	float  psize		  : PSIZE;
	float4 texC		      : COLOR0;
	float4 color		  : COLOR1;
};

PInside_OutVS VShaderParticleInside(float4 posL       : POSITION0,
	float2 TexCoord : TEXCOORD0)
{
	PInside_OutVS outVS = (PInside_OutVS)0;

	//先把所有顶点移动到区域外
	outVS.posWVP = float4(-2, -2, -2, 1);
	//默认颜色为0
	float thickness = 0;
	//GrayScaleMap的纹理采样
	float4 texColor = tex2Dlod(g_sampleMainColor, float4(TexCoord.x, TexCoord.y, 0, 0));
	//DiffuseMap的纹理采样
	float4 diffuseColor = tex2Dlod(g_sampleDiffuse, float4(TexCoord.x, TexCoord.y, 0, 0)) * float4(2, 2, 2, 1);

	//根据灰度图的颜色确定纹理颜色，这里值越大颜色越深,也可以理解为墨的浓度值
	thickness = 1 - diffuseColor.g;

	//根据diffuse的alpha来判断材质类型
	if (diffuseColor.a * 255.0f <= 0.5f)
	{
		thickness = 1 - diffuseColor.g;
	}
	else if (diffuseColor.a * 255.0f <= 1.5f && diffuseColor.a * 255.0f > 0.5f)
	{
		thickness = 1 - texColor.r;
	}
	else if (diffuseColor.a * 255.0f <= 2.5f && diffuseColor.a * 255.0f > 1.5f)
	{
		thickness = (1 - texColor.r * diffuseColor.b)*(diffuseColor.g);
	}
	else
	{
		thickness = 1 - diffuseColor.g;
	}

	//因为这里不需要纹理大小有区别，所以直接根据深度来确定距离
	float depth = tex2Dlod(g_samplePosition, float4(TexCoord.x, TexCoord.y, 0, 0));

	int a = 4;
	if (texColor.r < 0.4)
	{
		//float invDepth = 1 - depth / g_zFar;
		//a = 4 * invDepth;
		//a = 1;

		if (depth < 5)a = 4;
		else if (depth < 10)a = 3;
		else if (depth < 15)a = 2;
		else if (depth < 20)a = 1;
		else a = 1;
	}
	a = clamp(a, 1, 4);
	//a = 5;
	int pixIndex_X = TexCoord.x * g_ScreenWidth;
	int pixIndex_Y = TexCoord.y * (g_ScreenHeight+1);

	int mod_X = pixIndex_X - pixIndex_X / a * a;
	int mod_Y = pixIndex_Y - pixIndex_Y / a * a;

	if (mod_X < 1 && mod_Y < 1)
	{
		//根据传入的顶点的UV坐标来计算空间位置
		outVS.posWVP = float4(2 * TexCoord.x - 1, 1 - 2 * TexCoord.y, 0, 1);
	}
	
	
	//深度越大，纹理大小越小
	float sizeScaled = 1.0f * g_baseInsideTexSize * g_zNear / depth;

	outVS.psize = clamp(sizeScaled, g_minInsideTexSize, g_maxInsideTexSize);


	
	if (depth > 150)
	{
		float maxminFactor = depth / 150.0f;
		maxminFactor = clamp(maxminFactor, 1, 2);
		outVS.psize = clamp(sizeScaled, g_minInsideTexSize * maxminFactor, g_maxInsideTexSize);
	}

	if (depth > 1000) outVS.psize = 0;
	//储存纹理坐标，便于后面的计算
	outVS.texC = float4(TexCoord, 0, 0);
	//储存控制透明度的size和实际的纹理粒子大小
	outVS.color = float4(thickness, outVS.psize, 0, 0);

	return outVS;
}

float4 PShaderParticleInside(float2 TexCoord : TEXCOORD0,//粒子内部的纹理坐标
	float4 texC : COLOR0,//粒子中心点的全局纹理坐标
	float4 color : COLOR1
	) : COLOR
{
	//float outC = 1 - color, x;
	//return float4(outC, outC, outC, 1.0);
	//将粒子的局部纹理坐标转换到以粒子中心为原点的坐标
	float2 localT = TexCoord - float2(0.5, 0.5);
	//size.y是粒子的实际大小（像素），根据粒子大小、粒子纹理内部坐标和中心坐标，计算出粒子纹理上每个像素的全局坐标
	float2 globleT = texC.xy + float2(1.0f / g_ScreenWidth, 1.0f / g_ScreenHeight)* color.y * localT;
	float depth = tex2D(g_samplePosition, globleT);

	float4 diffuseColor = tex2D(g_sampleDiffuse, TexCoord) * float4(2, 2, 2, 1);
	float4 diffuseColor2 = tex2D(g_sampleDiffuse, texC.xy) * float4(2, 2, 2, 1);
	//如果深度过大，说明是没有渲染的部分，把那部分的纹理切掉
	//实际上这里后面应该改成和中心部分相差大于阈值后切掉
	float depthCenter = tex2D(g_samplePosition, texC.xy);
	if (abs(depth - depthCenter) > 0.2 && diffuseColor2.r > 0.3f)
		return float4(0,0,0,0);

	//获取中心点对应的法线
	float3 normal = normalize(GetNormal(g_sampleNormal, texC.xy));
	//return float4(normal, 1.0f);
	//计算法线在屏幕上的投影，据此计算出旋转角度
	float projectXY = sqrt(normal.x * normal.x + normal.y * normal.y);
	float cosA = normal.x / projectXY;
	float A = acos(cosA);
	if (normal.y < 0)
		A = -acos(cosA);

	//根据中心点坐标和法线计算一个伪随机数（其实完全不是随机），让随机数在0到1的范围内，角度是0-3.14，加1还算合理
	float random = (texC.x * texC.y + texC.x / texC.y) * (normal.x + normal.y + normal.z) / (normal.x * normal.y * normal.z);
	//random = (TexCoord.x * TexCoord.y + TexCoord.x / TexCoord.y) * (normal.x + normal.y + normal.z) / (normal.x * normal.y * normal.z);
	//random = clamp(random, 0, 3.1416f);
	random = frac(random);
	random *= (3.1415f / 2.0f);
	A += random;

	//HLSL内部声明的矩阵是列矩阵
	//vec(A,B) mat|m00,m01|
	//------------|m10,m11|
	//矩阵中储存的的数据是row0(m00,m10),row1(m01,m11)
	//这样mul计算的时候就可以这样计算
	//x=dp(vec,row0),y=dp(vec,row1),dp是笛卡尔积
	//但是在C++代码中储存的还是行向量，因此在SetMatrix的时候是会自动进行转置的
	//所以如果在hlsl直接声明矩阵，或者没有通过SetMatrix传入的话，就需要改变左右位置
	//所以这里写成mul(矩阵，向量)
	float2x2 rotationM = float2x2(float2(cos(A), -sin(A)), float2(sin(A), cos(A)));
	//TexCoord = mul(TexCoord - float2(0.5,0.5), rotationM) + float2(0.5,0.5);
	TexCoord = mul(rotationM, TexCoord - float2(0.5, 0.5)) + float2(0.5, 0.5);

	//TexCoord = saturate(TexCoord);
	float4 brush = float4(1, 1, 1, 1);
	brush = tex2D(g_sampleInkTex, TexCoord);

	//使用alphaTest处理笔迹纹理
	float alphaTestFactor = g_alphaTestFactor;
	if (brush.r < alphaTestFactor)
	{
		float factor = 0.0f;
		float thickness = color.x;
		float bC = (1 - factor) * thickness + factor;
		brush = float4(0, 0, 0, bC);// *1.5;
		//brush = float4(normal, 0.5f);
	}
	else
	{
		brush = float4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	if (TexCoord.x < 0 || TexCoord.x > 1 || TexCoord.y < 0 || TexCoord.y > 1)
		brush = float4(0.0f, 0.0f, 0.0f, 0.0f);

	if (brush.r > 0.99f)
		brush = float4(0.0f, 0.0f, 0.0f, 0.0f);

	return brush;
}

technique ColorChange
{
	pass p0
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShaderGrayscale();
	}
	
	pass p1
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PSAreaSplit();
	}

	pass p2
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShaderBlend();
	}

	pass p3
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShaderBlur();
	}

	pass p5
	{
		vertexShader = compile vs_3_0 VShader();
		pixelShader = compile ps_3_0 PShaderEdgeBlur();

		AlphaBlendEnable = false;
	}

	pass p6
	{
		vertexShader = compile vs_3_0 VShaderParticle();
		pixelShader = compile ps_3_0 PShaderParticle();

		AlphaBlendEnable = true;
		SrcBlend = SRCALPHA;
		DestBlend = INVSRCALPHA;
	}

	pass p7
	{
		vertexShader = compile vs_3_0 VShaderParticleInside();
		pixelShader = compile ps_3_0 PShaderParticleInside();

		AlphaBlendEnable = true;
		SrcBlend = SRCALPHA;
		DestBlend = INVSRCALPHA;
	}
}