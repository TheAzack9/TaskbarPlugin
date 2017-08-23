#include "BaseMeasure.h"
#include "Utility.h"


BaseMeasure::BaseMeasure(void* rm, std::wstring parent) : m_Rm(rm), m_Parent(parent)
{
}


BaseMeasure::~BaseMeasure()
{
}

const std::wstring& BaseMeasure::GetParent()
{
	return m_Parent;
}

void BaseMeasure::ExecuteBang(const std::wstring& args)
{
	std::vector<std::wstring> parameters = Utility::Tokenize2(args, L' ', PairedPunctuation::SingleQuote);

	if (parameters.size() < 1) return;

	auto first = parameters[0];
	parameters.erase(parameters.begin());
	for(auto it = m_Bangs.begin(); it != m_Bangs.end(); ++it)
	{
		if(Utility::IsEqual(first.c_str(), it->first.c_str()))
		{
			it->second(parameters);
		}
	}
}

void BaseMeasure::AddBang(const std::wstring& identifier, const std::function<void(std::vector<std::wstring>)>& callback)
{
	m_Bangs[identifier] = callback;
}
