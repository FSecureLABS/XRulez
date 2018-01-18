// Module's precompiled header pack.
#pragma once
#include "PreModule.hpp"
#define CPPTOOLSMODULE_HH_INCLUDED

#if !defined(INSIDE_PRECOMPILED_HEADER)
#	pragma message (COMPOSE_FILE_LINE_AND_TEXT("Warning: CppTools Module's precompiled header pack is included outside the precompiled header. It will have a negative impact on build performance."))
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard library includes.
#include <Functional>															//< Lambda expression manipulations.
#include <vector>																//< Required by ByteStream.
#include <locale>																//< Conversions mbcs <-> UNICODE.
#include <codecvt>																//< For std::codecvt_utf8_utf16.
#include <iomanip>																//< I/O streams manipulators.
#include <sstream>																//< For easy strings manipulation.
#include <cstdint>																//< int32_t and others.
#include <ctime>																//< Time manipulations.
#include <memory>																//< For std::unique_ptr.
#include <array>																//< You know what.
#include <fstream>																//< Files.
#include <experimental/filesystem>												//< Path manipulations.
namespace std { namespace filesystem = std::experimental::filesystem; }			//< Alias to remove "experimental" part from filesystem namespace.
