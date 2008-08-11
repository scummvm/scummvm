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

#include "backends/keymapper/remap-dialog.h"
#include "gui/eval.h"
#include "gui/newgui.h"
#include "gui/PopUpWidget.h"
#include "gui/ScrollBarWidget.h"

namespace Common {

enum {
	kRemapCmd = 'REMP'
};

RemapDialog::RemapDialog()
	: Dialog("remap"), _activeRemap(0), _currentActions(0), _topRow(0) {

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	_keymapper = g_system->getEventManager()->getKeymapper();
	assert(_keymapper);
	_activeKeymaps = &_keymapper->_activeMaps;

	int keymapCount = 0;
	_globalKeymaps = &_keymapper->_keymapMan->getGlobalDomain();
	if (_globalKeymaps->count() == 0)
		_globalKeymaps = 0;
	else
		keymapCount += _globalKeymaps->count();
	if (ConfMan.getActiveDomain() != 0) {
		_gameKeymaps = &_keymapper->_keymapMan->getGameDomain();
		if (_gameKeymaps->count() == 0)
			_gameKeymaps = 0;
		else
			keymapCount += _gameKeymaps->count();
	} else
		_gameKeymaps = 0;

	_keymapTable = (Keymap**)malloc(sizeof(Keymap*) * keymapCount);

	int labelWidth = g_gui.evaluator()->getVar("remap_popup_labelW");
	_kmPopUp = new GUI::PopUpWidget(this, "remap_popup", "Keymap: ", labelWidth);

	if (_activeKeymaps->size() > 0) {
		_kmPopUp->appendEntry(_activeKeymaps->top().keymap->getName() + " (Active)");
	}
	KeymapManager::Domain::iterator it;
	uint32 idx = 0;

	if (_globalKeymaps) {
		_kmPopUp->appendEntry("");
		for (it = _globalKeymaps->begin(); it != _globalKeymaps->end(); it++) {
			_kmPopUp->appendEntry(it->_value->getName() + " (Global)", idx);
			_keymapTable[idx++] = it->_value;
		}
	}
	if (_gameKeymaps) {
		_kmPopUp->appendEntry("");
		for (it = _gameKeymaps->begin(); it != _gameKeymaps->end(); it++) {
			_kmPopUp->appendEntry(it->_value->getName() + " (Game)", idx);
			_keymapTable[idx++] = it->_value;
		}
	}

	int scrollbarWidth;
	if (g_gui.getWidgetSize() == GUI::kBigWidgetSize) {
		_buttonHeight = GUI::kBigButtonHeight;
		scrollbarWidth = GUI::kBigScrollBarWidth;
	} else {
		_buttonHeight = GUI::kButtonHeight;
		scrollbarWidth = GUI::kNormalScrollBarWidth;
	}

	_colCount = g_gui.evaluator()->getVar("remap_col_count");
	_spacing = g_gui.evaluator()->getVar("remap_spacing");
	_keymapArea.left = g_gui.evaluator()->getVar("remap_keymap_area.x");
	_keymapArea.top = g_gui.evaluator()->getVar("remap_keymap_area.y");
	_keymapArea.setWidth(g_gui.evaluator()->getVar("remap_keymap_area.w"));
	_keymapArea.setHeight(g_gui.evaluator()->getVar("remap_keymap_area.h"));
	_colWidth = (_keymapArea.width() - scrollbarWidth - _colCount * _spacing) / _colCount;
	_rowCount = (_keymapArea.height() + _spacing) / (_buttonHeight + _spacing);
	_keymapArea.setHeight(_rowCount * (_buttonHeight + _spacing) - _spacing);

	_scrollBar = new GUI::ScrollBarWidget(this, 
		_keymapArea.right - scrollbarWidth, _keymapArea.top,
		scrollbarWidth,	_keymapArea.height());
	_scrollBar->_entriesPerPage = _rowCount;
	_scrollBar->_numEntries = 1;
	_scrollBar->recalc();
}

RemapDialog::~RemapDialog() {
	free(_keymapTable);
}

void RemapDialog::open() {
	Dialog::open();
	
	_kmPopUp->setSelected(0);
	loadKeymap();
}

void RemapDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd >= kRemapCmd && cmd < kRemapCmd + _keymapMappings.size()) {
		startRemapping(&_keymapMappings[cmd - kRemapCmd]);
	} else if (cmd == GUI::kPopUpItemSelectedCmd) {
		loadKeymap();
	} else if (cmd == GUI::kSetPositionCmd) {
		if (data != _topRow) refreshKeymap();
	} else {
		GUI::Dialog::handleCommand(sender, cmd, data);
	}
}

