#include "TaskbarChildMeasure.h"
#include "../../API/RainmeterAPI.h"
#include "../Utility.h"

TaskbarChildMeasure::TaskbarChildMeasure(void* rm, const std::wstring& parent): BaseMeasure(rm, parent)
{
}

TaskbarChildMeasure::~TaskbarChildMeasure()
{
}

bool TaskbarChildMeasure::Reload()
{
	auto convertEnum = [](const std::wstring& str)
	{
		if (Utility::IsEqual(str.c_str(), L"ProcessName")) return Type::ProcessName;
		else if (Utility::IsEqual(str.c_str(), L"WindowTitle")) return Type::WindowTitle;
		else if (Utility::IsEqual(str.c_str(), L"WindowClass")) return Type::WindowClass;
		else if (Utility::IsEqual(str.c_str(), L"Name")) return Type::Name;
		else if (Utility::IsEqual(str.c_str(), L"Icon")) return Type::Icon;
		else if (Utility::IsEqual(str.c_str(), L"ProcessId")) return Type::ProcessId;
		else if (Utility::IsEqual(str.c_str(), L"Path")) return Type::Path;
		else if (Utility::IsEqual(str.c_str(), L"VirtualDesktopId")) return Type::VirtualDesktopId;
		else if (Utility::IsEqual(str.c_str(), L"IsPinned")) return Type::IsPinned;
		return Type::Unknown;
	};

	std::wstring type = RmReadString(m_Rm, L"TaskbarType", L"");
	m_Type = convertEnum(type.c_str());

	if(m_Type == Type::Unknown)
	{
		// TODO: Error, unknown type
		RmLog(m_Rm, LOG_ERROR, (L"Invalid type: " + type).c_str());
		return false;
	}

	m_Index = RmReadInt(m_Rm, L"Index", 0);

	AddBang(L"Show", [&](const std::vector<std::wstring>& parameters)
	{
		RmLog(m_Rm, LOG_DEBUG, L"Worked \\o/");
	});

	return true;
}

std::wstring TaskbarChildMeasure::GetValue()
{
	auto item = GetItem();
	if (!item) return L""; // Found nothing :C
	//auto item = ;
	switch(m_Type)
	{
	case Type::ProcessName: return item->m_ProcessName;
	case Type::WindowTitle: return item->m_WindowTitle;
	case Type::WindowClass: return item->m_WindowClass;
	case Type::Name: return item->m_ProgramDescription;
	case Type::Icon: return item->GetIconPath();
	case Type::ProcessId: return std::to_wstring(item->m_ProcessId);
	case Type::Path: return item->m_ProgramPath;
	case Type::VirtualDesktopId: return std::to_wstring(item->GetVirtualDesktopId());
	case Type::IsPinned: return item->m_IsPinned ? L"1" : L"0";
	default: RmLog(m_Rm, LOG_ERROR, L"Unknown type"); return L""; // Unknown type...
	}
}

TaskbarItem* TaskbarChildMeasure::GetItem()
{
	if (m_Index < 0) return nullptr;
	// TODO: Save this in child and callback on destruction to minimize cost (goes from linear to constant time)
	auto parent = TaskbarParentMeasure::GetParent( m_Parent, RmGetSkin(m_Rm) );
	if (!parent) return nullptr;

	auto& taskbar = parent->GetTaskbar();
	const auto items = taskbar.GetTaskbarItems();
	if (!items) return nullptr;

	if (m_Index >= items->size()) return nullptr;
	return &(*items)[m_Index];
}