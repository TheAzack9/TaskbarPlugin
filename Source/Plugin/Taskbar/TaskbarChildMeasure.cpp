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
		if (Utility::IsEqual(str.c_str(), L"WindowTitle")) return Type::WindowTitle;
		if (Utility::IsEqual(str.c_str(), L"WindowClass")) return Type::WindowClass;
		if (Utility::IsEqual(str.c_str(), L"Name")) return Type::Name;
		if (Utility::IsEqual(str.c_str(), L"Icon")) return Type::Icon;
		if (Utility::IsEqual(str.c_str(), L"ProcessId")) return Type::ProcessId;
		if (Utility::IsEqual(str.c_str(), L"Path")) return Type::Path;
		if (Utility::IsEqual(str.c_str(), L"VirtualDesktopId")) return Type::VirtualDesktopId;
		if (Utility::IsEqual(str.c_str(), L"IsPinned")) return Type::IsPinned;
		if (Utility::IsEqual(str.c_str(), L"GroupCount")) return Type::GroupCount; // TODO: error if parent != grouped
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
	case Type::GroupCount: return std::to_wstring(item->m_GroupCount);
	default: RmLog(m_Rm, LOG_ERROR, L"Unknown type"); return L""; // Unknown type...
	}
}

TaskbarItem* TaskbarChildMeasure::GetItem()
{
	if (m_Index < 0) return nullptr;
	// TODO: Save this in child and callback on destruction to minimize cost (goes from linear to constant time)
	auto parent = TaskbarParentMeasure::GetParent( m_Parent, RmGetSkin(m_Rm) );
	if (!parent) return nullptr;

	return parent->GetItem(m_Index);
}