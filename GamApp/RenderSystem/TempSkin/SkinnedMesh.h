//=============================================================================
// SkinnedMesh.h by Frank Luna (C) 2005 All Rights Reserved.
//=============================================================================

#ifndef SKINNED_MESH_H
#define SKINNED_MESH_H

#include "d3dUtil.h"

class Entity;

struct FrameEx : public D3DXFRAME
{
	D3DXMATRIX toRoot;
};

class SkinnedMesh
{
public:
	SkinnedMesh(std::string XFilename);
	~SkinnedMesh();

	UINT numVertices();
	UINT numTriangles();
	UINT numBones();
	const D3DXMATRIX* getFinalXFormArray();

	void update(float deltaTime);
	void draw();
	//---------------------------
	void RenderDeferredGeometry(ID3DXEffect* pEffect);
	void RenderShadow(D3DXMATRIX lightViewMat, D3DXMATRIX lightProjMat, LightType lightType);

	void SetMaterial(Material* material);

protected:
	D3DXFRAME* findNodeWithMesh(D3DXFRAME* frame);
	bool hasNormals(ID3DXMesh* mesh);
	void buildSkinnedMesh(ID3DXMesh* mesh);
	void buildToRootXFormPtrArray();
	void buildToRootXForms(FrameEx* frame, D3DXMATRIX& parentsToRoot);

	// We do not implement the required functionality to do deep copies,
	// so restrict copying.
	SkinnedMesh(const SkinnedMesh& rhs);
	SkinnedMesh& operator=(const SkinnedMesh& rhs);

protected:
	ID3DXMesh*     mSkinnedMesh;

	Material pMaterial;
	LPDIRECT3DTEXTURE9 m_pTexture;
	LPDIRECT3DTEXTURE9 m_pNormalMap;
	LPDIRECT3DTEXTURE9 m_pSpecularMap;

	D3DXFRAME*     mRoot;
	DWORD          mMaxVertInfluences;
	DWORD          mNumBones;
	ID3DXSkinInfo* mSkinInfo;
	ID3DXAnimationController* mAnimCtrl;  
	
	std::vector<D3DXMATRIX>  mFinalXForms;
	std::vector<D3DXMATRIX*> mToRootXFormPtrs;

	static const int MAX_NUM_BONES_SUPPORTED = 55; 

protected:
	D3DXMATRIX					mWorldMat;
	D3DXMATRIX					mViewMat;
	D3DXMATRIX					mProjMat;
	D3DXMATRIX					mWorldView;
	D3DXMATRIX					mViewProj;
	D3DXMATRIX					mWorldViewProj;

	D3DXMATRIX	mWorldTransform;
public:
	D3DXMATRIX	GetWorldTransform();
	void		SetWorldTransform(D3DXMATRIX matrix);
	void		SetTexture(std::string fileName);
	void		SetNormalMap(std::string fileName);
	void		SetSpecularMap(std::string fileName);

	void	SetOwner(Entity* owner);

private:
	Entity*						mOwner;

protected:
	void BuildShadowEffectInfo(D3DXMATRIX lightViewMat, D3DXMATRIX lightProjMat);
};

inline D3DXMATRIX SkinnedMesh::GetWorldTransform()
{
	return mWorldTransform;
}

inline void SkinnedMesh::SetWorldTransform(D3DXMATRIX matrix)
{
	mWorldTransform = matrix;
}

inline void SkinnedMesh::SetMaterial(Material* material)
{
	pMaterial = *material;
}

#endif // SKINNED_MESH_H