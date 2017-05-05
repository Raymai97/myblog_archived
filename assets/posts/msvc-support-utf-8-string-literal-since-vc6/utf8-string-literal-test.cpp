#include <Windows.h>
#include <sstream>

char const * const g_TestStrs[] = {
	"العربية",
	"Български",
	"繁体中文",
	"简体中文",
	"hrvatski",
	"čeština",
	"English",
	"français",
	"Deutsch",
	"Ελληνικά",
	"עברית",
	"हिंदी",
	"日本語",
	"Қазақ",
	"한국어‫",
	"latviešu",
	"lietuvių",
	"bokmål",
	"Português",
	"română",
	"Русский",
	"slovenčina",
	"Español",
	"ไทย",
	"Türkçe",
	"українська",
	"Tiếng Việt"
};

std::wstring WideFromUtf8(std::string const & utf8)
{
	int cchWide = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
	LPWSTR szWide = new wchar_t[cchWide];
	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, szWide, cchWide);
	std::wstring wide(szWide);
	delete[] szWide;
	return wide;
}

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	std::wstringstream ss;
	int const count = sizeof(g_TestStrs) / sizeof(*g_TestStrs);
	for (int i = 0; i < count; ++i)
	{
		ss << WideFromUtf8(g_TestStrs[i]) << L"  ";
		if ((i+1) % 5 == 0) { ss << L"\n"; }
	}
	MessageBoxW(NULL, ss.str().c_str(),
		L"UTF-8 string literal test",
		MB_ICONINFORMATION);
	return 0;	
}
