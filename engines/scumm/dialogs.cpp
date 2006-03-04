/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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

#include "common/stdafx.h"

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"

#include "graphics/scaler.h"

#include "gui/about.h"
#include "gui/chooser.h"
#include "gui/newgui.h"
#include "gui/ListWidget.h"

#include "scumm/dialogs.h"
#include "scumm/sound.h"
#include "scumm/scumm.h"
#include "scumm/imuse/imuse.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/player_v2.h"
#include "scumm/verbs.h"
#include "sound/mididrv.h"
#include "sound/mixer.h"

#ifndef DISABLE_HELP
#include "scumm/help.h"
#endif

#ifdef SMALL_SCREEN_DEVICE
#include "KeysDialog.h"
#endif

using GUI::CommandSender;
using GUI::StaticTextWidget;
using GUI::kButtonWidth;
using GUI::kButtonHeight;
using GUI::kBigButtonWidth;
using GUI::kBigButtonHeight;
using GUI::kCloseCmd;
using GUI::kTextAlignCenter;
using GUI::kTextAlignLeft;
using GUI::WIDGET_ENABLED;

typedef GUI::OptionsDialog GUI_OptionsDialog;
typedef GUI::ChooserDialog GUI_ChooserDialog;
typedef GUI::Dialog GUI_Dialog;

