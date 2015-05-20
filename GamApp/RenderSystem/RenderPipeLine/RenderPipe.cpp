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
#include "RenderPipeLine/PostEffect/SSGI.h"
#include "RenderPipeLine/PostEffect/EdgeRecognize.h"
#include "RenderPipeLine/PostEffect/NPR.h"
#include "RenderPipeLine/PostEffect/sumi_e.h"

#include "Sky/SkyBox.h"

struct SCREENQUADVERTEX
{
	D3DXVECTOR3		position;
	FLOAT			tu, tv;
};

LPDIRECT3DVERTEXDECLARATION9	mScreenQuadDecl;
int								mScreenQuadByteSize;
const D3DVERTEXELEMENT9 SCREENQUADDECL[] =
{
	{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	D3DDECL_END()
};

LPDIRECT3DVERTEXBUFFER9		pScreenQuadVertex;
LPDIRECT3DINDEXBUFFER9		pScreenQuadIndex;

LPD3DXEFFECT GBufferEffect;

LPD3DXEFFECT deferredMultiPassEffect;
LPD3DXEFFECT shadingPassEffect;
LPD3DXEFFECT finalColorEffect;

D3DXMATRIX shadowOrthoWorld;
D3DXMATRIX shadowOrthoView;
D3DXMATRIX shadowOrthoProj;
D3DXMATRIX invShadowOrthoProj;

SSAO ssao;
HDRLighting hdrLighting;
DOF dof;
SSGI ssgi;
PostEffectBase fxaa;
PostEffectBase ditherHalfToning;
PostEffectBase colorChange;

EdgeRecognize edgeRecognize;
PostEffectBase edgeChange;
NPR npr;
SumiE sumiE;

SkyBox skyBox;

RenderPipe::RenderPipe()
{
	BuildScreenQuad();
	BuildBuffers();
	BuildEffects();

	//Build Post Effect
	ssao.CreatePostEffect();
	hdrLighting.CreatePostEffect();
	dof.CreatePostEffect();
	ssgi.CreatePostEffect();

	fxaa.CreatePostEffect("System\\FXAA.fx");
	ditherHalfToning.CreatePostEffect("System\\Dither_Halftoning.fx");
	colorChange.CreatePostEffect("System\\ColorChange.fx");

	edgeRecognize.CreatePostEffect("System\\EdgeRecognize.fx");
	edgeChange.CreatePostEffect("System\\EdgeChange.fx");
	npr.CreatePostEffect("System\\NPR.fx");
	sumiE.CreatePostEffect();

	m_enableAO = true;
	m_enableDOF = false;
	m_enableHDR = true;
	m_enableGI = false;
	m_enableFXAA = true;
	m_enableDither = false;
	m_enableColorChange = false;

	m_enableEdgeRecognize = false;

	m_showNormal = false;
	m_showPosition = false;
	m_showDiffuse = false;
	m_showDiffuseLight = false;
	m_showShadowResult = false;
	m_showShadowResult = false;

	m_debugMode = NONE;
	//==========================================================
	//Build SkyBox
	skyBox.BuildSkyBox();
	skyBox.SetSkyTexture("Res\\SkyBox\\bottom.jpg", 0);
	skyBox.SetSkyTexture("Res\\SkyBox\\left.jpg", 1);
	skyBox.SetSkyTexture("Res\\SkyBox\\right.jpg", 2);
	skyBox.SetSkyTexture("Res\\SkyBox\\top.jpg", 3);
	skyBox.SetSkyTexture("Res\\SkyBox\\back.jpg", 4);
	skyBox.SetSkyTexture("Res\\SkyBox\\front.jpg", 5);
	
}



void RenderPipe::BuildScreenQuad()
{
	RENDERDEVICE::Instance().g_pD3DDevice->CreateVertexDeclaration(SCREENQUADDECL, &mScreenQuadDecl);
	mScreenQuadByteSize = D3DXGetDeclVertexSize(SCREENQUADDECL, 0);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateVertexBuffer(4 * mScreenQuadByteSize
		, 0
		, 0
		//,D3DPOOL_DEFAULT
		, D3DPOOL_MANAGED
		, &pScreenQuadVertex
		, NULL);
	RENDERDEVICE::Instance().g_pD3DDevice->CreateIndexBuffer(2 * 3 * sizeof(DWORD),
		D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_MANAGED, &pScreenQuadIndex, 0);
	DWORD* indices = 0;
	pScreenQuadIndex->Lock(0, 0, (void**)&indices, 0);
	//全部逆时针绘制，在延迟渲染时剔除正面，就可以保证灯光和渲染面的剔除是统一的
	indices[0] = 0;
	indices[1] = 2;
	indices[2] = 1;

	indices[3] = 1;
	indices[4] = 2;
	indices[5] = 3;
	pScreenQuadIndex->Unlock();

	SCREENQUADVERTEX* pVertices1;
	pScreenQuadVertex->Lock(0, 4 * sizeof(SCREENQUADVERTEX), (void**)&pVertices1, 0);

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

	pScreenQuadVertex->Unlock();
}

void RenderPipe::BuildBuffers()
{
	//G-Buffer
	//B-Buffer分配
	//					   R8			 G8		      B8			  A8
	//DiffuseBuffer  |  diffuseR  |   diffuseG  |  diffuseG  |  specularIntensity |
	//NormalBuffer	 |		normalX		 |	 normalY	     |	  shininess       |
	//PositionBuffer |					  viewSpaceDepth						  |
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pDiffuseTarget, NULL);
	HRESULT hr = m_pDiffuseTarget->GetSurfaceLevel(0, &m_pDiffuseSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pNormalTarget, NULL);
	hr = m_pNormalTarget->GetSurfaceLevel(0, &m_pNormalSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_R32F, D3DPOOL_DEFAULT,
		&m_pPositionTarget, NULL);
	hr = m_pPositionTarget->GetSurfaceLevel(0, &m_pPositionSurface);

	//L-Buffer
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT,
		&m_pDiffuseLightTarget, NULL);
	hr = m_pDiffuseLightTarget->GetSurfaceLevel(0, &m_pDiffuseLightSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT,
		&m_pSpecularLightTarget, NULL);
	hr = m_pSpecularLightTarget->GetSurfaceLevel(0, &m_pSpecularLightSurface);

	//Shadow-Buffer
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&m_pShadowTarget, NULL);
	hr = m_pShadowTarget->GetSurfaceLevel(0, &m_pShadowSurface);

	//Main-Buffer
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT,
		&m_pMainColorTarget, NULL);
	hr = m_pMainColorTarget->GetSurfaceLevel(0, &m_pMainColorSurface);
}

