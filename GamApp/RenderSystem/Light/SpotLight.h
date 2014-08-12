#pragma once
#include "Light/BaseLight.h"

class SpotLight : 
	public BaseLight
{
public:
	SpotLight();
	~SpotLight();

	//virtual void OnFrame();

	virtual void RenderLightVolume();
	virtual void BuildLightVolume();

	virtual void RebuildProjMatrix();
	virtual void RebuildViewMatrix();

	virtual D3DXMATRIX GetLightVolumeTransform();
	//��ȡ��������������ƹ��嶥��������ľ���
	virtual D3DXMATRIX GetToViewDirMatrix();

	virtual void SetUseShadow(bool useShadow);
};
