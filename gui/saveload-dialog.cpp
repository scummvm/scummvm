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

#include "gui/saveload-dialog.h"
#include "common/translation.h"

#include "gui/message.h"
#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"

#include "graphics/scaler.h"

namespace GUI {

enum {
	kListSwitchCmd = 'LIST',
	kGridSwitchCmd = 'GRID'
};

SaveLoadChooserDialog::SaveLoadChooserDialog(const Common::String &dialogName, const bool saveMode)
	: Dialog(dialogName), _metaEngine(0), _delSupport(false), _metaInfoSupport(false),
	_thumbnailSupport(false), _saveDateSupport(false), _playTimeSupport(false), _saveMode(saveMode),
	_listButton(0), _gridButton(0) {
	addChooserButtons();
}

SaveLoadChooserDialog::SaveLoadChooserDialog(int x, int y, int w, int h, const bool saveMode)
	: Dialog(x, y, w, h), _metaEngine(0), _delSupport(false), _metaInfoSupport(false),
	_thumbnailSupport(false), _saveDateSupport(false), _playTimeSupport(false), _saveMode(saveMode),
	_listButton(0), _gridButton(0) {
	addChooserButtons();
}

void SaveLoadChooserDialog::open() {
	Dialog::open();

	// So that quitting ScummVM will not cause the dialog result to say a
	// savegame was selected.
	setResult(-1);
}

int SaveLoadChooserDialog::run(const Common::String &target, const MetaEngine *metaEngine) {
	_metaEngine = metaEngine;
	_target = target;
	_delSupport = _metaEngine->hasFeature(MetaEngine::kSupportsDeleteSave);
	_metaInfoSupport = _metaEngine->hasFeature(MetaEngine::kSavesSupportMetaInfo);
	_thumbnailSupport = _metaInfoSupport && _metaEngine->hasFeature(MetaEngine::kSavesSupportThumbnail);
	_saveDateSupport = _metaInfoSupport && _metaEngine->hasFeature(MetaEngine::kSavesSupportCreationDate);
	_playTimeSupport = _metaInfoSupport && _metaEngine->hasFeature(MetaEngine::kSavesSupportPlayTime);

	return runIntern();
}

void SaveLoadChooserDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kListSwitchCmd:
		setResult(kSwitchToList);
		close();
		break;

	case kGridSwitchCmd:
		setResult(kSwitchToGrid);
		close();
		break;

	default:
		break;
	}

	return Dialog::handleCommand(sender, cmd, data);
}

void SaveLoadChooserDialog::addChooserButtons() {
	if (_listButton) {
		removeWidget(_listButton);
		delete _listButton;
	}

	if (_gridButton) {
		removeWidget(_gridButton);
		delete _gridButton;
	}

	_listButton = createSwitchButton("SaveLoadChooser.ListSwitch", "L", _("List view"), ThemeEngine::kImageList, kListSwitchCmd);
	_gridButton = createSwitchButton("SaveLoadChooser.GridSwitch", "G", _("Grid view"), ThemeEngine::kImageGrid, kGridSwitchCmd);
	if (!_metaInfoSupport || !_thumbnailSupport || _saveMode)
		_gridButton->setEnabled(false);
}

void SaveLoadChooserDialog::reflowLayout() {
	addChooserButtons();

	Dialog::reflowLayout();
}

GUI::ButtonWidget *SaveLoadChooserDialog::createSwitchButton(const Common::String &name, const char *desc, const char *tooltip, const char *image, uint32 cmd) {
	ButtonWidget *button;

#ifndef DISABLE_FANCY_THEMES
	if (g_gui.xmlEval()->getVar("Globals.ShowChooserPics") == 1 && g_gui.theme()->supportsImages()) {
		button = new PicButtonWidget(this, name, tooltip, cmd);
		((PicButtonWidget *)button)->useThemeTransparency(true);
		((PicButtonWidget *)button)->setGfx(g_gui.theme()->getImageSurface(image));
	} else
#endif
		button = new ButtonWidget(this, name, desc, tooltip, cmd);

	return button;
}

// SaveLoadChooserSimple implementation

enum {
	kChooseCmd = 'CHOS',
	kDelCmd = 'DEL '
};

SaveLoadChooserSimple::SaveLoadChooserSimple(const String &title, const String &buttonLabel, bool saveMode)
	: SaveLoadChooserDialog("SaveLoadChooser", saveMode), _list(0), _chooseButton(0), _deleteButton(0), _gfxWidget(0)  {
	_backgroundType = ThemeEngine::kDialogBackgroundSpecial;

	new StaticTextWidget(this, "SaveLoadChooser.Title", title);

	// Add choice list
	_list = new GUI::ListWidget(this, "SaveLoadChooser.List");
	_list->setNumberingMode(GUI::kListNumberingZero);
	_list->setEditable(saveMode);

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

int SaveLoadChooserSimple::runIntern() {
	if (_gfxWidget)
		_gfxWidget->setGfx(0);

	_resultString.clear();
	reflowLayout();
	updateSaveList();

	return Dialog::runModal();
}

const Common::String &SaveLoadChooserSimple::getResultString() const {
	int selItem = _list->getSelected();
	return (selItem >= 0) ? _list->getSelectedString() : _resultString;
}

void SaveLoadChooserSimple::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
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
				_metaEngine->removeSaveState(_target.c_str(), _saveList[selItem].getSaveSlot());

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
		SaveLoadChooserDialog::handleCommand(sender, cmd, data);
	}
}

