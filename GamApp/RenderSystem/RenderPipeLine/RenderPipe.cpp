#include "RenderPipe.h"
#include "D3D9Device.h"
#include "CommonUtil/Input/Input.h"

#include "RenderUtil/EffectParam.h"
#include "Camera/CameraParam.h"

#include "Light/LightManager.h"
#include "Light/DirectionLight.h"

#include "RenderPipeLine/PostEffect/HBAO.h"
#include "RenderPipeLine/PostEffect/SSAO.h"
#include "RenderPipeLine/PostEffect/HDRLighting.h"
#include "RenderPipeLine/PostEffect/DOF.h"
#include "RenderPipeLine/PostEffect/SSGI.h"
#include "RenderPipeLine/PostEffect/SSR.h"

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
LPD3DXEFFECT ssrEffect;
LPD3DXEFFECT shadingPassEffect;
LPD3DXEFFECT finalColorEffect;

D3DXMATRIX shadowOrthoWorld;
D3DXMATRIX shadowOrthoView;
D3DXMATRIX shadowOrthoProj;
D3DXMATRIX invShadowOrthoProj;

HBAO hbao;
SSAO ssao;
HDRLighting hdrLighting;
DOF dof;
SSGI ssgi;
PostEffectBase fxaa;
PostEffectBase ditherHalfToning;
SSR ssr;

PostEffectBase colorChange;

SkyBox skyBox;

bool	m_enableHBAO;
bool	m_enableIBL;
bool	m_enableDOF;
bool	m_enableHDR;
bool	m_enableGI;
bool	m_enableFXAA;
bool	m_enableDither; 
bool	m_enableSSR;
bool	m_enableSSRPost;

bool	m_enableColorChange;


//--------------------------------------------------------
//ssr
float		m_Roughness;
float		m_RayAngle;
float		m_StepLength;
float		m_ScaleFactor;
float		m_ScaleFactor2;
float		m_rad_threshold;
bool		m_SSREnable;

bool m_Switch3 = false;
bool m_ReprojectPassSwitch = true;
bool m_HitReprojectSwitch = false;
bool m_ClampHistorySwitch = false;
bool m_UseMip = true;

LPDIRECT3DTEXTURE9			m_pRandomTex;
LPDIRECT3DTEXTURE9			m_pEnvBRDFLUT;

LPDIRECT3DTEXTURE9			m_pSSRTarget;
LPDIRECT3DSURFACE9			m_pSSRSurface;

LPDIRECT3DTEXTURE9			m_pSSRFinalTarget;
LPDIRECT3DSURFACE9			m_pSSRFinalSurface;

LPDIRECT3DTEXTURE9			m_pResolveTarget;
LPDIRECT3DSURFACE9			m_pResolveSurface;

LPDIRECT3DTEXTURE9			m_pTemporalTarget;
LPDIRECT3DSURFACE9			m_pTemporalSurface;
LPDIRECT3DTEXTURE9			m_pTemporalSwapTarget;
LPDIRECT3DSURFACE9			m_pTemporalSwapSurface;


LPDIRECT3DTEXTURE9			m_pMainLastTarget;
LPDIRECT3DSURFACE9			m_pMainLastSurface;
//--------------------------------------------------------