void RemapDialog::startRemapping(Mapping *remap) {
	_activeRemap = remap;
	_activeRemap->keyButton->setLabel("...");
	_keymapper->setEnabled(false);
}

void RemapDialog::stopRemapping() {
	refreshKeymap();
	_activeRemap = 0;
	_keymapper->setEnabled(true);
}

void RemapDialog::handleKeyDown(Common::KeyState state) {
	if (_activeRemap) {
		const HardwareKey *hwkey = _keymapper->getHardwareKey(state);
		if (hwkey) {
			_activeRemap->action->mapKey(hwkey);
			stopRemapping();
		}
	} else {
		GUI::Dialog::handleKeyDown(state);
	}

}

void RemapDialog::loadKeymap() {
	if (_activeKeymaps->size() > 0 && _kmPopUp->getSelected() == 0) {
		// TODO: show active keymaps (with inherited mappings)
	} else if (_kmPopUp->getSelected() != -1) {
		Keymap *km = _keymapTable[_kmPopUp->getSelectedTag()];
		_currentActions = &km->getActions();
		
		int actionCount = _currentActions->size();
		int maxActions = _colCount * _rowCount;
		if (actionCount < maxActions)
			setupWidgets(actionCount);
		else
			setupWidgets(maxActions);
		
		// refresh scroll bar
		_scrollBar->_currentPos = 0;
		_scrollBar->_numEntries = (actionCount + _colCount - 1) / _colCount;
		_scrollBar->recalc();

		_topRow = 0;
		_topAction = _currentActions->begin();
	} else {
		_currentActions = 0;
		setupWidgets(0);
	}

	refreshKeymap();

}

void RemapDialog::refreshKeymap() {
	uint newTopRow = _scrollBar->_currentPos;
	while (newTopRow < _topRow) {
		for (uint i = 0; i < _colCount; i++)
			_topAction--;
		_topRow--;
	}
	while (newTopRow > _topRow) {
		for (uint i = 0; i < _colCount; i++)
			_topAction++;
		_topRow++;
	}
	uint idx = 0;
	uint max = _keymapMappings.size();
	List<Action*>::iterator it;
	for (it = _topAction; it != _currentActions->end() && idx < max; it++, idx++) {
		Mapping& ma = _keymapMappings[idx];
		ma.action = *it;
		ma.actionText->setLabel(ma.action->description + ": ");
		const HardwareKey *mappedKey = ma.action->getMappedKey();
		if (mappedKey)
			ma.keyButton->setLabel(mappedKey->description);
		else
			ma.keyButton->setLabel("-");
		_keymapMappings[idx].actionText->clearFlags(GUI::WIDGET_INVISIBLE);
		_keymapMappings[idx].keyButton->clearFlags(GUI::WIDGET_INVISIBLE);
	}
	while (idx < max) {
		_keymapMappings[idx].actionText->setFlags(GUI::WIDGET_INVISIBLE);
		_keymapMappings[idx].keyButton->setFlags(GUI::WIDGET_INVISIBLE);
		idx++;
	}
	draw();
}

void RemapDialog::setupWidgets(uint newNum) {
	uint num = _keymapMappings.size();
	if (num == newNum) return;

	uint textYOff = (_buttonHeight - kLineHeight) / 2;
	while (num < newNum) {
		uint x = _keymapArea.left + (num % _colCount) * (_colWidth + _spacing);
		uint y = _keymapArea.top + (num / _colCount) * (_buttonHeight + _spacing);
		Mapping ma;
		ma.action = 0;
		ma.actionText = new GUI::StaticTextWidget(this, x, y + textYOff, 
			_colWidth / 2, kLineHeight, "", Graphics::kTextAlignRight);
		ma.keyButton = new GUI::ButtonWidget(this, x + _colWidth / 2,
			y, _colWidth / 2, _buttonHeight, "", kRemapCmd + num);
		_keymapMappings.push_back(ma);
		num++;
	}
	while (num > newNum) {
		Mapping ma = _keymapMappings.remove_at(num - 1);
		removeWidget(ma.actionText);
		delete ma.actionText;
		removeWidget(ma.keyButton);
		delete ma.keyButton;
		num--;
	}
}

} // end of namespace Common