namespace Scumm {

struct ResString {
	int num;
	char string[80];
};

#ifdef PALMOS_68K
static ResString *string_map_table_v7;
static ResString *string_map_table_v6;
static ResString *string_map_table_v5;
#else
static ResString string_map_table_v8[] = {
	{0, "/BT_100/Please insert disk %d. Press ENTER"},
	{0, "/BT__003/Unable to Find %s, (%s %d) Press Button."},
	{0, "/BT__004/Error reading disk %c, (%c%d) Press Button."},
	{0, "/BT__002/Game Paused.  Press SPACE to Continue."},
	{0, "/BT__005/Are you sure you want to restart?  (Y/N)"}, //BOOT.004
	{0, "/BT__006/Are you sure you want to quit?  (Y/N)"}, //BOOT.005
	{0, "/BT__008/Save"},
	{0, "/BT__009/Load"},
	{0, "/BT__010/Play"},
	{0, "/BT__011/Cancel"},
	{0, "/BT__012/Quit"},
	{0, "/BT__013/OK"},
	{0, ""},
	{0, "/BT__014/You must enter a name"},
	{0, "/BT__015/The game was NOT saved (disk full?)"},
	{0, "/BT__016/The game was NOT loaded"},
	{0, "/BT__017/Saving '%s'"},
	{0, "/BT__018/Loading '%s'"},
	{0, "/BT__019/Name your SAVE game"},
	{0, "/BT__020/Select a game to LOAD"}
};

static ResString string_map_table_v7[] = {
	{96, "game name and version"}, //that's how it's supposed to be
	{83, "Unable to Find %s, (%c%d) Press Button."},
	{84, "Error reading disk %c, (%c%d) Press Button."},
	{85, "/BOOT.003/Game Paused.  Press SPACE to Continue."},
	{86, "/BOOT.004/Are you sure you want to restart?  (Y/N)"},
	{87, "/BOOT.005/Are you sure you want to quit?  (Y/N)"},
	{70, "/BOOT.008/Save"},
	{71, "/BOOT.009/Load"},
	{72, "/BOOT.010/Play"},
	{73, "/BOOT.011/Cancel"}, 
	{74, "/BOOT.012/Quit"},
	{75, "/BOOT.013/OK"},
	{0, ""},
	{78, "/BOOT.014/You must enter a name"},
	{81, "/BOOT.015/The game was NOT saved (disk full?)"},
	{82, "/BOOT.016/The game was NOT loaded"},
	{79, "/BOOT.017/Saving '%s'"},
	{80, "/BOOT.018/Loading '%s'"},
	{76, "/BOOT.019/Name your SAVE game"},
	{77, "/BOOT.020/Select a game to LOAD"}

	/* This is the complete string map for v7
	{68, "/BOOT.007/c:\\dig"},
	{69, "/BOOT.021/The Dig"},
	{70, "/BOOT.008/Save"},
	{71, "/BOOT.009/Load"},
	{72, "/BOOT.010/Play"},
	{73, "/BOOT.011/Cancel"},
	{74, "/BOOT.012/Quit"},
	{75, "/BOOT.013/OK"},
	{76, "/BOOT.019/Name your SAVE game"},
	{77, "/BOOT.020/Select a game to LOAD"},
	{78, "/BOOT.014/You must enter a name"},
	{79, "/BOOT.017/Saving '%s'"},
	{80, "/BOOT.018/Loading '%s'"},
	{81, "/BOOT.015/The game was NOT saved (disk full?)"},
	{82, "/BOOT.016/The game was NOT loaded"},
	{83, "Unable to Find %s, (%c%d) Press Button."},
	{84, "Error reading disk %c, (%c%d) Press Button."},
	{85, "/BOOT.003/Game Paused.  Press SPACE to Continue."},
	{86, "/BOOT.004/Are you sure you want to restart?  (Y/N)"},
	{87, "/BOOT.005/Are you sure you want to quit?  (Y/N)"},
	{90, "/BOOT.022/Music"},
	{91, "/BOOT.023/Voice"},
	{92, "/BOOT.024/Sfx"},
	{93, "/BOOT.025/disabled"},
	{94, "/BOOT.026/Text speed"},
	{95, "/BOOT.027/Display Text"},
	{96, "The Dig v1.0"},
	{138, "/BOOT.028/Spooled Music"),
	{139, "/BOOT.029/Do you want to replace this saved game?  (Y/N)"},
	{141, "Voice Only"},
	{142, "Voice and Text"},
	{143, "Text Display Only"}, */

};

static ResString string_map_table_v6[] = {
	{90, "Insert Disk %c and Press Button to Continue."},
	{91, "Unable to Find %s, (%c%d) Press Button."},
	{92, "Error reading disk %c, (%c%d) Press Button."},
	{93, "Game Paused.  Press SPACE to Continue."},
	{94, "Are you sure you want to restart?  (Y/N)"},
	{95, "Are you sure you want to quit?  (Y/N)"},
	{96, "Save"},
	{97, "Load"},
	{98, "Play"},
	{99, "Cancel"},
	{100, "Quit"},
	{101, "OK"},
	{102, "Insert save/load game disk"},
	{103, "You must enter a name"},
	{104, "The game was NOT saved (disk full?)"},
	{105, "The game was NOT loaded"},
	{106, "Saving '%s'"},
	{107, "Loading '%s'"},
	{108, "Name your SAVE game"},
	{109, "Select a game to LOAD"},
	{117, "How may I serve you?"}
};

static ResString string_map_table_v345[] = {
	{1, "Insert Disk %c and Press Button to Continue."},
	{2, "Unable to Find %s, (%c%d) Press Button."},
	{3, "Error reading disk %c, (%c%d) Press Button."},
	{4, "Game Paused.  Press SPACE to Continue."},
	{5, "Are you sure you want to restart?  (Y/N)"},
	{6, "Are you sure you want to quit?  (Y/N)"},

	// Added in SCUMM4
	{7, "Save"},
	{8, "Load"},
	{9, "Play"},
	{10, "Cancel"},
	{11, "Quit"},
	{12, "OK"},
	{13, "Insert save/load game disk"},
	{14, "You must enter a name"},
	{15, "The game was NOT saved (disk full?)"},
	{16, "The game was NOT loaded"},
	{17, "Saving '%s'"},
	{18, "Loading '%s'"},
	{19, "Name your SAVE game"},
	{20, "Select a game to LOAD"},
	{28, "Game title"}
};
#endif

#pragma mark -

ScummDialog::ScummDialog(ScummEngine *scumm, int x, int y, int w, int h)
	: GUI::Dialog(x, y, w, h), _vm(scumm) {
_drawingHints |= GUI::THEME_HINT_SPECIAL_COLOR;
}

const Common::String ScummDialog::queryResString(int stringno) {
	byte buf[256];
	byte *result;

	if (stringno == 0)
		return String();

	if (_vm->_game.version == 8)
		result = (byte *)string_map_table_v8[stringno - 1].string;
	else if (_vm->_game.version == 7)
		result = _vm->getStringAddressVar(string_map_table_v7[stringno - 1].num);
	else if (_vm->_game.version == 6)
		result = _vm->getStringAddressVar(string_map_table_v6[stringno - 1].num);
	else if (_vm->_game.version >= 3)
		result = _vm->getStringAddress(string_map_table_v345[stringno - 1].num);
	else
		return string_map_table_v345[stringno - 1].string;

	if (result && *result == '/') {
		_vm->translateText(result, buf);
		result = buf;
	}

	if (!result || *result == '\0') {	// Gracelessly degrade to english :)
		return string_map_table_v345[stringno - 1].string;
	}

	// Convert to a proper string (take care of FF codes)
	byte chr;
	String tmp;
	while ((chr = *result++)) {
		if (chr == 0xFF) {
			result += 3;
		} else if (chr != '@') {
			tmp += chr;
		}
	}
	return tmp;
}

#pragma mark -

Common::StringList generateSavegameList(ScummEngine *scumm, bool saveMode);

enum {
	kSaveCmd = 'SAVE',
	kLoadCmd = 'LOAD',
	kPlayCmd = 'PLAY',
	kOptionsCmd = 'OPTN',
	kHelpCmd = 'HELP',
	kAboutCmd = 'ABOU',
	kQuitCmd = 'QUIT'
};

class SaveLoadChooser : public GUI::ChooserDialog, public BaseSaveLoadChooser {
	typedef Common::String String;
	typedef Common::StringList StringList;
protected:
	bool _saveMode;

public:
	SaveLoadChooser(const String &title, const String &buttonLabel, bool saveMode);

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
	const String &getResultString() const;
	void setList(const StringList& list) { GUI_ChooserDialog::setList(list); }
	int runModal() { return GUI_ChooserDialog::runModal(); }
};

SaveLoadChooser::SaveLoadChooser(const String &title, const String &buttonLabel, bool saveMode)
	: GUI::ChooserDialog(title, buttonLabel, 182), _saveMode(saveMode) {

	_list->setEditable(saveMode);
	_list->setNumberingMode(saveMode ? GUI::kListNumberingOne : GUI::kListNumberingZero);
}

const Common::String &SaveLoadChooser::getResultString() const {
	return _list->getSelectedString();
}

void SaveLoadChooser::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	int selItem = _list->getSelected();
	switch (cmd) {
	case GUI::kListItemActivatedCmd:
	case GUI::kListItemDoubleClickedCmd:
		if (selItem >= 0) {
			if (_saveMode || !getResultString().isEmpty()) {
				setResult(selItem);
				close();
			}
		}
		break;
	case GUI::kListSelectionChangedCmd:
		if (_saveMode) {
			_list->startEditMode();
		}
		// Disable button if nothing is selected, or (in load mode) if an empty
		// list item is selected. We allow choosing an empty item in save mode
		// because we then just assign a default name.
		_chooseButton->setEnabled(selItem >= 0 && (_saveMode || !getResultString().isEmpty()));
		_chooseButton->draw();
		break;
	default:
		GUI_ChooserDialog::handleCommand(sender, cmd, data);
	}
}

