#pragma once

namespace CppTools
{
	/// Parent structure for all exceptions.
	struct XException : public virtual std::exception
	{
		/// Public ctor.
		/// @param functionName signature of the caller.
		/// @param sourceCodeLocation file and line to refer to later.
		/// @param sourceCodeContent error description or source code line.
		XException(char const * const functionName, char const * const sourceCodeLocation, char const * const sourceCodeContent = nullptr)
			: m_FunctionName(functionName), m_SourceCodeLocation(sourceCodeLocation), m_Condition(sourceCodeContent) { }

		/// Overrides.
		virtual char const * what() const noexcept { return "XException"; }

		/// Composes message containing all available information.
		/// @return composed message.
		virtual std::tstring ComposeFullMessage() const noexcept
		{
			return (m_Condition.empty() ? TEXT("") : TEXT("Condition : ") + CppTools::StringConversions::Mbcs2Tstring(m_Condition) + TEXT("\n")) + TEXT("Function : ")
				+ CppTools::StringConversions::Mbcs2Tstring(m_FunctionName) + TEXT("\nSource Code : ") + CppTools::StringConversions::Mbcs2Tstring(m_SourceCodeLocation);
		}

		/// Composes message containing only the basic information.
		/// @return composed message.
		virtual std::tstring ComposeSimpleMessage() const noexcept
			{ return m_Condition.empty() ? TEXT("") : TEXT("Condition : ") + CppTools::StringConversions::Mbcs2Tstring(m_Condition) + TEXT("\n"); }

	private:
		std::string m_FunctionName, m_SourceCodeLocation, m_Condition;			///< Strings containing particular information.
	};

	/// Struct for invalid parameters exceptions.
	struct InvalidParamException : public virtual XException
	{
		/// Inherited public ctor.
		using XException::XException;

		/// Overrides.
		virtual char const * what() const noexcept { return "InvalidParamException"; }
	};

	// Macros.
#	define XEXCEPTION_PARAMS __FUNCSIG__, __FILE__ "(" STRINGITIZE(__LINE__) ")"
#	define ValidateParam(x) { if (!(x)) throw CppTools::InvalidParamException(XEXCEPTION_PARAMS, #x); }
};
