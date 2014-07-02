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
Entity* horseEntity;
Material testMat1;
EffectLoader effectLoader;
BaseLight* dirLight1;
BaseLight* dirLight2;
BaseLight* pointLight1;
BaseLight* pointLight2;
BaseLight* pointLight3;
BaseLight* pointLight4;
BaseLight* pointLight5;
BaseLight* spotLight1;
BaseLight* spotLight2;
BaseLight* spotLight3;
BaseLight* spotLight4;
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
	horseEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\horse\\horse.X");

	Entity* roomEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\room\\room.x");
	roomEntity->SetTexture("Res\\Mesh\\room\\wallpaper.dds",0);
	roomEntity->SetTexture("Res\\Mesh\\room\\DoorDiff.dds", 1);
	roomEntity->SetTexture("Res\\Mesh\\room\\oldwood.dds", 2);
	roomEntity->SetTexture("Res\\Mesh\\room\\FireplaceDiff.dds", 3);

	horseEntity->SetTexture("Res\\Mesh\\horse\\HorseB_512.jpg");
	horseEntity->SetNormalMap("Res\\Mesh\\horse\\HorseB _NRM_512.jpg");

	D3DXMATRIX planeM;
	D3DXVECTOR3 move = D3DXVECTOR3(0, -5, 0);
	D3DXMatrixTranslation(&planeM, move.x, move.y, move.z); 
	roomEntity->SetWorldTransform(planeM);

	D3DXMATRIX horseM;
	move = D3DXVECTOR3(0, -5, 0);
	D3DXMatrixTranslation(&horseM, move.x, move.y, move.z);
	move = D3DXVECTOR3(0.1, 0.1, 0.1);
	D3DXMATRIX horseS;
	D3DXMatrixScaling(&horseS, move.x, move.y, move.z);
	horseEntity->SetWorldTransform(horseS * horseM);

	effectLoader.LoadFxEffect("System\\BankBRDFLight.fx");
	testMat1.effect = effectLoader.GetEffect();
	krisEntity->SetMaterial(&testMat1);

	//--------------------------------------------------------------------------
	D3DXMATRIX lightMoveMat;
	D3DXMATRIX lightRot1Mat;
	D3DXMATRIX lightRot2Mat;
	dirLight1 = LIGHTMANAGER::Instance().CreateLight(eDirectionLight);
	dirLight1->SetLightColor(D3DXCOLOR(1.3, 1.3, 1.3, 1.0f));
	dirLight1->SetUseShadow(true);

	D3DXMatrixTranslation(&lightMoveMat, 0, 10, 0);
	dirLight1->SetWorldTransform(lightMoveMat);
	//--------------------------------------------------------------------------
	dirLight2 = LIGHTMANAGER::Instance().CreateLight(eDirectionLight);
	dirLight2->SetLightColor(D3DXCOLOR(0.0, 0.0, 0.3, 1.0f));
	dirLight2->SetUseShadow(false);
	
	D3DXMatrixTranslation(&lightMoveMat, 0, 10, 10);
	D3DXMatrixRotationX(&lightRot1Mat, 0.25 * D3DX_PI);

	dirLight2->SetWorldTransform(lightRot1Mat*lightMoveMat);
	//--------------------------------------------------------------------------
	pointLight1 = LIGHTMANAGER::Instance().CreateLight(ePointLight);
	pointLight1->SetLightColor(D3DXCOLOR(0.7,0.7,0.7, 1.0f));
	pointLight1->SetUseShadow(true);
	pointLight1->SetLightRange(20);
	
	D3DXMatrixTranslation(&lightMoveMat, -10, -4, 0);
	pointLight1->SetWorldTransform(lightMoveMat);

	//--------------------------------------------------------------------------
	pointLight2 = LIGHTMANAGER::Instance().CreateLight(ePointLight);
	pointLight2->SetLightColor(D3DXCOLOR(0.7, 0.7, 0.7, 1.0f));
	pointLight2->SetUseShadow(false);
	pointLight2->SetLightRange(2);

	D3DXMatrixTranslation(&lightMoveMat, 20, -4, 9);
	pointLight2->SetWorldTransform(lightMoveMat);

	//--------------------------------------------------------------------------
	pointLight3 = LIGHTMANAGER::Instance().CreateLight(ePointLight);
	pointLight3->SetLightColor(D3DXCOLOR(0.7, 0.7, 0.7, 1.0f));
	pointLight3->SetUseShadow(false);
	pointLight3->SetLightRange(2);

	D3DXMatrixTranslation(&lightMoveMat, 20, -4, 10);
	pointLight3->SetWorldTransform(lightMoveMat);

	//--------------------------------------------------------------------------
	pointLight4 = LIGHTMANAGER::Instance().CreateLight(ePointLight);
	pointLight4->SetLightColor(D3DXCOLOR(3.7, 3.7, 3.7, 1.0f));
	pointLight4->SetUseShadow(false);
	pointLight4->SetLightRange(0.3);

	D3DXMatrixTranslation(&lightMoveMat, 19.8, -4, 14);
	pointLight4->SetWorldTransform(lightMoveMat);

	//--------------------------------------------------------------------------
	pointLight5 = LIGHTMANAGER::Instance().CreateLight(ePointLight);
	pointLight5->SetLightColor(D3DXCOLOR(4.7, 0.7, 4.7, 1.0f));
	pointLight5->SetUseShadow(false);
	pointLight5->SetLightRange(0.3);

	D3DXMatrixTranslation(&lightMoveMat, 19.8, -4, 16);
	pointLight5->SetWorldTransform(lightMoveMat);
	//--------------------------------------------------------------------------
	spotLight1 = LIGHTMANAGER::Instance().CreateLight(eSpotLight);
	spotLight1->SetLightRange(20);
	spotLight1->SetLightColor(D3DXCOLOR(1.0, 0.0, 0.0, 1.0f));
	D3DXMatrixTranslation(&lightMoveMat, 5, 5, 5);
	spotLight1->SetWorldTransform(lightMoveMat);
	//--------------------------------------------------------------------------
	spotLight2 = LIGHTMANAGER::Instance().CreateLight(eSpotLight);
	spotLight2->SetLightRange(45);
	spotLight2->SetUseShadow(false);
	spotLight2->SetLightColor(D3DXCOLOR(0.0, 1.0, 0.0, 1.0f));
	D3DXMatrixTranslation(&lightMoveMat, 5, 10, -5);
	D3DXMatrixRotationX(&lightRot1Mat, 0.2f * D3DX_PI);
	spotLight2->SetWorldTransform(lightMoveMat*lightRot1Mat);
	//--------------------------------------------------------------------------
	spotLight3 = LIGHTMANAGER::Instance().CreateLight(eSpotLight);
	spotLight3->SetLightRange(2);
	spotLight3->SetLightColor(D3DXCOLOR(0.0, 0.5, 0.0, 1.0f));
	spotLight3->SetUseShadow(false);
	spotLight3->SetLightRange(20);
	D3DXMatrixTranslation(&lightMoveMat, -10, 10, 10);
	D3DXMatrixRotationX(&lightRot1Mat, 0.25f * D3DX_PI);
	spotLight3->SetWorldTransform(lightMoveMat*lightRot1Mat);
	//--------------------------------------------------------------------------
	spotLight4 = LIGHTMANAGER::Instance().CreateLight(eSpotLight);
	spotLight4->SetLightRange(20);
	spotLight4->SetLightColor(D3DXCOLOR(0.0, 0.5, 0.0, 1.0f));
	D3DXMatrixTranslation(&lightMoveMat, -10, 10, -10);
	D3DXMatrixRotationX(&lightRot1Mat, -0.25f * D3DX_PI);
	spotLight4->SetWorldTransform(lightMoveMat*lightRot1Mat);
	
}

