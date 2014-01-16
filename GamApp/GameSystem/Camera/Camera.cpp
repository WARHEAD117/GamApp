#include "Camera.h"
#include "D3D9Device.h"
#include "Camera/CameraParam.h"

Camera::Camera(void)
{
	D3DXMatrixIdentity(&mView);
	D3DXMatrixIdentity(&mProj);
	D3DXMatrixIdentity(&mWorld);

	mPosW = CameraParam::posW;
	mRightW = CameraParam::rightW;
	mUpW = CameraParam::upW;
	mLookAtW = CameraParam::lookAtW;

	mFOV = CameraParam::FOV;
}
Camera::~Camera(void)
{

}

void Camera::Init()
{
	D3DXMatrixTranslation(&mWorld, mPosW.x, mPosW.y, mPosW.z);
	BuildViewMtx();
	BuildProjMtx();
}

void Camera::OnFrame()
{
	D3DXMatrixInverse(&RENDERDEVICE::Instance().ViewMatrix, NULL, &mWorldTransform);
	RENDERDEVICE::Instance().ViewPosition = D3DXVECTOR3(mWorldTransform._41, mWorldTransform._42, mWorldTransform._43);
}

void Camera::BuildViewMtx()
{
// 	D3DXVECTOR3 mLookDirW = GetLookDir();
// 
// 	// Keep camera's axes orthogonal to each other and of unit length.
// 	D3DXVec3Normalize(&mLookDirW, &mLookDirW);
// 
// 	D3DXVec3Cross(&mUpW, &mLookDirW, &mRightW);
// 	D3DXVec3Normalize(&mUpW, &mUpW);
// 
// 	D3DXVec3Cross(&mRightW, &mUpW, &mLookDirW);
// 	D3DXVec3Normalize(&mRightW, &mRightW);
// 
// 	// Fill in the view matrix entries.
// 
// 	float x = -D3DXVec3Dot(&mPosW, &mRightW);
// 	float y = -D3DXVec3Dot(&mPosW, &mUpW);
// 	float z = -D3DXVec3Dot(&mPosW, &mLookDirW);
// 
// 	mView(0, 0) = mRightW.x;
// 	mView(1, 0) = mRightW.y;
// 	mView(2, 0) = mRightW.z;
// 	mView(3, 0) = x;
// 
// 	mView(0, 1) = mUpW.x;
// 	mView(1, 1) = mUpW.y;
// 	mView(2, 1) = mUpW.z;
// 	mView(3, 1) = y;
// 
// 	mView(0, 2) = mLookDirW.x;
// 	mView(1, 2) = mLookDirW.y;
// 	mView(2, 2) = mLookDirW.z;
// 	mView(3, 2) = z;
// 
// 	mView(0, 3) = 0.0f;
// 	mView(1, 3) = 0.0f;
// 	mView(2, 3) = 0.0f;
// 	mView(3, 3) = 1.0f;

	// 	D3DXMATRIX   matView;
	D3DXMatrixLookAtLH(&mView, &CameraParam::posW,
		&CameraParam::lookAtW,
		&CameraParam::upW);
	RENDERDEVICE::Instance().ViewMatrix = mView;
	D3DXMatrixInverse(&mWorldTransform, NULL, &mView);
}

void Camera::BuildProjMtx()
{

	float w = (float)RENDERDEVICE::Instance().g_pD3DPP.BackBufferWidth;
	float h = (float)RENDERDEVICE::Instance().g_pD3DPP.BackBufferHeight;

	D3DXMatrixPerspectiveFovLH(&mProj, mFOV, w / h, 1.0f, 5000.0f);

	RENDERDEVICE::Instance().ProjMatrix = mProj;
}
