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
VERTEX vertices[4]; //选择框;
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
		D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT,
		&m_pNormalDepthTarget, NULL);
	hr = m_pNormalDepthTarget->GetSurfaceLevel(0, &m_pNormalDepthSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT,
		&m_pPositionTarget, NULL);
	hr = m_pPositionTarget->GetSurfaceLevel(0, &m_pPositionSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT,
		&m_pAOTarget, NULL);
	hr = m_pAOTarget->GetSurfaceLevel(0, &m_pAOSurface);

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

	//初始化顶点缓冲区

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

	// 获得单位摄影矩阵
	D3DXMatrixIdentity(&orthoView);
	// 获得正交投影矩阵
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

void RenderPipe::RenderPosition()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pPositionSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0, 0), 1.0f, 0);

	for (int i = 0; i < mRenderUtilList.size(); ++i)
	{
		mRenderUtilList[i]->RenderPosition();
	}
}



float		g_intensity = 2;
float		g_scale = 1;
float		g_bias = 0.1;
float		g_sample_rad = 0.1;
bool		AOEnable = true;
bool		down = false;
void RenderPipe::RenderAO()
{
	if (KEYDOWN('P'))
	{
		down = true;
	}
	if (down == true && KEYUP('P'))
	{
		down = false;
		AOEnable = !AOEnable;
	}

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pAOSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 255, 255, 255), 1.0f, 0);

	if (!AOEnable)
		return;

	UINT numPasses = 0;
	ssaoEffect->Begin(&numPasses, 0);
	ssaoEffect->BeginPass(0);

	ssaoEffect->SetMatrix(WORLDVIEWPROJMATRIX, &orthoWorld);
	D3DXMATRIX InvProj;
	D3DXMatrixInverse(&InvProj, NULL, &RENDERDEVICE::Instance().ProjMatrix);
	ssaoEffect->SetMatrix(INVPROJMATRIX, &InvProj);

	ssaoEffect->SetTexture("g_NormalDepthBuffer", m_pNormalDepthTarget);
	ssaoEffect->SetTexture("g_RandomNormal", m_pRandomNormal);
	ssaoEffect->SetTexture("g_PositionBuffer", m_pPositionTarget);

	if (KEYDOWN('T'))
	{
		g_intensity += 0.0001;
	}
	if (KEYDOWN('Y'))
	{
		g_scale += 0.0001;
	}
	if (KEYDOWN('U'))
	{
		g_sample_rad += 0.0001;
	}
	if (KEYDOWN('I'))
	{
		g_bias += 0.0001;
	}

	if (KEYDOWN('G'))
	{
		g_intensity -= 0.0001;
	}
	if (KEYDOWN('H'))
	{
		g_scale -= 0.0001;
	}
	if (KEYDOWN('J'))
	{
		g_sample_rad -= 0.0001;
	}
	if (KEYDOWN('K'))
	{
		g_bias -= 0.0001;
	}

	if (KEYDOWN('R'))
	{
		g_intensity = 1;
		g_scale = 1;
		g_bias = 0.3;
		g_sample_rad = 0.03;
	}
	ssaoEffect->SetFloat("g_intensity", g_intensity);
	ssaoEffect->SetFloat("g_scale", g_scale);
	ssaoEffect->SetFloat("g_bias", g_bias);
	ssaoEffect->SetFloat("g_sample_rad", g_sample_rad);

	ssaoEffect->SetFloat("g_zNear", CameraParam::zNear);
	ssaoEffect->SetFloat("g_zFar", CameraParam::zFar);

	ssaoEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	ssaoEffect->SetTexture(0, NULL);

	ssaoEffect->EndPass();
	ssaoEffect->End();
}

void RenderPipe::DeferredRender()
{
	UINT numPasses = 0;
	deferredEffect->Begin(&numPasses, 0);
	deferredEffect->BeginPass(0);

	deferredEffect->SetMatrix(WORLDVIEWPROJMATRIX, &orthoWorld);

	deferredEffect->SetTexture("g_DiffuseBuffer", m_pDiffuseTarget);
	deferredEffect->SetTexture("g_NormalDepthBuffer", m_pNormalDepthTarget);

	deferredEffect->SetTexture("g_AOBuffer", m_pAOTarget);

	deferredEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	deferredEffect->SetTexture(0, NULL);

	deferredEffect->EndPass();
	deferredEffect->End();
}



void RenderPipe::RenderAll()
{
	RENDERDEVICE::Instance().g_pD3DDevice->BeginScene();
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	UpdateRenderState();

	RENDERDEVICE::Instance().g_pD3DDevice->GetRenderTarget(0, &m_pOriSurface);

	RenderDiffuse();
	
	RenderNormalDepth();

	RenderPosition();

	//ForwardRender();

	RenderAO();

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pOriSurface);

	DeferredRender();
	
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
