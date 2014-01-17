#pragma once
#include "CommonUtil/D3D9Header.h"
#include "RenderUtil/RenderUtil.h"

class RenderPipe
{
public:
	RenderPipe();
	~RenderPipe();

	void	ClearRenderUtil();
	void	PushRenderUtil(RenderUtil* const renderUtil);

	void	RenderAll();
	void	UpdateRenderState();

private:
	std::vector<RenderUtil*> mRenderUtilList;
};


typedef CSingleton<RenderPipe> RENDERPIPE;