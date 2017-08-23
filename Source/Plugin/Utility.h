#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <unordered_map>

enum class PairedPunctuation
{
	SingleQuote,
	DoubleQuote,
	Parentheses,
	Brackets,
	Braces,
	Guillemet
};

enum class IconSize
{
	Small = SHIL_SMALL,			// 16x16 - can be customized...
	Normal = SHIL_LARGE,		// 32x32 - can be 48x48 if icons set to bigger in the appearance tab in Display properties
	Big = SHIL_EXTRALARGE,		// 48x48 - can be customized...
	SysSmall = SHIL_SYSSMALL,	// This can aparently be any size... specified by GetSystemMetrics(SM_CXSMICON) icon used for top left in window i think
	Huge = SHIL_JUMBO			// 256x256 - usually
};

class IconResourceMap
{
public:
	const std::wstring& SaveIcon(const std::wstring& key, HICON icon);
	const std::wstring& SaveIcon(const std::wstring& key, const std::wstring& value);
	const std::wstring& GetIcon(const std::wstring& key);
	const std::wstring& GetIconPath();
	bool HasIcon(const std::wstring& key);
private:

	std::unordered_map<std::wstring, std::wstring> m_IconPaths;
	std::wstring m_IconPath;
	bool m_IsTemp;
};

class Utility
{
public:
	// Non-copyable
	Utility(Utility const&) = delete;
	void operator=(Utility const&) = delete;
	
	static std::wstring GetIconPath(const std::wstring& programName, HWND hwnd, bool isUWP, IconSize size);

	static IconResourceMap& GetResourceMap();

	static bool IsEqual(const LPCWSTR& a, const LPCWSTR& b, bool caseInsensitive = true);
	static std::vector<std::wstring> Tokenize2(const std::wstring& str, const WCHAR delimiter, const PairedPunctuation punct);

	static std::wstring GetPluginSetting(const std::wstring& key, const std::wstring& defaultValue);
	static int GetPluginSetting(const std::wstring& key, int defaultValue);
private:
	Utility();
	~Utility();
};

