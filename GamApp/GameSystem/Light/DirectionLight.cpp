#include "DirectionLight.h"

const D3DXVECTOR3 defaultDir(0.0f, -1.0f, 1.0f);
DirectionLight::DirectionLight()
{
	m_LightDir = ZEROVECTOR3;
}


DirectionLight::~DirectionLight()
{
}

void DirectionLight::OnFrame()
{
	mPosition = D3DXVECTOR3(mWorldTransform._41, mWorldTransform._42, mWorldTransform._43 );
	D3DXVECTOR4 tempVec4;
	D3DXVec3Transform(&tempVec4, &defaultDir, &mWorldTransform);
	m_LightDir = D3DXVECTOR3(tempVec4.x, tempVec4.y, tempVec4.z);
	D3DXVec3Normalize(&m_LightDir, &m_LightDir);
}

D3DXVECTOR3 DirectionLight::GetLightWorldDir()
{
	return m_LightDir;
}

D3DXCOLOR DirectionLight::GetLightColor()
{
	return m_LightColor;
}

void DirectionLight::SetLightDir(D3DXVECTOR3 dir)
{
	m_LightDir = dir;
}

void DirectionLight::SetLightColor(D3DXCOLOR color)
{
	m_LightColor = color;
}

D3DXVECTOR3 DirectionLight::GetLightWorldPos()
{
	return mPosition;
}
