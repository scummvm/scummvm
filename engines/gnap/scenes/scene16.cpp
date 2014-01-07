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

#include "gnap/gnap.h"
#include "gnap/gamesys.h"
#include "gnap/resource.h"

namespace Gnap {

void GnapEngine::scene16_initCutscene() {
	_s99_dword_47F370[0] = 0x1F2;
	_s99_dword_47F370[1] = 0x201;
	_s99_dword_47F370[2] = 0x1FC;
	_s99_dword_47F370[3] = 0x1F4;
	_s99_dword_47F370[4] = 0x1FB;
	_s99_dword_47F370[5] = 0x1F0;
	_s99_dword_47F370[6] = 0x1FD;
	_s99_dword_47F370[7] = 0x1FE;
	_s99_dword_47F370[8] = 0x1F7;
	_s99_dword_47F370[9] = 0x1F9;
	_s99_dword_47F370[10] = 0x1F8;
	_s99_dword_47F370[11] = 0x1F1;
	_s99_dword_47F370[12] = 0x202;
	_s99_dword_47F370[13] = 0x1F6;
	_s99_dword_47F370[14] = 0x1F3;
	_s99_dword_47F370[15] = 0x1FA;
	_s99_dword_47F370[16] = 0x1FF;
	_s99_dword_47F370[17] = 0x200;
	_s99_dword_47F370[18] = 0x203;
	_s99_dword_47F370[19] = 0x206;
	_s99_dword_47F370[20] = 0x207;
	_s99_dword_47F370[21] = 0x204;
	_s99_dword_47F370[22] = 0x205;
	_s99_dword_47F2F0[0] = 0x1C;
	_s99_dword_47F2F0[1] = 2;
	_s99_dword_47F2F0[2] = 0x1B;
	_s99_dword_47F2F0[3] = 0;
	_s99_dword_47F2F0[4] = 0x167;
	_s99_dword_47F2F0[5] = 1;
	_s99_dword_47F2F0[6] = 0x15B;
	_s99_dword_47F2F0[7] = 0x15A;
	_s99_dword_47F2F0[8] = 0x170;
	_s99_dword_47F2F0[9] = 0x1EB;
	_s99_dword_47F2F0[10] = 0x1EC;
	_s99_dword_47F2F0[11] = 0x1BE;
	_s99_dword_47F2F0[12] = 0x1BF;
	_s99_dword_47F330[0] = 4;
	_s99_dword_47F330[1] = 1;
	_s99_dword_47F330[2] = 1;
	_s99_dword_47F330[3] = 6;
	_s99_dword_47F330[4] = 1;
	_s99_dword_47F330[5] = 3;
	_s99_dword_47F330[6] = 1;
	_s99_dword_47F330[7] = 1;
	_s99_dword_47F330[8] = 1;
	_s99_dword_47F330[9] = 1;
	_s99_dword_47F330[10] = 1;
	_s99_dword_47F330[11] = 1;
	_s99_dword_47F330[12] = 1;
	_s99_itemsCount = 13;
}

} // End of namespace Gnap
