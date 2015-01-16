#include "PostEffectBase.h"
#include "D3D9Device.h"
#include "RenderUtil/EffectParam.h"
#include "RenderSystem/RenderPipeLine/RenderPipe.h"

PostEffectBase::PostEffectBase()
{
}


PostEffectBase::~PostEffectBase()
{
}

void PostEffectBase::CreatePostEffect(std::string effectName, D3DFORMAT postTargetFMT/* = D3DFMT_X8R8G8B8*/)
{
	//create renderTarget
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		postTargetFMT, D3DPOOL_DEFAULT,
		&m_pPostTarget, NULL);
	HRESULT hr = m_pPostTarget->GetSurfaceLevel(0, &m_pPostSurface);

	//Create post effect
	ID3DXBuffer* error = 0;
	if (E_FAIL == ::D3DXCreateEffectFromFile(RENDERDEVICE::Instance().g_pD3DDevice, effectName.c_str(), NULL, NULL, D3DXSHADER_DEBUG,
		NULL, &m_postEffect, &error))
	{
		MessageBox(GetForegroundWindow(), (char*)error->GetBufferPointer(), "Shader", MB_OK);
		abort();
	}

	//Create post vertex
	RENDERDEVICE::Instance().g_pD3DDevice->CreateVertexBuffer(4 * sizeof(VERTEX)
		, 0
		, D3DFVF_VERTEX
		//,D3DPOOL_DEFAULT
		, D3DPOOL_MANAGED
		, &m_pBufferVex
		, NULL);

	VERTEX* pVertices1;
	m_pBufferVex->Lock(0, 4 * sizeof(VERTEX), (void**)&pVertices1, 0);

	//初始化顶点缓冲区

	//==========================
	pVertices1->position = D3DXVECTOR3(1.0f, -1.0f, 0.0f);
	pVertices1->tu = 1.0f + 0.5f / RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth;
	pVertices1->tv = 1.0f + 0.5f / RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight;
	pVertices1++;

	pVertices1->position = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);
	pVertices1->tu = 0.0f + 0.5f / RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth;
	pVertices1->tv = 1.0f + 0.5f / RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight;
	pVertices1++;



	pVertices1->position = D3DXVECTOR3(1.0f, 1.0f, 0.0f);
	pVertices1->tu = 1.0f + 0.5f / RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth;
	pVertices1->tv = 0.0f + 0.5f / RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight;
	pVertices1++;

	pVertices1->position = D3DXVECTOR3(-1.0f, 1.0f, 0.0f);
	pVertices1->tu = 0.0f + 0.5f / RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth;
	pVertices1->tv = 0.0f + 0.5f / RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight;
	pVertices1++;

	m_pBufferVex->Unlock();
}

void PostEffectBase::RenderPost(LPDIRECT3DTEXTURE9 lastBuffer/* = NULL*/)
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pPostSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	UINT numPasses = 0;
	m_postEffect->Begin(&numPasses, 0);
	m_postEffect->BeginPass(0);

	m_postEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);
	if (lastBuffer != NULL)
		m_postEffect->SetTexture(MAINCOLORBUFFER, lastBuffer);
	else
		m_postEffect->SetTexture(MAINCOLORBUFFER, RENDERPIPE::Instance().m_pMainColorTarget);

	m_postEffect->SetInt(SCREENWIDTH, RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth);
	m_postEffect->SetInt(SCREENHEIGHT, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight);

	m_postEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_postEffect->SetTexture(0, NULL);

	m_postEffect->EndPass();
	m_postEffect->End();
}
