#include "GameTimer.h"

GameTimer::GameTimer()
{
	secondsPerCount=0.0f;
	deltaTime=-1.0;
	int_BaseTime=0;
	int_PausedTime=0;
	int_PrevTime=0;
	int_CurrTime=0;
	Stopped=false;

	//Convert counts to seconds
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	secondsPerCount=1.0/(double)countsPerSec;
}
