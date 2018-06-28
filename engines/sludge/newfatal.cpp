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
#include "sludge/errors.h"
#include "sludge/fileset.h"
#include "sludge/newfatal.h"
#include "sludge/sludge.h"
#include "sludge/sound.h"
#include "sludge/version.h"

namespace Common {
DECLARE_SINGLETON(Sludge::FatalMsgManager);
}

namespace Sludge {

int inFatal(const Common::String &str) {
	g_sludge->_soundMan->killSoundStuff();
	error("%s", str.c_str());
	return true;
}

FatalMsgManager::FatalMsgManager() {
	reset();
}

FatalMsgManager::~FatalMsgManager() {
}

void FatalMsgManager::reset() {
	_fatalMessage = "";
	_fatalInfo = "Initialisation error! Something went wrong before we even got started!";
	_resourceForFatal = -1;
}

bool FatalMsgManager::hasFatal() {
	if (!_fatalMessage.empty())
		return true;
	return false;
}

void FatalMsgManager::setFatalInfo(const Common::String &userFunc, const Common::String &BIF) {
	_fatalInfo = "Currently in this sub: " + userFunc + "\nCalling: " + BIF;
	debugC(0, kSludgeDebugFatal, "%s", _fatalInfo.c_str());
}

void FatalMsgManager::setResourceForFatal(int n) {
	_resourceForFatal = n;
}

int FatalMsgManager::fatal(const Common::String &str1) {
	ResourceManager *resMan = g_sludge->_resMan;
	if (resMan->hasResourceNames() && _resourceForFatal != -1) {
		Common::String r = resMan->resourceNameFromNum(_resourceForFatal);
		Common::String newStr = _fatalInfo + "\nResource: " + r + "\n\n" + str1;
		inFatal(newStr);
	} else {
		Common::String newStr = _fatalInfo + "\n\n" + str1;
		inFatal(newStr);
	}
	return 0;
}

int checkNew(const void *mem) {
	if (mem == NULL) {
		inFatal(ERROR_OUT_OF_MEMORY);
		return 0;
	}
	return 1;
}

int fatal(const Common::String &str1, const Common::String &str2) {
	Common::String newStr = str1 + " " + str2;
	fatal(newStr);
	return 0;
}

} // End of namespace Sludge
