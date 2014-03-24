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

void VertexManager::BuildAllVertex()
{
	for (int i = 0; i < 1000; i++)
	{
		BuildVertex(mVertexBuffer[i], mIndexBuffer[i]);
	}
}

void VertexManager::BuildVertex(LPDIRECT3DVERTEXBUFFER9& VB, LPDIRECT3DINDEXBUFFER9& IB)
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

	//初始化顶点缓冲区
	//=============================================================
	

	pVertices->position = D3DXVECTOR3(-1.0f, 0.0f, 1.0f);
	pVertices->tu = 0.0f;
	pVertices->tv = 0.0f;
	pVertices++;

	pVertices->position = D3DXVECTOR3(1.0f, 0.0f, 1.0f);
	pVertices->tu = 1.0f;
	pVertices->tv = 0.0f;
	pVertices++;

	pVertices->position = D3DXVECTOR3(-1.0f, 0.0f, -1.0f);
	pVertices->tu = 0.0f;
	pVertices->tv = 1.0f;
	pVertices++;

	pVertices->position = D3DXVECTOR3(1.0f, 0.0f, -1.0f);
	pVertices->tu = 1.0f;
	pVertices->tv = 1.0f;
	pVertices++;

	
	
	VB->Unlock();

	//================================================================================
	HRESULT(RENDERDEVICE::Instance().g_pD3DDevice->CreateIndexBuffer(6 * sizeof(DWORD),
		D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_MANAGED, &IB, 0));

	DWORD* k = 0;
	IB->Lock(0, 0, (void**)&k, 0);

	// Compute the indices for each triangle.

		k[0] = 0;
		k[1] = 1;
		k[2] = 2;

		k[3] = 1;
		k[4] = 3;
		k[5] = 2;

	IB->Unlock();
}

void VertexManager::GetVertex(int vertexIndex, LPDIRECT3DVERTEXBUFFER9& VB, LPDIRECT3DINDEXBUFFER9& IB)
{
	VB = mVertexBuffer[vertexIndex];
	IB = mIndexBuffer[vertexIndex];
}
