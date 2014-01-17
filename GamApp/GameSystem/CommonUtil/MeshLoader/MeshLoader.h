#pragma once
#include "CommonUtil\\D3D9Header.h"
#include "D3D9Device.h"
#include "RenderUtil/RenderUtil.h"

struct BoundingSphereInfo 
{
	D3DXVECTOR3	center;
	float		radius;

	BoundingSphereInfo()
	{
		radius = 0.0f;
		center = ZEROVECTOR3;
	}
};

struct BoundingBoxInfo
{
	D3DXVECTOR3	min;
	D3DXVECTOR3	max;

	BoundingBoxInfo()
	{
		max = ZEROVECTOR3;
		min = ZEROVECTOR3;
	}
};
class MeshLoader
{
public:
	MeshLoader();
	~MeshLoader();

public:
	HRESULT		LoadXMesh(std::string filePath);
	LPD3DXMESH&	GetD3DMesh();

	void						GetVertexBuffer(LPDIRECT3DVERTEXBUFFER9& vertexBuffer);
	void						GetIndexBuffer(LPDIRECT3DINDEXBUFFER9& indexBuffer);
	const std::vector<SubMesh>& GetSubMeshList();
	DWORD						GetVertexFVF();

	int							GetSubMeshCount();

	void		ComputeBoundingBox();
	void		ComputeBoundingSphere();

	BoundingSphereInfo	GetBoundingSphereInfo();
	BoundingBoxInfo		GetBoundingBoxInfo();

private:
	LPD3DXMESH						m_pMesh;
	std::vector<LPDIRECT3DTEXTURE9>	m_pTextures;
	std::vector<D3DMATERIAL9>		m_Materials; //材质
	DWORD							m_dwMtrlNum; //材质数量

	std::vector<D3DXATTRIBUTERANGE>	m_AttTable;

	LPDIRECT3DVERTEXBUFFER9			mVertexBuffer;
	LPDIRECT3DINDEXBUFFER9			mIndexBuffer;

	std::vector<SubMesh>	mSubMeshList;

	BoundingSphereInfo	mBoundingSphereInfo;
	BoundingBoxInfo		mBoundingBoxInfo;
};

inline int MeshLoader::GetSubMeshCount()
{
	return mSubMeshList.size();
}

inline BoundingSphereInfo MeshLoader::GetBoundingSphereInfo()
{
	return mBoundingSphereInfo;
}

inline BoundingBoxInfo MeshLoader::GetBoundingBoxInfo()
{
	return mBoundingBoxInfo;
}

inline const std::vector<SubMesh>& MeshLoader::GetSubMeshList()
{
	return mSubMeshList;
}

inline void MeshLoader::GetVertexBuffer(LPDIRECT3DVERTEXBUFFER9 &vertexBuffer)
{
	vertexBuffer = mVertexBuffer;
}

inline void MeshLoader::GetIndexBuffer(LPDIRECT3DINDEXBUFFER9 &indexBuffer)
{
	indexBuffer = mIndexBuffer;
}

inline DWORD MeshLoader::GetVertexFVF()
{
	if (m_pMesh)
		return m_pMesh->GetFVF();
	else
		return NULL;
}