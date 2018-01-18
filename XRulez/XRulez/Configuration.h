//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define APPLICATION_NAME        TEXT("XRulez 2.0")

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MSVC related includes.
#include <SDKDDKVer.h>															//< Including SDKDDKVer.h defines the highest available Windows platform. If you wish to build your application for a
																				//  previous Windows platform, include WinSDKVer.h and set the _WIN32_WINNT macro to the platform you wish to support
																				//  before including SDKDDKVer.h.

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Used modules.
#define USE_MODULE_CPPTOOLS					USE_MODULE_AS_PRECOMPILED_SOURCES
#define USE_MODULE_ENVIRO					USE_MODULE_AS_PRECOMPILED_SOURCES
#define USE_MODULE_WINTOOLS					USE_MODULE_AS_PRECOMPILED_SOURCES
#define USE_MODULE_WINSOCKETS				USE_MODULE_DETACHED
#define USE_MODULE_MAPITOOLS				USE_MODULE_AS_PRECOMPILED_SOURCES
#define USE_MODULE_ZIPTOOLS					USE_MODULE_DETACHED

// Modules and libraries configuration.
#if defined(CONFIGURATION_DEBUG) && CONFIGURATION_DEBUG == YES
#	ifndef _HAS_ITERATOR_DEBUGGING
#		define _HAS_ITERATOR_DEBUGGING			YES								//< STL debug iterators.
#	endif
#else
#	ifndef _HAS_ITERATOR_DEBUGGING
#		define _HAS_ITERATOR_DEBUGGING			NO								//< STL debug iterators.
#	endif
#endif
