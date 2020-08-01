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


#include "common/endian.h"
#include "common/memstream.h"
#include "common/textconsole.h"

#include "cine/cine.h"
#include "cine/various.h"
#include "cine/bg.h"

namespace Cine {

uint16 bgVar0;
byte *additionalBgTable[9];

int16 loadCtFW(const char *ctName) {
	debugC(1, kCineDebugCollision, "loadCtFW(\"%s\")", ctName);
	byte *ptr, *dataPtr;

	int16 foundFileIdx = findFileInBundle(ctName);
	if (foundFileIdx < 0) {
		warning("loadCtFW: Unable to find collision data file '%s'", ctName);
		return -1;
	}

	if (currentCtName != ctName)
		Common::strlcpy(currentCtName, ctName, sizeof(currentCtName));

	ptr = dataPtr = readBundleFile(foundFileIdx);

	loadRelatedPalette(ctName);

	assert(strstr(ctName, ".NEO"));

	gfxConvertSpriteToRaw(collisionPage, ptr + 0x80, 160, 200);

	free(dataPtr);
	return 0;
}

int16 loadCtOS(const char *ctName) {
	debugC(1, kCineDebugCollision, "loadCtOS(\"%s\")", ctName);
	byte *ptr, *dataPtr;

	int16 foundFileIdx = findFileInBundle(ctName);
	if (foundFileIdx < 0) {
		warning("loadCtOS: Unable to find collision data file '%s'", ctName);
		return -1;
	}

	if (currentCtName != ctName)
		Common::strlcpy(currentCtName, ctName, sizeof(currentCtName));

	ptr = dataPtr = readBundleFile(foundFileIdx);

	uint16 bpp = READ_BE_UINT16(ptr);
	ptr += 2;

	if (bpp == 8) {
		renderer->loadCt256(ptr, ctName);
	} else {
		renderer->loadCt16(ptr, ctName);
	}

	free(dataPtr);
	return 0;
}

int16 loadBg(const char *bgName) {
	byte *ptr, *dataPtr;

	int16 fileIdx = findFileInBundle(bgName);
	if (fileIdx < 0) {
		warning("loadBg(\"%s\"): Could not find background in file bundle.", bgName);
		return -1;
	}
	checkDataDisk(-1);
	ptr = dataPtr = readBundleFile(fileIdx);

	uint16 bpp = READ_BE_UINT16(ptr);
	ptr += 2;

	if (bpp == 8) {
		renderer->loadBg256(ptr, bgName);
	} else {
		if (g_cine->getGameType() == Cine::GType_FW) {
			loadRelatedPalette(bgName);
		}

		renderer->loadBg16(ptr, bgName);
	}
	free(dataPtr);

	return 0;
}

} // End of namespace Cine
