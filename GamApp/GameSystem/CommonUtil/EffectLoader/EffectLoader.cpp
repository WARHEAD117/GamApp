#include "EffectLoader.h"
#include "D3D9Device.h"

EffectLoader::EffectLoader()
{
	
}


EffectLoader::~EffectLoader()
{
	SafeRelease(mEffect);
}

void EffectLoader::LoadFxEffect(std::string fileName)
{
	HRESULT result = ::D3DXCreateEffectFromFile(RENDERDEVICE::Instance().g_pD3DDevice, fileName.c_str(), NULL, NULL, D3DXSHADER_DEBUG,
		NULL, &mEffect, NULL);

	if (result != S_OK)
	{
		MessageBox(GetForegroundWindow(), fileName.c_str(), "Shader", MB_OK);
	}
}

LPD3DXEFFECT EffectLoader::GetEffect()
{
	if (mEffect)
		return mEffect;
	else
		return RENDERDEVICE::Instance().defaultEffect;
}
