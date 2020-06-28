/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include <cstring> //for memset()

#include "bink_stub.h"

#include "common/textconsole.h"

namespace ICB {

#ifdef _MSC_VER
#define STUB_FUNC(...) warning("STUB: %s", __FUNCDNAME__)
#else
#define STUB_FUNC(...) warning("STUB: %s", __PRETTY_FUNCTION__)
#endif

HBINK BinkOpen(const char * /*name*/, uint32 /*flags*/) {
	STUB_FUNC();
	return new BINK();
}

void BinkClose(HBINK handle) {
	STUB_FUNC();

	if (handle) {
		delete handle;
	}
}

const char *BinkGetError() {
	STUB_FUNC();
	return "BinkGetError() stub";
}

int32 BinkWait(HBINK /*handle*/) {
	// STUB_FUNC();
	return 0;
}

void BinkService(HBINK /*handle*/) {
	// STUB_FUNC();
}

int32 BinkDoFrame(HBINK /*handle*/) {
	// STUB_FUNC();
	return 0;
}

void BinkNextFrame(HBINK /*handle*/) {
	// STUB_FUNC();
}

int32 BinkCopyToBuffer(HBINK /*handle*/, void *dst, int32 dstPitch, uint32 dstHeight, uint32 /*dstX*/, uint32 /*dstY*/, uint32 /*flags*/) {
	// STUB_FUNC();
	memset(dst, 0, dstPitch * dstHeight);
	return 0;
}

void BinkGoto(HBINK /*handle*/, uint32 /*frame*/, int32 /*flags*/) {
	// STUB_FUNC();
}

void BinkSoundUseDirectSound(uint32 /*flags*/) { STUB_FUNC(); }

void BinkSetVolume(HBINK /*handle*/, int /*volume*/) { STUB_FUNC(); }

} // End of namespace ICB
