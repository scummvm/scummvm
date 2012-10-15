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
private:
	int CHARGE_BANK_SPRITE1(int idx, const Common::String &filename);
public:
	HopkinsEngine *_vm;
	bool CLS_ANM;
	bool NO_SEQ;
	bool NO_COUL;
public:
	AnimationManager();
	void setParent(HopkinsEngine *vm) { _vm = vm; }

	void PLAY_ANM(const Common::String &filename, uint32 rate, uint32 rate2, uint32 rate3);
	void PLAY_ANM2(const Common::String &filename, uint32 a2, uint32 a3, uint32 a4);
	bool REDRAW_ANIM();
	void CHARGE_ANIM(const Common::String &animName);
	void CLEAR_ANIM();
	void RECHERCHE_ANIM(const byte *data, int animIndex, int count);
	void PLAY_SEQ(int a1, const Common::String &a2, uint32 a3, uint32 a4, uint32 a5);
	void PLAY_SEQ2(const Common::String &a1, uint32 a2, uint32 a3, uint32 a4);
};

} // End of namespace Hopkins

#endif /* HOPKINS_ANIM_H */
