#pragma once 
#include "CommonUtil\GlobalHeader.h"
#include "CommonUtil\D3D9Header.h"

class EffectLoader
{
public:
	EffectLoader();
	~EffectLoader();

	LPD3DXEFFECT	GetEffect();

	LPD3DXEFFECT	GetDefaultEffect();
	void			LoadFxEffect(std::string fileName);

private:
	LPD3DXEFFECT	mEffect;
};

