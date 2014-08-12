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

	template<typename CLASSTYPE>
	CLASSTYPE* CreateLight(LightType lightType);
};


template<typename CLASSTYPE>
inline CLASSTYPE* LightManager::CreateLight(LightType lightType)
{
	CLASSTYPE * newLight = new CLASSTYPE();
	newLight->SetLightType(lightType);
	AddLight(newLight);
	return newLight;
}

typedef CSingleton<LightManager> LIGHTMANAGER;
