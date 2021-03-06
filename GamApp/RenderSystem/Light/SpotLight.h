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
	//获取用来计算相机到灯光体顶点的向量的矩阵
	virtual D3DXMATRIX GetToViewDirMatrix();

	virtual void SetUseShadow(bool useShadow);
};
