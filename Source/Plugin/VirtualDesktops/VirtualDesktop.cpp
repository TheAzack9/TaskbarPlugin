#include "VirtualDesktop.h"
#include "../../API/RainmeterAPI.h"
#include <exception>
#include <sstream>
#include <algorithm>
#include <cstdint>

void VirtualDesktop::Reload(void* rm, double* maxValue)
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
double VirtualDesktop::Update()
{
	try
	{
		if (desktopManagerInternal != nullptr)
		{
			if (measureType == measureTypes::current)
			{
				//@TODO Replace polling this with listening and using internal objects
				IVirtualDesktop *currDesktop;
				desktopManagerInternal->GetCurrentDesktop(&currDesktop);
				GUID currGUID = GUID();
				currDesktop->GetID(&currGUID);

				return desktopToIndex(currGUID);
			}
			else if (measureType == measureTypes::count)
			{
				UINT desktopCount = 1;
				if (FAILED(desktopManagerInternal->GetCount(&desktopCount)))
				{
					RmLog(LOG_ERROR, L"Unable to get desktop count");
				}

				return desktopCount;
			}

		}
	}
	catch (std::exception& e)
	{
		std::wstringstream error;
		error << e.what();

		RmLog(LOG_ERROR, error.str().c_str());
	}

	return 0.0;
}
LPCWSTR VirtualDesktop::GetString()
{
	return NULL;
}
void VirtualDesktop::ExecuteBang(LPCWSTR args)
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
			switchToDesktop(desktopIndex);
		}
		catch (const std::string e)
		{
			RmLog(LOG_ERROR, L"Unable to convert dekstop number to an integer");
		}
	}
	else if (cmd.find(L"nextdesktop") != std::string::npos)
	{
		switchToDesktop(AdjacentDesktop::RightDirection);
	}
	else if (cmd.find(L"previousdesktop") != std::string::npos)
	{
		switchToDesktop(AdjacentDesktop::LeftDirection);
	}
	else if (cmd.find(L"createdesktop") != std::string::npos)
	{
		createDesktop();
	}
	else if (cmd.find(L"destroydesktop") != std::string::npos)
	{
		try
		{
			int desktopIndex = std::stoi(arg);
			destroyDesktop(desktopIndex);
		}
		catch (const std::string e)
		{
			RmLog(LOG_ERROR, L"Unable to convert dekstop number to an integer");
		}
	}
}
void VirtualDesktop::Finalize()
{
	if (desktopManagerInternal != nullptr)
	{
		//Release access to desktop manager
		desktopManagerInternal->Release();
		desktopManagerInternal = nullptr;
	}
	if (desktopManager != nullptr)
	{
		//Release access to desktop manager
		desktopManager->Release();
		desktopManager = nullptr;
	}
}



#pragma region Desktop type conversions
UINT VirtualDesktop::desktopToIndex(GUID desktopID)
{
	IObjectArray *desktops = nullptr;
	HRESULT hr = desktopManagerInternal->GetDesktops(&desktops);

	int i = 0;
	IVirtualDesktop *desktop = nullptr;
	desktops->GetAt(i, IID_PPV_ARGS(&desktop));

	//Since desktops have no implicit order loop through IObjectArray until I find a match
	while (desktop != nullptr)
	{
		GUID currID = GUID();
		desktop->GetID(&currID);

		if (currID == desktopID)
		{
			return i;
		}

		//Increment
		i++;
		desktops->GetAt(i, IID_PPV_ARGS(&desktop));
	}

	//Not found, just return default index
	RmLog(LOG_ERROR, L"Unable to find index of specified desktop");
	return 0;
}
IVirtualDesktop * VirtualDesktop::indexToDesktop(int desktopIndex)
{
	IObjectArray *desktops = nullptr;
	desktopManagerInternal->GetDesktops(&desktops);

	IVirtualDesktop *desktop = nullptr;
	desktops->GetAt(desktopIndex, IID_PPV_ARGS(&desktop));

	//Note this can be null in index was outside desktops
	return desktop;
}
#pragma endregion

#pragma region Window desktop getters
HRESULT VirtualDesktop::getWindowDesktopId(HWND topLevelWindow, UINT *desktopIndex)
{
	GUID desktopID = GUID();
	HRESULT hr = desktopManager->GetWindowDesktopId(topLevelWindow, &desktopID);

	desktopIndex = new UINT(desktopToIndex(desktopID));

	return hr;
}

//Just replicating for ease of use
HRESULT VirtualDesktop::getWindowDesktopId(HWND topLevelWindow, GUID *desktopID)
{
	return desktopManager->GetWindowDesktopId(topLevelWindow, desktopID);
}
#pragma endregion

#pragma region Virtual desktop switching
//Switch to specific desktop
HRESULT VirtualDesktop::switchToDesktop(IVirtualDesktop *desktop)
{
	if (desktop != nullptr)
	{
		return desktopManagerInternal->SwitchDesktop(desktop);
	}
	//@TODO Silent fail or not on switching to bad desktop
}
//Switch to specific desktop by index
HRESULT VirtualDesktop::switchToDesktop(UINT desktopIndex)
{
	IVirtualDesktop *desktop = indexToDesktop(desktopIndex);
	return switchToDesktop(desktop);
}
//Switch to desktop in certain direction
HRESULT VirtualDesktop::switchToDesktop(AdjacentDesktop direction)
{
	//Desktop to move to if current desktop is removed
	IVirtualDesktop *currDesktop = nullptr;
	HRESULT hr = desktopManagerInternal->GetCurrentDesktop(&currDesktop);

	if (SUCCEEDED(hr))
	{
		IVirtualDesktop *desktop;
		hr = desktopManagerInternal->GetAdjacentDesktop(currDesktop, direction, &desktop);

		if (SUCCEEDED(hr))
		{
			return desktopManagerInternal->SwitchDesktop(desktop);
		}
		//@TODO Silent fail or not on switching to bad desktop
	}

	return hr;
}
#pragma endregion

#pragma region Virtual desktop creation and destruction
//Instance new desktop
HRESULT VirtualDesktop::createDesktop()
{
	IVirtualDesktop *newDesktop = nullptr;
	return desktopManagerInternal->CreateDesktopW(&newDesktop);
}

//Destroy desktop
HRESULT VirtualDesktop::destroyDesktop(UINT desktopIndex)
{
	IVirtualDesktop *desktop = indexToDesktop(desktopIndex);
	return destroyDesktop(desktop);
}

//Destroy desktop
HRESULT VirtualDesktop::destroyDesktop(IVirtualDesktop *desktop)
{
	//Desktop to move to if current desktop is removed
	IVirtualDesktop *fallbackDesktop;
	HRESULT hr = desktopManagerInternal->GetAdjacentDesktop(desktop, AdjacentDesktop::LeftDirection, &fallbackDesktop);

	//Get desktop to the right if no desktop was found to left
	if (FAILED(hr))
	{
		hr = desktopManagerInternal->GetAdjacentDesktop(desktop, AdjacentDesktop::RightDirection, &fallbackDesktop);
	}

	if (SUCCEEDED(hr))
	{
		return desktopManagerInternal->RemoveDesktop(desktop, fallbackDesktop);
	}
	//If no match found 
	return hr;
}
#pragma endregion