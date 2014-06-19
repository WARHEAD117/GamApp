#include "RenderPipe.h"
#include "D3D9Device.h"
#include "CommonUtil/Input/Input.h"

#include "RenderUtil/EffectParam.h"
#include "Camera/CameraParam.h"

#include "Light/LightManager.h"
#include "Light/DirectionLight.h"

#include "RenderPipeLine/PostEffect/SSAO.h"

struct VERTEX
{
	D3DXVECTOR3		position;
	FLOAT			tu, tv;
};

#define D3DFVF_VERTEX (D3DFVF_XYZ|D3DFVF_TEX1)
VERTEX vertices[4];
LPDIRECT3DVERTEXBUFFER9		pBufferVex;

LPD3DXEFFECT GBufferEffect;

LPD3DXEFFECT deferredEffect; 
LPD3DXEFFECT deferredMultiPassEffect;
LPD3DXEFFECT mainColorEffect;

D3DXMATRIX shadowOrthoWorld;
D3DXMATRIX shadowOrthoView;
D3DXMATRIX shadowOrthoProj;
D3DXMATRIX invShadowOrthoProj;
int SHADOWMAPSIZE = 1024;

SSAO ssao;

RenderPipe::RenderPipe()
{

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT,
		&m_pDiffuseTarget, NULL);
	HRESULT hr = m_pDiffuseTarget->GetSurfaceLevel(0, &m_pDiffuseSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT,
		&m_pNormalDepthTarget, NULL);
	hr = m_pNormalDepthTarget->GetSurfaceLevel(0, &m_pNormalDepthSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT,
		&m_pPositionTarget, NULL);
	hr = m_pPositionTarget->GetSurfaceLevel(0, &m_pPositionSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT,
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

	RENDERDEVICE::Instance().g_pD3DDevice->CreateDepthStencilSurface(SHADOWMAPSIZE, SHADOWMAPSIZE, 
		D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, TRUE,
		&m_pDepthStencilShadowSurface, NULL);
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
	if (E_FAIL == ::D3DXCreateEffectFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "System\\DeferredGBuffer.fx", NULL, NULL, D3DXSHADER_DEBUG,
		NULL, &GBufferEffect, &error))
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

void RenderPipe::RenderGBuffer()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pDiffuseSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(1, m_pNormalDepthSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(2, m_pPositionSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0, 0), 1.0f, 0);

	UINT nPasses = 0;
	HRESULT r1 = GBufferEffect->Begin(&nPasses, 0);
	HRESULT r2 = GBufferEffect->BeginPass(0);

	for (int i = 0; i < mRenderUtilList.size(); ++i)
	{
		mRenderUtilList[i]->RenderDeferredGeometry(GBufferEffect);
	}

	GBufferEffect->EndPass();
	GBufferEffect->End();
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, NULL);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(1, NULL);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(2, NULL);
}

void RenderPipe::RenderDiffuse()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pDiffuseSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0, 0), 1.0f, 0);

	UINT nPasses = 0;
	HRESULT r1 = RENDERDEVICE::Instance().GetDiffuseEffect()->Begin(&nPasses, 0);
	HRESULT r2 = RENDERDEVICE::Instance().GetDiffuseEffect()->BeginPass(0);

	for (int i = 0; i < mRenderUtilList.size(); ++i)
	{
		mRenderUtilList[i]->RenderDeferredGeometry(RENDERDEVICE::Instance().GetDiffuseEffect());
	}

	RENDERDEVICE::Instance().GetDiffuseEffect()->EndPass();
	RENDERDEVICE::Instance().GetDiffuseEffect()->End();
}

