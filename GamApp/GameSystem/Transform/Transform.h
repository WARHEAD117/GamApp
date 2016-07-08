#pragma once 
#include "CommonUtil/GlobalHeader.h"
#include "CommonUtil/D3D9Header.h"

class Transform
{
public:
	Transform();
	~Transform();

	D3DXMATRIX	GetWorldTransform();
	void		SetWorldTransform(D3DXMATRIX matrix);

protected:
	D3DXMATRIX	mLocalTransform;
	D3DXMATRIX	mWorldTransform;

	D3DXVECTOR3	mLocalTranslate;
	D3DXVECTOR3	mLocalScale;
	D3DXVECTOR3	mLocalRotation;

	D3DXVECTOR3	mWorldTranslate;
	D3DXVECTOR3	mWorldScale;
	D3DXVECTOR3	mWorldRotation;
};

inline D3DXMATRIX Transform::GetWorldTransform()
{
	return mWorldTransform;
}

inline void Transform::SetWorldTransform(D3DXMATRIX matrix)
{
	mWorldTransform = matrix;
}