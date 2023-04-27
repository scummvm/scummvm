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

#ifndef TETRAEDGE_GAME_INVENTORY_H
#define TETRAEDGE_GAME_INVENTORY_H

#include "common/str.h"
#include "tetraedge/game/inventory_object.h"
#include "tetraedge/te/te_layout.h"
#include "tetraedge/te/te_lua_gui.h"

namespace Tetraedge {

class Cellphone;

class Inventory : public TeLayout {
public:
	struct InventoryObjectData {
		Common::String _id;
		Common::String _name;
		bool _isDocument;
	};

	Inventory();
	virtual ~Inventory();

	void enter();
	void leave();
	void load();
	void unload();
	void loadCellphone();

	//void loadFromBackup(TiXmlNode *node);
	//void saveToBackup(TiXmlNode *node);

	void addObject(const Common::String &objname);
	bool addObject(InventoryObject *obj);
	bool isDocument(const Common::String &objname);

	int objectCount(const Common::String &objname);
	Common::String objectDescription(const Common::String &objname);
	Common::String objectName(const Common::String &objname);

	void pauseAnims();
	void unPauseAnims();

	void removeObject(const Common::String &objname);
	void removeSelectedObject();

	InventoryObject *selectedInventoryObject();
	void selectedObject(const Common::String &objname);
	void selectedObject(InventoryObject *obj);
	const Common::String &selectedObject();

	bool updateLayout();

	Common::Error syncState(Common::Serializer &s);
	Common::Error syncStateWithCount(Common::Serializer &s, uint nitems);

	Cellphone *cellphone() { return _cellphone; }

private:
	// Amerzone navigation events
	void currentPage(uint page);
	bool onPreviousPage();
	bool onNextPage();

	// Syberia navigation events
	bool onMainMenuButton();
	bool onObjectSelected(InventoryObject &obj);
	bool onQuitButton();
	bool onTakeObjectSelected();
	bool onVisibleCellphone();
	bool onZoomed();

	void loadXMLFile(const Common::Path &path);

	TeLuaGUI _gui;
	Common::List<InventoryObject *> _invObjects;
	Cellphone *_cellphone;
	InventoryObject *_selectedObject;
	Common::HashMap<Common::String, InventoryObjectData> _objectData;

	// This is used when we need a reference to a blank str in selectedObject()
	const Common::String _blankStr;
	
	uint _currentPage;

	TeTimer _selectedTimer;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_INVENTORY_H
