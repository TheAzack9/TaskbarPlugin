#include "VirtualDesktopMeasure.h"
#include "../../API/RainmeterAPI.h"
#include <exception>
#include <sstream>
#include <algorithm>
#include <cstdint>

int VirtualDesktopMeasure::measureCount = 0;
VirtualDesktop VirtualDesktopMeasure::virtualDesktop = VirtualDesktop();

void VirtualDesktopMeasure::Initialize(void * rm)
{
	if (measureCount == 0)
	{
		virtualDesktop = VirtualDesktop();
	}
	++measureCount;
}

void VirtualDesktopMeasure::Reload(void* rm, double* maxValue)
{
	std::wstring typeString = RmReadString(rm, L"Type", L"Current", false);
	std::transform(typeString.begin(), typeString.end(), typeString.begin(), ::tolower);
	
	if (typeString.compare(L"current") == 0)
	{
		measureType = measureTypes::current;
	}
	else if (typeString.compare(L"count") == 0)
	{
		measureType = measureTypes::count;
	}
}

double VirtualDesktopMeasure::Update()
{
	if (measureType == measureTypes::current)
	{
		return virtualDesktop.getCurrentDesktop();
	}
	else if (measureType == measureTypes::count)
	{
		return virtualDesktop.getDesktopCount();
	}

	return 0.0;
}

LPCWSTR VirtualDesktopMeasure::GetString()
{
	return NULL;
}

void VirtualDesktopMeasure::ExecuteBang(LPCWSTR args)
{
	//Full command with argument
	std::wstring cmd = args;
	std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

	//Command argument or nil
	std::wstring arg = cmd.find(L" ") != std::string::npos ? cmd.substr(cmd.find(L" ")) : L"";

	if (cmd.find(L"setdesktop") != std::string::npos)
	{
		try
		{
			int desktopIndex = std::stoi(arg);
			virtualDesktop.switchToDesktop(desktopIndex);
		}
		catch (const std::string e)
		{
			RmLog(LOG_ERROR, L"Unable to convert dekstop number to an integer");
		}
	}
	else if (cmd.find(L"nextdesktop") != std::string::npos)
	{
		virtualDesktop.switchToDesktop(AdjacentDesktop::RightDirection);
	}
	else if (cmd.find(L"previousdesktop") != std::string::npos)
	{
		virtualDesktop.switchToDesktop(AdjacentDesktop::LeftDirection);
	}
	else if (cmd.find(L"createdesktop") != std::string::npos)
	{
		virtualDesktop.createDesktop();
	}
	else if (cmd.find(L"destroydesktop") != std::string::npos)
	{
		try
		{
			int desktopIndex = std::stoi(arg);
			virtualDesktop.destroyDesktop(desktopIndex);
		}
		catch (const std::string e)
		{
			RmLog(LOG_ERROR, L"Unable to convert dekstop number to an integer");
		}
	}
}

void VirtualDesktopMeasure::Finalize()
{
	--measureCount;
	if (measureCount == 0)
	{
		virtualDesktop.~VirtualDesktop();
	}
}