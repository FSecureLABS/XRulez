#include "StdAfx.h"

// Codebox includes.
#include "Application.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Main()
{
	// If were in a DLL build, proceed only if we are being attached to a process.
	if (Enviro::IsDllBuild && *Enviro::AccessDllLoadReason() != DLL_PROCESS_ATTACH)
		return TRUE;

	// Create and past processing to XRulez::Application object.
	XRulez::Application app;
	app.Process();

	// We are always returning true because we don't want to propagate any error codes to the operating system/host application.
	return Enviro::IsDllBuild ? TRUE : EXIT_SUCCESS;
}
