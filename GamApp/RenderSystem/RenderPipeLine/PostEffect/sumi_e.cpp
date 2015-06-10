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
int baseTexSize = 32;
int maxTexSize = 22;

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

	baseTexSize = 32;
	maxTexSize = 22;
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



	PostEffectBase::CreatePostEffect("System\\Sumi_e.fx");

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pEdgeBlur, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_Garyscale, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_BlurredGaryscale, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_Diffusion1, NULL);
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_Diffusion2, NULL);

	//create renderTarget
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_StrokesArea_1, NULL);
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_StrokesArea_2, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_StrokesArea_3, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_StrokesArea_4, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_StrokesArea_5, NULL);

	//==========================================================
	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "Res\\testInkImage.png", &m_pTestInk))
	{
		MessageBox(GetForegroundWindow(), "TextureError", "testInkImage", MB_OK);
		abort();
	}
	//==========================================================
	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "Res\\3.bmp", &m_pInkTex))//3.bmp//R_Test.png
	{
		MessageBox(GetForegroundWindow(), "TextureError", "InkTex", MB_OK);
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
	//StrokesArea1
	PDIRECT3DSURFACE9 pSurf_SA1 = NULL;
	m_StrokesArea_1->GetSurfaceLevel(0, &pSurf_SA1);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_SA1);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture("g_GrayscaleBuffer", m_BlurredGaryscale);//m_BlurredGaryscale    m_pTestInk

	m_postEffect->SetInt("minI", 0);
	m_postEffect->SetInt("maxI", 70);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(1);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	int d = 0;
	m_Diffusion2 = m_StrokesArea_1;
	for (int i = 0; i < d; i++)
	{
		//diffusion1
		PDIRECT3DSURFACE9 pSurf_diffusion1 = NULL;
		m_Diffusion1->GetSurfaceLevel(0, &pSurf_diffusion1);

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_diffusion1);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

		m_postEffect->SetTexture("g_DiffusionSource", m_Diffusion2);

		m_postEffect->CommitChanges();

		m_postEffect->BeginPass(4);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_postEffect->EndPass();
		//diffusion2
		PDIRECT3DSURFACE9 pSurf_diffusion2 = NULL;
		m_Diffusion2->GetSurfaceLevel(0, &pSurf_diffusion2);

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_diffusion2);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

		m_postEffect->SetTexture("g_DiffusionSource", m_Diffusion1);

		m_postEffect->CommitChanges();

		m_postEffect->BeginPass(4);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_postEffect->EndPass();
	}
	m_StrokesArea_1 = m_Diffusion2;
	//=============================================================================================================
	//StrokesArea2
	PDIRECT3DSURFACE9 pSurf_SA2 = NULL;
	m_StrokesArea_2->GetSurfaceLevel(0, &pSurf_SA2);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_SA2);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture("g_GrayscaleBuffer", m_BlurredGaryscale);
	m_postEffect->SetInt("minI", 70);
	m_postEffect->SetInt("maxI", 120);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(1);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	SafeRelease(pSurf_SA2);
	//=============================================================================================================
	//StrokesArea3
	PDIRECT3DSURFACE9 pSurf_SA3 = NULL;
	m_StrokesArea_3->GetSurfaceLevel(0, &pSurf_SA3);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_SA3);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture("g_GrayscaleBuffer", m_BlurredGaryscale);
	m_postEffect->SetInt("minI", 120);
	m_postEffect->SetInt("maxI", 160);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(1);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	SafeRelease(pSurf_SA3);

	//=============================================================================================================
	//StrokesArea4
	PDIRECT3DSURFACE9 pSurf_SA4 = NULL;
	m_StrokesArea_4->GetSurfaceLevel(0, &pSurf_SA4);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_SA4);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture("g_GrayscaleBuffer", m_BlurredGaryscale);
	m_postEffect->SetInt("minI", 160);
	m_postEffect->SetInt("maxI", 210);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(1);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	SafeRelease(pSurf_SA4);
	//=============================================================================================================
	//StrokesArea5
	PDIRECT3DSURFACE9 pSurf_SA5 = NULL;
	m_StrokesArea_5->GetSurfaceLevel(0, &pSurf_SA5);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_SA5);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture("g_GrayscaleBuffer", m_BlurredGaryscale);
	m_postEffect->SetInt("minI", 210);
	m_postEffect->SetInt("maxI", 255);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(1);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	SafeRelease(pSurf_SA5);

	//=============================================================================================================
	//模糊轮廓图
	PDIRECT3DSURFACE9 pSurf_EdgeBlur = NULL;
	m_pEdgeBlur->GetSurfaceLevel(0, &pSurf_EdgeBlur);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf_EdgeBlur);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture(POSITIONBUFFER, RENDERPIPE::Instance().m_pPositionTarget);
	m_postEffect->SetTexture(MAINCOLORBUFFER, mainBuffer);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(5);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	//=============================================================================================================
	//合并
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pPostSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	m_postEffect->SetTexture(MAINCOLORBUFFER, m_pEdgeBlur); //m_pEdgeImage//m_pEdgeBlur//mainBuffer//RENDERPIPE::Instance().m_pNormalTarget

	m_postEffect->SetTexture("SA1", m_StrokesArea_1);
	m_postEffect->SetTexture("SA2", m_StrokesArea_2);
	m_postEffect->SetTexture("SA3", m_StrokesArea_3);
	m_postEffect->SetTexture("SA4", m_StrokesArea_4);
	m_postEffect->SetTexture("SA5", m_StrokesArea_5);

	m_postEffect->CommitChanges();

	m_postEffect->BeginPass(2);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->EndPass();

	bool openParticle = true;
	if (openParticle)
	{
		//=============================================================================================================
		//粒子TEST

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, true);

		D3DXMATRIX temp = RENDERDEVICE::Instance().ViewMatrix * RENDERDEVICE::Instance().ProjMatrix;
		m_postEffect->SetMatrix(VIEWPROJMATRIX, &temp);
		m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &temp);
		m_postEffect->SetMatrix(PROJECTIONMATRIX, &RENDERDEVICE::Instance().ProjMatrix);

		m_postEffect->SetTexture(MAINCOLORBUFFER, m_pEdgeBlur);
		m_postEffect->SetTexture(NORMALBUFFER, RENDERPIPE::Instance().m_pNormalTarget);
		m_postEffect->SetTexture("g_InkTex", m_pInkTex);

		m_postEffect->SetInt("g_baseTexSize", baseTexSize);
		m_postEffect->SetInt("g_maxTexSize", maxTexSize);

		m_postEffect->CommitChanges();

		m_postEffect->BeginPass(6);
		RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, mParticleBuffer, 0, sizeof(Particle));
		RENDERDEVICE::Instance().g_pD3DDevice->SetVertexDeclaration(mParticleDecl);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_POINTLIST, 0, w*h);
		m_postEffect->EndPass();

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, false);
	}
	
	

	m_postEffect->End();
}

void SumiE::SetEdgeImage(LPDIRECT3DTEXTURE9 edgeImage)
{
	m_pEdgeImage = edgeImage;
}