#pragma mark -

enum {
	kChooseCmd = 'Chos'
};

// only for use with >= 640x400 resolutions
class SaveLoadChooserEx : public GUI::Dialog, public BaseSaveLoadChooser {
	typedef Common::String String;
	typedef Common::StringList StringList;
protected:
	bool _saveMode;
	GUI::ListWidget		*_list;
	GUI::ButtonWidget	*_chooseButton;
	GUI::GraphicsWidget	*_gfxWidget;
	GUI::StaticTextWidget	*_date;
	GUI::StaticTextWidget	*_time;
	GUI::StaticTextWidget	*_playtime;
	ScummEngine			*_scumm;

public:
	SaveLoadChooserEx(const String &title, const String &buttonLabel, bool saveMode, ScummEngine *engine);

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
	const String &getResultString() const;
	void setList(const StringList& list);
	int runModal();
};

SaveLoadChooserEx::SaveLoadChooserEx(const String &title, const String &buttonLabel, bool saveMode, ScummEngine *engine)
	: Dialog(8, 8, engine->_system->getOverlayWidth() - 2 * 8, engine->_system->getOverlayHeight() - 16), _saveMode(saveMode), _list(0), _chooseButton(0), _gfxWidget(0), _scumm(engine) {

	new StaticTextWidget(this, 10, 6, _w - 2 * 10, kLineHeight, title, kTextAlignCenter);

	// Add choice list
	_list = new GUI::ListWidget(this, 10, 18, _w - 2 * 10 - 180, _h - 14 - kBigButtonHeight - 18, GUI::kBigWidgetSize);
	_list->setEditable(saveMode);
	_list->setNumberingMode(saveMode ? GUI::kListNumberingOne : GUI::kListNumberingZero);

	// Add the thumbnail display
	_gfxWidget = new GUI::GraphicsWidget(this,
			_w - (kThumbnailWidth + 22),
			18,
			kThumbnailWidth + 8,
			((_scumm->_system->getHeight() % 200 && _scumm->_system->getHeight() != 350) ? kThumbnailHeight2 : kThumbnailHeight1) + 8);
	_gfxWidget->setFlags(GUI::WIDGET_BORDER);

	int height = 18 + ((_scumm->_system->getHeight() % 200 && _scumm->_system->getHeight() != 350) ? kThumbnailHeight2 : kThumbnailHeight1) + 8;

	_date = new StaticTextWidget(this,
					_w - (kThumbnailWidth + 22),
					height,
					kThumbnailWidth + 8,
					kLineHeight,
					"No date saved",
					kTextAlignCenter);
	_date->setFlags(GUI::WIDGET_CLEARBG);

	height += kLineHeight;

	_time = new StaticTextWidget(this,
					_w - (kThumbnailWidth + 22),
					height,
					kThumbnailWidth + 8,
					kLineHeight,
					"No time saved",
					kTextAlignCenter);
	_time->setFlags(GUI::WIDGET_CLEARBG);

	height += kLineHeight;

	_playtime = new StaticTextWidget(this,
					_w - (kThumbnailWidth + 22),
					height,
					kThumbnailWidth + 8,
					kLineHeight,
					"No playtime saved",
					kTextAlignCenter);
	_playtime->setFlags(GUI::WIDGET_CLEARBG);

	// Buttons
	addButton(this, _w - 2 * (kBigButtonWidth + 10), _h - kBigButtonHeight - 8, "Cancel", kCloseCmd, 0, GUI::kBigWidgetSize);
	_chooseButton = addButton(this, _w - (kBigButtonWidth + 10), _h - kBigButtonHeight - 8, buttonLabel, kChooseCmd, 0, GUI::kBigWidgetSize);
	_chooseButton->setEnabled(false);
}

