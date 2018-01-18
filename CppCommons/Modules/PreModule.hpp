// Modules tool set. It should be included by every Module on the first line of its precompiled header pack.
#pragma once
#define PREMODULE_INCLUDED

// Function argument modifiers. Allows you to explicitly describe the purpose of using argument address. None of the following modifiers have any effect (and sense) on arguments provided by value.
#define IN																		//< Parameter that is going to be modified in a function's body and has no sense on function exit.
																				//  Example: Sum(IN vector<int>& vals) - after calling that function, vals might be empty, because in terms of
																				//  performance, Sum haven't been working on a temporary copy of that vector. The rule of thumb here is to use
																				//  r-value references - it is the only way to be able to call a function with constants (e.g. fun(2)) while avoiding
																				//  unnecessary copying (void fun(IN Object s)). Additional advantage here is that the compiler will force caller to
																				//  use std::move, so he warns/reminds about an IN param. That's why use: void fun(IN Object&& s);
#define OUT																		//< Value is omitted when function is called, but it'll be set and will gain sense when the function returns. Usually
																				//  OUT parameter is a l-value reference.
#define INOUT																	//< A mixture of IN and OUT. Value is used as an input, then output will be set there in return. Usually
																				//  INOUT parameter is a l-value reference.

#define NO																(0)
#define YES																(1)

// Order-independent binary comparators. Useful for further optimizations processing.
#define OR ||
#define AND &&
// Logically order-independent binary comparators, dependent because of applied optimizations.
#define OPT_OR ||
#define OPT_AND &&

// Modules include mode.
#define USE_MODULE_DETACHED												(0)		//< To force excluding a Module. Can be used f.e. to hunt dependencies problems.
#define USE_MODULE_LIBRARY												(1)		//< A static library.
#define USE_MODULE_FULL_SOURCES											(3)		//< Module's Precompiled Header Pack included in a precompiled header.
#define USE_MODULE_AS_PRECOMPILED_SOURCES								(2)		//< Whole Module's source code included in a precompiled header.

// Destination platforms.
#if 1
#	define PLATFORM_WINDOWS												YES
#	define WIN32_LEAN_AND_MEAN													//< Always exclude rarely-used stuff from Windows headers.
#	define NOMINMAX
#	include <Windows.h>
#endif

// Compilation platform.
#if !defined(COMPILER_MSVC) && defined(_MSC_VER)
#	define COMPILER_MSVC												YES
#	if _MSC_VER >= 1600
#		define COMPILER_MSVC_VERSION_ABOVE_2010							YES
#	else
#		define COMPILER_MSVC_VERSION_ABOVE_2010							NO
#	endif
#endif

// Target architecture.
#if !defined(TARGET_ARCHITECTURE_TYPE)
#	define TARGET_ARCHITECTURE_TYPE_X86									(0)
#	define TARGET_ARCHITECTURE_TYPE_X86_64								(1)
#	if defined _WIN64
#		define TARGET_ARCHITECTURE_TYPE			TARGET_ARCHITECTURE_TYPE_X86_64
#	else
#		define TARGET_ARCHITECTURE_TYPE			TARGET_ARCHITECTURE_TYPE_X86
#	endif
#endif

// Target binary type
#if !defined(TARGET_BINARY_IMAGE_TYPE)
#	define TARGET_BINARY_IMAGE_TYPE_EXE									(0)
#	define TARGET_BINARY_IMAGE_TYPE_DLL									(1)
#	define TARGET_BINARY_IMAGE_TYPE_LIB									(2)
#	if defined _WINDLL
#		define TARGET_BINARY_IMAGE_TYPE			TARGET_BINARY_IMAGE_TYPE_DLL
#	else
#		define TARGET_BINARY_IMAGE_TYPE			TARGET_BINARY_IMAGE_TYPE_EXE
#	endif
#endif

// Do not use _DEBUG symbol directly. Use CONFIGURATION_DEBUG instead, and don't change it (treat it as read only). The same goes for UnitTesting.
#if defined(_DEBUG)
#	define CONFIGURATION_DEBUG											YES
#else
#	define CONFIGURATION_DEBUG											NO
#endif
#if defined(_UNIT_TESTING)
#	define CONFIGURATION_UNIT_TEST										YES
#else
#	define CONFIGURATION_UNIT_TEST										NO
#endif

// Macros.
#define UNUSED_PARAM(x) x;
#define WIDEN(x) L ## #x

#define STRINGITIZE(x) STRINGITIZE_EXPANDED(x)
#	define STRINGITIZE_EXPANDED(x) #x
#define WSTRINGITIZE(x) WSTRINGITIZE_EXPANDED(x)
#	define WSTRINGITIZE_EXPANDED(x) L ## #x
#if defined UNICODE
#	define TSTRINGITIZE(x) WSTRINGITIZE_EXPANDED(x)
#else
#	define TSTRINGITIZE(x) STRINGITIZE_EXPANDED(x)
#endif

#define CONCAT(a,b) CONCAT_EXPANDED(a,b)
#	define CONCAT_EXPANDED(a,b) a ## b

#define STRINGITIZE_FILE_AND_LINE __FILE__ "(" STRINGITIZE(__LINE__) "): "
#define UNIQUE_ID CONCAT(_cuid_, __COUNTER__)

// Compilation messages.
#define COMPOSE_FILE_LINE_AND_TEXT(text) STRINGITIZE_FILE_AND_LINE text								//< Usage: #pragma message(COMPOSE_FILE_LINE_AND_TEXT("Some comment"))
#define COMPILATION_MESSAGE(text) __pragma(message(COMPOSE_FILE_LINE_AND_TEXT(text)))				//< Usage: COMPILATION_MESSAGE("Some comment")

// Standard library includes.
#include <string>
#include <iostream>
#include <tchar.h>

// T-strings definitions.
namespace std
{
#ifdef _UNICODE
	typedef wstring tstring;
	typedef wstringstream tstringstream;
	inline wstring to_tstring(int value) { return (to_wstring(value)); }
#else
	typedef string tstring;
	typedef stringstream tstringstream;
	inline string to_tstring(int value) { return (to_string(value)); }
#endif
};
