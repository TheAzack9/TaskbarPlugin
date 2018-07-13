#include "TaskbarThread.h"
#include <Psapi.h>
#include <ios>
#include <iomanip>
#include <sstream>
#include "../Utility.h"
#include "../../API/RainmeterAPI.h"
#include <AppModel.h>
#include <winuser.h>
#include "TaskbarItem.h"
#include <Dwmapi.h>
#include <Propsys.h>
#include <Propkey.h>

std::wstring GetWindowClass(HWND hwnd)
{
	// TODO: Find real length
	std::wstring wndClass;
	wndClass.resize(50);
	GetClassName(hwnd, &wndClass[0], wndClass.size());

	return wndClass;
}

std::wstring GetWindowTitle(HWND hwnd)
{
	DWORD_PTR length = 0;
	if (SendMessageTimeout(hwnd, WM_GETTEXTLENGTH, NULL, NULL, SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG, 200, &length) == 0) return std::wstring();
	if (length == 0) return std::wstring();

	wchar_t* buffer = new wchar_t[length + 1];

	if(SendMessageTimeout(hwnd, WM_GETTEXT, length+1, (LPARAM)buffer, SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG, 200, nullptr) == 0)
	{
		delete[] buffer;
		return std::wstring();
	}
	std::wstring title(buffer);
	delete[] buffer;
	return title;
	/*

	int length = GetWindowTextLengthW(hwnd);
	if (length == 0) return std::wstring();

	std::wstring title;
	title.resize(length + 1);
	GetWindowTextW(hwnd, &title[0], title.size());

	return title;*/
}

LONG GetProcessId(HWND hwnd)
{
	LONG processId;
	GetWindowThreadProcessId(hwnd, (LPDWORD)&processId);
	return processId;
}

HANDLE GetProcessHandle(LONG processId)
{
	return OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
}

std::wstring GetProcessName(HANDLE processHandle)
{
	std::wstring processName;
	processName.resize(MAX_PATH);
	GetModuleBaseName(processHandle, 0, &processName[0], processName.size());
	return processName;
}

std::wstring GetProcessPath(HANDLE processHandle)
{
	std::wstring filePath;
	filePath.resize(MAX_PATH);
	GetModuleFileNameEx(processHandle, 0, &filePath[0], filePath.size());
	return filePath;
}

std::wstring GetExeFileDescription(const std::wstring& path)
{
	std::wstring exeDescription;
	DWORD aVersionInfoSize;
	DWORD dwHandle;
	aVersionInfoSize = GetFileVersionInfoSize(path.c_str(), &dwHandle);

	if (aVersionInfoSize != 0)
	{
		LPBYTE res = new BYTE[aVersionInfoSize];
		if (GetFileVersionInfo(path.c_str(), dwHandle, aVersionInfoSize, (void**)res))
		{
			DWORD* buffer;
			UINT length;
			DWORD   m_dwLangCharset;
			if (VerQueryValue((void**)res, L"\\VarFileInfo\\Translation", (void**)&buffer, &length))
			{
				m_dwLangCharset = MAKELONG(HIWORD(buffer[0]), LOWORD(buffer[0]));
				DWORD* result;
				std::wstringstream stream;
				stream << L"\\StringFileInfo\\" << std::hex << std::setfill(L'0') << std::setw(8) << m_dwLangCharset << L"\\FileDescription";
				if (VerQueryValue((void**)res, stream.str().c_str(), (void**)&result, &length))
				{
					LPCWSTR resu = (LPCWSTR)result;
					//exeDescription.resize(length);
					exeDescription = resu;
				}
			}
		}

		delete[] res;
	}

	return exeDescription;
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
}

BOOL CALLBACK EnumChildren(HWND hwnd, LPARAM param)
{
	std::wstring wndClass = GetWindowClass(hwnd);

	if (Utility::IsEqual(wndClass.c_str(), L"Windows.UI.Core.CoreWindow", false))
	{
		bool* isValid = (bool*)param;
		(*isValid) = true;
	}
	return true;
}

auto shell = GetShellWindow();

bool IsValidWindow(HWND hwnd)
{
	if (hwnd == shell) return false;

	LONG styles = GetWindowLong(hwnd, GWL_EXSTYLE);
	if (styles & WS_EX_TOOLWINDOW)
	{
		return false;
	}

	HWND root = GetAncestor(hwnd, GA_ROOTOWNER);
	// This fixes issue where either manage or about in rainmeter is visible, but not both... it seems that some extra hwnds are visible
	// if (GetLastPopupHwnd(root) != hwnd) return false; 
	if (!IsWindowVisible(hwnd)) return false;


	int cloaked = 0;
	DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, static_cast<LPVOID>(&cloaked), sizeof cloaked);
	if (cloaked != 0) return false;

	return true;
}

