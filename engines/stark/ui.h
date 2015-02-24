/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef STARK_UI_H
#define STARK_UI_H

#include "common/scummsys.h"
#include "common/str-array.h"

#include "engines/stark/gfx/renderentry.h"

namespace Stark {

namespace Resources {
class Object;
}

namespace Gfx {
class Driver;
}

class DialogInterface;
class TopMenu;
class Cursor;

class UI {
	DialogInterface *_dialogInterface;
	TopMenu *_topMenu;
	Resources::Object *_currentObject;
	Resources::Object *_objectUnderCursor;
	Gfx::Driver *_gfx;
	Cursor *_cursor;
	bool _hasClicked;
	bool _exitGame;
	void handleClick();
public:
	UI(Gfx::Driver *gfx, Cursor *cursor);
	virtual ~UI();
	void init();
	void update(Gfx::RenderEntryArray renderEntries, bool keepExisting = false);
	void render();
	void notifyClick();
	void notifySubtitle(const Common::String &subtitle);
	void notifyDialogOptions(const Common::StringArray &options);
	void notifyShouldExit() { _exitGame = true; }
	bool shouldExit() { return _exitGame; }
};

} // End of namespace Stark

#endif // STARK_UI_H