const Common::String &SaveLoadChooserEx::getResultString() const {
	return _list->getSelectedString();
}

void SaveLoadChooserEx::setList(const StringList& list) {
	_list->setList(list);
}

int SaveLoadChooserEx::runModal() {
	_gfxWidget->setGfx(0);
	int ret = Dialog::runModal();
	return ret;
}

void SaveLoadChooserEx::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	int selItem = _list->getSelected();
	switch (cmd) {
	case GUI::kListItemActivatedCmd:
	case GUI::kListItemDoubleClickedCmd:
		if (selItem >= 0) {
			if (_saveMode || !getResultString().isEmpty()) {
				_list->endEditMode();
				setResult(selItem);
				close();
			}
		}
		break;
	case kChooseCmd:
		_list->endEditMode();
		setResult(selItem);
		close();
		break;
	case GUI::kListSelectionChangedCmd: {
		Graphics::Surface *thumb;
		thumb = _scumm->loadThumbnailFromSlot(_saveMode ? selItem + 1 : selItem);
		_gfxWidget->setGfx(thumb);
		if (thumb)
			thumb->free();
		delete thumb;
		_gfxWidget->draw();

		InfoStuff infos;
		memset(&infos, 0, sizeof(InfoStuff));
		char buffer[32];
		if (_scumm->loadInfosFromSlot(_saveMode ? selItem + 1 : selItem, &infos)) {
			snprintf(buffer, 32, "Date: %.2d.%.2d.%.4d",
				(infos.date >> 24) & 0xFF, (infos.date >> 16) & 0xFF,
				infos.date & 0xFFFF);
			_date->setLabel(buffer);
			_date->draw();
			
			snprintf(buffer, 32, "Time: %.2d:%.2d",
				(infos.time >> 8) & 0xFF, infos.time & 0xFF);
			_time->setLabel(buffer);
			_time->draw();

			int minutes = infos.playtime / 60;
			int hours = minutes / 60;
			minutes %= 60;

			snprintf(buffer, 32, "Playtime: %.2d:%.2d",
				hours & 0xFF, minutes & 0xFF);
			_playtime->setLabel(buffer);
			_playtime->draw();
		} else {
			snprintf(buffer, 32, "No date saved");
			_date->setLabel(buffer);
			_date->draw();
			
			snprintf(buffer, 32, "No time saved");
			_time->setLabel(buffer);
			_time->draw();

			snprintf(buffer, 32, "No playtime saved");
			_playtime->setLabel(buffer);
			_playtime->draw();
		}

		if (_saveMode) {
			_list->startEditMode();
		}
		// Disable button if nothing is selected, or (in load mode) if an empty
		// list item is selected. We allow choosing an empty item in save mode
		// because we then just assign a default name.
		_chooseButton->setEnabled(selItem >= 0 && (_saveMode || !getResultString().isEmpty()));
		_chooseButton->draw();
	} break;
	case kCloseCmd:
		setResult(-1);
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

#pragma mark -

Common::StringList generateSavegameList(ScummEngine *scumm, bool saveMode) {
	// Get savegame names
	Common::StringList l;
	char name[32];
	uint i = saveMode ? 1 : 0;
	bool avail_saves[81];

	scumm->listSavegames(avail_saves, ARRAYSIZE(avail_saves));
	for (; i < ARRAYSIZE(avail_saves); i++) {
		if (avail_saves[i])
			scumm->getSavegameName(i, name);
		else
			name[0] = 0;
		l.push_back(name);
	}

	return l;
}

#define addBigButton(label, cmd, hotkey) \
	new GUI::ButtonWidget(this, hOffset, y, buttonWidth, buttonHeight, label, cmd, hotkey, ws); \
	y += (buttonHeight + vAddOff)

MainMenuDialog::MainMenuDialog(ScummEngine *scumm)
	: ScummDialog(scumm, 0, 0, 0, 0) {

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	int hOffset;
	int vSpace;
	int vAddOff;

	GUI::WidgetSize ws;
	int buttonWidth;
	int buttonHeight;

	if (screenW >= 400 && screenH >= 300) {
		buttonWidth = 160;
		buttonHeight = 28;
		ws = GUI::kBigWidgetSize;
		hOffset = 12;
		vSpace = 7;
		vAddOff = 3;
	} else {
		buttonWidth = 90;
		buttonHeight = 16;
		ws = GUI::kNormalWidgetSize;
		hOffset = 8;
		vSpace = 5;
		vAddOff = 2;
	}

	int y = vSpace + vAddOff;


	addBigButton("Resume", kPlayCmd, 'P');
	y += vSpace;

	addBigButton("Load", kLoadCmd, 'L');
	addBigButton("Save", kSaveCmd, 'S');
	y += vSpace;

	addBigButton("Options", kOptionsCmd, 'O');
#ifndef DISABLE_HELP
	addBigButton("Help", kHelpCmd, 'H');
#endif
	addBigButton("About", kAboutCmd, 'A');
	y += vSpace;

	addBigButton("Quit", kQuitCmd, 'Q');


	_w = buttonWidth + 2 * hOffset;
	_h = y + vSpace;

	_x = (screenW - _w) / 2;
	_y = (screenH - _h) / 2;


	//
	// Create the sub dialog(s)
	//
	_aboutDialog = new GUI::AboutDialog();
	_optionsDialog = new ConfigDialog(scumm);
#ifndef DISABLE_HELP
	_helpDialog = new HelpDialog(scumm);
#endif
	if (scumm->_system->getOverlayWidth() <= 320) {
		_saveDialog = new SaveLoadChooser("Save game:", "Save", true);
		_loadDialog = new SaveLoadChooser("Load game:", "Load", false);
	} else {
		_saveDialog = new SaveLoadChooserEx("Save game:", "Save", true, scumm);
		_loadDialog = new SaveLoadChooserEx("Load game:", "Load", false, scumm);
	}
}

MainMenuDialog::~MainMenuDialog() {
	delete _aboutDialog;
	delete _optionsDialog;
#ifndef DISABLE_HELP
	delete _helpDialog;
#endif
	delete _saveDialog;
	delete _loadDialog;
}

void MainMenuDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kSaveCmd:
		save();
		break;
	case kLoadCmd:
		load();
		break;
	case kPlayCmd:
		close();
		break;
	case kOptionsCmd:
		_optionsDialog->runModal();
		break;
	case kAboutCmd:
		_aboutDialog->runModal();
		break;
#ifndef DISABLE_HELP
	case kHelpCmd:
		_helpDialog->runModal();
		break;
#endif
	case kQuitCmd:
		_vm->_quit = true;
		close();
		break;
	default:
		ScummDialog::handleCommand(sender, cmd, data);
	}
}

