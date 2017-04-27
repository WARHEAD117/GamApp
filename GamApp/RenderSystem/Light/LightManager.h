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
	std::vector<BaseLight*> mIBLList;

	void AddLight(BaseLight* const light);
	void AddIBL(BaseLight* const light);
	void OnFrame();

	BaseLight* GetLight(int lightIndex);
	BaseLight* GetIBL(int lightIndex);
	int GetLightCount();
	int GetIBLCount();

	template<typename CLASSTYPE>
	CLASSTYPE* CreateLight(LightType lightType);
};


template<typename CLASSTYPE>
inline CLASSTYPE* LightManager::CreateLight(LightType lightType)
{
	CLASSTYPE * newLight = new CLASSTYPE();
	if (lightType == eImageBasedLight)
	{
		AddIBL(newLight);
	}
	else
	{
		AddLight(newLight);
	}
	newLight->SetLightType(lightType);
	return newLight;
}

typedef CSingleton<LightManager> LIGHTMANAGER;
