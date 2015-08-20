#include "sumi_e.h"
#include "D3D9Device.h"
#include "CommonUtil/Input/Input.h"
#include "RenderUtil/EffectParam.h"
#include "RenderSystem/RenderPipeLine/RenderPipe.h"
#include "Camera/CameraParam.h"

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
		&m_pEdgeForward, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_Garyscale, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_BlurredGaryscale, NULL);

	//create renderTarget
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_StrokesArea, NULL);
	
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pContourTarget, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pInsideTarget, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pBluredInside, NULL);

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
	

	ID3DXBuffer* error = 0;
	if (E_FAIL == ::D3DXCreateEffectFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "System\\SumieSynthesis.fx", NULL, NULL, D3DXSHADER_DEBUG,
		NULL, &m_SynthesisEffect, &error))
	{
		MessageBox(GetForegroundWindow(), (char*)error->GetBufferPointer(), "sumieSynthesis", MB_OK);
		abort();
	}
	//==========================================================
	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "Res\\testInkImage.png", &m_pTestInk))
	{
		MessageBox(GetForegroundWindow(), "TextureError", "testInkImage", MB_OK);
		abort();
	}
	//==========================================================
	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "Res\\0.bmp", &m_pInkTex))//3.bmp//R_Test.png//brush2.jpg
	{
		MessageBox(GetForegroundWindow(), "TextureError", "InkTex", MB_OK);
		abort();
	}
	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "Res\\1.bmp", &m_pInkTex1))//3.bmp//R_Test.png//brush2.jpg
	{
		MessageBox(GetForegroundWindow(), "TextureError", "InkTex", MB_OK);
		abort();
	}
	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "Res\\2.bmp", &m_pInkTex2))//3.bmp//R_Test.png//brush2.jpg
	{
		MessageBox(GetForegroundWindow(), "TextureError", "InkTex", MB_OK);
		abort();
	}
	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "Res\\3.bmp", &m_pInkTex3))//3.bmp//R_Test.png//brush2.jpg
	{
		MessageBox(GetForegroundWindow(), "TextureError", "InkTex", MB_OK);
		abort();
	}
	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "Res\\4.bmp", &m_pInkTex4))//3.bmp//R_Test.png//brush2.jpg
	{
		MessageBox(GetForegroundWindow(), "TextureError", "InkTex", MB_OK);
		abort();
	}

	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "Res\\testBrush2.bmp", &m_pInkMask))//3.bmp//R_Test.png//brush2.jpg//testBrush1.bmp
	{
		MessageBox(GetForegroundWindow(), "TextureError", "InkMask", MB_OK);
		abort();
	}

	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "Res\\background1.jpg", &m_pBackground))
	{
		MessageBox(GetForegroundWindow(), "TextureError", "background1", MB_OK);
		abort();
	}

	
	
}

