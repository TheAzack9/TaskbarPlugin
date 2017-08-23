#include "TaskbarParentMeasure.h"
#include "../../API/RainmeterAPI.h"

std::vector<TaskbarParentMeasure*> TaskbarParentMeasure::m_Parents;

TaskbarParentMeasure::TaskbarParentMeasure(void* rm) : BaseMeasure(rm), m_Taskbar()
{
	m_Skin = RmGetSkin(rm);
	m_Name = RmGetMeasureName(rm);
	m_Parents.emplace_back(this);
}

TaskbarParentMeasure::~TaskbarParentMeasure()
{
	std::vector<TaskbarParentMeasure*>::iterator iter = std::find(m_Parents.begin(), m_Parents.end(), this);
	m_Parents.erase(iter);
}

TaskbarParentMeasure* TaskbarParentMeasure::GetParent(const std::wstring& name, void* skin)
{
	auto iter = m_Parents.begin();
	for(;iter != m_Parents.end(); ++iter)
	{
		if (skin == (*iter)->m_Skin && Utility::IsEqual(name.c_str(), (*iter)->m_Name.c_str(), false))
		{
			return *iter;
		}
	}
}

Taskbar& TaskbarParentMeasure::GetTaskbar()
{
	return m_Taskbar;
}

bool TaskbarParentMeasure::Reload()
{
	return true;
}

std::wstring TaskbarParentMeasure::GetValue()
{
	return std::to_wstring(m_Taskbar.GetTaskbarItems()->size());
}
