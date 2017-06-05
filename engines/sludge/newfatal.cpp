/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#if 0
#include <SDL/SDL.h>

#include <string.h>
#include <stdlib.h>
#endif

#include "allfiles.h"

#include "common/debug.h"
#include "platform-dependent.h"
#include "CommonCode/version.h"
#include "sound.h"
#include "stringy.h"
#include "errors.h"
#include "graphics.h"
#include "sludge.h"

namespace Sludge {

const char emergencyMemoryMessage[] = "Out of memory displaying error message!";

static char *fatalMessage = NULL;
static char *fatalInfo = joinStrings("Initialisation error! Something went wrong before we even got started!", "");

extern int numResourceNames /* = 0*/;
extern char * *allResourceNames /*= NULL*/;

int resourceForFatal = -1;

const char *resourceNameFromNum(int i) {
	if (i == -1)
		return NULL;
	if (numResourceNames == 0)
		return "RESOURCE";
	if (i < numResourceNames)
		return allResourceNames[i];
	return "Unknown resource";
}

bool hasFatal() {
	if (fatalMessage)
		return true;
	return false;
}

void displayFatal() {
	if (fatalMessage) {
#if 0
		msgBox("SLUDGE v" TEXT_VERSION " fatal error!", fatalMessage);
#endif
	}
}

void warning(const char *l) {
#if 0
	setGraphicsWindow(false);
	msgBox("SLUDGE v" TEXT_VERSION " non-fatal indigestion report", l);
#endif
}

void registerWindowForFatal() {
	delete fatalInfo;
	fatalInfo =
			joinStrings("There's an error with this SLUDGE game! If you're designing this game, please turn on verbose error messages in the project manager and recompile. If not, please contact the author saying where and how this problem occured.", "");
}

#if 0
extern SDL_Event quit_event;
#endif

int inFatal(const char *str) {
#if 0
	FILE *fatFile = fopen("fatal.txt", "wt");
	if (fatFile) {
		fprintf(fatFile, "FATAL:\n%s\n", str);
		fclose(fatFile);
	}
#endif
	fatalMessage = copyString(str);
	if (fatalMessage == NULL)
		fatalMessage = copyString("Out of memory");

	killSoundStuff();

#if defined(HAVE_GLES2)
	EGL_Close();
#endif

#if 0
	SDL_Quit();

	atexit(displayFatal);
	exit(1);
#endif
}

int checkNew(const void *mem) {
	if (mem == NULL) {
		inFatal(ERROR_OUT_OF_MEMORY);
		return 0;
	}
	return 1;
}

void setFatalInfo(const char *userFunc, const char *BIF) {
	delete fatalInfo;
	fatalInfo = new char[strlen(userFunc) + strlen(BIF) + 38];
	if (fatalInfo)
		sprintf(fatalInfo, "Currently in this sub: %s\nCalling: %s", userFunc, BIF);
	debug(kSludgeDebugFatal, "%s", fatalInfo);
}

void setResourceForFatal(int n) {
	resourceForFatal = n;
}

int fatal(const char *str1) {
	if (numResourceNames && resourceForFatal != -1) {
		const char *r = resourceNameFromNum(resourceForFatal);
		char *newStr = new char[strlen(str1) + strlen(r) + strlen(fatalInfo) + 14];
		if (checkNew(newStr)) {
			sprintf(newStr, "%s\nResource: %s\n\n%s", fatalInfo, r, str1);
			inFatal(newStr);
		} else
			fatal(emergencyMemoryMessage);
	} else {
		char *newStr = new char[strlen(str1) + strlen(fatalInfo) + 3];
		if (checkNew(newStr)) {
			sprintf(newStr, "%s\n\n%s", fatalInfo, str1);
			inFatal(newStr);
		} else
			fatal(emergencyMemoryMessage);
	}
	return 0;
}

int fatal(const char *str1, const char *str2) {
	char *newStr = new char[strlen(str1) + strlen(str2) + 2];
	if (checkNew(newStr)) {
		sprintf(newStr, "%s %s", str1, str2);
		fatal(newStr);
	} else
		fatal(emergencyMemoryMessage);
	return 0;
}

} // End of namespace Sludge
