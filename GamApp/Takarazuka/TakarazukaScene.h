#pragma once
#include "GameScene\BaseScene\BaseScene.h"
#include "NoteManager.h"
#include "Light/DirectionLight.h"

#include "Camera\Camera.h"

class TakarazukaScene :
	public BaseScene
{
public:
	TakarazukaScene();
	virtual ~TakarazukaScene();

	virtual void OnLoad();
	virtual void OnBeginFrame();
	virtual void OnFrame();
	virtual void OnEndFrame();

private:
	NoteManager mNoteManager;
	DirectionLight mDirLight;

	Camera			mainCamera;
};

