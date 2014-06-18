#include "BaseLight.h"
#include "D3D9Device.h"

const D3DXVECTOR3 defaultDir(0.0f, -1.0f, 1.0f);

const D3DXVECTOR3 defaultPos(0.0f, 0.0f, 0.0f);
const D3DXVECTOR3 defaultLookAt(0.0f, -1.0f, 0.0f);
const D3DXVECTOR3 defaultUp(1.0f, 0.0f, 0.0f);
const int SHADOWMAP_SIZE = 1024;
BaseLight::BaseLight()
{
	m_LightDir = ZEROVECTOR3;

	D3DXMatrixLookAtLH(&m_lightViewMat, &defaultPos, &defaultLookAt, &defaultUp);
	D3DXMatrixInverse(&mWorldTransform, NULL, &m_lightViewMat);

	D3DXMatrixOrthoLH(&m_lightProjMat, 20, 20, 0.01f, 50.0f);
	D3DXMatrixInverse(&m_lightInvProjMat, NULL, &m_lightProjMat);
}


BaseLight::~BaseLight()
{
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

	D3DXMatrixLookAtLH(&m_lightViewMat, &m_LightPos, &m_LightDir, &m_LightUp);
}

D3DXVECTOR3 BaseLight::GetLightDir()
{
	return m_LightDir;
}

D3DXVECTOR3 BaseLight::GetLightUp()
{
	return m_LightUp;
}

D3DXCOLOR BaseLight::GetLightColor()
{
	return m_LightColor;
}

void BaseLight::SetLightColor(D3DXCOLOR color)
{
	m_LightColor = color;
}

D3DXVECTOR3 BaseLight::GetLightPos()
{
	return m_LightPos;
}

D3DXMATRIX BaseLight::GetLightViewMatrix()
{
	return m_lightViewMat;
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
	mUseShadow = useShadow;
	if (useShadow)
		BuildShadowMap();
}

void BaseLight::BuildShadowMap()
{
	RENDERDEVICE::Instance().g_pD3DDevice->CreateTexture(SHADOWMAP_SIZE, SHADOWMAP_SIZE,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT,
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
