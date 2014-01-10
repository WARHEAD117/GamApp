#pragma once
#include "CommonUtil\\D3D9Header.h"

class Camera
{
public:
	Camera();
	~Camera();

	void Init(); //��ʼ��
	void		SetWorldTransform(D3DXMATRIX matrix);
	D3DXMATRIX	GetWorldTransform();

	void OnFrame();

	void BuildViewMtx();
	void BuildProjMtx();

	D3DXMATRIX GetView(){ return mView; };
	D3DXMATRIX GetProj(){ return mProj; };
	D3DXMATRIX GetWorld(){ return mWorld; };

	D3DXVECTOR4 GetPos4() { return D3DXVECTOR4(mPosW, 1.0f); };
	D3DXVECTOR3 GetPos(){ return mPosW; };
	D3DXVECTOR3 GetRight(){ return mRightW; };
	D3DXVECTOR3 GetUp(){ return mUpW; };
	D3DXVECTOR3 GetLookAt(){ return mLookAtW; };
	D3DXVECTOR3 GetLookDir(){ return mLookAtW - mPosW; };

	D3DXMATRIX mView;   //�۲����
	D3DXMATRIX mProj;     //ͶӰ����
	D3DXMATRIX mWorld;  //�������

private:
	D3DXMATRIX	mWorldTransform;

	D3DXVECTOR3 mPosW;
	D3DXVECTOR3 mRightW;
	D3DXVECTOR3 mUpW;
	D3DXVECTOR3 mLookAtW;
	//	D3DXVECTOR3 mLookDirW;
	float mFOV;
};