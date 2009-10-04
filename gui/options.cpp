/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "gui/browser.h"
#include "gui/themebrowser.h"
#include "gui/chooser.h"
#include "gui/message.h"
#include "gui/GuiManager.h"
#include "gui/ThemeEval.h"
#include "gui/options.h"
#include "gui/PopUpWidget.h"
#include "gui/TabWidget.h"

#include "common/fs.h"
#include "common/config-manager.h"
#include "common/system.h"

#include "sound/mixer.h"

namespace GUI {

enum {
	kMidiGainChanged		= 'mgch',
	kMusicVolumeChanged		= 'muvc',
	kSfxVolumeChanged		= 'sfvc',
	kSubtitleToggle			= 'sttg',
	kSubtitleSpeedChanged	= 'stsc',
	kSpeechVolumeChanged	= 'vcvc',
	kChooseSoundFontCmd		= 'chsf',
	kClearSoundFontCmd      = 'clsf',
	kChooseSaveDirCmd		= 'chos',
	kChooseThemeDirCmd		= 'chth',
	kChooseExtraDirCmd		= 'chex',
	kChoosePluginsDirCmd	= 'chpl',
	kChooseThemeCmd			= 'chtf'
};

#ifdef SMALL_SCREEN_DEVICE
enum {
	kChooseKeyMappingCmd    = 'chma'
};
#endif

static const char *savePeriodLabels[] = { "Never", "every 5 mins", "every 10 mins", "every 15 mins", "every 30 mins", 0 };
static const int savePeriodValues[] = { 0, 5 * 60, 10 * 60, 15 * 60, 30 * 60, -1 };
static const char *outputRateLabels[] = { "<default>", "22 kHz", "8 kHz", "11kHz", "44 kHz", "48 kHz", 0 };
static const int outputRateValues[] = { 0, 22050, 8000, 11025, 44100, 48000, -1 };



OptionsDialog::OptionsDialog(const String &domain, int x, int y, int w, int h)
	: Dialog(x, y, w, h), _domain(domain), _graphicsTabId(-1), _tabWidget(0) {
	init();
}

OptionsDialog::OptionsDialog(const String &domain, const String &name)
	: Dialog(name), _domain(domain), _graphicsTabId(-1), _tabWidget(0) {
	init();
}

const char *OptionsDialog::_subModeDesc[] = {
	"Speech Only",
	"Speech and Subtitles",
	"Subtitles Only"
};

const char *OptionsDialog::_lowresSubModeDesc[] = {
	"Speech Only",
	"Speech & Subs",
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
	_oplPopUp = 0;
	_outputRatePopUp = 0;
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
	_muteCheckbox = 0;
	_subToggleDesc = 0;
	_subToggleButton = 0;
	_subSpeedDesc = 0;
	_subSpeedSlider = 0;
	_subSpeedLabel = 0;

	// Retrieve game GUI options
	_guioptions = 0;
	if (ConfMan.hasKey("guioptions", _domain))
		_guioptions = parseGameGUIOptions(ConfMan.get("guioptions", _domain));
}

void OptionsDialog::open() {
	Dialog::open();

	// Reset result value
	setResult(0);

	// Retrieve game GUI options
	_guioptions = 0;
	if (ConfMan.hasKey("guioptions", _domain))
		_guioptions = parseGameGUIOptions(ConfMan.get("guioptions", _domain));

	// Graphic options
	if (_fullscreenCheckbox) {


#ifdef SMALL_SCREEN_DEVICE
		_fullscreenCheckbox->setState(true);
		_fullscreenCheckbox->setEnabled(false);
		_aspectCheckbox->setEnabled(false);
#else // !SMALL_SCREEN_DEVICE
		// Fullscreen setting
		_fullscreenCheckbox->setState(ConfMan.getBool("fullscreen", _domain));
#endif // SMALL_SCREEN_DEVICE
	}

	if (_outputRatePopUp) {
		_outputRatePopUp->setSelected(1);
		int value = ConfMan.getInt("output_rate", _domain);
		for	(int i = 0; outputRateLabels[i]; i++) {
			if (value == outputRateValues[i])
				_outputRatePopUp->setSelected(i);
		}
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
}

void OptionsDialog::close() {
	if (getResult()) {

		// Graphic options
		if (_fullscreenCheckbox) {
			if (_enableGraphicSettings) {
				ConfMan.setBool("fullscreen", _fullscreenCheckbox->getState(), _domain);
			} else {
				ConfMan.removeKey("fullscreen", _domain);
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

		if (_outputRatePopUp) {
			if (_enableAudioSettings) {
				if (_outputRatePopUp->getSelectedTag() != 0)
					ConfMan.setInt("output_rate", _outputRatePopUp->getSelectedTag(), _domain);
				else
					ConfMan.removeKey("output_rate", _domain);
			} else {
				ConfMan.removeKey("output_rate", _domain);
			}
		}

		// Save config file
		ConfMan.flushToDisk();
	}

	Dialog::close();
}

void OptionsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	char buf[10];

	switch (cmd) {
	case kMidiGainChanged:
		sprintf(buf, "%.2f", (double)_midiGainSlider->getValue() / 100.0);
		_midiGainLabel->setLabel(buf);
		_midiGainLabel->draw();
		break;
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

		_subToggleButton->setLabel(g_system->getOverlayWidth() > 320 ? _subModeDesc[_subMode] : _lowresSubModeDesc[_subMode]);
		_subToggleButton->draw();
		_subSpeedDesc->draw();
		_subSpeedSlider->draw();
		_subSpeedLabel->draw();
		break;
	case kSubtitleSpeedChanged:
		_subSpeedLabel->setValue(_subSpeedSlider->getValue());
		_subSpeedLabel->draw();
		break;
	case kClearSoundFontCmd:
		_soundFont->setLabel("None");
		_soundFontClearButton->setEnabled(false);
		draw();
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

	_gfxPopUpDesc->setEnabled(enabled);
	_gfxPopUp->setEnabled(enabled);
	_renderModePopUpDesc->setEnabled(enabled);
	_renderModePopUp->setEnabled(enabled);
#ifndef SMALL_SCREEN_DEVICE
	_fullscreenCheckbox->setEnabled(enabled);
	_aspectCheckbox->setEnabled(enabled);
#endif
}

void OptionsDialog::setAudioSettingsState(bool enabled) {
	_enableAudioSettings = enabled;

	_midiPopUpDesc->setEnabled(enabled);
	_midiPopUp->setEnabled(enabled);
	_oplPopUpDesc->setEnabled(enabled);
	_oplPopUp->setEnabled(enabled);
	_outputRatePopUpDesc->setEnabled(enabled);
	_outputRatePopUp->setEnabled(enabled);
}

void OptionsDialog::setMIDISettingsState(bool enabled) {
	if (_guioptions & Common::GUIO_NOMIDI)
		enabled = false;

	_enableMIDISettings = enabled;

	_soundFontButton->setEnabled(enabled);
	_soundFont->setEnabled(enabled);

	if (enabled && !_soundFont->getLabel().empty() && (_soundFont->getLabel() != "None"))
		_soundFontClearButton->setEnabled(enabled);
	else
		_soundFontClearButton->setEnabled(false);

	_multiMidiCheckbox->setEnabled(enabled);
	_mt32Checkbox->setEnabled(enabled);
	_enableGSCheckbox->setEnabled(enabled);
	_midiGainDesc->setEnabled(enabled);
	_midiGainSlider->setEnabled(enabled);
	_midiGainLabel->setEnabled(enabled);
}

void OptionsDialog::setVolumeSettingsState(bool enabled) {
	bool ena;

	_enableVolumeSettings = enabled;

	ena = enabled;
	if (_guioptions & Common::GUIO_NOMUSIC)
		ena = false;

	_musicVolumeDesc->setEnabled(ena);
	_musicVolumeSlider->setEnabled(ena);
	_musicVolumeLabel->setEnabled(ena);

	ena = enabled;
	if (_guioptions & Common::GUIO_NOSFX)
		ena = false;

	_sfxVolumeDesc->setEnabled(ena);
	_sfxVolumeSlider->setEnabled(ena);
	_sfxVolumeLabel->setEnabled(ena);

	ena = enabled;
	if (_guioptions & Common::GUIO_NOSPEECH)
		ena = false;

	_speechVolumeDesc->setEnabled(ena);
	_speechVolumeSlider->setEnabled(ena);
	_speechVolumeLabel->setEnabled(ena);
}

void OptionsDialog::setSubtitleSettingsState(bool enabled) {
	bool ena;
	_enableSubtitleSettings = enabled;

	ena = enabled;
	if ((_guioptions & Common::GUIO_NOSUBTITLES) || (_guioptions & Common::GUIO_NOSPEECH))
		ena = false;

	_subToggleButton->setEnabled(ena);
	_subToggleDesc->setEnabled(ena);

	ena = enabled;
	if (_guioptions & Common::GUIO_NOSUBTITLES)
		ena = false;

	_subSpeedDesc->setEnabled(ena);
	_subSpeedSlider->setEnabled(ena);
	_subSpeedLabel->setEnabled(ena);
}

void OptionsDialog::addGraphicControls(GuiObject *boss, const String &prefix) {
	// Fullscreen checkbox
	_fullscreenCheckbox = new CheckboxWidget(boss, prefix + "grFullscreenCheckbox", "Fullscreen mode", 0, 0);
	_enableGraphicSettings = true;
}

void OptionsDialog::addAudioControls(GuiObject *boss, const String &prefix) {
	// Sample rate settings
	_outputRatePopUpDesc = new StaticTextWidget(boss, prefix + "auSampleRatePopupDesc", "Output rate:");
	_outputRatePopUp = new PopUpWidget(boss, prefix + "auSampleRatePopup");

	for (int i = 0; outputRateLabels[i]; i++) {
		_outputRatePopUp->appendEntry(outputRateLabels[i], outputRateValues[i]);
	}

	_enableAudioSettings = true;
}

void OptionsDialog::addMIDIControls(GuiObject *boss, const String &prefix) {
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
	if (_guioptions & Common::GUIO_NOSUBTITLES)
		return 0; // Speech only
	if (_guioptions & Common::GUIO_NOSPEECH)
		return 2; // Subtitles only

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

void OptionsDialog::reflowLayout() {
	Dialog::reflowLayout();

	if (_graphicsTabId != -1 && _tabWidget)
		_tabWidget->setTabTitle(_graphicsTabId, g_system->getOverlayWidth() > 320 ? "Graphics" : "GFX");
}

#pragma mark -


GlobalOptionsDialog::GlobalOptionsDialog()
	: OptionsDialog(Common::ConfigManager::kApplicationDomain, "GlobalOptions") {

	// The tab widget
	TabWidget *tab = new TabWidget(this, "GlobalOptions.TabWidget");

	//
	// 1) The graphics tab
	//
	_graphicsTabId = tab->addTab(g_system->getOverlayWidth() > 320 ? "Graphics" : "GFX");
	addGraphicControls(tab, "GlobalOptions_Graphics.");

	//
	// 2) The audio tab
	//
	tab->addTab("Audio");
	addAudioControls(tab, "GlobalOptions_Audio.");

	tab->addTab("Volume");
	addVolumeControls(tab, "GlobalOptions_Volume.");

	// TODO: cd drive setting

	//
	// 4) The miscellaneous tab
	//
	tab->addTab("Paths");

#if !( defined(__DC__) || defined(__GP32__) )
	// These two buttons have to be extra wide, or the text will be
	// truncated in the small version of the GUI.

	// Save game path
	new ButtonWidget(tab, "GlobalOptions_Paths.SaveButton", "Save Path: ", kChooseSaveDirCmd, 0);
	_savePath = new StaticTextWidget(tab, "GlobalOptions_Paths.SavePath", "/foo/bar");

	new ButtonWidget(tab, "GlobalOptions_Paths.ThemeButton", "Theme Path:", kChooseThemeDirCmd, 0);
	_themePath = new StaticTextWidget(tab, "GlobalOptions_Paths.ThemePath", "None");

	new ButtonWidget(tab, "GlobalOptions_Paths.ExtraButton", "Extra Path:", kChooseExtraDirCmd, 0);
	_extraPath = new StaticTextWidget(tab, "GlobalOptions_Paths.ExtraPath", "None");

#ifdef DYNAMIC_MODULES
	new ButtonWidget(tab, "GlobalOptions_Paths.PluginsButton", "Plugins Path:", kChoosePluginsDirCmd, 0);
	_pluginsPath = new StaticTextWidget(tab, "GlobalOptions_Paths.PluginsPath", "None");
#endif
#endif

	tab->addTab("Misc");

	new ButtonWidget(tab, "GlobalOptions_Misc.ThemeButton", "Theme:", kChooseThemeCmd, 0);
	_curTheme = new StaticTextWidget(tab, "GlobalOptions_Misc.CurTheme", g_gui.theme()->getThemeName());


	_rendererPopUpDesc = new StaticTextWidget(tab, "GlobalOptions_Misc.RendererPopupDesc", "GUI Renderer:");
	_rendererPopUp = new PopUpWidget(tab, "GlobalOptions_Misc.RendererPopup");

	for (uint i = 1; i < GUI::ThemeEngine::_rendererModesSize; ++i)
		_rendererPopUp->appendEntry(GUI::ThemeEngine::_rendererModes[i].name, GUI::ThemeEngine::_rendererModes[i].mode);

	_autosavePeriodPopUpDesc = new StaticTextWidget(tab, "GlobalOptions_Misc.AutosavePeriodPopupDesc", "Autosave:");
	_autosavePeriodPopUp = new PopUpWidget(tab, "GlobalOptions_Misc.AutosavePeriodPopup");

	for (int i = 0; savePeriodLabels[i]; i++) {
		_autosavePeriodPopUp->appendEntry(savePeriodLabels[i], savePeriodValues[i]);
	}

#ifdef SMALL_SCREEN_DEVICE
	new ButtonWidget(tab, "GlobalOptions_Misc.KeysButton", "Keys", kChooseKeyMappingCmd, 0);
#endif

	// TODO: joystick setting


	// Activate the first tab
	tab->setActiveTab(0);
	_tabWidget = tab;

	// Add OK & Cancel buttons
	new ButtonWidget(this, "GlobalOptions.Cancel", "Cancel", kCloseCmd, 0);
	new ButtonWidget(this, "GlobalOptions.Ok", "OK", kOKCmd, 0);

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

#if !( defined(__DC__) || defined(__GP32__) )
	// Set _savePath to the current save path
	Common::String savePath(ConfMan.get("savepath", _domain));
	Common::String themePath(ConfMan.get("themepath", _domain));
	Common::String extraPath(ConfMan.get("extrapath", _domain));

	if (savePath.empty() || !ConfMan.hasKey("savepath", _domain)) {
		_savePath->setLabel("None");
	} else {
		_savePath->setLabel(savePath);
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

#ifdef DYNAMIC_MODULES
	Common::String pluginsPath(ConfMan.get("pluginspath", _domain));
	if (pluginsPath.empty() || !ConfMan.hasKey("pluginspath", _domain)) {
		_pluginsPath->setLabel("None");
	} else {
		_pluginsPath->setLabel(pluginsPath);
	}
#endif
#endif

	// Misc Tab
	_autosavePeriodPopUp->setSelected(1);
	int value = ConfMan.getInt("autosave_period");
	for (int i = 0; savePeriodLabels[i]; i++) {
		if (value == savePeriodValues[i])
			_autosavePeriodPopUp->setSelected(i);
	}

	ThemeEngine::GraphicsMode mode = ThemeEngine::findMode(ConfMan.get("gui_renderer"));
	if (mode == ThemeEngine::kGfxDisabled)
		mode = ThemeEngine::_defaultRendererMode;
	_rendererPopUp->setSelectedTag(mode);
}

void GlobalOptionsDialog::close() {
	if (getResult()) {
		String savePath(_savePath->getLabel());
		if (!savePath.empty() && (savePath != "None"))
			ConfMan.set("savepath", savePath, _domain);

		String themePath(_themePath->getLabel());
		if (!themePath.empty() && (themePath != "None"))
			ConfMan.set("themepath", themePath, _domain);
		else
			ConfMan.removeKey("themepath", _domain);

		String extraPath(_extraPath->getLabel());
		if (!extraPath.empty() && (extraPath != "None"))
			ConfMan.set("extrapath", extraPath, _domain);
		else
			ConfMan.removeKey("extrapath", _domain);

#ifdef DYNAMIC_MODULES
		String pluginsPath(_pluginsPath->getLabel());
		if (!pluginsPath.empty() && (pluginsPath != "None"))
			ConfMan.set("pluginspath", pluginsPath, _domain);
		else
			ConfMan.removeKey("pluginspath", _domain);
#endif

		ConfMan.setInt("autosave_period", _autosavePeriodPopUp->getSelectedTag(), _domain);

		GUI::ThemeEngine::GraphicsMode selected = (GUI::ThemeEngine::GraphicsMode)_rendererPopUp->getSelectedTag();
		const char *cfg = GUI::ThemeEngine::findModeConfigName(selected);
		if (!ConfMan.get("gui_renderer").equalsIgnoreCase(cfg)) {
			// FIXME: Actually, any changes (including the theme change) should
			// only become active *after* the options dialog has closed.
			g_gui.loadNewTheme(g_gui.theme()->getThemeId(), selected);
			ConfMan.set("gui_renderer", cfg, _domain);
		}
	}
	OptionsDialog::close();
}

void GlobalOptionsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kChooseSaveDirCmd: {
		BrowserDialog browser("Select directory for savegames", true);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(browser.getResult());
			if (dir.isWritable()) {
				_savePath->setLabel(dir.getPath());
			} else {
				MessageDialog error("The chosen directory cannot be written to. Please select another one.");
				error.runModal();
				return;
			}
			draw();
		}
		break;
	}
	case kChooseThemeDirCmd: {
		BrowserDialog browser("Select directory for GUI themes", true);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(browser.getResult());
			_themePath->setLabel(dir.getPath());
			draw();
		}
		break;
	}
	case kChooseExtraDirCmd: {
		BrowserDialog browser("Select directory for extra files", true);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(browser.getResult());
			_extraPath->setLabel(dir.getPath());
			draw();
		}
		break;
	}
#ifdef DYNAMIC_MODULES
	case kChoosePluginsDirCmd: {
		BrowserDialog browser("Select directory for plugins", true);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(browser.getResult());
			_pluginsPath->setLabel(dir.getPath());
			draw();
		}
		break;
	}
#endif
	case kChooseSoundFontCmd: {
		BrowserDialog browser("Select SoundFont", false);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode file(browser.getResult());
			_soundFont->setLabel(file.getPath());

			if (!file.getPath().empty() && (file.getPath() != "None"))
				_soundFontClearButton->setEnabled(true);
			else
				_soundFontClearButton->setEnabled(false);

			draw();
		}
		break;
	}
	case kChooseThemeCmd: {
		ThemeBrowser browser;
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::String theme = browser.getSelected();
			// FIXME: Actually, any changes (including the theme change) should
			// only become active *after* the options dialog has closed.
			if (g_gui.loadNewTheme(theme)) {
				_curTheme->setLabel(g_gui.theme()->getThemeName());
				ConfMan.set("gui_theme", theme);
			}
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
