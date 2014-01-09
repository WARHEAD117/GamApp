#include "MainGame.h"
#include "CommonUtil\Timer\GlobalTimer.h"
#include "RenderSystem\D3D9Device.h"
#include "RenderPipeLine\RenderPipe.h"

#include "Camera\CameraParam.h"
#include "EntityFeature\Entity.h"

MainGame::MainGame()
{
}


MainGame::~MainGame()
{
	if (g_DrawSkyBox)
		Delete(g_DrawSkyBox);
}

//Just for test-------------------------------------------------------------------
Entity testEntity;
//--------------------------------------------------------------------------------

void MainGame::GameLoad()
{
	//Just for test---------------------------------------------------------------------------------
	//--初始化天空盒及其贴图
	g_DrawSkyBox = new CDrawSkyBox(RENDERDEVICE::Instance().g_pD3DDevice);

	g_DrawSkyBox->InitVB();
	g_DrawSkyBox->SetTexture("Res\\SkyBox\\bottom.jpg", 0);
	g_DrawSkyBox->SetTexture("Res\\SkyBox\\left.jpg", 1);
	g_DrawSkyBox->SetTexture("Res\\SkyBox\\right.jpg", 2);
	g_DrawSkyBox->SetTexture("Res\\SkyBox\\top.jpg", 3);
	g_DrawSkyBox->SetTexture("Res\\SkyBox\\front.jpg", 5);
	g_DrawSkyBox->SetTexture("Res\\SkyBox\\back.jpg", 4);

	
	//===================================================================================================
	//BuildCamera
	mainCamera.Init();

	//Create Entity
	testEntity.SetMeshFileName("Res\\Mesh\\tree3\\tree3.X");
	testEntity.BuildRenderUtil();

	m_EntityManager.AddEntity(testEntity);
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
	m_EntityManager.OnBeginFrame();
}

void MainGame::OnFrame()
{
	m_EntityManager.OnFrame();

	mainCamera.OnFrame(CameraParam::posW, CameraParam::lookAtW, CameraParam::rightW, CameraParam::upW);
}

void MainGame::Render()
{
	//g_DrawSkyBox->Render(D3DXVECTOR3(0, 0, 1));

	RENDERPIPE::Instance().RenderAll();
}
	
void MainGame::OnEndFrame()
{
	m_EntityManager.OnEndFrame();
}

void MainGame::GameStart()
{
	GLOBALTIMER::Instance().StartGameTimer();
}

void MainGame::GameEnd()
{
}
