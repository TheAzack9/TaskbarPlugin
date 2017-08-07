#pragma once

#include <Windows.h>
#include <objbase.h>
#include <ObjectArray.h>
#include <list>
#include "../../API/RainmeterAPI.h"

//Virtual desktop code
const CLSID CLSID_ImmersiveShell = {
	0xC2F03A33, 0x21F5, 0x47FA, 0xB4, 0xBB, 0x15, 0x63, 0x62, 0xA2, 0xF2, 0x39 };

const CLSID CLSID_VirtualDesktopAPI_Unknown = {
	0xC5E0CDCA, 0x7B6E, 0x41B2, 0x9F, 0xC4, 0xD9, 0x39, 0x75, 0xCC, 0x46, 0x7B };

const IID IID_IVirtualDesktopManagerInternal = {
	0xEF9F1A6C, 0xD3CC, 0x4358, 0xB7, 0x12, 0xF8, 0x4B, 0x63, 0x5B, 0xEB, 0xE7 };

const CLSID CLSID_IVirtualNotificationService = {
	0xA501FDEC, 0x4A09, 0x464C, 0xAE, 0x4E, 0x1B, 0x9C, 0x21, 0xB8, 0x49, 0x18 };

struct IApplicationView : public IUnknown
{
public:

};

EXTERN_C const IID IID_IVirtualDesktop;

MIDL_INTERFACE("FF72FFDD-BE7E-43FC-9C03-AD81681E88E4")
IVirtualDesktop : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE IsViewVisible(
		IApplicationView *pView,
		int *pfVisible) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetID(
		GUID *pGuid) = 0;
};

enum AdjacentDesktop
{
	// Neighboring desktop on the left 
	LeftDirection = 3,
	// Neighboring desktop on the right 
	RightDirection = 4
};

// Virtual Table Manager

EXTERN_C const IID IID_IVirtualDesktopManagerInternal;

// 10130
//MIDL_INTERFACE("EF9F1A6C-D3CC-4358-B712-F84B635BEBE7")
// 10240
//MIDL_INTERFACE("AF8DA486-95BB-4460-B3B7-6E7A6B2962B5")
// 14393
MIDL_INTERFACE("f31574d6-b682-4cdc-bd56-1827860abec6")
IVirtualDesktopManagerInternal : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetCount(
		UINT *pCount) = 0;

	virtual HRESULT STDMETHODCALLTYPE MoveViewToDesktop(
		IApplicationView *pView,
		IVirtualDesktop * pDesktop) = 0;

	// 10240 
	virtual HRESULT STDMETHODCALLTYPE CanViewMoveDesktops(
		IApplicationView * pView,
		int  * pfCanViewMoveDesktops) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetCurrentDesktop(
		IVirtualDesktop** desktop) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetDesktops(
		IObjectArray **ppDesktops) = 0;

	// Obtain a neighboring desktop relative to the specified, taking into account the direction of the 
	virtual HRESULT STDMETHODCALLTYPE GetAdjacentDesktop(
		IVirtualDesktop * pDesktopReference,
		AdjacentDesktop uDirection,
		IVirtualDesktop ** ppAdjacentDesktop) = 0;

	virtual HRESULT STDMETHODCALLTYPE SwitchDesktop(
		IVirtualDesktop *pDesktop) = 0;

	virtual HRESULT STDMETHODCALLTYPE CreateDesktopW(
		IVirtualDesktop **ppNewDesktop) = 0;

	// pFallbackDesktop - the desktop to which the transition will be made after removing the specified 
	virtual HRESULT STDMETHODCALLTYPE RemoveDesktop(
		IVirtualDesktop * pRemove,
		IVirtualDesktop * pFallbackDesktop) = 0;

	// 10240
	virtual HRESULT STDMETHODCALLTYPE FindDesktop(
		GUID *desktopId,
		IVirtualDesktop ** ppDesktop) = 0;
};

EXTERN_C const IID IID_IVirtualDesktopManager;

MIDL_INTERFACE("a5cd92ff-29be-454c-8d04-d82879fb3f1b")
IVirtualDesktopManager : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE IsWindowOnCurrentVirtualDesktop(
		/* [in] */ __RPC__in HWND topLevelWindow,
		/* [out] */ __RPC__out BOOL *onCurrentDesktop) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetWindowDesktopId(
		/* [in] */ __RPC__in HWND topLevelWindow,
		/* [out] */ __RPC__out GUID *desktopId) = 0;

	virtual HRESULT STDMETHODCALLTYPE MoveWindowToDesktop(
		/* [in] */ __RPC__in HWND topLevelWindow,
		/* [in] */ __RPC__in REFGUID desktopId) = 0;
};

EXTERN_C const IID IID_IVirtualDesktopNotification;

