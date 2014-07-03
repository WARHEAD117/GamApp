#include "RenderPipe.h"
#include "D3D9Device.h"
#include "CommonUtil/Input/Input.h"

#include "RenderUtil/EffectParam.h"
#include "Camera/CameraParam.h"

#include "Light/LightManager.h"
#include "Light/DirectionLight.h"

#include "RenderPipeLine/PostEffect/SSAO.h"
#include "RenderPipeLine/PostEffect/HDRLighting.h"
#include "RenderPipeLine/PostEffect/DOF.h"

struct VERTEX
{
	D3DXVECTOR3		position;
	FLOAT			tu, tv;
};

#define D3DFVF_VERTEX (D3DFVF_XYZ|D3DFVF_TEX1)
VERTEX vertices[4];
LPDIRECT3DVERTEXBUFFER9		pBufferVex;
LPDIRECT3DINDEXBUFFER9		pBufferIndex;

LPD3DXEFFECT GBufferEffect;

LPD3DXEFFECT deferredEffect; 
LPD3DXEFFECT deferredMultiPassEffect;
LPD3DXEFFECT finalColorEffect;

D3DXMATRIX shadowOrthoWorld;
D3DXMATRIX shadowOrthoView;
D3DXMATRIX shadowOrthoProj;
D3DXMATRIX invShadowOrthoProj;
int SHADOWMAPSIZE = 1024;

SSAO ssao;
HDRLighting hdrLighting;
DOF dof;

RenderPipe::RenderPipe()
{

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pDiffuseTarget, NULL);
	HRESULT hr = m_pDiffuseTarget->GetSurfaceLevel(0, &m_pDiffuseSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT,
		&m_pNormalTarget, NULL);
	hr = m_pNormalTarget->GetSurfaceLevel(0, &m_pNormalSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT,
		&m_pPositionTarget, NULL);
	hr = m_pPositionTarget->GetSurfaceLevel(0, &m_pPositionSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT,
		&m_pLightTarget, NULL);
	hr = m_pLightTarget->GetSurfaceLevel(0, &m_pLightSurface);

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
	RENDERDEVICE::Instance().g_pD3DDevice->CreateIndexBuffer(2 * 3 * sizeof(DWORD),
		D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_MANAGED, &pBufferIndex, 0);
	DWORD* indices = 0;
	pBufferIndex->Lock(0, 0, (void**)&indices, 0);
	//全部逆时针绘制，在延迟渲染时剔除正面，就可以保证灯光和渲染面的剔除是统一的
	indices[0] = 0;
	indices[1] = 2;
	indices[2] = 1;

	indices[3] = 1;
	indices[4] = 2;
	indices[5] = 3;
	pBufferIndex->Unlock();

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

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pShadowTarget, NULL);
	hr = m_pShadowTarget->GetSurfaceLevel(0, &m_pShadowSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateDepthStencilSurface(SHADOWMAPSIZE, SHADOWMAPSIZE, 
		D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, TRUE,
		&m_pDepthStencilShadowSurface, NULL);
	//=================================================
	ID3DXBuffer* error = 0;

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
	if (E_FAIL == ::D3DXCreateEffectFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "System\\FinalColor.fx", NULL, NULL, D3DXSHADER_DEBUG,
		NULL, &finalColorEffect, &error))
	{
		MessageBox(GetForegroundWindow(), (char*)error->GetBufferPointer(), "Shader", MB_OK);
		abort();
	}
	//=================================================


	ssao.CreatePostEffect();
	hdrLighting.CreatePostEffect();
	dof.CreatePostEffect();
}




RenderPipe::~RenderPipe()
{
	ClearRenderUtil();
}

void RenderPipe::RenderGBuffer()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pDiffuseSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(1, m_pNormalSurface);
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

