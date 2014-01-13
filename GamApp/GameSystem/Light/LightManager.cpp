#include "LightManager.h"


LightManager::LightManager()
{
}


LightManager::~LightManager()
{
}

void LightManager::AddLight(BaseLight* const light)
{
	mLightList.push_back(light);
}

void LightManager::OnFrame()
{
	for (int i = 0; i < mLightList.size(); i++)
	{
		mLightList[i]->OnFrame();
	}
}
