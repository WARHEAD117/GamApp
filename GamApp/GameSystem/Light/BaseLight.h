#pragma once
#include "CommonUtil/GlobalHeader.h"
#include "CommonUtil/D3D9Header.h"

#include "Transform/Transform.h"

enum LightType
{
	eDirectionLight,
	ePointLight,
	eSpotLight
};
class BaseLight :
	public Transform
{
public:
	BaseLight();
	~BaseLight();

	LightType mLightType;

	void SetLightType(LightType lightType);
	LightType GetLightType();

	virtual void OnFrame();
};

inline void BaseLight::SetLightType(LightType lightType)
{
	mLightType = lightType;
}

inline LightType BaseLight::GetLightType()
{
	return mLightType;
}