RenderPipe::RenderPipe()
{
	BuildScreenQuad();
	BuildBuffers();
	BuildEffects();

	//Build Post Effect
	hbao.CreatePostEffect();
	ssao.CreatePostEffect();
	hdrLighting.CreatePostEffect();
	dof.CreatePostEffect();
	ssgi.CreatePostEffect();
	ssr.CreatePostEffect();

	fxaa.CreatePostEffect("System\\FXAA.fx");
	ditherHalfToning.CreatePostEffect("System\\Dither_Halftoning.fx");
	colorChange.CreatePostEffect("System\\ColorChange.fx");

	m_enableHBAO = true; 
	m_enableIBL = true;
	m_enableDOF = false;
	m_enableHDR = true;
	m_enableGI = false;
	m_enableFXAA = true;
	m_enableDither = false;
	m_enableColorChange = false;
	m_enableSSR = true;
	m_enableSSRPost = false;

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

	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "Res\\Sky\\s\\uffizi-large.hdr", &m_pSkyTex))//daytime//uffizi-large
	{
		MessageBox(GetForegroundWindow(), "TextureError", "Sky", MB_OK);
		abort();
	}
	if (E_FAIL == D3DXCreateCubeTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "Res\\Sky\\s\\test.dds", &m_pSkyCube))//daytime//uffizi-large//env.dds
	{
		MessageBox(GetForegroundWindow(), "TextureError", "Sky", MB_OK);
		abort();
	}
	skyBox.BuildSkyQuad();


	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "System\\noiseColor.png", &m_pRandomTex))
	{
		MessageBox(GetForegroundWindow(), "TextureError", "randomNormal", MB_OK);
		abort();
	}

	if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "System\\EnvBRDFLUT.dds", &m_pEnvBRDFLUT))
	{
		MessageBox(GetForegroundWindow(), "TextureError", "EnvBRDFLUT", MB_OK);
		abort();
	}

	m_Roughness = 0.2;
	m_RayAngle = 0.1f;

	m_StepLength = 3.0f;
	m_ScaleFactor = 0.0f;
	m_ScaleFactor2 = 1.0f;
	m_rad_threshold = 4.0f;
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
	//DiffuseBuffer  |   albedoR  |    albedoG  |   albedoG  |  matelness |
	//NormalBuffer	 |		normalX		 |	 normalY	     |	Roughness |
	//PositionBuffer |					  viewSpaceDepth				  |
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
		&m_pLightTarget, NULL);
	hr = m_pLightTarget->GetSurfaceLevel(0, &m_pLightSurface);

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

	//======================================================================================================================
	//SSR
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT,
		&m_pSSRFinalTarget, NULL);
	hr = m_pSSRFinalTarget->GetSurfaceLevel(0, &m_pSSRFinalSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / 2, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight / 2,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT,
		&m_pSSRTarget, NULL);
	hr = m_pSSRTarget->GetSurfaceLevel(0, &m_pSSRSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT,
		&m_pResolveTarget, NULL);
	hr = m_pResolveTarget->GetSurfaceLevel(0, &m_pResolveSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT,
		&m_pTemporalTarget, NULL);
	hr = m_pTemporalTarget->GetSurfaceLevel(0, &m_pTemporalSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT,
		&m_pTemporalSwapTarget, NULL);
	hr = m_pTemporalSwapTarget->GetSurfaceLevel(0, &m_pTemporalSwapSurface);


	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight,
		8, D3DUSAGE_RENDERTARGET,
		D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT,
		&m_pMainLastTarget, NULL);
	hr = m_pMainLastTarget->GetSurfaceLevel(0, &m_pMainLastSurface);
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

	//Create SSR effect
	error = 0;
	if (E_FAIL == ::D3DXCreateEffectFromFile(RENDERDEVICE::Instance().g_pD3DDevice, "System\\SSR.fx", NULL, NULL, D3DXSHADER_DEBUG,
		NULL, &ssrEffect, &error))
	{
		MessageBox(GetForegroundWindow(), (char*)error->GetBufferPointer(), "ssrEffect", MB_OK);
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

	GBufferEffect->SetFloat("g_zNear", CameraParam::zNear);
	GBufferEffect->SetFloat("g_zFar", CameraParam::zFar);
	GBufferEffect->SetInt(SCREENWIDTH, RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth);
	GBufferEffect->SetInt(SCREENHEIGHT, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight);

	skyBox.RenderInGBuffer(GBufferEffect);

	for (int i = 0; i < mRenderUtilList.size(); ++i)
	{
		mRenderUtilList[i]->RenderDeferredGeometry(GBufferEffect);
	}

	GBufferEffect->EndPass();

	//skin
	r2 = GBufferEffect->BeginPass(1);

	for (int i = 0; i < mSkinnedMeshList.size(); ++i)
	{
		mSkinnedMeshList[i]->RenderDeferredGeometry(GBufferEffect);
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
		
		pLight->RenderShadow(mRenderUtilList,&mSkinnedMeshList);
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

int frameCount = 0;

void RenderPipe::DeferredRender_MultiPass()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	if (m_enableSSR)
	{
		//渲染SSR结果
		if (frameCount > 0)
			DeferredRender_SSR();
		frameCount++;
	}

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

void RenderPipe::ComputeSSRConfig()
{
	if (GAMEINPUT::Instance().KeyDown(DIK_M) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_Roughness += 0.002;
		m_Roughness = m_Roughness >= 1 ? 1 : m_Roughness;
	}

	if (GAMEINPUT::Instance().KeyDown(DIK_M) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_Roughness -= 0.002;
		m_Roughness = m_Roughness <= 0.0005 ? 0.0005 : m_Roughness;

	}

	if (GAMEINPUT::Instance().KeyDown(DIK_N) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_RayAngle += 0.001f;
		m_RayAngle = m_RayAngle >0.5 ? 0.5 : m_RayAngle;
	}

	if (GAMEINPUT::Instance().KeyDown(DIK_N) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_RayAngle -= 0.001f;
		m_RayAngle = m_RayAngle < 0 ? 0 : m_RayAngle;
	}


	if (GAMEINPUT::Instance().KeyDown(DIK_B) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_StepLength += 0.001f;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_B) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_StepLength -= 0.001f;
	}

	if (GAMEINPUT::Instance().KeyDown(DIK_V) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_ScaleFactor = m_ScaleFactor >= 1 ? 1 : m_ScaleFactor + 0.001;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_V) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_ScaleFactor = m_ScaleFactor <= 0.0 ? 0.0 : m_ScaleFactor - 0.001;
	}


	if (GAMEINPUT::Instance().KeyDown(DIK_C) && !GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_ScaleFactor2 = m_ScaleFactor2 >= 50 ? 50 : m_ScaleFactor2 + 0.04;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_C) && GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		m_ScaleFactor2 = m_ScaleFactor2 <= 0.0 ? 0.0 : m_ScaleFactor2 - 0.04;
	}

	if (GAMEINPUT::Instance().KeyPressed(DIK_NUMPAD1))
	{
		m_Switch3 = !m_Switch3;
	}
	if (GAMEINPUT::Instance().KeyPressed(DIK_NUMPAD2))
	{
		m_ReprojectPassSwitch = !m_ReprojectPassSwitch;
	}
	if (GAMEINPUT::Instance().KeyPressed(DIK_NUMPAD3))
	{
		m_HitReprojectSwitch = !m_HitReprojectSwitch;
	}
	if (GAMEINPUT::Instance().KeyPressed(DIK_NUMPAD4))
	{
		m_ClampHistorySwitch = !m_ClampHistorySwitch;
	}
	if (GAMEINPUT::Instance().KeyPressed(DIK_NUMPAD5))
	{
		m_UseMip = !m_UseMip;
	}


	if (GAMEINPUT::Instance().KeyDown(DIK_R))
	{
		m_Roughness = 0.2f;
		m_RayAngle = 0.1f;

		m_StepLength = 3.0f;
		m_ScaleFactor = 0.0f;
		m_ScaleFactor2 = 10.0f;
	}
}

