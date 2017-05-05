#include "ImageBasedLight.h"
#include "D3D9Device.h"
 
const D3DXVECTOR3 defaultDir(0.0f, -1.0f, 1.0f);
ImageBasedLight::ImageBasedLight() :
m_shadowAreaSize(D3DXVECTOR2(100,100))
{
	m_useCubemap = false;
	BuildLightVolume();
	RebuildViewMatrix();
	RebuildProjMatrix();
}


ImageBasedLight::~ImageBasedLight()
{
}

void ImageBasedLight::BuildLightVolume()
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

void ImageBasedLight::RenderLightVolume()
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 0, sizeof(LIGHTVOLUMEVERTEX));
	RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(LIGHTVOLUME_FVF);
	RENDERDEVICE::Instance().g_pD3DDevice->SetIndices(m_pBufferIndex);

	RENDERDEVICE::Instance().g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
}


void ImageBasedLight::RebuildViewMatrix()
{
	D3DXMatrixLookAtLH(&m_lightViewMat, &m_LightPos, &(m_LightPos + m_LightDir), &m_LightUp);
}

void ImageBasedLight::RebuildProjMatrix()
{
	D3DXMatrixOrthoLH(&m_lightProjMat, m_shadowAreaSize.x, m_shadowAreaSize.y, 0.01f, 100.0f);

	D3DXMatrixInverse(&m_lightInvProjMat, NULL, &m_lightProjMat);
}

D3DXMATRIX ImageBasedLight::GetLightVolumeTransform()
{
	return RENDERDEVICE::Instance().OrthoWVPMatrix;
}

D3DXMATRIX ImageBasedLight::GetToViewDirMatrix()
{

	return RENDERDEVICE::Instance().InvProjMatrix;
}

void ImageBasedLight::SetLightProbe(const std::string probeTex, bool useCube)
{
	if (!useCube)
	{
		if (E_FAIL == D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, probeTex.c_str(), &m_pLightProbeTex))//daytime//uffizi-large
		{
			MessageBox(GetForegroundWindow(), "TextureError", "m_pLightProbeTex", MB_OK);
			abort();
		}
		m_useCubemap = false;
	}
	else
	{
		if (E_FAIL == D3DXCreateCubeTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, probeTex.c_str(), &m_pLightProbeCube))//daytime//uffizi-large//env.dds
		{
			MessageBox(GetForegroundWindow(), "CubeError", "Sky", MB_OK);
			abort();
		}
		m_useCubemap = true;
	}
}

void ImageBasedLight::SetLightEffect(ID3DXEffect* pEffect)
{
	if (!m_useCubemap)
		pEffect->SetTexture("g_Sky", m_pLightProbeTex);
	else
		pEffect->SetTexture("g_SkyCube", m_pLightProbeCube);
	D3DXMATRIX invV;
	invV = RENDERDEVICE::Instance().InvViewMatrix;
	pEffect->SetMatrix("g_invView", &invV);
	pEffect->SetBool("g_useCubemap", m_useCubemap);
}