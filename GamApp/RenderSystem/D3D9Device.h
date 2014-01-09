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

	LPDIRECT3D9								g_pD3D;       //--Direct3D对象指针
	LPDIRECT3DDEVICE9						g_pD3DDevice; //--Direct3D的设备
	D3DPRESENT_PARAMETERS					g_pD3DPP;

	LPD3DXEFFECT							defaultEffect;

	D3DXMATRIX								ViewMatrix;
	D3DXMATRIX								ProjMatrix;
};

typedef CSingleton<D3D9Device> RENDERDEVICE;