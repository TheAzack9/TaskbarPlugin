#pragma once
#include "../BaseMeasure.h"
#include "Taskbar.h"
#include <vector>


class TaskbarParentMeasure : public BaseMeasure
{
public:
	TaskbarParentMeasure(void* rm);
	~TaskbarParentMeasure();
	TaskbarItem* GetItem(int index);

	static TaskbarParentMeasure* GetParent(const std::wstring& name, void* skin);

	Taskbar& GetTaskbar();

	bool Reload() override;
	std::wstring GetValue() override;
private:
	void* m_Skin;
	std::wstring m_Name;
	bool m_Group;
	Taskbar m_Taskbar;
	static std::vector<TaskbarParentMeasure*> m_Parents;
};

