/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
#include "dialogs.h"
#include "sound.h"
#include "scumm.h"
#include "imuse.h"
#include "player_v2.h"
#include "verbs.h"
#include "sound/mididrv.h"
#include "sound/mixer.h"

#ifndef DISABLE_HELP
#include "help.h"
#endif

#include "gui/newgui.h"
#include "gui/ListWidget.h"
#include "common/config-file.h"

#ifdef _WIN32_WCE
#include "gapi_keys.h"
extern bool _get_key_mapping;
extern void force_keyboard(bool);
extern void save_key_mapping();
extern void load_key_mapping();
#endif


#ifdef _MSC_VER
#	pragma warning( disable : 4068 )
#endif

struct ResString {
	int num;
	char string[80];
};

// String maps
static const char* string_map_table_custom[] = { 
	"Master Volume :",	//0
	"Music Volume :",	//1
	"SFX Volume :",		//2
	"+",				//3
	"-",				//4
	"Sound",			//5
	"Keys",				//6
	"About",			//7
	"Pocket ScummVM",		//8
	"This space intentionally left blank",	//9		SPARE
	"Do you have a monkey in your pocket?",	//10		SPARE
	"",					//11		SPARE
	"Quit",					//12
	"Pause",				//13
	"Save",					//14
	"Skip",					//15
	"Hide",					//16
	"Options",				//17
	"Misc",					//18
	"Show speech subtitles",		//19
	"Amiga palette conversion",		//20
	"If you can read this,",		//21		SPARE
	"you don't need glasses", 		//22		SPARE
	"Close",				//23
	"Map",					//24
	"Choose an action to map",		//25
	"Press the key to associate",		//26
	"Please select an action",		//27
	"Help"					//28
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


void ScummDialog::addResText(int x, int y, int w, int h, int resID) {
	// Get the string
	new StaticTextWidget(this, x, y, w, h, queryResString(resID), kTextAlignCenter);
}


const ScummVM::String ScummDialog::queryResString(int stringno) {
	byte *result;

	if (stringno == 0)
		return String();

	if (_scumm->_version >= 7)
		result = _scumm->getStringAddressVar(string_map_table_v7[stringno - 1].num);
	else if (_scumm->_version == 6)
		result = _scumm->getStringAddressVar(string_map_table_v6[stringno - 1].num);
	else if (_scumm->_version == 5)
		result = _scumm->getStringAddress(string_map_table_v5[stringno - 1].num);
	else
		// TODO: For V8 games, maybe grab the strings from the language file?
		return string_map_table_v5[stringno - 1].string;

	if (result && *result == '/') {
		byte tmp[256];
		_scumm->translateText(result, tmp);
		
		// FIXME: AARGH! We shouldn't just strcpy into the data we got from getStringAddress
		strcpy((char *)result, (char *)tmp);
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

const char *ScummDialog::queryCustomString(int stringno) {
	return string_map_table_custom[stringno];
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

SaveLoadDialog::SaveLoadDialog(NewGui *gui, Scumm *scumm)
	: ScummDialog(gui, scumm, 20, 8, 280, 184) {
	const int x = _w - kButtonWidth - 8;
	int y = 20;

	// The headline
	addResText(0, 7, 260, 16, 1);

	// The five buttons on the side
	_saveButton = addPushButton(x, y, queryResString(4), kSaveCmd, 'S'); y += 20;
	_loadButton = addPushButton(x, y, queryResString(5), kLoadCmd, 'L'); y += 20;
	y += 5;

	addButton(x, y, "About", kAboutCmd, 'A'); y += 20;	// About
#ifndef DISABLE_HELP
	addButton(x, y, queryCustomString(28), kHelpCmd, 'H'); y += 20;	// Help
#endif
	addButton(x, y, queryCustomString(17), kOptionsCmd, 'O'); y += 20;	// Options
	y += 5;

	addButton(x, y, queryResString(6), kPlayCmd, 'P'); y += 20;	// Play
	addButton(x, y, queryResString(8), kQuitCmd, 'Q'); y += 20;	// Quit

	//
	// Create the sub dialog(s)
	//
	_aboutDialog = new AboutDialog(gui, scumm);
#ifndef DISABLE_HELP
	_helpDialog = new HelpDialog(gui, scumm);
#endif

	// The save game list
	_savegameList = new ListWidget(this, 8, 20, x - 14, 156);
}

SaveLoadDialog::~SaveLoadDialog() {
	delete _aboutDialog;
#ifndef DISABLE_HELP
	delete _helpDialog;
#endif
}

void SaveLoadDialog::open() {
	switchToLoadMode();

#ifdef _WIN32_WCE
	force_keyboard(true);
#endif

	ScummDialog::open();
}

void SaveLoadDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kSaveCmd:
		if (!_saveMode) {
			switchToSaveMode();
		}
		break;
	case kLoadCmd:
		if (_saveMode) {
			switchToLoadMode();
		}
		break;
	case kListItemDoubleClickedCmd:
		if (_savegameList->getSelected() >= 0) {
			if (_saveMode) {
				if (_savegameList->getSelectedString().isEmpty()) {
					// Start editing the selected item, for saving
					_savegameList->startEditMode();
				} else {
					save();
				}
			} else if (!_savegameList->getSelectedString().isEmpty()) {
				load();
			}
		}
		break;
	case kListItemActivatedCmd:
		if (_savegameList->getSelected() >= 0 && !_savegameList->getSelectedString().isEmpty()) {
			if (_saveMode) {
				save();
			} else {
				load();
			}
		}
		break;
	case kListSelectionChangedCmd:
		if (_saveMode) {
			_savegameList->startEditMode();
		}
		break;
	case kPlayCmd:
		close();
		break;
	case kOptionsCmd:
		_scumm->optionsDialog();
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
		_scumm->_quit = true;
		close();
		break;
	default:
		ScummDialog::handleCommand(sender, cmd, data);
	}
}

void SaveLoadDialog::close() {
	ScummDialog::close();

#ifdef _WIN32_WCE
	force_keyboard(false);
#endif
}

void SaveLoadDialog::fillList() {
	// Get savegame names
	ScummVM::StringList l;
	char name[32];
	uint i = _saveMode ? 1 : 0;
	bool avail_saves[81];

	SaveFileManager *mgr = _scumm->_system->get_savefile_manager();

	_scumm->listSavegames(avail_saves, ARRAYSIZE(avail_saves), mgr);
	for (; i < ARRAYSIZE(avail_saves); i++) {
		if (avail_saves[i])
			_scumm->getSavegameName(i, name, mgr);
		else
			name[0] = 0;
		l.push_back(name);
	}

	delete mgr;

	_savegameList->setList(l);
	_savegameList->setNumberingMode(_saveMode ? kListNumberingOne : kListNumberingZero);
}

void SaveLoadDialog::save() {
	// Save the selected item
	_scumm->requestSave(_savegameList->getSelected() + 1, _savegameList->getSelectedString().c_str());
	close();
}

void SaveLoadDialog::load() {
	// Load the selected item
	_scumm->requestLoad(_savegameList->getSelected());
	close();
}

void SaveLoadDialog::switchToSaveMode() {
	_saveMode = true;
	_saveButton->setState(true);
	_loadButton->setState(false);
	_saveButton->clearFlags(WIDGET_ENABLED);
	_loadButton->setFlags(WIDGET_ENABLED);
	_savegameList->setEditable(true);
	fillList();
	draw();
}

void SaveLoadDialog::switchToLoadMode() {
	_saveMode = false;
	_saveButton->setState(false);
	_loadButton->setState(true);
	_saveButton->setFlags(WIDGET_ENABLED);
	_loadButton->clearFlags(WIDGET_ENABLED);
	_savegameList->setEditable(false);
	fillList();
	draw();
}

#pragma mark -

enum {
	kMasterVolumeChanged	= 'mavc',
	kMusicVolumeChanged		= 'muvc',
	kSfxVolumeChanged		= 'sfvc',
	kOKCmd					= 'ok  ',
	kCancelCmd				= 'cncl'
};

enum {
	kKeysCmd = 'KEYS'
};

#ifndef _WIN32_WCE
OptionsDialog::OptionsDialog(NewGui *gui, Scumm *scumm)
	: ScummDialog(gui, scumm, 40, 30, 240, 124) {
#else
OptionsDialog::OptionsDialog(NewGui *gui, Scumm *scumm)
	: ScummDialog(gui, scumm, 40, 30, 240, 124 + kButtonHeight + 4) {
#endif
	//
	// Add the buttons
	//
#ifdef _WIN32_WCE
	addButton(_w-kButtonWidth-8, _h-24 - kButtonHeight - 4, "OK", kOKCmd, 'O');
	addButton(_w-2*kButtonWidth-12, _h-24 - kButtonHeight - 4, "Cancel", kCancelCmd, 'C');

	addButton(kButtonWidth+12, _h-24, "Keys", kKeysCmd, 'K');
#else
	addButton(_w-kButtonWidth-8, _h-24, "OK", kOKCmd, 'O');
	addButton(_w-2*kButtonWidth-12, _h-24, "Cancel", kCancelCmd, 'C');
#endif

	//
	// Sound controllers
	//
	new StaticTextWidget(this, 15, 10, 95, 16, "Master volume:", kTextAlignRight);
	new StaticTextWidget(this, 15, 26, 95, 16, "Music volume:", kTextAlignRight);
	new StaticTextWidget(this, 15, 42, 95, 16, "SFX volume:", kTextAlignRight);

	_masterVolumeSlider = new SliderWidget(this, 125, 8, 80, 12, "Volume1", kMasterVolumeChanged);
	_musicVolumeSlider  = new SliderWidget(this, 125, 24, 80, 12, "Volume2", kMusicVolumeChanged);
	_sfxVolumeSlider    = new SliderWidget(this, 125, 40, 80, 12, "Volume3", kSfxVolumeChanged);

	_masterVolumeSlider->setMinValue(0);	_masterVolumeSlider->setMaxValue(255);
	_musicVolumeSlider->setMinValue(0);	_musicVolumeSlider->setMaxValue(255);
	_sfxVolumeSlider->setMinValue(0);	_sfxVolumeSlider->setMaxValue(255);

	_masterVolumeLabel = new StaticTextWidget(this, 210, 10, 24, 16, "Volume1", kTextAlignLeft);
	_musicVolumeLabel  = new StaticTextWidget(this, 210, 26, 24, 16, "Volume2", kTextAlignLeft);
	_sfxVolumeLabel    = new StaticTextWidget(this, 210, 42, 24, 16, "Volume3", kTextAlignLeft);
	
	_masterVolumeLabel->setFlags(WIDGET_CLEARBG);
	_musicVolumeLabel->setFlags(WIDGET_CLEARBG);
	_sfxVolumeLabel->setFlags(WIDGET_CLEARBG);

	//
	// Some misc options
	//
	subtitlesCheckbox = new CheckboxWidget(this, 15, 62, 200, 16, "Show subtitles", 0, 'S');
	amigaPalCheckbox  = new CheckboxWidget(this, 15, 80, 200, 16, "Amiga palette conversion", 0, 'P');

	//
	// Create the sub dialog(s)
	//
#ifdef _WIN32_WCE
	_keysDialog = new KeysDialog(gui, scumm);
#endif
}

OptionsDialog::~OptionsDialog() {
#ifdef _WIN32_WCE
	delete _keysDialog;
#endif
}

void OptionsDialog::open() {
	ScummDialog::open();

	// display current sound settings
	_soundVolumeMaster = _scumm->_sound->_sound_volume_master;
	_soundVolumeMusic = _scumm->_sound->_sound_volume_music;
	_soundVolumeSfx = _scumm->_sound->_sound_volume_sfx;

	_masterVolumeSlider->setValue(_soundVolumeMaster);
	_musicVolumeSlider->setValue(_soundVolumeMusic);
	_sfxVolumeSlider->setValue(_soundVolumeSfx);

	_masterVolumeLabel->setValue(_soundVolumeMaster);
	_musicVolumeLabel->setValue(_soundVolumeMusic);
	_sfxVolumeLabel->setValue(_soundVolumeSfx);

	// update checkboxes, too
	subtitlesCheckbox->setState(_scumm->_noSubtitles == false);
	amigaPalCheckbox->setState((_scumm->_features & GF_AMIGA) != 0);
}

void OptionsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kKeysCmd:
#ifdef _WIN32_WCE
		_keysDialog->runModal();
#endif
		break;
	case kMasterVolumeChanged:
		_soundVolumeMaster = _masterVolumeSlider->getValue();
		_masterVolumeLabel->setValue(_soundVolumeMaster);
		_masterVolumeLabel->draw();
		break;
	case kMusicVolumeChanged:
		_soundVolumeMusic = _musicVolumeSlider->getValue();
		_musicVolumeLabel->setValue(_soundVolumeMusic);
		_musicVolumeLabel->draw();
		break;
	case kSfxVolumeChanged:
		_soundVolumeSfx = _sfxVolumeSlider->getValue();
		_sfxVolumeLabel->setValue(_soundVolumeSfx);
		_sfxVolumeLabel->draw();
		break;
	case kOKCmd: {
		// Update the sound settings 
		_scumm->_sound->_sound_volume_master = _soundVolumeMaster;	// Master
		_scumm->_sound->_sound_volume_music = _soundVolumeMusic;	// Music
		_scumm->_sound->_sound_volume_sfx = _soundVolumeSfx;	// SFX
		
		if (_scumm->_imuse) {
			_scumm->_imuse->set_music_volume(_soundVolumeMusic);
			_scumm->_imuse->set_master_volume(_soundVolumeMaster);
		}
		if (_scumm->_playerV2) {
			_scumm->_playerV2->set_master_volume(_soundVolumeMaster);
		}

		_scumm->_mixer->setVolume(_soundVolumeSfx * _soundVolumeMaster / 255);
		_scumm->_mixer->setMusicVolume(_soundVolumeMusic);
		
		g_config->setInt("master_volume", _soundVolumeMaster);
		g_config->setInt("music_volume", _soundVolumeMusic);
		g_config->setInt("sfx_volume", _soundVolumeSfx);

		// Subtitles?
		_scumm->_noSubtitles = !subtitlesCheckbox->getState();
		g_config->setBool("nosubtitles", _scumm->_noSubtitles);
		
		// Amiga palette?
		if (amigaPalCheckbox->getState())
			_scumm->setFeatures (_scumm->_features | GF_AMIGA);
		else
			_scumm->setFeatures (_scumm->_features & (~GF_AMIGA));

		g_config->setBool("amiga", amigaPalCheckbox->getState());
		
		// Finally flush the modified config
		g_config->flush();
		}
	case kCancelCmd:
		close();
		break;
	default:
		ScummDialog::handleCommand(sender, cmd, data);
	}
}

#pragma mark -

AboutDialog::AboutDialog(NewGui *gui, Scumm *scumm)
	: ScummDialog(gui, scumm, 30, 20, 260, 124) {
	addButton((_w - kButtonWidth)/2, 100, queryCustomString(23), kCloseCmd, 'C');	// Close dialog - FIXME
	new StaticTextWidget(this, 10, 10, 240, 16, gScummVMFullVersion, kTextAlignCenter);
//	new StaticTextWidget(this, 10, 20, 240, 16, "(built on " __DATE__ ")", kTextAlignCenter);

	new StaticTextWidget(this, 10, 30, 240, 16, "http://www.scummvm.org", kTextAlignCenter);
	new StaticTextWidget(this, 10, 50, 240, 16, "SCUMM Games (c) LucasArts", kTextAlignCenter);
	new StaticTextWidget(this, 10, 64, 240, 16, "Simon the Sorcerer (c) Adventuresoft", kTextAlignCenter);
	new StaticTextWidget(this, 10, 78, 240, 16, "Beneath a Steel Sky (c) Revolution", kTextAlignCenter);
}

#ifndef DISABLE_HELP

#pragma mark -

enum {
	kNextCmd = 'NEXT',
	kPrevCmd = 'PREV'
};

HelpDialog::HelpDialog(NewGui *gui, Scumm *scumm)
	: ScummDialog(gui, scumm, 5, 5, 310, 190) {

	_page = 1;
	_numPages = ScummHelp::numPages(scumm->_gameId);

	_prevButton = addPushButton(10, 170, "Previous", kPrevCmd, 'P');
	_nextButton = addPushButton(90, 170, "Next", kNextCmd, 'N');
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

	ScummHelp::updateStrings(_scumm->_gameId, _scumm->_version, _page, titleStr, keyStr, dscStr);

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

InfoDialog::InfoDialog(NewGui *gui, Scumm *scumm, int res)
: ScummDialog(gui, scumm, 0, 80, 0, 16) { // dummy x and w
	setInfoText(queryResString (res));
}

InfoDialog::InfoDialog(NewGui *gui, Scumm *scumm, const String& message)
: ScummDialog(gui, scumm, 0, 80, 0, 16) { // dummy x and w
	setInfoText(message);
}

void InfoDialog::setInfoText(const String& message) {
	int width = _gui->getStringWidth(message.c_str()) + 16;

	_x = (_scumm->_screenWidth - width) >> 1;
	_w = width;

	new StaticTextWidget(this, 4, 4, _w-8, _h, message, kTextAlignCenter);
}

#pragma mark -

PauseDialog::PauseDialog(NewGui *gui, Scumm *scumm)
	: InfoDialog(gui, scumm, 10) {
}

#ifdef _WIN32_WCE

#pragma mark -

enum {
	kMapCmd					= 'map '
};


KeysDialog::KeysDialog(NewGui *gui, Scumm *scumm)
	: ScummDialog(gui, scumm, 30, 20, 260, 160) {
	addButton(160, 20, queryCustomString(24), kMapCmd, 'M');	// Map
	addButton(160, 40, "OK", kOKCmd, 'O');						// OK
	addButton(160, 60, "Cancel", kCancelCmd, 'C');				// Cancel

	_actionsList = new ListWidget(this, 10, 20, 140, 90);
	_actionsList->setNumberingMode(kListNumberingZero);

	_actionTitle = new StaticTextWidget(this, 10, 120, 240, 16, queryCustomString(25), kTextAlignCenter);
	_keyMapping = new StaticTextWidget(this, 10, 140, 240, 16, "", kTextAlignCenter);

	_actionTitle->setFlags(WIDGET_CLEARBG);
	_keyMapping->setFlags(WIDGET_CLEARBG);

	// Get actions names
	ScummVM::StringList l;

	for (int i = 1; i < TOTAL_ACTIONS; i++) 
		l.push_back(getActionName(i));

	_actionsList->setList(l);

	_actionSelected = -1;
	_get_key_mapping = false;
}

void KeysDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch(cmd) {

	case kListSelectionChangedCmd:
		if (_actionsList->getSelected() >= 0) {
				char selection[100];

				sprintf(selection, "Associated key : %s", getGAPIKeyName((unsigned int)getAction(_actionsList->getSelected() + 1)->action_key));				
				_keyMapping->setLabel(selection);
				_keyMapping->draw();
		}
		break;
	case kMapCmd:
		if (_actionsList->getSelected() < 0) {
				_actionTitle->setLabel(queryCustomString(27));
		}
		else {
				char selection[100];

				_actionSelected = _actionsList->getSelected() + 1;
				sprintf(selection, "Associated key : %s", getGAPIKeyName((unsigned int)getAction(_actionSelected)->action_key));				
				_actionTitle->setLabel(queryCustomString(26));
				_keyMapping->setLabel(selection);
				_keyMapping->draw();
				_get_key_mapping = true;
				_actionsList->setEnabled(false);
		}
		_actionTitle->draw();
		break;
	case kOKCmd:
		save_key_mapping();
		close();
		break;
	case kCancelCmd:
		load_key_mapping();
		close();
		break;
	default:
		ScummDialog::handleCommand(sender, cmd, data);
	}
}

void KeysDialog::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	if (modifiers == 0xff  && _get_key_mapping) {
		// GAPI key was selected
		char selection[100];

		clearActionKey(ascii & 0xff);
		getAction(_actionSelected)->action_key = (ascii & 0xff);
		sprintf(selection, "Associated key : %s", getGAPIKeyName((unsigned int)getAction(_actionSelected)->action_key));				
		_actionTitle->setLabel(queryCustomString(25));
		_keyMapping->setLabel(selection);
		_keyMapping->draw();
		_actionSelected = -1;
		_actionsList->setEnabled(true);
		_get_key_mapping = false;
	}
}

#endif

#ifdef __PALM_OS__
#include "scumm_globals.h" // init globals
void Dialogs_initGlobals()		{
	GSETPTR(string_map_table_v7, GBVARS_STRINGMAPTABLEV7_INDEX, ResString, GBVARS_SCUMM)
	GSETPTR(string_map_table_v6, GBVARS_STRINGMAPTABLEV6_INDEX, ResString, GBVARS_SCUMM)
	GSETPTR(string_map_table_v5, GBVARS_STRINGMAPTABLEV5_INDEX, ResString, GBVARS_SCUMM)
}
void Dialogs_releaseGlobals()	{
	GRELEASEPTR(GBVARS_STRINGMAPTABLEV7_INDEX, GBVARS_SCUMM)
	GRELEASEPTR(GBVARS_STRINGMAPTABLEV6_INDEX, GBVARS_SCUMM)
	GRELEASEPTR(GBVARS_STRINGMAPTABLEV5_INDEX, GBVARS_SCUMM)
}
#endif
