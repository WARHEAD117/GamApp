#pragma once
#include "CommonUtil/GlobalHeader.h"
#include "CommonUtil/Singleton.h"
#include "BaseScene.h"

class SceneManager
{
public:
	SceneManager();
	~SceneManager();

	void AddScene(BaseScene* const scene);

	void OnLoad();
	void OnBeginFrame();
	void OnFrame();
	void OnEndFrame();

private:
	std::vector<BaseScene*> mSceneList;
};

typedef CSingleton<SceneManager> SCENEMANAGER;