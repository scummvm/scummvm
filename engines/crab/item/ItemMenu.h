/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_ITEMMENU_H
#define CRAB_ITEMMENU_H

#include "crab/ui/ItemDesc.h"
#include "crab/item/ItemSlot.h"
#include "crab/stat/StatDrawHelper.h"
#include "crab/item/StatPreview.h"
#include "crab/ui/menu.h"
#include "crab/people/person.h"

namespace Crab {

namespace pyrodactyl {
namespace item {
// The menu is used to handle interactions like the player clicking on an item to equip it
class ItemMenu : public pyrodactyl::ui::Menu<ItemSlot> {
protected:
	int _previewIndex, _selectIndex;

	// Variable to determine if stats of object need updating
	bool _update;

public:
	// The preview for selected item and hovered item
	// StatPreview select, hover;

	ItemMenu() {
		_previewIndex = -1;
		_selectIndex = -1;
		_update = false;
	}
	~ItemMenu() {}

	int hoverIndex() { return _hoverIndex; }

	void draw(pyrodactyl::ui::ItemDesc &itemInfo);
	/*pyrodactyl::people::Person &obj, pyrodactyl::stat::StatDrawHelper &helper*/

	void init(const ItemSlot &ref, const Vector2i &inc, const uint &rows,
			  const uint &cols, const bool &keyboard);

	void handleEvents(const Common::Event &event, const int &xOffset = 0, const int &yOffset = 0);
#if 0
	void handleEvents(const SDL_Event &Event, const int &XOffset = 0, const int &YOffset = 0);
#endif
	/* pyrodactyl::people::Person &obj,*/

	bool swap(ItemMenu &target, int index);
	bool equip(Item &item);
	bool del(const Common::String &id);
	bool has(const Common::String &container, const Common::String &id);

	// Used to calculate enabled slots
	// void SetEnable();

	void saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void loadState(rapidxml::xml_node<char> *node);
};
} // End of namespace item
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_ITEMMENU_H
