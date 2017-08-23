#include "Utility.h"
#include "../API/RainmeterAPI.h"
#include <unordered_map>
#include <Gdiplus.h>
#include <Shlwapi.h>
#include <Commctrl.h>
#include <commoncontrols.h>
#include <Windows.h>

namespace {

	struct PairInfo
	{
		const WCHAR begin;
		const WCHAR end;
	};

	const std::unordered_map<PairedPunctuation, PairInfo> s_PairedPunct =
	{
		{ PairedPunctuation::SingleQuote,{ L'\'', L'\'' } },
		{ PairedPunctuation::DoubleQuote,{ L'\"', L'\"' } },
		{ PairedPunctuation::Parentheses,{ L'(', L')' } },
		{ PairedPunctuation::Brackets,{ L'[', L']' } },
		{ PairedPunctuation::Braces,{ L'{', L'}' } },
		{ PairedPunctuation::Guillemet,{ L'<', L'>' } }
	};

}  // namespace

std::wstring Utility::GetIconPath(const std::wstring& programName, HWND hwnd, bool isUWP, IconSize size)
{
	std::wstring val;
	auto& map = GetResourceMap();

	if(map.HasIcon(programName))
	{
		val = map.GetIcon(programName);
	}
	else
	{
		HICON icon = reinterpret_cast<HICON>(::SendMessageW(hwnd, WM_GETICON, ICON_BIG, 0));
		if (icon == 0)
		{
			icon = reinterpret_cast<HICON>(::GetClassLongPtrW(hwnd, GCLP_HICON));
		}

		if(icon == 0)
		{
			icon = reinterpret_cast<HICON>(::GetClassLongPtrW(hwnd, GCLP_HICONSM));
		}
		/*
		if (icon == 0) {
			// Alternative method. Get from the window class
			icon = reinterpret_cast<HICON>(::GetClassLongPtrW(hwnd, GCLP_HICONSM));
		}

		// Alternative method: get the first icon from the main module (executable image of the process)
		if (icon == 0) {
			icon = ::LoadIcon(GetModuleHandleW(0), MAKEINTRESOURCE(0));
		}
		*/
		// Alternative method. Use OS default icon
		if (icon == 0) {
			icon = ::LoadIcon(0, IDI_APPLICATION);
		}
		if (icon)
		{
			val = map.SaveIcon(programName, icon);
		}
		else
		{
			//TODO: Some default icon
			val = L"";
		}
		/*
		if (!isUWP)
		{
			// Get the image list index of the icon
			SHFILEINFO sfi;
			if (SHGetFileInfo(programPath.c_str(), 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX))
			{
				// Get the jumbo image list
				IImageList *piml;
				if (SUCCEEDED(SHGetImageList((int)size, IID_PPV_ARGS(&piml))))
				{
					// Extract an icon
					HICON hico;
					piml->GetIcon(sfi.iIcon, 0x00000001, &hico);
					if (hico)
					{
						val = map.SaveIcon(programName, hico);
					}
					else
					{
						//TODO: Some default icon
						val = L"";
					}
					piml->Release();
				}
			}
		}
		else
		{
			// TODO:
			val = L"";
		}*/
	}
	return val;
}

bool Utility::IsEqual(const LPCWSTR& a, const LPCWSTR& b, bool caseInsensitive)
{
	if (caseInsensitive)
	{
		return _wcsnicmp(a, b, wcslen(b)) == 0;
	}
	return wcscmp(a, b) == 0;
}

std::vector<std::wstring> Utility::Tokenize2(const std::wstring& str, const WCHAR delimiter, const PairedPunctuation punct)
{
	std::vector<std::wstring> tokens;

	int pairs = 0;
	size_t start = 0;
	size_t end = 0;

	auto getToken = [&]() -> void
	{
		start = str.find_first_not_of(L" \t\r\n", start); // skip any leading whitespace
		if (start <= end)
		{
			std::wstring temp = str.substr(start, end - start);
			temp.erase(temp.find_last_not_of(L" \t\r\n") + 1); // remove any trailing whitespace
			tokens.push_back(temp);
		}
	};

	for (auto& iter : str)
	{
		if (iter == s_PairedPunct.at(punct).begin) ++pairs;
		else if (iter == s_PairedPunct.at(punct).end) --pairs;
		else if (iter == delimiter && pairs == 0)
		{
			getToken();
			start = end + 1;  // skip delimiter
		}

		++end;
	}

	// Get last token
	getToken();

	return tokens;
}

// Supports only 255 characters for now...
std::wstring Utility::GetPluginSetting(const std::wstring& key, const std::wstring& defaultValue)
{
	std::wstring settingFile = RmGetSettingsFile();

	wchar_t res[MAX_PATH];
	GetPrivateProfileString(L"Taskbar Plugin", key.c_str(), L"", res, MAX_PATH, settingFile.c_str());
	std::wstring result(res);
	
	if(IsEqual(result.c_str(), L"", false))
	{
		result = defaultValue;
		WritePrivateProfileString(L"Taskbar Plugin", key.c_str(), result.c_str(), settingFile.c_str());
	}

	return result;
}

