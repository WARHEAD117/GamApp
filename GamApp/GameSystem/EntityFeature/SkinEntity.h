#pragma once
#include "CommonUtil\GlobalHeader.h"
#include "CommonUtil\D3D9Header.h"
#include "RenderSystem\TempSkin\SkinnedMesh.h"
#include "RenderUtil\RenderUtil.h"
#include "CommonUtil/Timer/GlobalTimer.h"

#include "Entity.h"

class SkinEntity :
	public Entity
{
public:
	SkinEntity();
	~SkinEntity();


protected:
	
public:
	virtual unsigned int GetIndex();

public:
	virtual void OnBeginFrame();
	virtual void OnFrame();
	virtual void OnEndFrame();

	virtual void SetMeshFileName(std::string fileName);
	virtual void SetTexture(std::string fileName, int subMeshIndex = -1);
	virtual void SetNormalMap(std::string fileName, int subMeshIndex = -1);
	virtual void SetSpecularMap(std::string fileName, int subMeshIndex = -1);

	virtual void SetMaterial(Material* material, int subMeshIndex = -1);

	virtual void AssignRenderUtil();
	virtual void BuildRenderUtil();

protected:
	RenderUtil		mRenderUtil;
	SkinnedMesh*	mSkinnedMesh;
};

inline unsigned int SkinEntity::GetIndex()
{
	return fastIndex;
}



