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

#include "stdafx.h"
#include "ItemSendKey.h"

namespace CEGUI {

	ItemSendKey::ItemSendKey(WORD reference, KeysBuffer *buffer, int ascii, int keycode, int flags) :
	PanelItem(reference) {
		_key.setAscii(ascii);
		_key.setKeycode(keycode);
		_key.setFlags(flags);
		_buffer = buffer->Instance();
	}
 
	
	ItemSendKey::~ItemSendKey() {   
	}

	bool ItemSendKey::action(int x, int y, bool pushed) {

		if (checkInside(x, y) && _visible) {
			_key.setPushed(pushed);
			_buffer->add(&_key);
			return true;
		}
		else
			return false;
	}
}