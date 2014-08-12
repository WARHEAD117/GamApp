#include "PointLight.h"
#include "D3D9Device.h"
#include "RenderUtil/RenderUtil.h"

PointLight::PointLight()
{
	BuildLightVolume();
	RebuildViewMatrix();
	RebuildProjMatrix();
}


PointLight::~PointLight()
{
}

void PointLight::BuildLightVolume()
{
	SafeRelease(m_pBufferVex);
	SafeRelease(m_pBufferIndex);

	//灯光实际范围应该是灯光体的内切球，根据1/cos(PI/(经线数量+1))来计算半径放大的比例，加0.2来防止误差（这里有疑问）
	int sphereSlices = 16;
	float factor = 1.0f / cos(D3DX_PI / (sphereSlices));
	D3DXCreateSphere(RENDERDEVICE::Instance().g_pD3DDevice, 1 * factor, sphereSlices, sphereSlices, &m_lightVolume, NULL);
}

void PointLight::RenderLightVolume()
{
	m_lightVolume->DrawSubset(0);
}


void PointLight::RebuildViewMatrix()
{
	D3DXMatrixLookAtLH(&m_lightViewMat, &m_LightPos, &(m_LightPos + m_LightDir), &m_LightUp);
	
	D3DXMatrixLookAtLH(&m_PointlightViewMat[0], &m_LightPos, &(m_LightPos + D3DXVECTOR3(1, 0, 0)), &D3DXVECTOR3(0, 1, 0)); //+X
	D3DXMatrixLookAtLH(&m_PointlightViewMat[1], &m_LightPos, &(m_LightPos + D3DXVECTOR3(-1, 0, 0)), &D3DXVECTOR3(0, 1, 0)); //-X
	D3DXMatrixLookAtLH(&m_PointlightViewMat[2], &m_LightPos, &(m_LightPos + D3DXVECTOR3(0, 1, 0)), &D3DXVECTOR3(0, 0, -1)); //+Y
	D3DXMatrixLookAtLH(&m_PointlightViewMat[3], &m_LightPos, &(m_LightPos + D3DXVECTOR3(0, -1, 0)), &D3DXVECTOR3(0, 0, 1)); //-Y
	D3DXMatrixLookAtLH(&m_PointlightViewMat[4], &m_LightPos, &(m_LightPos + D3DXVECTOR3(0, 0, 1)), &D3DXVECTOR3(0, 1, 0)); //+Z
	D3DXMatrixLookAtLH(&m_PointlightViewMat[5], &m_LightPos, &(m_LightPos + D3DXVECTOR3(0, 0, -1)), &D3DXVECTOR3(0, 1, 0)); //-Z
}

void PointLight::RebuildProjMatrix()
{
	D3DXMatrixPerspectiveFovLH(&m_lightProjMat, 0.5f * D3DX_PI, 1.0f, 0.01f, m_LightRange + 1);

	D3DXMatrixInverse(&m_lightInvProjMat, NULL, &m_lightProjMat);
}

D3DXMATRIX PointLight::GetLightVolumeTransform()
{
	D3DXMATRIX lightVolumeMatrix;
	D3DXMATRIX scaleMatrix;
	D3DXMatrixScaling(&scaleMatrix, m_LightRange, m_LightRange, m_LightRange);
	lightVolumeMatrix = scaleMatrix  * mWorldTransform;
	lightVolumeMatrix = lightVolumeMatrix * RENDERDEVICE::Instance().ViewMatrix * RENDERDEVICE::Instance().ProjMatrix;

	return lightVolumeMatrix;
}

D3DXMATRIX PointLight::GetToViewDirMatrix()
{
	D3DXMATRIX lightVolumeMatrix;
	D3DXMATRIX scaleMatrix;
	D3DXMatrixScaling(&scaleMatrix, m_LightRange, m_LightRange, m_LightRange);
	lightVolumeMatrix = scaleMatrix  * mWorldTransform;
	lightVolumeMatrix = lightVolumeMatrix * RENDERDEVICE::Instance().ViewMatrix;

	return lightVolumeMatrix;
}

void PointLight::SetUseShadow(bool useShadow)
{
	m_bUseShadow = false;
	if (useShadow)
	{
		m_bUseShadow = useShadow;
		BuildShadowMap();
	}
}

void PointLight::BuildShadowMap()
{
	RENDERDEVICE::Instance().g_pD3DDevice->CreateCubeTexture(m_ShadowMapSize,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_G16R16F, D3DPOOL_DEFAULT,
		&m_pPointShadowTarget, NULL);

	RENDERDEVICE::Instance().g_pD3DDevice->CreateDepthStencilSurface(m_ShadowMapSize, m_ShadowMapSize,
		D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, TRUE,
		&m_pDepthStencilShadowSurface, NULL);
}

LPDIRECT3DBASETEXTURE9 PointLight::GetShadowTarget()
{
	return m_pPointShadowTarget;
}

void PointLight::RenderShadow(const std::vector<RenderUtil*>& mRenderUtilList)
{
	if (!m_bUseShadow)
		return;

	for (int pointDir = 0; pointDir < 6; pointDir++)
	{
		D3DXMATRIX lightViewMat = GetPointLightViewMatrix(pointDir);
		D3DXMATRIX lightProjMat = GetLightProjMatrix();
		SetPointShadowTarget(pointDir);

		for (int i = 0; i < mRenderUtilList.size(); ++i)
		{
			mRenderUtilList[i]->RenderShadow(lightViewMat, lightProjMat, GetLightType());
		}
	}
}


void PointLight::SetPointShadowTarget(int index)
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetDepthStencilSurface(m_pDepthStencilShadowSurface);

	LPDIRECT3DSURFACE9 pSurf;
	m_pPointShadowTarget->GetCubeMapSurface((D3DCUBEMAP_FACES)index, 0, &pSurf);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 255, 255, 255), 1.0f, 0);
}

D3DXMATRIX PointLight::GetPointLightViewMatrix(int index)
{
	return m_PointlightViewMat[index];
}