#pragma once
#include "CommonUtil/GlobalHeader.h"
#include "EntityFeature/EntityManager.h"

class BaseScene
{
public:
	BaseScene();
	~BaseScene();

	virtual void OnLoad();
	virtual void OnBeginFrame();
	virtual void OnFrame();
	virtual void OnEndFrame();

	virtual void SetSceneName(std::string sceneName);
	virtual std::string GetSceneName();

protected:
	std::string		mSceneName;
};


inline void BaseScene::SetSceneName(std::string sceneName)
{
	mSceneName = sceneName;
}


inline std::string BaseScene::GetSceneName()
{
	return mSceneName;
}