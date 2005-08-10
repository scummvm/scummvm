/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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

#ifdef SMALL_SCREEN_DEVICE
#include "gui/KeysDialog.h"
#endif

namespace GUI {

class BrowserDialog;
class CheckboxWidget;
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

	ButtonWidget *_soundFontButton;
	StaticTextWidget *_soundFont;

	int addGraphicControls(GuiObject *boss, int yoffset, WidgetSize ws);
	int addAudioControls(GuiObject *boss, int yoffset, WidgetSize ws);
	int addMIDIControls(GuiObject *boss, int yoffset, WidgetSize ws);
	int addVolumeControls(GuiObject *boss, int yoffset, WidgetSize ws);

	void setGraphicSettingsState(bool enabled);
	void setAudioSettingsState(bool enabled);
	void setMIDISettingsState(bool enabled);
	void setVolumeSettingsState(bool enabled);

private:
	//
	// Graphics controls
	//
	bool _enableGraphicSettings;
	PopUpWidget *_gfxPopUp;
	CheckboxWidget *_fullscreenCheckbox;
	CheckboxWidget *_aspectCheckbox;
	PopUpWidget *_renderModePopUp;

	//
	// Audio controls
	//
	bool _enableAudioSettings;
	PopUpWidget *_midiPopUp;
	CheckboxWidget *_subCheckbox;

	//
	// MIDI controls
	//
	bool _enableMIDISettings;
	CheckboxWidget *_multiMidiCheckbox;
	CheckboxWidget *_mt32Checkbox;
	CheckboxWidget *_enableGSCheckbox;

	//
	// Volume controls
	//
	bool _enableVolumeSettings;

	SliderWidget *_musicVolumeSlider;
	StaticTextWidget *_musicVolumeLabel;

	SliderWidget *_sfxVolumeSlider;
	StaticTextWidget *_sfxVolumeLabel;

	SliderWidget *_speechVolumeSlider;
	StaticTextWidget *_speechVolumeLabel;
};


class GlobalOptionsDialog : public OptionsDialog {
	typedef Common::String String;
public:
	GlobalOptionsDialog();
	~GlobalOptionsDialog();

	void open();
	void close();
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

protected:
#ifdef SMALL_SCREEN_DEVICE
	KeysDialog *_keysDialog;
#endif
	StaticTextWidget *_savePath;
	StaticTextWidget *_extraPath;
};

} // End of namespace GUI

#endif
