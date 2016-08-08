#include "TestScene.h"
#include "EntityFeature/Entity.h"
#include "CommonUtil/Timer/GlobalTimer.h"
#include "Camera\CameraParam.h"
#include "CommonUtil/Input/Input.h"

#include "Light/DirectionLight.h"
#include "Light/PointLight.h"
#include "Light/SpotLight.h"
#include "Light/LightManager.h"

#include "RenderSystem/TempSkin/SkinnedMesh.h"
#include "RenderSystem/TempSkin/Vertex.h"

#include "RenderSystem/RenderPipeLine/RenderPipe.h"

TestScene::TestScene()
{
}


TestScene::~TestScene()
{
}


Entity* krisEntity;
Entity* shevaEntity;
Entity* horseEntity;
Entity* horse2Entity;
Entity* bunnyEntity;
Entity* deerEntity;
Entity* deerEntity2;

Entity* mountainEntity;
Entity* groundEntity;

Entity* rock1Entity;
Entity* rock2Entity;

Entity* grass1Entity;
Entity* grass2Entity;
Entity* grass3Entity;

Entity* houseEntity;

Entity* chestEntity;

Entity* sponzaEntity;


Material testMat1;
EffectLoader effectLoader;
DirectionLight* dirLight1;
DirectionLight* dirLight2;
PointLight* pointLight0;
PointLight* pointLight1;
PointLight* pointLight2;
PointLight* pointLight3;
PointLight* pointLight4;
PointLight* pointLight5;
SpotLight* spotLight1;
SpotLight* spotLight2;
SpotLight* spotLight3;
SpotLight* spotLight4;

SpotLight* spotLight5;


//-----------------------
//临时skinedmesh
SkinnedMesh* mSkinnedMesh;

SkinnedMesh* mSkinnedMeshLittle;

