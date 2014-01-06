#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <dinput.h>
#include <dxerr.h>

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")
#pragma comment(lib,"dxerr.lib")

#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dinput8.lib")
//===============================================================
// Debug
#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)                                      \
	{                                                  \
	HRESULT hr = x;                                \
if (FAILED(hr))                                 \
		{                                              \
		DXTrace(__FILE__, __LINE__, hr, #x, TRUE); \
		}                                              \
	}
#endif

#else
#ifndef HR
#define HR(x) x;
#endif
#endif 

//=================================================================
#define ZEROVECTOR3	D3DXVECTOR3(0.0f,0.0f,0.0f)