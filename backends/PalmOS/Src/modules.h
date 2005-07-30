#ifndef MODULES_H
#define MODULES_H

#include "globals.h"

#define sysAppLaunchCode_SVMRunEngine	(sysAppLaunchCmdCustomBase + 0)

void ModDelete();

typedef struct {
	UInt8 lightspeed;
	Boolean exitLauncher;

	struct {
		UInt8 argc;
		Char **argv;
	} args;

	GlobalsDataPtr gVars;
} LaunchParamType;

#endif
