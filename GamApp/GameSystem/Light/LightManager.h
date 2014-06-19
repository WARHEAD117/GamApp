#pragma once
#include "CommonUtil/GlobalHeader.h"
#include "CommonUtil/D3D9Header.h"
#include "CommonUtil/Singleton.h"
#include "BaseLight.h"

class LightManager
{
public:
	LightManager();
	~LightManager();

	std::vector<BaseLight*> mLightList;

	void AddLight(BaseLight* const light);
	void OnFrame();

	BaseLight* GetLight(int lightIndex);
	int GetLightCount();

	BaseLight* CreateLight(LightType lightType);
};


typedef CSingleton<LightManager> LIGHTMANAGER;
