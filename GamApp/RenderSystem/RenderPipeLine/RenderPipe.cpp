#include "RenderPipe.h"
#include "D3D9Device.h"
#include "CommonUtil/Input/Input.h"

#include "RenderUtil/EffectParam.h"
#include "Camera/CameraParam.h"

#include "RenderPipeLine/PostEffect/SSAO.h"

struct VERTEX
{
	D3DXVECTOR3		position;
	FLOAT			tu, tv;
};

#define D3DFVF_VERTEX (D3DFVF_XYZ|D3DFVF_TEX1)
VERTEX vertices[4];
LPDIRECT3DVERTEXBUFFER9		pBufferVex;

LPD3DXEFFECT deferredEffect; 
LPD3DXEFFECT deferredMultiPassEffect;
LPD3DXEFFECT mainColorEffect;

D3DXMATRIX shadowOrthoWorld;
D3DXMATRIX shadowOrthoView;
D3DXMATRIX shadowOrthoProj;
D3DXMATRIX invShadowOrthoProj;
int SHADOWMAPSIZE = 756;

SSAO ssao;

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
		D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT,
		&m_pMainColorTarget, NULL);
	hr = m_pMainColorTarget->GetSurfaceLevel(0, &m_pMainColorSurface);
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

	//=============================================================================
	//ShadowMap
	D3DXMatrixLookAtLH(&shadowOrthoView, &D3DXVECTOR3(0, 10, -10),
		&D3DXVECTOR3(0, -1, 1),
		&D3DXVECTOR3(0, 1, 1));
	D3DXMatrixInverse(&shadowOrthoWorld, NULL, &shadowOrthoView);
	D3DXMatrixOrthoLH(&shadowOrthoProj, 20, 20, 0.01f, 50.0f);
	D3DXMatrixInverse(&invShadowOrthoProj, NULL, &shadowOrthoProj);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(SHADOWMAPSIZE, SHADOWMAPSIZE,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT,
		&m_pShadowTarget, NULL);
	hr = m_pShadowTarget->GetSurfaceLevel(0, &m_pShadowSurface);
	//=================================================
	ID3DXBuffer* error = 0;
	if (E_FAIL == ::D3DXCreateEffectFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "System\\DeferredRender.fx", NULL, NULL, D3DXSHADER_DEBUG,
		NULL, &deferredEffect, &error))
	{
		MessageBox(GetForegroundWindow(), (char*)error->GetBufferPointer(), "Shader", MB_OK);
		abort();
	}

	error = 0;
	if (E_FAIL == ::D3DXCreateEffectFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "System\\DeferredRender_MultiPass.fx", NULL, NULL, D3DXSHADER_DEBUG,
		NULL, &deferredMultiPassEffect, &error))
	{
		MessageBox(GetForegroundWindow(), (char*)error->GetBufferPointer(), "Shader", MB_OK);
		abort();
	}

	error = 0;
	if (E_FAIL == ::D3DXCreateEffectFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "System\\MainColor.fx", NULL, NULL, D3DXSHADER_DEBUG,
		NULL, &mainColorEffect, &error))
	{
		MessageBox(GetForegroundWindow(), (char*)error->GetBufferPointer(), "Shader", MB_OK);
		abort();
	}
	//=================================================


	ssao.CreatePostEffect();
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

void RenderPipe::RenderShadow()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pShadowSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 255, 255, 255), 1.0f, 0);
	for (int i = 0; i < mRenderUtilList.size(); ++i)
	{
		mRenderUtilList[i]->RenderShadow();
	}
}

void RenderPipe::DeferredRender()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pMainColorSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0, 0), 1.0f, 0);

	UINT numPasses = 0;
	deferredEffect->Begin(&numPasses, 0);
	deferredEffect->BeginPass(0);

	deferredEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);
	deferredEffect->SetMatrix(INVPROJMATRIX, &RENDERDEVICE::Instance().InvProjMatrix);

	deferredEffect->SetTexture(DIFFUSEBUFFER, m_pDiffuseTarget);
	deferredEffect->SetTexture(NORMALDEPTHBUFFER, m_pNormalDepthTarget);
	deferredEffect->SetTexture("g_AOBuffer", ssao.GetPostTarget());
	deferredEffect->SetTexture("g_ShadowBuffer", m_pShadowTarget);

	D3DXMATRIX ShadowView;
	D3DXMATRIX invShadowProj;
	D3DXMATRIX invView;
	D3DXMatrixInverse(&invView, NULL, &RENDERDEVICE::Instance().ViewMatrix);
	deferredEffect->SetMatrix("g_ShadowView", &shadowOrthoView);
	deferredEffect->SetMatrix("g_ShadowProj", &shadowOrthoProj);
	deferredEffect->SetMatrix("g_invView", &invView);


	deferredEffect->SetFloat("g_zNear", CameraParam::zNear);
	deferredEffect->SetFloat("g_zFar", CameraParam::zFar);

	deferredEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	deferredEffect->SetTexture(0, NULL);

	deferredEffect->EndPass();
	deferredEffect->End();
}

D3DXVECTOR3 lightDirArrayWorld[16] = 
{ 
D3DXVECTOR3(0, 0, 1), D3DXVECTOR3(0, 0, -1), D3DXVECTOR3(0, -1, 0), D3DXVECTOR3(0, 0, 0),
D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 0),
D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 0),
D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 0) 
};

