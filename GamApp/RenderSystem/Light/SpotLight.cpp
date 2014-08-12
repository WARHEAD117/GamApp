#include "SpotLight.h"
#include "D3D9Device.h"

SpotLight::SpotLight()
{
	BuildLightVolume();
	RebuildViewMatrix();
	RebuildProjMatrix();
}


SpotLight::~SpotLight()
{
}

void SpotLight::BuildLightVolume()
{
	SafeRelease(m_pBufferVex);
	SafeRelease(m_pBufferIndex);

	//灯光实际范围应该是灯光体的内切球，根据1/cos(PI/(经线数量+1))来计算半径放大的比例，加0.2来防止误差（这里有疑问）
	int sphereSlices = m_SegmentCount;
	float factor = 1.0f / cos(D3DX_PI / (sphereSlices));
	//D3DXCreateSphere(RENDERDEVICE::Instance().g_pD3DDevice, 1 * factor, sphereSlices, sphereSlices, &m_lightVolume, NULL);

	int segmentCount = m_SegmentCount;
	float coneHeight = 1.0f;
	float coneRadius = 1.0f * factor;

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

		indices[2 * 3 * i] = 0;
		indices[2 * 3 * i + 1] = first;
		indices[2 * 3 * i + 2] = second;

		indices[2 * 3 * i + 3] = second;
		indices[2 * 3 * i + 4] = first;
		indices[2 * 3 * i + 5] = segmentCount + 1;
	}

	m_pBufferIndex->Unlock();
}

void SpotLight::RenderLightVolume()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(LIGHTVOLUMEVERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(LIGHTVOLUME_FVF);
	RENDERDEVICE::Instance().g_pD3DDevice->SetIndices(m_pBufferIndex);

	RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_SegmentCount + 2, 0, m_SegmentCount * 2);
	//m_lightVolume->DrawSubset(0);
}


void SpotLight::RebuildViewMatrix()
{
	D3DXMatrixLookAtLH(&m_lightViewMat, &m_LightPos, &(m_LightPos + m_LightDir), &m_LightUp);
}

void SpotLight::RebuildProjMatrix()
{
	D3DXMatrixPerspectiveFovLH(&m_lightProjMat, m_LightAngle.x / 180.0f * D3DX_PI, 1.0f, 0.01f, m_LightRange + 1);

	D3DXMatrixInverse(&m_lightInvProjMat, NULL, &m_lightProjMat);
}

D3DXMATRIX SpotLight::GetLightVolumeTransform()
{
	D3DXMATRIX lightVolumeMatrix;
	D3DXMATRIX scaleMatrix;
	D3DXMatrixScaling(&scaleMatrix, m_LightRange * tanf(m_LightAngle.x / 360.0f * D3DX_PI), m_LightRange, m_LightRange * tanf(m_LightAngle.x / 360.0f * D3DX_PI));
	lightVolumeMatrix = scaleMatrix  * mWorldTransform;
	lightVolumeMatrix = lightVolumeMatrix * RENDERDEVICE::Instance().ViewMatrix * RENDERDEVICE::Instance().ProjMatrix;

	return lightVolumeMatrix;
}

D3DXMATRIX SpotLight::GetToViewDirMatrix()
{
	D3DXMATRIX lightVolumeMatrix;
	D3DXMATRIX scaleMatrix;
	D3DXMatrixScaling(&scaleMatrix, m_LightRange * tanf(m_LightAngle.x / 360.0f * D3DX_PI), m_LightRange, m_LightRange * tanf(m_LightAngle.x / 360.0f * D3DX_PI));
	lightVolumeMatrix = scaleMatrix  * mWorldTransform;
	lightVolumeMatrix = lightVolumeMatrix * RENDERDEVICE::Instance().ViewMatrix;

	return lightVolumeMatrix;
}

void SpotLight::SetUseShadow(bool useShadow)
{
	m_bUseShadow = false;
	if (useShadow)
	{
		m_bUseShadow = useShadow;
		BuildShadowMap();
	}
}