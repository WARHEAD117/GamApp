#include "Transform.h"
 
Transform::Transform()
{
	D3DXMatrixIdentity(&mWorldTransform);
}


Transform::~Transform()
{
}