void RenderPipe::BuildEffects()
{
	ID3DXBuffer* error = 0;

	error = 0;
	if (E_FAIL == ::D3DXCreateEffectFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "System\\DeferredRender_MultiPass.fx", NULL, NULL, D3DXSHADER_DEBUG,
		NULL, &deferredMultiPassEffect, &error))
	{
		MessageBox(GetForegroundWindow(), (char*)error->GetBufferPointer(), "Shader", MB_OK);
		abort();
	}

	error = 0;
	if (E_FAIL == ::D3DXCreateEffectFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "System\\ShadingPass.fx", NULL, NULL, D3DXSHADER_DEBUG,
		NULL, &shadingPassEffect, &error))
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
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	UINT nPasses = 0;
	HRESULT r1 = GBufferEffect->Begin(&nPasses, 0);
	HRESULT r2 = GBufferEffect->BeginPass(0);

	deferredMultiPassEffect->SetFloat("g_zNear", CameraParam::zNear);
	deferredMultiPassEffect->SetFloat("g_zFar", CameraParam::zFar);

	skyBox.RenderInGBuffer(GBufferEffect);

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
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

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
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

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
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

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
		
		pLight->RenderShadow(mRenderUtilList);
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

bool enableStencilLight = true;

void RenderPipe::DeferredRender_MultiPass()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	//渲染灯光结果
	DeferredRender_Lighting();
	
	//使用灯光结果进行着色
	DeferredRender_Shading();

	//恢复渲染状态
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, NULL);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(1, NULL);
}

void RenderPipe::ComputeLightPassIndex(LightType type, UINT& lightPassIndex, UINT& shadowPassIndex)
{
	if (type == eDirectionLight)
	{
		lightPassIndex = 0;
		shadowPassIndex = 3;
	}
	else if (type == ePointLight)
	{
		lightPassIndex = 1;
		shadowPassIndex = 4;
	}
	else if (type == eSpotLight)
	{
		lightPassIndex = 2;
		shadowPassIndex = 3;
	}
	else
	{
		lightPassIndex = 0;
		shadowPassIndex = 3;
	}
}

