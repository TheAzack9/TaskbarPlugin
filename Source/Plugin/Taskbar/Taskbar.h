#pragma once
#include <vector>
#include <unordered_map>
#include <Windows.h>

class Taskbar;

struct TaskbarItem
{
	TaskbarItem(std::wstring processName, std::wstring windowTitle, std::wstring windowClass, std::wstring programPath, std::wstring programDescription, HWND hwnd, int virtualDesktopId, bool isPinned, LONG processId) :
		m_ProcessName(processName),
		m_WindowTitle(windowTitle),
		m_WindowClass(windowClass),
		m_ProgramPath(programPath),
		m_ProgramDescription(programDescription),
		m_Hwnd(hwnd),
		m_VirtualDesktopId(virtualDesktopId),
		m_IsPinned(isPinned),
		m_ProcessId(processId)
	{
		
	}

	std::wstring m_ProcessName;
	std::wstring m_WindowTitle;
	std::wstring m_WindowClass;
	std::wstring m_ProgramPath;
	std::wstring m_ProgramDescription;
	HWND m_Hwnd;
	int m_VirtualDesktopId;
	bool m_IsPinned;
	LONG m_ProcessId;

	const std::wstring& GetIconPath()
	{
		return m_IconPath;
	}

private:
	std::wstring m_IconPath;
};

class TaskbarWrapper
{
public:
	TaskbarWrapper();
	~TaskbarWrapper();
	Taskbar& GetTaskbar();
	const std::vector<TaskbarItem>& GetTaskbarItems();

private:
	Taskbar* m_Taskbar;
};

class Taskbar
{
public:
	// Non-copyable
	Taskbar(Taskbar const&) = delete;
	void operator=(Taskbar const&) = delete;

	void Update();
private:
	friend class TaskbarWrapper;

	Taskbar();

	static Taskbar& GetTaskbar();

	void AddReference();
	void RemoveReference();

	std::vector<HWND> m_Hwnds;
	std::vector<TaskbarItem> m_Programs;
	int m_References;
};