D3DXVECTOR4 lightColorArray[16] =
{ D3DXVECTOR4(0.3, 0.0, 0.0f, 1), D3DXVECTOR4(0.0, 0, 0.3, 0), D3DXVECTOR4(0, 0.3, 0.0, 0), D3DXVECTOR4(0.0, 0.0, 0, 0),
D3DXVECTOR4(0, 0, 0, 0), D3DXVECTOR4(0, 0, 0, 0), D3DXVECTOR4(0, 0, 0, 0), D3DXVECTOR4(0, 0, 0, 0),
D3DXVECTOR4(0, 0, 0, 0), D3DXVECTOR4(0, 0, 0, 0), D3DXVECTOR4(0, 0, 0, 0), D3DXVECTOR4(0, 0, 0, 0),
D3DXVECTOR4(0, 0, 0, 0), D3DXVECTOR4(0, 0, 0, 0), D3DXVECTOR4(0, 0, 0, 0), D3DXVECTOR4(0, 0, 0, 0)};

D3DXVECTOR4	AmbientColor = D3DXVECTOR4(0.2f, 0.2f, 0.2f, 0);

void RenderPipe::DeferredRender_MultiPass()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pMainColorSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0, 0), 1.0f, 0);

	deferredMultiPassEffect->SetMatrix(VIEWMATRIX, &RENDERDEVICE::Instance().ViewMatrix);
	deferredMultiPassEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);
	deferredMultiPassEffect->SetMatrix(INVPROJMATRIX, &RENDERDEVICE::Instance().InvProjMatrix);

	deferredMultiPassEffect->SetTexture(DIFFUSEBUFFER, m_pDiffuseTarget);
	deferredMultiPassEffect->SetTexture(NORMALDEPTHBUFFER, m_pNormalDepthTarget);
	deferredMultiPassEffect->SetTexture("g_AOBuffer", ssao.GetPostTarget());
	deferredMultiPassEffect->SetTexture("g_ShadowBuffer", m_pShadowTarget);

#if USE_POSITIONBUFFER
	deferredEffect->SetTexture("g_PositionBuffer", m_pPositionTarget);
#endif
	
	

	deferredMultiPassEffect->SetFloat("g_zNear", CameraParam::zNear);
	deferredMultiPassEffect->SetFloat("g_zFar", CameraParam::zFar);

	deferredMultiPassEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);

	UINT numPasses = 0;
	deferredMultiPassEffect->Begin(&numPasses, 0);

	

	D3DXVECTOR4 lightDirArrayView[16];
	for (int i = 0; i < 16; i++)
	{
		D3DXVECTOR3 viewDir = lightDirArrayWorld[i];
		D3DXVECTOR3 temp;
		D3DXVec3TransformNormal(&temp, &viewDir, &RENDERDEVICE::Instance().ViewMatrix);
		lightDirArrayView[i] = D3DXVECTOR4(temp,1.0f);
	}

	int lightCount = 1;
	for (int i = 0; i < lightCount; i++)
	{

		deferredMultiPassEffect->BeginPass(0);

		deferredMultiPassEffect->SetVectorArray("g_LightDirArray", lightDirArrayView, 16);
		deferredMultiPassEffect->SetVectorArray("g_LightColorArray", lightColorArray, 16);
		deferredMultiPassEffect->CommitChanges();

		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

		deferredMultiPassEffect->EndPass();
	}

	//阴影Pass
	deferredMultiPassEffect->BeginPass(1);
	D3DXMATRIX ShadowView;
	D3DXMATRIX invShadowProj;
	D3DXMATRIX invView;
	D3DXMatrixInverse(&invView, NULL, &RENDERDEVICE::Instance().ViewMatrix);
	deferredMultiPassEffect->SetMatrix("g_ShadowView", &shadowOrthoView);
	deferredMultiPassEffect->SetMatrix("g_ShadowProj", &shadowOrthoProj);
	deferredMultiPassEffect->SetMatrix("g_invView", &invView);
	deferredMultiPassEffect->SetInt("g_ShadowMapSize", SHADOWMAPSIZE);
	deferredMultiPassEffect->SetFloat("g_ShadowBias", 0.2f);

	deferredMultiPassEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	deferredMultiPassEffect->EndPass();

	//环境光Pass
	deferredMultiPassEffect->BeginPass(2);

	deferredMultiPassEffect->SetVector("g_AmbientColor", &AmbientColor);
	deferredMultiPassEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	deferredMultiPassEffect->EndPass();
	
	//纹理及AO的Pass
	deferredMultiPassEffect->BeginPass(3);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	deferredMultiPassEffect->EndPass();

	//DebugPass
	deferredMultiPassEffect->BeginPass(4);
	//RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	deferredMultiPassEffect->EndPass();

	deferredMultiPassEffect->End();

	deferredMultiPassEffect->SetTexture(0, NULL);
}


void RenderPipe::RenderMainColor()
{
	UINT numPasses = 0;
	mainColorEffect->Begin(&numPasses, 0);
	mainColorEffect->BeginPass(0);

	mainColorEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);
	mainColorEffect->SetMatrix(INVPROJMATRIX, &RENDERDEVICE::Instance().InvProjMatrix);

	mainColorEffect->SetTexture(MAINCOLORBUFFER, m_pMainColorTarget);

	mainColorEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	mainColorEffect->SetTexture(0, NULL);

	mainColorEffect->EndPass();
	mainColorEffect->End();
}

void RenderPipe::RenderAll()
{
	RENDERDEVICE::Instance().g_pD3DDevice->BeginScene();
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	UpdateRenderState();

	RENDERDEVICE::Instance().g_pD3DDevice->GetRenderTarget(0, &m_pOriSurface);

	RenderShadow();

	RenderDiffuse();
	
	RenderNormalDepth();

#if USE_POSITIONBUFFER
	//位置缓冲可以通过深度重建，但是如果需要较高精度的时候，可以考虑渲染位置图
	RenderPosition();
#endif	
	

	//ForwardRender();

	ssao.RenderPost();
	DeferredRender_MultiPass();
	//DeferredRender();

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pOriSurface);

	RenderMainColor();
	
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
