//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @page XErrorImplementationPage XError
/// ### XError Overview
/// XError is a handy wrapper for all error types returned by various APIs. Together with \c XErrorLogger object, XErrors are able to trace error propagation and even generate complete
/// callstack dumps on demand. It's main purpose is to wrap API calls return types like \c HRESULT, \c decltype(GetLastError()), etc. and provide a global standard of error handling along with a handy
/// tool set (like \c .\c IsFailure()). XError doesn't aggregate strings (like file and line numbers) because of security reasons. In order to rebuild call tree, use \c XErrorLogger.\c SetCallback(),
/// gather everything it logs, and parse it against PDBs.
/// @note If you need to, you can chain returned XErrors by creating more and more wider wrappers - \c XError(XError(XError)). This is another way to regenerate error propagation apart from
/// \c XErrorLogger.\c SetCallback() + parsing PDB.
/// @warning There is no \c XError<decltype(GetLastError())>. Value got by calling \c GetLastError() should be converted immediately to \c HRESULT, e.g. by using macros \c XERROR_GETLASTERROR and
/// \c XERRORRAW_GETLASTERROR.
///
/// XError templates automatically detect if provided underlying type has \c bool \c UnderlyingType::IsSuccess() \c const and \c bool \c UnderlyingType::IsFailure() \c const methods, and exposes
/// corresponding
/// \c IsSuccess and \c IsFailure methods by itself. That process can be stopped by using \c XErrorRaw template instead of XError. This also applies to pointers, enumeration types, and \c HRESULT which
/// all get both \c IsSuccess and \c IsFailure defined automatically in their wrapping XError instances.
/// @note None of \c bool \c UnderlyingType::IsSuccess() \c const and \c bool \c UnderlyingType::IsFailure() \c const methods are required. It's up to you what your UnderlyingType exposes.
/// @note Automatically generated \c IsSuccess and \c IsFailure for pointers are logical \c ptr \c != \c nullptr tests. For \c HRESULT, macros \c SUCCEEDED and \c FAILED are used. For enumeration types, success test is a
/// comparison to zero. If those implementations are not fulfilling your needs, use \c XErrorRaw() instead of \c XError(), or wrap your \c pointer/\c enum/\c HRESULT in a structure that exposes different logic in
/// \c IsSuccess and \c IsFailure implementations.
/// @note Special template functions called Creators were implemented to ease the problem of explicit template param specification. So instead of \c auto \c x = \c XError<enumType>(enumValue) or
/// \c auto \c x = \c XError<decltype(enumValue)>(enumValue), you can just use \c auto \c x = \c XErrorCreator(enumValue); @see XErrorCreator, XErrorRawCreator, XErrorEnumWithHresultCreator.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace CppTools
{
	/// XErrorLogger implementation. This is a singleton which is used to redirect all XError creation event notifications into one place. Can be used to aggregate data that can be compared further
	/// against program's PDB.
	struct XErrorLogger_t
	{
		/// Signature of the logger callback function.
		typedef std::function<void(void* value)> XErrorLoggerCallback_t;

		/// A handy function-call operator used for logging.
		/// @param value newly created XError is being initialized with this value.
		void operator()(void* value) { if (GetCallback()) m_Callback(value); }

		/// A handy assignment operator used for appending XErrorLogger with another logger callback.
		/// @param newHandler new logger callback function.
		void operator= (XErrorLoggerCallback_t newHandler) { m_Callback = newHandler; }

		/// Sets another logger callback.
		/// @param callback new logger callback function.
		void SetCallback(XErrorLoggerCallback_t callback) { m_Callback = callback; }

		/// Gets current logger callback.
		/// @return current callback function.
		XErrorLoggerCallback_t GetCallback() const { return m_Callback; }

	private:
		XErrorLoggerCallback_t m_Callback = nullptr;							///< The logger callback.
	};
	extern XErrorLogger_t XErrorLogger;											///< Definition of a XErrorLogger singleton.

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#	pragma region Internal structures

	/// Base, abstract structure for all XError incarnations.
	template <typename UnderlyingType> struct XErrorBase
	{
		/// Template params storage.
		typedef UnderlyingType UnderlyingType;

		/// Public ctor.
		/// @param value initialization value.
		XErrorBase(UnderlyingType value) : m_Value(value) { XErrorLogger(&value); }

		/// UnderlyingType conversion operator.
		/// @return XError's UnderlyingObject's current value.
		operator UnderlyingType() { return m_Value; }

		/// Constant UnderlyingType conversion operator.
		/// @return XError's UnderlyingObject's current value.
		operator UnderlyingType() const { return m_Value; }

		/// Dereference operator.
		/// @return XError's UnderlyingObject's current value.
		UnderlyingType* operator * () { return &m_Value; }

		/// Constant dereference operator.
		/// @return XError's UnderlyingObject's current value.
		UnderlyingType* operator * () const { return &m_Value; }

		/// Arrow operator.
		/// @return XError's UnderlyingObject's current value.
		UnderlyingType* operator -> () { return &m_Value; }

		/// Constant arrow operator.
		/// @return XError's UnderlyingObject's current value.
		UnderlyingType* operator -> () const { return &m_Value; }

	protected:
		UnderlyingType m_Value;													///< Underlying value.
	};

	/// SFINAE for IsSuccess and IsFailure member functions. Used by XError template to deduce its contents.
	template<typename Type> class XErrorContentDeducer
	{
		typedef char yes[1];													///< Return type used to describe positive SFINAE resolution.
		typedef char no[2];														///< Return type used to describe negative SFINAE resolution.
		template<typename U, bool (U::*)() const> struct SFINAE {};				///< Function signature. Note that it's the same for both IsSuccess and IsFailure.

		template<typename U> static yes& TestIsSuccess(SFINAE<U, &U::IsSuccess>*);					///< Positive tester for IsSuccess.
		template<typename U> static no& TestIsSuccess(...);						///< Negative tester for IsSuccess.

		template<typename U> static yes& TestIsFailure(SFINAE<U, &U::IsFailure>*);					///< Positive tester for IsFailure.
		template<typename U> static no& TestIsFailure(...);						///< Negative tester for IsFailure.

	public:
		static const bool hasIsSuccess = sizeof(TestIsSuccess<Type>(nullptr)) == sizeof(yes);		///< Final tester for IsSuccess.
		static const bool hasIsFailure = sizeof(TestIsFailure<Type>(nullptr)) == sizeof(yes);		///< Final tester for IsFailure.
	};

	/// Abstract parent template to all XErrorImpl. @note You shouldn't use XErrorImpl templates directly. They have internal purposes. Use XError template instead.
	template <typename UnderlyingType, bool HasIsSuccessMethod, bool HasIsFailureMethod, bool IsPointer, bool IsEnum> struct XErrorImpl;

	/// XError specialization for types devoid of both IsSuccess and IsFailure methods. @note You shouldn't use XErrorImpl templates directly. They have internal purposes. Use XError template instead.
	template <typename UnderlyingType> struct XErrorImpl<UnderlyingType, false, false, false, false> : public XErrorBase<UnderlyingType>
	{
		/// Public ctor.
		/// @param value object value
		XErrorImpl(UnderlyingType value) : XErrorBase(value) { }
	};

	/// XError specialization for types that have no IsFailure method but do have IsSuccess method implemented. @note You shouldn't use XErrorImpl templates directly. They have internal purposes. Use XError template instead.
	template <typename UnderlyingType> struct XErrorImpl<UnderlyingType, true, false, false, false> : public XErrorBase<UnderlyingType>
	{
		/// Public ctor.
		/// @param value object value
		XErrorImpl(UnderlyingType value) : XErrorBase(value) { }

		/// Success translator.
		/// @return true if current state of the object indicates success.
		bool IsSuccess() const { return m_Value.IsSuccess(m_Value); }
	};

	/// XError specialization for types that have no IsSuccess method but do have IsFailure method implemented. @note You shouldn't use XErrorImpl templates directly. They have internal purposes. Use XError template instead.
	template <typename UnderlyingType> struct XErrorImpl<UnderlyingType, false, true, false, false> : public XErrorBase<UnderlyingType>
	{
		/// Public ctor.
		/// @param value object value
		XErrorImpl(UnderlyingType value) : XErrorBase(value) { }

		/// Failure translator.
		/// @return true if current state of the object indicates failure.
		bool IsFailure() const { return m_Value.IsFailure(); }
	};

	/// XError specialization for types that have both IsSuccess and IsFailure methods implemented. @note You shouldn't use XErrorImpl templates directly. They have internal purposes. Use XError template instead.
	template <typename UnderlyingType> struct XErrorImpl<UnderlyingType, true, true, false, false> : public XErrorBase<UnderlyingType>
	{
		/// Public ctor.
		/// @param value object value
		XErrorImpl(UnderlyingType value) : XErrorBase(value) { }

		/// Success translator.
		/// @return true if current state of the object indicates success.
		bool IsSuccess() const { return m_Value.IsSuccess(); }

		/// Failure translator.
		/// @return true if current state of the object indicates failure.
		bool IsFailure() const { return m_Value.IsFailure(); }
	};

	/// XError specialization for HRESULT - let's provide IsSuccess and IsFailure. @note You shouldn't use XErrorImpl templates directly. They have internal purposes. Use XError template instead.
	template <> struct XErrorImpl<HRESULT, false, false, false, false> : public XErrorBase<HRESULT>
	{
		/// Public ctor.
		/// @param value object value
		XErrorImpl(HRESULT value) : XErrorBase(value) { }

		/// Success translator.
		/// @return true if current state of the object indicates success.
		bool IsSuccess() const { return SUCCEEDED(m_Value); }

		/// Failure translator.
		/// @return true if current state of the object indicates failure.
		bool IsFailure() const { return FAILED(m_Value); }
	};

	/// XError specialization for bool type. IsSuccess and IsFailure methods are just value redirectors. @note You shouldn't use XErrorImpl templates directly. They have internal purposes. Use XError template instead.
	template <> struct XErrorImpl<bool, false, false, false, false> : public XErrorBase<bool>
	{
		/// Public ctor.
		/// @param value object value
		XErrorImpl(bool value) : XErrorBase(value) { }

		/// Success translator.
		/// @return true if current state of the object indicates success.
		bool IsSuccess() const { return m_Value; }

		/// Failure translator.
		/// @return true if current state of the object indicates failure.
		bool IsFailure() const { return !m_Value; }
	};

	/// XError specialization for pointers. IsSuccess and IsFailure are comparisons to nullptr. @note You shouldn't use XErrorImpl templates directly. They have internal purposes. Use XError template instead.
	template <typename UnderlyingPointerType> struct XErrorImpl<UnderlyingPointerType, false, false, true, false> : public XErrorBase<UnderlyingPointerType>
	{
		/// Public ctor.
		/// @param value object value
		XErrorImpl(UnderlyingPointerType value) : XErrorBase(value) { }

		/// Success translator.
		/// @return true if current state of the object indicates success.
		bool IsSuccess() const { return m_Value; }

		/// Failure translator.
		/// @return true if current state of the object indicates failure.
		bool IsFailure() const { return !m_Value; }
	};

	/// XError specialization for enums. We assume here that value equal to zero means success. @note You shouldn't use XErrorImpl templates directly. They have internal purposes. Use XError template instead.
	template <typename UnderlyingEnumType> struct XErrorImpl<UnderlyingEnumType, false, false, false, true> : public XErrorBase<UnderlyingEnumType>
	{
		/// Public ctor.
		/// @param value object value
		XErrorImpl(UnderlyingEnumType value) : XErrorBase(value) { }

		/// Success translator.
		/// @return true if current state of the object indicates success.
		bool IsSuccess() const { return (long long int)(m_Value) == 0; }

		/// Failure translator.
		/// @return true if current state of the object indicates failure.
		bool IsFailure() const { return (long long int)(m_Value) != 0; }
	};

	/// EnumWithHresult structure. A handy wrapper for functions performing many API calls, to indicate not only which error (HRESULT), but where (provided enum) it was obtained.
	template<typename Enum> struct EnumWithHresult
	{
		/// Template params storage.
		typedef Enum Enum;

		/// Getter for underlying enum.
		/// @return underlying enum.
		Enum GetApiCall() const { return m_ApiCall; }

		/// Getter for hresult.
		/// @return hresult value.
		HRESULT GetHresult() const { return m_ErrorCode; }

		/// Success translator.
		/// @return true if current state of the object indicates success.
		bool IsSuccess() const { return SUCCEEDED(m_ErrorCode); }

		/// Failure translator.
		/// @return true if current state of the object indicates failure.
		bool IsFailure() const { return FAILED(m_ErrorCode); }

		// Public members provided to allow simple initialization of EnumWithHresult with aggregation operator { }.
		Enum m_ApiCall;															///< Underlying enum value.
		HRESULT m_ErrorCode;													///< HRESULT value.
	};

	/// SystemError structure. A handy wrapper that can be used in code sections which disallows converting system error code to HRESULT.
	struct SystemErrorCode
	{
		/// Public ctor.
		SystemErrorCode(DWORD code) : m_ErrorCode(code) { }

		/// Success translator.
		/// @return true if current state of the object indicates success.
		bool IsSuccess() const { return !m_ErrorCode; }

		/// Failure translator.
		/// @return true if current state of the object indicates failure.
		bool IsFailure() const { return m_ErrorCode != 0; }

		/// Const DWORD conversion operator.
		operator DWORD () const { return m_ErrorCode; }

		/// DWORD conversion operator.
		operator DWORD () { return m_ErrorCode; }

		// Public members provided to allow simple initialization of this structure with aggregation operator { }.
		DWORD m_ErrorCode;														///< System error code value.
	};

#	pragma endregion Internal structures
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Finally, the XError alias templates.
	template<typename T> using XError = XErrorImpl<T, XErrorContentDeducer<T>::hasIsSuccess, XErrorContentDeducer<T>::hasIsFailure, std::is_pointer<T>::value, std::is_enum<T>::value>; ///< Main deducer.
	template<typename T> using XErrorRaw = XErrorImpl<T, false, false, false, false>;				///< XError devoid of implicit IsSuccess and IsFailure implementations.
	template<typename T> using XErrorEnumWithHresult = XError<EnumWithHresult<T>>;					///< A handy alias for XError specialization with EnumWithHresult.

	// Creator templates.
	template<typename T> XError<T> XErrorCreator(const T& value) { return value; }										///< A universal Creator.
	template<typename T> XErrorRaw<T> XErrorRawCreator(const T& value) { return value; }								///< Creator of raw XErrors.
	template<typename TEnum> XErrorEnumWithHresult<TEnum> XErrorEnumWithHresultCreator(TEnum enumValue, HRESULT hr)		///< Creator for EnumWithHresult template.
		{ static_assert(std::is_enum<TEnum>::value, "XErrorEwhCreator template requires enumeration type."); return CppTools::EnumWithHresult<TEnum>{ enumValue, hr }; }

	// Macros. @warning There is no XError<decltype(GetLastError())>. Value got by calling GetLastError() should be immediately converted to HRESULT, e.g. by using those macros:
#	define XERROR_GETLASTERROR CppTools::XErrorCreator(WinTools::SystemErrorToHresult(GetLastError()))
#	define XERRORRAW_GETLASTERROR CppTools::XErrorRawCreator(WinTools::SystemErrorToHresult(GetLastError()))
}