void SaveLoadChooserSimple::reflowLayout() {
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

	SaveLoadChooserDialog::reflowLayout();
}

void SaveLoadChooserSimple::updateSelection(bool redraw) {
	int selItem = _list->getSelected();

	bool isDeletable = _delSupport;
	bool isWriteProtected = false;
	bool startEditMode = _list->isEditable();

	_gfxWidget->setGfx(-1, -1, _fillR, _fillG, _fillB);
	_date->setLabel(_("No date saved"));
	_time->setLabel(_("No time saved"));
	_playtime->setLabel(_("No playtime saved"));

	if (selItem >= 0 && _metaInfoSupport) {
		SaveStateDescriptor desc = _metaEngine->querySaveMetaInfos(_target.c_str(), _saveList[selItem].getSaveSlot());

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

void SaveLoadChooserSimple::close() {
	_metaEngine = 0;
	_target.clear();
	_saveList.clear();
	_list->setList(StringArray());

	SaveLoadChooserDialog::close();
}

void SaveLoadChooserSimple::updateSaveList() {
	_saveList = _metaEngine->listSaves(_target.c_str());

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

	int maximumSaveSlots = _metaEngine->getMaximumSaveSlot();

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

// LoadChooserThumbnailed implementation

enum {
	kNextCmd = 'NEXT',
	kPrevCmd = 'PREV'
};

LoadChooserThumbnailed::LoadChooserThumbnailed(const Common::String &title)
	: SaveLoadChooserDialog("SaveLoadChooser", false), _lines(0), _columns(0), _entriesPerPage(0),
	_curPage(0), _buttons() {
	_backgroundType = ThemeEngine::kDialogBackgroundSpecial;

	new StaticTextWidget(this, "SaveLoadChooser.Title", title);

	// Buttons
	new GUI::ButtonWidget(this, "SaveLoadChooser.Delete", _("Cancel"), 0, kCloseCmd);
	_nextButton = new GUI::ButtonWidget(this, "SaveLoadChooser.Choose", _("Next"), 0, kNextCmd);
	_nextButton->setEnabled(false);

	_prevButton = new GUI::ButtonWidget(this, "SaveLoadChooser.Cancel", _("Prev"), 0, kPrevCmd);
	_prevButton->setEnabled(false);
}

const Common::String &LoadChooserThumbnailed::getResultString() const {
	// FIXME: This chooser is only for loading, thus the result is never used
	// anyway. But this is still an ugly hack.
	return _target;
}

void LoadChooserThumbnailed::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd <= _entriesPerPage) {
		setResult(_saveList[cmd - 1 + _curPage * _entriesPerPage].getSaveSlot());
		close();
	}

	switch (cmd) {
	case kNextCmd:
		++_curPage;
		updateSaves();
		draw();
		break;

	case kPrevCmd:
		--_curPage;
		updateSaves();
		draw();
		break;

	case kCloseCmd:
		setResult(-1);
	default:
		SaveLoadChooserDialog::handleCommand(sender, cmd, data);
	}
}

void LoadChooserThumbnailed::handleMouseWheel(int x, int y, int direction) {
	if (direction > 0) {
		if (_nextButton->isEnabled()) {
			++_curPage;
			updateSaves();
			draw();
		}
	} else {
		if (_prevButton->isEnabled()) {
			--_curPage;
			updateSaves();
			draw();
		}
	}
}

void LoadChooserThumbnailed::open() {
	SaveLoadChooserDialog::open();

	_curPage = 0;
	_saveList = _metaEngine->listSaves(_target.c_str());
	updateSaves();
}

void LoadChooserThumbnailed::reflowLayout() {
	SaveLoadChooserDialog::reflowLayout();
	destroyButtons();

	const uint16 availableWidth = getWidth() - 20;
	uint16 availableHeight;

	int16 x, y;
	uint16 w;
	g_gui.xmlEval()->getWidgetData("SaveLoadChooser.List", x, y, w, availableHeight);

	const int16 buttonWidth = kThumbnailWidth + 6;
	const int16 buttonHeight = kThumbnailHeight2 + 6;

	const int16 containerFrameWidthAdd = 10;
	const int16 containerFrameHeightAdd = 0;
	const int16 containerWidth = buttonWidth + containerFrameWidthAdd;
	const int16 containerHeight = buttonHeight + kLineHeight + containerFrameHeightAdd;

	const int16 defaultSpacingHorizontal = 4;
	const int16 defaultSpacingVertical = 4;
	const int16 slotAreaWidth = containerWidth + defaultSpacingHorizontal;
	const int16 slotAreaHeight = containerHeight + defaultSpacingVertical;

	const uint oldEntriesPerPage = _entriesPerPage;
	_columns = MAX<uint>(1, availableWidth / slotAreaWidth);
	_lines = MAX<uint>(1, availableHeight / slotAreaHeight);
	_entriesPerPage = _columns * _lines;
	// Recalculate the page number
	if (!_saveList.empty() && oldEntriesPerPage != 0) {
		_curPage = (_curPage * oldEntriesPerPage) / _entriesPerPage;
	}

	const uint addX = _columns > 1 ? (availableWidth % slotAreaWidth) / (_columns - 1) : 0;
	//const uint addY = _lines > 1 ? (availableHeight % slotAreaHeight) / (_lines - 1) : 0;

	_buttons.reserve(_lines * _columns);
	y += defaultSpacingVertical / 2;
	for (uint curLine = 0; curLine < _lines; ++curLine, y += slotAreaHeight/* + addY*/) {
		for (uint curColumn = 0, curX = x + defaultSpacingHorizontal / 2; curColumn < _columns; ++curColumn, curX += slotAreaWidth + addX) {
			ContainerWidget *container = new ContainerWidget(this, curX, y, containerWidth, containerHeight);
			container->setVisible(false);
	
			int dstY = y + containerFrameHeightAdd / 2;
			int dstX = curX + containerFrameWidthAdd / 2;

			PicButtonWidget *button = new PicButtonWidget(this, dstX, dstY, buttonWidth, buttonHeight, 0, curLine * _columns + curColumn + 1);
			button->setVisible(false);
			dstY += buttonHeight;

			StaticTextWidget *description = new StaticTextWidget(this, dstX, dstY, buttonWidth, kLineHeight, Common::String(), Graphics::kTextAlignLeft);
			description->setVisible(false);

			_buttons.push_back(SlotButton(container, button, description));
		}
	}

	if (!_target.empty())
		updateSaves();
}

void LoadChooserThumbnailed::close() {
	SaveLoadChooserDialog::close();
	hideButtons();
}

int LoadChooserThumbnailed::runIntern() {
	return SaveLoadChooserDialog::runModal();
}

void LoadChooserThumbnailed::destroyButtons() {
	for (ButtonArray::iterator i = _buttons.begin(), end = _buttons.end(); i != end; ++i) {
		removeWidget(i->container);
		delete i->container;
		removeWidget(i->button);
		delete i->button;
		removeWidget(i->description);
		delete i->description;
	}

	_buttons.clear();
}

void LoadChooserThumbnailed::hideButtons() {
	for (ButtonArray::iterator i = _buttons.begin(), end = _buttons.end(); i != end; ++i) {
		i->button->setGfx(0);
		i->setVisible(false);
	}

}

void LoadChooserThumbnailed::updateSaves() {
	hideButtons();

	for (uint i = _curPage * _entriesPerPage, curNum = 0; i < _saveList.size() && curNum < _entriesPerPage; ++i, ++curNum) {
		const uint saveSlot = _saveList[i].getSaveSlot();

		SaveStateDescriptor desc = _metaEngine->querySaveMetaInfos(_target.c_str(), saveSlot);
		SlotButton &curButton = _buttons[curNum];
		curButton.setVisible(true);
		curButton.button->setGfx(desc.getThumbnail());
		curButton.description->setLabel(Common::String::format("%d. %s", saveSlot, desc.getDescription().c_str()));

		Common::String tooltip(_("Name: "));
		tooltip += desc.getDescription();

		if (_saveDateSupport) {
			const Common::String &saveDate = desc.getSaveDate();
			if (!saveDate.empty()) {
				tooltip += "\n";
				tooltip +=  _("Date: ") + saveDate;
			}

			const Common::String &saveTime = desc.getSaveTime();
			if (!saveTime.empty()) {
				tooltip += "\n";
				tooltip += _("Time: ") + saveTime;
			}
		}

		if (_playTimeSupport) {
			const Common::String &playTime = desc.getPlayTime();
			if (!playTime.empty()) {
				tooltip += "\n";
				tooltip += _("Playtime: ") + playTime;
			}
		}

		curButton.button->setTooltip(tooltip);
	}

	if (_curPage > 0)
		_prevButton->setEnabled(true);
	else
		_prevButton->setEnabled(false);

	if ((_curPage + 1) * _entriesPerPage < _saveList.size())
		_nextButton->setEnabled(true);
	else
		_nextButton->setEnabled(false);
}

} // End of namespace GUI
