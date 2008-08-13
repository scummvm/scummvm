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
	: Dialog("remap"), _activeRemapAction(0), _topAction(0), _remapTimeout(0) {

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	_keymapper = g_system->getEventManager()->getKeymapper();
	assert(_keymapper);
	_activeKeymaps = &_keymapper->_activeMaps;


	KeymapManager::Domain *_globalKeymaps = &_keymapper->_keymapMan->getGlobalDomain();
	KeymapManager::Domain *_gameKeymaps = 0;
	int keymapCount = 0;
	if (_globalKeymaps->count() == 0) {
		_globalKeymaps = 0;
	} else {
		keymapCount += _globalKeymaps->count();
	}
	if (ConfMan.getActiveDomain() != 0) {
		_gameKeymaps = &_keymapper->_keymapMan->getGameDomain();
		if (_gameKeymaps->count() == 0) {
			_gameKeymaps = 0;
		} else {
			keymapCount += _gameKeymaps->count();
		}
	}
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

	setupWidgets();
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
	if (cmd >= kRemapCmd && cmd < kRemapCmd + _keymapWidgets.size()) {
		startRemapping(cmd - kRemapCmd);
	} else if (cmd == GUI::kPopUpItemSelectedCmd) {
		loadKeymap();
	} else if (cmd == GUI::kSetPositionCmd) {
		refreshKeymap();
	} else {
		GUI::Dialog::handleCommand(sender, cmd, data);
	}
}

void RemapDialog::startRemapping(uint i) {
	_remapTimeout = getMillis() + kRemapTimeoutDelay;
	_activeRemapAction = _currentActions[_topAction + i].action;
	_keymapWidgets[i].keyButton->setLabel("...");
	_keymapWidgets[i].keyButton->draw();
	_keymapper->setEnabled(false);

}

void RemapDialog::stopRemapping() {
	_topAction = -1;
	refreshKeymap();
	_activeRemapAction = 0;
	_keymapper->setEnabled(true);
}

void RemapDialog::handleKeyUp(Common::KeyState state) {
	if (_activeRemapAction) {
		const HardwareKey *hwkey = _keymapper->getHardwareKey(state);
		if (hwkey) {
			_activeRemapAction->mapKey(hwkey);
			stopRemapping();
		}
	} else {
		GUI::Dialog::handleKeyDown(state);
	}
}

void RemapDialog::handleMouseDown(int x, int y, int button, int clickCount) {
	if (_activeRemapAction)
		stopRemapping();
	else
		Dialog::handleMouseDown(x, y, button, clickCount);
}

void RemapDialog::handleTickle() {
	if (_activeRemapAction && getMillis() > _remapTimeout)
		stopRemapping();
	Dialog::handleTickle();
}

void RemapDialog::loadKeymap() {
	_currentActions.clear();
	if (_activeKeymaps->size() > 0 && _kmPopUp->getSelected() == 0) {
		List<const HardwareKey*> freeKeys (_keymapper->_keymapMan->getHardwareKeySet()->getHardwareKeys());

		// add most active keymap's keys
		Keymapper::MapRecord top = _activeKeymaps->top();
		List<Action*>::iterator actIt;
		for (actIt = top.keymap->getActions().begin(); actIt != top.keymap->getActions().end(); ++actIt) {
			Action *act = *actIt;
			ActionInfo info = {act, false, act->description};
			_currentActions.push_back(info);
			if (act->getMappedKey())
				freeKeys.remove(act->getMappedKey());
		}
		
		// loop through remaining finding mappings for unmapped keys
		if (top.inherit) {
			for (int i = _activeKeymaps->size() - 2; i >= 0; --i) {
				Keymapper::MapRecord mr = (*_activeKeymaps)[i];
				List<const HardwareKey*>::iterator keyIt = freeKeys.begin();
				while (keyIt != freeKeys.end()) {
					Action *act = mr.keymap->getMappedAction((*keyIt)->key);
					if (act) {
						ActionInfo info = {act, true, act->description + " (" + mr.keymap->getName() + ")"};
						_currentActions.push_back(info);
						freeKeys.erase(keyIt++);
					} else {
						++keyIt;
					}
				}
				if (mr.inherit == false || freeKeys.empty()) break;
			}
		}

	} else if (_kmPopUp->getSelected() != -1) {
		Keymap *km = _keymapTable[_kmPopUp->getSelectedTag()];

		List<Action*>::iterator it;
		for (it = km->getActions().begin(); it != km->getActions().end(); it++) {
			ActionInfo info = {*it, false, (*it)->description};
			_currentActions.push_back(info);
		}
	}

	// refresh scroll bar
	_scrollBar->_currentPos = 0;
	_scrollBar->_numEntries = (_currentActions.size() + _colCount - 1) / _colCount;
	_scrollBar->recalc();
	// force refresh
	_topAction = -1;
	refreshKeymap();
}

