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
 * $URL$
 * $Id$
 */

#ifndef REMAP_DIALOG_H
#define REMAP_DIALOG_H

#include "backends/keymapper/keymapper.h"
#include "gui/dialog.h"

namespace GUI {
	class PopupWidget;
	class ScrollBarWidget;
}

namespace Common {

class RemapDialog : public GUI::Dialog {
public:
	RemapDialog();
	virtual ~RemapDialog();
	virtual void open();
	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
	virtual void handleKeyDown(Common::KeyState state);

protected:
	struct Mapping {
		Action *action;
		GUI::StaticTextWidget *actionText;
		GUI::ButtonWidget *keyButton;
	};

	void loadKeymap();
	void refreshKeymap();
	void setupWidgets(uint num);
	void startRemapping(Mapping *remap);
	void stopRemapping();

	Keymapper *_keymapper;
	Stack<Keymapper::MapRecord> *_activeKeymaps;
	KeymapManager::Domain *_globalKeymaps;
	KeymapManager::Domain *_gameKeymaps;

	List<Action*> *_currentActions;
	List<Action*>::iterator _topAction;
	uint _topRow;

	Rect _keymapArea;

	GUI::PopUpWidget *_kmPopUp;
	Keymap** _keymapTable;

	GUI::ScrollBarWidget *_scrollBar;

	uint _colWidth;
	uint _colCount, _rowCount;
	uint _spacing;
	uint _buttonHeight;

	Mapping *_activeRemap;
	Array<Mapping> _keymapMappings;

};

} // end of namespace Common

#endif
