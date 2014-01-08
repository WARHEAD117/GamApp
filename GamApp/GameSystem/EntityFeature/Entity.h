#pragma once
#include "CommonUtil\GlobalHeader.h"
#include "CommonUtil\D3D9Header.h"
#include "CommonUtil\MeshLoader\MeshLoader.h"
#include "CommonUtil\EffectLoader\EffectLoader.h"
#include "RenderUtil\RenderUtil.h"

class Entity
{
public:
	Entity();
	~Entity();

private:
	unsigned int fastIndex;
	static unsigned int baseFastIndex;
public:
	unsigned int GetIndex();

	D3DXMATRIX	GetWorldTransform();
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

	D3DXMATRIX	mLocalTransform;
	D3DXMATRIX	mWorldTramsform;

	D3DXVECTOR3	mLocalTranslate;
	D3DXVECTOR3	mLocalScale;
	D3DXVECTOR3	mLocalRotation;

	D3DXVECTOR3	mWorldTranslate;
	D3DXVECTOR3	mWorldScale;
	D3DXVECTOR3	mWorldRotation;
};

inline unsigned int Entity::GetIndex()
{
	return fastIndex;
}

inline D3DXMATRIX Entity::GetWorldTransform()
{
	return mWorldTramsform;
}