void ComputeMove(D3DXVECTOR3& move)
{
	double dTime = GLOBALTIMER::Instance().GetFrameTime();
	
	float speed = CameraParam::speed;
	if (GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		speed *= 5;
	}

	if (GAMEINPUT::Instance().KeyDown(DIK_W))
	{
		move = D3DXVECTOR3(0, 0, 1) * speed*(float)dTime;
	}

	if (GAMEINPUT::Instance().KeyDown(DIK_S))
	{
		move = D3DXVECTOR3(0, 0, -1) * speed*(float)dTime;
	}

	if (GAMEINPUT::Instance().KeyDown(DIK_A))
	{
		move = D3DXVECTOR3(-1, 0, 0) * speed*(float)dTime;
	}

	if (GAMEINPUT::Instance().KeyDown(DIK_D))
	{
		move = D3DXVECTOR3(1, 0, 0) * speed*(float)dTime;
	}
	if (GAMEINPUT::Instance().KeyDown(DIK_Q))
	{
		move = D3DXVECTOR3(0, 1, 0) * speed*(float)dTime;
	}

	if (GAMEINPUT::Instance().KeyDown(DIK_E))
	{
		move = D3DXVECTOR3(0, -1, 0) * speed*(float)dTime;
	}
}

void BuildRot(D3DXMATRIX& world)
{
	D3DXVECTOR3 pos;
	pos.x = world(3, 0);
	pos.y = world(3, 1);
	pos.z = world(3, 2);

	world(3, 0) = 0;
	world(3, 1) = 0;
	world(3, 2) = 0;

	D3DXVECTOR3 right;
	right.x = world(0, 0);
	right.y = world(0, 1);
	right.z = world(0, 2);

	D3DXVECTOR3 up;
	up.x = world(1, 0);
	up.y = world(1, 1);
	up.z = world(1, 2);

	D3DXVECTOR3 front;
	front.x = world(2, 0);
	front.y = world(2, 1);
	front.z = world(2, 2);

	D3DXVec3Normalize(&right, &right);
	double dTime = GLOBALTIMER::Instance().GetFrameTime();
	D3DXVECTOR2 mouseMove = GAMEINPUT::Instance().GetMouseMove();
	float speed = 0.2;
	if (GAMEINPUT::Instance().KeyDown(DIK_LSHIFT))
	{
		speed *= 5;
	}

	D3DXMATRIX rot1;
	D3DXMATRIX rot2;

	D3DXMatrixRotationAxis(&rot1, &right, mouseMove.y*speed*(float)dTime);
	D3DXMatrixRotationY(&rot2, mouseMove.x*speed*(float)dTime);

	world = world * rot1 * rot2;

	world(3, 0) = pos.x;
	world(3, 1) = pos.y;
	world(3, 2) = pos.z;
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

	D3DXMatrixTranslation(&moveMat, move.x, move.y, move.z);
	BuildRot(cW);
	cW = moveMat *cW;
	mainCamera.SetWorldTransform(cW);
	
	move = D3DXVECTOR3(0, -5, -5);
	D3DXMatrixTranslation(&moveMat, move.x, move.y, move.z);
	D3DXMATRIX rotMat;
	D3DXMatrixRotationY(&rotMat, (180+R) / 180.0f * D3DX_PI);
	R+=1.0f;
	rotMat *= moveMat;
	krisEntity->SetWorldTransform(rotMat);

	move = D3DXVECTOR3(0, -5, 5);
	D3DXMatrixTranslation(&moveMat, move.x, move.y, move.z);
	rotMat;
	D3DXMatrixIdentity(&rotMat);
	D3DXMatrixRotationY(&rotMat, -0.5f * D3DX_PI);
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
