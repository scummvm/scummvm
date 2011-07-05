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
 */

#include "common/config-manager.h"
#include "common/translation.h"

#include "gui/widgets/list.h"
#include "gui/message.h"
#include "gui/saveload.h"
#include "gui/ThemeEval.h"
#include "gui/gui-manager.h"

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

	_date = new StaticTextWidget(this, 0, 0, 10, 10, _("No date saved"), Graphics::kTextAlignCenter);
	_time = new StaticTextWidget(this, 0, 0, 10, 10, _("No time saved"), Graphics::kTextAlignCenter);
	_playtime = new StaticTextWidget(this, 0, 0, 10, 10, _("No playtime saved"), Graphics::kTextAlignCenter);

	// Buttons
	new GUI::ButtonWidget(this, "SaveLoadChooser.Cancel", _("Cancel"), 0, kCloseCmd);
	_chooseButton = new GUI::ButtonWidget(this, "SaveLoadChooser.Choose", buttonLabel, 0, kChooseCmd);
	_chooseButton->setEnabled(false);

	_deleteButton = new GUI::ButtonWidget(this, "SaveLoadChooser.Delete", _("Delete"), 0, kDelCmd);
	_deleteButton->setEnabled(false);

	_delSupport = _metaInfoSupport = _thumbnailSupport = false;

	_container = new GUI::ContainerWidget(this, 0, 0, 10, 10);
//	_container->setHints(GUI::THEME_HINT_USE_SHADOW);
}

SaveLoadChooser::~SaveLoadChooser() {
}

int SaveLoadChooser::runModalWithPluginAndTarget(const EnginePlugin *plugin, const String &target) {
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
	int selItem = _list->getSelected();
	return (selItem >= 0) ? _list->getSelectedString() : _resultString;
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
					setResult(_saveList[selItem].getSaveSlot());
					_resultString = _list->getSelectedString();
				}
				close();
			}
		}
		break;
	case kChooseCmd:
		_list->endEditMode();
		if (!_saveList.empty()) {
			setResult(_saveList[selItem].getSaveSlot());
			_resultString = _list->getSelectedString();
		}
		close();
		break;
	case GUI::kListSelectionChangedCmd:
		updateSelection(true);
		break;
	case kDelCmd:
		if (selItem >= 0 && _delSupport) {
			MessageDialog alert(_("Do you really want to delete this savegame?"),
								_("Delete"), _("Cancel"));
			if (alert.runModal() == GUI::kMessageOK) {
				(*_plugin)->removeSaveState(_target.c_str(), _saveList[selItem].getSaveSlot());

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
			error("Error when loading position data for Save/Load Thumbnails");

		int thumbW = kThumbnailWidth;
		int thumbH = kThumbnailHeight2;
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
	_date->setLabel(_("No date saved"));
	_time->setLabel(_("No time saved"));
	_playtime->setLabel(_("No playtime saved"));

	if (selItem >= 0 && !_list->getSelectedString().empty() && _metaInfoSupport) {
		SaveStateDescriptor desc = (*_plugin)->querySaveMetaInfos(_target.c_str(), _saveList[selItem].getSaveSlot());

		isDeletable = desc.getDeletableFlag() && _delSupport;
		isWriteProtected = desc.getWriteProtectedFlag();

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
			const Common::String &saveDate = desc.getSaveDate();
			if (!saveDate.empty())
				_date->setLabel(_("Date: ") + saveDate);

			const Common::String &saveTime = desc.getSaveTime();
			if (!saveTime.empty())
				_time->setLabel(_("Time: ") + saveTime);
		}

		if (_playTimeSupport) {
			const Common::String &playTime = desc.getPlayTime();
			if (!playTime.empty())
				_playtime->setLabel(_("Playtime: ") + playTime);
		}
	}


	if (_list->isEditable()) {
		// Disable the save button if nothing is selected, or if the selected
		// game is write protected
		_chooseButton->setEnabled(selItem >= 0 && !isWriteProtected);

		if (startEditMode) {
			_list->startEditMode();

			if (_chooseButton->isEnabled() && _list->getSelectedString() == _("Untitled savestate") &&
					_list->getSelectionColor() == ThemeEngine::kFontColorAlternate) {
				_list->setEditString("");
				_list->setEditColor(ThemeEngine::kFontColorNormal);
			}
		}
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
	_list->setList(StringArray());

	Dialog::close();
}

void SaveLoadChooser::updateSaveList() {
	_saveList = (*_plugin)->listSaves(_target.c_str());

	int curSlot = 0;
	int saveSlot = 0;
	StringArray saveNames;
	ListWidget::ColorList colors;
	for (SaveStateList::const_iterator x = _saveList.begin(); x != _saveList.end(); ++x) {
		// Handle gaps in the list of save games
		saveSlot = x->getSaveSlot();
		if (curSlot < saveSlot) {
			while (curSlot < saveSlot) {
				SaveStateDescriptor dummySave(curSlot, "");
				_saveList.insert_at(curSlot, dummySave);
				saveNames.push_back(dummySave.getDescription());
				colors.push_back(ThemeEngine::kFontColorNormal);
				curSlot++;
			}

			// Sync the save list iterator
			for (x = _saveList.begin(); x != _saveList.end(); ++x) {
				if (x->getSaveSlot() == saveSlot)
					break;
			}
		}

		// Show "Untitled savestate" for empty/whitespace savegame descriptions
		Common::String description = x->getDescription();
		Common::String trimmedDescription = description;
		trimmedDescription.trim();
		if (trimmedDescription.empty()) {
			description = _("Untitled savestate");
			colors.push_back(ThemeEngine::kFontColorAlternate);
		} else {
			colors.push_back(ThemeEngine::kFontColorNormal);
		}

		saveNames.push_back(description);
		curSlot++;
	}

	// Fill the rest of the save slots with empty saves

	int maximumSaveSlots = (*_plugin)->getMaximumSaveSlot();

#ifdef __DS__
	// Low memory on the DS means too many save slots are impractical, so limit
	// the maximum here.
	if (maximumSaveSlots > 99) {
		maximumSaveSlots = 99;
	}
#endif

	Common::String emptyDesc;
	for (int i = curSlot; i <= maximumSaveSlots; i++) {
		saveNames.push_back(emptyDesc);
		SaveStateDescriptor dummySave(i, "");
		_saveList.push_back(dummySave);
		colors.push_back(ThemeEngine::kFontColorNormal);
	}

	_list->setList(saveNames, &colors);
}

} // End of namespace GUI
