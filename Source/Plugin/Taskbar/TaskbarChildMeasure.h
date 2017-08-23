#pragma once
#include "Taskbar.h"
#include "../BaseMeasure.h"
#include "TaskbarParentMeasure.h"

class TaskbarChildMeasure : public BaseMeasure
{
public:
	TaskbarChildMeasure(void* rm, const std::wstring& parent);
	~TaskbarChildMeasure();
	
	bool Reload() override;
	std::wstring GetValue() override;

	enum class Type
	{
		ProcessName,
		WindowTitle,
		WindowClass,
		Name,
		Icon,
		ProcessId,
		Path,
		VirtualDesktopId,
		IsPinned,
		Unknown
	};

private:
	TaskbarItem* GetItem();

	Type m_Type = Type::ProcessName;
	int m_Index = 0;
};

