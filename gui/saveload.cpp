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

#include "common/config-manager.h"

#include "gui/ListWidget.h"
#include "gui/message.h"
#include "gui/saveload.h"
#include "gui/ThemeEval.h"

#include "graphics/scaler.h"

#include "engines/metaengine.h"

namespace GUI {

enum {
	kChooseCmd = 'CHOS',
	kDelCmd = 'DEL '

};

SaveLoadChooser::SaveLoadChooser(const String &title, const String &buttonLabel)
	: Dialog("SaveLoadChooser"), _delSupport(0), _list(0), _chooseButton(0), _deleteButton(0), _gfxWidget(0)  {
	_delSupport = _metaInfoSupport = _thumbnailSupport = _saveDateSupport = _playTimeSupport = false;

	_backgroundType = ThemeEngine::kDialogBackgroundSpecial;

	new StaticTextWidget(this, "SaveLoadChooser.Title", title);

	// Add choice list
	_list = new GUI::ListWidget(this, "SaveLoadChooser.List");
	_list->setNumberingMode(GUI::kListNumberingZero);
	setSaveMode(false);

	_gfxWidget = new GUI::GraphicsWidget(this, 0, 0, 10, 10);

	_date = new StaticTextWidget(this, 0, 0, 10, 10, "No date saved", Graphics::kTextAlignCenter);
	_time = new StaticTextWidget(this, 0, 0, 10, 10, "No time saved", Graphics::kTextAlignCenter);
	_playtime = new StaticTextWidget(this, 0, 0, 10, 10, "No playtime saved", Graphics::kTextAlignCenter);

	// Buttons
	new GUI::ButtonWidget(this, "SaveLoadChooser.Cancel", "Cancel", kCloseCmd, 0);
	_chooseButton = new GUI::ButtonWidget(this, "SaveLoadChooser.Choose", buttonLabel, kChooseCmd, 0);
	_chooseButton->setEnabled(false);

	_deleteButton = new GUI::ButtonWidget(this, "SaveLoadChooser.Delete", "Delete", kDelCmd, 0);
	_deleteButton->setEnabled(false);

	_delSupport = _metaInfoSupport = _thumbnailSupport = false;

	_container = new GUI::ContainerWidget(this, 0, 0, 10, 10);
//	_container->setHints(GUI::THEME_HINT_USE_SHADOW);
}

SaveLoadChooser::~SaveLoadChooser() {
}

int SaveLoadChooser::runModal(const EnginePlugin *plugin, const String &target) {
	if (_gfxWidget)
		_gfxWidget->setGfx(0);

	// Set up the game domain as newly active domain, so
	// target specific savepath will be checked
	String oldDomain = ConfMan.getActiveDomainName();
	ConfMan.setActiveDomain(target);

	_plugin = plugin;
	_target = target;
	_delSupport = (*_plugin)->hasFeature(MetaEngine::kSupportsDeleteSave);
	_metaInfoSupport = (*_plugin)->hasFeature(MetaEngine::kSavesSupportMetaInfo);
	_thumbnailSupport = _metaInfoSupport && (*_plugin)->hasFeature(MetaEngine::kSavesSupportThumbnail);
	_saveDateSupport = _metaInfoSupport && (*_plugin)->hasFeature(MetaEngine::kSavesSupportCreationDate);
	_playTimeSupport = _metaInfoSupport && (*_plugin)->hasFeature(MetaEngine::kSavesSupportPlayTime);
	_resultString = "";
	reflowLayout();
	updateSaveList();

	int ret = Dialog::runModal();

	// Revert to the old active domain
	ConfMan.setActiveDomain(oldDomain);

	return ret;
}

void SaveLoadChooser::open() {
	Dialog::open();

	// So that quitting ScummVM will not cause the dialog result to say a
	// savegame was selected.
	setResult(-1);
}

const Common::String &SaveLoadChooser::getResultString() const {
	return (_list->getSelected() > -1) ? _list->getSelectedString() : _resultString;
}

void SaveLoadChooser::setSaveMode(bool saveMode) {
	_list->setEditable(saveMode);
}

void SaveLoadChooser::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	int selItem = _list->getSelected();

