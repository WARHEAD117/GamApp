#pragma once
#include "CommonUtil/GlobalHeader.h"
#include "CommonUtil/D3D9Header.h"

class DirectionLight
{
public:
	DirectionLight();
	~DirectionLight();

	D3DXVECTOR3 mPosition;
	D3DXVECTOR3 mLightDir;

	float		mLightIntensity;
	D3DXCOLOR	mLightColor;
};

