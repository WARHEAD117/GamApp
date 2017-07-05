#include "sumi_e.h"
#include "D3D9Device.h"
#include "CommonUtil/Input/Input.h"
#include "RenderUtil/EffectParam.h"
#include "RenderSystem/RenderPipeLine/RenderPipe.h"
#include "Camera/CameraParam.h"
#include <math.h>

SumiE::SumiE()
{
}


SumiE::~SumiE()
{
}
struct Particle
{
	D3DXVECTOR3 particlePos;
	D3DXVECTOR2	uv;
	//static LPDIRECT3DVERTEXDECLARATION9 mParticleDecl;
};

const D3DVERTEXELEMENT9 PARTICLEVERTEXDECL[] =
{
	{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	D3DDECL_END()
};

int w = 100; //RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth
int h = 100; //RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight
int w2 = 100; //RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth
int h2 = 100; //RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight
int baseTexSize = 0;
int minTexSize = 0;
int maxTexSize = 0;
int baseInsideTexSize = 0;
int maxInsideTexSize = 0;
int minInsideTexSize = 0;

void SumiE::CreatePostEffect()
{
	///////////////////////////////////////////////////////////
	RENDERDEVICE::Instance().g_pD3DDevice->CreateVertexDeclaration(PARTICLEVERTEXDECL, &mParticleDecl);
	int particleNum = RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth * RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight;
	RENDERDEVICE::Instance().g_pD3DDevice->CreateVertexBuffer(particleNum*sizeof(Particle),
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_POINTS,
		0, D3DPOOL_DEFAULT, &mParticleBuffer, 0);

	Particle* p = 0;
	mParticleBuffer->Lock(0, 0, (void**)&p, D3DLOCK_DISCARD);

	//w = 100; //RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth
	//h = 100; //RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight
	w = RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / 1;
	h = RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight / 1;
	// For each living particle.
	for (UINT j = 0; j < h; ++j)
	{
		for (UINT i = 0; i < w; ++i)
		{
			// Copy particle to VB
			p[j*w+i].uv = D3DXVECTOR2(i * 1.0f / w, j * 1.0f / h);
		}
	}
	mParticleBuffer->Unlock();
	///////////////////////////////////////////////////////////
	RENDERDEVICE::Instance().g_pD3DDevice->CreateVertexBuffer(particleNum*sizeof(Particle),
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_POINTS,
		0, D3DPOOL_DEFAULT, &mParticleBuffer2, 0);

	Particle* p2 = 0;
	mParticleBuffer2->Lock(0, 0, (void**)&p2, D3DLOCK_DISCARD);

	//w = 100; //RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth
	//h = 100; //RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight
	w2 = RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / 1;
	h2 = RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight / 1;
	// For each living particle.
	for (UINT j = 0; j < h2; ++j)
	{
		for (UINT i = 0; i < w2; ++i)
		{
			// Copy particle to VB
			p2[j*w2 + i].uv = D3DXVECTOR2(i * 1.0f / w2, j * 1.0f / h2);
		}
	}
	mParticleBuffer2->Unlock();
	///////////////////////////////////////////////////////////



	PostEffectBase::CreatePostEffect("System\\Sumi_e.fx");

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pEdgeBlur, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pEdgeBlur2, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pJudgeImage, NULL);

	//create renderTarget
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_StrokesArea, NULL);
	
	//create renderTarget
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_StrokesArea2, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pInsideTarget, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pInsideTarget2, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pBluredInside, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pDarkPart, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pHorizontalBlur, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pVerticalBlur, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pTexList[0], NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pTexList[1], NULL);
	for (int i = 0; i < texCount; i++)
	{
		RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
			1, D3DUSAGE_RENDERTARGET,
			D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
			&m_pTexList[i], NULL);
	}

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pFootprintTarget, NULL);
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pFootprintTarget_LF, NULL);
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pDiffusionTarget, NULL);
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pOUTTarget, NULL);

	

	ID3DXBuffer* error = 0;
	if (E_FAIL == ::D3DXCreateEffectFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "System\\SumieSynthesis.fx", NULL, NULL, D3DXSHADER_DEBUG,
		NULL, &m_SynthesisEffect, &error))
	{
		MessageBox(GetForegroundWindow(), (char*)error->GetBufferPointer(), "sumieSynthesis", MB_OK);
		abort();
	}
	//==========================================================
	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "Res\\1.bmp", &m_pInkTex1))
	{
		MessageBox(GetForegroundWindow(), "TextureError", "InkTex1", MB_OK);
		abort();
	}
	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "Res\\2.bmp", &m_pInkTex2))
	{
		MessageBox(GetForegroundWindow(), "TextureError", "InkTex2", MB_OK);
		abort();
	}
	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "Res\\3.bmp", &m_pInkTex3))
	{
		MessageBox(GetForegroundWindow(), "TextureError", "InkTex3", MB_OK);
		abort();
	}


	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "Res\\insideMask.bmp", &m_pInkMask))
	{
		MessageBox(GetForegroundWindow(), "TextureError", "InkMask", MB_OK);
		abort();
	}

	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "Res\\paper.jpg", &m_pBackground))
	{
		MessageBox(GetForegroundWindow(), "TextureError", "background1", MB_OK);
		abort();
	}

	
	
}
int test = 0;
int useWhich2 = 1;
void ConfigInput()
{
	if (GAMEINPUT::Instance().KeyDown(DIK_I) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		useWhich2 = 1;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_K) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		useWhich2 = 2;
	}
}

