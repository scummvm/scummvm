/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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

#include "common/stdafx.h"
#include "gui/browser.h"
#include "gui/chooser.h"
#include "gui/eval.h"
#include "gui/newgui.h"
#include "gui/options.h"
#include "gui/PopUpWidget.h"
#include "gui/TabWidget.h"

#include "backends/fs/fs.h"
#include "common/config-manager.h"
#include "graphics/scaler.h"
#include "common/system.h"
#include "sound/mididrv.h"
#include "sound/mixer.h"

#if (!( defined(PALMOS_MODE) || defined(__DC__) || defined(__GP32__) || defined(__amigaos4__) ) && !defined(_MSC_VER))
#include <sys/param.h>
#include <unistd.h>
#endif

#if !(defined(MAXPATHLEN))
#ifndef PALMOS_MODE
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
// - aspect ratio, language, platform, debug mode/level, cd drive, joystick, multi midi, native mt32

enum {
	kMusicVolumeChanged		= 'muvc',
	kSfxVolumeChanged		= 'sfvc',
	kSubtitleToggle			= 'sttg',
	kSubtitleSpeedChanged	= 'stsc',
	kSpeechVolumeChanged	= 'vcvc',
	kChooseSoundFontCmd		= 'chsf',
	kChooseSaveDirCmd		= 'chos',
	kChooseThemeDirCmd		= 'chth',
	kChooseExtraDirCmd		= 'chex'
};

#ifdef SMALL_SCREEN_DEVICE
enum {
	kChooseKeyMappingCmd    = 'chma'
};
#endif


OptionsDialog::OptionsDialog(const String &domain, int x, int y, int w, int h)
	: Dialog(x, y, w, h), _domain(domain) {
	init();
}

OptionsDialog::OptionsDialog(const String &domain, const String &name)
	: Dialog(name), _domain(domain) {
	init();
}

const char *OptionsDialog::_subModeDesc[] = {
	"Speech Only",
	"Speech and Subtitles",
	"Subtitles Only"
};

void OptionsDialog::init() {
	_enableGraphicSettings = false;
	_gfxPopUp = 0;
	_renderModePopUp = 0;
	_fullscreenCheckbox = 0;
	_aspectCheckbox = 0;
	_enableAudioSettings = false;
	_midiPopUp = 0;
	_enableMIDISettings = false;
	_multiMidiCheckbox = 0;
	_mt32Checkbox = 0;
	_enableGSCheckbox = 0;
	_enableVolumeSettings = false;
	_musicVolumeDesc = 0;
	_musicVolumeSlider = 0;
	_musicVolumeLabel = 0;
	_sfxVolumeDesc = 0;
	_sfxVolumeSlider = 0;
	_sfxVolumeLabel = 0;
	_speechVolumeDesc = 0;
	_speechVolumeSlider = 0;
	_speechVolumeLabel = 0;
	_subToggleDesc = 0;
	_subToggleButton = 0;
	_subSpeedDesc = 0;
	_subSpeedSlider = 0;
	_subSpeedLabel = 0;
}

