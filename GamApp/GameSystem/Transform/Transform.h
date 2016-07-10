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

	void		SetWorldTranslate(float x, float y, float z);
	void		SetLocalTranslate(float x, float y, float z);
	void		SetWorldScale(float x, float y, float z);
	void		SetLocalScale(float x, float y, float z);
	void		SetWorldRotation(float x, float y, float z);
	void		SetLocalRotation(float x, float y, float z);

protected:
	D3DXMATRIX	mLocalTransform;
	D3DXMATRIX	mWorldTransform;

	D3DXVECTOR3	mLocalTranslate;
	D3DXVECTOR3	mLocalScale;
	D3DXVECTOR3	mLocalRotation;

	D3DXVECTOR3	mWorldTranslate;
	D3DXVECTOR3	mWorldScale;
	D3DXVECTOR3	mWorldRotation;

	void BuildTransform();
};

inline D3DXMATRIX Transform::GetWorldTransform()
{
	return mWorldTransform;
}

inline void Transform::SetWorldTransform(D3DXMATRIX matrix)
{
	mWorldTransform = matrix;
}

inline void Transform::SetWorldTranslate(float x, float y, float z)
{
	mWorldTranslate = D3DXVECTOR3(x, y, z);
	BuildTransform();
}
inline void Transform::SetLocalTranslate(float x, float y, float z)
{
	mLocalTranslate = D3DXVECTOR3(x, y, z);
	BuildTransform();
}
inline void Transform::SetWorldScale(float x, float y, float z)
{
	mWorldScale = D3DXVECTOR3(x, y, z);
	BuildTransform();
}
inline void Transform::SetLocalScale(float x, float y, float z)
{
	mLocalScale = D3DXVECTOR3(x, y, z);
	BuildTransform();
}
inline void Transform::SetWorldRotation(float x, float y, float z)
{
	mWorldRotation = D3DXVECTOR3(x, y, z);
	BuildTransform();
}
inline void Transform::SetLocalRotation(float x, float y, float z)
{
	mLocalRotation = D3DXVECTOR3(x, y, z);
	BuildTransform();
}
inline void Transform::BuildTransform()
{
	D3DXMATRIX translateMat;
	D3DXMatrixTranslation(&translateMat, mWorldTranslate.x, mWorldTranslate.y, mWorldTranslate.z);
	D3DXMATRIX scaleMat;
	D3DXMatrixScaling(&scaleMat, mWorldScale.x, mWorldScale.y, mWorldScale.z);
	D3DXMATRIX rotateMatX;
	D3DXMATRIX rotateMatY;
	D3DXMATRIX rotateMatZ;
	D3DXMatrixRotationX(&rotateMatX, mWorldRotation.x);
	D3DXMatrixRotationY(&rotateMatY, mWorldRotation.y);
	D3DXMatrixRotationZ(&rotateMatZ, mWorldRotation.z);

	mWorldTransform = scaleMat * rotateMatX * rotateMatY * rotateMatZ * translateMat;
}