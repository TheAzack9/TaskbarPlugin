#pragma once
#include <memory>
#include "TaskbarThread.h"

class TakbarInstance
{
	friend class Taskbar;

	void AddReference();
	void RemoveReference();

	std::unique_ptr<TaskbarThread> m_Thread = nullptr; // TODO: To constructor
	int m_References = 0;
};

// All values in here are thread safe
class Taskbar
{
public:
	Taskbar();
	~Taskbar();

	std::vector<TaskbarItem>* GetTaskbarItems();

	// Non-copyable
	Taskbar(Taskbar const&) = delete;
	void operator=(Taskbar const&) = delete;

private:
	static TakbarInstance& GetTaskbarInstance();
};

