#include "BaseLight.h"
#include "D3D9Device.h"
 
#include "RenderUtil/RenderUtil.h"

const D3DXVECTOR3 defaultDir(0.0f, -1.0f, 1.0f);

const D3DXVECTOR3 defaultPos(0.0f, 0.0f, 0.0f);
const D3DXVECTOR3 defaultLookAt(0.0f, -1.0f, 0.0f);
const D3DXVECTOR3 defaultUp(1.0f, 0.0f, 0.0f);


BaseLight::BaseLight() :
m_LightType(eDirectionLight),
m_bUseShadow(false),
m_LightRange(5.0f),
m_LightAngle(D3DXVECTOR2(45.0f, 20.0f)),
m_LightDir(ZEROVECTOR3),
m_LightColor(D3DXCOLOR(0.3f,0.3f,0.3f,1.0f)),
m_LightAttenuation(D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f)),
m_SegmentCount(16),
m_ShadowMapSize(2048)
{
	Init();
}

BaseLight::~BaseLight()
{
}

void BaseLight::Init()
{
	m_LightPos = defaultPos;
	m_LightDir = defaultLookAt;
	m_LightUp = defaultUp;
}

void BaseLight::BuildLightVolume()
{
	//empty
}

void BaseLight::RenderLightVolume()
{
	//empty
}

D3DXMATRIX BaseLight::GetLightVolumeTransform()
{
	return RENDERDEVICE::Instance().OrthoWVPMatrix;
}

D3DXMATRIX BaseLight::GetToViewDirMatrix()
{
	return RENDERDEVICE::Instance().InvProjMatrix;
}

D3DXMATRIX BaseLight::GetLightInvProjMatrix()
{
	return m_lightInvProjMat;
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
	float half_outer = cos(m_LightAngle.x / 360.0F * D3DX_PI);
	float half_inner = cos(m_LightAngle.y / 360.0F * D3DX_PI);

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

void BaseLight::SetLightType(LightType lightType)
{
	m_LightType = lightType;
	BuildLightVolume();
}

LightType BaseLight::GetLightType()
{
	return m_LightType;
}


//======================================================================================
//Shadow

void BaseLight::RebuildViewMatrix()
{
	//empty
}

void BaseLight::RebuildProjMatrix()
{
	//empty
}

D3DXMATRIX BaseLight::GetLightViewMatrix()
{
	return m_lightViewMat;
}


D3DXMATRIX BaseLight::GetLightProjMatrix()
{
	return m_lightProjMat;
}

bool BaseLight::GetUseShadow()
{
	return m_bUseShadow;
}

void BaseLight::SetUseShadow(bool useShadow)
{
	m_bUseShadow = false;
}

void BaseLight::BuildShadowMap()
{
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(m_ShadowMapSize, m_ShadowMapSize,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_G32R32F, D3DPOOL_DEFAULT,
		&m_pShadowTarget, NULL);
	HRESULT hr = m_pShadowTarget->GetSurfaceLevel(0, &m_pShadowSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateDepthStencilSurface(m_ShadowMapSize, m_ShadowMapSize,
		D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, TRUE,
		&m_pDepthStencilShadowSurface, NULL);
}

void BaseLight::SetShadowTarget()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetDepthStencilSurface(m_pDepthStencilShadowSurface);

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, m_pShadowSurface);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 255, 255, 255), 1.0f, 0);

}

void BaseLight::RenderShadow(const std::vector<RenderUtil*>& mRenderUtilList, const std::vector<SkinnedMesh*>* mSkinnedMeshList)
{
	if (!m_bUseShadow)
		return;

	D3DXMATRIX lightViewMat = GetLightViewMatrix();
	D3DXMATRIX lightProjMat = GetLightProjMatrix();
	SetShadowTarget();

	for (int i = 0; i < mRenderUtilList.size(); ++i)
	{
		mRenderUtilList[i]->RenderShadow(m_lightViewMat, m_lightProjMat, GetLightType());
	}

	if (mSkinnedMeshList)
	{
		for (int i = 0; i < mSkinnedMeshList->size(); ++i)
		{
			(*mSkinnedMeshList)[i]->RenderShadow(m_lightViewMat, m_lightProjMat, GetLightType());
		}
	}
}

LPDIRECT3DBASETEXTURE9 BaseLight::GetShadowTarget()
{
	return m_pShadowTarget;
}

int BaseLight::GetShadowMapSize()
{
	return m_ShadowMapSize;
}

void BaseLight::SetShadowMapSize(int size)
{
	if (size <= 0)
		return;

	m_ShadowMapSize = size;
}
