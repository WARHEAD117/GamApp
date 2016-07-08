#pragma once
#include "BaseScene\BaseScene.h"
 
#include "Camera\Camera.h"

class TestScene :
	public BaseScene
{
public:
	TestScene();
	virtual ~TestScene();

	virtual void OnLoad();
	virtual void OnBeginFrame();
	virtual void OnFrame();
	virtual void OnEndFrame();

	Camera			mainCamera;
};

