// Module's precompiled header pack.
#pragma once
#include "PreModule.hpp"
#define ENVIROMODULE_HH_INCLUDED

#if !defined(INSIDE_PRECOMPILED_HEADER)
#	pragma message (COMPOSE_FILE_LINE_AND_TEXT("Warning: EnviroTools Module's precompiled header pack is included outside the precompiled header. It will have a negative impact on build performance."))
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard library includes.
#include <assert.h>																//< Standard assertions.
#include <vector>																//< Vectors.
#include <fcntl.h>																//< For _O_U16TEXT.
#include <io.h>																	//< For _setmode().
