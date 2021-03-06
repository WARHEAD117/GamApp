//#define DEBUG_PS
//#define DEBUG_VS

#include "D3D9Device.h"

static std::string defaultEffectName = "System\\commonDiffuse.fx";
static std::string diffuseEffectName = "System\\DeferredDiffuse.fx";
static std::string normalEffectName = "System\\DeferredNormal.fx";
static std::string defaultTextureName = "System\\white.dds";
static std::string defaultNormalMapName = "System\\defaultNormal.png";
static std::string positionEffectName = "System\\DeferredPosition.fx";
static std::string shadowEffectName = "System\\DeferredShadowVSM.fx";

D3D9Device::D3D9Device()
{
	ZeroMemory(&g_pD3DPP, sizeof(g_pD3DPP));
}


D3D9Device::~D3D9Device()
{
	SafeRelease(g_pD3D);
	SafeRelease(g_pD3DDevice);
	SafeRelease(defaultEffect);
	SafeRelease(normalEffect);
	SafeRelease(defaultTexture);
}

void D3D9Device::InitD3DDevice(HWND hWnd)
{
	// Step 1: Create the IDirect3D9 object.

	g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!g_pD3D)
	{
		MessageBox(0, "Direct3DCreate9 FAILED", 0, 0);
		PostQuitMessage(0);
	}

	// Step 2: Verify hardware support for specified formats in windowed and full screen modes.
	D3DDEVTYPE devType = D3DDEVTYPE_HAL;
	DWORD requestedVP = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	D3DDISPLAYMODE mode;
	g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
	HR(g_pD3D->CheckDeviceType(D3DADAPTER_DEFAULT, devType, mode.Format, mode.Format, true));
	HR(g_pD3D->CheckDeviceType(D3DADAPTER_DEFAULT, devType, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, false));

	// Step 3: Check for requested vertex processing and pure device.

	D3DCAPS9 caps;
	HR(g_pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, devType, &caps));

	DWORD devBehaviorFlags = 0;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		devBehaviorFlags |= requestedVP;
	else
		devBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	// If pure device and HW T&L supported
	if (caps.DevCaps & D3DDEVCAPS_PUREDEVICE &&
		devBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING)
		devBehaviorFlags |= D3DCREATE_PUREDEVICE;

	// Step 4: Fill out the D3DPRESENT_PARAMETERS structure.

	g_pD3DPP.BackBufferWidth = 0;
	g_pD3DPP.BackBufferHeight = 0;
	g_pD3DPP.BackBufferFormat = D3DFMT_UNKNOWN;
	g_pD3DPP.BackBufferCount = 1;
	g_pD3DPP.MultiSampleType = D3DMULTISAMPLE_NONE;
	g_pD3DPP.MultiSampleQuality = 0;
	g_pD3DPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_pD3DPP.hDeviceWindow = hWnd;
	g_pD3DPP.Windowed = true;
	g_pD3DPP.EnableAutoDepthStencil = true;
	g_pD3DPP.AutoDepthStencilFormat = D3DFMT_D24S8;
	g_pD3DPP.Flags = 0;
	g_pD3DPP.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	g_pD3DPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;


	// Step 5: Create the device.

	HR(g_pD3D->CreateDevice(
		D3DADAPTER_DEFAULT, // primary adapter
		devType,           // device type
		hWnd,          // window associated with device
		devBehaviorFlags,   // vertex processing
		&g_pD3DPP,            // present parameters
		&g_pD3DDevice));      // return created device

	//SetDefaultState
	g_pD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
}

LPD3DXEFFECT D3D9Device::GetDefaultEffect()
{
	if (!defaultEffect)
	{
		ID3DXBuffer* error = 0;
		if (E_FAIL == ::D3DXCreateEffectFromFile(g_pD3DDevice, defaultEffectName.c_str(), NULL, NULL, D3DXSHADER_DEBUG,
			NULL, &defaultEffect, &error))
		{
			MessageBox(GetForegroundWindow(), (char*)error->GetBufferPointer(), "Shader", MB_OK);
			abort();
		}
	}

	return defaultEffect;
}

LPD3DXEFFECT D3D9Device::GetDiffuseEffect()
{
	if (!diffuseEffect)
	{
		ID3DXBuffer* error = 0;
		if (E_FAIL == ::D3DXCreateEffectFromFile(g_pD3DDevice, diffuseEffectName.c_str(), NULL, NULL, D3DXSHADER_DEBUG,
			NULL, &diffuseEffect, &error))
		{
			MessageBox(GetForegroundWindow(), (char*)error->GetBufferPointer(), "Shader", MB_OK);
			abort();
		}
	}

	return diffuseEffect;
}

LPD3DXEFFECT D3D9Device::GetNormalEffect()
{
	if (!normalEffect)
	{
		ID3DXBuffer* error = 0;
		if (E_FAIL == ::D3DXCreateEffectFromFile(g_pD3DDevice, normalEffectName.c_str(), NULL, NULL, D3DXSHADER_DEBUG,
			NULL, &normalEffect, &error))
		{
			MessageBox(GetForegroundWindow(), (char*)error->GetBufferPointer(), "Shader", MB_OK);
			abort();
		}
	}

	return normalEffect;
}

LPD3DXEFFECT D3D9Device::GetPositionEffect()
{
	if (!positionEffect)
	{
		ID3DXBuffer* error = 0;
		if (E_FAIL == ::D3DXCreateEffectFromFile(g_pD3DDevice, positionEffectName.c_str(), NULL, NULL, D3DXSHADER_DEBUG,
			NULL, &positionEffect, &error))
		{
			MessageBox(GetForegroundWindow(), (char*)error->GetBufferPointer(), "Shader", MB_OK);
			abort();
		}
	}

	return positionEffect;
}

LPD3DXEFFECT D3D9Device::GetShadowEffect()
{
	if (!shadowEffect)
	{
		ID3DXBuffer* error = 0;
		if (E_FAIL == ::D3DXCreateEffectFromFile(g_pD3DDevice, shadowEffectName.c_str(), NULL, NULL, D3DXSHADER_DEBUG,
			NULL, &shadowEffect, &error))
		{
			MessageBox(GetForegroundWindow(), (char*)error->GetBufferPointer(), "Shader", MB_OK);
			abort();
		}
	}

	return shadowEffect;
}

LPDIRECT3DTEXTURE9 D3D9Device::GetDefaultTexture()
{
	if (!defaultTexture)
	{
		if (FAILED(D3DXCreateTextureFromFile(g_pD3DDevice, defaultTextureName.c_str(), &defaultTexture)))
		{
			MessageBox(GetForegroundWindow(), "Can't find default texture", "Texture", MB_OK);
			abort();
		}
	}
	
	return defaultTexture;
}

LPDIRECT3DTEXTURE9 D3D9Device::GetDefaultNormalMap()
{
	if (!defaultNormalMap)
	{
		if (FAILED(D3DXCreateTextureFromFile(g_pD3DDevice, defaultNormalMapName.c_str(), &defaultNormalMap)))
		{
			MessageBox(GetForegroundWindow(), "Can't find default texture", "Texture", MB_OK);
			abort();
		}
	}

	return defaultNormalMap;
}

Material* D3D9Device::GetDefaultMaterial()
{
	defaultMaterial.effect = GetDefaultEffect();
	return &defaultMaterial;
}

