/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ICB_PSX_ANIMS_HH
#define ICB_PSX_ANIMS_HH

#include "engines/icb/debug.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_anims.h"

namespace ICB {

inline PXframe_PSX *PXFrameEnOfAnim(uint32 n, PXanim_PSX *pAnim) {
	// Convert to the new schema
	ConvertPXanim(pAnim);
	if (n >= pAnim->frame_qty) {
		Real_Fatal_error("Illegal frame %d %d %s %d", n, pAnim->frame_qty);
		error("Should exit with error-code -1");
		return NULL;
	}
	return (PXframe_PSX *)((uint8 *)pAnim + FROM_LE_16(pAnim->offsets[n]));
}

} // End of namespace ICB

#endif // #ifndef PSX_ANIMS_HH
