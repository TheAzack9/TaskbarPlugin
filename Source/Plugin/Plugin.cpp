#include <Windows.h>
#include "../API/RainmeterAPI.h"
#include "Utility.h"
#include "BaseMeasure.h"
#include <string>
#include "Taskbar/TaskbarParentMeasure.h"
#include "Taskbar/TaskbarChildMeasure.h"

struct Measure
{
	Measure() {}
	BaseMeasure* m_Measure = nullptr;

	enum class Type
	{
		Taskbar
	} m_Type;

	std::wstring m_LastValue = L"";
};

PLUGIN_EXPORT void Initialize(void** data, void* rm)
{
	Measure* measure = new Measure;
	*data = measure;
}

PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue)
{
	Measure* measure = (Measure*)data;

	// Decide module type
	std::wstring type = RmReadString(rm, L"Type", L"");
	if(Utility::IsEqual(type.c_str(), L"Taskbar"))
	{
		std::wstring parent = RmReadString(rm, L"Parent", L"");
		// Has parent state changed
		if(measure->m_Type == Measure::Type::Taskbar && measure->m_Measure && !Utility::IsEqual(parent.c_str(), measure->m_Measure->GetParent().c_str()))
		{
			delete measure->m_Measure;
		}

		if (!measure->m_Measure)
		{
			measure->m_Type = Measure::Type::Taskbar;
			if (parent.empty())
			{
				measure->m_Measure = new TaskbarParentMeasure(rm);
			}
			else
			{
				measure->m_Measure = new TaskbarChildMeasure(rm, parent);
			}
		}
	}

	if(measure->m_Measure)
	{
		measure->m_Measure->Reload();
	}
}

PLUGIN_EXPORT double Update(void* data)
{
	Measure* measure = (Measure*)data;
	return 0.0;
}

PLUGIN_EXPORT LPCWSTR GetString(void* data)
{
	Measure* measure = (Measure*)data;
	if (measure->m_Measure)
	{
		measure->m_LastValue = measure->m_Measure->GetValue();
	}
	else
	{
		measure->m_LastValue = L"";
	}
	return measure->m_LastValue.c_str();
}

PLUGIN_EXPORT void ExecuteBang(void* data, LPCWSTR args)
{
	Measure* measure = (Measure*)data;
}

PLUGIN_EXPORT void Finalize(void* data)
{
	Measure* measure = (Measure*)data;
	if(measure->m_Measure)
	{
		delete measure->m_Measure;
	}
	delete measure;
}
