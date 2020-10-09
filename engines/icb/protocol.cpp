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

#include "engines/icb/p4.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/debug.h"
#include "engines/icb/protocol.h"
#include "engines/icb/res_man.h"
#include "engines/icb/global_objects.h"

namespace ICB {

_animHeader *FetchAnimHeader(uint8 *animFile) {
	return (_animHeader *)(animFile + sizeof(_standardHeader));
}

_cdtEntry *FetchCdtEntry(uint8 *animFile, uint16 frameNo) {
	_animHeader *animHead;

	animHead = FetchAnimHeader(animFile);

	return (_cdtEntry *)((uint8 *)animHead + sizeof(_animHeader) + frameNo * sizeof(_cdtEntry));
}

_frameHeader *FetchFrameHeader(uint8 *animFile, uint16 frameNo) {
	// required address = (address of the start of the anim header) + frameOffset
	return (_frameHeader *)(animFile + sizeof(_standardHeader) + (FetchCdtEntry(animFile, frameNo)->frameOffset));
}

} // End of namespace ICB
