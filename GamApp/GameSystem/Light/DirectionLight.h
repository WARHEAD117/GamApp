#pragma once
#include "Light/BaseLight.h"

class DirectionLight : 
	public BaseLight
{
public:
	DirectionLight();
	~DirectionLight();

	void OnFrame();

	D3DXVECTOR3 GetLightDir();
	D3DXVECTOR3 GetLightPos();
	D3DXCOLOR GetLightColor();

	void SetLightDir(D3DXVECTOR3 dir);
	void SetLightColor(D3DXCOLOR color);

	D3DXVECTOR3 mPosition;
	D3DXVECTOR3 m_LightDir;

	float		mLightIntensity;
	D3DXCOLOR	m_LightColor;
};