	switch (cmd) {
	case GUI::kListItemActivatedCmd:
	case GUI::kListItemDoubleClickedCmd:
		if (selItem >= 0 && _chooseButton->isEnabled()) {
			if (_list->isEditable() || !_list->getSelectedString().empty()) {
				_list->endEditMode();
				if (!_saveList.empty()) {
					setResult(atoi(_saveList[selItem].save_slot().c_str()));
					_resultString = _list->getSelectedString();
				}
				close();
			}
		}
		break;
	case kChooseCmd:
		_list->endEditMode();
		if (!_saveList.empty()) {
			setResult(atoi(_saveList[selItem].save_slot().c_str()));
			_resultString = _list->getSelectedString();
		}
		close();
		break;
	case GUI::kListSelectionChangedCmd:
		updateSelection(true);
		break;
	case kDelCmd:
		if (selItem >= 0 && _delSupport) {
			MessageDialog alert("Do you really want to delete this savegame?",
								"Delete", "Cancel");
			if (alert.runModal() == GUI::kMessageOK) {
				(*_plugin)->removeSaveState(_target.c_str(), atoi(_saveList[selItem].save_slot().c_str()));

				setResult(-1);
				_list->setSelected(-1);

				updateSaveList();
				updateSelection(true);
			}
		}
		break;
	case kCloseCmd:
		setResult(-1);
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void SaveLoadChooser::reflowLayout() {
	if (g_gui.xmlEval()->getVar("Globals.SaveLoadChooser.ExtInfo.Visible") == 1 && _thumbnailSupport) {
		int16 x, y;
		uint16 w, h;

		if (!g_gui.xmlEval()->getWidgetData("SaveLoadChooser.Thumbnail", x, y, w, h))
			error("Error when loading position data for Save/Load Thumbnails.");

		int thumbW = kThumbnailWidth;
		int thumbH = ((g_system->getHeight() % 200 && g_system->getHeight() != 350) ? kThumbnailHeight2 : kThumbnailHeight1);
		int thumbX = x + (w >> 1) - (thumbW >> 1);
		int thumbY = y + kLineHeight;

		int textLines = 0;
		if (!_saveDateSupport)
			textLines++;
		if (!_playTimeSupport)
			textLines++;

		_container->resize(x, y, w, h - (kLineHeight * textLines));
		_gfxWidget->resize(thumbX, thumbY, thumbW, thumbH);

		int height = thumbY + thumbH + kLineHeight;

		if (_saveDateSupport) {
			_date->resize(thumbX, height, kThumbnailWidth, kLineHeight);
			height += kLineHeight;
			_time->resize(thumbX, height, kThumbnailWidth, kLineHeight);
			height += kLineHeight;
		}

		if (_playTimeSupport)
			_playtime->resize(thumbX, height, kThumbnailWidth, kLineHeight);

		_container->setVisible(true);
		_gfxWidget->setVisible(true);

		_date->setVisible(_saveDateSupport);
		_time->setVisible(_saveDateSupport);

		_playtime->setVisible(_playTimeSupport);

		_fillR = 0;
		_fillG = 0;
		_fillB = 0;
		updateSelection(false);
	} else {
		_container->setVisible(false);
		_gfxWidget->setVisible(false);
		_date->setVisible(false);
		_time->setVisible(false);
		_playtime->setVisible(false);
	}

	Dialog::reflowLayout();
}

void SaveLoadChooser::updateSelection(bool redraw) {
	int selItem = _list->getSelected();

	bool isDeletable = _delSupport;
	bool isWriteProtected = false;
	bool startEditMode = _list->isEditable();

	_gfxWidget->setGfx(-1, -1, _fillR, _fillG, _fillB);
	_date->setLabel("No date saved");
	_time->setLabel("No time saved");
	_playtime->setLabel("No playtime saved");

	if (selItem >= 0 && !_list->getSelectedString().empty() && _metaInfoSupport) {
		SaveStateDescriptor desc = (*_plugin)->querySaveMetaInfos(_target.c_str(), atoi(_saveList[selItem].save_slot().c_str()));

		isDeletable = desc.getBool("is_deletable") && _delSupport;
		isWriteProtected = desc.getBool("is_write_protected");

		// Don't allow the user to change the description of write protected games
		if (isWriteProtected)
			startEditMode = false;

		if (_thumbnailSupport) {
			const Graphics::Surface *thumb = desc.getThumbnail();
			if (thumb) {
				_gfxWidget->setGfx(thumb);
				_gfxWidget->useAlpha(256);
			}
		}

		if (_saveDateSupport) {
			if (desc.contains("save_date"))
				_date->setLabel("Date: " + desc.getVal("save_date"));

			if (desc.contains("save_time"))
				_time->setLabel("Time: " + desc.getVal("save_time"));
		}

		if (_playTimeSupport) {
			if (desc.contains("play_time"))
				_playtime->setLabel("Playtime: " + desc.getVal("play_time"));
		}
	}


	if (_list->isEditable()) {
		// Disable the save button if nothing is selected, or if the selected
		// game is write protected
		_chooseButton->setEnabled(selItem >= 0 && !isWriteProtected);

		if (startEditMode)
			_list->startEditMode();
	} else {
		// Disable the load button if nothing is selected, or if an empty
		// list item is selected.
		_chooseButton->setEnabled(selItem >= 0 && !_list->getSelectedString().empty());
	}

	// Delete will always be disabled if the engine doesn't support it.
	_deleteButton->setEnabled(isDeletable && (selItem >= 0) && (!_list->getSelectedString().empty()));

	if (redraw) {
		_gfxWidget->draw();
		_date->draw();
		_time->draw();
		_playtime->draw();
		_chooseButton->draw();
		_deleteButton->draw();

		draw();
	}
}

void SaveLoadChooser::close() {
	_plugin = 0;
	_target.clear();
	_saveList.clear();
	_list->setList(StringList());

	Dialog::close();
}

void SaveLoadChooser::updateSaveList() {
	_saveList = (*_plugin)->listSaves(_target.c_str());

	int curSlot = 0;
	int saveSlot = 0;
	StringList saveNames;
	ListWidget::ColorList colors;
	for (SaveStateList::const_iterator x = _saveList.begin(); x != _saveList.end(); ++x) {
		// Handle gaps in the list of save games
		saveSlot = atoi(x->save_slot().c_str());
		if (curSlot < saveSlot) {
			while (curSlot < saveSlot) {
				SaveStateDescriptor dummySave(curSlot, "");
				_saveList.insert_at(curSlot, dummySave);
				saveNames.push_back(dummySave.description());
				colors.push_back(ThemeEngine::kFontColorNormal);
				curSlot++;
			}

			// Sync the save list iterator
			for (x = _saveList.begin(); x != _saveList.end(); ++x) {
				if (atoi(x->save_slot().c_str()) == saveSlot)
					break;
			}
		}

		// Show "Untitled savestate" for empty/whitespace savegame descriptions
		Common::String description = x->description();
		Common::String trimmedDescription = description;
		trimmedDescription.trim();
		if (trimmedDescription.empty()) {
			description = "Untitled savestate";
			colors.push_back(ThemeEngine::kFontColorAlternate);
		} else {
			colors.push_back(ThemeEngine::kFontColorNormal);
		}

		saveNames.push_back(description);
		curSlot++;
	}

	// Fill the rest of the save slots with empty saves
	Common::String emptyDesc;
	for (int i = curSlot; i <= (*_plugin)->getMaximumSaveSlot(); i++) {
		saveNames.push_back(emptyDesc);
		SaveStateDescriptor dummySave(i, "");
		_saveList.push_back(dummySave);
		colors.push_back(ThemeEngine::kFontColorNormal);
	}

	_list->setList(saveNames, &colors);
}

} // End of namespace GUI
