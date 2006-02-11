/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#ifndef CEGUI_ITEMSWITCH
#define CEGUI_ITEMSWITCH

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"

#include "Panel.h"
#include "EventsBuffer.h"

using GUI::Key;

namespace CEGUI {

	class ItemSwitch : public PanelItem {
	public:
		ItemSwitch(WORD referenceTrue, WORD referenceFalse, bool *item);
		virtual ~ItemSwitch();
		virtual bool action(int x, int y, bool pushed);
	private:
		bool *_item;
		SDL_ImageResource *_backgroundTrue;
		SDL_ImageResource *_backgroundFalse;
	};
}

#endif