bool RenderPipe::BuildMipMap(const LPDIRECT3DTEXTURE9 src, LPDIRECT3DTEXTURE9 dest)
{
	if (!src || !dest)
		return false;

	int maxlevel = dest->GetLevelCount();

	LPDIRECT3DSURFACE9 srcSurface;
	src->GetSurfaceLevel(0, &srcSurface);

	LPDIRECT3DSURFACE9 destSurface;
	dest->GetSurfaceLevel(0, &destSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->StretchRect(srcSurface, NULL, destSurface, NULL, D3DTEXF_LINEAR);

	for (int i = 1; i < maxlevel; i++)
	{
		LPDIRECT3DSURFACE9 lastSurface;
		dest->GetSurfaceLevel(i-1, &lastSurface);
		dest->GetSurfaceLevel(i, &destSurface);
		RENDERDEVICE::Instance().g_pD3DDevice->StretchRect(lastSurface, NULL, destSurface, NULL, D3DTEXF_LINEAR);
	}

	return true;
}

void RenderPipe::DeferredRender_SSR()
{
	ComputeSSRConfig();
	int maxMipLevel = m_pMainLastTarget->GetLevelCount();

	//设置全屏矩形
	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, pScreenQuadVertex, 0, mScreenQuadByteSize);
	//RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(mFVF);
	RENDERDEVICE::Instance().g_pD3DDevice->SetVertexDeclaration(mScreenQuadDecl);
	RENDERDEVICE::Instance().g_pD3DDevice->SetIndices(pScreenQuadIndex);


	UINT numPasses = 0;
	ssrEffect->Begin(&numPasses, 0);


	ssrEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);
	ssrEffect->SetMatrix(INVPROJMATRIX, &RENDERDEVICE::Instance().InvProjMatrix);
	ssrEffect->SetMatrix(PROJECTIONMATRIX, &RENDERDEVICE::Instance().ProjMatrix);

	ssrEffect->SetFloat("g_zNear", CameraParam::zNear);
	ssrEffect->SetFloat("g_zFar", CameraParam::zFar);

	float angle = tan(CameraParam::FOV / 2);
	ssrEffect->SetFloat("g_ViewAngle_half_tan", angle);
	float aspect = (float)RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth / RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight;
	ssrEffect->SetFloat("g_ViewAspect", aspect);

	ssrEffect->SetInt(SCREENWIDTH, RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth);
	ssrEffect->SetInt(SCREENHEIGHT, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight);

	int screenSize[2] = { RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth, RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight };
	ssrEffect->SetIntArray("g_ScreenSize", screenSize, 2);

	ssrEffect->SetTexture(NORMALBUFFER, RENDERPIPE::Instance().m_pNormalTarget);
	ssrEffect->SetTexture(POSITIONBUFFER, RENDERPIPE::Instance().m_pPositionTarget);
	ssrEffect->SetTexture("g_RandTex", m_pRandomTex);
	D3DSURFACE_DESC randTexDesc;
	m_pRandomTex->GetLevelDesc(0, &randTexDesc);
	int randTexSize[2] = { randTexDesc.Width, randTexDesc.Height };
	ssrEffect->SetIntArray("g_RandTexSize", randTexSize, 2);

	ssrEffect->SetTexture(MAINCOLORBUFFER, m_pMainLastTarget);

	ssrEffect->SetInt("g_MaxMipLevel", maxMipLevel);
	ssrEffect->SetFloat("g_Roughness", m_Roughness);
	ssrEffect->SetFloat("g_RayAngle", m_RayAngle);

	ssrEffect->SetFloat("g_StepLength", m_StepLength);
	ssrEffect->SetFloat("g_ScaleFactor", m_ScaleFactor);
	ssrEffect->SetFloat("g_ScaleFactor2", m_ScaleFactor2);


	ssrEffect->SetBool("g_Switch1", m_Switch3);

	float randomOffset = 1.0f * rand() / RAND_MAX;
	ssrEffect->SetFloat("g_randomOffset", randomOffset);

	ssrEffect->SetMatrix("g_Proj", &RENDERDEVICE::Instance().ProjMatrix);
	ssrEffect->SetMatrix("g_InverseProj", &RENDERDEVICE::Instance().InvProjMatrix);

	ssrEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pSSRSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 255, 255, 255), 1.0f, 0);
	ssrEffect->CommitChanges();

	ssrEffect->BeginPass(0);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
	ssrEffect->EndPass();

	//================================================================================================================
	//Color Resolve
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pResolveSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 255, 255, 255), 1.0f, 0);

	ssrEffect->SetIntArray("g_ScreenSize", screenSize, 2);
	ssrEffect->SetTexture("g_SSRBuffer", m_pSSRTarget);

	ssrEffect->SetTexture("g_EnvBRDFLUT", m_pEnvBRDFLUT);
	ssrEffect->SetTexture(NORMALBUFFER, RENDERPIPE::Instance().m_pNormalTarget);
	ssrEffect->SetTexture(MAINCOLORBUFFER, m_pMainLastTarget);

	ssrEffect->SetTexture("g_TemporalBuffer", m_pTemporalTarget);

	ssrEffect->SetMatrix("g_LastView", &RENDERDEVICE::Instance().ViewLastMatrix);
	ssrEffect->SetMatrix("g_invView", &RENDERDEVICE::Instance().InvViewMatrix);
	ssrEffect->SetMatrix("g_View", &RENDERDEVICE::Instance().ViewMatrix);
	ssrEffect->SetMatrix("g_Proj", &RENDERDEVICE::Instance().ProjMatrix);
	ssrEffect->SetMatrix("g_InverseProj", &RENDERDEVICE::Instance().InvProjMatrix);

	ssrEffect->SetBool("g_ReprojectPassSwitch", m_ReprojectPassSwitch);
	ssrEffect->SetBool("g_HitReprojectSwitch", m_HitReprojectSwitch);
	ssrEffect->SetBool("g_UseMip", m_UseMip);

	ssrEffect->CommitChanges();

	ssrEffect->BeginPass(3);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
	ssrEffect->EndPass();

	if (m_ReprojectPassSwitch)
	{
		//---------------------------------------------------------------------------------------------------
		//把这次的内容累加到之前累加好的Temporal上，得到用来Swap的Temporal
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pTemporalSwapSurface);
		RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 255, 255, 255), 1.0f, 0);

		ssrEffect->SetTexture("g_TemporalBuffer", m_pTemporalTarget);
		ssrEffect->SetTexture("g_SSRBuffer", m_pResolveTarget);

		ssrEffect->SetMatrix("g_LastView", &RENDERDEVICE::Instance().ViewLastMatrix);
		ssrEffect->SetMatrix("g_invView", &RENDERDEVICE::Instance().InvViewMatrix);
		ssrEffect->SetMatrix("g_View", &RENDERDEVICE::Instance().ViewMatrix);
		ssrEffect->SetMatrix("g_Proj", &RENDERDEVICE::Instance().ProjMatrix);
		ssrEffect->SetMatrix("g_InverseProj", &RENDERDEVICE::Instance().InvProjMatrix);

		ssrEffect->SetBool("g_ClampHistorySwitch", m_ClampHistorySwitch);

		ssrEffect->CommitChanges();

		ssrEffect->BeginPass(4);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
		ssrEffect->EndPass();

		//把最新的累加好的内容交换到实际使用的Temporal上
		//Copy SSRTarget
		RENDERDEVICE::Instance().g_pD3DDevice->StretchRect(m_pTemporalSwapSurface, NULL, m_pTemporalSurface, NULL, D3DTEXF_LINEAR);
	}
	else
	{
		//把最新的累加好的内容交换到实际使用的Temporal上
		//Copy SSRTarget
		RENDERDEVICE::Instance().g_pD3DDevice->StretchRect(m_pResolveSurface, NULL, m_pTemporalSurface, NULL, D3DTEXF_LINEAR);
	}

	//-------------------------------------------------------------------------------------------------------
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pSSRFinalSurface); //renderSurface//m_pSSRFinalSurface
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 255, 255, 255), 1.0f, 0);

	if (m_ReprojectPassSwitch)
	{
		ssrEffect->SetTexture("g_SSRBuffer", m_pTemporalTarget);
	}
	else
	{
		ssrEffect->SetTexture("g_SSRBuffer", m_pResolveTarget);
	}
	//这里是临时的，并不应该用这个作为输出的底色（这个MainColor是上一帧的，SSR反而是这一帧的） 
	ssrEffect->SetTexture(MAINCOLORBUFFER, m_pMainLastTarget);

	ssrEffect->CommitChanges();

	ssrEffect->BeginPass(2);
	RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
	ssrEffect->EndPass();

	ssrEffect->End();
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
	else if (type == eImageBasedLight)
	{
		lightPassIndex = 7;
		shadowPassIndex = 3;
	}
	else
	{
		lightPassIndex = 0;
		shadowPassIndex = 3;
	}
}

