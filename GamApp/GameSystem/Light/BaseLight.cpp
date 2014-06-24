#include "BaseLight.h"
#include "D3D9Device.h"

const D3DXVECTOR3 defaultDir(0.0f, -1.0f, 1.0f);

const D3DXVECTOR3 defaultPos(0.0f, 0.0f, 0.0f);
const D3DXVECTOR3 defaultLookAt(0.0f, -1.0f, 0.0f);
const D3DXVECTOR3 defaultUp(1.0f, 0.0f, 0.0f);
const int SHADOWMAP_SIZE = 1024;

LPD3DXMESH box;

BaseLight::BaseLight() :
m_LightType(eDirectionLight),
m_bUseShadow(false),
m_LightRange(5.0f),
m_LightAngle(D3DXVECTOR2(45.0f, 20.0f)),
m_LightDir(ZEROVECTOR3),
m_LightColor(D3DXCOLOR(0.3,0.3f,0.3f,1.0f)),
m_LightAttenuation(D3DXVECTOR4(0.0, 1.0f, 0.0f, 1.0f))
{
	D3DXCreateBox(RENDERDEVICE::Instance().g_pD3DDevice, 2, 2, 2, &box, NULL);
	//Create post vertex
	RENDERDEVICE::Instance().g_pD3DDevice->CreateVertexBuffer(24 * sizeof(VERTEX)
		, 0
		, D3DFVF_VERTEX
		//,D3DPOOL_DEFAULT
		, D3DPOOL_MANAGED
		, &m_pBufferVex
		, NULL);

	VERTEX* pVertices;
	m_pBufferVex->Lock(0, 24 * sizeof(VERTEX), (void**)&pVertices, 0);

	//初始化顶点缓冲区

	//==========================
	//=============================================================下表面
	pVertices->position = D3DXVECTOR3(1.0f, -1.0f, 1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(1.0f, -1.0f, -1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
	pVertices++;


	//==========================================================================左表面 
	pVertices->position = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(-1.0f, 1.0f, -1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
	pVertices++;

	//==========================================================================右表面
	pVertices->position = D3DXVECTOR3(1.0f, 1.0f, -1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(1.0f, -1.0f, -1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(1.0f, -1.0f, 1.0f);
	pVertices++;

	// 	//==========================================================================上表面
	pVertices->position = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(-1.0f, 1.0f, -1.0f);
	pVertices++;



	pVertices->position = D3DXVECTOR3(1.0f, 1.0f, -1.0f);
	pVertices++;

	// 
	// 	//==========================================================================后表面
	pVertices->position = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(1.0f, -1.0f, 1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
	pVertices++;
	// 
	// 	//==========================================================================前表面
	pVertices->position = D3DXVECTOR3(1.0f, -1.0f, -1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(1.0f, 1.0f, -1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(-1.0f, 1.0f, -1.0f);
	pVertices++;


	m_pBufferVex->Unlock();

	Init();
}

void BaseLight::RenderLightVolume()
{
	box->DrawSubset(0);
}

BaseLight::~BaseLight()
{
}

void BaseLight::Init()
{
	m_LightPos = defaultPos;
	m_LightDir = defaultLookAt;
	m_LightUp = defaultUp;

	RebuildViewMatrix();
	RebuildProjMatrix();
}

void BaseLight::RebuildViewMatrix()
{
	D3DXMatrixLookAtLH(&m_lightViewMat, &m_LightPos, &(m_LightPos + m_LightDir), &m_LightUp);
	if (m_LightType == ePointLight)
	{
		D3DXMatrixLookAtLH(&m_PointlightViewMat[0], &m_LightPos, &(m_LightPos + D3DXVECTOR3(1, 0, 0)), &D3DXVECTOR3(0, 1, 0)); //+X
		D3DXMatrixLookAtLH(&m_PointlightViewMat[1], &m_LightPos, &(m_LightPos + D3DXVECTOR3(-1, 0, 0)), &D3DXVECTOR3(0, 1, 0)); //-X
		D3DXMatrixLookAtLH(&m_PointlightViewMat[2], &m_LightPos, &(m_LightPos + D3DXVECTOR3(0, 1, 0)), &D3DXVECTOR3(0, 0, -1)); //+Y
		D3DXMatrixLookAtLH(&m_PointlightViewMat[3], &m_LightPos, &(m_LightPos + D3DXVECTOR3(0, -1, 0)), &D3DXVECTOR3(0, 0, 1)); //-Y
		D3DXMatrixLookAtLH(&m_PointlightViewMat[4], &m_LightPos, &(m_LightPos + D3DXVECTOR3(0, 0, 1)), &D3DXVECTOR3(0, 1, 0)); //+Z
		D3DXMatrixLookAtLH(&m_PointlightViewMat[5], &m_LightPos, &(m_LightPos + D3DXVECTOR3(0, 0, -1)), &D3DXVECTOR3(0, 1, 0)); //-Z
	}
}

void BaseLight::RebuildProjMatrix()
{
	if (m_LightType == eDirectionLight)
	{
		D3DXMatrixOrthoLH(&m_lightProjMat, 20, 20, 0.01f, 50.0f);
	}
	else if (m_LightType == ePointLight)
	{
		D3DXMatrixPerspectiveFovLH(&m_lightProjMat, 0.5f * D3DX_PI, 1.0f, 0.01, m_LightRange);
	}
	else if (m_LightType == eSpotLight)
	{
		D3DXMatrixPerspectiveFovLH(&m_lightProjMat, m_LightAngle.x / 180.0f * D3DX_PI, 1.0f, 0.01, m_LightRange);
	}

	D3DXMatrixInverse(&m_lightInvProjMat, NULL, &m_lightProjMat);
}

void BaseLight::OnFrame()
{
	D3DXVECTOR4 tempDir;
	D3DXVECTOR4 tempUp;
	D3DXVec3TransformNormal(&m_LightDir, &defaultLookAt, &mWorldTransform);
	D3DXVec3TransformNormal(&m_LightUp, &defaultUp, &mWorldTransform);

	//m_LightUp = D3DXVECTOR3(tempUp.x, tempUp.y, tempUp.z);
	//m_LightDir = D3DXVECTOR3(tempDir.x, tempDir.y, tempDir.z);

	D3DXVec3Normalize(&m_LightDir, &m_LightDir);
	D3DXVec3Normalize(&m_LightUp, &m_LightUp);

	m_LightPos = D3DXVECTOR3(mWorldTransform._41, mWorldTransform._42, mWorldTransform._43);

	RebuildViewMatrix();
}

D3DXVECTOR3 BaseLight::GetLightWorldDir()
{
	return m_LightDir;
}

D3DXVECTOR3 BaseLight::GetLightViewDir()
{
	D3DXVECTOR3 tempDir;
	D3DXVec3TransformNormal(&tempDir, &m_LightDir, &RENDERDEVICE::Instance().ViewMatrix);
	return tempDir;
}

D3DXVECTOR3 BaseLight::GetLightWorldUp()
{
	return m_LightUp;
}

D3DXVECTOR3 BaseLight::GetLightViewUp()
{
	D3DXVECTOR3 tempUp;
	D3DXVec3TransformNormal(&tempUp, &m_LightUp, &RENDERDEVICE::Instance().ViewMatrix);
	return tempUp;
}

D3DXVECTOR3 BaseLight::GetLightWorldPos()
{
	return m_LightPos;
}

D3DXVECTOR3 BaseLight::GetLightViewPos()
{
	D3DXVECTOR3 tempPos;
	D3DXVec3TransformCoord(&tempPos, &m_LightPos, &RENDERDEVICE::Instance().ViewMatrix);
	return tempPos;
}

D3DXCOLOR BaseLight::GetLightColor()
{
	return m_LightColor;
}

void BaseLight::SetLightColor(D3DXCOLOR color)
{
	m_LightColor = color;
}

D3DXMATRIX BaseLight::GetLightViewMatrix()
{
	return m_lightViewMat;
}

D3DXMATRIX BaseLight::GetPointLightViewMatrix(int index)
{
	return m_PointlightViewMat[index];
}

D3DXMATRIX BaseLight::GetLightInvProjMatrix()
{
	return m_lightInvProjMat;
}

D3DXMATRIX BaseLight::GetLightProjMatrix()
{
	return m_lightProjMat;
}


void BaseLight::SetUseShadow(bool useShadow)
{
	m_bUseShadow = false;
	if (useShadow && (m_LightType == eDirectionLight || m_LightType == eSpotLight))
	{
		m_bUseShadow = useShadow;
		BuildShadowMap();
	}
	else if (useShadow && m_LightType == ePointLight)
	{
		m_bUseShadow = useShadow;
		BuildPointShadowMap();
	}
}

void BaseLight::BuildPointShadowMap()
{
	RENDERDEVICE::Instance().g_pD3DDevice->CreateCubeTexture(SHADOWMAP_SIZE,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_G16R16F, D3DPOOL_DEFAULT,
		&m_pPointShadowTarget, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateDepthStencilSurface(SHADOWMAP_SIZE, SHADOWMAP_SIZE,
		D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, TRUE,
		&m_pDepthStencilShadowSurface, NULL);
}

void BaseLight::BuildShadowMap()
{
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(SHADOWMAP_SIZE, SHADOWMAP_SIZE,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_G16R16F, D3DPOOL_DEFAULT,
		&m_pShadowTarget, NULL);
	HRESULT hr = m_pShadowTarget->GetSurfaceLevel(0, &m_pShadowSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateDepthStencilSurface(SHADOWMAP_SIZE, SHADOWMAP_SIZE,
		D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, TRUE,
		&m_pDepthStencilShadowSurface, NULL);
}

void BaseLight::SetShadowTarget()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetDepthStencilSurface(m_pDepthStencilShadowSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pShadowSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 255, 255, 255), 1.0f, 0);

}

LPDIRECT3DTEXTURE9 BaseLight::GetShadowTarget()
{
	return m_pShadowTarget;
}

void BaseLight::SetPointShadowTarget(int index)
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetDepthStencilSurface(m_pDepthStencilShadowSurface);

	LPDIRECT3DSURFACE9 pSurf;
	m_pPointShadowTarget->GetCubeMapSurface((D3DCUBEMAP_FACES)index, 0, &pSurf);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 255, 255, 255), 1.0f, 0);
}

LPDIRECT3DCUBETEXTURE9 BaseLight::GetPointShadowTarget()
{
	return m_pPointShadowTarget;
}

float BaseLight::GetLightRange()
{
	return m_LightRange;
}

void BaseLight::SetLightRange(float range)
{
	m_LightRange = range;

	RebuildProjMatrix();
}

D3DXVECTOR2 BaseLight::GetLightAngle()
{
	return m_LightAngle;
}

D3DXVECTOR2 BaseLight::GetLightCosHalfAngle()
{
	float half_outer = cos(m_LightAngle.x / 360.0f * D3DX_PI);
	float half_inner = cos(m_LightAngle.y / 360.0f * D3DX_PI);

	return D3DXVECTOR2(half_outer, half_inner);
}

void BaseLight::SetLightAngle(D3DXVECTOR2 angle)
{
	m_LightAngle = angle;

	RebuildProjMatrix();
	
}

D3DXVECTOR4 BaseLight::GetLightAttenuation()
{
	return m_LightAttenuation;
}

void BaseLight::SetLightAttenuation(D3DXVECTOR4 lightAttenuation)
{
	m_LightAttenuation = lightAttenuation;
}