BOOL CALLBACK EnumWindows(HWND hwnd, LPARAM lparam)
{
	if (!IsValidWindow(hwnd)) return true;

	

	/*
	* Best way i found to distinguish apps from exe's is that the uwp add is only valid if it is an ApplicationFrameWindow with a child window that is Windows.UI.Core.CoreWindow
	* It seems that when windows sends the app to the background that it detaches the Windows.UI.Core.CoreWindow and lets it stay as a separate top level window.
	* Not sure why they would do it this way, but it is how it is
	* NOT: Seems that when an application is minimized that it also is detached for some stupid fucking reason...
	* By removing the code beneath will there sometimes be that uwp applications will show even though they are not opened and have even never been active since boot
	* This is just some stupid problem with the ApplicationFrameWindow...
	*
	* The current solution is:
	* if CoreWindow glued to ApplicationFrameWindow, show in taskbar
	* if ApplicationFrameWindow is minimized, show in taskbar
	* else hide from taskbar
	*
	* This is because it seems that app windows are always displaying, but somehow not rendering when you start the computer, but when activated they start rendering.
	*/

	LONG styles = GetWindowLong(hwnd, GWL_STYLE);


	/*if (!Utility::IsEqual(windowClass.c_str(), L"ApplicationFrameWindow", false) || styles & WS_MINIMIZE)
	{
		// TODO: DO THIS SHIT TREVOR, NOW >:D
		// auto& vd = VirtualDesktopMeasure::virtualDesktop;
		// UINT id;
		// HRESULT hr = vd.getWindowDesktopId(hwnd, &id);
		// if(SUCCEEDED(hr) && id == vd.getCurrentDesktop())
		{
			bool isValidWindow = false;
			EnumChildWindows(hwnd, EnumChildren, (LPARAM)&isValidWindow);
			if (!isValidWindow) return true;
		}
	}*/

	std::vector<HWND>& handles = *(std::vector<HWND>*)lparam;
	handles.push_back(hwnd);
	return true;
}