void MainMenuDialog::save() {
	int idx;
	_saveDialog->setList(generateSavegameList(_vm, true));
	idx = _saveDialog->runModal();
	if (idx >= 0) {
		const String &result = _saveDialog->getResultString();
		char buffer[20];
		const char *str;
		if (result.isEmpty()) {
			// If the user was lazy and entered no save name, come up with a default name.
			sprintf(buffer, "Save %d", idx + 1);
			str = buffer;
		} else
			str = result.c_str();
		_vm->requestSave(idx + 1, str);
		close();
	}
}

void MainMenuDialog::load() {
	int idx;
	_loadDialog->setList(generateSavegameList(_vm, false));
	idx = _loadDialog->runModal();
	if (idx >= 0) {
		_vm->requestLoad(idx);
		close();
	}
}

#pragma mark -

enum {
	kOKCmd					= 'ok  '
};

enum {
	kKeysCmd = 'KEYS'
};

ConfigDialog::ConfigDialog(ScummEngine *scumm)
	: GUI::OptionsDialog("", 40, 30, 240, 124), _vm(scumm) {

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	_w = screenW - 2 * 40;

	GUI::WidgetSize ws;
	int buttonWidth;
	int buttonHeight;

	if (screenW >= 400 && screenH >= 300) {
		ws = GUI::kBigWidgetSize;
		buttonWidth = kBigButtonWidth;
		buttonHeight = kBigButtonHeight;
	} else {
		ws = GUI::kNormalWidgetSize;
		buttonWidth = kButtonWidth;
		buttonHeight = kButtonHeight;
	}

	int yoffset = 8;

	//
	// Sound controllers
	//

	yoffset = addVolumeControls(this, yoffset, ws) + 4;

	//
	// Some misc options
	//

	_subtitlesCheckbox = addCheckbox(this, 15, yoffset, "Show subtitles", 0, 'S', ws);
	yoffset += _subtitlesCheckbox->getHeight();

	_speechCheckbox = addCheckbox(this, 15, yoffset, "Enable speech", 0, 'E', ws);
	yoffset += _speechCheckbox->getHeight() + 4;

	//
	// Add the buttons
	//

	_w = 8 + 3 * (buttonWidth + 4); // FIXME/TODO

	addButton(this, _w - (buttonWidth + 4) - 4, yoffset, "OK", GUI::OptionsDialog::kOKCmd, 'O', ws);
	addButton(this, _w - 2 * (buttonWidth + 4) - 4, yoffset, "Cancel", kCloseCmd, 'C', ws);
#ifdef SMALL_SCREEN_DEVICE
	addButton(this, _w - 3 * (buttonWidth + 4) - 4, yoffset, "Keys", kKeysCmd, 'K', ws);
#endif

	yoffset += buttonHeight;

	_h = yoffset + 8;

	_x = (screenW - _w) / 2;
	_y = (screenH - _h) / 2;

#ifdef SMALL_SCREEN_DEVICE
	//
	// Create the sub dialog(s)
	//

	_keysDialog = new GUI::KeysDialog();
#endif
}