void RenderPipe::Lighting(LPD3DXEFFECT effect, BaseLight* pLight)
{
	D3DXVECTOR3 lightDir = pLight->GetLightViewDir();
	D3DXVECTOR3 lightPos = pLight->GetLightViewPos();
	D3DXVECTOR4 lightColor = pLight->GetLightColor();
	D3DXVECTOR4 lightAttenuation = pLight->GetLightAttenuation();
	D3DXVECTOR4 lightCosHalfAngle = pLight->GetLightCosHalfAngle();
	float lightRange = pLight->GetLightRange();
	bool useShadow = pLight->GetUseShadow();

	D3DXVECTOR4 lightDir_View = D3DXVECTOR4(lightDir, 1.0f);
	D3DXVECTOR4 lightPos_View = D3DXVECTOR4(lightPos, 1.0f);

	effect->SetVector("g_LightDir", &lightDir_View);
	effect->SetVector("g_LightPos", &lightPos_View);
	effect->SetVector("g_LightColor", &lightColor);
	effect->SetVector("g_LightAttenuation", &lightAttenuation);
	effect->SetFloat("g_LightRange", lightRange);
	effect->SetVector("g_LightCosAngle", &D3DXVECTOR4(lightCosHalfAngle.x, lightCosHalfAngle.y, 0.0f, 0.0f));

	effect->SetBool("g_bUseShadow", useShadow);

	D3DXMATRIX lightVolumeMatrix = pLight->GetLightVolumeTransform();
	effect->SetMatrix("g_LightVolumeWVP", &lightVolumeMatrix);
	D3DXMATRIX toViewDirMatrix = pLight->GetToViewDirMatrix();
	effect->SetMatrix("g_ToViewDirMatrix", &toViewDirMatrix);

	LightType lt = pLight->GetLightType();
	UINT lightpass = 0;
	UINT shadowPass = 0;

	ComputeLightPassIndex(lt, lightpass, shadowPass);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pShadowSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(1, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 255, 255, 255), 1.0f, 0);
	if (useShadow)
	{
		effect->SetMatrix("g_invView", &RENDERDEVICE::Instance().InvViewMatrix);
		effect->SetMatrix("g_ShadowView", &pLight->GetLightViewMatrix());
		effect->SetMatrix("g_ShadowProj", &pLight->GetLightProjMatrix());

		D3DXMATRIX viewToLightProjMat = RENDERDEVICE::Instance().InvViewMatrix * pLight->GetLightViewMatrix() * pLight->GetLightProjMatrix();
		effect->SetMatrix("g_viewToLightProj", &viewToLightProjMat);
		D3DXMATRIX viewToLightMat = RENDERDEVICE::Instance().InvViewMatrix * pLight->GetLightViewMatrix();
		effect->SetMatrix("g_viewToLight", &viewToLightMat);

		effect->SetTexture("g_ShadowBuffer", pLight->GetShadowTarget());

		effect->SetInt("g_ShadowMapSize", pLight->GetShadowMapSize());
		effect->SetFloat("g_ShadowBias", 0.4f);

		effect->CommitChanges();

		effect->BeginPass(shadowPass);

		pLight->RenderLightVolume();

		effect->EndPass();

	}
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pLightSurface);


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
	if (lt != eDirectionLight && lt != eImageBasedLight && enableStencilLight)
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
		effect->BeginPass(6);
		pLight->RenderLightVolume();
		effect->EndPass();
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
		effect->BeginPass(6);
		pLight->RenderLightVolume();
		effect->EndPass();
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


	effect->SetTexture("g_ShadowResult", m_pShadowTarget);

	pLight->SetLightEffect(effect);

	effect->CommitChanges();

	effect->BeginPass(lightpass);

	pLight->RenderLightVolume();

	effect->EndPass();

	//方向光不使用模板剔除
	if (lt != eDirectionLight && lt != eImageBasedLight && enableStencilLight)
	{
		//关闭模板
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
		//关闭深度
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	}
}
//
void RenderPipe::DeferredRender_Lighting()
{
	//Lighting Pass
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pLightSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_STENCIL, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	deferredMultiPassEffect->SetMatrix(VIEWMATRIX, &RENDERDEVICE::Instance().ViewMatrix);
	deferredMultiPassEffect->SetMatrix(WORLDVIEWPROJMATRIX, &RENDERDEVICE::Instance().OrthoWVPMatrix);
	deferredMultiPassEffect->SetMatrix(INVPROJMATRIX, &RENDERDEVICE::Instance().InvProjMatrix);


	deferredMultiPassEffect->SetTexture(NORMALBUFFER, m_pNormalTarget);
	deferredMultiPassEffect->SetTexture(POSITIONBUFFER, m_pPositionTarget);
	deferredMultiPassEffect->SetTexture(DIFFUSEBUFFER, m_pDiffuseTarget);

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

	deferredMultiPassEffect->SetTexture("g_EnvBRDFLUT", m_pEnvBRDFLUT);

	deferredMultiPassEffect->CommitChanges();


	if (m_enableIBL)
	{
		int iblCount = LIGHTMANAGER::Instance().GetIBLCount();

		for (int i = 0; i < iblCount; i++)
		{
			BaseLight* pLight = LIGHTMANAGER::Instance().GetIBL(i);

			Lighting(deferredMultiPassEffect, pLight);
		}
	}

	if (m_enableSSR)
	{
		//SSR反射Pass
		RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, pScreenQuadVertex, 0, mScreenQuadByteSize);
		RENDERDEVICE::Instance().g_pD3DDevice->SetVertexDeclaration(mScreenQuadDecl);
		RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pLightSurface);
		deferredMultiPassEffect->SetTexture("g_SSRBuffer", m_pSSRFinalTarget);
		deferredMultiPassEffect->BeginPass(8);
		deferredMultiPassEffect->CommitChanges();
		RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
		deferredMultiPassEffect->EndPass();
	}

	if (true)
	{
		int lightCount = LIGHTMANAGER::Instance().GetLightCount();
		for (int i = 0; i < lightCount; i++)
		{
			BaseLight* pLight = LIGHTMANAGER::Instance().GetLight(i);

			Lighting(deferredMultiPassEffect, pLight);
		}
	}


	//设置全屏矩形
	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, pScreenQuadVertex, 0, mScreenQuadByteSize);
	//RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(mFVF);
	RENDERDEVICE::Instance().g_pD3DDevice->SetVertexDeclaration(mScreenQuadDecl);
	RENDERDEVICE::Instance().g_pD3DDevice->SetIndices(pScreenQuadIndex);

	//SSR反射Pass

	deferredMultiPassEffect->SetTexture("g_SSRBuffer", ssr.GetPostTarget());

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
	D3DXVECTOR3 cameraPos = RENDERDEVICE::Instance().ViewPosition;
	D3DXMATRIX worldMat;
	D3DXMatrixTranslation(&worldMat, cameraPos.x, cameraPos.y, cameraPos.z);

	D3DXMATRIX invVP;
	invVP = RENDERDEVICE::Instance().InvProjMatrix * RENDERDEVICE::Instance().InvViewMatrix;
	shadingPassEffect->SetMatrix("g_InverseProj", &invVP);

	shadingPassEffect->SetTexture(DIFFUSEBUFFER, m_pDiffuseTarget);
	shadingPassEffect->SetTexture("g_LightBuffer", m_pLightTarget);
	
	shadingPassEffect->SetTexture("g_Sky", m_pSkyTex);
	//shadingPassEffect->SetTexture(NORMALBUFFER, m_pNormalTarget);
	shadingPassEffect->SetTexture(POSITIONBUFFER, m_pPositionTarget);


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


	finalColorEffect->SetBool("g_debugPos", false);