int sampleCount = 30;
float m_SampleWeights[30];
float m_SampleOffsets[60];

void SumiE::RenderPost(LPDIRECT3DTEXTURE9 mainBuffer)
{
	ConfigInput();
	m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);
	m_postEffect->SetMatrix(INVPROJMATRIX, &RENDERDEVICE::Instance().InvProjMatrix);
	m_postEffect->SetTexture(POSITIONBUFFER, RENDERPIPE::Instance().m_pPositionTarget);

	m_postEffect->SetFloat("g_zNear", CameraParam::zNear);
	m_postEffect->SetFloat("g_zFar", CameraParam::zFar);

	m_postEffect->SetInt(SCREENWIDTH, RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth);
	m_postEffect->SetInt(SCREENHEIGHT, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight);

	float angle = tan(CameraParam::FOV / 2);
	m_postEffect->SetFloat("g_ViewAngle_half_tan", angle);
	float aspect = (float)RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight;
	m_postEffect->SetFloat("g_ViewAspect", aspect);

	m_postEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);

	UINT numPasses = 0;
	m_postEffect->Begin(&numPasses, 0);

	
	//分割灰度图，分成深色部分和浅色部分
	//StrokesArea
	PDIRECT3DSURFACE9 pSurf_SA = NULL;
	m_StrokesArea->GetSurfaceLevel(0, &pSurf_SA);

	PDIRECT3DSURFACE9 pSurf_SA2 = NULL;
	m_StrokesArea2->GetSurfaceLevel(0, &pSurf_SA2);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_SA2);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(1, pSurf_SA);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);


	m_postEffect->SetTexture("g_GrayscaleBuffer", RENDERPIPE::Instance().m_pGrayscaleTarget);



	bool useSplit = false;
	if (useSplit)
	{
		m_postEffect->SetInt("minI", 60);//70
		m_postEffect->SetInt("maxI", 150);//120
		m_postEffect->SetInt("minI_2", 180);//70
		m_postEffect->SetInt("maxI_2", 255);//120
	}
	else
	{
		m_postEffect->SetInt("minI", 60);//70
		m_postEffect->SetInt("maxI", 150);//120
		m_postEffect->SetInt("minI_2", 100);//70
		m_postEffect->SetInt("maxI_2", 180);//120
	}

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(0);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	SafeRelease(pSurf_SA);
	SafeRelease(pSurf_SA2);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, NULL);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(1, NULL);
	

	//=============================================================================================================
	//模糊轮廓图
	PDIRECT3DSURFACE9 pSurf_EdgeBlur = NULL;
	m_pEdgeBlur2->GetSurfaceLevel(0, &pSurf_EdgeBlur);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_EdgeBlur);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture(POSITIONBUFFER, RENDERPIPE::Instance().m_pPositionTarget);
	m_postEffect->SetTexture(MAINCOLORBUFFER, mainBuffer);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(3);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	//模糊轮廓图(第二次)
	pSurf_EdgeBlur = NULL;
	m_pEdgeBlur->GetSurfaceLevel(0, &pSurf_EdgeBlur);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_EdgeBlur);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture(POSITIONBUFFER, RENDERPIPE::Instance().m_pPositionTarget);
	m_postEffect->SetTexture(MAINCOLORBUFFER, m_pEdgeBlur2);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(3);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();
	//m_pEdgeBlur = m_pEdgeBlur2;
	//=============================================================================================================
	//用来测试各种RT的效果
	PDIRECT3DSURFACE9 pSurf_Judge = NULL;
	m_pJudgeImage->GetSurfaceLevel(0, &pSurf_Judge);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pPostSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture(MAINCOLORBUFFER, m_pEdgeImage); //m_pEdgeImage//RENDERPIPE::Instance().m_pGrayscaleTarget//m_pEdgeBlur//mainBuffer//m_pEdgeForward//RENDERPIPE::Instance().m_pNormalTarget//m_StrokesArea

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(1);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	//=====================================================================================================
	//实际的水墨画渲染部分
	float useParticle = false;
	useParticle = true;
	bool openInsideParticle = false;
	openInsideParticle = true;

	//渲染第一次的内部粒子，浅色部分
	if (openInsideParticle && useParticle)
	{
		//渲染内部纹理
		PDIRECT3DSURFACE9 pSurf_Inside = NULL;
		m_pInsideTarget->GetSurfaceLevel(0, &pSurf_Inside);
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_Inside);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, true);

		D3DXMATRIX temp = RENDERDEVICE::Instance().ViewMatrix * RENDERDEVICE::Instance().ProjMatrix;
		m_postEffect->SetMatrix(VIEWPROJMATRIX, &temp);
		m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &temp);
		m_postEffect->SetMatrix(PROJECTIONMATRIX, &RENDERDEVICE::Instance().ProjMatrix);

		m_postEffect->SetFloat("g_zNear", CameraParam::zNear);
		m_postEffect->SetFloat("g_zFar", CameraParam::zFar);
		m_postEffect->SetInt(SCREENWIDTH, RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth);
		m_postEffect->SetInt(SCREENHEIGHT, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight);

		m_postEffect->SetTexture(MAINCOLORBUFFER, m_StrokesArea);
		m_postEffect->SetTexture(NORMALBUFFER, RENDERPIPE::Instance().m_pNormalTarget);
		m_postEffect->SetTexture(POSITIONBUFFER, RENDERPIPE::Instance().m_pPositionTarget);
		m_postEffect->SetTexture(DIFFUSEBUFFER, RENDERPIPE::Instance().m_pDiffuseTarget);
		m_postEffect->SetTexture("g_JudgeTex", mainBuffer); //RENDERPIPE::Instance().m_pGrayscaleTarget
		m_postEffect->SetTexture("g_InkTex1", m_pInkMask);
		m_postEffect->SetBool("g_UpperLayer", false);

		baseInsideTexSize = 2130;
		maxInsideTexSize = 33;
		minInsideTexSize = 10;

		m_postEffect->SetInt("g_baseInsideTexSize", baseInsideTexSize);
		m_postEffect->SetInt("g_maxInsideTexSize",  maxInsideTexSize);
		m_postEffect->SetInt("g_minInsideTexSize",  minInsideTexSize);

		m_postEffect->SetFloat("g_SizeFactor",   1.0f);
		m_postEffect->SetFloat("g_alphaTestFactor", 0.79f);

		m_postEffect->CommitChanges();

		m_postEffect->BeginPass(5);
		RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, mParticleBuffer2, 0, sizeof(Particle));
		RENDERDEVICE::Instance().g_pD3DDevice->SetVertexDeclaration(mParticleDecl);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_POINTLIST, 0, w2*h2);
		m_postEffect->EndPass();

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, false);
	}

	//渲染第二次的内部粒子，深色部分
	if (openInsideParticle && useParticle && useSplit)
	{
		//渲染内部纹理
		PDIRECT3DSURFACE9 pSurf_Inside = NULL;
		m_pInsideTarget2->GetSurfaceLevel(0, &pSurf_Inside);
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_Inside);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, true);

		D3DXMATRIX temp = RENDERDEVICE::Instance().ViewMatrix * RENDERDEVICE::Instance().ProjMatrix;
		m_postEffect->SetMatrix(VIEWPROJMATRIX, &temp);
		m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &temp);
		m_postEffect->SetMatrix(PROJECTIONMATRIX, &RENDERDEVICE::Instance().ProjMatrix);

		m_postEffect->SetFloat("g_zNear", CameraParam::zNear);
		m_postEffect->SetFloat("g_zFar", CameraParam::zFar);
		m_postEffect->SetInt(SCREENWIDTH, RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth);
		m_postEffect->SetInt(SCREENHEIGHT, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight);

		m_postEffect->SetTexture(MAINCOLORBUFFER, m_StrokesArea2);
		m_postEffect->SetTexture(NORMALBUFFER, RENDERPIPE::Instance().m_pNormalTarget);
		m_postEffect->SetTexture(POSITIONBUFFER, RENDERPIPE::Instance().m_pPositionTarget);
		m_postEffect->SetTexture(DIFFUSEBUFFER, RENDERPIPE::Instance().m_pDiffuseTarget);
		m_postEffect->SetTexture("g_InkTex1", m_pInkMask);
		m_postEffect->SetTexture("g_JudgeTex", mainBuffer); //RENDERPIPE::Instance().m_pGrayscaleTarget
		
		m_postEffect->SetBool("g_UpperLayer", true);

		baseInsideTexSize = 2130;
		maxInsideTexSize = 33;
		minInsideTexSize = 10;

		m_postEffect->SetInt("g_baseInsideTexSize", baseInsideTexSize);
		m_postEffect->SetInt("g_maxInsideTexSize", maxInsideTexSize);
		m_postEffect->SetInt("g_minInsideTexSize", minInsideTexSize);

		m_postEffect->SetFloat("g_SizeFactor", 1.0f);
		m_postEffect->SetFloat("g_alphaTestFactor", 0.79f);

		m_postEffect->CommitChanges();

		m_postEffect->BeginPass(5);
		RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, mParticleBuffer2, 0, sizeof(Particle));
		RENDERDEVICE::Instance().g_pD3DDevice->SetVertexDeclaration(mParticleDecl);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_POINTLIST, 0, w2*h2);
		m_postEffect->EndPass();

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, false);
	}
	
	m_postEffect->End();

	//=======================================================================================================
	//合并各种纹理
	//=======================================================================================================
	if (openInsideParticle && useParticle)
	{
		
		numPasses = 0;
		m_SynthesisEffect->Begin(&numPasses, 0);

		m_SynthesisEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);
		m_SynthesisEffect->SetMatrix(INVPROJMATRIX, &RENDERDEVICE::Instance().InvProjMatrix);
		m_SynthesisEffect->SetMatrix(PROJECTIONMATRIX, &RENDERDEVICE::Instance().ProjMatrix);
		m_SynthesisEffect->SetMatrix(VIEWMATRIX, &RENDERDEVICE::Instance().ViewMatrix);

		m_SynthesisEffect->SetFloat("g_zNear", CameraParam::zNear);
		m_SynthesisEffect->SetFloat("g_zFar", CameraParam::zFar);

		float angle = tan(CameraParam::FOV / 2);
		m_SynthesisEffect->SetFloat("g_ViewAngle_half_tan", angle);
		float aspect = (float)RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight;
		m_SynthesisEffect->SetFloat("g_ViewAspect", aspect);
		m_SynthesisEffect->SetInt(SCREENWIDTH, RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth);
		m_SynthesisEffect->SetInt(SCREENHEIGHT, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight);

		RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
		RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);

		m_SynthesisEffect->CommitChanges();

		
		//======================================================================================================
		//合并两次的内部纹理，并高斯模糊
		PDIRECT3DSURFACE9 pSurf_BlurredInside = NULL;
		m_pBluredInside->GetSurfaceLevel(0, &pSurf_BlurredInside);

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_BlurredInside);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);


		m_SynthesisEffect->SetTexture("g_Inside", m_pInsideTarget);
		if (useSplit)
		{
			m_SynthesisEffect->SetTexture("g_Inside2", m_pInsideTarget2);
		}
		else
		{
			m_SynthesisEffect->SetTexture("g_Inside2", m_pInsideTarget);
		}

		m_SynthesisEffect->CommitChanges();

		m_SynthesisEffect->BeginPass(1);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_SynthesisEffect->EndPass();

		//======================================================================================================
		//提取暗部
		PDIRECT3DSURFACE9 pSurf_DarkPart = NULL;
		m_pDarkPart->GetSurfaceLevel(0, &pSurf_DarkPart);

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_DarkPart);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

		m_SynthesisEffect->SetTexture("g_Inside", m_pBluredInside);

		m_SynthesisEffect->CommitChanges();

		m_SynthesisEffect->BeginPass(4);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_SynthesisEffect->EndPass();

		//======================================================================================================
		//水平模糊
		PDIRECT3DSURFACE9 pSurf_HorizontalBlur = NULL;
		m_pHorizontalBlur->GetSurfaceLevel(0, &pSurf_HorizontalBlur);

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_HorizontalBlur);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

		m_SynthesisEffect->SetTexture(POSITIONBUFFER, RENDERPIPE::Instance().m_pPositionTarget);
		m_SynthesisEffect->SetTexture("g_InputTex", m_pDarkPart);

		SetGaussian(m_SynthesisEffect, 1.0f / RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, 0, "g_SampleWeights", "g_SampleOffsets");

		m_SynthesisEffect->CommitChanges();

		m_SynthesisEffect->BeginPass(5);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_SynthesisEffect->EndPass();
		//======================================================================================================
		//垂直模糊
		PDIRECT3DSURFACE9 pSurf_VerticalBlur = NULL;
		m_pVerticalBlur->GetSurfaceLevel(0, &pSurf_VerticalBlur);

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_VerticalBlur);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

		m_SynthesisEffect->SetTexture(POSITIONBUFFER, RENDERPIPE::Instance().m_pPositionTarget);
		m_SynthesisEffect->SetTexture("g_InputTex", m_pHorizontalBlur);

		SetGaussian(m_SynthesisEffect, 0, 1.0f / RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight, "g_SampleWeights", "g_SampleOffsets");

		m_SynthesisEffect->CommitChanges();

		m_SynthesisEffect->BeginPass(5);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_SynthesisEffect->EndPass();

		//======================================================================================================
		//第二次高斯模糊内部的纹理
		//并且与背景进行混合
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pPostSurface);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 255, 255, 255), 1.0f, 0);

		//只处理内部的纹理，边缘纹理其实是不应该和后面混合的。或者找找能让边缘的透明度更均匀的方法
		//m_SynthesisEffect->SetTexture("g_Contour", m_pContourTarget);
		m_SynthesisEffect->SetTexture("g_Bloomed", m_pVerticalBlur);
		
		m_SynthesisEffect->SetTexture("g_Inside", m_pBluredInside);
		//m_SynthesisEffect->SetTexture("g_Background", m_pBackground);

		m_SynthesisEffect->SetTexture("g_Background", RENDERDEVICE::Instance().defaultTexture);

		m_SynthesisEffect->SetFloat("g_zNear", CameraParam::zNear);
		m_SynthesisEffect->SetFloat("g_zFar", CameraParam::zFar);

		//控制内部黑色的程度
		m_SynthesisEffect->SetFloat("g_AlphaFactor", 0.5);
		m_SynthesisEffect->CommitChanges();


		m_SynthesisEffect->BeginPass(0);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_SynthesisEffect->EndPass();

		m_SynthesisEffect->End();
	}
	

	//====================================================================================================
	//渲染边缘纹理
	//因为边缘纹理现在颜色和透明度作用不同，就导致颜色看起来很正常但是透明度是不均匀的
	//这样如果也渲染到一张纹理上在融合的话就会出现问题，所以只能最后重新渲染一次
	//====================================================================================================
	bool openParticle = true;
	if (openParticle && useParticle)
	{
		m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);
		m_postEffect->SetMatrix(INVPROJMATRIX, &RENDERDEVICE::Instance().InvProjMatrix);
		m_postEffect->SetTexture(POSITIONBUFFER, RENDERPIPE::Instance().m_pPositionTarget);

		m_postEffect->SetFloat("g_zNear", CameraParam::zNear);
		m_postEffect->SetFloat("g_zFar", CameraParam::zFar);

		m_postEffect->SetInt(SCREENWIDTH, RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth);
		m_postEffect->SetInt(SCREENHEIGHT, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight);

		m_postEffect->CommitChanges();

		RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
		RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);

		UINT numPasses = 0;
		m_postEffect->Begin(&numPasses, 0);
		//=============================================================================================================
		
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pPostSurface);

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, true);

		D3DXMATRIX temp = RENDERDEVICE::Instance().ViewMatrix * RENDERDEVICE::Instance().ProjMatrix;
		m_postEffect->SetMatrix(VIEWPROJMATRIX, &temp);
		m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &temp);
		m_postEffect->SetMatrix(PROJECTIONMATRIX, &RENDERDEVICE::Instance().ProjMatrix);

		m_postEffect->SetTexture(MAINCOLORBUFFER, m_pEdgeBlur);

		m_postEffect->SetTexture(NORMALBUFFER, RENDERPIPE::Instance().m_pNormalTarget);
		m_postEffect->SetTexture("g_InkTex1", m_pInkTex1);
		m_postEffect->SetTexture("g_InkTex2", m_pInkTex2);
		m_postEffect->SetTexture("g_InkTex3", m_pInkTex3);

		m_postEffect->SetTexture("g_JudgeTex", m_pJudgeImage);

		baseTexSize = 3250;
		maxTexSize = 17;
		minTexSize = 9;
		m_postEffect->SetInt("g_baseTexSize", baseTexSize);
		m_postEffect->SetInt("g_minTexSize", minTexSize);
		m_postEffect->SetInt("g_maxTexSize", maxTexSize);

		//控制边缘黑色的程度
		m_postEffect->SetFloat("g_colorFactor", 0.3);

		m_postEffect->CommitChanges();

		m_postEffect->BeginPass(4);
		RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, mParticleBuffer, 0, sizeof(Particle));
		RENDERDEVICE::Instance().g_pD3DDevice->SetVertexDeclaration(mParticleDecl);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_POINTLIST, 0, w*h);
		m_postEffect->EndPass();

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, false);
		m_postEffect->End();

	}

	//渲染五张图，使用高斯模糊合并，来减轻轮廓的扰动
	bool openCache = true;
	//m_pTexList[0] = m_pTexList[1];
	//m_pTexList[1] = m_pPostTarget;

	if (useParticle && openCache)
	{
		numPasses = 0;
		m_SynthesisEffect->Begin(&numPasses, 0);

		m_SynthesisEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);
		m_SynthesisEffect->SetMatrix(INVPROJMATRIX, &RENDERDEVICE::Instance().InvProjMatrix);
		m_SynthesisEffect->SetMatrix(PROJECTIONMATRIX, &RENDERDEVICE::Instance().ProjMatrix);
		m_SynthesisEffect->SetMatrix(VIEWMATRIX, &RENDERDEVICE::Instance().ViewMatrix);

		m_SynthesisEffect->SetFloat("g_zNear", CameraParam::zNear);
		m_SynthesisEffect->SetFloat("g_zFar", CameraParam::zFar);

		float angle = tan(CameraParam::FOV / 2);
		m_SynthesisEffect->SetFloat("g_ViewAngle_half_tan", angle);
		float aspect = (float)RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight;
		m_SynthesisEffect->SetFloat("g_ViewAspect", aspect);
		m_SynthesisEffect->SetInt(SCREENWIDTH, RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth);
		m_SynthesisEffect->SetInt(SCREENHEIGHT, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight);

		RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
		RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
		//-------------------------------------------------------------------------------------------------------------------
		//生成墨迹图
		PDIRECT3DSURFACE9 pSurf_FP = NULL;
		pSurf_FP = NULL;
		m_pFootprintTarget->GetSurfaceLevel(0, &pSurf_FP);

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_FP);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 255, 255, 255), 1.0f, 0);

		m_SynthesisEffect->SetTexture("g_Src", m_pPostTarget);
		if (test == 0)
		{
			test = 1;
			m_SynthesisEffect->SetTexture("g_FP_LF", RENDERDEVICE::Instance().defaultTexture);
		}
		else
		{

			m_SynthesisEffect->SetTexture("g_FP_LF", m_pFootprintTarget_LF);
		}

		m_SynthesisEffect->CommitChanges();

		m_SynthesisEffect->BeginPass(6);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_SynthesisEffect->EndPass();

		//-------------------------------------------------------------------------------------------------------------------
		//使用墨迹图进行扩散，渲染扩散图
		pSurf_FP = NULL;
		m_pDiffusionTarget->GetSurfaceLevel(0, &pSurf_FP);

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_FP);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

		m_SynthesisEffect->SetTexture("g_Src", m_pFootprintTarget);

		m_SynthesisEffect->CommitChanges();

		m_SynthesisEffect->BeginPass(7);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_SynthesisEffect->EndPass();

		//-------------------------------------------------------------------------------------------------------------------
		//根据扩散图和水墨渲染的结果进行合成
		PDIRECT3DSURFACE9 pSurf_result = NULL;
		m_pOUTTarget->GetSurfaceLevel(0, &pSurf_result);

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_result);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

		m_SynthesisEffect->SetTexture("g_Src", m_pPostTarget);
		m_SynthesisEffect->SetTexture("g_Src2", m_pDiffusionTarget);

		m_SynthesisEffect->SetTexture(NORMALBUFFER, RENDERPIPE::Instance().m_pNormalTarget);
		m_SynthesisEffect->CommitChanges();

		m_SynthesisEffect->BeginPass(8);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_SynthesisEffect->EndPass();

		//-------------------------------------------------------------------------------------------------------------------
		//把扩散图渲染回FP_LF
		m_pFootprintTarget_LF->GetSurfaceLevel(0, &pSurf_result);

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_result);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

		m_SynthesisEffect->SetTexture("g_Src", m_pDiffusionTarget);

		m_SynthesisEffect->CommitChanges();

		m_SynthesisEffect->BeginPass(9);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_SynthesisEffect->EndPass();

		//-------------------------------------------------------------------------------------------------------------------
		pSurf_result = NULL;
		m_pPostTarget->GetSurfaceLevel(0, &pSurf_result);

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_result);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

		m_SynthesisEffect->SetTexture("g_Src", m_pOUTTarget);

		m_SynthesisEffect->CommitChanges();

		m_SynthesisEffect->BeginPass(9);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_SynthesisEffect->EndPass();

		m_SynthesisEffect->End();
	}
}


