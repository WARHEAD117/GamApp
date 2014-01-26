#include "VertexManager.h"
#include "D3D9Device.h"

#define D3DFVF_VERTEX (D3DFVF_XYZ |D3DFVF_NORMAL|D3DFVF_TEX1| D3DFVF_DIFFUSE)

struct VERTEX
{
	D3DXVECTOR3		position;
	D3DXVECTOR3		normalize;
	FLOAT			tu, tv;
};

VertexManager::VertexManager()
{
}


VertexManager::~VertexManager()
{
}

void VertexManager::BuildVertex(LPDIRECT3DVERTEXBUFFER9 VB)
{
	if (FAILED(RENDERDEVICE::Instance().g_pD3DDevice->CreateVertexBuffer(4 * sizeof(VERTEX)
		, 0
		, D3DFVF_VERTEX
		//,D3DPOOL_DEFAULT
		, D3DPOOL_MANAGED
		, &VB
		, NULL)))
	{
		return;
	}
	VERTEX* pVertices;
	if (FAILED(VB->Lock(0, 4 * sizeof(VERTEX), (void**)&pVertices,
		0)))
	{
		return;
	}

	//初始化天空盒的顶点缓冲区
	//=============================================================下表面
	pVertices->position = D3DXVECTOR3(12.0f, -12.0f, 12.0f);
	pVertices->tu = 1.0f;
	pVertices->tv = 0.0f;
	pVertices++;

	pVertices->position = D3DXVECTOR3(-12.0f, -12.0f, 12.0f);
	pVertices->tu = 0.0f;
	pVertices->tv = 0.0f;
	pVertices++;

	pVertices->position = D3DXVECTOR3(12.0f, -12.0f, -12.0f);
	pVertices->tu = 1.0f;
	pVertices->tv = 1.0f;
	pVertices++;

	pVertices->position = D3DXVECTOR3(-12.0f, -12.0f, -12.0f);
	pVertices->tu = 0.0f;
	pVertices->tv = 1.0f;
	pVertices++;
	
	VB->Unlock();
}
