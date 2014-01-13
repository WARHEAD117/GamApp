#pragma once
#include "BaseScene\BaseScene.h"


#include "DrawSkyBox.h"
#include "Camera\Camera.h"

class TestScene :
	public BaseScene
{
public:
	TestScene();
	~TestScene();

	virtual void OnLoad();
	virtual void OnBeginFrame();
	virtual void OnFrame();
	virtual void OnEndFrame();


	CDrawSkyBox*	g_DrawSkyBox;
	Camera			mainCamera;
};

