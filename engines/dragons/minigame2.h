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
#ifndef DRAGONS_MINIGAME2_H
#define DRAGONS_MINIGAME2_H

#include "common/system.h"

namespace Dragons {

class DragonsEngine;

class Minigame2 {
private:
	DragonsEngine *_vm;
	uint16 _dat_80093cb4;
	uint16 _dat_80093cbc;
	uint16 _dat_80093cb8;
	uint16 _dat_80093cc0;

	uint16 _dat_80093ca4;
	uint16 _dat_80093c90;
	uint16 _dat_80093c94;

	uint16 _dat_80093cac;
	bool _dat_80093cb0;
	uint16 _dat_80093c9c;

	uint16 _dat_80093c98;
	uint16 _dat_80093ca0;
	uint16 _dat_80093cc4;
	uint16 _dat_80093cc8;

	bool _dat_80093c70;
	bool _dat_80093c72;
	uint16 _dat_80093c74;
	bool _dat_80093ca8;
public:
	Minigame2(DragonsEngine *vm);

	void run(int16 param_1, uint16 param_2, int16 param_3);
private:
	void fun_80093aec_dialog(uint32 textId, int16 x, int16 y);
	bool fun_80093520();
	bool fun_80093a30();
	bool fun_80093248();
	bool fun_80093800();
	bool fun_80093990();
};

} // End of namespace Dragons

#endif //DRAGONS_MINIGAME2_H
