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
#include "browser.h"
#include "chooser.h"
#include "newgui.h"
#include "options.h"
#include "PopUpWidget.h"

//#include "backends/fs/fs.h"
#include "common/config-file.h"
//#include "common/engine.h"
#include "common/gameDetector.h"

// TODO - allow changing options for:
// - the save path (use _browser!)
// - music & graphics driver (but see also the comments on EditGameDialog
//   for some techincal difficulties with this)
// - default volumes (sfx/master/music)

enum {
	kOKCmd = 'OK  '
};

GlobalOptionsDialog::GlobalOptionsDialog(NewGui *gui)
	: Dialog(gui, 10, 15, 320-2*10, 200-2*15)
{
	// The GFX mode popup & a label
	// TODO - add an API to query the list of available GFX modes, and to get/set the mode
	new StaticTextWidget(this, 5, 10+1, 100, kLineHeight, "Graphics mode: ", kTextAlignRight);
	PopUpWidget *gfxPopUp;
	gfxPopUp = new PopUpWidget(this, 105, 10, 180, kLineHeight);
	gfxPopUp->appendEntry("<default>");
	gfxPopUp->appendEntry("-");
	gfxPopUp->appendEntry("Normal (no scaling)");
	gfxPopUp->appendEntry("2x");
	gfxPopUp->appendEntry("3x");
	gfxPopUp->appendEntry("2xSAI");
	gfxPopUp->appendEntry("Super2xSAI");
	gfxPopUp->appendEntry("SuperEagle");
	gfxPopUp->appendEntry("AdvMAME2x");
	gfxPopUp->setSelected(0);

	// The MIDI mode popup & a label
	new StaticTextWidget(this, 5, 26+1, 100, kLineHeight, "Music driver: ", kTextAlignRight);
	PopUpWidget *midiPopUp;
	midiPopUp = new PopUpWidget(this, 105, 26, 180, kLineHeight);
	
	// Populate it
	const MusicDrivers *md = GameDetector::getMusicDrivers();
	while (md->name) {
		if (GameDetector::isMusicDriverAvailable(md->id))
			midiPopUp->appendEntry(md->description, md->id);
		md++;
	}
	midiPopUp->setSelected(0);


	//
	// Sound controllers
	//
	const int yoffset = 40;
	new StaticTextWidget(this, 5, yoffset+10, 100, 16, "Master volume: ", kTextAlignRight);
	new StaticTextWidget(this, 5, yoffset+26, 100, 16, "Music volume: ", kTextAlignRight);
	new StaticTextWidget(this, 5, yoffset+42, 100, 16, "SFX volume: ", kTextAlignRight);

	SliderWidget *masterVolumeSlider, *musicVolumeSlider, *sfxVolumeSlider;

	masterVolumeSlider = new SliderWidget(this, 105, yoffset+8, 85, 12, "Volume1", 0);
	musicVolumeSlider  = new SliderWidget(this, 105, yoffset+24, 85, 12, "Volume2", 0);
	sfxVolumeSlider    = new SliderWidget(this, 105, yoffset+40, 85, 12, "Volume3", 0);

	masterVolumeSlider->setMinValue(0);	masterVolumeSlider->setMaxValue(255);
	musicVolumeSlider->setMinValue(0);	musicVolumeSlider->setMaxValue(255);
	sfxVolumeSlider->setMinValue(0);	sfxVolumeSlider->setMaxValue(255);

	Widget *masterVolumeLabel, *musicVolumeLabel, *sfxVolumeLabel;
	
	masterVolumeLabel = new StaticTextWidget(this, 200, yoffset+10, 24, 16, "100%", kTextAlignLeft);
	musicVolumeLabel  = new StaticTextWidget(this, 200, yoffset+26, 24, 16, "100%", kTextAlignLeft);
	sfxVolumeLabel    = new StaticTextWidget(this, 200, yoffset+42, 24, 16, "100%", kTextAlignLeft);
	
	masterVolumeLabel->setFlags(WIDGET_CLEARBG);
	musicVolumeLabel->setFlags(WIDGET_CLEARBG);
	sfxVolumeLabel->setFlags(WIDGET_CLEARBG);


	//
	// Save game path
	//
	new StaticTextWidget(this, 5, 106, 100, kLineHeight, "Savegame path: ", kTextAlignRight);
	new StaticTextWidget(this, 105, 106, 180, kLineHeight, "/foo/bar", kTextAlignLeft);
	new ButtonWidget(this, 105, 120, 64, 16, "Choose...", 0, 0);
	

	//
	// Add OK & Cancel buttons
	//
	addButton(_w-2*(kButtonWidth+10), _h-24, "Cancel", kCloseCmd, 0);
	addButton(_w-(kButtonWidth+10), _h-24, "OK", kOKCmd, 0);
}

GlobalOptionsDialog::~GlobalOptionsDialog()
{
}

void GlobalOptionsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data)
{
	if (cmd == kOKCmd) {
		// TODO Write back changes made to config object
		setResult(1);
		close();
	} else {
		Dialog::handleCommand(sender, cmd, data);
	}
}
