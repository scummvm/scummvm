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

#ifndef ACTIONMENU_H_
#define ACTIONMENU_H_

#include "engines/stark/gfx/renderentry.h"

namespace Stark {

class VisualImageXMG;

namespace Resources {
class Item;
}

class ActionMenu {
	Resources::Item *_eye;
	Resources::Item *_hand;
	Resources::Item *_mouth;
	VisualImageXMG *_background;
	Gfx::RenderEntry *_eyeEntry;
	Gfx::RenderEntry *_handEntry;
	Gfx::RenderEntry *_mouthEntry;
	Gfx::RenderEntryArray _renderEntries;
	Gfx::Driver *_gfx;
public:
	enum ActionMenuType {
		kActionHand,
		kActionMouth,
		kActionEye
	};
	ActionMenu(Gfx::Driver *gfx);
	~ActionMenu();
	void render(Common::Point pos);
	void clearActions();
	void enableAction(ActionMenuType action);
};


} // End of namespace Stark

 #endif