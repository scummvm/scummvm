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
#include "browser.h"
#include "chooser.h"
#include "newgui.h"
#include "options.h"
#include "PopUpWidget.h"

#include "backends/fs/fs.h"
#include "common/config-file.h"
#include "common/gameDetector.h"

#if (!( defined(__DC__) || defined(__GP32__)) && !defined(_MSC_VER))
#include <unistd.h>
#endif

/*
 _____ _     _       _                   _                                 _ 
|_   _| |__ (_)___  (_)___    __ _    __| |_   _ _ __ ___  _ __ ___  _   _| |
  | | | '_ \| / __| | / __|  / _` |  / _` | | | | '_ ` _ \| '_ ` _ \| | | | |
  | | | | | | \__ \ | \__ \ | (_| | | (_| | |_| | | | | | | | | | | | |_| |_|
  |_| |_| |_|_|___/ |_|___/  \__,_|  \__,_|\__,_|_| |_| |_|_| |_| |_|\__, (_)
                                                                     |___/   

This just looks like an option dialog, but it doesn't change any actual settings currently!

*/

// TODO - allow changing options for:
// - the save path (use _browser!)
// - music & graphics driver (but see also the comments on EditGameDialog
//   for some techincal difficulties with this)
// - default volumes (sfx/master/music)

enum {
	kMasterVolumeChanged	= 'mavc',
	kMusicVolumeChanged		= 'muvc',
	kSfxVolumeChanged		= 'sfvc',
	kChooseSaveDirCmd		= 'chos',
	kOKCmd					= 'ok  '
};

GlobalOptionsDialog::GlobalOptionsDialog(NewGui *gui, GameDetector &detector)
	: Dialog(gui, 10, 15, 320 - 2 * 10, 200 - 2 * 15), _detector(detector) {
	// The GFX mode popup & a label
	// TODO - add an API to query the list of available GFX modes, and to get/set the mode
	new StaticTextWidget(this, 5, 10+1, 100, kLineHeight, "Graphics mode: ", kTextAlignRight);
	PopUpWidget *gfxPopUp;
	gfxPopUp = new PopUpWidget(this, 105, 10, 180, kLineHeight);
	gfxPopUp->appendEntry("<default>");
	gfxPopUp->appendEntry("");
	gfxPopUp->appendEntry("Normal (no scaling)");
	gfxPopUp->appendEntry("2x");
	gfxPopUp->appendEntry("3x");
	gfxPopUp->appendEntry("2xSAI");
	gfxPopUp->appendEntry("Super2xSAI");
	gfxPopUp->appendEntry("SuperEagle");
	gfxPopUp->appendEntry("AdvMAME2x");
	gfxPopUp->appendEntry("TV2x");
	gfxPopUp->appendEntry("DotMatrix");
	gfxPopUp->setSelected(0);
	
	// FIXME - disable GFX popup for now
	gfxPopUp->setEnabled(false);
	

	// The MIDI mode popup & a label
	StaticTextWidget *foo = new StaticTextWidget(this, 5, 26+1, 100, kLineHeight, "Music driver: ", kTextAlignRight);
	foo->setEnabled(false);
	_midiPopUp = new PopUpWidget(this, 105, 26, 180, kLineHeight);
	int midiSelected = 0, i = 0;;
	
	// Populate it
	const MusicDriver *md = GameDetector::getMusicDrivers();
	while (md->name) {
		if (GameDetector::isMusicDriverAvailable(md->id)) {
			_midiPopUp->appendEntry(md->description, md->id);
			if (md->id == _detector._midi_driver)
				midiSelected = i;
			i++;
		}
		md++;
	}
	_midiPopUp->setSelected(midiSelected);
	
	//
	// Sound controllers
	//
	const int yoffset = 40;
	new StaticTextWidget(this, 5, yoffset+10, 100, 16, "Master volume: ", kTextAlignRight);
	new StaticTextWidget(this, 5, yoffset+26, 100, 16, "Music volume: ", kTextAlignRight);
	new StaticTextWidget(this, 5, yoffset+42, 100, 16, "SFX volume: ", kTextAlignRight);

	_masterVolumeSlider = new SliderWidget(this, 105, yoffset+8, 85, 12, "Volume1", kMasterVolumeChanged);
	_musicVolumeSlider  = new SliderWidget(this, 105, yoffset+24, 85, 12, "Volume2", kMusicVolumeChanged);
	_sfxVolumeSlider    = new SliderWidget(this, 105, yoffset+40, 85, 12, "Volume3", kSfxVolumeChanged);

	_masterVolumeSlider->setMinValue(0);	_masterVolumeSlider->setMaxValue(255);
	_musicVolumeSlider->setMinValue(0);	_musicVolumeSlider->setMaxValue(255);
	_sfxVolumeSlider->setMinValue(0);	_sfxVolumeSlider->setMaxValue(255);

	_masterVolumeLabel = new StaticTextWidget(this, 200, yoffset+10, 24, 16, "100%", kTextAlignLeft);
	_musicVolumeLabel  = new StaticTextWidget(this, 200, yoffset+26, 24, 16, "100%", kTextAlignLeft);
	_sfxVolumeLabel    = new StaticTextWidget(this, 200, yoffset+42, 24, 16, "100%", kTextAlignLeft);
	
	_masterVolumeLabel->setFlags(WIDGET_CLEARBG);
	_musicVolumeLabel->setFlags(WIDGET_CLEARBG);
	_sfxVolumeLabel->setFlags(WIDGET_CLEARBG);


#if !( defined(__DC__) || defined(__GP32__) )
	//
	// Save game path
	//
	new StaticTextWidget(this, 5, 106, 100, kLineHeight, "Savegame path: ", kTextAlignRight);
	_savePath = new StaticTextWidget(this, 105, 106, 180, kLineHeight, "/foo/bar", kTextAlignLeft);
	new ButtonWidget(this, 105, 120, 64, 16, "Choose...", kChooseSaveDirCmd, 0);
	
// TODO: set _savePath to the current save path, i.e. as obtained via
	const char *dir = NULL;
	dir = g_config->get("savepath", "scummvm");
	if (dir) {
		_savePath->setLabel(dir);
	} else {
		// Default to the current directory...
		char buf[256];
		getcwd(buf, sizeof(buf));
		_savePath->setLabel(buf);
	}
#endif

	//
	// Add OK & Cancel buttons
	//
	addButton(_w - 2 * (kButtonWidth + 10), _h - 24, "Cancel", kCloseCmd, 0);
	addButton(_w - (kButtonWidth + 10), _h - 24, "OK", kOKCmd, 0);

	// Create file browser dialog
	_browser = new BrowserDialog(_gui, "Select directory for savegames");
}

