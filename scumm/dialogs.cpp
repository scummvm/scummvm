/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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
#include "sound/mididrv.h"
#include "scumm.h"
#include "imuse.h"

#include "gui/newgui.h"
#include "gui/ListWidget.h"
#include "common/config-file.h"


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
	"Pocket ScummVM",	//8
	"Build " SCUMMVM_VERSION " (" SCUMMVM_CVS ")",	//9
	"ScummVM http://scummvm.sourceforge.net",		//10
	"All games (c) LucasArts",						//11
	"Quit",											//12
	"Pause",										//13
	"Save",											//14
	"Skip",											//15
	"Hide",											//16
	"Options",									//17
	"Misc",											//18
	"Show speech subtitles",		//19
	"Amiga palette conversion",	//20
	"Except:",									//21
	"Simon the Sorcerer (c) Adventuresoft", //22
	"Close"											//23
};

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
	{101, "Ok"}, 
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
	{12, "Ok"},
	{4, "Game paused"}
};


#pragma mark -


void ScummDialog::addResText(int x, int y, int w, int h, int resID)
{
	// Get the string
	new StaticTextWidget(this, x, y, w, h, queryResString(resID), kTextAlignCenter);
}


const ScummVM::String ScummDialog::queryResString(int stringno)
{
	char *result;
	int string;

	if (stringno == 0)
		return String();

	if (_scumm->_features & GF_AFTER_V7)
		string = _scumm->_vars[string_map_table_v7[stringno - 1].num];
	else if (_scumm->_features & GF_AFTER_V6)
		string = _scumm->_vars[string_map_table_v6[stringno - 1].num];
	else
		string = string_map_table_v5[stringno - 1].num;

	result = (char *)_scumm->getStringAddress(string);
	if (result && *result == '/') {
		_scumm->translateText((char*)result, (char*)&_scumm->transText);
		strcpy((char*)result, (char*)&_scumm->transText);
	}

	if (!result) {								// Gracelessly degrade to english :)
		if (_scumm->_features & GF_AFTER_V6)
			result = string_map_table_v6[stringno - 1].string;
		else
			result = string_map_table_v5[stringno - 1].string;
	}

	// Convert to a proper string (take care of FF codes)
	int value;
	byte chr;
	String tmp;

	while ((chr = *result++)) {		
		if (chr == 0xFF) {
			chr = *result++;			
			switch (chr) {
			case 4: { // add value
				value = _scumm->readVar(READ_LE_UINT16(result));
				if (value < 0) {
					tmp += '-';
					value = -value;
				}

				int flag = 0;
				int max = 10000;
				do {
					if (value >= max || flag) {
						tmp += value / max + '0';
						value %= max;
						flag = 1;
					}
					max /= 10;
					if (max == 1)
						flag = 1;
				} while (max);
				result += 2;
				break;
			}

			case 5: { //add verb
				value = _scumm->readVar(READ_LE_UINT16(result));
				int i;
				if (!value)
					break;
				
				for (i = 1; i < _scumm->_maxVerbs; i++) {
					if (value == _scumm->_verbs[i].verbid && !_scumm->_verbs[i].type && !_scumm->_verbs[i].saveid) {
						char* verb = (char*)_scumm->getResourceAddress(rtVerb, i);
						if (verb) {
							tmp += verb;
						}
						break;
					}
				}
				result += 2;
				break;
			}

			case 6: { // add object or actor name
				value = _scumm->readVar(READ_LE_UINT16(result));
				if (!value)
					break;

				char* name = (char*)_scumm->getObjOrActorName(value);
				if (name) {
					tmp += name;
				}
				result += 2;
				break;
			}
			case 7: { // add string
				value = READ_LE_UINT16(result);
				if (_scumm->_features & GF_AFTER_V6 || _scumm->_gameId == GID_INDY3_256)			
					value = _scumm->readVar(value);

				if (value) {
					char *str = (char*)_scumm->getStringAddress(value);
					if (str) {
						tmp += str;
					}
				}
				result += 2;
				break;
			}
				// Do these ever occur in the Gui?
			case 9:
			case 10:
			case 12:
			case 13:
			case 14:				
				result += 2;				
			default:
				warning("Ignoring unknown resource string of type %d", (int)chr);
			}
		} else {
			if (chr != '@') {
				tmp += chr;
			}
		}
	}
	return tmp;
}

const char *ScummDialog::queryCustomString(int stringno)
{
	return string_map_table_custom[stringno];
}


#pragma mark -

enum {
	kSaveCmd = 'SAVE',
	kLoadCmd = 'LOAD',
	kPlayCmd = 'PLAY',
	kOptionsCmd = 'OPTN',
	kQuitCmd = 'QUIT'
};

/*
 * TODO
 * - Maybe go back to the old way of differentiating between the save and the load mode?
 *   This would include that in the load mode the list is not editable.
 * - Currently the savegame list is only loaded once when the dialog is created. Instead,
 *   it should be loaded whenever the dialog is opened. Might want to add an open()
 *   method to Dialog for that.
 */

