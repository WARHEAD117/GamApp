#include "DirectionLight.h"
#include "D3D9Device.h"

const D3DXVECTOR3 defaultDir(0.0f, -1.0f, 1.0f);
DirectionLight::DirectionLight()
{
	BuildLightVolume();
	RebuildViewMatrix();
	RebuildProjMatrix();
}


DirectionLight::~DirectionLight()
{
}

void DirectionLight::BuildLightVolume()
{
	SafeRelease(m_pBufferVex);
	SafeRelease(m_pBufferIndex);

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

void DirectionLight::RenderLightVolume()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(LIGHTVOLUMEVERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(LIGHTVOLUME_FVF);
	RENDERDEVICE::Instance().g_pD3DDevice->SetIndices(m_pBufferIndex);

	RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
}


void DirectionLight::RebuildViewMatrix()
{
	D3DXMatrixLookAtLH(&m_lightViewMat, &m_LightPos, &(m_LightPos + m_LightDir), &m_LightUp);
}

void DirectionLight::RebuildProjMatrix()
{
	D3DXMatrixOrthoLH(&m_lightProjMat, 40, 40, 0.01f, 50.0f);

	D3DXMatrixInverse(&m_lightInvProjMat, NULL, &m_lightProjMat);
}

D3DXMATRIX DirectionLight::GetLightVolumeTransform()
{
	return RENDERDEVICE::Instance().OrthoWVPMatrix;
}

D3DXMATRIX DirectionLight::GetToViewDirMatrix()
{

	return RENDERDEVICE::Instance().InvProjMatrix;
}

void DirectionLight::SetUseShadow(bool useShadow)
{
	m_bUseShadow = false;
	if (useShadow)
	{
		m_bUseShadow = useShadow;
		BuildShadowMap();
	}
}