ConfigDialog::~ConfigDialog() {
#ifdef SMALL_SCREEN_DEVICE
	delete _keysDialog;
#endif
}

void ConfigDialog::open() {
	GUI_OptionsDialog::open();

	// update checkboxes, too
	_subtitlesCheckbox->setState(ConfMan.getBool("subtitles"));
	_speechCheckbox->setState(!ConfMan.getBool("speech_mute"));
}

void ConfigDialog::close() {
	if (getResult()) {
		// Subtitles
		ConfMan.set("subtitles", _subtitlesCheckbox->getState(), _domain);
		ConfMan.set("speech_mute", !_speechCheckbox->getState(), _domain);
		// Sync with current setting
		if (ConfMan.getBool("speech_mute"))
			_vm->_voiceMode = 2;
		else
			_vm->_voiceMode = ConfMan.getBool("subtitles");

		if (_vm->_game.version >= 7)
			_vm->VAR(_vm->VAR_VOICE_MODE) = _vm->_voiceMode;
	}

	GUI_OptionsDialog::close();

	_vm->setupVolumes();
}

void ConfigDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kKeysCmd:
#ifdef SMALL_SCREEN_DEVICE
		_keysDialog->runModal();
#endif
		break;
	default:
		GUI_OptionsDialog::handleCommand (sender, cmd, data);
	}
}

