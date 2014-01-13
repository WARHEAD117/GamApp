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
};

typedef CSingleton<LightManager> LIGHTMANAGER;
