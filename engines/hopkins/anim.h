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
 */

#ifndef HOPKINS_ANIM_H
#define HOPKINS_ANIM_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/str.h"
#include "graphics/surface.h"

namespace Hopkins {

class HopkinsEngine;

class AnimationManager {
public:
	HopkinsEngine *_vm;
	bool CLS_ANM;
	bool NO_SEQ;
public:
	AnimationManager();
	void setParent(HopkinsEngine *vm) { _vm = vm; }

	void PLAY_ANM(const Common::String &filename, uint32 rate, uint32 rate2, uint32 rate3);
	bool REDRAW_ANIM();
};

} // End of namespace Hopkins

#endif /* HOPKINS_ANIM_H */