#ifndef DISABLE_HELP

#pragma mark -

enum {
	kNextCmd = 'NEXT',
	kPrevCmd = 'PREV'
};

HelpDialog::HelpDialog(ScummEngine *scumm)
	: ScummDialog(scumm, 5, 5, 310, 190) {
	_drawingHints &= ~GUI::THEME_HINT_SPECIAL_COLOR;

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	GUI::WidgetSize ws;
	int buttonHeight;
	int buttonWidth;

	if (screenW >= 400 && screenH >= 300) {
		ws = GUI::kBigWidgetSize;
		buttonHeight = kBigButtonHeight;
		buttonWidth = kBigButtonWidth;
		_w = 370;
		_x = (screenW - _w) / 2;
	} else {
		ws = GUI::kNormalWidgetSize;
		buttonHeight = kButtonHeight;
		buttonWidth = kButtonWidth;
		_x = 5;
		_w = screenW - 2 * 5;
	}

	int lineHeight = g_gui.getFontHeight();

	_h = 5 + (2 + HELP_NUM_LINES) * lineHeight + buttonHeight + 7;
	_y = (screenH - _h) / 2;

	_title = new StaticTextWidget(this, 10, 5, _w, lineHeight, "", kTextAlignCenter, ws);

	for (int i = 0; i < HELP_NUM_LINES; i++) {
		_key[i] = new StaticTextWidget(this, 10, 5 + lineHeight * (i + 2), 80, lineHeight, "", kTextAlignLeft, ws);
		_dsc[i] = new StaticTextWidget(this, 90, 5 + lineHeight * (i + 2), _w - 10 - 90, lineHeight, "", kTextAlignLeft, ws);
	}

	_page = 1;
	_numPages = ScummHelp::numPages(scumm->_game.id);

	int y = 5 + lineHeight * (HELP_NUM_LINES + 2) + 2;

	_prevButton = addButton(this, 10, y, "Previous", kPrevCmd, 'P', ws);
	_nextButton = addButton(this, 10 + buttonWidth + 8, y, "Next", kNextCmd, 'N', ws);
	addButton(this, _w - 8 - buttonWidth, y, "Close", kCloseCmd, 'C', ws);
	_prevButton->clearFlags(WIDGET_ENABLED);

	displayKeyBindings();
}

void HelpDialog::displayKeyBindings() {

	String titleStr, *keyStr, *dscStr;

	ScummHelp::updateStrings(_vm->_game.id, _vm->_game.version, _vm->_game.platform, _page, titleStr, keyStr, dscStr);

	_title->setLabel(titleStr);
	for (int i = 0; i < HELP_NUM_LINES; i++) {
		_key[i]->setLabel(keyStr[i]);
		_dsc[i]->setLabel(dscStr[i]);
	}

	delete [] keyStr;
	delete [] dscStr;
}

void HelpDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {

	switch (cmd) {
	case kNextCmd:
		_page++;
		if (_page >= _numPages) {
			_nextButton->clearFlags(WIDGET_ENABLED);
		}
		if (_page >= 2) {
			_prevButton->setFlags(WIDGET_ENABLED);
		}
		displayKeyBindings();
		draw();
		break;
	case kPrevCmd:
		_page--;
		if (_page <= _numPages) {
			_nextButton->setFlags(WIDGET_ENABLED);
		}
		if (_page <= 1) {
			_prevButton->clearFlags(WIDGET_ENABLED);
		}
		displayKeyBindings();
		draw();
		break;
	default:
		ScummDialog::handleCommand(sender, cmd, data);
	}
}

#endif

#pragma mark -

InfoDialog::InfoDialog(ScummEngine *scumm, int res)
: ScummDialog(scumm, 0, 80, 0, 16) { // dummy x and w
	setInfoText(queryResString (res));
}

InfoDialog::InfoDialog(ScummEngine *scumm, const String& message)
: ScummDialog(scumm, 0, 80, 0, 16) { // dummy x and w
	setInfoText(message);
}