void RenderPipe::RenderNormalDepth()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pNormalDepthSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0, 0), 1.0f, 0);

	UINT nPasses = 0;
	HRESULT r1 = RENDERDEVICE::Instance().GetNormalDepthEffect()->Begin(&nPasses, 0);
	HRESULT r2 = RENDERDEVICE::Instance().GetNormalDepthEffect()->BeginPass(0);

	for (int i = 0; i < mRenderUtilList.size(); ++i)
	{
		//mRenderUtilList[i]->RenderNormalDepth();
		mRenderUtilList[i]->RenderDeferredGeometry(RENDERDEVICE::Instance().GetNormalDepthEffect());
	}

	RENDERDEVICE::Instance().GetNormalDepthEffect()->EndPass();
	RENDERDEVICE::Instance().GetNormalDepthEffect()->End();
}

void RenderPipe::RenderPosition()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pPositionSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0, 0), 1.0f, 0);

	UINT nPasses = 0;
	HRESULT r1 = RENDERDEVICE::Instance().GetPositionEffect()->Begin(&nPasses, 0);
	HRESULT r2 = RENDERDEVICE::Instance().GetPositionEffect()->BeginPass(0);

	for (int i = 0; i < mRenderUtilList.size(); ++i)
	{
		//mRenderUtilList[i]->RenderPosition();
		mRenderUtilList[i]->RenderDeferredGeometry(RENDERDEVICE::Instance().GetPositionEffect());
	}

	RENDERDEVICE::Instance().GetPositionEffect()->EndPass();
	RENDERDEVICE::Instance().GetPositionEffect()->End();
}