#ifdef RENDER_DEBUG
	if (m_debugMode == ShowPosition)
		finalColorEffect->SetBool("g_debugPos", true);
#endif
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


	if (GAMEINPUT::Instance().KeyPressed(DIK_7))
	{
		m_enableHBAO = !m_enableHBAO;
	}

	if (m_enableHBAO)
	{
		hbao.RenderPost(m_pPostTarget);
		m_pPostTarget = hbao.GetPostTarget();
	}

	if (GAMEINPUT::Instance().KeyPressed(DIK_1))
	{
		m_enableIBL = !m_enableIBL;
	}
	if (m_enableIBL)
	{
		//ssao.RenderPost(m_pPostTarget);
		//m_pPostTarget = ssao.GetPostTarget();
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

	//==========================
	//Copy Main
	LPDIRECT3DSURFACE9	postSurface;
	HRESULT hr = m_pPostTarget->GetSurfaceLevel(0, &postSurface);
	//RENDERDEVICE::Instance().g_pD3DDevice->StretchRect(postSurface, NULL, m_pMainLastSurface, NULL, D3DTEXF_LINEAR);
	BuildMipMap(m_pPostTarget, m_pMainLastTarget);
	//===========================

	if (GAMEINPUT::Instance().KeyPressed(DIK_2))
	{
		m_enableHDR = !m_enableHDR;
	}

	if (m_enableHDR)
	{
		hdrLighting.RenderPost(m_pPostTarget);
		m_pPostTarget = hdrLighting.GetPostTarget();
	}

	if (GAMEINPUT::Instance().KeyPressed(DIK_8))
	{
		m_enableSSR = !m_enableSSR;
	}

	if (GAMEINPUT::Instance().KeyPressed(DIK_9))
	{
		m_enableSSRPost = !m_enableSSRPost;
	}

	if (m_enableSSRPost)
	{
		ssr.RenderPost(m_pPostTarget);
		m_pPostTarget = ssr.GetPostTarget();
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


	if (GAMEINPUT::Instance().KeyPressed(DIK_3))
	{
		m_enableDOF = !m_enableDOF;
	}

	if (m_enableDOF)
	{
		dof.RenderPost(m_pPostTarget);
		m_pPostTarget = dof.GetPostTarget();
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

	/*
	if (GAMEINPUT::Instance().KeyPressed(DIK_7))
	{
		m_enableColorChange = !m_enableColorChange;
	}

	if (m_enableColorChange)
	{
		colorChange.RenderPost(m_pPostTarget);
		m_pPostTarget = colorChange.GetPostTarget();
	}*/

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
		if (m_debugMode == ShowLight)
			m_debugMode = NONE;
		else
			m_debugMode = ShowLight;
	}


	if (GAMEINPUT::Instance().KeyPressed(DIK_F8))
	{
		if (m_debugMode == ShowSSR)
			m_debugMode = NONE;
		else
			m_debugMode = ShowSSR;
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
	case ShowLight:
		m_pPostTarget = m_pLightTarget;
		break;
	case ShowSSR:
		m_pPostTarget = m_pSSRFinalTarget;
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

void RenderPipe::PushSkinnedMesh(SkinnedMesh* const skinnedMesh)
{
	mSkinnedMeshList.push_back(skinnedMesh);
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

