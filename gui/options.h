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

#ifndef OPTIONS_DIALOG_H
#define OPTIONS_DIALOG_H

#include "gui/dialog.h"
#include "common/str.h"
#include "common/list.h"

class BrowserDialog;
class CheckboxWidget;
class GameDetector;
class PopUpWidget;
class SliderWidget;
class StaticTextWidget;

class OptionsDialog : public Dialog {
	typedef Common::String String;
public:
	OptionsDialog(const String &domain, int x, int y, int w, int h);

	void open();
	void close();
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

	enum {
		kOKCmd					= 'ok  '
	};

protected:
	/** Config domain this dialog is used to edit. */
	String _domain;
	
	int addGraphicControls(GuiObject *boss, int yoffset);
	int addMIDIControls(GuiObject *boss, int yoffset);
	int addVolumeControls(GuiObject *boss, int yoffset);

private:
	//
	// Graphics controls
	//
	PopUpWidget *_gfxPopUp;
	CheckboxWidget *_fullscreenCheckbox;
	CheckboxWidget *_aspectCheckbox;

	//
	// MIDI controls
	//
	PopUpWidget *_midiPopUp;

	CheckboxWidget *_multiMidiCheckbox;
	CheckboxWidget *_mt32Checkbox;

	//
	// Volume controls
	//
	SliderWidget *_masterVolumeSlider;
	StaticTextWidget *_masterVolumeLabel;

	SliderWidget *_musicVolumeSlider;
	StaticTextWidget *_musicVolumeLabel;

	SliderWidget *_sfxVolumeSlider;
	StaticTextWidget *_sfxVolumeLabel;
};


class GlobalOptionsDialog : public OptionsDialog {
	typedef Common::String String;
public:
	GlobalOptionsDialog(GameDetector &detector);
	~GlobalOptionsDialog();

	void open();
	void close();
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

protected:
	BrowserDialog *_browser;
	StaticTextWidget *_savePath;
};

#endif
