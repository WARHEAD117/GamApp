#include "RenderPipe.h"
#include "D3D9Device.h"
#include "CommonUtil/Input/Input.h"

#include "RenderUtil/EffectParam.h"
#include "Camera/CameraParam.h"

struct VERTEX
{
	D3DXVECTOR3		position;
	FLOAT			tu, tv;
};
#define D3DFVF_VERTEX (D3DFVF_XYZ|D3DFVF_TEX1)
VERTEX vertices[4]; //ѡ���;
LPDIRECT3DVERTEXBUFFER9		pBufferVex;
D3DXMATRIX orthoWorld;
D3DXMATRIX orthoView;
D3DXMATRIX orthoProj;

LPD3DXEFFECT deferredEffect; 
LPD3DXEFFECT ssaoEffect;
LPDIRECT3DTEXTURE9         m_pRandomNormal;

RenderPipe::RenderPipe()
{
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A16B16G16R16, D3DPOOL_DEFAULT,
		&m_pDiffuseTarget, NULL);
	HRESULT hr = m_pDiffuseTarget->GetSurfaceLevel(0, &m_pDIffuseSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A16B16G16R16, D3DPOOL_DEFAULT,
		&m_pNormalDepthTarget, NULL);
	hr = m_pNormalDepthTarget->GetSurfaceLevel(0, &m_pNormalDepthSurface);

	//=======================================================================
	RENDERDEVICE::Instance().g_pD3DDevice->CreateVertexBuffer(4 * sizeof(VERTEX)
		, 0
		, D3DFVF_VERTEX
		//,D3DPOOL_DEFAULT
		, D3DPOOL_MANAGED
		, &pBufferVex
		, NULL);

	VERTEX* pVertices1;
	pBufferVex->Lock(0, 4 * sizeof(VERTEX), (void**)&pVertices1, 0);

	//��ʼ�����㻺����

	//==========================
	pVertices1->position = D3DXVECTOR3(1.0f, -1.0f, 0.0f);
	pVertices1->tu = 1.0f +0.5f / RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth;
	pVertices1->tv = 1.0f +0.5f / RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight;
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

	pBufferVex->Unlock();

	// ��õ�λ��Ӱ����
	D3DXMatrixIdentity(&orthoView);
	// �������ͶӰ����
	D3DXMatrixOrthoOffCenterLH(&orthoProj, 0.0f, RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight, 0.0f, 0.0f, 1.0f);
	D3DXMatrixOrthoLH(&orthoProj, RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight, 0.01f, 10000.0f);
	//
	D3DXMatrixIdentity(&orthoWorld);
	D3DXMatrixTranslation(&orthoWorld, 0, 0, 0);

	//=================================================
	ID3DXBuffer* error = 0;
	if (E_FAIL == ::D3DXCreateEffectFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "System\\DeferredRender.fx", NULL, NULL, D3DXSHADER_DEBUG,
		NULL, &deferredEffect, &error))
	{
		MessageBox(GetForegroundWindow(), (char*)error->GetBufferPointer(), "Shader", MB_OK);
		abort();
	}

	//=================================================
 	if (E_FAIL == ::D3DXCreateEffectFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "System\\SSAONew.fx", NULL, NULL, D3DXSHADER_DEBUG,
 		NULL, &ssaoEffect, &error))
 	{
 		MessageBox(GetForegroundWindow(), (char*)error->GetBufferPointer(), "Shader", MB_OK);
 		abort();
 	}

	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "System\\randomNormal.dds", &m_pRandomNormal))
	{
		MessageBox(GetForegroundWindow(), "","234", MB_OK);
		abort();
	}
}




RenderPipe::~RenderPipe()
{
	ClearRenderUtil();
}

void RenderPipe::RenderDiffuse()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pDIffuseSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0, 0), 1.0f, 0);

	for (int i = 0; i < mRenderUtilList.size(); ++i)
	{
		mRenderUtilList[i]->RenderDiffuse();
	}
}

void RenderPipe::RenderNormalDepth()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pNormalDepthSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0, 0), 1.0f, 0);

	for (int i = 0; i < mRenderUtilList.size(); ++i)
	{
		mRenderUtilList[i]->RenderNormalDepth();
	}
}

void RenderPipe::RenderPost()
{
	UINT numPasses = 0;
	ssaoEffect->Begin(&numPasses, 0);
	ssaoEffect->BeginPass(0);

	ssaoEffect->SetMatrix(WORLDVIEWPROJMATRIX, &orthoWorld);
	D3DXMATRIX InvProj;
	D3DXMatrixInverse(&InvProj, NULL, &RENDERDEVICE::Instance().ProjMatrix);
	ssaoEffect->SetMatrix(INVPROJMATRIX, &InvProj);

	ssaoEffect->SetTexture("g_NormalDepthBuffer", m_pNormalDepthTarget);
	ssaoEffect->SetTexture("g_RandomNormal", m_pRandomNormal);

	RENDERDEVICE::Instance().GetDiffuseEffect()->SetFloat("g_zNear", CameraParam::zNear);
	RENDERDEVICE::Instance().GetDiffuseEffect()->SetFloat("g_zFar", CameraParam::zFar);

	ssaoEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	ssaoEffect->SetTexture(0, NULL);

	ssaoEffect->EndPass();
	ssaoEffect->End();
}

void RenderPipe::RenderAll()
{
	RENDERDEVICE::Instance().g_pD3DDevice->BeginScene();
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	UpdateRenderState();

	RENDERDEVICE::Instance().g_pD3DDevice->GetRenderTarget(0, &m_pOriSurface);

	RenderDiffuse();
	
	RenderNormalDepth();

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pOriSurface);

	//ForwardRender();

	DeferredRender();

	RenderPost();

	RENDERDEVICE::Instance().g_pD3DDevice->EndScene();
	RENDERDEVICE::Instance().g_pD3DDevice->Present(0, 0, 0, 0);
}

void RenderPipe::ForwardRender()
{
	for (int i = 0; i < mRenderUtilList.size(); ++i)
	{
		mRenderUtilList[i]->Render();
	}
}

void RenderPipe::DeferredRender()
{
	UINT numPasses = 0;
	deferredEffect->Begin(&numPasses, 0);
	deferredEffect->BeginPass(0);

	deferredEffect->SetMatrix(WORLDVIEWPROJMATRIX, &orthoWorld);

	deferredEffect->SetTexture("g_DiffuseBuffer", m_pDiffuseTarget);
	deferredEffect->SetTexture("g_NormalDepthBuffer", m_pNormalDepthTarget);

	deferredEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	deferredEffect->SetTexture(0, NULL);

	deferredEffect->EndPass();
	deferredEffect->End();
}

void RenderPipe::PushRenderUtil(RenderUtil* const renderUtil)
{
	mRenderUtilList.push_back(renderUtil);
}

void RenderPipe::ClearRenderUtil()
{
	mRenderUtilList.clear();
}

void RenderPipe::UpdateRenderState()
{
	if (KEYDOWN(VK_F1))
	{
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);
	}
	if (KEYDOWN(VK_F2))
	{
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	}
	if (KEYDOWN(VK_F3))
	{
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	}
	if (KEYDOWN(VK_F4))
	{
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	}
	if (KEYDOWN(VK_F5))
	{
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
	}
	if (KEYDOWN(VK_F6))
	{
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_PHONG);
	}
}
