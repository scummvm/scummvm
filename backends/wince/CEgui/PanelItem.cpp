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

#include "common/stdafx.h"
#include "PanelItem.h"

namespace CEGUI {

	PanelItem::PanelItem(WORD reference) : GUIElement() {
		setBackground(reference);
		_panel = NULL;
	}


	PanelItem::~PanelItem() {
	}

	bool PanelItem::action(int x, int y, bool pushed) {
		return false;
	}

	void PanelItem::setPanel(Panel *panel) {
		_panel = panel;
	}
}

