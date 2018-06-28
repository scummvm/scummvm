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

#ifndef PINK_MODULE_H
#define PINK_MODULE_H

#include "common/debug.h"
#include "common/hash-str.h"

#include "pink/archive.h"
#include "pink/objects/object.h"
#include "pink/objects/inventory.h"

namespace Pink {

class ModuleProxy : public NamedObject {
public:
	ModuleProxy();
	ModuleProxy(const Common::String &name);
};

class PinkEngine;
class GamePage;

class Module : public NamedObject {
public:
	Module(PinkEngine *game, const Common::String &name);
	~Module();

	void load(Archive &archive);
	void init(bool isLoadingSave, const Common::String &pageName);
	void changePage(const Common::String &pageName);

	void OnLeftButtonDown();
	void OnMouseMove();
	void OnKeyboardButtonClick();

	PinkEngine *getGame() const;
	InventoryMgr *getInventoryMgr();

	bool checkValueOfVariable(Common::String &variable, Common::String &value);
	void setVariable(Common::String &variable, Common::String &value);

private:
	GamePage *findPage(const Common::String &pageName) const;

	PinkEngine *_game;
	GamePage *_page;
	Array<GamePage*> _pages;
	InventoryMgr _invMgr;
	Common::StringMap _variables;
};


} // End of namespace Pink

#endif
