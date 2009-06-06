/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#ifndef OPTIONS_DIALOG_H
#define OPTIONS_DIALOG_H

#include "gui/dialog.h"
#include "gui/TabWidget.h"
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
class ListWidget;

class OptionsDialog : public Dialog {
	typedef Common::String String;
public:
	OptionsDialog(const String &domain, int x, int y, int w, int h);
	OptionsDialog(const String &domain, const String &name);

	void init();

	void open();
	void close();
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
	const Common::String& getDomain() const { return _domain; }

	virtual void reflowLayout();

	enum {
		kOKCmd					= 'ok  '
	};

protected:
	/** Config domain this dialog is used to edit. */
	String _domain;

	ButtonWidget *_soundFontButton;
	StaticTextWidget *_soundFont;
	ButtonWidget *_soundFontClearButton;

	void addGraphicControls(GuiObject *boss, const String &prefix);
	void addAudioControls(GuiObject *boss, const String &prefix);
	void addMIDIControls(GuiObject *boss, const String &prefix);
	void addVolumeControls(GuiObject *boss, const String &prefix);
	// The default value is the launcher's non-scaled talkspeed value. When SCUMM uses the widget,
	// it uses its own scale
	void addSubtitleControls(GuiObject *boss, const String &prefix, int maxSliderVal = 255);

	void setGraphicSettingsState(bool enabled);
	void setAudioSettingsState(bool enabled);
	void setMIDISettingsState(bool enabled);
	void setVolumeSettingsState(bool enabled);
	void setSubtitleSettingsState(bool enabled);

	TabWidget *_tabWidget;
	int _graphicsTabId;

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
	PopUpWidget *_oplPopUp;
	PopUpWidget *_outputRatePopUp;

	//
	// MIDI controls
	//
	bool _enableMIDISettings;
	CheckboxWidget *_multiMidiCheckbox;
	CheckboxWidget *_mt32Checkbox;
	CheckboxWidget *_enableGSCheckbox;
	StaticTextWidget *_midiGainDesc;
	SliderWidget *_midiGainSlider;
	StaticTextWidget *_midiGainLabel;

	//
	// Subtitle controls
	//
	int getSubtitleMode(bool subtitles, bool speech_mute);
	bool _enableSubtitleSettings;
	StaticTextWidget *_subToggleDesc;
	ButtonWidget *_subToggleButton;
	int _subMode;
	static const char *_subModeDesc[];
	static const char *_lowresSubModeDesc[];
	StaticTextWidget *_subSpeedDesc;
	SliderWidget *_subSpeedSlider;
	StaticTextWidget *_subSpeedLabel;

	//
	// Volume controls
	//
	bool _enableVolumeSettings;

	StaticTextWidget *_musicVolumeDesc;
	SliderWidget *_musicVolumeSlider;
	StaticTextWidget *_musicVolumeLabel;

	StaticTextWidget *_sfxVolumeDesc;
	SliderWidget *_sfxVolumeSlider;
	StaticTextWidget *_sfxVolumeLabel;

	StaticTextWidget *_speechVolumeDesc;
	SliderWidget *_speechVolumeSlider;
	StaticTextWidget *_speechVolumeLabel;

	CheckboxWidget *_muteCheckbox;
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
	StaticTextWidget *_themePath;
	StaticTextWidget *_extraPath;
#ifdef DYNAMIC_MODULES
	StaticTextWidget *_pluginsPath;
#endif

	//
	// Misc controls
	//
	StaticTextWidget *_curTheme;
	PopUpWidget *_rendererPopUp;
	PopUpWidget *_autosavePeriodPopUp;
};

} // End of namespace GUI

#endif
