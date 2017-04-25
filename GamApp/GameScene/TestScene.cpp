#include "TestScene.h"
#include "EntityFeature/Entity.h"
#include "EntityFeature/SkinEntity.h"
#include "CommonUtil/Timer/GlobalTimer.h"
#include "Camera\CameraParam.h"
#include "CommonUtil/Input/Input.h"

#include "Light/DirectionLight.h"
#include "Light/PointLight.h"
#include "Light/SpotLight.h"
#include "Light/ImageBasedLight.h"
#include "Light/LightManager.h"

#include "RenderSystem/TempSkin/SkinnedMesh.h"
//should done in skin component
#include "RenderSystem/TempSkin/Vertex.h"
//should done in Entity!
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
Entity* sponzaEntity;
Material testMat1;


SkinEntity* mSkinnedMesh;
SkinEntity* mSkinnedMeshLittle;

EffectLoader effectLoader;

ImageBasedLight* iblLight;
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
void TestScene::OnLoad()
{
	//===================================================================================================
	//BuildCamera
	mainCamera.Init();
	
	//Create Entity
	//krisEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\Cube.X");
	/*
	krisEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile<Entity>("Res\\Mesh\\kris_sheva\\kris.X");
	{
		krisEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\378DAAED_SPEC.png", 0);
		krisEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\3D9BBDC1_SPEC.png", 1);
		krisEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\4240D889_SPEC.png", 2);
		krisEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\292B0322_SPEC.png", 7);
		krisEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\47732EA1_SPEC.png", 11);
		krisEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\F0F64CA9_SPEC.png", 13);
		krisEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\ED3E42C6_SPEC.png", 14);
		krisEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\566EC93F_SPEC.png", 16);
	}
	//shevaEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile("Res\\Mesh\\teapot.X");
	shevaEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile<Entity>("Res\\Mesh\\kris_sheva\\sheva.X");
	{
		shevaEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\0A370D7F_SPEC.png", 0);
		shevaEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\174D3E25_SPEC.png", 1);
		shevaEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\E2813884_SPEC.png", 2);
		shevaEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\D10453BE_SPEC.png", 3);
		shevaEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\5120C7F9_SPEC.png", 4);
		shevaEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\BF40F1B9_SPEC.png", 6);
		shevaEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\6F98F51D_SPEC.png", 10);
		shevaEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\168C52A9_SPEC.png", 12);
		shevaEntity->SetSpecularMap("Res\\Mesh\\kris_sheva\\6D714412_SPEC.png", 14);
	}*/
	//testEntity.SetMeshFileName("Res\\Mesh\\car\\car25.X");
	//testEntity.SetMeshFileName("Res\\Mesh\\tree3\\tree3.X");
	
	if (true)
	{
		sponzaEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile<Entity>("Res\\Mesh\\sponza\\sponza.X");
		
		if (true)
		{
			sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_floor_a_spec.tga", 0);

			sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\spnza_bricks_a_ddn.tga", 1);
			sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\spnza_bricks_a_spec.tga", 1);

			sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\lion_ddn.tga", 3);

			sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\background_ddn.tga", 4);

			sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\vase_ddn.tga", 5);

			sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\vase_plant_spec.tga", 6);

			sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\vase_round_ddn.tga", 7);
			sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\vase_round_spec.tga", 7);

			sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\vase_round_ddn.tga", 8);
			sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\vase_round_spec.tga", 8);

			sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\chain_texture_ddn.tga", 9);

			sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_curtain_spec.tga", 10);
			sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_curtain_spec.tga", 11);
			sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_curtain_spec.tga", 12);

			sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_flagpole_spec.tga", 13);

			sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_fabric_spec.tga", 14);
			sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_fabric_spec.tga", 15);
			sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_fabric_spec.tga", 16);

			sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\sponza_thorn_ddn.tga", 17);
			sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_thorn_spec.tga", 17);

			sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\sponza_column_b_ddn.tga", 18);
			sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_column_b_spec.tga", 18);

			sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\sponza_arch_ddn.tga", 19);
			sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_arch_spec.tga", 19);

			sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\sponza_column_a_ddn.tga", 20);
			sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_column_a_spec.tga", 20);

			sponzaEntity->SetNormalMap("Res\\Mesh\\sponza\\sponza_column_c_ddn.tga", 21);
			sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_column_c_spec.tga", 21);

			sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_details_spec.tga", 22);

			sponzaEntity->SetSpecularMap("Res\\Mesh\\sponza\\sponza_ceiling_a_spec.tga", 23);
		}

		//sponzaEntity->SetTexture("");

		Material sponzaMat;
		sponzaMat.Power = 1.0;
		sponzaEntity->SetMaterial(&sponzaMat);
	}
	
	//Entity* roomEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile<Entity>("Res\\Mesh\\room\\room.x");
	//roomEntity->SetTexture("Res\\Mesh\\room\\wallpaper.dds",0);
	//roomEntity->SetTexture("Res\\Mesh\\room\\DoorDiff.dds", 1);
	//roomEntity->SetTexture("Res\\Mesh\\room\\oldwood.dds", 2);
	//roomEntity->SetTexture("Res\\Mesh\\room\\FireplaceDiff.dds", 3);

	//D3DXMATRIX planeM;
	//D3DXVECTOR3 move = D3DXVECTOR3(0, -5, 0);
	//D3DXMatrixTranslation(&planeM, move.x, move.y, move.z); 
	//roomEntity->SetWorldTransform(planeM);

	
	Entity* DofMesh = ENTITYMANAGER::Instance().CreateEntityFromXFile<Entity>("Res\\Mesh\\DOFMesh.X");
	DofMesh->SetTexture("Res\\Mesh\\trans.png");
	DofMesh->SetWorldTranslate(-6, 0, 0);
	DofMesh->SetWorldScale(0.05f, 0.03f, 0.03f);
	DofMesh->SetWorldRotation(0, -0.5f * D3DX_PI, 0);
	

	for (int i = 0; i < 10; i++)
	{
		Entity* SphereMesh = ENTITYMANAGER::Instance().CreateEntityFromXFile<Entity>("Res\\Mesh\\Sphere.X");
		SphereMesh->SetWorldTranslate(0, 0.5, (-i/10.0f +0.5f) * 6);
		SphereMesh->SetWorldScale(0.03f, 0.03f, 0.03f);
		SphereMesh->SetWorldRotation(0, -0.5f * D3DX_PI, 0);
		Material SphereMat;
		SphereMat.Power = i / 10.0f;
		SphereMesh->SetMaterial(&SphereMat);
	}

	/*

	horseEntity = ENTITYMANAGER::Instance().CreateEntityFromXFile<Entity>("Res\\Mesh\\horse\\horse.X");
	horseEntity->SetTexture("Res\\Mesh\\horse\\HorseB_512.jpg");
	horseEntity->SetNormalMap("Res\\Mesh\\horse\\HorseB _NRM_512.jpg");
	horseEntity->SetWorldScale(0.07f, 0.07f, 0.07f);


	//skinnedmesh
	InitAllVertexDeclarations();

	mSkinnedMesh = ENTITYMANAGER::Instance().CreateEntityFromXFile<SkinEntity>("Res\\Mesh\\DeerAnim\\deer5.x");
	mSkinnedMesh->SetTexture("Res\\Mesh\\DeerAnim\\lu_tex.jpg");
	mSkinnedMesh->SetWorldTranslate(-15, -2.2f, -2.0f);
	mSkinnedMesh->SetWorldScale(28, 28, 28);
	mSkinnedMesh->SetWorldRotation(0, 0.225f * D3DX_PI, 0);

	mSkinnedMeshLittle = ENTITYMANAGER::Instance().CreateEntityFromXFile<SkinEntity>("Res\\Mesh\\DeerAnim\\LittleDeer_1.x");
	//mSkinnedMeshLittle->SetTexture("Res\\Mesh\\DeerAnim\\lu_tex.jpg");
	mSkinnedMeshLittle->SetWorldTranslate(-12.5, -0.1f, -4.0f);
	mSkinnedMeshLittle->SetWorldScale(7, 7, 7);
	mSkinnedMeshLittle->SetWorldRotation(0, -1.225f * D3DX_PI, 0);
	*/


	/*
	SkinEntity* BallBot;
	BallBot = ENTITYMANAGER::Instance().CreateEntityFromXFile<SkinEntity>("Res\\Mesh\\BallBot\\BallBot.x");
	//mSkinnedMeshLittle->SetTexture("Res\\DeerAnim\\deer1.png");
	BallBot->SetWorldTranslate(-10, 0, -5.0f);
	BallBot->SetWorldScale(1.0f, 1.0f, 1.0f);
	BallBot->SetWorldRotation(0, -1.225f * D3DX_PI, 0);
	*/


	//----------------------------------------------------------
	bool useIbl = false;
	useIbl = true;
	if (useIbl)
	{
		iblLight = LIGHTMANAGER::Instance().CreateLight<ImageBasedLight>(eImageBasedLight);
		iblLight->SetLightProbe("Res\\Sky\\s\\uffizi-large.hdr");
	}
	//--------------------------------------------------------------------------
	dirLight1 = LIGHTMANAGER::Instance().CreateLight<DirectionLight>(eDirectionLight);
	dirLight1->SetLightColor(D3DXCOLOR(15.0f, 14.0f, 12.7f, 1.0f));
	dirLight1->SetShadowAreaSize(120, 120);
	dirLight1->SetUseShadow(true);
	dirLight1->SetWorldTranslate(0, 70, 0);
	dirLight1->SetWorldRotation(0.055f * D3DX_PI, 0.125f * D3DX_PI, 0);
	//--------------------------------------------------------------------------
	/*
	dirLight2 = LIGHTMANAGER::Instance().CreateLight<DirectionLight>(eDirectionLight);
	dirLight2->SetLightColor(D3DXCOLOR(1.0f, 1.0f, 1.3f, 1.0f));
	dirLight2->SetUseShadow(true);
	dirLight2->SetShadowAreaSize(150,150);
	D3DXMatrixTranslation(&lightMoveMat, 0, 60, 10);
	D3DXMatrixRotationX(&lightRot1Mat, 0.125f * D3DX_PI);
	D3DXMatrixRotationY(&lightRot2Mat, 0.125f * D3DX_PI);
	dirLight2->SetWorldTransform(lightRot1Mat*lightRot2Mat*lightMoveMat);
	*/
	//--------------------------------------------------------------------------
	pointLight0 = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);
	pointLight0->SetLightColor(D3DXCOLOR(0.01f, 0.01f, 0.01f, 1.0f));
	pointLight0->SetUseShadow(false);
	pointLight0->SetLightRange(100);
	//dirLight3->SetLightAttenuation(D3DXVECTOR4(0,0,0,0));
	pointLight0->SetWorldTranslate(0, 10, 0);
	pointLight0->SetWorldRotation(0, 0, 0);
	//--------------------------------------------------------------------------
	/*
	pointLight1 = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);
	pointLight1->SetLightColor(D3DXCOLOR(01.7f, 01.7f, 01.7f, 1.0f));
	pointLight1->SetUseShadow(true);
	pointLight1->SetLightRange(40);
	
	D3DXMatrixTranslation(&lightMoveMat, -5, 3, 0);
	pointLight1->SetWorldTransform(lightMoveMat);
	*/
	//--------------------------------------------------------------------------
	pointLight2 = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);
	pointLight2->SetLightColor(D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f));
	pointLight2->SetUseShadow(false);
	pointLight2->SetLightRange(2);
	pointLight2->SetWorldTranslate(20.0001f, -4, 9);

	//--------------------------------------------------------------------------
	pointLight3 = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);
	pointLight3->SetLightColor(D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f));
	pointLight3->SetUseShadow(false);
	pointLight3->SetLightRange(2);
	pointLight3->SetWorldTranslate(19.995f, -4, 10);

	//--------------------------------------------------------------------------
	/*
	pointLight4 = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);
	pointLight4->SetLightColor(D3DXCOLOR(3.7f, 3.7f, 3.7f, 1.0f));
	pointLight4->SetUseShadow(false);
	pointLight4->SetLightRange(2);

	D3DXMatrixTranslation(&lightMoveMat, 10, -4, 19.998f);
	pointLight4->SetWorldTransform(lightMoveMat);
	*/
	//--------------------------------------------------------------------------
	/*
	pointLight5 = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);
	pointLight5->SetLightColor(D3DXCOLOR(4.7f, 0.7f, 4.7f, 1.0f));
	pointLight5->SetUseShadow(false);
	pointLight5->SetLightRange(0.3f);

	D3DXMatrixTranslation(&lightMoveMat, 10.0f, -4, -19.998f);
	pointLight5->SetWorldTransform(lightMoveMat);
	
	//--------------------------------------------------------------------------
	spotLight1 = LIGHTMANAGER::Instance().CreateLight<SpotLight>(eSpotLight);
	spotLight1->SetLightRange(20);
	spotLight1->SetUseShadow(false);
	spotLight1->SetLightColor(D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f));
	spotLight1->SetWorldTranslate(5, 5, 5);
	
	//--------------------------------------------------------------------------
	spotLight2 = LIGHTMANAGER::Instance().CreateLight<SpotLight>(eSpotLight);
	spotLight2->SetLightAngle(D3DXVECTOR2(60, 30));
	spotLight2->SetLightRange(45);
	spotLight2->SetUseShadow(false);
	spotLight2->SetLightColor(D3DXCOLOR(0.0f, 0.1f, 0.0f, 1.0f));
	spotLight2->SetWorldRotation(-0.4f * D3DX_PI, 0, 0);
	spotLight2->SetWorldTranslate(2, 10, -5);
	//--------------------------------------------------------------------------
	
	spotLight3 = LIGHTMANAGER::Instance().CreateLight<SpotLight>(eSpotLight);
	spotLight3->SetLightRange(2);
	spotLight3->SetLightColor(D3DXCOLOR(1.5, 1.5, 0.0, 1.0f));
	spotLight3->SetUseShadow(false);
	spotLight3->SetLightRange(20);
	spotLight3->SetWorldRotation(0.25f * D3DX_PI, 0, 0);
	spotLight3->SetWorldTranslate(-10, 4, 5);
	//--------------------------------------------------------------------------
	spotLight4 = LIGHTMANAGER::Instance().CreateLight<SpotLight>(eSpotLight);
	spotLight4->SetLightRange(20); 
	spotLight4->SetUseShadow(false);
	spotLight4->SetLightColor(D3DXCOLOR(0.0, 1.5, 1.5, 1.0f));
	spotLight4->SetWorldTranslate(-10, 4, -5);
	spotLight4->SetWorldRotation(-0.25f * D3DX_PI, 0, 0);
	*/
	if (true)
	{
		spotLight5 = LIGHTMANAGER::Instance().CreateLight<SpotLight>(eSpotLight);
		spotLight5->SetLightRange(100);
		spotLight5->SetUseShadow(true);
		spotLight5->SetLightColor(D3DXCOLOR(1.0, 0.2, 0.0, 1.0f));
		spotLight5->SetWorldTranslate(0, 50, 0);
		spotLight5->SetWorldRotation(0, 0, 0);
	}

	for (int i = 0; i < 50; i++)
	{
		//--------------------------------------------------------------------------
		PointLight* testLight;
		testLight = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);
		
		testLight->SetLightColor(D3DXCOLOR(i / 50.0f, (50 - i) / 50.0f, (50 - i) / 50.0f, 1.0f));
		testLight->SetUseShadow(false);
		testLight->SetLightRange(2);
		testLight->SetWorldTranslate(3 * (-25 + i), 1.5, 10);
	}

	for (int i = 0; i < 50; i++)
	{
		//--------------------------------------------------------------------------
		PointLight* testLight;
		testLight = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);

		testLight->SetLightColor(D3DXCOLOR(0.0f, 0.0f, (50 - i) / 50.0f, 1.0f));
		testLight->SetUseShadow(false);
		testLight->SetLightRange(2);
		testLight->SetWorldTranslate(3 * (-25 + i), 0.5, 13);
	}

	for (int i = 0; i < 50; i++)
	{
		//--------------------------------------------------------------------------
		PointLight* testLight;
		testLight = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);

		testLight->SetLightColor(D3DXCOLOR((50 - i) / 50.0f, i / 50.0f, 0.0f, 1.0f));
		testLight->SetUseShadow(false);
		testLight->SetLightRange(2);
		testLight->SetWorldTranslate(3 * (-25 + i), 0.5, 16);
	}
	{
		
	}
	{
		//--------------------------------------------------------------------------
		PointLight* testLight;
		testLight = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);
		testLight->SetLightColor(D3DXCOLOR(0.7f, 1.7f, 0.7f, 1.0f));
		testLight->SetUseShadow(false);
		testLight->SetLightRange(2);
		testLight->SetWorldTranslate(17.995f, 0.5, 10);

	}
	{
		//--------------------------------------------------------------------------
		PointLight* testLight;
		testLight = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);
		testLight->SetLightColor(D3DXCOLOR(1.7f, 0.7f, 0.7f, 1.0f));
		testLight->SetUseShadow(false);
		testLight->SetLightRange(2);
		testLight->SetWorldTranslate(15.995f, 0.5, 10);
	}

	{
		//--------------------------------------------------------------------------
		PointLight* testLight;
		testLight = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);
		testLight->SetLightColor(D3DXCOLOR(0.7f, 0.7f, 1.7f, 1.0f));
		testLight->SetUseShadow(false);
		testLight->SetLightRange(2);
		testLight->SetWorldTranslate(14.995f, 0.5, 10);
	}

	{
		//--------------------------------------------------------------------------
		PointLight* testLight;
		testLight = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);
		testLight->SetLightColor(D3DXCOLOR(0.7f, 1.7f, 0.7f, 1.0f));
		testLight->SetUseShadow(false);
		testLight->SetLightRange(2);
		testLight->SetWorldTranslate(13.995f, 0.5, 10);
	}

	{
		//--------------------------------------------------------------------------
		PointLight* testLight;
		testLight = LIGHTMANAGER::Instance().CreateLight<PointLight>(ePointLight);
		testLight->SetLightColor(D3DXCOLOR(1.7f, 0.7f, 0.7f, 1.0f));
		testLight->SetUseShadow(false);
		testLight->SetLightRange(2);
		testLight->SetWorldTranslate(12.995f, 0.5, 10);
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

	double dTime = GLOBALTIMER::Instance().GetFrameTime();
	R += 5.0f * dTime;

	if (krisEntity)
	{
		krisEntity->SetWorldTranslate(0, -5, -5);
		krisEntity->SetWorldRotation(0, (180 + R) / 180.0f * D3DX_PI, 0);
	}

	if (shevaEntity)
	{
		shevaEntity->SetWorldTranslate(0, 12.5, -2.7);
		shevaEntity->SetWorldRotation(0.25f * D3DX_PI, 0.07f * D3DX_PI, 0);
	}
}

void TestScene::OnFrame()
{
	mainCamera.OnFrame();
}

void TestScene::OnEndFrame()
{

}
