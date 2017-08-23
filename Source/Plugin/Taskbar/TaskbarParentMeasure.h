#pragma once
#include "../BaseMeasure.h"
#include "Taskbar.h"
#include <vector>


class TaskbarParentMeasure : public BaseMeasure
{
public:
	TaskbarParentMeasure(void* rm);
	~TaskbarParentMeasure();

	static TaskbarParentMeasure* GetParent(const std::wstring& name, void* skin);

	Taskbar& GetTaskbar();

	bool Reload() override;
	std::wstring GetValue() override;
private:
	void* m_Skin;
	std::wstring m_Name;
	Taskbar m_Taskbar;
	static std::vector<TaskbarParentMeasure*> m_Parents;
};

