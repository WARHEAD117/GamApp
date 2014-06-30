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
#define ZEROVECTOR4	D3DXVECTOR4(0.0f,0.0f,0.0f)
//=================================================================
struct Material
{
	D3DXVECTOR4   Diffuse;        /* Diffuse color RGBA */
	D3DXVECTOR4   Ambient;        /* Ambient color RGB */
	D3DXVECTOR4   Specular;       /* Specular 'shininess' */
	D3DXVECTOR4   Emissive;       /* Emissive color RGB */
	float           Power;          /* Sharpness if specular highlight */

	LPD3DXEFFECT	effect;

	Material()
	{
		Diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
		Ambient = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
		Specular = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
		Emissive = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	void operator = (const D3DMATERIAL9& d3dmat)
	{
		Ambient = D3DXVECTOR4(max(d3dmat.Ambient.r, 0), max(d3dmat.Ambient.g, 0), max(d3dmat.Ambient.b, 0), max(d3dmat.Ambient.a, 0));
		Diffuse = D3DXVECTOR4(max(d3dmat.Diffuse.r, 0), max(d3dmat.Diffuse.g, 0), max(d3dmat.Diffuse.b, 0), max(d3dmat.Diffuse.a, 0));
		Specular = D3DXVECTOR4(max(d3dmat.Specular.r, 0), max(d3dmat.Specular.g, 0), max(d3dmat.Specular.b, 0), max(d3dmat.Specular.a, 0));
		Emissive = D3DXVECTOR4(max(d3dmat.Emissive.r, 0), max(d3dmat.Emissive.g, 0), max(d3dmat.Emissive.b, 0), max(d3dmat.Emissive.a, 0));
		Power = d3dmat.Power;
	}
};