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
#include "gui/browser.h"
#include "gui/chooser.h"
#include "gui/newgui.h"
#include "gui/options.h"
#include "gui/PopUpWidget.h"
#include "gui/TabWidget.h"

#include "backends/fs/fs.h"
#include "base/gameDetector.h"
#include "common/config-manager.h"
#include "sound/mididrv.h"

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
// - aspect ratio, language, platform, subtitles, debug mode/level, cd drive, joystick, multi midi, native mt32

enum {
	kMasterVolumeChanged	= 'mavc',
	kMusicVolumeChanged		= 'muvc',
	kSfxVolumeChanged		= 'sfvc',
	kChooseSaveDirCmd		= 'chos',
	kOKCmd					= 'ok  '
};

GlobalOptionsDialog::GlobalOptionsDialog(GameDetector &detector)
	: Dialog(10, 20, 320 - 2 * 10, 200 - 2 * 20) {

	const int vBorder = 5;

	// The tab widget
	TabWidget *tab = new TabWidget(this, 0, vBorder, _w, _h - 24 - 2*vBorder);

	//
	// 1) The graphics tab
	//
	tab->addTab("Graphics");

	// The GFX mode popup & a label
	// TODO - add an API to query the list of available GFX modes, and to get/set the mode
	//new StaticTextWidget(tab, 5, vBorder+2, 100, kLineHeight, "Graphics mode: ", kTextAlignRight);
	PopUpWidget *gfxPopUp;
	gfxPopUp = new PopUpWidget(tab, 5, vBorder, 280, kLineHeight, "Graphics mode: ", 100);
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
	
	// TODO: Aspect ratio setting
	// TODO: Fullscreen setting


	//
	// 2) The audio tab
	//
	tab->addTab("Audio");

	// The MIDI mode popup & a label
	//new StaticTextWidget(tab, 5, vBorder+2, 100, kLineHeight, "Music driver: ", kTextAlignRight);
	_midiPopUp = new PopUpWidget(tab, 5, vBorder, 280, kLineHeight, "Music driver: ", 100);
	
	// Populate it
	const MidiDriverDescription *md = getAvailableMidiDrivers();
	const int midiDriver = parseMusicDriver(ConfMan.get("music_driver"));
	int midiSelected = 0, i = 0;
	while (md->name) {
		_midiPopUp->appendEntry(md->description, md->id);
		if (md->id == midiDriver)
			midiSelected = i;
		i++;
		md++;
	}
	_midiPopUp->setSelected(midiSelected);
	
	// Volume controllers
	int yoffset = vBorder + 16;

	_masterVolumeSlider = new SliderWidget(tab, 5, yoffset, 185, 12,  "Master volume: ", 100, kMasterVolumeChanged);
	_masterVolumeLabel = new StaticTextWidget(tab, 200, yoffset+2, 24, 16, "100%", kTextAlignLeft);
	_masterVolumeSlider->setMinValue(0); _masterVolumeSlider->setMaxValue(255);
	_masterVolumeLabel->setFlags(WIDGET_CLEARBG);
	yoffset += 16;

	_musicVolumeSlider = new SliderWidget(tab, 5, yoffset, 185, 12, "Music volume: ", 100, kMusicVolumeChanged);
	_musicVolumeLabel = new StaticTextWidget(tab, 200, yoffset+2, 24, 16, "100%", kTextAlignLeft);
	_musicVolumeSlider->setMinValue(0); _musicVolumeSlider->setMaxValue(255);
	_musicVolumeLabel->setFlags(WIDGET_CLEARBG);
	yoffset += 16;

	_sfxVolumeSlider = new SliderWidget(tab, 5, yoffset, 185, 12, "SFX volume: ", 100, kSfxVolumeChanged);
	_sfxVolumeLabel = new StaticTextWidget(tab, 200, yoffset+2, 24, 16, "100%", kTextAlignLeft);
	_sfxVolumeSlider->setMinValue(0); _sfxVolumeSlider->setMaxValue(255);
	_sfxVolumeLabel->setFlags(WIDGET_CLEARBG);
	yoffset += 16;
	
	// TODO: cd drive setting
	// TODO: multi midi setting
	// TODO: native mt32 setting


	//
	// 3) The miscellaneous tab
	//
	tab->addTab("Misc");

#if !( defined(__DC__) || defined(__GP32__) )
	// Save game path
	new StaticTextWidget(tab, 5, vBorder+2, 100, kLineHeight, "Savegame path: ", kTextAlignRight);
	_savePath = new StaticTextWidget(tab, 105, vBorder+2, 180, kLineHeight, "/foo/bar", kTextAlignLeft);
	new ButtonWidget(tab, 105, vBorder+14, 64, 16, "Choose...", kChooseSaveDirCmd, 0);
	
// TODO: set _savePath to the current save path
	Common::String dir(ConfMan.get("savepath"));
	if (!dir.isEmpty()) {
		_savePath->setLabel(dir);
	} else {
		// Default to the current directory...
		char buf[256];
		getcwd(buf, sizeof(buf));
		_savePath->setLabel(buf);
	}
#endif
	// TODO: joystick setting


	//
	// Add OK & Cancel buttons
	//
	addButton(_w - 2 * (kButtonWidth + 10), _h - 24, "Cancel", kCloseCmd, 0);
	addButton(_w - (kButtonWidth + 10), _h - 24, "OK", kOKCmd, 0);

	// Create file browser dialog
	_browser = new BrowserDialog("Select directory for savegames");
	
	
	// Activate the first tab
	tab->setActiveTab(0);
}

GlobalOptionsDialog::~GlobalOptionsDialog() {
	delete _browser;
}

void GlobalOptionsDialog::open() {
	Dialog::open();

	_soundVolumeMaster = ConfMan.getInt("master_volume");
	_masterVolumeSlider->setValue(_soundVolumeMaster);
	_masterVolumeLabel->setValue(_soundVolumeMaster);

	_soundVolumeMusic = ConfMan.getInt("music_volume");
	_musicVolumeSlider->setValue(_soundVolumeMusic);
	_musicVolumeLabel->setValue(_soundVolumeMusic);

	_soundVolumeSfx = ConfMan.getInt("sfx_volume");
	_sfxVolumeSlider->setValue(_soundVolumeSfx);
	_sfxVolumeLabel->setValue(_soundVolumeSfx);
}

void GlobalOptionsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kChooseSaveDirCmd:
		if (_browser->runModal()) {
			// User made his choice...
			FilesystemNode *dir = _browser->getResult();
			_savePath->setLabel(dir->path());
			// TODO - we should check if the directory is writeable before accepting it
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
			const MidiDriverDescription *md = getAvailableMidiDrivers();
			for (; md->name; md++) {
				if (md->id == (int) data) {
					ConfMan.set("music_driver", md->name);
					break;
				}
			}
		}
		break;
	case kOKCmd:
		setResult(1);
		ConfMan.set("master_volume", _soundVolumeMaster);
		ConfMan.set("music_volume", _soundVolumeMusic);
		ConfMan.set("sfx_volume", _soundVolumeSfx);
		close();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}