void RenderPipe::RenderNormal()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pNormalSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0, 0), 1.0f, 0);

	UINT nPasses = 0;
	HRESULT r1 = RENDERDEVICE::Instance().GetNormalEffect()->Begin(&nPasses, 0);
	HRESULT r2 = RENDERDEVICE::Instance().GetNormalEffect()->BeginPass(0);

	for (int i = 0; i < mRenderUtilList.size(); ++i)
	{
		//mRenderUtilList[i]->RenderNormal();
		mRenderUtilList[i]->RenderDeferredGeometry(RENDERDEVICE::Instance().GetNormalEffect());
	}

	RENDERDEVICE::Instance().GetNormalEffect()->EndPass();
	RENDERDEVICE::Instance().GetNormalEffect()->End();
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
			if (pLight->GetLightType() == eDirectionLight || pLight->GetLightType() == eSpotLight)
			{
				D3DXMATRIX lightViewMat = pLight->GetLightViewMatrix();
				D3DXMATRIX lightProjMat = pLight->GetLightProjMatrix();
				pLight->SetShadowTarget();

				for (int i = 0; i < mRenderUtilList.size(); ++i)
				{
					mRenderUtilList[i]->RenderShadow(lightViewMat, lightProjMat, pLight->GetLightType());
				}
			}
			else if (pLight->GetLightType() == ePointLight)
			{
				for (int pointDir = 0; pointDir < 6; pointDir++)
				{
					D3DXMATRIX lightViewMat = pLight->GetPointLightViewMatrix(pointDir);
					D3DXMATRIX lightProjMat = pLight->GetLightProjMatrix();
					pLight->SetPointShadowTarget(pointDir);

					for (int i = 0; i < mRenderUtilList.size(); ++i)
					{
						mRenderUtilList[i]->RenderShadow(lightViewMat, lightProjMat, pLight->GetLightType());
					}
				}
			}
			
		}
	}

	if (NULL != pOldDS)
	{
		RENDERDEVICE::Instance().g_pD3DDevice->SetDepthStencilSurface(pOldDS);
		SafeRelease(pOldDS);
	}
}

D3DXVECTOR4	AmbientColor = D3DXVECTOR4(0.2f, 0.2f, 0.2f, 0);

float g_minVariance = 0.2f;
float g_Amount = 0.7f;