void TestScene::OnLoad()
{
	//===================================================================================================
	//BuildCamera
	mainCamera.Init();


	//==========================================================================================
	//Create Entity
	if (false)
	{
		horseEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\horse\\horse.X");
		horseEntity->SetTexture("Res\\Mesh\\horse\\HorseB_512.jpg");
		//horseEntity->SetNormalMap("Res\\Mesh\\horse\\HorseB _NRM_512.jpg");

		Material horseMat;
		horseMat.Thickness = D3DXVECTOR4(0.5, 0.7, 1, 1);
		horseEntity->SetMaterial(&horseMat);

		D3DXMATRIX horseM;
		D3DXVECTOR3 horseMV = D3DXVECTOR3(-12, 0, 1.5f);
		D3DXMatrixTranslation(&horseM, horseMV.x, horseMV.y, horseMV.z);
		D3DXVECTOR3 horseSV = D3DXVECTOR3(0.1f, 0.1f, 0.1f);
		horseSV = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		D3DXMATRIX horseS;
		D3DXMatrixScaling(&horseS, horseSV.x, horseSV.y, horseSV.z);
		D3DXMATRIX horseRotMat;
		D3DXMatrixRotationY(&horseRotMat, -0.225f * D3DX_PI);
		horseEntity->SetWorldTransform(horseRotMat * horseS * horseM);
	}
	

	//==========================================================================================
	if (false)
	{
		horse2Entity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\horse2.X");

		Material horse2Mat;
		horse2Mat.Thickness = D3DXVECTOR4(1.5, 0.8, 1, 1);
		horse2Mat.MatIndex = 1;
		horse2Entity->SetMaterial(&horse2Mat);

		D3DXMATRIX horse2M;
		D3DXVECTOR3 horse2MV = D3DXVECTOR3(-12, 25, -20.5f);
		D3DXMatrixTranslation(&horse2M, horse2MV.x, horse2MV.y, horse2MV.z);
		D3DXVECTOR3 horse2SV = D3DXVECTOR3(0.01f, 0.01f, 0.01f);
		D3DXMATRIX horse2S;
		D3DXMatrixScaling(&horse2S, horse2SV.x, horse2SV.y, horse2SV.z);
		D3DXMATRIX horse2RotMat;
		D3DXMatrixRotationY(&horse2RotMat, -0.225f * D3DX_PI);
		horse2Entity->SetWorldTransform(horse2RotMat * horse2S * horse2M);
	}

	//==========================================================================================
	if (true)
	{
		deerEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\deer\\deer.X");

		D3DXMATRIX deerM;
		D3DXVECTOR3 deerMove = D3DXVECTOR3(-12, 0, 32.5f);
		D3DXMatrixTranslation(&deerM, deerMove.x, deerMove.y, deerMove.z);
		D3DXMATRIX deerS;
		D3DXVECTOR3 deerSV = D3DXVECTOR3(0.1, 0.1, 0.1);
		D3DXMatrixScaling(&deerS, deerSV.x, deerSV.y, deerSV.z);
		deerEntity->SetWorldTransform(deerS * deerM);

		Material deerMat;
		deerMat.Thickness = D3DXVECTOR4(1, 1, 1, 1);
		deerMat.MatIndex = 0;
		deerEntity->SetMaterial(&deerMat);

	}
	//==========================================================================================
	if (false)
	{
		deerEntity2 = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\deer\\deer.X");

		D3DXMATRIX deer2M;
		D3DXVECTOR3 deer2Move = D3DXVECTOR3(-12, 0, 48.5f);
		D3DXMatrixTranslation(&deer2M, deer2Move.x, deer2Move.y, deer2Move.z);
		D3DXMATRIX deer2S;
		D3DXVECTOR3 deer2SV = D3DXVECTOR3(0.1, 0.1, 0.1);
		D3DXMatrixScaling(&deer2S, deer2SV.x, deer2SV.y, deer2SV.z);
		deerEntity2->SetWorldTransform(deer2S * deer2M);

	}
	
	//==========================================================================================
	if (true)
	{
		mountainEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\farMountain2.X");
		//mountainEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\plane2.X");
		//mountainEntity->SetTexture("System\\white.dds");

		Material mountainMat;
		mountainMat.Thickness = D3DXVECTOR4(0, 1, 0.5f, 1);
		mountainMat.MatIndex = 2;
		mountainEntity->SetMaterial(&mountainMat);


		D3DXMATRIX mountainM;
		D3DXVECTOR3 mountainMV = D3DXVECTOR3(-30, -0.5f, 0);
		D3DXMatrixTranslation(&mountainM, mountainMV.x, mountainMV.y, mountainMV.z);
		D3DXMATRIX mountainS;
		D3DXVECTOR3 mountainSV = D3DXVECTOR3(1, 1, 1);
		D3DXMatrixScaling(&mountainS, mountainSV.x, mountainSV.y, mountainSV.z);
		D3DXMATRIX mountainRotMat;
		D3DXMatrixRotationY(&mountainRotMat, -0.5f * D3DX_PI);
		mountainEntity->SetWorldTransform(mountainRotMat * mountainS * mountainM);
	}

	if (true)
	{
		//==========================================================================================
		//groundEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\farMountain2.X");
		groundEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\ground.X");
		//groundEntity->SetTexture("Res\\Mesh\\ground.png");

		Material groundMat;
		groundMat.Thickness = D3DXVECTOR4(0, 1, 1, 1);
		groundMat.MatIndex = 3;
		groundEntity->SetMaterial(&groundMat);

		D3DXMATRIX groundM;
		D3DXVECTOR3 groundMV = D3DXVECTOR3(-20, -0.5f, 0);
		D3DXMatrixTranslation(&groundM, groundMV.x, groundMV.y, groundMV.z);
		D3DXMATRIX groundS;
		D3DXVECTOR3 groundSV = D3DXVECTOR3(2, 2, 2);
		D3DXMatrixScaling(&groundS, groundSV.x, groundSV.y, groundSV.z);
		D3DXMATRIX groundRotMat;
		D3DXMatrixRotationY(&groundRotMat, -0.5f * D3DX_PI);
		groundEntity->SetWorldTransform(groundRotMat * groundS * groundM);
	}

	//==========================================================================================
	if (true)
	{

		bunnyEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\bunny.X");

		Material bunnyMat;
		bunnyMat.Thickness = D3DXVECTOR4(0.7, 1, 1, 1);
		bunnyMat.MatIndex = 1;
		bunnyEntity->SetMaterial(&bunnyMat);

		D3DXMATRIX bunnyM;
		D3DXVECTOR3 bunnyMV = D3DXVECTOR3(-12, 0, 20.5f);
		D3DXMatrixTranslation(&bunnyM, bunnyMV.x, bunnyMV.y, bunnyMV.z);
		D3DXVECTOR3 bunnySV = D3DXVECTOR3(0.5f, 0.5f, 0.5f);
		D3DXMATRIX bunnyS;
		D3DXMatrixScaling(&bunnyS, bunnySV.x, bunnySV.y, bunnySV.z);
		D3DXMATRIX bunnyRotMat;
		D3DXMatrixRotationY(&bunnyRotMat, -0.5f * D3DX_PI);
		bunnyEntity->SetWorldTransform(bunnyRotMat * bunnyS * bunnyM);
	}

	//==========================================================================================
	if (true)
	{
		rock1Entity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\rock\\rock1.X");
		//rock2Entity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\rock\\rock2.X");

		D3DXMATRIX rock1M;
		D3DXVECTOR3 rock1Move = D3DXVECTOR3(-30, -0.5f, 45);
		D3DXMatrixTranslation(&rock1M, rock1Move.x, rock1Move.y, rock1Move.z);
		D3DXMATRIX rock1S;
		D3DXVECTOR3 rock1SV = D3DXVECTOR3(0.3, 0.3, 0.3);
		D3DXMatrixScaling(&rock1S, rock1SV.x, rock1SV.y, rock1SV.z);
		D3DXMATRIX rock1RotMat;
		D3DXMatrixRotationY(&rock1RotMat, -0.5f * D3DX_PI);
		rock1Entity->SetWorldTransform(rock1RotMat * rock1S * rock1M);

	}
	

	//==========================================================================================
	if (true)
	{
		rock2Entity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\rock\\rock2.X");
		//rock2Entity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\rock\\rock2.X");

		Material rock2Mat;
		rock2Mat.Thickness = D3DXVECTOR4(1.5, 1, 1, 1);
		rock2Mat.MatIndex = 1;
		rock2Entity->SetMaterial(&rock2Mat);

		D3DXMATRIX rock2M;
		D3DXVECTOR3 rock2MV = D3DXVECTOR3(-35, -0.5f, 25);
		D3DXMatrixTranslation(&rock2M, rock2MV.x, rock2MV.y, rock2MV.z);
		D3DXMATRIX rock2S;
		D3DXVECTOR3 rock2SV = D3DXVECTOR3(0.3, 0.3, 0.3);
		D3DXMatrixScaling(&rock2S, rock2SV.x, rock2SV.y, rock2SV.z);
		D3DXMATRIX rock2RotMat;
		D3DXMatrixRotationY(&rock2RotMat, -0.5f * D3DX_PI);
		rock2Entity->SetWorldTransform(rock2RotMat * rock2S * rock2M);
	}

	//==========================================================================================
	if (true)
	{
		grass1Entity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\tree\\grass.X");

		Material grassMat;
		grassMat.Thickness = D3DXVECTOR4(0.4, 1, 1, 1);
		grass1Entity->SetMaterial(&grassMat);

		D3DXMATRIX grass1M;
		D3DXVECTOR3 grass1Move = D3DXVECTOR3(-30, -0.5f, 50);
		D3DXMatrixTranslation(&grass1M, grass1Move.x, grass1Move.y, grass1Move.z);
		D3DXMATRIX grass1S;
		D3DXVECTOR3 grass1SV = D3DXVECTOR3(0.2, 0.2, 0.2);
		D3DXMatrixScaling(&grass1S, grass1SV.x, grass1SV.y, grass1SV.z);
		D3DXMATRIX grass1RotMat;
		D3DXMatrixRotationY(&grass1RotMat, -0.8f * D3DX_PI);
		grass1Entity->SetWorldTransform(grass1RotMat * grass1S * grass1M);

		//==========================================================================================
		grass2Entity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\tree\\grass.X");
		grass2Entity->SetMaterial(&grassMat);

		D3DXMATRIX grass2M;
		D3DXVECTOR3 grass2Move = D3DXVECTOR3(-25, -0.5f, 47);
		D3DXMatrixTranslation(&grass2M, grass2Move.x, grass2Move.y, grass2Move.z);
		D3DXMATRIX grass2S;
		D3DXVECTOR3 grass2SV = D3DXVECTOR3(0.1, 0.1, 0.1);
		D3DXMatrixScaling(&grass2S, grass2SV.x, grass2SV.y, grass2SV.z);
		D3DXMATRIX grass2RotMat;
		D3DXMatrixRotationY(&grass2RotMat, -0.2f * D3DX_PI);
		grass2Entity->SetWorldTransform(grass2RotMat * grass2S * grass2M);

		//==========================================================================================
		grass3Entity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\tree\\grass.X");
		grass3Entity->SetMaterial(&grassMat);

		D3DXMATRIX grass3M;
		D3DXVECTOR3 grass3Move = D3DXVECTOR3(-30, -0.5f, 35);
		D3DXMatrixTranslation(&grass3M, grass3Move.x, grass3Move.y, grass3Move.z);
		D3DXMATRIX grass3S;
		D3DXVECTOR3 grass3SV = D3DXVECTOR3(0.17, 0.17, 0.17);
		D3DXMatrixScaling(&grass3S, grass3SV.x, grass3SV.y, grass3SV.z);
		D3DXMATRIX grass3RotMat;
		D3DXMatrixRotationY(&grass3RotMat, -0.5f * D3DX_PI);
		grass3Entity->SetWorldTransform(grass3RotMat * grass3S * grass3M);

	}
	//==========================================================================================
	if (false)
	{
		houseEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\city\\city.X");
		houseEntity->SetTexture("");
		D3DXMATRIX houseM;
		D3DXVECTOR3 houseMove = D3DXVECTOR3(-52, -20, 48.5f);
		D3DXMatrixTranslation(&houseM, houseMove.x, houseMove.y, houseMove.z);
		D3DXMATRIX houseS;
		D3DXVECTOR3 houseSV = D3DXVECTOR3(0.1, 0.1, 0.1);
		D3DXMatrixScaling(&houseS, houseSV.x, houseSV.y, houseSV.z);
		houseEntity->SetWorldTransform(houseS * houseM);

		Material houseMat;
		houseMat.Thickness = D3DXVECTOR4(0.4, 2, 0.5, 1);
		houseMat.MatIndex = 3;
		houseEntity->SetMaterial(&houseMat);
	}
	//==========================================================================================
	if (false)
	{
		chestEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\chest\\Chest.X");
		//chestEntity->SetTexture("Res\\Mesh\\chest\\chest.jpg");
		chestEntity->SetTexture("");
		D3DXMATRIX chestM;
		D3DXVECTOR3 chestMove = D3DXVECTOR3(-12, 0, 28.5f);
		D3DXMatrixTranslation(&chestM, chestMove.x, chestMove.y, chestMove.z);
		D3DXMATRIX chestS;
		D3DXVECTOR3 chestSV = D3DXVECTOR3(10, 10, 10);
		D3DXMatrixScaling(&chestS, chestSV.x, chestSV.y, chestSV.z);
		chestEntity->SetWorldTransform(chestS * chestM);

		Material chestMat;
		chestMat.Thickness = D3DXVECTOR4(0.5, 1, 1, 1);
		chestMat.MatIndex = 1;
		chestEntity->SetMaterial(&chestMat);
	}
	//==========================================================================================
	if (false)
	{

		Entity* cowEntity;
		cowEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\cow\\cow.X");
		//cowEntity->SetTexture("Res\\Mesh\\cow\\cow.jpg");
		cowEntity->SetTexture("");
		D3DXMATRIX cowM;
		D3DXVECTOR3 cowMove = D3DXVECTOR3(-12, 0, 22.5f);
		D3DXMatrixTranslation(&cowM, cowMove.x, cowMove.y, cowMove.z);
		D3DXMATRIX cowS;
		D3DXVECTOR3 cowSV = D3DXVECTOR3(1, 1, 1);
		D3DXMatrixScaling(&cowS, cowSV.x, cowSV.y, cowSV.z);
		cowEntity->SetWorldTransform(cowS * cowM);

		Material cowMat;
		cowMat.Thickness = D3DXVECTOR4(1, 1, 1, 1);
		cowMat.MatIndex = 1;
		cowEntity->SetMaterial(&cowMat);
	}
	//==========================================================================================
	if (false)
	{

		Entity* goatEntity;
		goatEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\anim\\goat.X");
		//goatEntity->SetTexture("Res\\Mesh\\goat\\goat.jpg");
		goatEntity->SetTexture("");
		D3DXMATRIX goatM;
		D3DXVECTOR3 goatMove = D3DXVECTOR3(-12, 0, 22.5f);
		D3DXMatrixTranslation(&goatM, goatMove.x, goatMove.y, goatMove.z);
		D3DXMATRIX goatS;
		D3DXVECTOR3 goatSV = D3DXVECTOR3(1, 1, 1);
		D3DXMatrixScaling(&goatS, goatSV.x, goatSV.y, goatSV.z);
		goatEntity->SetWorldTransform(goatS * goatM);

		Material goatMat;
		goatMat.Thickness = D3DXVECTOR4(1, 1, 1, 1);
		goatMat.MatIndex = 1;
		goatEntity->SetMaterial(&goatMat);
	}
	if (false)
	{

		Entity* craneEntity;
		craneEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\Anim\\crane.X");
		//craneEntity->SetTexture("Res\\Mesh\\crane\\crane.jpg");
		craneEntity->SetTexture("");
		D3DXMATRIX craneM;
		D3DXVECTOR3 craneMove = D3DXVECTOR3(-12, 0, 16.5f);
		D3DXMatrixTranslation(&craneM, craneMove.x, craneMove.y, craneMove.z);
		D3DXMATRIX craneS;
		D3DXVECTOR3 craneSV = D3DXVECTOR3(1, 1, 1);
		D3DXMatrixScaling(&craneS, craneSV.x, craneSV.y, craneSV.z);
		craneEntity->SetWorldTransform(craneS * craneM);

		Material craneMat;
		craneMat.Thickness = D3DXVECTOR4(0.7, 1, 1, 1);
		craneMat.MatIndex = 1;
		craneEntity->SetMaterial(&craneMat);
	}
	if (false)
	{

		Entity* EagleEntity;
		EagleEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\Anim\\Eagle.X");
		//EagleEntity->SetTexture("Res\\Mesh\\Eagle\\Eagle.jpg");
		EagleEntity->SetTexture("");
		D3DXMATRIX EagleM;
		D3DXVECTOR3 EagleMove = D3DXVECTOR3(-12, 0, 10.5f);
		D3DXMatrixTranslation(&EagleM, EagleMove.x, EagleMove.y, EagleMove.z);
		D3DXMATRIX EagleS;
		D3DXVECTOR3 EagleSV = D3DXVECTOR3(1, 1, 1);
		D3DXMatrixScaling(&EagleS, EagleSV.x, EagleSV.y, EagleSV.z);
		EagleEntity->SetWorldTransform(EagleS * EagleM);

		Material EagleMat;
		EagleMat.Thickness = D3DXVECTOR4(1, 1, 1, 1);
		EagleMat.MatIndex = 1;
		EagleEntity->SetMaterial(&EagleMat);
	}
	if (false)
	{

		Entity* HenEntity;
		HenEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\Anim\\Hen.X");
		//HenEntity->SetTexture("Res\\Mesh\\Hen\\Hen.jpg");
		HenEntity->SetTexture("");
		D3DXMATRIX HenM;
		D3DXVECTOR3 HenMove = D3DXVECTOR3(-12, 0, 4.5f);
		D3DXMatrixTranslation(&HenM, HenMove.x, HenMove.y, HenMove.z);
		D3DXMATRIX HenS;
		D3DXVECTOR3 HenSV = D3DXVECTOR3(1, 1, 1);
		D3DXMatrixScaling(&HenS, HenSV.x, HenSV.y, HenSV.z);
		HenEntity->SetWorldTransform(HenS * HenM);

		Material HenMat;
		HenMat.Thickness = D3DXVECTOR4(0.75, 1, 1, 1);
		HenMat.MatIndex = 1;
		HenEntity->SetMaterial(&HenMat);
	}
	if (false)
	{

		//skinnedmesh
		InitAllVertexDeclarations();

		mSkinnedMesh = new SkinnedMesh("Res\\DeerAnim\\deer5.x");
		//mSkinnedMesh = new SkinnedMesh("Res\\DeerAnim\\deer4.x");

		mSkinnedMesh->SetTexture("Res\\DeerAnim\\deer1.png");

		Material ADeerMat;
		ADeerMat.Thickness = D3DXVECTOR4(1, 1, 1, 1);
		ADeerMat.MatIndex = 0;
		mSkinnedMesh->SetMaterial(&ADeerMat);

		D3DXMATRIX ADeerM;
		D3DXVECTOR3 ADeerMV = D3DXVECTOR3(100, 220, 90);
		ADeerMV = D3DXVECTOR3(-15, -3, -2.0f);
		D3DXMatrixTranslation(&ADeerM, ADeerMV.x, ADeerMV.y, ADeerMV.z);
		D3DXVECTOR3 ADeerSV = D3DXVECTOR3(40.1f, 40.1f, 40.1f);
		D3DXMATRIX ADeerS;
		D3DXMatrixScaling(&ADeerS, ADeerSV.x, ADeerSV.y, ADeerSV.z);
		D3DXMATRIX ADeerRotMat;
		D3DXMatrixRotationY(&ADeerRotMat, 0.225f * D3DX_PI);
		mSkinnedMesh->SetWorldTransform(ADeerRotMat * ADeerS * ADeerM);
		RENDERPIPE::Instance().PushSkinnedMesh(mSkinnedMesh);

		mSkinnedMeshLittle = new SkinnedMesh("Res\\DeerAnim\\LittleDeer_1.x");
		//mSkinnedMeshLittle->SetTexture("Res\\DeerAnim\\deer1.png");

		Material ALittleDeerMat;
		ALittleDeerMat.Thickness = D3DXVECTOR4(0.65, 0.6, 1, 1);
		ALittleDeerMat.MatIndex = 2;
		mSkinnedMeshLittle->SetMaterial(&ALittleDeerMat);

		D3DXMATRIX ALittleDeerM;
		D3DXVECTOR3 ALittleDeerMV = D3DXVECTOR3(100, 220, 90);
		ALittleDeerMV = D3DXVECTOR3(-12.5, 0, -4.0f);
		D3DXMatrixTranslation(&ALittleDeerM, ALittleDeerMV.x, ALittleDeerMV.y, ALittleDeerMV.z);
		D3DXVECTOR3 ALittleDeerSV = D3DXVECTOR3(10.1f, 10.1f, 10.1f);
		D3DXMATRIX ALittleDeerS;
		D3DXMatrixScaling(&ALittleDeerS, ALittleDeerSV.x, ALittleDeerSV.y, ALittleDeerSV.z);
		D3DXMATRIX ALittleDeerRotMat;
		D3DXMatrixRotationY(&ALittleDeerRotMat, -1.225f * D3DX_PI);
		mSkinnedMeshLittle->SetWorldTransform(ALittleDeerRotMat * ALittleDeerS * ALittleDeerM);
		RENDERPIPE::Instance().PushSkinnedMesh(mSkinnedMeshLittle);
	}
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
	if (!GAMEINPUT::Instance().m_CurMState.right)
		return;

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
	float speed = 0.2f;
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
float R2 = 0;
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

	D3DXMATRIX rotMat;

	move = D3DXVECTOR3(-12, 0, 48.5f);
	D3DXMatrixTranslation(&moveMat, move.x, move.y, move.z);
	D3DXMatrixRotationY(&rotMat, (180 + R2) / 180.0f * D3DX_PI);
	D3DXMATRIX deer2S;
	D3DXVECTOR3 deer2SV = D3DXVECTOR3(0.1, 0.1, 0.1);
	D3DXMatrixScaling(&deer2S, deer2SV.x, deer2SV.y, deer2SV.z);
	R2 += 1.0f;
	rotMat *= moveMat;
	if (deerEntity2)
		deerEntity2->SetWorldTransform(deer2S * rotMat);

	//-------------------------------------------
	double dTime = GLOBALTIMER::Instance().GetFrameTime();
	if (mSkinnedMesh)
		mSkinnedMesh->update(dTime);
	if (mSkinnedMeshLittle)
		mSkinnedMeshLittle->update(dTime);
}

void TestScene::OnFrame()
{
	mainCamera.OnFrame();
}

void TestScene::OnEndFrame()
{

}