//
void RenderPipe::DeferredRender_Lighting()
{
	//Lighting Pass
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pDiffuseLightSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(1, m_pSpecularLightSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_STENCIL, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	deferredMultiPassEffect->SetMatrix(VIEWMATRIX, &RENDERDEVICE::Instance().ViewMatrix);
	deferredMultiPassEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);
	deferredMultiPassEffect->SetMatrix(INVPROJMATRIX, &RENDERDEVICE::Instance().InvProjMatrix);


	deferredMultiPassEffect->SetTexture(NORMALBUFFER, m_pNormalTarget);
	deferredMultiPassEffect->SetTexture(POSITIONBUFFER, m_pPositionTarget);

	deferredMultiPassEffect->SetInt(SCREENWIDTH, RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth);
	deferredMultiPassEffect->SetInt(SCREENHEIGHT, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight);
	deferredMultiPassEffect->SetFloat("g_zNear", CameraParam::zNear);
	deferredMultiPassEffect->SetFloat("g_zFar", CameraParam::zFar);

	deferredMultiPassEffect->CommitChanges();

	UINT numPasses = 0;
	deferredMultiPassEffect->Begin(&numPasses, 0);

	if (GAMEINPUT::Instance().KeyDown(DIK_G) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_minVariance += 0.001f;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_G) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_minVariance -= 0.001f;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_H) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_Amount += 0.001f;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_H) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		g_Amount -= 0.001f;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_R))
	{
		g_minVariance = 0.2f;
		g_Amount = 0.7f;
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

		deferredMultiPassEffect->SetBool("g_bUseShadow", useShadow);

		D3DXMATRIX lightVolumeMatrix = pLight->GetLightVolumeTransform();
		deferredMultiPassEffect->SetMatrix("g_LightVolumeWVP", &lightVolumeMatrix);
		D3DXMATRIX toViewDirMatrix = pLight->GetToViewDirMatrix();
		deferredMultiPassEffect->SetMatrix("g_ToViewDirMatrix", &toViewDirMatrix);

		LightType lt = pLight->GetLightType();
		UINT lightpass = 0;
		UINT shadowPass = 0;

		ComputeLightPassIndex(lt, lightpass, shadowPass);

		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pShadowSurface);
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(1, NULL);

		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 255, 255, 255), 1.0f, 0);
		if (useShadow)
		{
			deferredMultiPassEffect->SetMatrix("g_invView", &RENDERDEVICE::Instance().InvViewMatrix);
			deferredMultiPassEffect->SetMatrix("g_ShadowView", &pLight->GetLightViewMatrix());
			deferredMultiPassEffect->SetMatrix("g_ShadowProj", &pLight->GetLightProjMatrix());

			D3DXMATRIX viewToLightProjMat = RENDERDEVICE::Instance().InvViewMatrix * pLight->GetLightViewMatrix() * pLight->GetLightProjMatrix();
			deferredMultiPassEffect->SetMatrix("g_viewToLightProj", &viewToLightProjMat);
			D3DXMATRIX viewToLightMat = RENDERDEVICE::Instance().InvViewMatrix * pLight->GetLightViewMatrix();
			deferredMultiPassEffect->SetMatrix("g_viewToLight", &viewToLightMat);

			deferredMultiPassEffect->SetTexture("g_ShadowBuffer", pLight->GetShadowTarget());

			deferredMultiPassEffect->SetInt("g_ShadowMapSize", pLight->GetShadowMapSize());
			deferredMultiPassEffect->SetFloat("g_ShadowBias", 0.4f);

			deferredMultiPassEffect->CommitChanges();

			deferredMultiPassEffect->BeginPass(shadowPass);

			pLight->RenderLightVolume();

			deferredMultiPassEffect->EndPass();

		}
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pDiffuseLightSurface);
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(1, m_pSpecularLightSurface);


		if (GAMEINPUT::Instance().KeyPressed(DIK_RETURN))
		{
			enableStencilLight = !enableStencilLight;
		}

		//====================================================================================
		//灯光的模板剔除
		//通过模板剔除掉灯光没有实际照到的像素
		//
		//已知的问题：1.只有VS的话，PS会怎么执行？为什么在点光源阴影之后执行没有PS阶段的StencilPass会导致Stencil结果不正确？看起来是转了九十度
		//			 2.为什么把阴影也应用模板剔除的话，效率比起只处理灯光要低？
		//====================================================================================
		//方向光不使用模板剔除
		if (lt != eDirectionLight && enableStencilLight)
		{
			//清空模板缓冲，使Stencil值均为0
			RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_STENCIL, 0x0, 1.0f, 0);

			//开启深度
			RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
			//启用模板
			RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_STENCILENABLE, true);

			//===============================================
			//Step.1 渲染灯光体的背面
			//第一步需要标记出在灯光照射后表面前方的所有像素，即Z-Buffer深度小于灯光体后表面深度的像素
			//所以将深度测试方式设置为GREATER，使灯光体后表面深度大于Z-Buffer深度的部分标记为1
			//================================================

			//设置模板值时，使用“1”作为要设置的值
			RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_STENCILREF, 0x1);

			//深度大于Z-Buffer时Z-Test结果为Pass
			RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATER);
			//Stencil-Test的结果永远为Pass，即不做Stencil测试
			RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
			//STENCILPASS代表着深度和模板均通过，而模板测试永远通过，所以代表着深度测试通过，即将深度大于Z-Buffer的像素的模板设置为上面设置的“1”
			RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);

			//设置剔除方式为剔除正面，渲染灯光体的背面
			RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
			deferredMultiPassEffect->BeginPass(6);
			pLight->RenderLightVolume();
			deferredMultiPassEffect->EndPass();
			RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);


			//===============================================
			//Step.2 渲染灯光体的正面
			//第二步需要标记出在灯光照射前表面前方的所有像素，即Z-Buffer深度小于灯光体前表面深度的像素
			//然后将这些像素从第一步的结果中去掉，就可以得到同时满足(1)位于灯光后表面前方(2)位于灯光前表面后方 的所有像素
			//将深度测试方式设置为GREATER，使灯光体前表面深度大于Z-Buffer深度的部分标记为0
			//
			//说明：当相机位于灯光范围之内的时候，灯光的前表面深度均为负值，没有像素会通过深度测试
			//================================================

			//设置模板值时，使用“0”作为要设置的值
			RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_STENCILREF, 0x0);

			//深度大于Z-Buffer时Z-Test结果为Pass
			RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATER);
			//Stencil-Test的结果永远为Pass，即不做Stencil测试
			RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
			//将深度大于Z-Buffer的像素的模板设置为上面设置的“0”
			RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);

			//设置剔除方式为剔除背面，渲染灯光体的正面
			RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
			deferredMultiPassEffect->BeginPass(6);
			pLight->RenderLightVolume();
			deferredMultiPassEffect->EndPass();
			RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);


			//===============================================
			//Step.3 渲染实际的灯光
			//将Z-Test方式设置为ALWAYS，即相当于不做深度测试
			//剔除所有模板值等于0的像素，剩余的像素就是灯光实际能够照亮的像素了
			//================================================

			//Z-Test结果永远为Pass
			RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);

			//设置模板值时，使用“0”作为要设置的值
			RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_STENCILREF, 0x0);

			//使用模板测试，模板值不等于上面设置的值，即“0”时，模板测试通过
			RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_NOTEQUAL);
			//模板测试及深度测试通过时，保持模板的值
			RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
		}


		deferredMultiPassEffect->SetTexture("g_ShadowResult", m_pShadowTarget);
		deferredMultiPassEffect->CommitChanges();

		deferredMultiPassEffect->BeginPass(lightpass);

		pLight->RenderLightVolume();

		deferredMultiPassEffect->EndPass();

		//方向光不使用模板剔除
		if (lt != eDirectionLight && enableStencilLight)
		{
			//关闭模板
			RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
			//关闭深度
			RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		}
	}

	//设置全屏矩形
	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, pScreenQuadVertex, 0, mScreenQuadByteSize);
	//RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(mFVF);
	RENDERDEVICE::Instance().g_pD3DDevice->SetVertexDeclaration(mScreenQuadDecl);
	RENDERDEVICE::Instance().g_pD3DDevice->SetIndices(pScreenQuadIndex);

	//环境光Pass
	deferredMultiPassEffect->BeginPass(5);

	deferredMultiPassEffect->SetVector("g_AmbientColor", &AmbientColor);
	deferredMultiPassEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
	deferredMultiPassEffect->EndPass();

	deferredMultiPassEffect->End();
}

