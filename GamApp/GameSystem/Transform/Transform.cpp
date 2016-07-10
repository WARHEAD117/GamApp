#include "Transform.h"
 
Transform::Transform()
{
	D3DXMatrixIdentity(&mWorldTransform);
	D3DXMatrixIdentity(&mLocalTransform);
	mWorldTranslate = ZEROVECTOR3;
	mLocalTranslate = ZEROVECTOR3;
	mWorldScale = D3DXVECTOR3(1,1,1);
	mLocalScale = D3DXVECTOR3(1, 1, 1);
	mWorldRotation = ZEROVECTOR3;
	mLocalRotation = ZEROVECTOR3;
}


Transform::~Transform()
{
}

