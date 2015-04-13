#pragma once
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>

#include "Singleton.h"

template<class COM>
void TRelease(COM& t)
{
	if (t)
	{
		int count = t->Release();
		//t = NULL;
	}
}

template<class POINT>
void Delete(POINT t)
{
	if (t)
	{
		delete t;
		t = NULL;
	}
}

//安全释放内存
#define SafeDelete(p)		{if(p) { delete (p); (p) = NULL;}}
#define SafeDeleteArray(p)	{if(p) { delete[] (p); (p) = NULL;}}
#define SafeRelease(p)		{if(p) { (p)->Release();(p) = NULL;}}

#define RENDER_DEBUG