void RenderPipe::DeferredRender_Shading()
{
	//Shading Pass
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pMainColorSurface);

	//设置全屏矩形
	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, pScreenQuadVertex, 0, mScreenQuadByteSize);
	//RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(mFVF);
	RENDERDEVICE::Instance().g_pD3DDevice->SetVertexDeclaration(mScreenQuadDecl);
	RENDERDEVICE::Instance().g_pD3DDevice->SetIndices(pScreenQuadIndex);

	UINT numPasses = 0;
	shadingPassEffect->Begin(&numPasses, 0);

	shadingPassEffect->SetInt(SCREENWIDTH, RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth);
	shadingPassEffect->SetInt(SCREENHEIGHT, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight);
	shadingPassEffect->SetFloat("g_zNear", CameraParam::zNear);
	shadingPassEffect->SetFloat("g_zFar", CameraParam::zFar);

	shadingPassEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);

	shadingPassEffect->SetTexture(DIFFUSEBUFFER, m_pDiffuseTarget);
	shadingPassEffect->SetTexture("g_DiffuseLightBuffer", m_pDiffuseLightTarget);
	shadingPassEffect->SetTexture("g_SpecularLightBuffer", m_pSpecularLightTarget);
	shadingPassEffect->CommitChanges();

	shadingPassEffect->BeginPass(0);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
	shadingPassEffect->EndPass();

	shadingPassEffect->End();
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

	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, pScreenQuadVertex, 0, mScreenQuadByteSize);
	RENDERDEVICE::Instance().g_pD3DDevice->SetVertexDeclaration(mScreenQuadDecl);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

	finalColorEffect->SetTexture(0, NULL);

	finalColorEffect->EndPass();
	finalColorEffect->End();
}

