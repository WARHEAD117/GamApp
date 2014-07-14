#include "BaseLight.h"
#include "D3D9Device.h"

const D3DXVECTOR3 defaultDir(0.0f, -1.0f, 1.0f);

const D3DXVECTOR3 defaultPos(0.0f, 0.0f, 0.0f);
const D3DXVECTOR3 defaultLookAt(0.0f, -1.0f, 0.0f);
const D3DXVECTOR3 defaultUp(1.0f, 0.0f, 0.0f);
const int SHADOWMAP_SIZE = 1024;

LPD3DXMESH m_lightVolume;

BaseLight::BaseLight() :
m_LightType(eDirectionLight),
m_bUseShadow(false),
m_LightRange(5.0f),
m_LightAngle(D3DXVECTOR2(45.0f, 20.0f)),
m_LightDir(ZEROVECTOR3),
m_LightColor(D3DXCOLOR(0.3,0.3f,0.3f,1.0f)),
m_LightAttenuation(D3DXVECTOR4(0.0, 1.0f, 0.0f, 1.0f)),
m_SegmentCount(16)
{
	BuildLightVolume();
	Init();
}

void BaseLight::BuildLightVolume()
{
	SafeRelease(m_pBufferVex);
	SafeRelease(m_pBufferIndex);

	
	
	if (m_LightType == eDirectionLight)
	{
		//=======================================================================
		RENDERDEVICE::Instance().g_pD3DDevice->CreateVertexBuffer(4 * sizeof(LIGHTVOLUMEVERTEX)
			, 0
			, LIGHTVOLUME_FVF
			//,D3DPOOL_DEFAULT
			, D3DPOOL_MANAGED
			, &m_pBufferVex
			, NULL);
		RENDERDEVICE::Instance().g_pD3DDevice->CreateIndexBuffer(2 * 3 * sizeof(DWORD),
			D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_MANAGED, &m_pBufferIndex, 0);
		DWORD* indices = 0;
		m_pBufferIndex->Lock(0, 0, (void**)&indices, 0);
		//全部逆时针绘制，在延迟渲染时剔除正面，就可以保证灯光和渲染面的剔除是统一的
		indices[0] = 0;
		indices[1] = 2;
		indices[2] = 1;

		indices[3] = 1;
		indices[4] = 2;
		indices[5] = 3;
		m_pBufferIndex->Unlock();

		LIGHTVOLUMEVERTEX* pVertices;
		m_pBufferVex->Lock(0, 4 * sizeof(LIGHTVOLUMEVERTEX), (void**)&pVertices, 0);

		//初始化顶点缓冲区
		//==========================
		pVertices->position = D3DXVECTOR3(1.0f, -1.0f, 0.0f);
		pVertices++;

		pVertices->position = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);
		pVertices++;

		pVertices->position = D3DXVECTOR3(1.0f, 1.0f, 0.0f);
		pVertices++;

		pVertices->position = D3DXVECTOR3(-1.0f, 1.0f, 0.0f);
		pVertices++;

		m_pBufferVex->Unlock();
	}
	else if (m_LightType == ePointLight)
	{
		//灯光实际范围应该是灯光体的内切球，根据1/cos(PI/(经线数量+1))来计算半径放大的比例，加0.2来防止误差（这里有疑问）
		int sphereSlices = 16;
		float factor = 1.0f / cos(D3DX_PI / (sphereSlices));
		D3DXCreateSphere(RENDERDEVICE::Instance().g_pD3DDevice, 1 * factor, sphereSlices, sphereSlices, &m_lightVolume, NULL);
	}
	else if (m_LightType == eSpotLight)
	{
		//灯光实际范围应该是灯光体的内切球，根据1/cos(PI/(经线数量+1))来计算半径放大的比例，加0.2来防止误差（这里有疑问）
		int sphereSlices = m_SegmentCount;
		float factor = 1.0f / cos(D3DX_PI / (sphereSlices));
		//D3DXCreateSphere(RENDERDEVICE::Instance().g_pD3DDevice, 1 * factor, sphereSlices, sphereSlices, &m_lightVolume, NULL);

		int segmentCount = m_SegmentCount;
		float coneHeight = 1.0f;
		float coneRadius = 1.0 * factor;

		float deltaAngle = 2.0f * D3DX_PI / segmentCount;

		//=======================================================================
		RENDERDEVICE::Instance().g_pD3DDevice->CreateVertexBuffer((segmentCount + 2 + 1) * sizeof(LIGHTVOLUMEVERTEX)
			, 0
			, LIGHTVOLUME_FVF
			//,D3DPOOL_DEFAULT
			, D3DPOOL_MANAGED
			, &m_pBufferVex
			, NULL);
		RENDERDEVICE::Instance().g_pD3DDevice->CreateIndexBuffer((2 * 3 * segmentCount) * sizeof(DWORD),
			D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_MANAGED, &m_pBufferIndex, 0);
		DWORD* indices = 0;
		m_pBufferIndex->Lock(0, 0, (void**)&indices, 0);
		//全部逆时针绘制，在延迟渲染时剔除正面，就可以保证灯光和渲染面的剔除是统一的

		LIGHTVOLUMEVERTEX* pVertices;
		m_pBufferVex->Lock(0, 4 * sizeof(LIGHTVOLUMEVERTEX), (void**)&pVertices, 0);

		pVertices->position = D3DXVECTOR3(0, 0, 0);
		pVertices++;
		//初始化顶点缓冲区
		//==========================
		for (int i = 0; i < segmentCount; i++)
		{
			float x = coneRadius * sin(i * deltaAngle);
			float z = coneRadius * cos(i * deltaAngle);

			pVertices->position = D3DXVECTOR3(x, -coneHeight, z);
			pVertices++;
		}

		pVertices->position = D3DXVECTOR3(0, -coneHeight, 0);
		pVertices++;

		m_pBufferVex->Unlock();

		for (int i = 0; i < segmentCount; i++)
		{
			int first = i + 1;
			int second = i + 2;
			if (i == segmentCount - 1)
			{
				second = 1;
			}

			indices[2 * 3 * i]		= 0;
			indices[2 * 3 * i + 1] = first;
			indices[2 * 3 * i + 2] = second;

			indices[2 * 3 * i + 3] = second;
			indices[2 * 3 * i + 4] = first;
			indices[2 * 3 * i + 5] = segmentCount + 1;
		}

		m_pBufferIndex->Unlock();
	}
	else
	{
	}
}