void RenderPipe::RenderShadow()
{
	LPDIRECT3DSURFACE9 pOldDS = NULL;
	RENDERDEVICE::Instance().g_pD3DDevice->GetDepthStencilSurface(&pOldDS);

	int lightCount = LIGHTMANAGER::Instance().GetLightCount();
	for (int index = 0; index < lightCount; index++)
	{
		BaseLight* pLight = LIGHTMANAGER::Instance().GetLight(index);
		bool useShadow = pLight->GetUseShadow();
		if (!useShadow)
			continue;
		else
		{
			pLight->SetShadowTarget();

			for (int i = 0; i < mRenderUtilList.size(); ++i)
			{
				mRenderUtilList[i]->RenderShadow(index);
			}
		}
	}

	if (NULL != pOldDS)
	{
		RENDERDEVICE::Instance().g_pD3DDevice->SetDepthStencilSurface(pOldDS);
		SafeRelease(pOldDS);
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

D3DXVECTOR4	AmbientColor = D3DXVECTOR4(0.2f, 0.2f, 0.2f, 0);

float g_minVariance = 0.2f;
float g_Amount = 0.8f;

void RenderPipe::DeferredRender_MultiPass()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pMainColorSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0, 0), 1.0f, 0);

	deferredMultiPassEffect->SetMatrix(VIEWMATRIX, &RENDERDEVICE::Instance().ViewMatrix);
	deferredMultiPassEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);
	deferredMultiPassEffect->SetMatrix(INVPROJMATRIX, &RENDERDEVICE::Instance().InvProjMatrix);

	deferredMultiPassEffect->SetTexture(DIFFUSEBUFFER, m_pDiffuseTarget);
	deferredMultiPassEffect->SetTexture(NORMALDEPTHBUFFER, m_pNormalDepthTarget);
	deferredMultiPassEffect->SetTexture(POSITIONBUFFER, m_pPositionTarget);

	deferredMultiPassEffect->SetTexture("g_AOBuffer", ssao.GetPostTarget());
	

	deferredMultiPassEffect->SetFloat("g_zNear", CameraParam::zNear);
	deferredMultiPassEffect->SetFloat("g_zFar", CameraParam::zFar);

	deferredMultiPassEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);

	UINT numPasses = 0;
	deferredMultiPassEffect->Begin(&numPasses, 0);


	
	deferredMultiPassEffect->SetInt("g_ShadowMapSize", SHADOWMAPSIZE);
	deferredMultiPassEffect->SetFloat("g_ShadowBias", 0.2f);

	if (KEYDOWN('O'))
	{
		g_minVariance += 0.001;
	}
	if (KEYDOWN('L'))
	{
		g_minVariance -= 0.001;
	}
	if (KEYDOWN('N'))
	{
		g_Amount += 0.001;
	}
	if (KEYDOWN('M'))
	{
		g_Amount -= 0.001;
	}
	if (KEYDOWN('R'))
	{
		g_minVariance = 0.2f;
		g_Amount = 0.8f;
	}
	deferredMultiPassEffect->SetFloat("g_MinVariance", g_minVariance);
	deferredMultiPassEffect->SetFloat("g_Amount", g_Amount);

	deferredMultiPassEffect->CommitChanges();

	int lightCount = LIGHTMANAGER::Instance().GetLightCount();
	for (int i = 0; i < lightCount; i++)
	{
		BaseLight* pLight = LIGHTMANAGER::Instance().GetLight(i);
		D3DXVECTOR3 lightDir = pLight->GetLightViewDir();
		D3DXVECTOR3 lightPos = pLight->GetLightViewPos();
		D3DXVECTOR4 lightColor = pLight->GetLightColor();
		D3DXVECTOR4 lightAttenuation = pLight->GetLightAttenuation();
		D3DXVECTOR4 lightCosHalfAngle = pLight->GetLightCosHalfAngle();
		float lightRange = pLight->GetLightRange();
		bool useShadow = pLight->GetUseShadow();

		D3DXVECTOR4 lightDir_View = D3DXVECTOR4(lightDir, 1.0f);
		D3DXVECTOR4 lightPos_View = D3DXVECTOR4(lightPos, 1.0f);

		deferredMultiPassEffect->SetVector("g_LightDir", &lightDir_View);
		deferredMultiPassEffect->SetVector("g_LightPos", &lightPos_View);
		deferredMultiPassEffect->SetVector("g_LightColor", &lightColor);
		deferredMultiPassEffect->SetVector("g_LightAttenuation", &lightAttenuation);
		deferredMultiPassEffect->SetFloat("g_LightRange", lightRange);
		deferredMultiPassEffect->SetVector("g_LightCosAngle", &D3DXVECTOR4(lightCosHalfAngle.x, lightCosHalfAngle.y, 0.0f, 0.0f));

		D3DXMATRIX invView;
		D3DXMatrixInverse(&invView, NULL, &RENDERDEVICE::Instance().ViewMatrix);
		deferredMultiPassEffect->SetMatrix("g_ShadowView", &pLight->GetLightViewMatrix());
		deferredMultiPassEffect->SetMatrix("g_ShadowProj", &pLight->GetLightProjMatrix());
		deferredMultiPassEffect->SetMatrix("g_invView", &invView);
		deferredMultiPassEffect->SetBool("g_bUseShadow", useShadow);

		if (useShadow)
			deferredMultiPassEffect->SetTexture("g_ShadowBuffer", pLight->GetShadowTarget());

		UINT pass = 0;
		LightType lt = pLight->GetLightType();
		if (lt == eDirectionLight)
		{
			pass = 0;
		}
		else if (lt == ePointLight)
		{
			pass = 4;
		}
		else if (lt == eSpotLight)
		{
			pass = 5;
		}
		else
		{
			pass = 0;
		}

		deferredMultiPassEffect->CommitChanges();

		deferredMultiPassEffect->BeginPass(pass);

		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

		deferredMultiPassEffect->EndPass();
	}

	//环境光Pass
	deferredMultiPassEffect->BeginPass(1);

	deferredMultiPassEffect->SetVector("g_AmbientColor", &AmbientColor);
	deferredMultiPassEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	deferredMultiPassEffect->EndPass();
	
	//纹理及AO的Pass
	deferredMultiPassEffect->BeginPass(2);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	deferredMultiPassEffect->EndPass();

	//DebugPass
	deferredMultiPassEffect->BeginPass(3);
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

	RenderGBuffer();

	//RenderDiffuse();
	//RenderNormalDepth();

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