void RenderPipe::RenderAll()
{
	RENDERDEVICE::Instance().g_pD3DDevice->BeginScene();
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	UpdateRenderState();

	RENDERDEVICE::Instance().g_pD3DDevice->GetRenderTarget(0, &m_pOriSurface);

	RenderShadow();

	RenderGBuffer();

	//RenderDiffuse();
	//RenderNormal();
	//RenderPosition();
	
	//ForwardRender();

	DeferredRender_MultiPass();

	m_pPostTarget = m_pMainColorTarget;

	if (GAMEINPUT::Instance().KeyPressed(DIK_1))
	{
		m_enableAO = !m_enableAO;
	}
	if (m_enableAO)
	{
		ssao.RenderPost(m_pPostTarget);
		m_pPostTarget = ssao.GetPostTarget();
	}

	if (GAMEINPUT::Instance().KeyPressed(DIK_4))
	{
		m_enableGI = !m_enableGI;
	}

	if (m_enableGI)
	{
		ssgi.RenderPost(m_pPostTarget);
		m_pPostTarget = ssgi.GetPostTarget();
	}

	if (GAMEINPUT::Instance().KeyPressed(DIK_2))
	{
		m_enableHDR = !m_enableHDR;
	}

	if (m_enableHDR)
	{
		hdrLighting.RenderPost(m_pPostTarget);
		m_pPostTarget = hdrLighting.GetPostTarget();
	}

	if (GAMEINPUT::Instance().KeyPressed(DIK_3))
	{
		m_enableDOF = !m_enableDOF;
	}

	if (m_enableDOF)
	{
		dof.RenderPost(m_pPostTarget);
		m_pPostTarget = dof.GetPostTarget();
	}

	if (GAMEINPUT::Instance().KeyPressed(DIK_5))
	{
		m_enableFXAA = !m_enableFXAA;
	}

	if (m_enableFXAA)
	{
		fxaa.RenderPost(m_pPostTarget);
		m_pPostTarget = fxaa.GetPostTarget();
	}

	if (GAMEINPUT::Instance().KeyPressed(DIK_6))
	{
		m_enableDither = !m_enableDither;
	}

	if (m_enableDither)
	{
		ditherHalfToning.RenderPost(m_pPostTarget);
		m_pPostTarget = ditherHalfToning.GetPostTarget();
	}

	if (GAMEINPUT::Instance().KeyPressed(DIK_7))
	{
		m_enableColorChange = !m_enableColorChange;
	}

	if (m_enableColorChange)
	{
		colorChange.RenderPost(m_pPostTarget);
		m_pPostTarget = colorChange.GetPostTarget();
	}

	if (GAMEINPUT::Instance().KeyPressed(DIK_8))
	{
		m_enableEdgeRecognize = !m_enableEdgeRecognize;
	}

	if (m_enableEdgeRecognize)
	{
		sumiE.RenderPost(m_pPostTarget);
		m_pPostTarget = sumiE.GetPostTarget();
		//edgeRecognize.RenderPost(m_pPostTarget);
		//m_pPostTarget = edgeRecognize.GetPostTarget();
		/*
		edgeChange.RenderPost(m_pPostTarget);
		m_pPostTarget = edgeChange.GetPostTarget();

		fxaa.RenderPost(m_pPostTarget);
		m_pPostTarget = fxaa.GetPostTarget();

		edgeChange.RenderPost(m_pPostTarget);
		m_pPostTarget = edgeChange.GetPostTarget();

		fxaa.RenderPost(m_pPostTarget);
		m_pPostTarget = fxaa.GetPostTarget();

		edgeChange.RenderPost(m_pPostTarget);
		m_pPostTarget = edgeChange.GetPostTarget();

		fxaa.RenderPost(m_pPostTarget);
		m_pPostTarget = fxaa.GetPostTarget();

		edgeChange.RenderPost(m_pPostTarget);
		m_pPostTarget = edgeChange.GetPostTarget();
		*/
		//npr.RenderPost(m_pPostTarget, m_pMainColorTarget);
		//m_pPostTarget = npr.GetPostTarget();
	}

#ifdef RENDER_DEBUG
	if (GAMEINPUT::Instance().KeyPressed(DIK_F4))
	{
		if (m_debugMode == ShowNormal)
			m_debugMode = NONE;
		else
			m_debugMode = ShowNormal;
	}

	if (GAMEINPUT::Instance().KeyPressed(DIK_F5))
	{
		if (m_debugMode == ShowPosition)
			m_debugMode = NONE;
		else
			m_debugMode = ShowPosition;
	}

	if (GAMEINPUT::Instance().KeyPressed(DIK_F6))
	{
		if (m_debugMode == ShowDiffuse)
			m_debugMode = NONE;
		else
			m_debugMode = ShowDiffuse;
	}


	if (GAMEINPUT::Instance().KeyPressed(DIK_F7))
	{
		if (m_debugMode == ShowDiffuseLight)
			m_debugMode = NONE;
		else
			m_debugMode = ShowDiffuseLight;
	}


	if (GAMEINPUT::Instance().KeyPressed(DIK_F8))
	{
		if (m_debugMode == ShowSpecularLight)
			m_debugMode = NONE;
		else
			m_debugMode = ShowSpecularLight;
	}


	if (GAMEINPUT::Instance().KeyPressed(DIK_F9))
	{
		if (m_debugMode == ShowShadowResult)
			m_debugMode = NONE;
		else
			m_debugMode = ShowShadowResult;
	}

	switch (m_debugMode)
	{
	case ShowNormal:
		m_pPostTarget = m_pNormalTarget;
		break;
	case ShowPosition:
		m_pPostTarget = m_pPositionTarget;
		break;
	case ShowDiffuse:
		m_pPostTarget = m_pDiffuseTarget;
		break;
	case ShowDiffuseLight:
		m_pPostTarget = m_pDiffuseLightTarget;
		break;
	case ShowSpecularLight:
		m_pPostTarget = m_pSpecularLightTarget;
		break;
	case ShowShadowResult:
		m_pPostTarget = m_pShadowTarget;
		break;
	default:
		break;
	}
	
#endif // RENDER_DEBUG

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pOriSurface);

	RenderFinalColor();
	
	RENDERDEVICE::Instance().g_pD3DDevice->EndScene();
	RENDERDEVICE::Instance().g_pD3DDevice->Present(0, 0, 0, 0);
}

void RenderPipe::ForwardRender()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pMainColorSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
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
// 	if (GAMEINPUT::Instance().KeyDown(DIK_F4))
// 	{
// 		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
// 	}
// 	if (GAMEINPUT::Instance().KeyDown(DIK_F5))
// 	{
// 		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
// 	}
// 	if (GAMEINPUT::Instance().KeyDown(DIK_F6))
// 	{
// 		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_PHONG);
// 	}
}

