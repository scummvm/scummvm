/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/scaler.h"

#include "gui/about.h"
#include "gui/chooser.h"
#include "gui/newgui.h"
#include "gui/ListWidget.h"

#include "scumm/dialogs.h"
#include "scumm/sound.h"
#include "scumm/scumm.h"
#include "scumm/imuse.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/player_v2.h"
#include "scumm/verbs.h"
#include "sound/mididrv.h"
#include "sound/mixer.h"

#ifndef DISABLE_HELP
#include "scumm/help.h"
#endif

#ifdef _WIN32_WCE
#include "backends/wince/CEKeysDialog.h"
#endif

using GUI::CommandSender;
using GUI::StaticTextWidget;
using GUI::kButtonWidth;
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

#ifdef __PALM_OS__
static ResString *string_map_table_v7;
static ResString *string_map_table_v6;
static ResString *string_map_table_v5;
#else
static ResString string_map_table_v7[] = {
	{96, "game name and version"}, //that's how it's supposed to be
	{77, "Select a game to LOAD"},
	{76, "Name your SAVE game"},
	{70, "save"}, //boot8
	{71, "load"}, //boot9
	{72, "play"}, //boot10
	{73, "cancel"}, //boot11
	{74, "quit"}, //boot12
	{75, "ok"}, //boot13
	{85, "game paused"}, // boot3
	{96, "the dig v1.0"},

	/* this is the almost complete string map for v7
	{63, "how may I serve you?"},
	{64, "the dig v1.0"}, //(game name/version)
	{67, "text display only"},
	{68, "c:\\dig"}, //boot007 (save path ?)
	{69, "the dig"}, //boot21 (game name)
	{70, "save"}, //boot8
	{71, "load"}, //boot9
	{72, "play"}, //boot10
	{73, "cancel"}, //boot11
	{74, "quit"}, //boot12
	{75, "ok"}, //boot13
	{76, "name your save game"}, //boot19
	{77, "select a game to load"}, //boot20
	{78, "you must enter a name"},//boot14
	{79, "saving '%s'"}, //boot17
	{80, "loading '%s'"}, //boot18
	{81, "the game was NOT saved"}, //boot15
	{82, "the game was NOT loaded"}, //boot16
	{83, "how may I serve you?"},
	{84, "how may I serve you?"},
	{85, "game paused"}, // boot3
	{86, "Are you sure you want to restart"},
	{87, "Are you sure you want to quit?"}, //boot05
	{89, "how may I serve you?"},
	{90, "music"}, //boot22
	{91, "voice"}, //boot23
	{92, "sfx"}, //boot24
	{93, "disabled"}, //boot25
	{94, "text speed"}, //boot26
	{95, "text display"}, //boot27
	{96, "the dig v1.0"},*/
	
};

static ResString string_map_table_v6[] = {
	{117, "How may I serve you?"}, 
	{109, "Select a game to LOAD"}, 
	{108, "Name your SAVE game"}, 
	{96, "Save"}, 
	{97, "Load"}, 
	{98, "Play"}, 
	{99, "Cancel"}, 
	{100, "Quit"}, 
	{101, "OK"}, 
	{93, "Game paused"}, 
	{210, "Game version"}
};

static ResString string_map_table_v5[] = {
	{28, "How may I serve you?"}, 
	{20, "Select a game to LOAD"},
	{19, "Name your SAVE game"},
	{7, "Save"},
	{8, "Load"},
	{9, "Play"},
	{10, "Cancel"},
	{11, "Quit"},
	{12, "OK"},
	{4, "Game paused"}
};
#endif

#pragma mark -