void SumiE::SetGaussian(LPD3DXEFFECT effect, float deltaX, float deltaY, std::string weightArrayName, std::string offsetArrayName)
{
	//===
	m_SampleWeights[0] = ComputeGaussianWeight(0);
	m_SampleOffsets[0] = 0.0f;
	m_SampleOffsets[1] = 0.0f;

	float totalWeights = m_SampleWeights[0];

	for (int i = 0; i < sampleCount / 2; i++)
	{
		float weight = ComputeGaussianWeight(i + 1);
		m_SampleWeights[i * 2 + 1] = weight;
		m_SampleWeights[i * 2 + 2] = weight;
		totalWeights += weight * 2;

		float sampleOffset = i * 2 + 1.5f;
		D3DXVECTOR2 delta = D3DXVECTOR2(deltaX, deltaY) * sampleOffset;

		m_SampleOffsets[i * 4 + 1] = delta.x;
		m_SampleOffsets[i * 4 + 2] = delta.y;
		m_SampleOffsets[i * 4 + 3] = -delta.x;
		m_SampleOffsets[i * 4 + 4] = -delta.y;
	}

	for (int i = 0; i < sampleCount; i++)
	{
		m_SampleWeights[i] /= totalWeights;
	}

	// 将计算结果传递到GaussianBlur特效
	effect->SetFloatArray(weightArrayName.c_str(), m_SampleWeights, sampleCount);
	effect->SetFloatArray(offsetArrayName.c_str(), m_SampleOffsets, sampleCount * 2);
	//==
}

float m_BlurAmount = 5.5;
float SumiE::ComputeGaussianWeight(float n)
{
	//高斯参数计算公式
	float theta = m_BlurAmount;
	return (float)((1.0 / sqrt(2 * D3DX_PI * theta)) *
		exp(-(n * n) / (2 * theta * theta)));
}

void SumiE::SetEdgeImage(LPDIRECT3DTEXTURE9 edgeImage)
{
	m_pEdgeImage = edgeImage;
}