void RemapDialog::refreshKeymap() {
	int newTopAction = _scrollBar->_currentPos * _colCount;
	if (newTopAction == _topAction) return;
	_topAction = newTopAction;

	//_container->draw();
	_scrollBar->draw();

	uint widgetI = 0;
	uint actionI = _topAction;
	for (uint widgetI = 0; widgetI < _keymapWidgets.size(); widgetI++) {
		ActionWidgets& widg = _keymapWidgets[widgetI];
		if (actionI < _currentActions.size()) {
			ActionInfo&    info = _currentActions[actionI];
			widg.actionText->setLabel(info.description + ": ");
			widg.actionText->setEnabled(!info.inherited);
			const HardwareKey *mappedKey = info.action->getMappedKey();
			if (mappedKey)
				widg.keyButton->setLabel(mappedKey->description);
			else
				widg.keyButton->setLabel("-");
			widg.actionText->clearFlags(GUI::WIDGET_INVISIBLE);
			widg.keyButton->clearFlags(GUI::WIDGET_INVISIBLE);
			actionI++; 
		} else {
			widg.actionText->setFlags(GUI::WIDGET_INVISIBLE);
			widg.keyButton->setFlags(GUI::WIDGET_INVISIBLE);
		}
		//widg.actionText->draw();
		//widg.keyButton->draw();
	}
	// need to redraw entire Dialog so that invisible 
	// widgets disappear
	draw();
}

void RemapDialog::setupWidgets() {

	int scrollbarWidth, buttonHeight;
	if (g_gui.getWidgetSize() == GUI::kBigWidgetSize) {
		buttonHeight = GUI::kBigButtonHeight;
		scrollbarWidth = GUI::kBigScrollBarWidth;
	} else {
		buttonHeight = GUI::kButtonHeight;
		scrollbarWidth = GUI::kNormalScrollBarWidth;
	}
	int areaX = g_gui.evaluator()->getVar("remap_keymap_area.x");
	int areaY = g_gui.evaluator()->getVar("remap_keymap_area.y");
	int areaW = g_gui.evaluator()->getVar("remap_keymap_area.w");
	int areaH = g_gui.evaluator()->getVar("remap_keymap_area.h");
	int spacing = g_gui.evaluator()->getVar("remap_spacing");

	_colCount = g_gui.evaluator()->getVar("remap_col_count");
	_rowCount = (areaH + spacing) / (buttonHeight + spacing);
	int colWidth = (areaW - scrollbarWidth - _colCount * spacing) / _colCount;

	//_container = new GUI::ContainerWidget(this, areaX, areaY, areaW, areaH);
	//_container->setHints(GUI::THEME_HINT_USE_SHADOW);

	_scrollBar = new GUI::ScrollBarWidget(this, 
		areaX + areaW - scrollbarWidth, areaY, scrollbarWidth, areaH);
	_scrollBar->_entriesPerPage = _rowCount;
	_scrollBar->_numEntries = 1;
	_scrollBar->recalc();

	uint textYOff = (buttonHeight - kLineHeight) / 2;
	int n = _rowCount * _colCount;
	for (int i = 0; i < n; i++) {
		uint x = areaX + (i % _colCount) * (colWidth + spacing);
		uint y = areaY + (i / _colCount) * (buttonHeight + spacing);
		ActionWidgets widg;
		widg.actionText = new GUI::StaticTextWidget(this, x, y + textYOff, 
			colWidth / 2, kLineHeight, "", Graphics::kTextAlignRight);
		widg.keyButton = new GUI::ButtonWidget(this, x + colWidth / 2,
			y, colWidth / 2, buttonHeight, "", kRemapCmd + i);
		_keymapWidgets.push_back(widg);
	}
}

} // end of namespace Common
