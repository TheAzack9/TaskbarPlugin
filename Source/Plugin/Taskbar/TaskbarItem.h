#pragma once
#include <Windows.h>
#include <string>
#include "../Utility.h"

struct TaskbarItem
{
	std::wstring m_ProcessName = L"";
	std::wstring m_WindowTitle = L"";
	std::wstring m_WindowClass = L"";
	std::wstring m_ProgramPath = L"";
	std::wstring m_ProgramDescription = L"";
	std::wstring m_RealPath = L"";
	std::wstring m_IconPath = L"";
	HWND m_Hwnd = {0};
	LONG m_ProcessId = 0;
	int m_VirtualDesktopId = 0;
	bool m_IsUWP = false;
	bool m_IsPinned = false;
	bool m_IsVisible = false;

	std::wstring& GetIconPath()
	{
		//TODO: This is still recreated every time the vector is copied, try to optimize :)
		if(m_IconPath.empty())
		{
			m_IconPath = Utility::GetIconPath(m_ProgramDescription, m_Hwnd, m_IsUWP, IconSize::Normal);
		}
		return m_IconPath;
	}
};