void TaskbarThread::Update(TaskbarThread* taskbar, bool runOnce)
{
	if(!runOnce)
	taskbar->m_IsRunning = true;
	while (true)
	{
		std::map<HWND, TaskbarItem>* programMap;

		{
			std::lock_guard<std::mutex> guard(taskbar->m_Mutex);

			programMap = &taskbar->m_ProgramMap;
		} // destroy lock

		std::vector<HWND> handles;
		handles.reserve(programMap->size());
		shell = GetShellWindow();
		EnumWindows(EnumWindows, (LPARAM)&handles);
		for (auto it = handles.begin(); it != handles.end(); ++it)
		{
			auto hwnd = *it;
			std::wstring windowTitle = GetWindowTitle(hwnd);
			bool isVisible = IsWindowVisible(hwnd);
			if (programMap->find(hwnd) == programMap->end())
			{

				std::wstring windowClass = GetWindowClass(hwnd);
				auto processId = GetProcessId(hwnd);
				HANDLE processHandle = GetProcessHandle(processId);
				HANDLE testHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);

				std::wstring processPath;
				std::wstring processName;
				std::wstring exeDescription;

				std::wstring uwpLocation;
				bool isUwp = false;

				IPropertyStore *pps;
				HRESULT hr = SHGetPropertyStoreForWindow(hwnd, IID_PPV_ARGS(&pps));
				if(SUCCEEDED(hr))
				{
					PROPVARIANT var;
					
					hr = pps->GetValue(PKEY_AppUserModel_ID, &var);
					pps->Release();
				}

				if (!Utility::IsEqual(windowClass.c_str(), L"ApplicationFrameWindow", false))
				{
					processPath = GetProcessPath(processHandle);
					processName = GetProcessName(processHandle);
					exeDescription = GetExeFileDescription(processPath);
					if (Utility::IsEqual(exeDescription.c_str(), L"", false) || Utility::IsEqual(exeDescription.c_str(), L"COM Surrogate", false))
					{
						auto it = processName.find(L'.');
						if (it == std::wstring::npos)
						{
							exeDescription = processName;
						}
						else
						{
							exeDescription = processName.substr(0, it);
						}

						if (Utility::IsEqual(exeDescription.c_str(), L"DllHost", false))
						{
							exeDescription = L"Properties";
						}
					}

					std::wstring test;
					test.resize(200);
					UINT32 l =0;
					LONG result = GetApplicationUserModelId(testHandle, &l, NULL);

					if(result != ERROR_INSUFFICIENT_BUFFER)
					{
						if(result == APPMODEL_ERROR_NO_APPLICATION)
						{
							int i = 123;
						}
						else
						{
							int i = 123;

						}
					}

				}
				else // Is UWP app
				{
					isUwp = true;
					HWND child = FindWindowEx(hwnd, NULL, L"Windows.UI.Core.CoreWindow", NULL);
					if (child)
					{
						std::wstring windowTitle = GetWindowTitle(child);
						auto cprocId = GetProcessId(child);
						HANDLE cprocHandle = GetProcessHandle(cprocId);
						uwpLocation = GetProcessPath(cprocHandle);
						UINT32 length = 0;
						LONG rc = GetPackageFamilyName(cprocHandle, &length, nullptr);
						if (rc != ERROR_INSUFFICIENT_BUFFER)
						{
							if (rc == APPMODEL_ERROR_NO_PACKAGE)
							{
								RmLog(LOG_DEBUG, L"Could not fetch package name, invalid package");
								RmLog(LOG_DEBUG, std::to_wstring(rc).c_str());
								RmLog(LOG_DEBUG, std::to_wstring(length).c_str());
								RmLog(LOG_DEBUG, windowTitle.c_str());
							}
							else
							{

								RmLog(LOG_DEBUG, L"Could not fetch package name, unknown");
								RmLog(LOG_DEBUG, std::to_wstring(rc).c_str());
								RmLog(LOG_DEBUG, std::to_wstring(length).c_str());
								RmLog(LOG_DEBUG, windowTitle.c_str());
							}
						}

						std::wstring name;
						name.resize(length);
						rc = GetPackageFamilyName(cprocHandle, &length, &name[0]);
						if (rc == ERROR_SUCCESS)
						{
							processName = name;

							std::wstringstream stream;
							stream << L"Shell:AppsFolder\\" << processName.c_str() << L"!App";
							processPath = stream.str();
						}
					}
				}

				CloseHandle(processHandle);

				if (Utility::IsEqual(processName.c_str(), L"", false)) processName = windowTitle;
				if (Utility::IsEqual(exeDescription.c_str(), L"", false)) exeDescription = windowTitle;

				TaskbarItem item;
				item.m_ProcessName = processName;
				item.m_WindowTitle = windowTitle;
				item.m_WindowClass = windowClass;
				item.m_ProgramPath = processPath;
				item.m_ProgramDescription = exeDescription;
				item.m_Hwnd = hwnd;
				item.m_VirtualDesktopId = 0;
				item.m_IsPinned = true; // TODO: Probably has to be set elsewhere or with some pinned table...
				item.m_ProcessId = processId;
				item.m_IsUWP = isUwp;
				item.m_RealPath = uwpLocation;
				item.m_IsVisible = isVisible;

				programMap->insert({ hwnd, item });
			}
			else
			{
				auto& item = programMap->at(hwnd);
				item.m_WindowTitle = windowTitle;
				item.m_IsVisible = isVisible;
			}
		}

		for (auto it2 = programMap->begin(); it2 != programMap->end(); )
		{
			bool found = false;
			for (auto it = handles.begin(); it != handles.end(); ++it)
			{
				if (*it == it2->first)
				{
					found = true;
				}
			}

			if (!found)
			{
				it2 = programMap->erase(it2);
			}
			else
			{
				++it2;
			}
		}

		std::vector<TaskbarItem> programs;

		for (auto it = programMap->begin(); it != programMap->end(); ++it)
		{
			programs.push_back(it->second);
		}

		{
			std::lock_guard<std::mutex> guard(taskbar->m_Mutex);
			taskbar->m_Programs = programs;
			taskbar->m_HasChanged = true;
		} // destroy lock

		if (runOnce)
		{
			return;
		}
		{
			std::unique_lock<std::mutex> guard(taskbar->m_Mutex);
			if (taskbar->m_StopThread.wait_for(guard, std::chrono::milliseconds(100), [&]() {return taskbar->m_Interrupted == true; }))
			{
				taskbar->m_IsRunning = false;
				guard.unlock();
				taskbar->m_ThreadStopped.notify_all();
				return;
			}
		}
	}
}

TaskbarThread::TaskbarThread() : m_Mutex()
{
	Update(this, true);
	m_Thread = std::thread(Update, this, false);
	m_Thread.detach();
}


TaskbarThread::~TaskbarThread()
{
	m_Interrupted = true;
	m_StopThread.notify_all();
	{
		std::unique_lock<std::mutex> guard(m_Mutex);
		m_ThreadStopped.wait(guard, [&]() {return m_IsRunning == false; });

	} // destroy lock

}

std::vector<TaskbarItem>& TaskbarThread::GetPrograms(bool grouped)
{
	if(m_HasChanged)
	{
		{
			std::lock_guard<std::mutex> guard(m_Mutex);
			m_ProgramsCache = m_Programs; // I hope the guard is destroyed after the copy, not sure tho TODO: Check this
			m_HasChanged = false;
		} // force guard release

		std::map<std::wstring, TaskbarItem> group;

		for(auto& item : m_ProgramsCache)
		{
			item.m_GroupCount = 1;
			if(group.find(item.m_ProgramPath) == group.end())
			{
				group[item.m_ProgramPath] = item;
			}
			else
			{
				group[item.m_ProgramPath].m_GroupCount++;
			}
			group[item.m_ProgramPath].m_Items.push_back(item);
		}

		m_ProgramsGroupedCache.clear();
		for (auto& it : group) m_ProgramsGroupedCache.push_back(it.second);
	}
	return grouped ? m_ProgramsGroupedCache : m_ProgramsCache;
}

