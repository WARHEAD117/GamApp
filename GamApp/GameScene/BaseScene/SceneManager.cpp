#include "SceneManager.h"


SceneManager::SceneManager()
{
}


SceneManager::~SceneManager()
{
}

void SceneManager::OnLoad()
{
	for (int i = 0; i < mSceneList.size(); i++)
	{
		mSceneList[i]->OnLoad();
	}

}
void SceneManager::OnBeginFrame()
{
	for (int i = 0; i < mSceneList.size(); i++)
	{
		mSceneList[i]->OnBeginFrame();
	}
}

void SceneManager::OnFrame()
{
	for (int i = 0; i < mSceneList.size(); i++)
	{
		mSceneList[i]->OnFrame();
	}
}

void SceneManager::OnEndFrame()
{
	for (int i = 0; i < mSceneList.size(); i++)
	{
		mSceneList[i]->OnEndFrame();
	}
}

void SceneManager::AddScene(BaseScene* const scene)
{
	mSceneList.push_back(scene);
}
