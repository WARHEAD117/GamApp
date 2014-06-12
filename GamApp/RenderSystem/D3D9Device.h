#pragma once
#include "CommonUtil\Singleton.h"
#include "CommonUtil\D3D9Header.h"
#include "CommonUtil\GlobalHeader.h"

class D3D9Device
{
public:
	D3D9Device();
	~D3D9Device();

	void InitD3DDevice(HWND hWnd);
	LPD3DXEFFECT GetDefaultEffect();
	LPD3DXEFFECT GetDiffuseEffect();
	LPD3DXEFFECT GetPositionEffect();
	LPD3DXEFFECT GetNormalDepthEffect();
	LPDIRECT3DTEXTURE9 GetDefaultTexture();
	Material* GetDefaultMaterial();

	LPDIRECT3D9								g_pD3D;       //--Direct3D对象指针
	LPDIRECT3DDEVICE9						g_pD3DDevice; //--Direct3D的设备
	D3DPRESENT_PARAMETERS					g_pD3DPP;

	Material								defaultMaterial;
	LPD3DXEFFECT							defaultEffect;
	LPD3DXEFFECT							diffuseEffect;
	LPD3DXEFFECT							normalDepthEffect;
	LPD3DXEFFECT							positionEffect;
	LPDIRECT3DTEXTURE9						defaultTexture;

	D3DXMATRIX								ViewMatrix;
	D3DXMATRIX								ProjMatrix;
	D3DXVECTOR3								ViewPosition;
};

typedef CSingleton<D3D9Device> RENDERDEVICE;