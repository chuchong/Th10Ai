#include "Base/Common.h"
#include "Base/String.h"

#include <boost/locale.hpp>

namespace base
{
	namespace blc = boost::locale::conv;

	std::wstring String::Utf8ToWide(const char* str)
	{
		return blc::utf_to_utf<wchar_t>(str);
	}

	std::wstring String::Utf8ToWide(const std::string& str)
	{
		return blc::utf_to_utf<wchar_t>(str);
	}

	std::string String::WideToUtf8(const wchar_t* str)
	{
		return blc::utf_to_utf<char>(str);
	}

	std::string String::WideToUtf8(const std::wstring& str)
	{
		return blc::utf_to_utf<char>(str);
	}
}
