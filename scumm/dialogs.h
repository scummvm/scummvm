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

#ifndef SCUMM_DIALOGS_H
#define SCUMM_DIALOGS_H

#include "common/str.h"
#include "gui/about.h"
#include "gui/dialog.h"
#include "gui/widget.h"

#ifndef DISABLE_HELP
#include "scumm/help.h"
#endif

class ListWidget;

namespace Scumm {

class ScummEngine;

class ScummDialog : public Dialog {
public:
	ScummDialog(ScummEngine *scumm, int x, int y, int w, int h)
		: Dialog(x, y, w, h), _scumm(scumm) {}
	
protected:
	typedef Common::String String;

	ScummEngine *_scumm;

	void addResText(int x, int y, int w, int h, int resID);

	// Query a string from the resources
	const String queryResString(int stringno);
};

class MainMenuDialog : public ScummDialog {
public:
	MainMenuDialog(ScummEngine *scumm);
	~MainMenuDialog();
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
	virtual void open();	
	virtual void close();

protected:
	Dialog		*_aboutDialog;
#ifndef DISABLE_HELP
	Dialog		*_helpDialog;
#endif

	void save();
	void load();
};

#ifndef DISABLE_HELP

class HelpDialog : public ScummDialog {
public:
	HelpDialog(ScummEngine *scumm);
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

protected:
	typedef Common::String String;

	ButtonWidget *_nextButton;
	ButtonWidget *_prevButton;

	StaticTextWidget *_title;
	StaticTextWidget *_key[HELP_NUM_LINES];
	StaticTextWidget *_dsc[HELP_NUM_LINES];

	int _page;
	int _numPages;

	void displayKeyBindings();
};

#endif

class OptionsDialog : public ScummDialog {
protected:
#ifdef _WIN32_WCE
	Dialog		*_keysDialog;
#endif

public:
	OptionsDialog(ScummEngine *scumm);
	~OptionsDialog();

	virtual void open();
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

protected:

	int _soundVolumeMaster;
	int _soundVolumeMusic;
	int _soundVolumeSfx;

	SliderWidget *_masterVolumeSlider;
	SliderWidget *_musicVolumeSlider;
	SliderWidget *_sfxVolumeSlider;

	StaticTextWidget *_masterVolumeLabel;
	StaticTextWidget *_musicVolumeLabel;
	StaticTextWidget *_sfxVolumeLabel;
	
	CheckboxWidget *subtitlesCheckbox;
};

class InfoDialog : public ScummDialog {
public:
	// arbitrary message
	InfoDialog(ScummEngine *scumm, const String& message);
	// from resources
	InfoDialog(ScummEngine *scumm, int res);

	virtual void handleMouseDown(int x, int y, int button, int clickCount) { 
		close();
	}
	virtual void handleKeyDown(uint16 ascii, int keycode, int modifiers) {
		setResult(ascii);
		close();
	}

protected:
	void setInfoText (const String& message);
};

class PauseDialog : public InfoDialog {
public:
	PauseDialog(ScummEngine *scumm);
	virtual void handleKeyDown(uint16 ascii, int keycode, int modifiers)
		{
			if (ascii == ' ')  // Close pause dialog if space key is pressed
				close();
			else
				ScummDialog::handleKeyDown(ascii, keycode, modifiers);
		}
};

class ConfirmExitDialog : public InfoDialog {
public:
	ConfirmExitDialog(ScummEngine *scumm);
	virtual void handleKeyDown(uint16 ascii, int keycode, int modifiers);
};

#ifdef _WIN32_WCE

class KeysDialog : public ScummDialog {
public:
	KeysDialog(ScummEngine *scumm);

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
	virtual void handleKeyDown(uint16 ascii, int keycode, int modifiers);

protected:

	ListWidget		 *_actionsList;
	StaticTextWidget *_actionTitle;
	StaticTextWidget *_keyMapping;
	int				 _actionSelected;
};

#endif

} // End of namespace Scumm

#endif
