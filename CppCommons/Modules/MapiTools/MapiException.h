#pragma once

namespace MapiTools
{
	/// Exception structure for WinApi calls.
	struct MapiException : public virtual WinTools::WinApiHrException
	{
		/// Public ctor.
		/// @param hr error code to store.
		/// @param functionName signature of the caller.
		/// @param sourceCodeLocation file and line to refer to later.
		/// @param sourceCodeContent error description or source code line.
		MapiException(HRESULT hr, char const * const functionName, char const * const sourceCodeLocation, char const * const sourceCodeContent)
			: XException(functionName, sourceCodeLocation, sourceCodeContent), WinApiHrException(hr, functionName, sourceCodeLocation, sourceCodeContent) { }

		/// Overrides.
		virtual char const * what() const noexcept override { return "MapiException"; }

		/// Composes message containing all available information.
		/// @return composed message.
		virtual std::tstring ComposeFullMessage() const noexcept override
			{ return TEXT("HRESULT : ") + WinTools::ConvertHresultToMessageWithHresult(m_Hresult) + TEXT("\n") + XException::ComposeFullMessage(); }

		/// Composes message containing only the basic information.
		/// @return composed message.
		virtual std::tstring ComposeSimpleMessage() const noexcept override
			{ return TEXT("HRESULT : ") + WinTools::ConvertHresultToMessageWithHresult(m_Hresult) + TEXT("\n") + XException::ComposeSimpleMessage(); }
	};

	// Macros.
#	define CallMapi(x) { auto hresult_ = x; if (FAILED(hresult_)) throw MapiTools::MapiException(hresult_, XEXCEPTION_PARAMS, #x); }
}