const Common::String ScummDialog::queryResString(int stringno) {
	byte buf[256];
	byte *result;

	if (stringno == 0)
		return String();

	if (_vm->_version >= 7)
		result = _vm->getStringAddressVar(string_map_table_v7[stringno - 1].num);
	else if (_vm->_version == 6)
		result = _vm->getStringAddressVar(string_map_table_v6[stringno - 1].num);
	else if (_vm->_version == 5)
		result = _vm->getStringAddress(string_map_table_v5[stringno - 1].num);
	else
		// TODO: For V8 games, maybe grab the strings from the language file?
		return string_map_table_v5[stringno - 1].string;

	if (result && *result == '/') {
		_vm->translateText(result, buf);
		result = buf;
	}

	if (!result || *result == '\0') {	// Gracelessly degrade to english :)
		return string_map_table_v5[stringno - 1].string;
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
	void setList(const StringList& list) { GUI::ChooserDialog::setList(list); }
	int runModal() { return GUI::ChooserDialog::runModal(); }
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
	ScummEngine			*_scumm;

public:
	SaveLoadChooserEx(const String &title, const String &buttonLabel, bool saveMode, ScummEngine *engine);
	
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
	const String &getResultString() const;	
	void setList(const StringList& list);
	int runModal();

	bool wantsScaling() const { return false; }
};

SaveLoadChooserEx::SaveLoadChooserEx(const String &title, const String &buttonLabel, bool saveMode, ScummEngine *engine)
	: Dialog(8, 8, engine->_system->getOverlayWidth() - 2 * 8, engine->_system->getOverlayHeight() - 16), _saveMode(saveMode), _list(0), _chooseButton(0), _gfxWidget(0), _scumm(engine) {
	
	new StaticTextWidget(this, 10, 6, _w - 2 * 10, kLineHeight, title, kTextAlignCenter);
	
	// Add choice list
	_list = new GUI::ListWidget(this, 10, 18, _w - 2 * 10 - 180, _h - 14 - 24 - 10);
	_list->setEditable(saveMode);
	_list->setNumberingMode(saveMode ? GUI::kListNumberingOne : GUI::kListNumberingZero);
	
	// Add the thumbnail display
	_gfxWidget = new GUI::GraphicsWidget(this,
			_w - (kThumbnailWidth + 22),
			18,
			kThumbnailWidth + 8,
			((_scumm->_system->getHeight() % 200 && _scumm->_system->getHeight() != 350) ? kThumbnailHeight2 : kThumbnailHeight1) + 8);
	_gfxWidget->setFlags(GUI::WIDGET_BORDER);
	
	// Buttons
	addButton(_w - 2 * (kButtonWidth + 10), _h - 24, "Cancel", kCloseCmd, 0);
	_chooseButton = addButton(_w-(kButtonWidth + 10), _h - 24, buttonLabel, kChooseCmd, 0);
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
	int ret = GUI::Dialog::runModal();
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
		const Graphics::Surface *thumb;
		thumb = _scumm->loadThumbnailFromSlot(_saveMode ? selItem + 1 : selItem);
		_gfxWidget->setGfx(thumb);
		delete thumb;
		_gfxWidget->draw();

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
		GUI::Dialog::handleCommand(sender, cmd, data);
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

enum {
	kRowHeight = 18,
	kBigButtonWidth = 90,
	kMainMenuWidth 	= (kBigButtonWidth + 2 * 8),
	kMainMenuHeight 	= 7 * kRowHeight + 3 * 5 + 7 + 5
};

#define addBigButton(label, cmd, hotkey) \
	new GUI::ButtonWidget(this, x, y, kBigButtonWidth, 16, label, cmd, hotkey); y += kRowHeight

MainMenuDialog::MainMenuDialog(ScummEngine *scumm)
	: ScummDialog(scumm, (320 - kMainMenuWidth) / 2, (200 - kMainMenuHeight)/2, kMainMenuWidth, kMainMenuHeight) {
	int y = 7;

	const int x = (_w - kBigButtonWidth) / 2;
	addBigButton("Resume", kPlayCmd, 'P');
	y += 5;

	addBigButton("Load", kLoadCmd, 'L');
	addBigButton("Save", kSaveCmd, 'S');
	y += 5;

	addBigButton("Options", kOptionsCmd, 'O');
#ifndef DISABLE_HELP
	addBigButton("Help", kHelpCmd, 'H');
#endif
	addBigButton("About", kAboutCmd, 'A');
	y += 5;

	addBigButton("Quit", kQuitCmd, 'Q');

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
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	idx = _saveDialog->runModal();
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
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

#ifndef _WIN32_WCE
ConfigDialog::ConfigDialog(ScummEngine *scumm)
	: GUI::OptionsDialog("", 40, 30, 240, 124), _vm(scumm) {
#else
ConfigDialog::ConfigDialog(ScummEngine *scumm)
	: GUI::OptionsDialog("", 40, 30, 240, 124 + 4), _vm(scumm) {
#endif
	//
	// Add the buttons
	//
#ifdef _WIN32_WCE
	addButton(_w - kButtonWidth - 8, _h - 24 - 4, "OK", GUI::OptionsDialog::kOKCmd, 'O');
	addButton(_w - 2 * kButtonWidth - 12, _h - 24 - 4, "Cancel", kCloseCmd, 'C');
	addButton(_w - 3 * kButtonWidth - 16, _h - 24 - 4, "Keys", kKeysCmd, 'K');
#else
	addButton(_w - kButtonWidth-8, _h - 24, "OK", GUI::OptionsDialog::kOKCmd, 'O');
	addButton(_w - 2 * kButtonWidth-12, _h - 24, "Cancel", kCloseCmd, 'C');
#endif

	//
	// Sound controllers
	//
	int yoffset = 8;
	yoffset = addVolumeControls(this, yoffset);

	//
	// Some misc options
	//
	subtitlesCheckbox = new GUI::CheckboxWidget(this, 15, 78, 200, 16, "Show subtitles", 0, 'S');
	speechCheckbox = new GUI::CheckboxWidget(this, 130, 78, 200, 16, "Enable speech", 0, 'E');

	//
	// Create the sub dialog(s)
	//
#ifdef _WIN32_WCE
	_keysDialog = new CEKeysDialog();
#endif
}

ConfigDialog::~ConfigDialog() {
#ifdef _WIN32_WCE
	delete _keysDialog;
#endif
}

void ConfigDialog::open() {
	GUI_OptionsDialog::open();

	// update checkboxes, too
	subtitlesCheckbox->setState(ConfMan.getBool("subtitles"));
	speechCheckbox->setState(!ConfMan.getBool("speech_mute"));
}

void ConfigDialog::close() {
	
	if (getResult()) {
		// Subtitles
		ConfMan.set("subtitles", subtitlesCheckbox->getState(), _domain);
		ConfMan.set("speech_mute", !speechCheckbox->getState(), _domain);
		// Sync with current setting
		if (ConfMan.getBool("speech_mute"))
			_vm->_voiceMode = 2;
		else
			_vm->_voiceMode = ConfMan.getBool("subtitles");

		if (_vm->_version >= 7)
			_vm->VAR(_vm->VAR_VOICE_MODE) = _vm->_voiceMode;
	}

	GUI_OptionsDialog::close();
	
	_vm->setupVolumes();
}


void ConfigDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kKeysCmd:
#ifdef _WIN32_WCE
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

	_page = 1;
	_numPages = ScummHelp::numPages(scumm->_gameId);

	_prevButton = addButton(10, 170, "Previous", kPrevCmd, 'P');
	_nextButton = addButton(90, 170, "Next", kNextCmd, 'N');
	addButton(210, 170, "Close", kCloseCmd, 'C');
	_prevButton->clearFlags(WIDGET_ENABLED);

	_title = new StaticTextWidget(this, 10, 5, 290, 16, "", kTextAlignCenter);
	for (int i = 0; i < HELP_NUM_LINES; i++) {
		_key[i] = new StaticTextWidget(this, 10, 18 + (10 * i), 80, 16, "", kTextAlignLeft);
		_dsc[i] = new StaticTextWidget(this, 90, 18 + (10 * i), 210, 16, "", kTextAlignLeft);
	}

	displayKeyBindings();
}

void HelpDialog::displayKeyBindings() {

	String titleStr, *keyStr, *dscStr;

	ScummHelp::updateStrings(_vm->_gameId, _vm->_version, _page, titleStr, keyStr, dscStr);

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
	int width = g_gui.getStringWidth(message) + 16;

	_x = (320 - width) / 2;
	_w = width;

	new StaticTextWidget(this, 4, 4, _w - 8, _h, message, kTextAlignCenter);
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

ConfirmDialog::ConfirmDialog(ScummEngine *scumm, const String& message)
	: InfoDialog(scumm, message) {
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

	int width = g_gui.getStringWidth(label) + 16 + kPercentBarWidth;

	_x = (320 - width) / 2;
	_w = width;
}

void ValueDisplayDialog::drawDialog() {
	g_gui.blendRect(_x, _y, _w, _h, g_gui._bgcolor);
	g_gui.box(_x, _y, _w, _h, g_gui._color, g_gui._shadowcolor);
	
	const int labelWidth = _w - 8 - kPercentBarWidth;

	// Draw the label
	g_gui.drawString(_label, _x + 4, _y + 4, labelWidth, g_gui._textcolor);
	
	// Draw the percentage bar
	g_gui.fillRect(_x + 4 + labelWidth, _y + 4, kPercentBarWidth * (_value - _min) / (_max - _min), 8, g_gui._textcolorhi);
	g_gui.frameRect(_x + 4 + labelWidth, _y + 4, kPercentBarWidth, 8, g_gui._textcolor);

	// Flag the draw area as dirty
	g_gui.addDirtyRect(_x, _y, _w, _h);
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

#ifdef __PALM_OS__
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
