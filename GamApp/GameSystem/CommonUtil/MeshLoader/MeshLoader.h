#pragma once
#include "CommonUtil\\D3D9Header.h"
#include "D3D9Device.h"

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
	HRESULT		LoadXMesh(char* filePath);
	LPD3DXMESH	GetD3DMesh();

	void		ComputeBoundingBox();
	void		ComputeBoundingSphere();

	BoundingSphereInfo	GetBoundingSphereInfo();
	BoundingBoxInfo		GetBoundingBoxInfo();

private:
	LPD3DXMESH						m_pMesh;
	std::vector<LPDIRECT3DTEXTURE9>	m_pTextures;
	std::vector<D3DMATERIAL9>		m_Materials; //材质
	DWORD							m_dwMtrlNum; //材质数量

	BoundingSphereInfo	mBoundingSphereInfo;
	BoundingBoxInfo		mBoundingBoxInfo;
};

inline BoundingSphereInfo MeshLoader::GetBoundingSphereInfo()
{
	return mBoundingSphereInfo;
}

inline BoundingBoxInfo MeshLoader::GetBoundingBoxInfo()
{
	return mBoundingBoxInfo;
}