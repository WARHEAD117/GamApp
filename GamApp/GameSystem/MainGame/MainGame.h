#pragma once
#include "CommonUtil\GlobalHeader.h"
#include "EntityFeature\EntityManager.h"
#include "DrawSkyBox.h"
#include "Camera\Camera.h"

class MainGame
{
public:
	MainGame();
	~MainGame();

	void GameLoop();
	void GameLoad();
	void GameStart();
	void GameEnd();
private:
	void Run();
	void OnBeginFrame();
	void OnFrame();
	void OnEndFrame();
	void Render();

private:
	EntityManager	m_EntityManager;
	CDrawSkyBox*	g_DrawSkyBox;
	Camera			mCamera;
};