int Utility::GetPluginSetting(const std::wstring& key, int defaultValue)
{
	std::wstring settingFile = RmGetSettingsFile();

	
	int result = GetPrivateProfileInt(L"Taskbar Plugin", key.c_str(), 0, settingFile.c_str());

	if (result == 0)
	{
		result = defaultValue;
		WritePrivateProfileString(L"Taskbar Plugin", key.c_str(), std::to_wstring(defaultValue).c_str(), settingFile.c_str());
	}

	return result;
}

Utility::Utility()
{
}

Utility::~Utility()
{
}

IconResourceMap& Utility::GetResourceMap()
{
	static IconResourceMap map;
	return map;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

struct BITMAP_AND_BYTES {
	Gdiplus::Bitmap* bmp;
	int32_t* bytes;
};

static BITMAP_AND_BYTES createAlphaChannelBitmapFromIcon(HICON hIcon) {

	// Get the icon info
	ICONINFO iconInfo = { 0 };
	GetIconInfo(hIcon, &iconInfo);

	// Get the screen DC
	HDC dc = GetDC(NULL);

	// Get icon size info
	BITMAP bm = { 0 };
	GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bm);

	// Set up BITMAPINFO
	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = bm.bmWidth;
	bmi.bmiHeader.biHeight = -bm.bmHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	// Extract the color bitmap
	int nBits = bm.bmWidth * bm.bmHeight;
	int32_t* colorBits = new int32_t[nBits];
	GetDIBits(dc, iconInfo.hbmColor, 0, bm.bmHeight, colorBits, &bmi, DIB_RGB_COLORS);

	// Check whether the color bitmap has an alpha channel.
	// (On my Windows 7, all file icons I tried have an alpha channel.)
	BOOL hasAlpha = FALSE;
	for (int i = 0; i < nBits; i++) {
		if ((colorBits[i] & 0xff000000) != 0) {
			hasAlpha = TRUE;
			break;
		}
	}

	// If no alpha values available, apply the mask bitmap
	if (!hasAlpha) {
		// Extract the mask bitmap
		int32_t* maskBits = new int32_t[nBits];
		GetDIBits(dc, iconInfo.hbmMask, 0, bm.bmHeight, maskBits, &bmi, DIB_RGB_COLORS);
		// Copy the mask alphas into the color bits
		for (int i = 0; i < nBits; i++) {
			if (maskBits[i] == 0) {
				colorBits[i] |= 0xff000000;
			}
		}
		delete[] maskBits;
	}

	// Release DC and GDI bitmaps
	ReleaseDC(NULL, dc);
	::DeleteObject(iconInfo.hbmColor);
	::DeleteObject(iconInfo.hbmMask);

	// Create GDI+ Bitmap
	Gdiplus::Bitmap* bmp = new Gdiplus::Bitmap(bm.bmWidth, bm.bmHeight, bm.bmWidth * 4, PixelFormat32bppARGB, (BYTE*)colorBits);
	BITMAP_AND_BYTES ret = { bmp, colorBits };

	return ret;
}

static void saveFileIconAsPng(HICON hIcon, const std::wstring& pngFile) {
	BITMAP_AND_BYTES bbs = createAlphaChannelBitmapFromIcon(hIcon);

	IStream* fstrm = NULL;
	CLSID encoderClsid;
	GetEncoderClsid(L"image/png", &encoderClsid);
	bbs.bmp->Save(pngFile.c_str(), &encoderClsid, NULL);

	delete bbs.bmp;
	delete[] bbs.bytes;
	DestroyIcon(hIcon);
}

const std::wstring& IconResourceMap::SaveIcon(const std::wstring& key, HICON icon)
{
	std::wstring other = GetIconPath();
	std::wstring path = other + key + L".png";
	saveFileIconAsPng(icon, path);
	m_IconPaths.insert({ key, path });
	return m_IconPaths[key];
}

const std::wstring& IconResourceMap::SaveIcon(const std::wstring& key, const std::wstring& icon)
{
	//TODO:
	/*std::wstring other = GetIconPath();
	std::wstringstream ss;
	std::wstring fuckthiscopy(key);
	fuckthiscopy.resize(fuckthiscopy.size() - 1);
	ss << other << fuckthiscopy << L".png";
	UWPIconFetcher::CopyIconToPath(icon, ss.str());
	m_IconPaths.insert({ key, ss.str() });
	return m_IconPaths[key];*/
	return L"";
}

const std::wstring& IconResourceMap::GetIcon(const std::wstring& key)
{
	if (HasIcon(key))
	{
		return m_IconPaths[key];
	}
	return L"";
}

const std::wstring& IconResourceMap::GetIconPath()
{
	// TODO: Prefer path from settings...

	if (m_IconPath.empty())
	{
		m_IsTemp = true;
		wchar_t string[MAX_PATH];
		// TODO: This may fail, i'm not sure what we should do if it does..
		GetTempPath(MAX_PATH, string);
		m_IconPath = std::wstring(string) + L"Icons\\";
		CreateDirectory(m_IconPath.c_str(), NULL);
	}
	return m_IconPath;
}

bool IconResourceMap::HasIcon(const std::wstring& key)
{
	return m_IconPaths.find(key) != m_IconPaths.end();
}