GlobalOptionsDialog::~GlobalOptionsDialog() {
	delete _browser;
}

void GlobalOptionsDialog::open() {
	Dialog::open();

	_soundVolumeMaster = _detector._master_volume;
	_soundVolumeMusic = _detector._music_volume;
	_soundVolumeSfx = _detector._sfx_volume;

	_masterVolumeSlider->setValue(_soundVolumeMaster);
	_musicVolumeSlider->setValue(_soundVolumeMusic);
	_sfxVolumeSlider->setValue(_soundVolumeSfx);

	_masterVolumeLabel->setValue(_soundVolumeMaster);
	_musicVolumeLabel->setValue(_soundVolumeMusic);
	_sfxVolumeLabel->setValue(_soundVolumeSfx);
}

void GlobalOptionsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kChooseSaveDirCmd:
		if (_browser->runModal()) {
			// User made his choice...
			FilesystemNode *dir = _browser->getResult();
			_savePath->setLabel(dir->path());
			// TODO - we should check if the director is writeable before accepting it
		}
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
	case kPopUpItemSelectedCmd:
		if (sender == _midiPopUp) {
			const MusicDriver *md = GameDetector::getMusicDrivers();
			for (; md->name; md++) {
				if (md->id == (int) data) {
					g_config->set ("music_driver", md->name, "_USER_OVERRIDES");
					break;
				}
			}
		}
		break;
	case kOKCmd:
		// TODO Write back changes made to config object
		setResult(1);
		_detector._master_volume = _soundVolumeMaster;
		_detector._music_volume = _soundVolumeMusic;
		_detector._sfx_volume = _soundVolumeSfx;
		close();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}
