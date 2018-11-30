#pragma once

namespace WinTools
{
	/// Exception structure for WinApi calls.
	struct WinApiHrException : public virtual CppTools::XException
	{
		/// Public ctor.
		/// @param hr error code to store.
		/// @param functionName signature of the caller.
		/// @param sourceCodeLocation file and line to refer to later.
		/// @param sourceCodeContent error description or source code line.
		WinApiHrException(HRESULT hr, char const * const functionName, char const * const sourceCodeLocation, char const * const sourceCodeContent)
			: XException(functionName, sourceCodeLocation, sourceCodeContent), m_Hresult(hr) { }

		/// Overrides.
		virtual char const * what() const noexcept override { return "WinApiHrException"; }

		/// Composes message containing all available information.
		/// @return composed message.
		virtual std::tstring ComposeFullMessage() const noexcept override
			{ return TEXT("HRESULT : ") + WinTools::ConvertHresultToMessageWithHresult(m_Hresult) + TEXT("\n") + XException::ComposeFullMessage(); }

		/// Composes message containing only the basic information.
		/// @return composed message.
		virtual std::tstring ComposeSimpleMessage() const noexcept override
			{ return TEXT("HRESULT : ") + WinTools::ConvertHresultToMessageWithHresult(m_Hresult) + TEXT("\n") + XException::ComposeSimpleMessage(); }

	protected:
		HRESULT m_Hresult;														//< Underlying error code.
	};

	// Macros.
#	define CallWinApiHr(x) { auto hresult_ = x; if (FAILED(hresult_)) throw WinTools::WinApiHrException(hresult_, XEXCEPTION_PARAMS, #x); }
}