SaveLoadDialog::SaveLoadDialog(NewGui *gui, Scumm *scumm)
	: ScummDialog(gui, scumm, 30, 20, 260, 124)
{
	addResText(10, 7, 240, 16, 1);
//  addResText(10, 7, 240, 16, 2);
//  addResText(10, 7, 240, 16, 3);

	addButton(200, 20, queryResString(4), kSaveCmd, 'S');	// Save
	addButton(200, 40, queryResString(5), kLoadCmd, 'L');	// Load
	addButton(200, 60, queryResString(6), kPlayCmd, 'P');	// Play
	addButton(200, 80, queryCustomString(17), kOptionsCmd, 'O');	// Options
	addButton(200, 100, queryResString(8), kQuitCmd, 'Q');	// Quit
	
	_savegameList = new ListWidget(this, 10, 20, 180, 90);
	_savegameList->setNumberingMode(kListNumberingZero);
	
	// Get savegame names
	ScummVM::StringList l;
	char name[32];

	for (int i = 0; i <= 80; i++) {		// 80 - got this value from the old GUI
		_scumm->getSavegameName(i, name);
		l.push_back(name);
	}

	_savegameList->setList(l);
}

void SaveLoadDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data)
{
	switch (cmd) {
	case kListItemActivatedCmd:
	case kSaveCmd:
		if (_savegameList->getSelected() >= 1 && !_savegameList->getSelectedString().isEmpty()) {
			_scumm->_saveLoadSlot = _savegameList->getSelected();
			_scumm->_saveLoadCompatible = false;
			_scumm->_saveLoadFlag = 1;		// 1 for save, I assume (Painelf)
			strcpy(_scumm->_saveLoadName, _savegameList->getSelectedString().c_str());
			close();
		}
		break;
	case kListItemDoubleClickedCmd:
	case kLoadCmd:
		if (_savegameList->getSelected() >= 0 && !_savegameList->getSelectedString().isEmpty()) {
			_scumm->_saveLoadSlot = _savegameList->getSelected();
			_scumm->_saveLoadCompatible = false;
			_scumm->_saveLoadFlag = 2;		// 2 for load. Magic number anyone?
			close();
		}
		break;
	case kPlayCmd:
		close();
		break;
	case kOptionsCmd:
		_scumm->optionsDialog();
		break;
	case kQuitCmd:
		_scumm->_system->quit();
		break;
	default:
		ScummDialog::handleCommand(sender, cmd, data);
	}
}


#pragma mark -

enum {
	kMasterVolumeChanged	= 'mavc',
	kMusicVolumeChanged		= 'muvc',
	kSfxVolumeChanged		= 'sfvc',
	kOKCmd					= 'ok  ',
	kCancelCmd				= 'cncl',
};

enum {
	kKeysCmd = 'KEYS',
	kAboutCmd = 'ABOU',
};

OptionsDialog::OptionsDialog(NewGui *gui, Scumm *scumm)
	: ScummDialog(gui, scumm, 30, 20, 260, 124)
{
	//
	// Add the buttons
	//
	addButton(_w-kButtonWidth-8, _h-24, "OK", kOKCmd, 'O');
	addButton(_w-2*kButtonWidth-12, _h-24, "Cancel", kCancelCmd, 'C');

	addButton(8, _h-24, "About", kAboutCmd, 'A');
#ifdef _WIN32_WCE
	addButton(kButtonWidth+12, _h-24, "Keys", kKeysCmd, 'K');
#endif


	//
	// Sound controllers
	//
	new StaticTextWidget(this, 25, 10, 85, 16, "Master volume:", kTextAlignRight);
	new StaticTextWidget(this, 25, 26, 85, 16, "Music volume:", kTextAlignRight);
	new StaticTextWidget(this, 25, 42, 85, 16, "SFX volume:", kTextAlignRight);

	masterVolumeSlider = new SliderWidget(this, 115, 8, 80, 12, "Volume1", kMasterVolumeChanged);
	musicVolumeSlider  = new SliderWidget(this, 115, 24, 80, 12, "Volume2", kMusicVolumeChanged);
	sfxVolumeSlider    = new SliderWidget(this, 115, 40, 80, 12, "Volume3", kSfxVolumeChanged);

	masterVolumeSlider->setMinValue(0);	masterVolumeSlider->setMaxValue(256);
	musicVolumeSlider->setMinValue(0);	musicVolumeSlider->setMaxValue(256);
	sfxVolumeSlider->setMinValue(0);	sfxVolumeSlider->setMaxValue(256);

	masterVolumeLabel = new StaticTextWidget(this, 200, 10, 60, 16, "Volume1", kTextAlignLeft);
	musicVolumeLabel  = new StaticTextWidget(this, 200, 26, 60, 16, "Volume2", kTextAlignLeft);
	sfxVolumeLabel    = new StaticTextWidget(this, 200, 42, 60, 16, "Volume3", kTextAlignLeft);
	
	masterVolumeLabel->setFlags(WIDGET_CLEARBG);
	musicVolumeLabel->setFlags(WIDGET_CLEARBG);
	sfxVolumeLabel->setFlags(WIDGET_CLEARBG);

	//
	// Some misc options
	//
	subtitlesCheckbox = new CheckboxWidget(this, 25, 62, 100, 16, "Show subtitles", 0, 'S');
	amigaPalCheckbox  = new CheckboxWidget(this, 25, 80, 100, 16, "Amiga palette conversion", 0, 'P');


	//
	// Finally create the sub dialogs
	//
	_aboutDialog = new AboutDialog(gui, scumm);
#ifdef _WIN32_WCE
	// TODO - create _keysDialog
#endif
}