void InfoDialog::setInfoText(const String& message) {
	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();
	GUI::WidgetSize ws;

	if (screenW >= 400 && screenH >= 300) {
		ws = GUI::kBigWidgetSize;
	} else {
		ws = GUI::kNormalWidgetSize;
	}

	int width = g_gui.getStringWidth(message) + 16;
	int height = g_gui.getFontHeight() + 8;

	_w = width;
	_h = height;
	_x = (screenW - width) / 2;
	_y = (screenH - height) / 2;

	new StaticTextWidget(this, 4, 4, _w - 8, _h, message, kTextAlignCenter, ws);
}

#pragma mark -

PauseDialog::PauseDialog(ScummEngine *scumm, int res)
	: InfoDialog(scumm, res) {
}

void PauseDialog::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	if (ascii == ' ')  // Close pause dialog if space key is pressed
		close();
	else
		ScummDialog::handleKeyDown(ascii, keycode, modifiers);
}

ConfirmDialog::ConfirmDialog(ScummEngine *scumm, int res)
	: InfoDialog(scumm, res) {
}

void ConfirmDialog::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	if (tolower(ascii) == 'n') {
		setResult(0);
		close();
	} else if (tolower(ascii) == 'y') {
		setResult(1);
		close();
	} else
		ScummDialog::handleKeyDown(ascii, keycode, modifiers);
}

#pragma mark -

ValueDisplayDialog::ValueDisplayDialog(const Common::String& label, int minVal, int maxVal, int val, uint16 incKey, uint16 decKey)
	: GUI::Dialog(0, 80, 0, 16), _label(label), _min(minVal), _max(maxVal), _value(val), _incKey(incKey), _decKey(decKey) {
	assert(_min <= _value && _value <= _max);

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	if (screenW >= 400 && screenH >= 300) {
		_percentBarWidth = kBigPercentBarWidth;
	} else {
		_percentBarWidth = kPercentBarWidth;
	}

	int width = g_gui.getStringWidth(label) + 16 + _percentBarWidth;
	int height = g_gui.getFontHeight() + 4 * 2;

	_x = (screenW - width) / 2;
	_y = (screenH - height) / 2;
	_w = width;
	_h = height;
}

void ValueDisplayDialog::drawDialog() {
	const int labelWidth = _w - 8 - _percentBarWidth;
	g_gui.theme()->drawDialogBackground(Common::Rect(_x, _y, _x+_w, _y+_h), GUI::THEME_HINT_SAVE_BACKGROUND | GUI::THEME_HINT_FIRST_DRAW);
	g_gui.theme()->drawText(Common::Rect(_x+4, _y+4, _x+labelWidth+4, _y+g_gui.theme()->getFontHeight()+4), _label);
	g_gui.theme()->drawSlider(Common::Rect(_x+4+labelWidth, _y+4, _x+_w-4, _y+_h-4), _percentBarWidth * (_value - _min) / (_max - _min));
}

void ValueDisplayDialog::handleTickle() {
	if (getMillis() > _timer)
		close();
}

void ValueDisplayDialog::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	if (ascii == _incKey || ascii == _decKey) {
		if (ascii == _incKey && _value < _max)
			_value++;
		else if (ascii == _decKey && _value > _min)
			_value--;

		setResult(_value);
		_timer = getMillis() + kDisplayDelay;
		draw();
	} else {
		close();
	}
}

void ValueDisplayDialog::open() {
	GUI_Dialog::open();
	setResult(_value);
	_timer = getMillis() + kDisplayDelay;
}



} // End of namespace Scumm

#ifdef PALMOS_68K
#include "scumm_globals.h"

_GINIT(Dialogs)
_GSETPTR(Scumm::string_map_table_v7, GBVARS_STRINGMAPTABLEV7_INDEX, Scumm::ResString, GBVARS_SCUMM)
_GSETPTR(Scumm::string_map_table_v6, GBVARS_STRINGMAPTABLEV6_INDEX, Scumm::ResString, GBVARS_SCUMM)
_GSETPTR(Scumm::string_map_table_v5, GBVARS_STRINGMAPTABLEV5_INDEX, Scumm::ResString, GBVARS_SCUMM)
_GEND

_GRELEASE(Dialogs)
_GRELEASEPTR(GBVARS_STRINGMAPTABLEV7_INDEX, GBVARS_SCUMM)
_GRELEASEPTR(GBVARS_STRINGMAPTABLEV6_INDEX, GBVARS_SCUMM)
_GRELEASEPTR(GBVARS_STRINGMAPTABLEV5_INDEX, GBVARS_SCUMM)
_GEND

#endif