void SumiE::RenderPost(LPDIRECT3DTEXTURE9 mainBuffer)
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
	//渲染灰度图
	PDIRECT3DSURFACE9 pSurf_Garyscale = NULL;
	m_Garyscale->GetSurfaceLevel(0, &pSurf_Garyscale);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_Garyscale);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture(NORMALBUFFER, RENDERPIPE::Instance().m_pNormalTarget);
	m_postEffect->SetTexture(POSITIONBUFFER, RENDERPIPE::Instance().m_pPositionTarget);
	m_postEffect->SetTexture(MAINCOLORBUFFER, mainBuffer);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(0);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	//=============================================================================================================
	//高斯模糊
	PDIRECT3DSURFACE9 pSurf_BlurredGaryscale = NULL;
	m_BlurredGaryscale->GetSurfaceLevel(0, &pSurf_BlurredGaryscale);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_BlurredGaryscale);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture("g_GrayscaleBuffer", m_Garyscale);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(3);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();
	//高斯模糊2
	PDIRECT3DSURFACE9 pSurf_BlurredGaryscale2 = NULL;
	m_Garyscale->GetSurfaceLevel(0, &pSurf_BlurredGaryscale2);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_BlurredGaryscale2);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture("g_GrayscaleBuffer", m_BlurredGaryscale);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(3);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();
	//高斯模糊3
	PDIRECT3DSURFACE9 pSurf_BlurredGaryscale3 = NULL;
	m_BlurredGaryscale->GetSurfaceLevel(0, &pSurf_BlurredGaryscale3);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_BlurredGaryscale3);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture("g_GrayscaleBuffer", m_Garyscale);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(3);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();
	//高斯模糊4
	PDIRECT3DSURFACE9 pSurf_BlurredGaryscale4 = NULL;
	m_Garyscale->GetSurfaceLevel(0, &pSurf_BlurredGaryscale4);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_BlurredGaryscale4);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture("g_GrayscaleBuffer", m_BlurredGaryscale);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(3);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();
	//高斯模糊5
	PDIRECT3DSURFACE9 pSurf_BlurredGaryscale5 = NULL;
	m_BlurredGaryscale->GetSurfaceLevel(0, &pSurf_BlurredGaryscale5);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_BlurredGaryscale5);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture("g_GrayscaleBuffer", m_Garyscale);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(3);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();
	//=============================================================================================================
	//StrokesArea
	PDIRECT3DSURFACE9 pSurf_SA = NULL;
	m_StrokesArea->GetSurfaceLevel(0, &pSurf_SA);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_SA);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture("g_GrayscaleBuffer", m_BlurredGaryscale);
	m_postEffect->SetInt("minI", 30);//70
	m_postEffect->SetInt("maxI", 165);//120

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(1);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	SafeRelease(pSurf_SA);
	//=============================================================================================================
	//去除边缘的后景像素
	PDIRECT3DSURFACE9 pSurf_EdgeForward = NULL;
	m_pEdgeForward->GetSurfaceLevel(0, &pSurf_EdgeForward);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_EdgeForward);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture(POSITIONBUFFER, RENDERPIPE::Instance().m_pPositionTarget);
	m_postEffect->SetTexture(MAINCOLORBUFFER, mainBuffer);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(4);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	//=============================================================================================================
	//模糊轮廓图
	PDIRECT3DSURFACE9 pSurf_EdgeBlur = NULL;
	m_pEdgeBlur2->GetSurfaceLevel(0, &pSurf_EdgeBlur);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_EdgeBlur);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture(POSITIONBUFFER, RENDERPIPE::Instance().m_pPositionTarget);
	m_postEffect->SetTexture(MAINCOLORBUFFER, m_pEdgeForward);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(5);
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

	m_postEffect->BeginPass(5);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();
	//m_pEdgeBlur = m_pEdgeBlur2;
	//=============================================================================================================
	//合并
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pPostSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture(MAINCOLORBUFFER, m_pTexList[1]); //m_pEdgeImage//m_pEdgeBlur//mainBuffer//m_pEdgeForward//RENDERPIPE::Instance().m_pNormalTarget//m_StrokesArea

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(2);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	//=====================================================================================================

	float useParticle = true;

	bool openInsideParticle = true;
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
		m_postEffect->SetTexture("g_InkTex", m_pInkMask);

		baseInsideTexSize = 2130;
		maxInsideTexSize = 33;
		minInsideTexSize = 10;

		m_postEffect->SetInt("g_baseInsideTexSize", baseInsideTexSize);
		m_postEffect->SetInt("g_maxInsideTexSize",  maxInsideTexSize);
		m_postEffect->SetInt("g_minInsideTexSize",  minInsideTexSize);

		m_postEffect->SetFloat("g_SizeFactor",   1.0f);
		m_postEffect->SetFloat("g_alphaTestFactor", 0.79f);

		m_postEffect->CommitChanges();

		m_postEffect->BeginPass(7);
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
		//高斯模糊内部的纹理
		PDIRECT3DSURFACE9 pSurf_BlurredInside = NULL;
		m_pBluredInside->GetSurfaceLevel(0, &pSurf_BlurredInside);

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_BlurredInside);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

		m_SynthesisEffect->SetTexture("g_Inside", m_pInsideTarget);

		m_SynthesisEffect->CommitChanges();

		m_SynthesisEffect->BeginPass(1);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_SynthesisEffect->EndPass();
		//======================================================================================================
		//第二次高斯模糊内部的纹理
		//并且与背景进行混合
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pPostSurface);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 255, 255, 255), 1.0f, 0);

		//只处理内部的纹理，边缘纹理其实是不应该和后面混合的。或者找找能让边缘的透明度更均匀的方法
		//m_SynthesisEffect->SetTexture("g_Contour", m_pContourTarget);
		m_SynthesisEffect->SetTexture("g_Inside", m_pBluredInside);
		m_SynthesisEffect->SetTexture("g_Background", m_pBackground);
		//控制内部黑色的程度
		m_SynthesisEffect->SetFloat("g_AlphaFactor", 0.6);
		m_SynthesisEffect->CommitChanges();


		m_SynthesisEffect->BeginPass(0);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_SynthesisEffect->EndPass();

		m_SynthesisEffect->End();
	}
	

	//====================================================================================================
	//重新渲染边缘纹理
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
		//粒子TEST
		
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pPostSurface);

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, true);

		D3DXMATRIX temp = RENDERDEVICE::Instance().ViewMatrix * RENDERDEVICE::Instance().ProjMatrix;
		m_postEffect->SetMatrix(VIEWPROJMATRIX, &temp);
		m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &temp);
		m_postEffect->SetMatrix(PROJECTIONMATRIX, &RENDERDEVICE::Instance().ProjMatrix);

		m_postEffect->SetTexture(MAINCOLORBUFFER, m_pEdgeBlur);
		m_postEffect->SetTexture(NORMALBUFFER, RENDERPIPE::Instance().m_pNormalTarget);
		m_postEffect->SetTexture("g_InkTex", m_pInkTex);
		m_postEffect->SetTexture("g_InkTex1", m_pInkTex1);
		m_postEffect->SetTexture("g_InkTex2", m_pInkTex2);
		m_postEffect->SetTexture("g_InkTex3", m_pInkTex3);
		m_postEffect->SetTexture("g_InkTex4", m_pInkTex4);


		baseTexSize = 1750;
		maxTexSize = 17;
		minTexSize = 5;
		m_postEffect->SetInt("g_baseTexSize", baseTexSize);
		m_postEffect->SetInt("g_minTexSize", minTexSize);
		m_postEffect->SetInt("g_maxTexSize", maxTexSize);

		//控制边缘黑色的程度
		m_postEffect->SetFloat("g_colorFactor", 0.3);

		m_postEffect->CommitChanges();

		m_postEffect->BeginPass(6);
		RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, mParticleBuffer, 0, sizeof(Particle));
		RENDERDEVICE::Instance().g_pD3DDevice->SetVertexDeclaration(mParticleDecl);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_POINTLIST, 0, w*h);
		m_postEffect->EndPass();

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, false);
		m_postEffect->End();

	}

	//m_pTexList[0] = m_pTexList[1];
	//m_pTexList[1] = m_pPostTarget;
	if (true)
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
		PDIRECT3DSURFACE9 pSurf_ReDraw = NULL;

		for (int i = 0; i < texCount-1; i++)
		{
			pSurf_ReDraw = NULL;
			m_pTexList[i]->GetSurfaceLevel(0, &pSurf_ReDraw);

			RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_ReDraw);
			RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

			m_SynthesisEffect->SetTexture("g_Src", m_pTexList[i+1]);

			m_SynthesisEffect->CommitChanges();

			m_SynthesisEffect->BeginPass(2);
			RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
			m_SynthesisEffect->EndPass();
		}//-------------------------------------------------------------------------------------------------------------------
		pSurf_ReDraw = NULL;
		m_pTexList[texCount-1]->GetSurfaceLevel(0, &pSurf_ReDraw);

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_ReDraw);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

		m_SynthesisEffect->SetTexture("g_Src", m_pPostTarget);

		m_SynthesisEffect->CommitChanges();

		m_SynthesisEffect->BeginPass(2);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_SynthesisEffect->EndPass();

		//-------------------------------------------------------------------------------------------------------------------
		PDIRECT3DSURFACE9 pSurf_ReDraw2 = NULL;
		m_pPostTarget->GetSurfaceLevel(0, &pSurf_ReDraw2);

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_ReDraw2);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

		m_SynthesisEffect->SetTexture("g_Src", m_pTexList[0]);
		m_SynthesisEffect->SetTexture("g_Src2", m_pTexList[1]);
		m_SynthesisEffect->SetTexture("g_Src3", m_pTexList[2]);
		m_SynthesisEffect->SetTexture("g_Src4", m_pTexList[3]);
		m_SynthesisEffect->SetTexture("g_Src5", m_pTexList[4]);

		m_SynthesisEffect->CommitChanges();

		m_SynthesisEffect->BeginPass(3);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_SynthesisEffect->EndPass();
		m_SynthesisEffect->End();
	}
}

void SumiE::SetEdgeImage(LPDIRECT3DTEXTURE9 edgeImage)
{
	m_pEdgeImage = edgeImage;
}
