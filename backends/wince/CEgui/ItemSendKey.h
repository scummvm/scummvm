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

#ifndef CEGUI_ITEMSENDKEY
#define CEGUI_ITEMSENDKEY

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"

#include "PanelItem.h"
#include "KeysBuffer.h"

using CEKEYS::KeysBuffer;
using CEKEYS::Key;

namespace CEGUI {

	class ItemSendKey : public PanelItem {
	public:
		ItemSendKey(WORD reference, KeysBuffer *buffer, int ascii, int keycode = 0, int flags = 0);
		virtual ~ItemSendKey();
		virtual bool action(int x, int y, bool pushed); 
	private:
		KeysBuffer *_buffer;
		Key _key;
	};
}

#endif