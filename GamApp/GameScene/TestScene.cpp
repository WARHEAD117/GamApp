#include "TestScene.h"
#include "EntityFeature/Entity.h"
#include "CommonUtil/Timer/GlobalTimer.h"
#include "Camera\CameraParam.h"
#include "CommonUtil/Input/Input.h"

#include "Light/DirectionLight.h"
#include "Light/LightManager.h"

TestScene::TestScene()
{
}


TestScene::~TestScene()
{
	if (g_DrawSkyBox)
		Delete(g_DrawSkyBox);
}


Entity* krisEntity;
Entity* shevaEntity;
Material testMat1;
EffectLoader effectLoader;
DirectionLight dirLight;

void TestScene::OnLoad()
{
	//
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
	//krisEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\Cube.X");
	krisEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\kris_sheva\\kris.X");
	shevaEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\kris_sheva\\sheva.X");
	//testEntity.SetMeshFileName("Res\\Mesh\\car\\car25.X");
	//testEntity.SetMeshFileName("Res\\Mesh\\tree3\\tree3.X");

	Entity* planeEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\plane.X");
	D3DXMATRIX planeM;
	D3DXVECTOR3 move = D3DXVECTOR3(0, -5, 0);
	D3DXMatrixTranslation(&planeM, move.x, move.y, move.z); 
	planeEntity->SetWorldTransform(planeM);

	effectLoader.LoadFxEffect("System\\BankBRDFLight.fx");
	testMat1.effect = effectLoader.GetEffect();
	krisEntity->SetMaterial(&testMat1);


	LIGHTMANAGER::Instance().AddLight(&dirLight);
}

void ComputeMove(D3DXVECTOR3& move)
{
	double dTime = GLOBALTIMER::Instance().GetFrameTime();
	
	float speed = CameraParam::speed;
	if (KEYDOWN(VK_SHIFT))
	{
		speed *= 5;
	}

	if (KEYDOWN('W'))
	{
		move = D3DXVECTOR3(-1, 0, 0) * speed*(float)dTime;
	}

	if (KEYDOWN('S'))
	{
		move = D3DXVECTOR3(1, 0, 0) * speed*(float)dTime;
	}

	if (KEYDOWN('A'))
	{
		move = D3DXVECTOR3(0, 0, -1) * speed*(float)dTime;
	}

	if (KEYDOWN('D'))
	{
		move = D3DXVECTOR3(0, 0, 1) * speed*(float)dTime;
	}
	if (KEYDOWN('Q'))
	{
		move = D3DXVECTOR3(0, 1, 0) * speed*(float)dTime;
	}

	if (KEYDOWN('E'))
	{
		move = D3DXVECTOR3(0, -1, 0) * speed*(float)dTime;
	}
}

float R = 0;
void TestScene::OnBeginFrame()
{
	D3DXMATRIX cW = mainCamera.GetWorldTransform();
	D3DXMATRIX moveMat;

	D3DXVECTOR3 move(0, 0, 0);
	ComputeMove(move);

	D3DXMatrixTranslation(&moveMat, move.x, move.y, move.z);
	cW = cW * moveMat;
	mainCamera.SetWorldTransform(cW);
	
	move = D3DXVECTOR3(0, -5, -3);
	D3DXMatrixTranslation(&moveMat, move.x, move.y, move.z);
	D3DXMATRIX rotMat;
	D3DXMatrixRotationY(&rotMat, 180+R);
	R+=0.001f;
	rotMat *= moveMat;
	krisEntity->SetWorldTransform(rotMat);

	move = D3DXVECTOR3(0, -5, 3);
	D3DXMatrixTranslation(&moveMat, move.x, move.y, move.z);
	rotMat;
	D3DXMatrixIdentity(&rotMat);
	D3DXMatrixRotationY(&rotMat, 180);
	rotMat *= moveMat;
	shevaEntity->SetWorldTransform(rotMat);
}

void TestScene::OnFrame()
{
	mainCamera.OnFrame();
}

void TestScene::OnEndFrame()
{

}
