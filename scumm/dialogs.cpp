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
	const char *str = queryResString(resID);
	if (!str)
		str = "Dummy!";
	new StaticTextWidget(this, x, y, w, h, str, kTextAlignCenter);
}


const char *ScummDialog::queryResString(int stringno)
{
	char *result;
	int string;

	if (stringno == 0)
		return NULL;

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
			return string_map_table_v6[stringno - 1].string;
		else
			return string_map_table_v5[stringno - 1].string;
	}

	return result;
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

	addButton(200, 20, 54, 16, RES_STRING(4), kSaveCmd, 'S');	// Save
	addButton(200, 40, 54, 16, RES_STRING(5), kLoadCmd, 'L');	// Load
	addButton(200, 60, 54, 16, RES_STRING(6), kPlayCmd, 'P');	// Play
	addButton(200, 80, 54, 16, CUSTOM_STRING(17), kOptionsCmd, 'O');	// Options
	addButton(200, 100, 54, 16, RES_STRING(8), kQuitCmd, 'Q');	// Quit
	
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
	case kListItemChangedCmd:
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
		Dialog::handleCommand(sender, cmd, data);
	}
}


#pragma mark -

enum {
	kSoundCmd = 'SOUN',
	kKeysCmd = 'KEYS',
	kAboutCmd = 'ABOU',
	kMiscCmd = 'OPTN'
};

OptionsDialog::OptionsDialog(NewGui *gui, Scumm *scumm)
	: ScummDialog(gui, scumm, 50, 80, 210, 60)
{
	addButton( 10, 10, 40, 16, CUSTOM_STRING(5), kSoundCmd, 'S');	// Sound
	addButton( 80, 10, 40, 16, CUSTOM_STRING(6), kKeysCmd, 'K');	// Keys
	addButton(150, 10, 40, 16, CUSTOM_STRING(7), kAboutCmd, 'A');	// About
	addButton( 10, 35, 40, 16, CUSTOM_STRING(18), kMiscCmd, 'M');	// Misc
	addButton(150, 35, 40, 16, CUSTOM_STRING(23), kCloseCmd, 'C');	// Close dialog - FIXME

	_aboutDialog = new AboutDialog(gui, scumm);
	_soundDialog = new SoundDialog(gui, scumm);
}

OptionsDialog::~OptionsDialog()
{
	delete _aboutDialog;
	delete _soundDialog;
}

void OptionsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data)
{
	switch (cmd) {
	case kSoundCmd:
		_soundDialog->open();
		break;
	case kKeysCmd:
		break;
	case kAboutCmd:
		_aboutDialog->open();
		break;
	case kMiscCmd:
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

#pragma mark -

AboutDialog::AboutDialog(NewGui *gui, Scumm *scumm)
	: ScummDialog(gui, scumm, 30, 20, 260, 124)
{
	addButton(110, 100, 40, 16, CUSTOM_STRING(23), kCloseCmd, 'C');	// Close dialog - FIXME
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

#pragma mark -

SoundDialog::SoundDialog(NewGui *gui, Scumm *scumm)
	: ScummDialog(gui, scumm, 30, 20, 260, 110)
{

	// set up dialog
	addButton(70, 90, 54, 16, "OK", kOKCmd, 'O');	// Confirm dialog
	addButton(136, 90, 54, 16, "Cancel", kCancelCmd, 'C');	// Abort dialog
	new StaticTextWidget(this, 20, 17, 85, 16, "Master volume:", kTextAlignRight);
	new StaticTextWidget(this, 20, 37, 85, 16, "Music volume:", kTextAlignRight);
	new StaticTextWidget(this, 20, 57, 85, 16, "SFX volume:", kTextAlignRight);

	masterVolumeSlider = new SliderWidget(this, 110, 13, 80, 16, "Volume1", kMasterVolumeChanged);
	musicVolumeSlider = new SliderWidget(this, 110, 33, 80, 16, "Volume2", kMusicVolumeChanged);
	sfxVolumeSlider = new SliderWidget(this, 110, 53, 80, 16, "Volume3", kSfxVolumeChanged);

	masterVolumeSlider->setMinValue(0);	masterVolumeSlider->setMaxValue(256);
	musicVolumeSlider->setMinValue(0);	musicVolumeSlider->setMaxValue(256);
	sfxVolumeSlider->setMinValue(0);	sfxVolumeSlider->setMaxValue(256);

	masterVolumeLabel = new StaticTextWidget(this, 195, 17, 60, 16, "Volume1", kTextAlignLeft);
	musicVolumeLabel = new StaticTextWidget(this, 195, 37, 60, 16, "Volume2", kTextAlignLeft);
	sfxVolumeLabel = new StaticTextWidget(this, 195, 57, 60, 16, "Volume3", kTextAlignLeft);
	
	masterVolumeLabel->setFlags(WIDGET_CLEARBG);
	musicVolumeLabel->setFlags(WIDGET_CLEARBG);
	sfxVolumeLabel->setFlags(WIDGET_CLEARBG);
}

void SoundDialog::open()
{
	Dialog::open();

	// get current variables
	_soundVolumeMaster = _scumm->_sound->_sound_volume_master;
	_soundVolumeMusic = _scumm->_sound->_sound_volume_music;
	_soundVolumeSfx = _scumm->_sound->_sound_volume_sfx;

	masterVolumeSlider->setValue(_soundVolumeMaster);
	musicVolumeSlider->setValue(_soundVolumeMusic);
	sfxVolumeSlider->setValue(_soundVolumeSfx);

	masterVolumeLabel->setValue(_soundVolumeMaster);
	musicVolumeLabel->setValue(_soundVolumeMusic);
	sfxVolumeLabel->setValue(_soundVolumeSfx);
}


void SoundDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data)
{
	switch (cmd) {
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
		// FIXME: Look at Fingolfins comments in Gui::handleSoundDialogCommand(), gui.cpp 
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
		g_config->flush();
		}
	case kCancelCmd:
		close();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}
