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
	kMasterVolumeChanged		= 'mavc',
	kMusicVolumeChanged		= 'muvc',
	kSfxVolumeChanged		= 'sfvc',
	kChooseSaveDirCmd		= 'chos',
	kOKCmd				= 'ok  '
};

GlobalOptionsDialog::GlobalOptionsDialog(GameDetector &detector)
	: Dialog(10, 20, 320 - 2 * 10, 200 - 2 * 20) {

	const int vBorder = 5;
	int yoffset;

	// The tab widget
	TabWidget *tab = new TabWidget(this, 0, vBorder, _w, _h - 24 - 2*vBorder);

	//
	// 1) The graphics tab
	//
	tab->addTab("Graphics");
	yoffset = vBorder;

	// The GFX mode popup & a label
	// TODO - add an API to query the list of available GFX modes, and to get/set the mode
	_gfxPopUp = new PopUpWidget(tab, 5, yoffset, 280, kLineHeight, "Graphics mode: ", 100);
	yoffset += 16;

	// Ender: We don't really want a <default> here at all, we want to setSelected to the current global
	_gfxPopUp->appendEntry("<default>");
	_gfxPopUp->appendEntry("");
	_gfxPopUp->appendEntry("Normal (no scaling)");
	_gfxPopUp->appendEntry("2x");
	_gfxPopUp->appendEntry("3x");
	_gfxPopUp->appendEntry("2xSAI");
	_gfxPopUp->appendEntry("Super2xSAI");
	_gfxPopUp->appendEntry("SuperEagle");
	_gfxPopUp->appendEntry("AdvMAME2x");
	_gfxPopUp->appendEntry("AdvMAME3x");
	_gfxPopUp->appendEntry("hq2x");
	_gfxPopUp->appendEntry("hq3x");
	_gfxPopUp->appendEntry("TV2x");
	_gfxPopUp->appendEntry("DotMatrix");
	_gfxPopUp->setSelected(0);
	
	// FIXME - disable GFX popup for now
	_gfxPopUp->setEnabled(false);
	
#if 1
	// TODO: Aspect ratio setting
	// TODO: Fullscreen setting
	_fullscreenCheckbox = new CheckboxWidget(tab, 10, yoffset, 280, 16, "Fullscreen mode");
	_fullscreenCheckbox->setState(ConfMan.getBool("fullscreen"));
	yoffset += 16;

	_aspectCheckbox = new CheckboxWidget(tab, 10, yoffset, 280, 16, "Aspect ratio correction");
	_aspectCheckbox->setState(ConfMan.getBool("aspect_ratio"));
	yoffset += 16;
#endif


	//
	// 2) The audio tab
	//
	tab->addTab("Audio");
	yoffset = vBorder;

	// The MIDI mode popup & a label
	_midiPopUp = new PopUpWidget(tab, 5, yoffset, 280, kLineHeight, "Music driver: ", 100);
	yoffset += 16;
	
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
	
	// Multi midi setting
	_multiMidiCheckbox = new CheckboxWidget(tab, 10, yoffset, 280, 16, "Mixed Adlib/MIDI mode");
	_multiMidiCheckbox->setState(ConfMan.getBool("multi_midi"));
	yoffset += 16;

	// Native mt32 setting
	_mt32Checkbox = new CheckboxWidget(tab, 10, yoffset, 280, 16, "True Roland MT-32 (disable GM emulation)");
	_mt32Checkbox->setState(ConfMan.getBool("native_mt32"));
	yoffset += 16;

	// TODO: cd drive setting

	//
	// 3) The miscellaneous tab
	//
	tab->addTab("Misc");
	yoffset = vBorder;

#if !( defined(__DC__) || defined(__GP32__) )
	// Save game path
	new StaticTextWidget(tab, 5, yoffset+2, 100, kLineHeight, "Savegame path: ", kTextAlignRight);
	_savePath = new StaticTextWidget(tab, 105, yoffset+2, 180, kLineHeight, "/foo/bar", kTextAlignLeft);
	new ButtonWidget(tab, 105, yoffset+14, 64, 16, "Choose...", kChooseSaveDirCmd, 0);
	
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
	
	int vol;

	vol = ConfMan.getInt("master_volume");
	_masterVolumeSlider->setValue(vol);
	_masterVolumeLabel->setValue(vol);

	vol = ConfMan.getInt("music_volume");
	_musicVolumeSlider->setValue(vol);
	_musicVolumeLabel->setValue(vol);

	vol = ConfMan.getInt("sfx_volume");
	_sfxVolumeSlider->setValue(vol);
	_sfxVolumeLabel->setValue(vol);
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
		_masterVolumeLabel->setValue(_masterVolumeSlider->getValue());
		_masterVolumeLabel->draw();
		break;
	case kMusicVolumeChanged:
		_musicVolumeLabel->setValue(_musicVolumeSlider->getValue());
		_musicVolumeLabel->draw();
		break;
	case kSfxVolumeChanged:
		_sfxVolumeLabel->setValue(_sfxVolumeSlider->getValue());
		_sfxVolumeLabel->draw();
		break;
	case kOKCmd: {
		setResult(1);
		// TODO: All these settings should take effect immediately.
		// There are two ways to ensure that:
		// 1) Add code here which pushes the changes on to the mixer/backend
		// 2) Implement the ConfigManager callback API; then, let the mixer/backend
		//    and any other interested parties register for notifications sent
		//    whenever these config values change.
		ConfMan.set("master_volume", _masterVolumeSlider->getValue());
		ConfMan.set("music_volume", _musicVolumeSlider->getValue());
		ConfMan.set("sfx_volume", _sfxVolumeSlider->getValue());
		ConfMan.set("fullscreen", _fullscreenCheckbox->getState());
		ConfMan.set("aspect_ratio", _aspectCheckbox->getState());
		ConfMan.set("multi_midi", _multiMidiCheckbox->getState());
		ConfMan.set("native_mt32", _mt32Checkbox->getState());

		const MidiDriverDescription *md = getAvailableMidiDrivers();
		for (; md->name; md++) {
			if (md->id == (int)_midiPopUp->getSelectedTag()) {
				ConfMan.set("music_driver", md->name);
				break;
			}
		}
		if (!md->name)
			ConfMan.removeKey("music_driver", Common::ConfigManager::kApplicationDomain);

		close();
		break; }
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}
