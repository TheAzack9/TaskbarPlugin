#include "Taskbar.h"
#include <Windows.h>
#include <string>
#include <Psapi.h>
#include "../../API/RainmeterAPI.h"
#include <sstream>
#include <iomanip>

TaskbarWrapper::TaskbarWrapper()
{
	m_Taskbar = &Taskbar::GetTaskbar();
	m_Taskbar->AddReference();
}

TaskbarWrapper::~TaskbarWrapper()
{
	m_Taskbar->RemoveReference();
}

Taskbar& TaskbarWrapper::GetTaskbar()
{
	return *m_Taskbar;
}

const std::vector<TaskbarItem>& TaskbarWrapper::GetTaskbarItems()
{
	return m_Taskbar->m_Programs;
}

Taskbar& Taskbar::GetTaskbar()
{
	static Taskbar taskbar;
	return taskbar;
}

void Taskbar::AddReference()
{
	++m_References;
}

void Taskbar::RemoveReference()
{
	--m_References;
}

HWND GetLastPopupHwnd(HWND hwnd)
{
	int maxLevel = 50;
	HWND currentHwnd = hwnd;
	while (--maxLevel >= 0)
	{
		HWND last = GetLastActivePopup(currentHwnd);

		if (IsWindowVisible(last)) return last;

		if (last == currentHwnd) return (HWND)NULL;

		currentHwnd = last;
	}
	return (HWND)NULL;
};

BOOL CALLBACK EnumChildren(HWND hwnd, LPARAM param)
{
	// TODO: Find real length
	std::wstring wndClass;
	wndClass.resize(50);
	GetClassName(hwnd, &wndClass[0], wndClass.size());

	if (wcscmp(wndClass.c_str(), L"Windows.UI.Core.CoreWindow") == 0)
	{
		bool* isValid = (bool*)param;
		(*isValid) = true;
	}
	return true;
}

BOOL CALLBACK EnumWindows(HWND hwnd, LPARAM lparam)
{
	auto shell = GetShellWindow();
	if (hwnd == shell) return true;
	if (!IsWindowVisible(hwnd)) return true;

	HWND root = GetAncestor(hwnd, GA_ROOTOWNER);

	if (GetLastPopupHwnd(root) != hwnd) return true;

	int length = GetWindowTextLengthW(hwnd);
	if (length == 0) return true;

	std::wstring title;
	title.resize(length + 1);
	GetWindowTextW(hwnd, &title[0], title.size());

	if (title.size() == 0) return true;

	// TODO: Find real length
	std::wstring wndClass;
	wndClass.resize(50);
	GetClassName(hwnd, &wndClass[0], wndClass.size());

	if (wcscmp(wndClass.c_str(), L"Windows.UI.Core.CoreWindow") == 0)
	{
		return true;
	}

	/*
	* Best way i found to distinguish apps from exe's is that the uwp add is only valid if it is an ApplicationFrameWindow with a child window that is Windows.UI.Core.CoreWindow
	* It seems that when windows sends the app to the background that it detaches the Windows.UI.Core.CoreWindow and lets it stay as a separate top level window.
	* Not sure why they would do it this way, but it is how it is
	*/

	if (wcscmp(wndClass.c_str(), L"ApplicationFrameWindow") == 0)
	{
		bool isValidWindow = false;
		EnumChildWindows(hwnd, EnumChildren, (LPARAM)&isValidWindow);
		if (!isValidWindow) return true;
	}

	LONG styles = GetWindowLong(hwnd, GWL_EXSTYLE);
	if (styles & WS_EX_TOOLWINDOW)
	{
		 return true;
	}

	std::vector<HWND>* programs = (std::vector<HWND>*)lparam;

	programs->push_back(hwnd);

	return true;
};

void Taskbar::Update()
{
	m_Hwnds.clear();

	EnumWindows(EnumWindows, (LPARAM)&m_Hwnds);

	for(auto hwnd : m_Hwnds)
	{
		LONG processId;
		GetWindowThreadProcessId(hwnd, (LPDWORD)&processId);

		// MAX_PATH will suffice most of the time, i hope
		std::wstring filePath;
		filePath.resize(MAX_PATH);
		HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
		GetModuleFileNameEx(handle, 0, &filePath[0], filePath.size());
		/*std::wstring processName;
		processName.resize(MAX_PATH);
		QueryFullProcessImageName(handle, 0, &processName[0], &processName.size());
		*/
		CloseHandle(handle);

		DWORD aVersionInfoSize;
		DWORD dwHandle;
		aVersionInfoSize = GetFileVersionInfoSize(filePath.c_str(), &dwHandle);

		std::wstring exeDescription;

		if (aVersionInfoSize != 0)
		{
			LPBYTE res = new BYTE[aVersionInfoSize];
			if (GetFileVersionInfo(filePath.c_str(), dwHandle, aVersionInfoSize, (void**)res))
			{
				DWORD* buffer;
				UINT length;
				DWORD   m_dwLangCharset;
				if(VerQueryValue((void**)res, L"\\VarFileInfo\\Translation", (void**)&buffer, &length))
				{
					m_dwLangCharset = MAKELONG(HIWORD(buffer[0]), LOWORD(buffer[0]));
					DWORD* result;
					std::wstringstream stream;
					stream << L"\\StringFileInfo\\" << std::hex << std::setfill(L'0') << std::setw(8) << m_dwLangCharset << L"\\FileDescription";
					if (VerQueryValue((void**)res, stream.str().c_str(), (void**)&result, &length))
					{
						LPCWSTR resu = (LPCWSTR)result;
						exeDescription = resu;
					}
				}
			}

			delete[] res;
		}

		int length = GetWindowTextLengthW(hwnd);
		std::wstring title;
		title.resize(length + 1);
		GetWindowTextW(hwnd, &title[0], title.size());

		// TODO: Find real length
		std::wstring wndClass;
		wndClass.resize(50);
		GetClassName(hwnd, &wndClass[0], wndClass.size());

		m_Programs.emplace_back(L"", title, wndClass, filePath, exeDescription, hwnd, 0, true, processId );
	}
}

Taskbar::Taskbar()
{
}
