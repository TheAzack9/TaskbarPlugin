#pragma once
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <map>
#include <vector>
#include <Windows.h>
#include "TaskbarItem.h"
#include <atomic>

class TaskbarThread
{
public:
	TaskbarThread();
	~TaskbarThread();

	std::vector<TaskbarItem>& GetPrograms();

	static void Update(TaskbarThread* taskbar, bool runOnce = false);
private:
	std::thread m_Thread;
	std::mutex m_Mutex;
	std::map<HWND, TaskbarItem> m_ProgramMap;
	std::vector<TaskbarItem> m_Programs;
	std::vector<TaskbarItem> m_ProgramsCache;

	std::atomic<bool> m_Interrupted = false;
	std::atomic<bool> m_IsRunning = false;
	std::atomic<bool> m_HasChanged = true;

	std::condition_variable m_StopThread;
	std::condition_variable m_ThreadStopped;
};

