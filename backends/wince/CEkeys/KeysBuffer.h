/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef CEKEYS_KEYSBUFFER
#define CEKEYS_KEYSBUFFER

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/list.h"

#include "Key.h"

namespace CEKEYS {

	class KeysBuffer {
	public:
		static KeysBuffer* Instance(int capacity = 5);
		bool add(const Key *key);
		bool simulate(const Key *key);
		Key* get();
		~KeysBuffer();
	private:
		KeysBuffer(int capacity = 5);
		static KeysBuffer* _instance;
		Key *_buffer;
		int _capacity;
		int _currentAdd;
		int _currentGet;
	};	
}

#endif