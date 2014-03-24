#pragma once

#include "CommonUtil/D3D9Header.h"
#include "CommonUtil/GlobalHeader.h"

class VertexManager
{
public:
	VertexManager();
	~VertexManager();

	void BuildAllVertex();
	void GetVertex(int vertexIndex, LPDIRECT3DVERTEXBUFFER9& VB, LPDIRECT3DINDEXBUFFER9& IB);

private:
	LPDIRECT3DVERTEXBUFFER9		mVertexBuffer[1000];
	LPDIRECT3DINDEXBUFFER9		mIndexBuffer[1000];

	void BuildVertex(LPDIRECT3DVERTEXBUFFER9& VB, LPDIRECT3DINDEXBUFFER9& IB);
};

