#include "MainGame.h"
#include "CommonUtil\Timer\GlobalTimer.h"
#include "RenderSystem\D3D9Device.h"
#include "RenderPipeLine\RenderPipe.h"

#include "EntityFeature\Entity.h"

#include "GameScene/BaseScene/SceneManager.h"
#include "Light/LightManager.h"

#include "GameScene/TestScene.h"

MainGame::MainGame()
{
}

MainGame::~MainGame()
{
}

//Just for test-------------------------------------------------------------------
TestScene testScene;
//--------------------------------------------------------------------------------

void MainGame::GameLoad()
{
	SCENEMANAGER::Instance().AddScene(&testScene);

	SCENEMANAGER::Instance().OnLoad();
}

void MainGame::GameLoop()
{
	GLOBALTIMER::Instance().StartFrame();
	Run();
	GLOBALTIMER::Instance().EndFrame();
}

void MainGame::Run()
{
	OnBeginFrame();
	OnFrame();
	Render();
	OnEndFrame();
}

void MainGame::OnBeginFrame()
{
	SCENEMANAGER::Instance().OnBeginFrame();
	ENTITYMANAGER::Instance().OnBeginFrame();
}

void MainGame::OnFrame()
{
	SCENEMANAGER::Instance().OnFrame();
	ENTITYMANAGER::Instance().OnFrame();

	LIGHTMANAGER::Instance().OnFrame();
}

void MainGame::Render()
{
	RENDERPIPE::Instance().RenderAll();
}
	
void MainGame::OnEndFrame()
{
	SCENEMANAGER::Instance().OnEndFrame();
	ENTITYMANAGER::Instance().OnEndFrame();
}

void MainGame::GameStart()
{
	GLOBALTIMER::Instance().StartGameTimer();
}

void MainGame::GameEnd()
{
}
