#include "GlobalTimer.h"


GlobalTimer::GlobalTimer()
{
	mLastFrameDurTime = 0;
	QueryPerformanceFrequency(&mPerformanceFrequency);
}


GlobalTimer::~GlobalTimer()
{
}

void GlobalTimer::StartGameTimer()
{
	QueryPerformanceCounter(&mGameStartTime);
}

void GlobalTimer::ResetGameTimer()
{
	QueryPerformanceCounter(&mGameStartTime);
}

double GlobalTimer::GetCurGameTime()
{
	LARGE_INTEGER curTime;
	QueryPerformanceCounter(&curTime);

	return (double)((curTime.QuadPart - mGameStartTime.QuadPart) * 1000 / mPerformanceFrequency.QuadPart);
}

void GlobalTimer::StartFrame()
{
	QueryPerformanceCounter(&mFrameStartTime);
}

void GlobalTimer::EndFrame()
{
	QueryPerformanceCounter(&mFrameEndTime);
	mLastFrameDurTime = (double)((mFrameEndTime.QuadPart - mGameStartTime.QuadPart) * 1000 / mPerformanceFrequency.QuadPart);
}

double GlobalTimer::GetFrameTime()
{
	return mLastFrameDurTime;
}