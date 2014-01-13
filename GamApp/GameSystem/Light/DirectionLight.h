#pragma once
#include "Light/BaseLight.h"

class DirectionLight : 
	public BaseLight
{
public:
	DirectionLight();
	~DirectionLight();

	void OnFrame();

	D3DXVECTOR3 mPosition;
	D3DXVECTOR3 mLightDir;

	float		mLightIntensity;
	D3DXCOLOR	mLightColor;
};