MIDL_INTERFACE("C179334C-4295-40D3-BEA1-C654D965605A")
IVirtualDesktopNotification : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopCreated(
		IVirtualDesktop *pDesktop) = 0;

	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyBegin(
		IVirtualDesktop *pDesktopDestroyed,
		IVirtualDesktop * pDesktopFallback) = 0;

	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyFailed(
		IVirtualDesktop *pDesktopDestroyed,
		IVirtualDesktop * pDesktopFallback) = 0;

	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyed(
		IVirtualDesktop *pDesktopDestroyed,
		IVirtualDesktop * pDesktopFallback) = 0;

	virtual HRESULT STDMETHODCALLTYPE ViewVirtualDesktopChanged(
		IApplicationView *pView) = 0;

	virtual HRESULT STDMETHODCALLTYPE CurrentVirtualDesktopChanged(
		IVirtualDesktop *pDesktopOld,
		IVirtualDesktop * pDesktopNew) = 0;

};


class virtualDesktopNotification : public IVirtualDesktopNotification
{
private:
	ULONG _referenceCount;
public:
	//Inherited from IVirtualDesktopNotification, most of these are not implemented
	virtualDesktopNotification(IVirtualDesktop *currDesktop, GUID currDesktopID, UINT desktopCount)
	{
		this->currDesktop = currDesktop;
		this->currDesktopID = currDesktopID;
		this->desktopCount = desktopCount;
	}
#pragma region Methods replicating originals
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject) override
	{
		// Always set out parameter to NULL, validating it first.
		if (!ppvObject)
			return E_INVALIDARG;
		*ppvObject = NULL;

		if (riid == IID_IUnknown || riid == __uuidof(IVirtualDesktopNotification))
		{
			// Increment the reference count and return the pointer.
			*ppvObject = (LPVOID)this;
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef() override
	{
		return InterlockedIncrement(&_referenceCount);
	}

	virtual ULONG STDMETHODCALLTYPE Release() override
	{
		ULONG result = InterlockedDecrement(&_referenceCount);
		if (result == 0)
		{
			delete this;
		}
		return 0;
	}
#pragma endregion
#pragma region Unimplmented since unused
	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyBegin(IVirtualDesktop *pDesktopDestroyed, IVirtualDesktop * pDesktopFallback)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyFailed(IVirtualDesktop *pDesktopDestroyed, IVirtualDesktop * pDesktopFallback) 
	{
		return E_NOTIMPL;
	}
#pragma endregion
	
	//I may implement this once I figure out what it does
	virtual HRESULT STDMETHODCALLTYPE ViewVirtualDesktopChanged(IApplicationView * pView) override
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopCreated(IVirtualDesktop *desktopNew) override
	{
		++desktopCount;
		return S_OK;
	}
	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyed(IVirtualDesktop *desktopDestroyed, IVirtualDesktop *desktopFallback) override
	{
		--desktopCount;
		return S_OK;
	}
	virtual HRESULT STDMETHODCALLTYPE CurrentVirtualDesktopChanged(IVirtualDesktop *desktopOld, IVirtualDesktop *desktopCurr) override
	{
		currDesktop = desktopCurr;

		//Get GUID here because getting it in the conversion later it fails when > virtual desktop when rainmeter boots and then it is only done once per change
		currDesktop->GetID(&currDesktopID);

		return S_OK;
	}

	IVirtualDesktop* getCurrDesktop()
	{
		return currDesktop;
	}
	GUID getCurrDesktopID()
	{
		return currDesktopID;
	}
	UINT getDesktopCount()
	{
		return desktopCount;
	}
private:
	IVirtualDesktop *currDesktop;
	GUID currDesktopID = GUID();
	UINT desktopCount = 0;

};

EXTERN_C const IID IID_IVirtualDesktopNotificationService;

MIDL_INTERFACE("0CD45E71-D927-4F15-8B0A-8FEF525337BF")
IVirtualDesktopNotificationService : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE Register(
		IVirtualDesktopNotification *pNotification,
		DWORD *pdwCookie) = 0;

	virtual HRESULT STDMETHODCALLTYPE Unregister(
		DWORD dwCookie) = 0;
};

class VirtualDesktop
{
	friend class virtualDesktopNotification;
public:
	VirtualDesktop();
	~VirtualDesktop();

	HRESULT createDesktop();
	HRESULT destroyDesktop(IVirtualDesktop *desktop);
	HRESULT destroyDesktop(UINT desktopIndex);

	HRESULT switchToDesktop(IVirtualDesktop *desktop);
	HRESULT switchToDesktop(UINT desktopIndex);
	HRESULT switchToDesktop(AdjacentDesktop direction);

	UINT desktopToIndex(GUID desktopID);
	UINT desktopToIndex(IVirtualDesktop *desktop);
	IVirtualDesktop* indexToDesktop(int desktopIndex);

	UINT getCurrentDesktop();
	UINT getDesktopCount();

	HRESULT getWindowDesktopId(HWND topLevelWindow, UINT *desktopIndex);
	HRESULT getWindowDesktopId(HWND topLevelWindow, GUID *desktopID);

private:

	//Virtual Desktop managers and services
	IServiceProvider* serviceProvider;
	IVirtualDesktopManagerInternal* desktopManagerInternal;
	IVirtualDesktopManager* desktopManager;

	IVirtualDesktopNotificationService* desktopNotificationService;
	virtualDesktopNotification* desktopNotifications;
	DWORD notificationCookie;
};