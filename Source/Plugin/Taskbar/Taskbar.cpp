#include "Taskbar.h"
#include "TaskbarThread.h"

Taskbar::Taskbar()
{
	GetTaskbarInstance().AddReference();
}

Taskbar::~Taskbar()
{
	GetTaskbarInstance().RemoveReference();
}

std::vector<TaskbarItem>* Taskbar::GetTaskbarItems(bool grouped)
{
	return &GetTaskbarInstance().m_Thread->GetPrograms(grouped);
}

TakbarInstance& Taskbar::GetTaskbarInstance()
{
	static TakbarInstance taskbar;
	return taskbar;
}

void TakbarInstance::AddReference()
{
	if(m_References == 0 && !m_Thread)
	{
		m_Thread = std::make_unique<TaskbarThread>();
	}
	++m_References;
}

void TakbarInstance::RemoveReference()
{
	--m_References;
	if(m_References == 0)
	{
		m_Thread.reset();
		m_Thread = nullptr;
	}
}