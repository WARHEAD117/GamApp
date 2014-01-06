#pragma once
#include "CommonUtil\\GlobalHeader.h"
#include "CommonUtil\D3D9Header.h"

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
public:
	void OnBeginFrame();
	void OnFrame();
	void OnEndFrame();

private:
	D3DMATRIX	mLocalTransform;
	D3DMATRIX	mWorldTramsform;

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
