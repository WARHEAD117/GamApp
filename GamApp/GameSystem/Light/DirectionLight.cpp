#include "DirectionLight.h"

const D3DXVECTOR3 defaultDir(1.0f, 0.0f, 0.0f);
DirectionLight::DirectionLight()
{
	mLightDir = ZEROVECTOR3;
}


DirectionLight::~DirectionLight()
{
}

void DirectionLight::OnFrame()
{
	mPosition = D3DXVECTOR3(mWorldTransform._41, mWorldTransform._42, mWorldTransform._43 );
	D3DXVECTOR4 tempVec4;
	D3DXVec3Transform(&tempVec4, &defaultDir, &mWorldTransform);
	mLightDir = D3DXVECTOR3(tempVec4.x, tempVec4.y, tempVec4.z);
}
