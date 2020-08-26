#include "Windows/WindowsError.h"

#include "Windows/Apis.h"

namespace win
{
	WindowsError::WindowsError(DWORD errorId) :
		m_errorId(errorId)
	{
	}

	void WindowsError::print(std::ostream& os) const
	{
		os << '[' << m_errorId << ']' << Apis::GetErrorDesc(m_errorId);

		printSourceLocation(os);
		printStackTrace(os);
	}
}
