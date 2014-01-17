#include "EffectLoader.h"
#include "D3D9Device.h"

EffectLoader::EffectLoader()
{
	
}


EffectLoader::~EffectLoader()
{
	TRelease(mEffect);
}

void EffectLoader::LoadFxEffect(std::string fileName)
{
	if (E_FAIL == ::D3DXCreateEffectFromFile(RENDERDEVICE::Instance().g_pD3DDevice, fileName.c_str(), NULL, NULL, D3DXSHADER_DEBUG,
		NULL, &mEffect, NULL))
	{
		MessageBox(GetForegroundWindow(), "ShadowMap_Shader_Error!", "Shader", MB_OK);
		abort();
	}
}

LPD3DXEFFECT EffectLoader::GetEffect()
{
	return mEffect;
}