OptionsDialog::~OptionsDialog()
{
	delete _aboutDialog;
}

void OptionsDialog::open()
{
	ScummDialog::open();

	// display current sound settings
	_soundVolumeMaster = _scumm->_sound->_sound_volume_master;
	_soundVolumeMusic = _scumm->_sound->_sound_volume_music;
	_soundVolumeSfx = _scumm->_sound->_sound_volume_sfx;

	masterVolumeSlider->setValue(_soundVolumeMaster);
	musicVolumeSlider->setValue(_soundVolumeMusic);
	sfxVolumeSlider->setValue(_soundVolumeSfx);

	masterVolumeLabel->setValue(_soundVolumeMaster);
	musicVolumeLabel->setValue(_soundVolumeMusic);
	sfxVolumeLabel->setValue(_soundVolumeSfx);

	// update checkboxes, too
	subtitlesCheckbox->setState(_scumm->_noSubtitles == false);
	amigaPalCheckbox->setState(_scumm->_features & GF_AMIGA);
}

void OptionsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data)
{
	switch (cmd) {
	case kKeysCmd:
		// TODO
		break;
	case kAboutCmd:
		_aboutDialog->open();
		break;
	case kMasterVolumeChanged:
		_soundVolumeMaster = masterVolumeSlider->getValue();
		masterVolumeLabel->setValue(_soundVolumeMaster);
		masterVolumeLabel->draw();
		break;
	case kMusicVolumeChanged:
		_soundVolumeMusic = musicVolumeSlider->getValue();
		musicVolumeLabel->setValue(_soundVolumeMusic);
		musicVolumeLabel->draw();
		break;
	case kSfxVolumeChanged:
		_soundVolumeSfx = sfxVolumeSlider->getValue();
		sfxVolumeLabel->setValue(_soundVolumeSfx);
		sfxVolumeLabel->draw();
		break;
	case kOKCmd: {
		// Update the sound settings 
		_scumm->_sound->_sound_volume_master = _soundVolumeMaster;	// Master
		_scumm->_sound->_sound_volume_music = _soundVolumeMusic;	// Music
		_scumm->_sound->_sound_volume_sfx = _soundVolumeSfx;	// SFX
		
		_scumm->_imuse->set_music_volume(_soundVolumeMusic);
		_scumm->_imuse->set_master_volume(_soundVolumeMaster);
		_scumm->_mixer->setVolume(_soundVolumeSfx);
		_scumm->_mixer->setMusicVolume(_soundVolumeMusic);
		
		g_config->setInt("master_volume", _soundVolumeMaster);
		g_config->setInt("music_volume", _soundVolumeMusic);
		g_config->setInt("sfx_volume", _soundVolumeSfx);

		// Subtitles?
		_scumm->_noSubtitles = !subtitlesCheckbox->getState();
		g_config->setBool("nosubtitles", _scumm->_noSubtitles);
		
		// Amiga palette?
		if (amigaPalCheckbox->getState())
			_scumm->_features |= GF_AMIGA;
		else
			_scumm->_features &= ~GF_AMIGA;
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
	: ScummDialog(gui, scumm, 30, 20, 260, 124)
{
	addButton(110, 100, queryCustomString(23), kCloseCmd, 'C');	// Close dialog - FIXME
	new StaticTextWidget(this, 10, 10, 240, 16, "ScummVM " SCUMMVM_VERSION " (" SCUMMVM_CVS ")", kTextAlignCenter);
	new StaticTextWidget(this, 10, 30, 240, 16, "http://scummvm.sourceforge.net", kTextAlignCenter);
	new StaticTextWidget(this, 10, 50, 240, 16, "All games (c) LucasArts", kTextAlignCenter);
	new StaticTextWidget(this, 10, 64, 240, 16, "Except", kTextAlignCenter);
	new StaticTextWidget(this, 10, 78, 240, 16, "Simon the Sorcerer (c) Adventuresoft", kTextAlignCenter);
}

#pragma mark -

PauseDialog::PauseDialog(NewGui *gui, Scumm *scumm)
	: ScummDialog(gui, scumm, 35, 80, 250, 16)
{
	addResText(4, 4, 250-8, 16, 10);
}