void OptionsDialog::open() {
	Dialog::open();

	// Reset result value
	setResult(0);

	// Graphic options
	if (_fullscreenCheckbox) {
		_gfxPopUp->setSelected(0);

		if (ConfMan.hasKey("gfx_mode", _domain)) {
			const OSystem::GraphicsMode *gm = g_system->getSupportedGraphicsModes();
			String gfxMode(ConfMan.get("gfx_mode", _domain));
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

#ifndef SMALL_SCREEN_DEVICE
		// Fullscreen setting
		_fullscreenCheckbox->setState(ConfMan.getBool("fullscreen", _domain));

		// Aspect ratio setting
		_aspectCheckbox->setState(ConfMan.getBool("aspect_ratio", _domain));
#endif
	}

	// Audio options
	if (_midiPopUp) {
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
	}

	if (_multiMidiCheckbox) {

		// Multi midi setting
		_multiMidiCheckbox->setState(ConfMan.getBool("multi_midi", _domain));

		// Native mt32 setting
		_mt32Checkbox->setState(ConfMan.getBool("native_mt32", _domain));

		// GS extensions setting
		_enableGSCheckbox->setState(ConfMan.getBool("enable_gs", _domain));

		String soundFont(ConfMan.get("soundfont", _domain));
		if (soundFont.empty() || !ConfMan.hasKey("soundfont", _domain))
			_soundFont->setLabel("None");
		else
			_soundFont->setLabel(soundFont);
	}

	// Volume options
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

	// Subtitle options
	if (_subToggleButton) {
		int speed;
		int sliderMaxValue = _subSpeedSlider->getMaxValue();

		_subMode = getSubtitleMode(ConfMan.getBool("subtitles", _domain), ConfMan.getBool("speech_mute"));
		_subToggleButton->setLabel(_subModeDesc[_subMode]);

		// Engines that reuse the subtitle speed widget set their own max value.
		// Scale the config value accordingly (see addSubtitleControls)
		speed = (ConfMan.getInt("talkspeed", _domain) * sliderMaxValue + 255 / 2) / 255;
		_subSpeedSlider->setValue(speed);
		_subSpeedLabel->setValue(speed);
	}
}

void OptionsDialog::close() {
	if (getResult()) {

		// Graphic options
		if (_fullscreenCheckbox) {
			if (_enableGraphicSettings) {
				ConfMan.setBool("fullscreen", _fullscreenCheckbox->getState(), _domain);
				ConfMan.setBool("aspect_ratio", _aspectCheckbox->getState(), _domain);

				bool isSet = false;

				if ((int32)_gfxPopUp->getSelectedTag() >= 0) {
					const OSystem::GraphicsMode *gm = g_system->getSupportedGraphicsModes();

					while (gm->name) {
						if (gm->id == (int)_gfxPopUp->getSelectedTag()) {
							ConfMan.set("gfx_mode", gm->name, _domain);
							isSet = true;
							break;
						}
						gm++;
					}
				}
				if (!isSet)
					ConfMan.removeKey("gfx_mode", _domain);

				if ((int32)_renderModePopUp->getSelectedTag() >= 0)
					ConfMan.set("render_mode", Common::getRenderModeCode((Common::RenderMode)_renderModePopUp->getSelectedTag()), _domain);
			} else {
				ConfMan.removeKey("fullscreen", _domain);
				ConfMan.removeKey("aspect_ratio", _domain);
				ConfMan.removeKey("gfx_mode", _domain);
				ConfMan.removeKey("render_mode", _domain);
			}
		}

		// Volume options
		if (_musicVolumeSlider) {
			if (_enableVolumeSettings) {
				ConfMan.setInt("music_volume", _musicVolumeSlider->getValue(), _domain);
				ConfMan.setInt("sfx_volume", _sfxVolumeSlider->getValue(), _domain);
				ConfMan.setInt("speech_volume", _speechVolumeSlider->getValue(), _domain);
			} else {
				ConfMan.removeKey("music_volume", _domain);
				ConfMan.removeKey("sfx_volume", _domain);
				ConfMan.removeKey("speech_volume", _domain);
			}
		}

		// Audio options
		if (_midiPopUp) {
			if (_enableAudioSettings) {
				const MidiDriverDescription *md = MidiDriver::getAvailableMidiDrivers();
				while (md->name && md->id != (int)_midiPopUp->getSelectedTag())
					md++;
				if (md->name)
					ConfMan.set("music_driver", md->name, _domain);
				else
					ConfMan.removeKey("music_driver", _domain);
			} else {
				ConfMan.removeKey("music_driver", _domain);
			}
		}

		// MIDI options
		if (_multiMidiCheckbox) {
			if (_enableMIDISettings) {
				ConfMan.setBool("multi_midi", _multiMidiCheckbox->getState(), _domain);
				ConfMan.setBool("native_mt32", _mt32Checkbox->getState(), _domain);
				ConfMan.setBool("enable_gs", _enableGSCheckbox->getState(), _domain);

				String soundFont(_soundFont->getLabel());
				if (!soundFont.empty() && (soundFont != "None"))
					ConfMan.set("soundfont", soundFont, _domain);
			} else {
				ConfMan.removeKey("multi_midi", _domain);
				ConfMan.removeKey("native_mt32", _domain);
				ConfMan.removeKey("enable_gs", _domain);
				ConfMan.removeKey("soundfont", _domain);
			}
		}

		// Subtitle options
		if (_subToggleButton) {
			if (_enableSubtitleSettings) {
				bool subtitles, speech_mute;
				int talkspeed;
				int sliderMaxValue = _subSpeedSlider->getMaxValue();

				switch (_subMode) {
				case 0:
					subtitles = speech_mute = false;
					break;
				case 1:
					subtitles = true;
					speech_mute = false;
					break;
				case 2:
				default:
					subtitles = speech_mute = true;
					break;
				}

				ConfMan.setBool("subtitles", subtitles, _domain); 
				ConfMan.setBool("speech_mute", speech_mute, _domain);

				// Engines that reuse the subtitle speed widget set their own max value.
				// Scale the config value accordingly (see addSubtitleControls)
				talkspeed = (_subSpeedSlider->getValue() * 255 + sliderMaxValue / 2) / sliderMaxValue;
				ConfMan.setInt("talkspeed", talkspeed, _domain);

			} else {
				ConfMan.removeKey("subtitles", _domain);
				ConfMan.removeKey("talkspeed", _domain);
				ConfMan.removeKey("speech_mute", _domain);
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
	case kSubtitleToggle:
		if (_subMode < 2)
			_subMode++;
		else
			_subMode = 0;	

		_subToggleButton->setLabel(_subModeDesc[_subMode]);
		_subToggleButton->draw();
		_subSpeedDesc->draw();
		_subSpeedSlider->draw();
		_subSpeedLabel->draw();
		break;
	case kSubtitleSpeedChanged:
		_subSpeedLabel->setValue(_subSpeedSlider->getValue());
		_subSpeedLabel->draw();
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
#ifndef SMALL_SCREEN_DEVICE
	_fullscreenCheckbox->setEnabled(enabled);
	_aspectCheckbox->setEnabled(enabled);
#endif
}

void OptionsDialog::setAudioSettingsState(bool enabled) {
	_enableAudioSettings = enabled;

	_midiPopUp->setEnabled(enabled);
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

	_musicVolumeDesc->setEnabled(enabled);
	_musicVolumeSlider->setEnabled(enabled);
	_musicVolumeLabel->setEnabled(enabled);
	_sfxVolumeDesc->setEnabled(enabled);
	_sfxVolumeSlider->setEnabled(enabled);
	_sfxVolumeLabel->setEnabled(enabled);
	_speechVolumeDesc->setEnabled(enabled);
	_speechVolumeSlider->setEnabled(enabled);
	_speechVolumeLabel->setEnabled(enabled);
}

void OptionsDialog::setSubtitleSettingsState(bool enabled) {
	_enableSubtitleSettings = enabled;

	_subToggleButton->setEnabled(enabled);
	_subToggleDesc->setEnabled(enabled);
	_subSpeedDesc->setEnabled(enabled);
	_subSpeedSlider->setEnabled(enabled);
	_subSpeedLabel->setEnabled(enabled);
}

void OptionsDialog::addGraphicControls(GuiObject *boss, const String &prefix) {
	const OSystem::GraphicsMode *gm = g_system->getSupportedGraphicsModes();

	int labelWidth = g_gui.evaluator()->getVar("tabPopupsLabelW");

	// The GFX mode popup
	_gfxPopUp = new PopUpWidget(boss, prefix + "grModePopup", "Graphics mode: ", labelWidth);

	_gfxPopUp->appendEntry("<default>");
	_gfxPopUp->appendEntry("");
	while (gm->name) {
		_gfxPopUp->appendEntry(gm->description, gm->id);
		gm++;
	}

	// RenderMode popup
	_renderModePopUp = new PopUpWidget(boss, prefix + "grRenderPopup", "Render mode: ", labelWidth);
	_renderModePopUp->appendEntry("<default>", Common::kRenderDefault);
	_renderModePopUp->appendEntry("");
	const Common::RenderModeDescription *rm = Common::g_renderModes;
	for (; rm->code; ++rm) {
		_renderModePopUp->appendEntry(rm->description, rm->id);
	}

	// Fullscreen checkbox
	_fullscreenCheckbox = new CheckboxWidget(boss, prefix + "grFullscreenCheckbox", "Fullscreen mode", 0, 0);

	// Aspect ratio checkbox
	_aspectCheckbox = new CheckboxWidget(boss, prefix + "grAspectCheckbox", "Aspect ratio correction", 0, 0);

#ifdef SMALL_SCREEN_DEVICE
	_fullscreenCheckbox->setState(true);
	_fullscreenCheckbox->setEnabled(false);
	_aspectCheckbox->setEnabled(false);
#endif

	_enableGraphicSettings = true;
}

void OptionsDialog::addAudioControls(GuiObject *boss, const String &prefix) {
	int labelWidth = g_gui.evaluator()->getVar("tabPopupsLabelW");

	// The MIDI mode popup & a label
	_midiPopUp = new PopUpWidget(boss, prefix + "auMidiPopup", "Music driver: ", labelWidth);

	// Populate it
	const MidiDriverDescription *md = MidiDriver::getAvailableMidiDrivers();
	while (md->name) {
		_midiPopUp->appendEntry(md->description, md->id);
		md++;
	}

	_enableAudioSettings = true;
}

void OptionsDialog::addMIDIControls(GuiObject *boss, const String &prefix) {
	// SoundFont
	_soundFontButton = new ButtonWidget(boss, prefix + "mcFontButton", "SoundFont:", kChooseSoundFontCmd, 0);
	_soundFont = new StaticTextWidget(boss, prefix + "mcFontPath", "None");

	// Multi midi setting
	_multiMidiCheckbox = new CheckboxWidget(boss, prefix + "mcMixedCheckbox", "Mixed Adlib/MIDI mode", 0, 0);

	// Native mt32 setting
	_mt32Checkbox = new CheckboxWidget(boss, prefix + "mcMt32Checkbox", "True Roland MT-32 (disable GM emulation)", 0, 0);

	// GS Extensions setting
	_enableGSCheckbox = new CheckboxWidget(boss, prefix + "mcGSCheckbox", "Enable Roland GS Mode", 0, 0);

	_enableMIDISettings = true;
}

// The function has an extra slider range parameter, since both the launcher and SCUMM engine
// make use of the widgets. The launcher range is 0-255. SCUMM's 0-9
void OptionsDialog::addSubtitleControls(GuiObject *boss, const String &prefix, int maxSliderVal) {

	_subToggleDesc = new StaticTextWidget(boss, prefix + "subToggleDesc", "Text and Speech:");
	_subToggleButton = new ButtonWidget(boss, prefix + "subToggleButton", "", kSubtitleToggle, 0);

	// Subtitle speed
	_subSpeedDesc = new StaticTextWidget(boss, prefix + "subSubtitleSpeedDesc", "Subtitle speed:");
	_subSpeedSlider = new SliderWidget(boss, prefix + "subSubtitleSpeedSlider", kSubtitleSpeedChanged);
	_subSpeedLabel = new StaticTextWidget(boss, prefix + "subSubtitleSpeedLabel", "100%");
	_subSpeedSlider->setMinValue(0); _subSpeedSlider->setMaxValue(maxSliderVal);
	_subSpeedLabel->setFlags(WIDGET_CLEARBG);

	_enableSubtitleSettings = true;
}

void OptionsDialog::addVolumeControls(GuiObject *boss, const String &prefix) {

	// Volume controllers
	_musicVolumeDesc = new StaticTextWidget(boss, prefix + "vcMusicText", "Music volume:");
	_musicVolumeSlider = new SliderWidget(boss, prefix + "vcMusicSlider", kMusicVolumeChanged);
	_musicVolumeLabel = new StaticTextWidget(boss, prefix + "vcMusicLabel", "100%");
	_musicVolumeSlider->setMinValue(0);
	_musicVolumeSlider->setMaxValue(Audio::Mixer::kMaxMixerVolume);
	_musicVolumeLabel->setFlags(WIDGET_CLEARBG);

	_sfxVolumeDesc = new StaticTextWidget(boss, prefix + "vcSfxText", "SFX volume:");
	_sfxVolumeSlider = new SliderWidget(boss, prefix + "vcSfxSlider", kSfxVolumeChanged);
	_sfxVolumeLabel = new StaticTextWidget(boss, prefix + "vcSfxLabel", "100%");
	_sfxVolumeSlider->setMinValue(0);
	_sfxVolumeSlider->setMaxValue(Audio::Mixer::kMaxMixerVolume);
	_sfxVolumeLabel->setFlags(WIDGET_CLEARBG);

	_speechVolumeDesc = new StaticTextWidget(boss, prefix + "vcSpeechText" , "Speech volume:");
	_speechVolumeSlider = new SliderWidget(boss, prefix + "vcSpeechSlider", kSpeechVolumeChanged);
	_speechVolumeLabel = new StaticTextWidget(boss, prefix + "vcSpeechLabel", "100%");
	_speechVolumeSlider->setMinValue(0);
	_speechVolumeSlider->setMaxValue(Audio::Mixer::kMaxMixerVolume);
	_speechVolumeLabel->setFlags(WIDGET_CLEARBG);

	_enableVolumeSettings = true;
}

int OptionsDialog::getSubtitleMode(bool subtitles, bool speech_mute) {
	if (!subtitles && !speech_mute) // Speech only
		return 0;
	else if (subtitles && !speech_mute) // Speech and subtitles
		return 1;
	else if (subtitles && speech_mute) // Subtitles only
		return 2;
	else 
		warning("Wrong configuration: Both subtitles and speech are off. Assuming subtitles only");
	return 2;
}

void OptionsDialog::handleScreenChanged() {
	Dialog::handleScreenChanged();

	int labelWidth = g_gui.evaluator()->getVar("tabPopupsLabelW");

	if (_midiPopUp)
		_midiPopUp->changeLabelWidth(labelWidth);
	if (_gfxPopUp)
		_gfxPopUp->changeLabelWidth(labelWidth);
	if (_renderModePopUp)
		_renderModePopUp->changeLabelWidth(labelWidth);
}

#pragma mark -


GlobalOptionsDialog::GlobalOptionsDialog()
	: OptionsDialog(Common::ConfigManager::kApplicationDomain, "globaloptions") {

	// The tab widget
	TabWidget *tab = new TabWidget(this, "globaloptions_tabwidget");
	tab->setHints(THEME_HINT_FIRST_DRAW | THEME_HINT_SAVE_BACKGROUND);

	//
	// 1) The graphics tab
	//
	tab->addTab("Graphics");
	addGraphicControls(tab, "globaloptions_");

	//
	// 2) The audio tab
	//
	tab->addTab("Audio");
	addAudioControls(tab, "globaloptions_");

	int volControlPos = g_gui.evaluator()->getVar("volumeControlsInAudio", true);

	if (volControlPos)
		addVolumeControls(tab, "globaloptions_");

	addSubtitleControls(tab, "globaloptions_");

	if (!volControlPos) {
		tab->addTab("Volume");
		addVolumeControls(tab, "globaloptions_");
	}

	// TODO: cd drive setting

	//
	// 3) The MIDI tab
	//
	tab->addTab("MIDI");
	addMIDIControls(tab, "globaloptions_");

	//
	// 4) The miscellaneous tab
	//
	tab->addTab("Paths");

#if !( defined(__DC__) || defined(__GP32__) )
	// These two buttons have to be extra wide, or the text will be
	// truncated in the small version of the GUI.

	// Save game path
	new ButtonWidget(tab, "globaloptions_savebutton", "Save Path: ", kChooseSaveDirCmd, 0);
	_savePath = new StaticTextWidget(tab, "globaloptions_savepath", "/foo/bar");

	new ButtonWidget(tab, "globaloptions_themebutton", "Theme Path:", kChooseThemeDirCmd, 0);
	_themePath = new StaticTextWidget(tab, "globaloptions_themepath", "None");

	new ButtonWidget(tab, "globaloptions_extrabutton", "Extra Path:", kChooseExtraDirCmd, 0);
	_extraPath = new StaticTextWidget(tab, "globaloptions_extrapath", "None");
#endif

#ifdef SMALL_SCREEN_DEVICE
	new ButtonWidget(tab, "globaloptions_keysbutton", "Keys", kChooseKeyMappingCmd, 0);
#endif

	// TODO: joystick setting


	// Activate the first tab
	tab->setActiveTab(0);

	// Add OK & Cancel buttons
	new ButtonWidget(this, "globaloptions_cancel", "Cancel", kCloseCmd, 0);
	new ButtonWidget(this, "globaloptions_ok", "OK", kOKCmd, 0);

#ifdef SMALL_SCREEN_DEVICE
	_keysDialog = new KeysDialog();
#endif
}

GlobalOptionsDialog::~GlobalOptionsDialog() {
#ifdef SMALL_SCREEN_DEVICE
	delete _keysDialog;
#endif
}

void GlobalOptionsDialog::open() {
	OptionsDialog::open();

#if !( defined(__DC__) || defined(__GP32__) || defined(__PLAYSTATION2__) )
	// Set _savePath to the current save path
	Common::String savePath(ConfMan.get("savepath", _domain));
	Common::String themePath(ConfMan.get("themepath", _domain));
	Common::String extraPath(ConfMan.get("extrapath", _domain));

	if (!savePath.empty()) {
		_savePath->setLabel(savePath);
	} else {
		// Default to the current directory...
		char buf[MAXPATHLEN];
		getcwd(buf, sizeof(buf));
		_savePath->setLabel(buf);
	}

	if (themePath.empty() || !ConfMan.hasKey("themepath", _domain)) {
		_themePath->setLabel("None");
	} else {
		_themePath->setLabel(themePath);
	}

	if (extraPath.empty() || !ConfMan.hasKey("extrapath", _domain)) {
		_extraPath->setLabel("None");
	} else {
		_extraPath->setLabel(extraPath);
	}
#endif
}

void GlobalOptionsDialog::close() {
	if (getResult()) {
		// Savepath
		ConfMan.set("savepath", _savePath->getLabel(), _domain);

		String themePath(_themePath->getLabel());
		if (!themePath.empty() && (themePath != "None"))
			ConfMan.set("themepath", themePath, _domain);

		String extraPath(_extraPath->getLabel());
		if (!extraPath.empty() && (extraPath != "None"))
			ConfMan.set("extrapath", extraPath, _domain);
	}
	OptionsDialog::close();
}

void GlobalOptionsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kChooseSaveDirCmd: {
		BrowserDialog browser("Select directory for savegames", true);
		if (browser.runModal() > 0) {
			// User made his choice...
			FilesystemNode dir(browser.getResult());
			_savePath->setLabel(dir.path());
			draw();
			// TODO - we should check if the directory is writeable before accepting it
		}
		break;
	}
	case kChooseThemeDirCmd: {
		BrowserDialog browser("Select directory for GUI themes", true);
		if (browser.runModal() > 0) {
			// User made his choice...
			FilesystemNode dir(browser.getResult());
			_themePath->setLabel(dir.path());
			draw();
		}
		break;
	}
	case kChooseExtraDirCmd: {
		BrowserDialog browser("Select directory for extra files", true);
		if (browser.runModal() > 0) {
			// User made his choice...
			FilesystemNode dir(browser.getResult());
			_extraPath->setLabel(dir.path());
			draw();
		}
		break;
	}
	case kChooseSoundFontCmd: {
		BrowserDialog browser("Select SoundFont", false);
		if (browser.runModal() > 0) {
			// User made his choice...
			FilesystemNode file(browser.getResult());
			_soundFont->setLabel(file.path());
			draw();
		}
		break;
	}
#ifdef SMALL_SCREEN_DEVICE
	case kChooseKeyMappingCmd:
		_keysDialog->runModal();
		break;
#endif
	default:
		OptionsDialog::handleCommand(sender, cmd, data);
	}
}

} // End of namespace GUI
