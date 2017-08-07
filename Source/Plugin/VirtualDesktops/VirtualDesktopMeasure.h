#pragma once

#include <Windows.h>
#include "../../API/RainmeterAPI.h"
#include "VirtualDesktop.h"

class VirtualDesktopMeasure
{
public:
	void Initialize(void* rm);
	void Reload(void* rm, double* maxValue);
	double Update();
	LPCWSTR GetString();
	void ExecuteBang(LPCWSTR args);
	void Finalize();

	static VirtualDesktop virtualDesktop;
private:
	//Possible measure types
	enum measureTypes
	{
		count = 0,
		current = 1
	};

	static int measureCount;
	measureTypes measureType = measureTypes::current;
};

