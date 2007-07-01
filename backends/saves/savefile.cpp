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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/util.h"
#include "common/config-manager.h"
#include "common/savefile.h"

#include <stdio.h>
#include <string.h>

namespace Common {

const char *SaveFileManager::getSavePath() const {

#if defined(PALMOS_MODE) || defined(__PSP__)
	return SCUMMVM_SAVEPATH;
#else

	const char *dir = NULL;

	// Try to use game specific savepath from config
	dir = ConfMan.get("savepath").c_str();

	// Work around a bug (#999122) in the original 0.6.1 release of
	// ScummVM, which would insert a bad savepath value into config files.
	if (0 == strcmp(dir, "None")) {
		ConfMan.removeKey("savepath", ConfMan.getActiveDomainName());
		ConfMan.flushToDisk();
		dir = ConfMan.get("savepath").c_str();
	}

#ifdef _WIN32_WCE
	if (dir[0] == 0)
		dir = ConfMan.get("path").c_str();
#endif

	assert(dir);

	return dir;
#endif
}

} // End of namespace Common
