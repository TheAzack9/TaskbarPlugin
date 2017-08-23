#pragma once
#include <string>
#include <map>
#include <functional>
#include <vector>

class BaseMeasure
{
public:
	BaseMeasure(void* rm, std::wstring parent = L"");
	virtual ~BaseMeasure();

	const std::wstring& GetParent();
	void ExecuteBang(const std::wstring& args);

	virtual bool Reload() = 0;
	virtual std::wstring GetValue() = 0;
protected:
	void AddBang(const std::wstring& identifier, const std::function<void(std::vector < std::wstring>)>& callback);
	
	void* m_Rm;
	std::wstring m_Parent;
	
private:
	std::map<std::wstring, std::function<void(std::vector<std::wstring>)>> m_Bangs;
};
