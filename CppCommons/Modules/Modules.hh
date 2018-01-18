// Modules precompiled header.
#pragma once

#define INSIDE_PRECOMPILED_HEADER

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#	ifdef USE_MODULE_ENVIRO
#		if USE_MODULE_ENVIRO == USE_MODULE_AS_PRECOMPILED_SOURCES
#			include "Enviro/EnviroModule.h"
#		elif USE_MODULE_ENVIRO == USE_MODULE_FULL_SOURCES
#			include "Enviro/EnviroModule.hh"
#		elif USE_MODULE_ENVIRO == USE_MODULE_LIBRARY
#			include "Enviro/EnviroModule.h"
#			pragma comment(lib, "../../Lib/Enviro.Lib")
#		endif
#	endif

#	ifdef USE_MODULE_CPPTOOLS
#		if USE_MODULE_CPPTOOLS == USE_MODULE_AS_PRECOMPILED_SOURCES
#			include "CppTools/CppToolsModule.h"
#		elif USE_MODULE_CPPTOOLS == USE_MODULE_FULL_SOURCES
#			include "CppTools/CppToolsModule.hh"
#		elif USE_MODULE_CPPTOOLS == USE_MODULE_LIBRARY
#			include "CppTools/CppToolsModule.h"
#			pragma comment(lib, "../../Lib/Enviro.Lib")
#		endif
#	endif

#	ifdef USE_MODULE_WINTOOLS
#		if USE_MODULE_WINTOOLS == USE_MODULE_AS_PRECOMPILED_SOURCES
#			include "WinTools/WinToolsModule.h"
#		elif USE_MODULE_WINTOOLS == USE_MODULE_FULL_SOURCES
#			include "WinTools/WinToolsModule.hh"
#		elif USE_MODULE_WINTOOLS == USE_MODULE_LIBRARY
#			include "WinTools/WinToolsModule.h"
#			pragma comment(lib, "../../Lib/WinTools.Lib")
#		endif
#	endif

#	ifdef USE_MODULE_WINSOCKETS
#		if USE_MODULE_WINSOCKETS == USE_MODULE_AS_PRECOMPILED_SOURCES
#			include "WinSockets/WinSocketsModule.h"
#		elif USE_MODULE_WINSOCKETS == USE_MODULE_FULL_SOURCES
#			include "WinSockets/WinSocketsModule.hh"
#		elif USE_MODULE_WINSOCKETS == USE_MODULE_LIBRARY
#			include "WinSockets/WinSocketsModule.h"
#			pragma comment(lib, "../../Lib/WinSockets.Lib")
#		endif
#	endif

#	ifdef USE_MODULE_MAPITOOLS
#		if USE_MODULE_MAPITOOLS == USE_MODULE_AS_PRECOMPILED_SOURCES
#			include "MapiTools/MapiToolsModule.h"
#		elif USE_MODULE_MAPITOOLS == USE_MODULE_FULL_SOURCES
#			include "MapiTools/MapiToolsModule.hh"
#		elif USE_MODULE_MAPITOOLS == USE_MODULE_LIBRARY
#			include "MapiTools/MapiToolsModule.h"
#			pragma comment(lib, "../../Lib/MapiTools.Lib")
#		endif
#	endif

#	ifdef USE_MODULE_ZIPTOOLS
#		if USE_MODULE_ZIPTOOLS == USE_MODULE_AS_PRECOMPILED_SOURCES
#			include "ZipTools/ZipToolsModule.h"
#		elif USE_MODULE_ZIPTOOLS == USE_MODULE_FULL_SOURCES
#			include "ZipTools/ZipToolsModule.hh"
#		elif USE_MODULE_ZIPTOOLS == USE_MODULE_LIBRARY
#			include "ZipTools/ZipToolsModule.h"
#			pragma comment(lib, "../../Lib/ZipTools.Lib")
#		endif
#	endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef INSIDE_PRECOMPILED_HEADER
