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
BaseLight* dirLight;
BaseLight* dirLight2;
BaseLight* dirLight3;

BaseLight* dirLight4;
BaseLight* dirLight5;
BaseLight* dirLight6;
BaseLight* dirLight7;
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
	//shevaEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\teapot.X");
	shevaEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\kris_sheva\\sheva.X");
	//testEntity.SetMeshFileName("Res\\Mesh\\car\\car25.X");
	//testEntity.SetMeshFileName("Res\\Mesh\\tree3\\tree3.X");

	Entity* planeEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\room.x");
	D3DXMATRIX planeM;
	D3DXVECTOR3 move = D3DXVECTOR3(0, -5, 0);
	D3DXMatrixTranslation(&planeM, move.x, move.y, move.z); 
	planeEntity->SetWorldTransform(planeM);

	effectLoader.LoadFxEffect("System\\BankBRDFLight.fx");
	testMat1.effect = effectLoader.GetEffect();
	krisEntity->SetMaterial(&testMat1);

	
	dirLight = LIGHTMANAGER::Instance().CreateLight(eDirectionLight);
	dirLight->SetLightColor(D3DXCOLOR(0.3, 0.3, 0.3, 1.0f));
	D3DXMATRIX light1MoveMat;
	D3DXVECTOR3 moveLight = D3DXVECTOR3(0, 10, 0);
	D3DXMatrixTranslation(&light1MoveMat, moveLight.x, moveLight.y, moveLight.z);
	dirLight->SetUseShadow(true);
	dirLight->SetWorldTransform(light1MoveMat);

	dirLight2 = LIGHTMANAGER::Instance().CreateLight(ePointLight);
	dirLight2->SetLightColor(D3DXCOLOR(0.0, 0.0, 1.3, 1.0f));
	dirLight2->SetUseShadow(false);
	D3DXMATRIX light2MoveMat;
	moveLight = D3DXVECTOR3(0, 10, 10);
	D3DXMatrixTranslation(&light2MoveMat, moveLight.x, moveLight.y, moveLight.z);
	D3DXMATRIX light2Mat;
	D3DXMatrixRotationX(&light2Mat, 45);
	dirLight2->SetWorldTransform(light2Mat*light2MoveMat);
	
	dirLight3 = LIGHTMANAGER::Instance().CreateLight(ePointLight);
	dirLight3->SetLightColor(D3DXCOLOR(0.6, 0.6, 0.6, 1.0f));
	dirLight3->SetUseShadow(false);
	dirLight3->SetLightRange(30);
	D3DXMATRIX light3MoveMat;
	D3DXVECTOR3 moveLightP3 = D3DXVECTOR3(2, -2, 0);
	D3DXMatrixTranslation(&light3MoveMat, moveLightP3.x, moveLightP3.y, moveLightP3.z);
	dirLight3->SetWorldTransform(light3MoveMat);
	
	dirLight4 = LIGHTMANAGER::Instance().CreateLight(ePointLight);
	dirLight4->SetLightColor(D3DXCOLOR(1.0, 1.0, 0.0, 1.0f));
	moveLight = D3DXVECTOR3(4, -2, 4);
	D3DXMatrixTranslation(&light3MoveMat, moveLight.x, moveLight.y, moveLight.z);
	dirLight4->SetWorldTransform(light3MoveMat);

	dirLight5 = LIGHTMANAGER::Instance().CreateLight(ePointLight);
	dirLight5->SetLightColor(D3DXCOLOR(0.5, 0.0, 1.0, 1.0f));
	moveLight = D3DXVECTOR3(-4, -2, 4);
	D3DXMatrixTranslation(&light3MoveMat, moveLight.x, moveLight.y, moveLight.z);
	dirLight5->SetWorldTransform(light3MoveMat);

	dirLight6 = LIGHTMANAGER::Instance().CreateLight(eSpotLight);
	dirLight6->SetLightColor(D3DXCOLOR(0.0, 0.0, 1.0, 1.0f));
	dirLight6->SetUseShadow(false);
	dirLight6->SetLightRange(100);
	moveLight = D3DXVECTOR3(0, 10, 0);
	D3DXMatrixTranslation(&light3MoveMat, moveLight.x, moveLight.y, moveLight.z);
	D3DXMatrixRotationX(&light2Mat, -0.4);
	dirLight6->SetWorldTransform(light3MoveMat*light2Mat );

	dirLight7 = LIGHTMANAGER::Instance().CreateLight(eSpotLight);
	dirLight7->SetLightColor(D3DXCOLOR(1.5, 0.0, 0.0, 1.0f));
	moveLight = D3DXVECTOR3(4, -4, -4);
	D3DXMatrixTranslation(&light3MoveMat, moveLight.x, moveLight.y, moveLight.z);
	D3DXMatrixRotationX(&light2Mat, -89);
	dirLight7->SetWorldTransform(light2Mat * light3MoveMat);
	
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
		move = D3DXVECTOR3(0, 0, 1) * speed*(float)dTime;
	}

	if (KEYDOWN('S'))
	{
		move = D3DXVECTOR3(0, 0, -1) * speed*(float)dTime;
	}

	if (KEYDOWN('A'))
	{
		move = D3DXVECTOR3(-1, 0, 0) * speed*(float)dTime;
	}

	if (KEYDOWN('D'))
	{
		move = D3DXVECTOR3(1, 0, 0) * speed*(float)dTime;
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

void ComputeRotate(D3DXMATRIX& rot)
{
	double dTime = GLOBALTIMER::Instance().GetFrameTime();

	float speed = 0.1;
	if (KEYDOWN(VK_SHIFT))
	{
		speed *= 5;
	}

	if (KEYDOWN('X'))
	{
		D3DXMatrixRotationY(&rot, speed*(float)dTime);
	}

	if (KEYDOWN('Z'))
	{
		D3DXMatrixRotationY(&rot, -speed*(float)dTime);
	}
}

float R = 0;
void TestScene::OnBeginFrame()
{
	D3DXMATRIX cW = mainCamera.GetWorldTransform();
	D3DXMATRIX moveMat;

	D3DXVECTOR3 move(0, 0, 0);
	ComputeMove(move);

	D3DXMATRIX rot;
	D3DXMatrixIdentity(&rot);
	ComputeRotate(rot);

	D3DXMatrixTranslation(&moveMat, move.x, move.y, move.z);
	cW = moveMat *cW *  rot;
	mainCamera.SetWorldTransform(cW);
	
	move = D3DXVECTOR3(0, -5, -2);
	D3DXMatrixTranslation(&moveMat, move.x, move.y, move.z);
	D3DXMATRIX rotMat;
	D3DXMatrixRotationY(&rotMat, 180+R);
	R+=0.001f;
	rotMat *= moveMat;
	krisEntity->SetWorldTransform(rotMat);

	move = D3DXVECTOR3(0, -4, 2);
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
