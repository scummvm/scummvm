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

#ifndef SCUMM_DIALOGS_H
#define SCUMM_DIALOGS_H

#include "common/str.h"
#include "gui/dialog.h"

class ListWidget;
class Scumm;

class ScummDialog : public Dialog {
public:
	ScummDialog(NewGui *gui, Scumm *scumm, int x, int y, int w, int h)
		: Dialog(gui, x, y, w, h), _scumm(scumm) {}
	
protected:
	typedef ScummVM::String String;

	Scumm *_scumm;

	void addResText(int x, int y, int w, int h, int resID);

	// Query a string from the resources
	const String queryResString(int stringno);
	
	// Query a custom string. This is in a seperate method so that we
	// can easily localize the messages in the future if we want to.
	const char *queryCustomString(int stringno);
};

class SaveLoadDialog : public ScummDialog {
public:
	SaveLoadDialog(NewGui *gui, Scumm *scumm);

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

protected:
	ListWidget	*_savegameList;
};


class AboutDialog : public ScummDialog {
public:
	AboutDialog(NewGui *gui, Scumm *scumm);
};


class OptionsDialog : public ScummDialog {
protected:
	Dialog		*_aboutDialog;
	Dialog		*_soundDialog;
	Dialog		*_keysDialog;
	Dialog		*_miscDialog;

public:
	OptionsDialog(NewGui *gui, Scumm *scumm);
	~OptionsDialog();

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
};

class PauseDialog : public ScummDialog {
public:
	PauseDialog(NewGui *gui, Scumm *scumm);

	virtual void handleMouseDown(int x, int y, int button, int clickCount)
		{ close(); }
	virtual void handleKeyDown(char key, int modifiers)
		{
			if (key == 32)
				close();
			else
				Dialog::handleKeyDown(key, modifiers);
		}

	// Enforce no transparency!
	virtual void	setupScreenBuf() {}
	virtual void	teardownScreenBuf() {}

};


class SoundDialog : public ScummDialog {
public:
	SoundDialog(NewGui *gui, Scumm *scumm);

	enum {
		kMasterVolumeChanged	= 'mavc',
		kMusicVolumeChanged		= 'muvc',
		kSfxVolumeChanged		= 'sfvc',
		kOKCmd					= 'ok  ',
		kCancelCmd				= 'cncl',
	};

	virtual void open();

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

protected:
	
	int _soundVolumeMaster;
	int _soundVolumeMusic;
	int _soundVolumeSfx;

	SliderWidget *masterVolumeSlider;
	SliderWidget *musicVolumeSlider;
	SliderWidget *sfxVolumeSlider;
	
	StaticTextWidget *masterVolumeLabel;
	StaticTextWidget *musicVolumeLabel;
	StaticTextWidget *sfxVolumeLabel;
};


#endif