void RenderPipe::DeferredRender_MultiPass()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pMainColorSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0, 0), 1.0f, 0);

	deferredMultiPassEffect->SetMatrix(VIEWMATRIX, &RENDERDEVICE::Instance().ViewMatrix);
	deferredMultiPassEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);
	deferredMultiPassEffect->SetMatrix(INVPROJMATRIX, &RENDERDEVICE::Instance().InvProjMatrix);

	deferredMultiPassEffect->SetTexture(DIFFUSEBUFFER, m_pDiffuseTarget);
	deferredMultiPassEffect->SetTexture(NORMALBUFFER, m_pNormalTarget);
	deferredMultiPassEffect->SetTexture(POSITIONBUFFER, m_pPositionTarget);

	deferredMultiPassEffect->SetInt(SCREENWIDTH, RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth);
	deferredMultiPassEffect->SetInt(SCREENHEIGHT, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight);
	deferredMultiPassEffect->SetFloat("g_zNear", CameraParam::zNear);
	deferredMultiPassEffect->SetFloat("g_zFar", CameraParam::zFar);

	deferredMultiPassEffect->SetTexture("g_AOBuffer", ssao.GetPostTarget());
	
	deferredMultiPassEffect->CommitChanges();

	UINT numPasses = 0;
	deferredMultiPassEffect->Begin(&numPasses, 0);


	deferredMultiPassEffect->SetInt("g_ShadowMapSize", SHADOWMAPSIZE);
	deferredMultiPassEffect->SetFloat("g_ShadowBias", 0.2f);
	
	if (GAMEINPUT::Instance().KeyDown(DIK_G) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_minVariance += 0.001;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_G) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_minVariance -= 0.001;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_H) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_Amount += 0.001;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_H) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_Amount -= 0.001;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_R))
	{
		g_minVariance = 0.2f;
		g_Amount = 0.7f;
	}
	deferredMultiPassEffect->SetFloat("g_MinVariance", g_minVariance);
	deferredMultiPassEffect->SetFloat("g_Amount", g_Amount);

	deferredMultiPassEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

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

		deferredMultiPassEffect->SetBool("g_bUseShadow", useShadow);

		RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, pBufferVex, 0, sizeof(VERTEX));
		RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
		RENDERDEVICE::Instance().g_pD3DDevice->SetIndices(pBufferIndex);

		D3DXMATRIX lightVolumeMatrix = pLight->GetWorldTransform();
		D3DXMATRIX scaleMatrix;
		D3DXMatrixScaling(&scaleMatrix, lightRange, lightRange, lightRange);
		lightVolumeMatrix = scaleMatrix  * lightVolumeMatrix;
		lightVolumeMatrix = lightVolumeMatrix * RENDERDEVICE::Instance().ViewMatrix * RENDERDEVICE::Instance().ProjMatrix;

		UINT pass = 0;
		UINT shadowPass = 0;
		LightType lt = pLight->GetLightType();

		if (lt == eDirectionLight)
		{
			pass = 0;
			shadowPass = 3;
		}
		else if (lt == ePointLight)
		{
			deferredMultiPassEffect->SetMatrix("g_LightVolumeWVP", &lightVolumeMatrix);
			pass = 1;
			shadowPass = 4;
		}
		else if (lt == eSpotLight)
		{
			deferredMultiPassEffect->SetMatrix("g_LightVolumeWVP", &lightVolumeMatrix);
			pass = 2;
			shadowPass = 3;
		}
		else
		{
			pass = 0;
			shadowPass = 3;
		}

		HRESULT  ret = RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pShadowSurface);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 255, 255, 255), 1.0f, 0);
		if (useShadow)
		{
			D3DXMATRIX invView;
			D3DXMatrixInverse(&invView, NULL, &RENDERDEVICE::Instance().ViewMatrix);
			deferredMultiPassEffect->SetMatrix("g_invView", &invView);
			deferredMultiPassEffect->SetMatrix("g_ShadowView", &pLight->GetLightViewMatrix());
			deferredMultiPassEffect->SetMatrix("g_ShadowProj", &pLight->GetLightProjMatrix());

			if (pLight->GetLightType() == eDirectionLight || pLight->GetLightType() == eSpotLight)
			{
				deferredMultiPassEffect->SetTexture("g_ShadowBuffer", pLight->GetShadowTarget());
			}
			else if (pLight->GetLightType() == ePointLight)
			{
				deferredMultiPassEffect->SetTexture("g_PointShadowBuffer", pLight->GetPointShadowTarget());
			}
			
			deferredMultiPassEffect->CommitChanges();

			
			deferredMultiPassEffect->BeginPass(shadowPass);
			if (lt == eDirectionLight || pLight->GetLightType() == eSpotLight)
			{

				RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
			}
			else
			{
				pLight->RenderLightVolume();
			}

			deferredMultiPassEffect->EndPass();

		}
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pMainColorSurface);

		deferredMultiPassEffect->SetTexture("g_ShadowResult", m_pShadowTarget);
		deferredMultiPassEffect->CommitChanges();

		deferredMultiPassEffect->BeginPass(pass);

		if (lt == eDirectionLight)
		{
			RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
		}
		else
		{
			pLight->RenderLightVolume();
		}
		
		deferredMultiPassEffect->EndPass();

	}

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	
	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->SetIndices(pBufferIndex);
	

	
	//环境光Pass
	deferredMultiPassEffect->BeginPass(5);

	deferredMultiPassEffect->SetVector("g_AmbientColor", &AmbientColor);
	deferredMultiPassEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
	deferredMultiPassEffect->EndPass();
	

	//纹理及AO的Pass
	deferredMultiPassEffect->BeginPass(6);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
	deferredMultiPassEffect->EndPass();

	//DebugPass
	deferredMultiPassEffect->BeginPass(7);
	//RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
	deferredMultiPassEffect->EndPass();

	deferredMultiPassEffect->End();

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	deferredMultiPassEffect->SetTexture(0, NULL);
}


void RenderPipe::RenderFinalColor()
{
	UINT numPasses = 0;
	finalColorEffect->Begin(&numPasses, 0);
	finalColorEffect->BeginPass(0);

	finalColorEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);
	finalColorEffect->SetMatrix(INVPROJMATRIX, &RENDERDEVICE::Instance().InvProjMatrix);

	if (m_pPostTarget == NULL)
		m_pPostTarget = m_pMainColorTarget;

	finalColorEffect->SetTexture(FINALCOLORBUFFER, m_pPostTarget);

	finalColorEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, pBufferVex, 0, sizeof(VERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(D3DFVF_VERTEX);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	finalColorEffect->SetTexture(0, NULL);

	finalColorEffect->EndPass();
	finalColorEffect->End();
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
	//RenderNormal();
	//RenderPosition();
	
	//ForwardRender();

	ssao.RenderPost();

	DeferredRender_MultiPass();

	//hdrLighting.RenderPost(m_pMainColorTarget);
	//m_pPostTarget = hdrLighting.GetPostTarget();

	//dof.RenderPost(m_pPostTarget);
	//m_pPostTarget = dof.GetPostTarget();

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pOriSurface);

	RenderFinalColor();
	
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
	if (GAMEINPUT::Instance().KeyDown(DIK_F1))
	{
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_F2))
	{
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_F3))
	{
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_F4))
	{
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_F5))
	{
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_F6))
	{
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_PHONG);
	}
}
