#pragma once
#include "CommonUtil/D3D9Header.h"
#include "RenderUtil/RenderUtil.h"

class RenderPipe
{
public:
	RenderPipe();
	~RenderPipe();

	void	ClearRenderUtil();
	void	PushRenderUtil(const RenderUtil& renderUtil);

	void	RenderAll();

private:
	std::vector<RenderUtil> mRenderUtilList;
};


typedef CSingleton<RenderPipe> RENDERPIPE;