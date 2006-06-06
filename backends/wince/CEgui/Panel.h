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

#ifndef CEGUI_PANEL
#define CEGUI_PANEL

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/hashmap.h"
#include "common/str.h"
#include "common/config-manager.h"

#include "PanelItem.h"
#include "Toolbar.h"

using Common::String;
using Common::HashMap;

namespace CEGUI {

	class Panel : public Toolbar {
	public:
		Panel(int interleave_first, int interleave);
		virtual bool draw(SDL_Surface *surface);
		virtual ~Panel();
		bool add(const String &name, const PanelItem *item);
		void clear();
		virtual void forceRedraw();
		virtual bool action(int x, int y, bool pushed);
	private:
		struct IgnoreCaseComparator {
          int operator()(const String& x, const String& y) const {
			  return scumm_stricmp(x.c_str(), y.c_str()); }
        };

		typedef HashMap<String, PanelItem*, Common::IgnoreCase_Hash , IgnoreCaseComparator> ItemMap;

		ItemMap _itemsMap;
		int _interleave;
		int _currentItem;
	};
}

#endif
