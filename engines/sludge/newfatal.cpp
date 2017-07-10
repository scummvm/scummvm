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

#include "common/debug.h"

#include "sludge/allfiles.h"
#include "sludge/sound.h"
#include "sludge/stringy.h"
#include "sludge/errors.h"
#include "sludge/graphics.h"
#include "sludge/sludge.h"
#include "sludge/CommonCode/version.h"

namespace Sludge {

const char emergencyMemoryMessage[] = "Out of memory displaying error message!";

static Common::String fatalMessage;
static Common::String fatalInfo = "Initialisation error! Something went wrong before we even got started!";

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
	if (!fatalMessage.empty())
		return true;
	return false;
}

void displayFatal() {
	if (!fatalMessage.empty()) {
#if 0
		msgBox("SLUDGE v" TEXT_VERSION " fatal error!", fatalMessage);
#endif
	}
}

void warning(const Common::String &l) {
#if 0
	setGraphicsWindow(false);
	msgBox("SLUDGE v" TEXT_VERSION " non-fatal indigestion report", l);
#endif
}

void registerWindowForFatal() {
	fatalInfo = "There's an error with this SLUDGE game! If you're designing this game, please turn on verbose error messages in the project manager and recompile. If not, please contact the author saying where and how this problem occured.";
}

int inFatal(const Common::String &str) {
	killSoundStuff();
	error(str.c_str());
	return true;
}

int checkNew(const void *mem) {
	if (mem == NULL) {
		inFatal(ERROR_OUT_OF_MEMORY);
		return 0;
	}
	return 1;
}

void setFatalInfo(const Common::String &userFunc, const Common::String &BIF) {
	fatalInfo = "Currently in this sub: " + userFunc + "\nCalling: " + BIF;
	debug(kSludgeDebugFatal, "%s", fatalInfo.c_str());
}

void setResourceForFatal(int n) {
	resourceForFatal = n;
}

int fatal(const Common::String &str1) {
	if (numResourceNames && resourceForFatal != -1) {
		Common::String r = resourceNameFromNum(resourceForFatal);
		Common::String newStr = fatalInfo + "\nResource: " + r + "\n\n" + str1;
		inFatal(newStr);
	} else {
		Common::String newStr = fatalInfo + "\n\n" + str1;
		inFatal(newStr);
	}
	return 0;
}

int fatal(const Common::String &str1, const Common::String &str2) {
	Common::String newStr = str1 + " " + str2;
	fatal(newStr);
	return 0;
}

} // End of namespace Sludge
