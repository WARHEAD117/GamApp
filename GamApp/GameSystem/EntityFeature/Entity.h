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

private:
	unsigned int fastIndex;
	static unsigned int baseFastIndex;
public:
	unsigned int GetIndex();

public:
	void OnBeginFrame();
	void OnFrame();
	void OnEndFrame();

	void SetMeshFileName(std::string fileName);
	void SetEffectFileName(int subMeshIndex, std::string fileName);

	void AssignRenderUtil();
	void BuildRenderUtil();

private:
	RenderUtil		mRenderUtil;
	MeshLoader		mMeshLoader;
	EffectLoader	mEffectLoader;
};

inline unsigned int Entity::GetIndex()
{
	return fastIndex;
}