void BaseLight::RenderLightVolume()
{
	switch (m_LightType)
	{
	case eDirectionLight:
		RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(LIGHTVOLUMEVERTEX));
		RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(LIGHTVOLUME_FVF);
		RENDERDEVICE::Instance().g_pD3DDevice->SetIndices(m_pBufferIndex);

		RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
		break;
	case ePointLight:
		m_lightVolume->DrawSubset(0);
		break;
	case eSpotLight:
		RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(LIGHTVOLUMEVERTEX));
		RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(LIGHTVOLUME_FVF);
		RENDERDEVICE::Instance().g_pD3DDevice->SetIndices(m_pBufferIndex);

		RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_SegmentCount + 2, 0, m_SegmentCount * 2);
		//m_lightVolume->DrawSubset(0);
		break;
	default:
		m_lightVolume->DrawSubset(0);
		break;
	}
	
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


bool BaseLight::GetUseShadow()
{
	return m_bUseShadow;
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
		D3DXMatrixPerspectiveFovLH(&m_lightProjMat, 0.5f * D3DX_PI, 1.0f, 0.01, m_LightRange + 1);
	}
	else if (m_LightType == eSpotLight)
	{
		D3DXMatrixPerspectiveFovLH(&m_lightProjMat, m_LightAngle.x / 180.0f * D3DX_PI, 1.0f, 0.01, m_LightRange + 1);
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


D3DXMATRIX BaseLight::GetLightVolumeTransform()
{
	D3DXMATRIX lightVolumeMatrix;
	D3DXMATRIX scaleMatrix;
	D3DXMatrixScaling(&scaleMatrix, m_LightRange, m_LightRange, m_LightRange);
	lightVolumeMatrix = scaleMatrix  * mWorldTransform;
	lightVolumeMatrix = lightVolumeMatrix * RENDERDEVICE::Instance().ViewMatrix * RENDERDEVICE::Instance().ProjMatrix;

	switch (m_LightType)
	{
	case eDirectionLight:
		return RENDERDEVICE::Instance().OrthoWVPMatrix;
		break;
	case ePointLight:
		return lightVolumeMatrix;
		break;
	case eSpotLight:

		D3DXMatrixScaling(&scaleMatrix, m_LightRange * tanf(m_LightAngle.x / 360.0f * D3DX_PI), m_LightRange, m_LightRange * tanf(m_LightAngle.x / 360.0f * D3DX_PI));
		lightVolumeMatrix = scaleMatrix  * mWorldTransform;
		lightVolumeMatrix = lightVolumeMatrix * RENDERDEVICE::Instance().ViewMatrix * RENDERDEVICE::Instance().ProjMatrix;

		return lightVolumeMatrix;
		break;
	default:
		return RENDERDEVICE::Instance().OrthoWVPMatrix;
		break;
	}
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

LPDIRECT3DBASETEXTURE9 BaseLight::GetShadowTarget()
{
	switch (m_LightType)
	{
	case eDirectionLight:
		return m_pShadowTarget;
		break;
	case ePointLight:
		return m_pPointShadowTarget;
		break;
	case eSpotLight:
		return m_pShadowTarget;
		break;
	default:
		return m_pShadowTarget;
		break;
	}
}

void BaseLight::SetPointShadowTarget(int index)
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetDepthStencilSurface(m_pDepthStencilShadowSurface);

	LPDIRECT3DSURFACE9 pSurf;
	m_pPointShadowTarget->GetCubeMapSurface((D3DCUBEMAP_FACES)index, 0, &pSurf);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderTarget(0, pSurf);
	RENDERDEVICE::Instance().g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 255, 255, 255), 1.0f, 0);
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

