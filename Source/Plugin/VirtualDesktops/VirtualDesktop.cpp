#include "VirtualDesktop.h"
#include "../../API/RainmeterAPI.h"
#include <exception>
#include <sstream>
#include <algorithm>
#include <cstdint>

VirtualDesktop::VirtualDesktop()
{
	//Setup desktop manager
	HRESULT hr = ::CoCreateInstance(CLSID_ImmersiveShell, NULL, CLSCTX_LOCAL_SERVER, __uuidof(IServiceProvider), (PVOID*)&serviceProvider);
	if (SUCCEEDED(hr))
	{
		//Init internal virtual desktop API
		hr = serviceProvider->QueryService(CLSID_VirtualDesktopAPI_Unknown, &desktopManagerInternal);
		if (FAILED(hr))
		{
			RmLog(LOG_ERROR, L"Unable to get an instance of the internal desktop manager, the internal API must have changed");
		}

		//Init official virtual desktop API
		hr = serviceProvider->QueryService(__uuidof(IVirtualDesktopManager), &desktopManager);
		if (FAILED(hr))
		{
			RmLog(LOG_ERROR, L"Unable to get an instance of the desktop manager, the API must have changed");
		}

		//Init service for virtual dekstop notifying
		hr = serviceProvider->QueryService(CLSID_IVirtualNotificationService, &desktopNotificationService);
		if (FAILED(hr))
		{
			RmLog(LOG_ERROR, L"Unable to get an instance of the desktop manager notifier service, the API must have changed");
		}
		else
		{
			//Initialize internal variables 
			IVirtualDesktop *currDesktop;
			desktopManagerInternal->GetCurrentDesktop(&currDesktop);

			GUID currDesktopID = GUID();
			currDesktop->GetID(&currDesktopID);

			UINT desktopCount;
			desktopManagerInternal->GetCount(&desktopCount);

			desktopNotifications = new virtualDesktopNotification(currDesktop, currDesktopID, desktopCount);

			//Register for notifications
			hr = desktopNotificationService->Register(desktopNotifications, &notificationCookie);
			if (FAILED(hr))
			{
				RmLog(LOG_ERROR, L"Unable to register for virtual desktop changes");
			}
		}
	}
	else
	{
		RmLog(LOG_ERROR, L"Unable to get an instance of the desktop service, the API must have changed");
	}
}

VirtualDesktop::~VirtualDesktop()
{
	if (desktopManagerInternal)
	{
		//Release access to desktop manager
		desktopManagerInternal->Release();
		desktopManagerInternal = nullptr;
	}
	if (desktopManager)
	{
		//Release access to desktop manager
		desktopManager->Release();
		desktopManager = nullptr;
	}
	if (desktopNotificationService)
	{
		//Release access to desktop manager
		desktopNotificationService->Unregister(notificationCookie);
		desktopNotificationService->Release();
		desktopNotificationService = nullptr;
	}
	if (desktopNotifications)
	{
		//Release access to desktop manager
		//desktopNotifications->Release();
		desktopNotifications = nullptr;
	}
}

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
HRESULT VirtualDesktop::switchToDesktopAnim(AdjacentDesktop direction)
{
	//Key controls are:
	//Left ctrl VK_LCONTROL 0xA2
	//Left windows key VK_LWIN 0x5B
	//Left arrow VK_LEFT 0x25
	//Right arrow VK_RIGHT 0x27

	const int inputCount = 3;
	INPUT input[inputCount];
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = 0xA2;
	input[0].ki.wScan = 0;
	input[0].ki.dwFlags = 0;
	input[0].ki.time = 0;

	input[1].type = INPUT_KEYBOARD;
	input[1].ki.wVk = 0x5B;
	input[1].ki.wScan = 0;
	input[1].ki.dwFlags = 0;
	input[1].ki.time = 0;

	if (direction == AdjacentDesktop::LeftDirection)
	{

		input[2].type = INPUT_KEYBOARD;
		input[2].ki.wVk = 0x25;
		input[2].ki.wScan = 0;
		input[2].ki.dwFlags = 0;
		input[2].ki.time = 0;
	}
	else
	{

		input[2].type = INPUT_KEYBOARD;
		input[2].ki.wVk = 0x27;
		input[2].ki.wScan = 0;
		input[2].ki.dwFlags = 0;
		input[2].ki.time = 0;

	}
	
	UINT keysSent = SendInput(inputCount, input, sizeof(INPUT));

	if (keysSent > 0)
	{
		input[0].ki.dwFlags = KEYEVENTF_KEYUP;
		input[1].ki.dwFlags = KEYEVENTF_KEYUP;
		input[2].ki.dwFlags = KEYEVENTF_KEYUP;

		keysSent = SendInput(inputCount, input, sizeof(INPUT));
		if (keysSent > 0)
		{
			return S_OK;
		}
		else
		{
			return GetLastError();
		}
	}
	else
	{
		return GetLastError();
	}
}
#pragma endregion

#pragma region Desktop type conversions
UINT VirtualDesktop::desktopToIndex(IVirtualDesktop *desktop)
{
	GUID desktopID = GUID();
	HRESULT hr = desktop->GetID(&desktopID);

	return desktopToIndex(desktopID);
}
UINT VirtualDesktop::desktopToIndex(GUID desktopID)
{
	IObjectArray *desktops = nullptr;
	HRESULT hr = desktopManagerInternal->GetDesktops(&desktops);

	UINT count;
	desktops->GetCount(&count);
	int i = 0;
	IVirtualDesktop *currDesktop = nullptr;
	desktops->GetAt(i, IID_PPV_ARGS(&currDesktop));

	//Since desktops have no implicit order loop through IObjectArray until I find a match
	while (currDesktop != nullptr)
	{
		GUID currID = GUID();
		currDesktop->GetID(&currID);

		if (currID == desktopID)
		{
			return i;
		}

		//Increment
		i++;
		desktops->GetAt(i, IID_PPV_ARGS(&currDesktop));
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

#pragma region Functions for rainmeter measure types
UINT VirtualDesktop::getCurrentDesktop()
{
	if (desktopNotifications)
	{
		//I would love to make this no require conversion on every call and instead only on desktop change but that will require another refactor
		return desktopToIndex(desktopNotifications->getCurrDesktopID());
	}
	return 0;
}

UINT VirtualDesktop::getDesktopCount()
{
	if (desktopNotifications)
	{
		return desktopNotifications->getDesktopCount();
	}
	return 1;
}
#pragma endregion