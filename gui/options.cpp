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

#include "stdafx.h"
#include "gui/browser.h"
#include "gui/chooser.h"
#include "gui/newgui.h"
#include "gui/options.h"
#include "gui/PopUpWidget.h"
#include "gui/TabWidget.h"

#include "backends/fs/fs.h"
#include "common/config-manager.h"
#include "common/scaler.h"
#include "common/system.h"
#include "sound/mididrv.h"
#include "sound/mixer.h"

#if (!( defined(__PALM_OS__) || defined(__DC__) || defined(__GP32__) || defined(__amigaos4__) ) && !defined(_MSC_VER))
#include <sys/param.h>
#include <unistd.h>
#endif

#if !(defined(MAXPATHLEN))
#ifndef __PALM_OS__
#define MAXPATHLEN 1024
#else
#define MAXPATHLEN 256
#endif
#endif

namespace GUI {

// TODO - allow changing options for:
// - the save path (use _dirBrowser!)
// - music & graphics driver (but see also the comments on EditGameDialog
//   for some techincal difficulties with this)
// - default volumes (sfx/speech/music)
// - aspect ratio, language, platform, subtitles, debug mode/level, cd drive, joystick, multi midi, native mt32

enum {
	kMusicVolumeChanged		= 'muvc',
	kSfxVolumeChanged		= 'sfvc',
	kSpeechVolumeChanged	= 'vcvc',
	kChooseSoundFontCmd		= 'chsf',
	kChooseSaveDirCmd		= 'chos',
	kChooseExtraDirCmd		= 'chex'
};

#ifdef _WIN32_WCE
enum {
	kChooseKeyMappingCmd    = 'chma'
};
#endif


OptionsDialog::OptionsDialog(const String &domain, int x, int y, int w, int h)
	: Dialog(x, y, w, h),
	_domain(domain),
	_enableGraphicSettings(false),
	_gfxPopUp(0), _renderModePopUp(0), _fullscreenCheckbox(0), _aspectCheckbox(0),
	_enableAudioSettings(false),
	_subCheckbox(0),
	_enableMIDISettings(false),
	_multiMidiCheckbox(0), _mt32Checkbox(0), _enableGSCheckbox(0),
	_enableVolumeSettings(false),
	_musicVolumeSlider(0), _musicVolumeLabel(0),
	_sfxVolumeSlider(0), _sfxVolumeLabel(0),
	_speechVolumeSlider(0), _speechVolumeLabel(0) {
}

void OptionsDialog::open() {
	Dialog::open();

	// Reset result value
	setResult(0);
	
	if (_fullscreenCheckbox) {
		_gfxPopUp->setSelected(0);

		if (ConfMan.hasKey("gfx_mode", _domain)) {
			const OSystem::GraphicsMode *gm = g_system->getSupportedGraphicsModes();
			String gfxMode = ConfMan.get("gfx_mode", _domain);
			int gfxCount = 1;
			while (gm->name) {
				gfxCount++;

				if (scumm_stricmp(gm->name, gfxMode.c_str()) == 0)
					_gfxPopUp->setSelected(gfxCount);

				gm++;
			}
		}

		_renderModePopUp->setSelected(0);

		if (ConfMan.hasKey("render_mode", _domain)) {
			const Common::RenderModeDescription *p = Common::g_renderModes;
			const Common::RenderMode renderMode = Common::parseRenderMode(ConfMan.get("render_mode", _domain));
			int sel = 0;
			for (int i = 0; p->code; ++p, ++i) {
				if (renderMode == p->id)
					sel = i + 2;
			}
			_renderModePopUp->setSelected(sel);
		}

#ifndef _WIN32_WCE
		// Fullscreen setting
		_fullscreenCheckbox->setState(ConfMan.getBool("fullscreen", _domain));
	
		// Aspect ratio setting
		_aspectCheckbox->setState(ConfMan.getBool("aspect_ratio", _domain));
#endif
	}

	if (_subCheckbox) {
		// Music driver
		const MidiDriverDescription *md = MidiDriver::getAvailableMidiDrivers();
		int i = 0;
		const int midiDriver =
			ConfMan.hasKey("music_driver", _domain)
				? MidiDriver::parseMusicDriver(ConfMan.get("music_driver", _domain))
				: MD_AUTO;
		while (md->name && md->id != midiDriver) {
			i++;
			md++;
		}
		_midiPopUp->setSelected(md->name ? i : 0);

		// Subtitles setting
		_subCheckbox->setState(ConfMan.getBool("subtitles", _domain));
	}

	if (_multiMidiCheckbox) {
		
		// Multi midi setting
		_multiMidiCheckbox->setState(ConfMan.getBool("multi_midi", _domain));

		// Native mt32 setting
		_mt32Checkbox->setState(ConfMan.getBool("native_mt32", _domain));

		// GS extensions setting
		_enableGSCheckbox->setState(ConfMan.getBool("enable_gs", _domain));
	}
	

	if (_musicVolumeSlider) {
		int vol;

		vol = ConfMan.getInt("music_volume", _domain);
		_musicVolumeSlider->setValue(vol);
		_musicVolumeLabel->setValue(vol);

		vol = ConfMan.getInt("sfx_volume", _domain);
		_sfxVolumeSlider->setValue(vol);
		_sfxVolumeLabel->setValue(vol);

		vol = ConfMan.getInt("speech_volume", _domain);
		_speechVolumeSlider->setValue(vol);
		_speechVolumeLabel->setValue(vol);
	}
}

void OptionsDialog::close() {
	if (getResult()) {
		if (_fullscreenCheckbox) {
			if (_enableGraphicSettings) {
				ConfMan.set("fullscreen", _fullscreenCheckbox->getState(), _domain);
				ConfMan.set("aspect_ratio", _aspectCheckbox->getState(), _domain);

				if ((int32)_gfxPopUp->getSelectedTag() >= 0)
					ConfMan.set("gfx_mode", _gfxPopUp->getSelectedString(), _domain);

				if ((int32)_renderModePopUp->getSelectedTag() >= 0)
					ConfMan.set("render_mode", Common::getRenderModeCode((Common::RenderMode)_renderModePopUp->getSelectedTag()), _domain);
			} else {
				ConfMan.removeKey("fullscreen", _domain);
				ConfMan.removeKey("aspect_ratio", _domain);
				ConfMan.removeKey("gfx_mode", _domain);
				ConfMan.removeKey("render_mode", _domain);
			}
		}

		if (_musicVolumeSlider) {
			if (_enableVolumeSettings) {
				ConfMan.set("music_volume", _musicVolumeSlider->getValue(), _domain);
				ConfMan.set("sfx_volume", _sfxVolumeSlider->getValue(), _domain);
				ConfMan.set("speech_volume", _speechVolumeSlider->getValue(), _domain);
			} else {
				ConfMan.removeKey("music_volume", _domain);
				ConfMan.removeKey("sfx_volume", _domain);
				ConfMan.removeKey("speech_volume", _domain);
			}
		}

		if (_subCheckbox) {
			if (_enableAudioSettings) {
				ConfMan.set("subtitles", _subCheckbox->getState(), _domain); 
				const MidiDriverDescription *md = MidiDriver::getAvailableMidiDrivers();
				while (md->name && md->id != (int)_midiPopUp->getSelectedTag())
					md++;
				if (md->name)
					ConfMan.set("music_driver", md->name, _domain);
				else
					ConfMan.removeKey("music_driver", _domain);
			} else {
				ConfMan.removeKey("music_driver", _domain);
				ConfMan.removeKey("subtitles", _domain); 
			}
		}

		if (_multiMidiCheckbox) {
			if (_enableMIDISettings) {
				ConfMan.set("multi_midi", _multiMidiCheckbox->getState(), _domain);
				ConfMan.set("native_mt32", _mt32Checkbox->getState(), _domain);
				ConfMan.set("enable_gs", _enableGSCheckbox->getState(), _domain);
			} else {
				ConfMan.removeKey("multi_midi", _domain);
				ConfMan.removeKey("native_mt32", _domain);
				ConfMan.removeKey("enable_gs", _domain);
			}
		}

		// Save config file
		ConfMan.flushToDisk();
	}

	Dialog::close();
}

void OptionsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kMusicVolumeChanged:
		_musicVolumeLabel->setValue(_musicVolumeSlider->getValue());
		_musicVolumeLabel->draw();
		break;
	case kSfxVolumeChanged:
		_sfxVolumeLabel->setValue(_sfxVolumeSlider->getValue());
		_sfxVolumeLabel->draw();
		break;
	case kSpeechVolumeChanged:
		_speechVolumeLabel->setValue(_speechVolumeSlider->getValue());
		_speechVolumeLabel->draw();
		break;
	case kOKCmd:
		setResult(1);
		close();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void OptionsDialog::setGraphicSettingsState(bool enabled) {
	_enableGraphicSettings = enabled;

	_gfxPopUp->setEnabled(enabled);
	_renderModePopUp->setEnabled(enabled);
#ifndef _WIN32_WCE
	_fullscreenCheckbox->setEnabled(enabled);
	_aspectCheckbox->setEnabled(enabled);
#endif
}

void OptionsDialog::setAudioSettingsState(bool enabled) {
	_enableAudioSettings = enabled;

	_midiPopUp->setEnabled(enabled);
	_subCheckbox->setEnabled(enabled);
}

void OptionsDialog::setMIDISettingsState(bool enabled) {
	_enableMIDISettings = enabled;

	_soundFontButton->setEnabled(enabled);
	_soundFont->setEnabled(enabled);
	_multiMidiCheckbox->setEnabled(enabled);
	_mt32Checkbox->setEnabled(enabled);
	_enableGSCheckbox->setEnabled(enabled);
}

void OptionsDialog::setVolumeSettingsState(bool enabled) {
	_enableVolumeSettings = enabled;

	_musicVolumeSlider->setEnabled(enabled);
	_musicVolumeLabel->setEnabled(enabled);
	_sfxVolumeSlider->setEnabled(enabled);
	_sfxVolumeLabel->setEnabled(enabled);
	_speechVolumeSlider->setEnabled(enabled);
	_speechVolumeLabel->setEnabled(enabled);
}

int OptionsDialog::addGraphicControls(GuiObject *boss, int yoffset, WidgetSize ws) {
	const int x = 10;
	const int w = _w - 2 * 10;
	const OSystem::GraphicsMode *gm = g_system->getSupportedGraphicsModes();

	// The GFX mode popup
	_gfxPopUp = new PopUpWidget(boss, x-5, yoffset, w+5, kLineHeight, "Graphics mode: ", 100);
	yoffset += 16;

	_gfxPopUp->appendEntry("<default>");
	_gfxPopUp->appendEntry("");
	while (gm->name) {
		_gfxPopUp->appendEntry(gm->name, gm->id);
		gm++;
	}

	// RenderMode popup
	_renderModePopUp = new PopUpWidget(boss, x-5, yoffset, w+5, kLineHeight, "Render mode: ", 100);
	yoffset += 16;
	_renderModePopUp->appendEntry("<default>", Common::kRenderDefault);
	_renderModePopUp->appendEntry("");
	const Common::RenderModeDescription *rm = Common::g_renderModes;
	for (; rm->code; ++rm) {
		_renderModePopUp->appendEntry(rm->description, rm->id);
	}

	// Fullscreen checkbox
	_fullscreenCheckbox = addCheckbox(boss, x, yoffset, "Fullscreen mode", 0, 0, ws);
	yoffset += _fullscreenCheckbox->getHeight();

	// Aspect ratio checkbox
	_aspectCheckbox = addCheckbox(boss, x, yoffset, "Aspect ratio correction", 0, 0, ws);
	yoffset += _aspectCheckbox->getHeight();

#ifdef _WIN32_WCE
	_fullscreenCheckbox->setState(TRUE);
	_fullscreenCheckbox->setEnabled(FALSE);
	_aspectCheckbox->setEnabled(FALSE);	
#endif
	
	_enableGraphicSettings = true;

	return yoffset;
}

int OptionsDialog::addAudioControls(GuiObject *boss, int yoffset, WidgetSize ws) {
	const int x = 10;
	const int w = _w - 20;

	// The MIDI mode popup & a label
	_midiPopUp = new PopUpWidget(boss, x-5, yoffset, w+5, kLineHeight, "Music driver: ", 100);
	yoffset += 18;
	
	// Populate it
	const MidiDriverDescription *md = MidiDriver::getAvailableMidiDrivers();
	while (md->name) {
		_midiPopUp->appendEntry(md->description, md->id);
		md++;
	}

	// Subtitles on/off
	_subCheckbox = addCheckbox(boss, x, yoffset, "Display subtitles", 0, 0, ws);
	yoffset += _subCheckbox->getHeight();

	yoffset += 18;
		
	_enableAudioSettings = true;

	return yoffset;
}

int OptionsDialog::addMIDIControls(GuiObject *boss, int yoffset, WidgetSize ws) {
	const int x = 10;
	int spacing;
	int buttonWidth, buttonHeight;

	if (ws == kBigWidgetSize) {
		buttonWidth = kBigButtonWidth;
		buttonHeight = kBigButtonHeight;
		spacing = 2;
	} else {
		buttonWidth = kButtonWidth;
		buttonHeight = kButtonHeight;
		spacing = 1;
	}

	// SoundFont
	_soundFontButton = addButton(boss, x, yoffset, "SoundFont: ", kChooseSoundFontCmd, 0, ws);
	_soundFont = new StaticTextWidget(boss, x + buttonWidth + 20, yoffset + 3, _w - (x + buttonWidth + 20) - 10, kLineHeight, "None", kTextAlignLeft, ws);
	yoffset += buttonHeight + 2 * spacing;

	// Multi midi setting
	_multiMidiCheckbox = addCheckbox(boss, x, yoffset, "Mixed Adlib/MIDI mode", 0, 0, ws);
	yoffset += _multiMidiCheckbox->getHeight() + spacing;
	
	// Native mt32 setting
	_mt32Checkbox = addCheckbox(boss, x, yoffset, "True Roland MT-32 (disable GM emulation)", 0, 0, ws);
	yoffset += _mt32Checkbox->getHeight() + spacing;

	// GS Extensions setting
	_enableGSCheckbox = addCheckbox(boss, x, yoffset, "Enable Roland GS Mode", 0, 0, ws);
	yoffset += _enableGSCheckbox->getHeight() + spacing;
	
	_enableMIDISettings = true;

	return yoffset;
}

int OptionsDialog::addVolumeControls(GuiObject *boss, int yoffset, WidgetSize ws) {
	const char *slider_labels[] = {
		"Music volume:",
		"SFX volume:",
		"Speech volume:"
	};

	const Graphics::Font *font;

	if (ws == kBigWidgetSize) {
		font = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	} else {
		font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	}

	int textwidth = 0;

	for (int i = 0; i < ARRAYSIZE(slider_labels); i++) {
		int width = font->getStringWidth(slider_labels[i]);

		if (width > textwidth)
			textwidth = width;
	}

	int xoffset = textwidth + 15;

	// Volume controllers
	new StaticTextWidget(boss, 10, yoffset + 2, textwidth, kLineHeight, slider_labels[0], kTextAlignRight, ws);
	_musicVolumeSlider = addSlider(boss, xoffset, yoffset, kMusicVolumeChanged, ws);
	_musicVolumeLabel = new StaticTextWidget(boss, xoffset + _musicVolumeSlider->getWidth() + 10, yoffset + 2, 24, kLineHeight, "100%", kTextAlignLeft, ws);
	_musicVolumeSlider->setMinValue(0);
	_musicVolumeSlider->setMaxValue(Audio::Mixer::kMaxMixerVolume);
	_musicVolumeLabel->setFlags(WIDGET_CLEARBG);
	yoffset += _musicVolumeSlider->getHeight() + 4;

	new StaticTextWidget(boss, 10, yoffset + 2, textwidth, kLineHeight, slider_labels[1], kTextAlignRight, ws);
	_sfxVolumeSlider = addSlider(boss, xoffset, yoffset, kSfxVolumeChanged, ws);
	_sfxVolumeLabel = new StaticTextWidget(boss, xoffset + _musicVolumeSlider->getWidth() + 10, yoffset + 2, 24, kLineHeight, "100%", kTextAlignLeft, ws);
	_sfxVolumeSlider->setMinValue(0);
	_sfxVolumeSlider->setMaxValue(Audio::Mixer::kMaxMixerVolume);
	_sfxVolumeLabel->setFlags(WIDGET_CLEARBG);
	yoffset += _sfxVolumeSlider->getHeight() + 4;

	new StaticTextWidget(boss, 10, yoffset + 2, textwidth, kLineHeight, slider_labels[2], kTextAlignRight, ws);
	_speechVolumeSlider = addSlider(boss, xoffset, yoffset, kSpeechVolumeChanged, ws);
	_speechVolumeLabel = new StaticTextWidget(boss, xoffset + _musicVolumeSlider->getWidth() + 10, yoffset + 2, 24, kLineHeight, "100%", kTextAlignLeft, ws);
	_speechVolumeSlider->setMinValue(0);
	_speechVolumeSlider->setMaxValue(Audio::Mixer::kMaxMixerVolume);
	_speechVolumeLabel->setFlags(WIDGET_CLEARBG);
	yoffset += _speechVolumeSlider->getHeight() + 4;

	_enableVolumeSettings = true;

	return yoffset;
}

#pragma mark -


GlobalOptionsDialog::GlobalOptionsDialog()
	: OptionsDialog(Common::ConfigManager::kApplicationDomain, 10, 40, 320 - 2 * 10, 140) {

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	GUI::WidgetSize ws;
	int buttonWidth, buttonHeight;
	
	if (screenW >= 400 && screenH >= 300) {
		ws = GUI::kBigWidgetSize;
		buttonWidth = kBigButtonWidth;
		buttonHeight = kBigButtonHeight;
		_w = screenW - 2 * 10;
		_h = screenH - 2 * 40;
		_x = 10;
		_y = 40;
	} else {
		ws = GUI::kNormalWidgetSize;
		buttonWidth = kButtonWidth;
		buttonHeight = kButtonHeight;
		_w = screenW - 2 * 10;
		_h = screenH - 1 * 20;
		_x = 10;
		_y = 20;
	}

	const int vBorder = 5;	// Tab border
	int yoffset;

	// The tab widget
	TabWidget *tab = new TabWidget(this, 0, vBorder, _w, _h - buttonHeight - 8 - 2 * vBorder, ws);

	//
	// 1) The graphics tab
	//
	tab->addTab("Graphics");
	yoffset = vBorder;
	yoffset = addGraphicControls(tab, yoffset, ws);

	//
	// 2) The audio tab
	//
	tab->addTab("Audio");
	yoffset = vBorder;
	yoffset = addAudioControls(tab, yoffset, ws);
	yoffset = addVolumeControls(tab, yoffset, ws);
	// TODO: cd drive setting
	
	//
	// 3) The MIDI tab
	//
	tab->addTab("MIDI");
	yoffset = vBorder;
	yoffset = addMIDIControls(tab, yoffset, ws);

	//
	// 4) The miscellaneous tab
	//
	tab->addTab("Paths");
	yoffset = vBorder;

#if !( defined(__DC__) || defined(__GP32__) )
	// Save game path
	addButton(tab, 5, yoffset, "Save Path: ", kChooseSaveDirCmd, 0, ws);
	_savePath = new StaticTextWidget(tab, 5 + buttonWidth + 20, yoffset + 3, _w - (5 + buttonWidth + 20) - 10, kLineHeight, "/foo/bar", kTextAlignLeft, ws);
	yoffset += buttonHeight + 4;

	addButton(tab, 5, yoffset, "Extra Path:", kChooseExtraDirCmd, 0, ws);
	_extraPath = new StaticTextWidget(tab, 5 + buttonWidth + 20, yoffset + 3, _w - (5 + buttonWidth + 20) - 10, kLineHeight, "None", kTextAlignLeft, ws);
	yoffset += buttonHeight + 4;
#endif

#ifdef _WIN32_WCE
	addButton(tab, 5, yoffset, "Keys", kChooseKeyMappingCmd, 0, ws);
	yoffset += buttonHeight + 4;
#endif

	// TODO: joystick setting


	// Activate the first tab
	tab->setActiveTab(0);

	// Add OK & Cancel buttons
	addButton(_w - 2 * (buttonWidth + 10), _h - buttonHeight - 8, "Cancel", kCloseCmd, 0, ws);
	addButton(_w - (buttonWidth + 10), _h - buttonHeight - 8, "OK", kOKCmd, 0, ws);

	// Create file browser dialogs
	_dirBrowser = new BrowserDialog("Select directory for savegames", true);
	_fileBrowser = new BrowserDialog("Select SoundFont", false);

#ifdef _WIN32_WCE
	_keysDialog = new CEKeysDialog();
#endif
}

GlobalOptionsDialog::~GlobalOptionsDialog() {
	delete _dirBrowser;
	delete _fileBrowser;

#ifdef _WIN32_WCE
	delete _keysDialog;
#endif
}

void GlobalOptionsDialog::open() {
	OptionsDialog::open();

#if !( defined(__DC__) || defined(__GP32__) || defined(__PLAYSTATION2__) )
	// Set _savePath to the current save path
	Common::String dir(ConfMan.get("savepath", _domain));
	Common::String extraPath(ConfMan.get("extrapath", _domain));
	Common::String soundFont(ConfMan.get("soundfont", _domain));

	if (!dir.isEmpty()) {
		_savePath->setLabel(dir);
	} else {
		// Default to the current directory...
		char buf[MAXPATHLEN];
		getcwd(buf, sizeof(buf));
		_savePath->setLabel(buf);
	}

	if (extraPath.isEmpty() || !ConfMan.hasKey("extrapath", _domain)) {
		_extraPath->setLabel("None");
	} else {
		_extraPath->setLabel(extraPath);
	}

	if (soundFont.isEmpty() || !ConfMan.hasKey("soundfont", _domain)) {
		_soundFont->setLabel("None");
	} else {
		_soundFont->setLabel(soundFont);
	}
#endif
}

void GlobalOptionsDialog::close() {
	if (getResult()) {
		// Savepath
		ConfMan.set("savepath", _savePath->getLabel(), _domain);

		String extraPath = _extraPath->getLabel();
		if (!extraPath.isEmpty() && (extraPath != "None"))
			ConfMan.set("extrapath", extraPath, _domain);

		String soundFont = _soundFont->getLabel();
		if (!soundFont.isEmpty() && (soundFont != "None"))
			ConfMan.set("soundfont", soundFont, _domain);
	}
	OptionsDialog::close();
}

void GlobalOptionsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kChooseSaveDirCmd:
		if (_dirBrowser->runModal() > 0) {
			// User made his choice...
			FilesystemNode dir(_dirBrowser->getResult());
			_savePath->setLabel(dir.path());
			draw();
			// TODO - we should check if the directory is writeable before accepting it
		}
		break;
	case kChooseExtraDirCmd:
		if (_dirBrowser->runModal() > 0) {
			// User made his choice...
			FilesystemNode dir(_dirBrowser->getResult());
			_extraPath->setLabel(dir.path());
			draw();
		}
		break;
	case kChooseSoundFontCmd:
		if (_fileBrowser->runModal() > 0) {
			// User made his choice...
			FilesystemNode file(_fileBrowser->getResult());
			_soundFont->setLabel(file.path());
			draw();
		}
		break;
#ifdef _WIN32_WCE
	case kChooseKeyMappingCmd:
		_keysDialog->runModal();
		break;
#endif
	default:
		OptionsDialog::handleCommand(sender, cmd, data);
	}
}

} // End of namespace GUI
