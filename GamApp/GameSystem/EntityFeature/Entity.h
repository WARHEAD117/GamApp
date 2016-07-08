#pragma once
#include "CommonUtil\GlobalHeader.h"
#include "CommonUtil\D3D9Header.h"
#include "CommonUtil\MeshLoader\MeshLoader.h"
#include "CommonUtil\EffectLoader\EffectLoader.h"
#include "RenderUtil\RenderUtil.h"
 
#include "Transform/Transform.h"

class Entity :
	public Transform
{
public:
	Entity();
	~Entity();

protected:
	unsigned int fastIndex;
	static unsigned int baseFastIndex;
public:
	virtual unsigned int GetIndex();

public:
	virtual void OnBeginFrame();
	virtual void OnFrame();
	virtual void OnEndFrame();

	virtual void SetMeshFileName(std::string fileName);
	virtual void SetEffect(std::string fileName, int subMeshIndex = -1);
	virtual void SetEffect(LPD3DXEFFECT effect, int subMeshIndex = -1);
	virtual void SetTexture(std::string fileName, int subMeshIndex = -1);
	virtual void SetNormalMap(std::string fileName, int subMeshIndex = -1);
	virtual void SetSpecularMap(std::string fileName, int subMeshIndex = -1);

	virtual void SetMaterial(Material* material, int subMeshIndex = -1);

	virtual void AssignRenderUtil();
	virtual void BuildRenderUtil();

protected:
	RenderUtil		mRenderUtil;
	MeshLoader		mMeshLoader;
	EffectLoader	mEffectLoader;
};

inline unsigned int Entity::GetIndex()
{
	return fastIndex;
}



