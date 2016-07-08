#pragma once
#include "CommonUtil\\GlobalHeader.h"
 
class GlobalTimer
{
public:
	GlobalTimer();
	~GlobalTimer();

private:
	LARGE_INTEGER mFrameStartTime;
	LARGE_INTEGER mFrameEndTime;
	double mLastFrameDurTime;

	LARGE_INTEGER mGameStartTime;

protected:
	LARGE_INTEGER mPerformanceFrequency;
	LARGE_INTEGER mFrequencyStart;
	LARGE_INTEGER mFrequencyEnd;
public:
	//All unit is second(s)
	void StartGameTimer();
	void ResetGameTimer();
	double GetCurGameTime();
	void StartFrame();
	void EndFrame();
	double GetFrameTime();


};

typedef CSingleton<GlobalTimer> GLOBALTIMER;