#include "SkyBox.h"
#include "D3D9Device.h"
#include "RenderUtil/EffectParam.h"
#include "RenderSystem/RenderPipeLine/RenderPipe.h"

SkyBox::SkyBox()
{
}

SkyBox::~SkyBox()
{
	if (m_pBufferVex)
	{
		SafeRelease(m_pBufferVex);
		m_pBufferVex = NULL;
	}
	if (*m_pSkyTextures)
	{
		SafeRelease(*m_pSkyTextures);
		*m_pSkyTextures = NULL;
	}
}

void SkyBox::BuildSkyBox()
{
	RENDERDEVICE::Instance().g_pD3DDevice->CreateVertexDeclaration(COMMONVERTEXDECL, &mVertexDecl);
	mVertexByteSize = D3DXGetDeclVertexSize(COMMONVERTEXDECL, 0);

	if (FAILED(RENDERDEVICE::Instance().g_pD3DDevice->CreateVertexBuffer(32 * sizeof(COMMONVERTEX)
		, 0
		, D3DFVF_VERTEX
		//,D3DPOOL_DEFAULT
		, D3DPOOL_MANAGED
		, &m_pBufferVex
		, NULL)))
	{
		return;
	}
	COMMONVERTEX* pVertices;
	if (FAILED(m_pBufferVex->Lock(0, 32 * sizeof(COMMONVERTEX), (void**)&pVertices,
		0)))
	{
		return;
	}

	//初始化天空盒的顶点缓冲区
	//=============================================================下表面
	pVertices->position = D3DXVECTOR3(12.0f, -12.0f, 12.0f);
	pVertices->uv = D3DXVECTOR2(1.0f, 0.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(-12.0f, -12.0f, 12.0f);
	pVertices->uv = D3DXVECTOR2(0.0f, 0.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(12.0f, -12.0f, -12.0f);
	pVertices->uv = D3DXVECTOR2(1.0f, 1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(-12.0f, -12.0f, -12.0f);
	pVertices->uv = D3DXVECTOR2(0.0f, 1.0f);
	pVertices++;


	//==========================================================================左表面 
	pVertices->position = D3DXVECTOR3(-12.0f, 12.0f, 12.0f);
	pVertices->uv = D3DXVECTOR2(1.0f, 0.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(-12.0f, 12.0f, -12.0f);
	pVertices->uv = D3DXVECTOR2(0.0f, 0.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(-12.0f, -12.0f, 12.0f);
	pVertices->uv = D3DXVECTOR2(1.0f, 1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(-12.0f, -12.0f, -12.0f);
	pVertices->uv = D3DXVECTOR2(0.0f, 1.0f);
	pVertices++;



	//==========================================================================右表面
	pVertices->position = D3DXVECTOR3(12.0f, 12.0f, -12.0f);
	pVertices->uv = D3DXVECTOR2(1.0f, 0.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(12.0f, 12.0f, 12.0f);
	pVertices->uv = D3DXVECTOR2(0.0f, 0.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(12.0f, -12.0f, -12.0f);
	pVertices->uv = D3DXVECTOR2(1.0f, 1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(12.0f, -12.0f, 12.0f);
	pVertices->uv = D3DXVECTOR2(0.0f, 1.0f);
	pVertices++;

	// 	//==========================================================================上表面
	pVertices->position = D3DXVECTOR3(-12.0f, 12.0f, 12.0f);
	pVertices->uv = D3DXVECTOR2(0.0f, 1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(12.0f, 12.0f, 12.0f);
	pVertices->uv = D3DXVECTOR2(1.0f, 1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(-12.0f, 12.0f, -12.0f);
	pVertices->uv = D3DXVECTOR2(0.0f, 0.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(12.0f, 12.0f, -12.0f);
	pVertices->uv = D3DXVECTOR2(1.0f, 0.0f);
	pVertices++;

	// 
	// 	//==========================================================================后表面
	pVertices->position = D3DXVECTOR3(-12.0f, -12.0f, 12.0f);
	pVertices->uv = D3DXVECTOR2(0.0f, 1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(12.0f, -12.0f, 12.0f);
	pVertices->uv = D3DXVECTOR2(1.0f, 1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(-12.0f, 12.0f, 12.0f);
	pVertices->uv = D3DXVECTOR2(0.0f, 0.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(12.0f, 12.0f, 12.0f);
	pVertices->uv = D3DXVECTOR2(1.0f, 0.0f);
	pVertices++;
	// 
	// 	//==========================================================================前表面
	pVertices->position = D3DXVECTOR3(12.0f, -12.0f, -12.0f);
	pVertices->uv = D3DXVECTOR2(0.0f, 1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(-12.0f, -12.0f, -12.0f);
	pVertices->uv = D3DXVECTOR2(1.0f, 1.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(12.0f, 12.0f, -12.0f);
	pVertices->uv = D3DXVECTOR2(0.0f, 0.0f);
	pVertices++;

	pVertices->position = D3DXVECTOR3(-12.0f, 12.0f, -12.0f);
	pVertices->uv = D3DXVECTOR2(1.0f, 0.0f);
	pVertices++;


	m_pBufferVex->Unlock();
}

void SkyBox::SetSkyTexture(const std::string filename, int flag)
{
	if (FAILED(D3DXCreateTextureFromFile(RENDERDEVICE::Instance().g_pD3DDevice, filename.c_str(),
		&m_pSkyTextures[flag])))
	{
		m_pSkyTextures[flag] = RENDERDEVICE::Instance().GetDefaultTexture();
	}

}

void SkyBox::RenderInGBuffer(ID3DXEffect* pEffect)
{
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	D3DXVECTOR3 cameraPos = RENDERDEVICE::Instance().ViewPosition;
	D3DXMATRIX worldMat;
	D3DXMatrixTranslation(&worldMat, cameraPos.x, cameraPos.y, cameraPos.z);
	D3DXMATRIX viewMat = RENDERDEVICE::Instance().ViewMatrix;
	D3DXMATRIX projMat = RENDERDEVICE::Instance().ProjMatrix;

	D3DXMATRIX worldView = worldMat * viewMat;
	D3DXMATRIX viewProj = viewMat * projMat;
	D3DXMATRIX worldViewProj = worldMat * viewProj;

	pEffect->SetBool("g_IsSky", true);

	pEffect->SetMatrix(WORLDVIEWPROJMATRIX, &worldViewProj);
	pEffect->SetMatrix(WORLDVIEWMATRIX, &worldView);
	pEffect->SetMatrix(WORLDMATRIX, &worldMat);
	pEffect->SetMatrix(VIEWMATRIX, &viewMat);

	//渲染6个面==========================================================================下左右上后前
	for (int i = 0; i < 6; i++)
	{
		pEffect->SetTexture(DIFFUSETEXTURE, m_pSkyTextures[i]);
		pEffect->SetTexture(NORMALMAP, RENDERDEVICE::Instance().GetDefaultNormalMap());
		pEffect->SetTexture(SPECULARMAP, RENDERDEVICE::Instance().GetDefaultTexture());
		pEffect->CommitChanges();

		RENDERDEVICE::Instance().g_pD3DDevice->SetStreamSource(0, m_pBufferVex, 4 * i*mVertexByteSize, mVertexByteSize);
		//RENDERDEVICE::Instance().g_pD3DDevice->SetFVF(mFVF);
		RENDERDEVICE::Instance().g_pD3DDevice->SetVertexDeclaration(mVertexDecl);
		RENDERDEVICE::Instance().g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	}

	pEffect->SetBool("g_IsSky", false);
	pEffect->CommitChanges();

	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	RENDERDEVICE::Instance().g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	RENDERDEVICE::Instance().g_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
}
