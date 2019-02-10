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
 */

#include "gui/browser.h"
#include "gui/themebrowser.h"
#include "gui/message.h"
#include "gui/gui-manager.h"
#include "gui/options.h"
#include "gui/widgets/popup.h"
#include "gui/widgets/tab.h"
#include "gui/ThemeEval.h"
#include "gui/launcher.h"

#include "common/fs.h"
#include "common/config-manager.h"
#include "common/gui_options.h"
#include "common/rendermode.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "common/updates.h"

#include "audio/mididrv.h"
#include "audio/musicplugin.h"
#include "audio/mixer.h"
#include "audio/fmopl.h"
#include "widgets/scrollcontainer.h"
#include "widgets/edittext.h"

#ifdef USE_CLOUD
#ifdef USE_LIBCURL
#include "backends/cloud/cloudmanager.h"
#include "gui/downloaddialog.h"
#include "gui/storagewizarddialog.h"
#endif

#ifdef USE_SDL_NET
#include "backends/networking/sdl_net/localwebserver.h"
#endif
#endif

namespace GUI {

enum {
	kMidiGainChanged		= 'mgch',
	kMusicVolumeChanged		= 'muvc',
	kSfxVolumeChanged		= 'sfvc',
	kMuteAllChanged			= 'mute',
	kSubtitleToggle			= 'sttg',
	kSubtitleSpeedChanged	= 'stsc',
	kSpeechVolumeChanged	= 'vcvc',
	kChooseSoundFontCmd		= 'chsf',
	kClearSoundFontCmd      = 'clsf',
	kChooseSaveDirCmd		= 'chos',
	kSavePathClearCmd		= 'clsp',
	kChooseThemeDirCmd		= 'chth',
	kThemePathClearCmd		= 'clth',
	kChooseExtraDirCmd		= 'chex',
	kExtraPathClearCmd		= 'clex',
	kChoosePluginsDirCmd	= 'chpl',
	kChooseThemeCmd			= 'chtf',
	kUpdatesCheckCmd		= 'updc',
	kKbdMouseSpeedChanged	= 'kmsc',
	kJoystickDeadzoneChanged= 'jodc',
	kGraphicsTabContainerReflowCmd = 'gtcr'
};

enum {
	kSubtitlesSpeech,
	kSubtitlesSubs,
	kSubtitlesBoth
};

#ifdef GUI_ENABLE_KEYSDIALOG
enum {
	kChooseKeyMappingCmd    = 'chma'
};
#endif

#ifdef USE_FLUIDSYNTH
enum {
	kFluidSynthSettingsCmd		= 'flst'
};
#endif

#ifdef USE_CLOUD
enum {
	kConfigureStorageCmd = 'cfst',
	kRefreshStorageCmd = 'rfst',
	kDownloadStorageCmd = 'dlst',
	kRunServerCmd = 'rnsv',
	kCloudTabContainerReflowCmd = 'ctcr',
	kServerPortClearCmd = 'spcl',
	kChooseRootDirCmd = 'chrp',
	kRootPathClearCmd = 'clrp'
};
#endif

enum {
	kApplyCmd = 'appl'
};

static const char *savePeriodLabels[] = { _s("Never"), _s("Every 5 mins"), _s("Every 10 mins"), _s("Every 15 mins"), _s("Every 30 mins"), 0 };
static const int savePeriodValues[] = { 0, 5 * 60, 10 * 60, 15 * 60, 30 * 60, -1 };
// The keyboard mouse speed values range from 0 to 7 and correspond to speeds shown in the label
// "10" (value 3) is the default speed corresponding to the speed before introduction of this control
static const char *kbdMouseSpeedLabels[] = { "3", "5", "8", "10", "13", "15", "18", "20", 0 };

OptionsDialog::OptionsDialog(const Common::String &domain, int x, int y, int w, int h)
	: Dialog(x, y, w, h), _domain(domain), _graphicsTabId(-1), _midiTabId(-1), _pathsTabId(-1), _tabWidget(0) {
	init();
}

OptionsDialog::OptionsDialog(const Common::String &domain, const Common::String &name)
	: Dialog(name), _domain(domain), _graphicsTabId(-1), _midiTabId(-1), _pathsTabId(-1), _tabWidget(0) {
	init();
}

OptionsDialog::~OptionsDialog() {
	delete _subToggleGroup;
}

void OptionsDialog::init() {
	_enableControlSettings = false;
	_onscreenCheckbox = 0;
	_touchpadCheckbox = 0;
	_swapMenuAndBackBtnsCheckbox = 0;
	_kbdMouseSpeedDesc = 0;
	_kbdMouseSpeedSlider = 0;
	_kbdMouseSpeedLabel = 0;
	_joystickDeadzoneDesc = 0;
	_joystickDeadzoneSlider = 0;
	_joystickDeadzoneLabel = 0;
	_enableGraphicSettings = false;
	_gfxPopUp = 0;
	_gfxPopUpDesc = 0;
	_renderModePopUp = 0;
	_renderModePopUpDesc = 0;
	_stretchPopUp = 0;
	_stretchPopUpDesc = 0;
	_fullscreenCheckbox = 0;
	_filteringCheckbox = 0;
	_aspectCheckbox = 0;
	_enableShaderSettings = false;
	_shaderPopUpDesc = 0;
	_shaderPopUp = 0;
	_enableAudioSettings = false;
	_midiPopUp = 0;
	_midiPopUpDesc = 0;
	_oplPopUp = 0;
	_oplPopUpDesc = 0;
	_enableMIDISettings = false;
	_gmDevicePopUp = 0;
	_gmDevicePopUpDesc = 0;
	_soundFont = 0;
	_soundFontButton = 0;
	_soundFontClearButton = 0;
	_multiMidiCheckbox = 0;
	_midiGainDesc = 0;
	_midiGainSlider = 0;
	_midiGainLabel = 0;
	_enableMT32Settings = false;
	_mt32Checkbox = 0;
	_mt32DevicePopUp = 0;
	_mt32DevicePopUpDesc = 0;
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
	_enableSubtitleSettings = false;
	_subToggleDesc = 0;
	_subToggleGroup = 0;
	_subToggleSubOnly = 0;
	_subToggleSpeechOnly = 0;
	_subToggleSubBoth = 0;
	_subSpeedDesc = 0;
	_subSpeedSlider = 0;
	_subSpeedLabel = 0;

	// Retrieve game GUI options
	_guioptions.clear();
	if (ConfMan.hasKey("guioptions", _domain)) {
		_guioptionsString = ConfMan.get("guioptions", _domain);
		_guioptions = parseGameGUIOptions(_guioptionsString);
	}
}

void OptionsDialog::build() {
	// Retrieve game GUI options
	_guioptions.clear();
	if (ConfMan.hasKey("guioptions", _domain)) {
		_guioptionsString = ConfMan.get("guioptions", _domain);
		_guioptions = parseGameGUIOptions(_guioptionsString);
	}

	// Control options
	if (g_system->hasFeature(OSystem::kFeatureOnScreenControl)) {
		if (ConfMan.hasKey("onscreen_control", _domain)) {
			bool onscreenState =  g_system->getFeatureState(OSystem::kFeatureOnScreenControl);
			if (_onscreenCheckbox != 0)
				_onscreenCheckbox->setState(onscreenState);
		}
	}
	if (g_system->hasFeature(OSystem::kFeatureTouchpadMode)) {
		if (ConfMan.hasKey("touchpad_mouse_mode", _domain)) {
			bool touchpadState =  g_system->getFeatureState(OSystem::kFeatureTouchpadMode);
			if (_touchpadCheckbox != 0)
				_touchpadCheckbox->setState(touchpadState);
		}
	}
	if (g_system->hasFeature(OSystem::kFeatureSwapMenuAndBackButtons)) {
		if (ConfMan.hasKey("swap_menu_and_back_buttons", _domain)) {
			bool state =  g_system->getFeatureState(OSystem::kFeatureSwapMenuAndBackButtons);
			if (_swapMenuAndBackBtnsCheckbox != 0)
				_swapMenuAndBackBtnsCheckbox->setState(state);
		}
	}
	if (g_system->hasFeature(OSystem::kFeatureKbdMouseSpeed)) {
		int value = ConfMan.getInt("kbdmouse_speed", _domain);
		if (_kbdMouseSpeedSlider && value < ARRAYSIZE(kbdMouseSpeedLabels) - 1 && value >= 0) {
			_kbdMouseSpeedSlider->setValue(value);
			_kbdMouseSpeedLabel->setLabel(_(kbdMouseSpeedLabels[value]));
		}
	}
	if (g_system->hasFeature(OSystem::kFeatureJoystickDeadzone)) {
		int value = ConfMan.getInt("joystick_deadzone", _domain);
		if (_joystickDeadzoneSlider != 0) {
			_joystickDeadzoneSlider->setValue(value);
			_joystickDeadzoneLabel->setValue(value);
		}
	}

	// Graphic options
	if (_fullscreenCheckbox) {
		_gfxPopUp->setSelected(0);

		if (ConfMan.hasKey("gfx_mode", _domain)) {
			const OSystem::GraphicsMode *gm = g_system->getSupportedGraphicsModes();
			Common::String gfxMode(ConfMan.get("gfx_mode", _domain));
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
					sel = p->id;
			}
			_renderModePopUp->setSelectedTag(sel);
		}

		_stretchPopUp->setSelected(0);

		if (g_system->hasFeature(OSystem::kFeatureStretchMode)) {
			if (ConfMan.hasKey("stretch_mode", _domain)) {
				const OSystem::GraphicsMode *sm = g_system->getSupportedStretchModes();
				Common::String stretchMode(ConfMan.get("stretch_mode", _domain));
				int stretchCount = 1;
				while (sm->name) {
					stretchCount++;
					if (scumm_stricmp(sm->name, stretchMode.c_str()) == 0)
						_stretchPopUp->setSelected(stretchCount);
					sm++;
				}
			}
		} else {
			_stretchPopUpDesc->setVisible(false);
			_stretchPopUp->setVisible(false);
		}

#ifdef GUI_ONLY_FULLSCREEN
		_fullscreenCheckbox->setState(true);
		_fullscreenCheckbox->setEnabled(false);
#else // !GUI_ONLY_FULLSCREEN
		// Fullscreen setting
		_fullscreenCheckbox->setState(ConfMan.getBool("fullscreen", _domain));
#endif // GUI_ONLY_FULLSCREEN

		// Filtering setting
		if (g_system->hasFeature(OSystem::kFeatureFilteringMode))
			_filteringCheckbox->setState(ConfMan.getBool("filtering", _domain));
		else
			_filteringCheckbox->setVisible(false);

		// Aspect ratio setting
		if (_guioptions.contains(GUIO_NOASPECT)) {
			_aspectCheckbox->setState(false);
			_aspectCheckbox->setEnabled(false);
		} else {
			_aspectCheckbox->setEnabled(true);
			_aspectCheckbox->setState(ConfMan.getBool("aspect_ratio", _domain));
		}

	}

	// Shader options
	if (g_system->hasFeature(OSystem::kFeatureShader)) {
		if (_shaderPopUp) {
			int value = ConfMan.getInt("shader", _domain);
			_shaderPopUp->setSelected(value);
		}
	}

	// Audio options
	if (!loadMusicDeviceSetting(_midiPopUp, "music_driver"))
		_midiPopUp->setSelected(0);

	if (_oplPopUp) {
		OPL::Config::DriverId id = MAX<OPL::Config::DriverId>(OPL::Config::parse(ConfMan.get("opl_driver", _domain)), 0);
		_oplPopUp->setSelectedTag(id);
	}

	if (_multiMidiCheckbox) {
		if (!loadMusicDeviceSetting(_gmDevicePopUp, "gm_device"))
			_gmDevicePopUp->setSelected(0);

		// Multi midi setting
		_multiMidiCheckbox->setState(ConfMan.getBool("multi_midi", _domain));

		Common::String soundFont(ConfMan.get("soundfont", _domain));
		if (soundFont.empty() || !ConfMan.hasKey("soundfont", _domain)) {
			_soundFont->setLabel(_c("None", "soundfont"));
			_soundFontClearButton->setEnabled(false);
		} else {
			_soundFont->setLabel(soundFont);
			_soundFontClearButton->setEnabled(true);
		}

		// MIDI gain setting
		_midiGainSlider->setValue(ConfMan.getInt("midi_gain", _domain));
		_midiGainLabel->setLabel(Common::String::format("%.2f", (double)_midiGainSlider->getValue() / 100.0));
	}

	// MT-32 options
	if (_mt32DevicePopUp) {
		if (!loadMusicDeviceSetting(_mt32DevicePopUp, "mt32_device"))
			_mt32DevicePopUp->setSelected(0);

		// Native mt32 setting
		_mt32Checkbox->setState(ConfMan.getBool("native_mt32", _domain));

		// GS extensions setting
		_enableGSCheckbox->setState(ConfMan.getBool("enable_gs", _domain));
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

		bool val = false;
		if (ConfMan.hasKey("mute", _domain)) {
			val = ConfMan.getBool("mute", _domain);
		} else {
			ConfMan.setBool("mute", false);
		}
		_muteCheckbox->setState(val);
	}

	// Subtitle options
	if (_subToggleGroup) {
		int speed;
		int sliderMaxValue = _subSpeedSlider->getMaxValue();

		int subMode = getSubtitleMode(ConfMan.getBool("subtitles", _domain), ConfMan.getBool("speech_mute", _domain));
		_subToggleGroup->setValue(subMode);

		// Engines that reuse the subtitle speed widget set their own max value.
		// Scale the config value accordingly (see addSubtitleControls)
		speed = (ConfMan.getInt("talkspeed", _domain) * sliderMaxValue + 255 / 2) / 255;
		_subSpeedSlider->setValue(speed);
		_subSpeedLabel->setValue(speed);
	}
}

void OptionsDialog::clean() {
	delete _subToggleGroup;
	while (_firstWidget) {
		Widget* w = _firstWidget;
		removeWidget(w);
		// This is called from rebuild() which may result from handleCommand being called by
		// a child widget sendCommand call. In such a case sendCommand is still being executed
		// so we should not delete yet the child widget. Thus delay the deletion.
		g_gui.addToTrash(w, this);
	}
	init();
}

void OptionsDialog::rebuild() {
	int currentTab = _tabWidget->getActiveTab();
	clean();
	build();
	reflowLayout();
	_tabWidget->setActiveTab(currentTab);
	setDefaultFocusedWidget();
}

void OptionsDialog::open() {
	build();

	Dialog::open();

	// Reset result value
	setResult(0);
}

void OptionsDialog::apply() {
	bool graphicsModeChanged = false;

	// Graphic options
	if (_fullscreenCheckbox) {
		if (_enableGraphicSettings) {
			if (ConfMan.getBool("filtering", _domain) != _filteringCheckbox->getState())
				graphicsModeChanged = true;
			if (ConfMan.getBool("fullscreen", _domain) != _fullscreenCheckbox->getState())
				graphicsModeChanged = true;
			if (ConfMan.getBool("aspect_ratio", _domain) != _aspectCheckbox->getState())
				graphicsModeChanged = true;

			ConfMan.setBool("filtering", _filteringCheckbox->getState(), _domain);
			ConfMan.setBool("fullscreen", _fullscreenCheckbox->getState(), _domain);
			ConfMan.setBool("aspect_ratio", _aspectCheckbox->getState(), _domain);

			bool isSet = false;

			if ((int32)_gfxPopUp->getSelectedTag() >= 0) {
				const OSystem::GraphicsMode *gm = g_system->getSupportedGraphicsModes();

				while (gm->name) {
					if (gm->id == (int)_gfxPopUp->getSelectedTag()) {
						if (ConfMan.get("gfx_mode", _domain) != gm->name)
							graphicsModeChanged = true;
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

			isSet = false;
			if ((int32)_stretchPopUp->getSelectedTag() >= 0) {
				const OSystem::GraphicsMode *sm = g_system->getSupportedStretchModes();
				while (sm->name) {
					if (sm->id == (int)_stretchPopUp->getSelectedTag()) {
						if (ConfMan.get("stretch_mode", _domain) != sm->name)
							graphicsModeChanged = true;
						ConfMan.set("stretch_mode", sm->name, _domain);
						isSet = true;
						break;
					}
					sm++;
				}
			}
			if (!isSet)
				ConfMan.removeKey("stretch_mode", _domain);
		} else {
			ConfMan.removeKey("fullscreen", _domain);
			ConfMan.removeKey("filtering", _domain);
			ConfMan.removeKey("aspect_ratio", _domain);
			ConfMan.removeKey("gfx_mode", _domain);
			ConfMan.removeKey("stretch_mode", _domain);
			ConfMan.removeKey("render_mode", _domain);
		}
	}

	// Setup graphics again if needed
	if (_domain == Common::ConfigManager::kApplicationDomain && graphicsModeChanged) {
		g_system->beginGFXTransaction();
		g_system->setGraphicsMode(ConfMan.get("gfx_mode", _domain).c_str());

		if (ConfMan.hasKey("stretch_mode"))
			g_system->setStretchMode(ConfMan.get("stretch_mode", _domain).c_str());
		if (ConfMan.hasKey("aspect_ratio"))
			g_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, ConfMan.getBool("aspect_ratio", _domain));
		if (ConfMan.hasKey("fullscreen"))
			g_system->setFeatureState(OSystem::kFeatureFullscreenMode, ConfMan.getBool("fullscreen", _domain));
		if (ConfMan.hasKey("filtering"))
			g_system->setFeatureState(OSystem::kFeatureFilteringMode, ConfMan.getBool("filtering", _domain));

		OSystem::TransactionError gfxError = g_system->endGFXTransaction();

		// Since this might change the screen resolution we need to give
		// the GUI a chance to update it's internal state. Otherwise we might
		// get a crash when the GUI tries to grab the overlay.
		//
		// This fixes bug #3303501 "Switching from HQ2x->HQ3x crashes ScummVM"
		//
		// It is important that this is called *before* any of the current
		// dialog's widgets are destroyed (for example before
		// Dialog::close) is called, to prevent crashes caused by invalid
		// widgets being referenced or similar errors.
		g_gui.checkScreenChange();

		if (gfxError != OSystem::kTransactionSuccess) {
			// Revert ConfMan to what OSystem is using.
			Common::String message = _("Failed to apply some of the graphic options changes:");

			if (gfxError & OSystem::kTransactionModeSwitchFailed) {
				const OSystem::GraphicsMode *gm = g_system->getSupportedGraphicsModes();
				while (gm->name) {
					if (gm->id == g_system->getGraphicsMode()) {
						ConfMan.set("gfx_mode", gm->name, _domain);
						break;
					}
					gm++;
				}
				message += "\n";
				message += _("the video mode could not be changed");
			}

			if (gfxError & OSystem::kTransactionStretchModeSwitchFailed) {
				const OSystem::GraphicsMode *sm = g_system->getSupportedStretchModes();
				while (sm->name) {
					if (sm->id == g_system->getStretchMode()) {
						ConfMan.set("stretch_mode", sm->name, _domain);
						break;
					}
					sm++;
				}
				message += "\n";
				message += _("the stretch mode could not be changed");
			}

			if (gfxError & OSystem::kTransactionAspectRatioFailed) {
				ConfMan.setBool("aspect_ratio", g_system->getFeatureState(OSystem::kFeatureAspectRatioCorrection), _domain);
				message += "\n";
				message += _("the aspect ratio setting could not be changed");
			}

			if (gfxError & OSystem::kTransactionFullscreenFailed) {
				ConfMan.setBool("fullscreen", g_system->getFeatureState(OSystem::kFeatureFullscreenMode), _domain);
				message += "\n";
				message += _("the fullscreen setting could not be changed");
			}

			if (gfxError & OSystem::kTransactionFilteringFailed) {
				ConfMan.setBool("filtering", g_system->getFeatureState(OSystem::kFeatureFilteringMode), _domain);
				message += "\n";
				message += _("the filtering setting could not be changed");
			}

			// And display the error
			GUI::MessageDialog dialog(message);
			dialog.runModal();
		}
	}

	// Shader options
	if (_enableShaderSettings) {
		if (g_system->hasFeature(OSystem::kFeatureShader)) {
			if (_shaderPopUp) {
				if (ConfMan.getInt("shader", _domain) != (int32)_shaderPopUp->getSelectedTag()) {
					ConfMan.setInt("shader", _shaderPopUp->getSelectedTag(), _domain);
					g_system->setShader(_shaderPopUp->getSelectedTag());
				}
			}
		}
	}

	// Control options
	if (_enableControlSettings) {
		if (g_system->hasFeature(OSystem::kFeatureOnScreenControl)) {
			if (ConfMan.getBool("onscreen_control", _domain) != _onscreenCheckbox->getState()) {
				g_system->setFeatureState(OSystem::kFeatureOnScreenControl, _onscreenCheckbox->getState());
			}
		}
		if (g_system->hasFeature(OSystem::kFeatureTouchpadMode)) {
			if (ConfMan.getBool("touchpad_mouse_mode", _domain) != _touchpadCheckbox->getState()) {
				g_system->setFeatureState(OSystem::kFeatureTouchpadMode, _touchpadCheckbox->getState());
			}
		}
		if (g_system->hasFeature(OSystem::kFeatureSwapMenuAndBackButtons)) {
			if (ConfMan.getBool("swap_menu_and_back_buttons", _domain) != _swapMenuAndBackBtnsCheckbox->getState()) {
				g_system->setFeatureState(OSystem::kFeatureSwapMenuAndBackButtons, _swapMenuAndBackBtnsCheckbox->getState());
			}
		}
		if (g_system->hasFeature(OSystem::kFeatureKbdMouseSpeed)) {
			if (ConfMan.getInt("kbdmouse_speed", _domain) != _kbdMouseSpeedSlider->getValue()) {
				ConfMan.setInt("kbdmouse_speed", _kbdMouseSpeedSlider->getValue(), _domain);
			}
		}
		if (g_system->hasFeature(OSystem::kFeatureJoystickDeadzone)) {
			if (ConfMan.getInt("joystick_deadzone", _domain) != _joystickDeadzoneSlider->getValue()) {
				ConfMan.setInt("joystick_deadzone", _joystickDeadzoneSlider->getValue(), _domain);
			}
		}
	}

	// Volume options
	if (_musicVolumeSlider) {
		if (_enableVolumeSettings) {
			ConfMan.setInt("music_volume", _musicVolumeSlider->getValue(), _domain);
			ConfMan.setInt("sfx_volume", _sfxVolumeSlider->getValue(), _domain);
			ConfMan.setInt("speech_volume", _speechVolumeSlider->getValue(), _domain);
			ConfMan.setBool("mute", _muteCheckbox->getState(), _domain);
		} else {
			ConfMan.removeKey("music_volume", _domain);
			ConfMan.removeKey("sfx_volume", _domain);
			ConfMan.removeKey("speech_volume", _domain);
			ConfMan.removeKey("mute", _domain);
		}
	}

	// Audio options
	if (_midiPopUp) {
		if (_enableAudioSettings) {
			saveMusicDeviceSetting(_midiPopUp, "music_driver");
		} else {
			ConfMan.removeKey("music_driver", _domain);
		}
	}

	if (_oplPopUp) {
		if (_enableAudioSettings) {
			const OPL::Config::EmulatorDescription *ed = OPL::Config::findDriver(_oplPopUp->getSelectedTag());

			if (ed)
				ConfMan.set("opl_driver", ed->name, _domain);
			else
				ConfMan.removeKey("opl_driver", _domain);
		} else {
			ConfMan.removeKey("opl_driver", _domain);
		}
	}

	// MIDI options
	if (_multiMidiCheckbox) {
		if (_enableMIDISettings) {
			saveMusicDeviceSetting(_gmDevicePopUp, "gm_device");

			ConfMan.setBool("multi_midi", _multiMidiCheckbox->getState(), _domain);
			ConfMan.setInt("midi_gain", _midiGainSlider->getValue(), _domain);

			Common::String soundFont(_soundFont->getLabel());
			if (!soundFont.empty() && (soundFont != _c("None", "soundfont")))
				ConfMan.set("soundfont", soundFont, _domain);
			else
				ConfMan.removeKey("soundfont", _domain);
		} else {
			ConfMan.removeKey("gm_device", _domain);
			ConfMan.removeKey("multi_midi", _domain);
			ConfMan.removeKey("midi_gain", _domain);
			ConfMan.removeKey("soundfont", _domain);
		}
	}

	// MT-32 options
	if (_mt32DevicePopUp) {
		if (_enableMT32Settings) {
			saveMusicDeviceSetting(_mt32DevicePopUp, "mt32_device");
			ConfMan.setBool("native_mt32", _mt32Checkbox->getState(), _domain);
			ConfMan.setBool("enable_gs", _enableGSCheckbox->getState(), _domain);
		} else {
			ConfMan.removeKey("mt32_device", _domain);
			ConfMan.removeKey("native_mt32", _domain);
			ConfMan.removeKey("enable_gs", _domain);
		}
	}

	// Subtitle options
	if (_subToggleGroup) {
		if (_enableSubtitleSettings) {
			bool subtitles, speech_mute;
			int talkspeed;
			int sliderMaxValue = _subSpeedSlider->getMaxValue();

			switch (_subToggleGroup->getValue()) {
				case kSubtitlesSpeech:
					subtitles = speech_mute = false;
					break;
				case kSubtitlesBoth:
					subtitles = true;
					speech_mute = false;
					break;
				case kSubtitlesSubs:
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

void OptionsDialog::close() {
	if (getResult())
		apply();

	Dialog::close();
}

void OptionsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kMidiGainChanged:
		_midiGainLabel->setLabel(Common::String::format("%.2f", (double)_midiGainSlider->getValue() / 100.0));
		_midiGainLabel->markAsDirty();
		break;
	case kMusicVolumeChanged: {
		const int newValue = _musicVolumeSlider->getValue();
		_musicVolumeLabel->setValue(newValue);
		_musicVolumeLabel->markAsDirty();

		if (_guioptions.contains(GUIO_LINKMUSICTOSFX)) {
			updateSfxVolume(newValue);

			if (_guioptions.contains(GUIO_LINKSPEECHTOSFX)) {
				updateSpeechVolume(newValue);
			}
		}

		break;
	}
	case kSfxVolumeChanged: {
		const int newValue = _sfxVolumeSlider->getValue();
		_sfxVolumeLabel->setValue(_sfxVolumeSlider->getValue());
		_sfxVolumeLabel->markAsDirty();

		if (_guioptions.contains(GUIO_LINKMUSICTOSFX)) {
			updateMusicVolume(newValue);
		}

		if (_guioptions.contains(GUIO_LINKSPEECHTOSFX)) {
			updateSpeechVolume(newValue);
		}

		break;
	}
	case kSpeechVolumeChanged: {
		const int newValue = _speechVolumeSlider->getValue();
		_speechVolumeLabel->setValue(newValue);
		_speechVolumeLabel->markAsDirty();

		if (_guioptions.contains(GUIO_LINKSPEECHTOSFX)) {
			updateSfxVolume(newValue);

			if (_guioptions.contains(GUIO_LINKMUSICTOSFX)) {
				updateMusicVolume(newValue);
			}
		}

		break;
	}
	case kMuteAllChanged:
		// 'true' because if control is disabled then event do not pass
		setVolumeSettingsState(true);
		break;
	case kSubtitleToggle:
		// We update the slider settings here, when there are sliders, to
		// disable the speech volume in case we are in subtitle only mode.
		if (_musicVolumeSlider)
			setVolumeSettingsState(true);
		break;
	case kSubtitleSpeedChanged:
		_subSpeedLabel->setValue(_subSpeedSlider->getValue());
		_subSpeedLabel->markAsDirty();
		break;
	case kClearSoundFontCmd:
		_soundFont->setLabel(_c("None", "soundfont"));
		_soundFontClearButton->setEnabled(false);
		g_gui.scheduleTopDialogRedraw();
		break;
	case kKbdMouseSpeedChanged:
		_kbdMouseSpeedLabel->setLabel(_(kbdMouseSpeedLabels[_kbdMouseSpeedSlider->getValue()]));
		_kbdMouseSpeedLabel->markAsDirty();
		break;
	case kJoystickDeadzoneChanged:
		_joystickDeadzoneLabel->setValue(_joystickDeadzoneSlider->getValue());
		_joystickDeadzoneLabel->markAsDirty();
		break;
	case kGraphicsTabContainerReflowCmd:
		setupGraphicsTab();
		break;
	case kApplyCmd:
		apply();
		break;
	case kOKCmd:
		setResult(1);
		close();
		break;
	case kCloseCmd:
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
	_stretchPopUpDesc->setEnabled(enabled);
	_stretchPopUp->setEnabled(enabled);
	_filteringCheckbox->setEnabled(enabled);
#ifndef GUI_ENABLE_KEYSDIALOG
#ifndef GUI_ONLY_FULLSCREEN
	_fullscreenCheckbox->setEnabled(enabled);
#endif // !GUI_ONLY_FULLSCREEN
	if (_guioptions.contains(GUIO_NOASPECT))
		_aspectCheckbox->setEnabled(false);
	else
		_aspectCheckbox->setEnabled(enabled);
#endif // !GUI_ENABLE_KEYSDIALOG
}

void OptionsDialog::setAudioSettingsState(bool enabled) {
	_enableAudioSettings = enabled;
	_midiPopUpDesc->setEnabled(enabled);
	_midiPopUp->setEnabled(enabled);

	const Common::String allFlags = MidiDriver::musicType2GUIO((uint32)-1);
	bool hasMidiDefined = (strpbrk(_guioptions.c_str(), allFlags.c_str()) != NULL);

	if (_domain != Common::ConfigManager::kApplicationDomain && // global dialog
		hasMidiDefined && // No flags are specified
		!(_guioptions.contains(GUIO_MIDIADLIB))) {
		_oplPopUpDesc->setEnabled(false);
		_oplPopUp->setEnabled(false);
	} else {
		_oplPopUpDesc->setEnabled(enabled);
		_oplPopUp->setEnabled(enabled);
	}
}

void OptionsDialog::setMIDISettingsState(bool enabled) {
	if (_guioptions.contains(GUIO_NOMIDI))
		enabled = false;

	_gmDevicePopUpDesc->setEnabled(_domain.equals(Common::ConfigManager::kApplicationDomain) ? enabled : false);
	_gmDevicePopUp->setEnabled(_domain.equals(Common::ConfigManager::kApplicationDomain) ? enabled : false);

	_enableMIDISettings = enabled;

	_soundFontButton->setEnabled(enabled);
	_soundFont->setEnabled(enabled);

	if (enabled && !_soundFont->getLabel().empty() && (_soundFont->getLabel() != _c("None", "soundfont")))
		_soundFontClearButton->setEnabled(enabled);
	else
		_soundFontClearButton->setEnabled(false);

	_multiMidiCheckbox->setEnabled(enabled);
	_midiGainDesc->setEnabled(enabled);
	_midiGainSlider->setEnabled(enabled);
	_midiGainLabel->setEnabled(enabled);
}

void OptionsDialog::setMT32SettingsState(bool enabled) {
	_enableMT32Settings = enabled;

	_mt32DevicePopUpDesc->setEnabled(_domain.equals(Common::ConfigManager::kApplicationDomain) ? enabled : false);
	_mt32DevicePopUp->setEnabled(_domain.equals(Common::ConfigManager::kApplicationDomain) ? enabled : false);

	_mt32Checkbox->setEnabled(enabled);
	_enableGSCheckbox->setEnabled(enabled);
}

void OptionsDialog::setVolumeSettingsState(bool enabled) {
	bool ena;

	_enableVolumeSettings = enabled;

	ena = enabled && !_muteCheckbox->getState();
	if (_guioptions.contains(GUIO_NOMUSIC))
		ena = false;

	_musicVolumeDesc->setEnabled(ena);
	_musicVolumeSlider->setEnabled(ena);
	_musicVolumeLabel->setEnabled(ena);

	ena = enabled && !_muteCheckbox->getState();
	if (_guioptions.contains(GUIO_NOSFX))
		ena = false;

	_sfxVolumeDesc->setEnabled(ena);
	_sfxVolumeSlider->setEnabled(ena);
	_sfxVolumeLabel->setEnabled(ena);

	ena = enabled && !_muteCheckbox->getState();
	// Disable speech volume slider, when we are in subtitle only mode.
	if (_subToggleGroup)
		ena = ena && _subToggleGroup->getValue() != kSubtitlesSubs;
	if (_guioptions.contains(GUIO_NOSPEECH) || _guioptions.contains(GUIO_NOSPEECHVOLUME))
		ena = false;

	_speechVolumeDesc->setEnabled(ena);
	_speechVolumeSlider->setEnabled(ena);
	_speechVolumeLabel->setEnabled(ena);

	_muteCheckbox->setEnabled(enabled);
}

void OptionsDialog::setSubtitleSettingsState(bool enabled) {
	bool ena;
	_enableSubtitleSettings = enabled;

	ena = enabled;
	if ((_guioptions.contains(GUIO_NOSUBTITLES)) || (_guioptions.contains(GUIO_NOSPEECH)))
		ena = false;

	_subToggleGroup->setEnabled(ena);
	_subToggleDesc->setEnabled(ena);

	ena = enabled;
	if (_guioptions.contains(GUIO_NOSUBTITLES))
		ena = false;

	_subSpeedDesc->setEnabled(ena);
	_subSpeedSlider->setEnabled(ena);
	_subSpeedLabel->setEnabled(ena);
}

void OptionsDialog::addControlControls(GuiObject *boss, const Common::String &prefix) {
	// Show On-Screen control
	if (g_system->hasFeature(OSystem::kFeatureOnScreenControl))
		_onscreenCheckbox = new CheckboxWidget(boss, prefix + "grOnScreenCheckbox", _("Show On-screen control"));

	// Touchpad Mouse mode
	if (g_system->hasFeature(OSystem::kFeatureTouchpadMode))
		_touchpadCheckbox = new CheckboxWidget(boss, prefix + "grTouchpadCheckbox", _("Touchpad mouse mode"));

	// Swap menu and back buttons
	if (g_system->hasFeature(OSystem::kFeatureSwapMenuAndBackButtons))
		_swapMenuAndBackBtnsCheckbox = new CheckboxWidget(boss, prefix + "grSwapMenuAndBackBtnsCheckbox", _("Swap Menu and Back buttons"));

	// Keyboard and joystick mouse speed
	if (g_system->hasFeature(OSystem::kFeatureKbdMouseSpeed)) {
		if (g_system->getOverlayWidth() > 320)
			_kbdMouseSpeedDesc = new StaticTextWidget(boss, prefix + "grKbdMouseSpeedDesc", _("Pointer Speed:"), _("Speed for keyboard/joystick mouse pointer control"));
		else
			_kbdMouseSpeedDesc = new StaticTextWidget(boss, prefix + "grKbdMouseSpeedDesc", _c("Pointer Speed:", "lowres"), _("Speed for keyboard/joystick mouse pointer control"));
		_kbdMouseSpeedSlider = new SliderWidget(boss, prefix + "grKbdMouseSpeedSlider", _("Speed for keyboard/joystick mouse pointer control"), kKbdMouseSpeedChanged);
		_kbdMouseSpeedLabel = new StaticTextWidget(boss, prefix + "grKbdMouseSpeedLabel", "  ");
		_kbdMouseSpeedSlider->setMinValue(0);
		_kbdMouseSpeedSlider->setMaxValue(7);
		_kbdMouseSpeedLabel->setFlags(WIDGET_CLEARBG);
	}

	// Joystick deadzone
	if (g_system->hasFeature(OSystem::kFeatureJoystickDeadzone)) {
		if (g_system->getOverlayWidth() > 320)
			_joystickDeadzoneDesc = new StaticTextWidget(boss, prefix + "grJoystickDeadzoneDesc", _("Joy Deadzone:"), _("Analog joystick Deadzone"));
		else
			_joystickDeadzoneDesc = new StaticTextWidget(boss, prefix + "grJoystickDeadzoneDesc", _c("Joy Deadzone:", "lowres"), _("Analog joystick Deadzone"));
		_joystickDeadzoneSlider = new SliderWidget(boss, prefix + "grJoystickDeadzoneSlider", _("Analog joystick Deadzone"), kJoystickDeadzoneChanged);
		_joystickDeadzoneLabel = new StaticTextWidget(boss, prefix + "grJoystickDeadzoneLabel", "  ");
		_joystickDeadzoneSlider->setMinValue(1);
		_joystickDeadzoneSlider->setMaxValue(10);
		_joystickDeadzoneLabel->setFlags(WIDGET_CLEARBG);
	}
	_enableControlSettings = true;
}

void OptionsDialog::addShaderControls(GuiObject *boss, const Common::String &prefix) {
	// Shader selector
	if (g_system->hasFeature(OSystem::kFeatureShader)) {
		if (g_system->getOverlayWidth() > 320)
			_shaderPopUpDesc = new StaticTextWidget(boss, prefix + "grShaderPopUpDesc", _("HW Shader:"), _("Different hardware shaders give different visual effects"));
		else
			_shaderPopUpDesc = new StaticTextWidget(boss, prefix + "grShaderPopUpDesc", _c("HW Shader:", "lowres"), _("Different hardware shaders give different visual effects"));
		_shaderPopUp = new PopUpWidget(boss, prefix + "grShaderPopUp", _("Different shaders give different visual effects"));
		const OSystem::GraphicsMode *p = g_system->getSupportedShaders();
		while (p->name) {
			_shaderPopUp->appendEntry(p->name, p->id);
			p++;
		}
	}
	_enableShaderSettings = true;
}

void OptionsDialog::addGraphicControls(GuiObject *boss, const Common::String &prefix) {
	const OSystem::GraphicsMode *gm = g_system->getSupportedGraphicsModes();
	Common::String context;
	if (g_system->getOverlayWidth() <= 320)
		context = "lowres";

	// The GFX mode popup
	_gfxPopUpDesc = new StaticTextWidget(boss, prefix + "grModePopupDesc", _("Graphics mode:"));
	_gfxPopUp = new PopUpWidget(boss, prefix + "grModePopup");

	_gfxPopUp->appendEntry(_("<default>"));
	_gfxPopUp->appendEntry("");
	while (gm->name) {
		_gfxPopUp->appendEntry(_c(gm->description, context), gm->id);
		gm++;
	}

	// RenderMode popup
	const Common::String allFlags = Common::allRenderModesGUIOs();
	bool renderingTypeDefined = (strpbrk(_guioptions.c_str(), allFlags.c_str()) != NULL);

	_renderModePopUpDesc = new StaticTextWidget(boss, prefix + "grRenderPopupDesc", _("Render mode:"), _("Special dithering modes supported by some games"));
	_renderModePopUp = new PopUpWidget(boss, prefix + "grRenderPopup", _("Special dithering modes supported by some games"));
	_renderModePopUp->appendEntry(_("<default>"), Common::kRenderDefault);
	_renderModePopUp->appendEntry("");
	const Common::RenderModeDescription *rm = Common::g_renderModes;
	for (; rm->code; ++rm) {
		Common::String renderGuiOption = Common::renderMode2GUIO(rm->id);
		if ((_domain == Common::ConfigManager::kApplicationDomain) || (_domain != Common::ConfigManager::kApplicationDomain && !renderingTypeDefined) || (_guioptions.contains(renderGuiOption)))
			_renderModePopUp->appendEntry(_c(rm->description, context), rm->id);
	}

	// The Stretch mode popup
	const OSystem::GraphicsMode *sm = g_system->getSupportedStretchModes();
	_stretchPopUpDesc = new StaticTextWidget(boss, prefix + "grStretchModePopupDesc", _("Stretch mode:"));
	_stretchPopUp = new PopUpWidget(boss, prefix + "grStretchModePopup");

	_stretchPopUp->appendEntry(_("<default>"));
	_stretchPopUp->appendEntry("");
	while (sm->name) {
		_stretchPopUp->appendEntry(_c(sm->description, context), sm->id);
		sm++;
	}

	// Fullscreen checkbox
	_fullscreenCheckbox = new CheckboxWidget(boss, prefix + "grFullscreenCheckbox", _("Fullscreen mode"));

	// Filtering checkbox
	_filteringCheckbox = new CheckboxWidget(boss, prefix + "grFilteringCheckbox", _("Filter graphics"), _("Use linear filtering when scaling graphics"));

	// Aspect ratio checkbox
	_aspectCheckbox = new CheckboxWidget(boss, prefix + "grAspectCheckbox", _("Aspect ratio correction"), _("Correct aspect ratio for 320x200 games"));

	_enableGraphicSettings = true;
}

void OptionsDialog::addAudioControls(GuiObject *boss, const Common::String &prefix) {
	// The MIDI mode popup & a label
	if (g_system->getOverlayWidth() > 320)
		_midiPopUpDesc = new StaticTextWidget(boss, prefix + "auMidiPopupDesc", _domain == Common::ConfigManager::kApplicationDomain ? _("Preferred device:") : _("Music device:"), _domain == Common::ConfigManager::kApplicationDomain ? _("Specifies preferred sound device or sound card emulator") : _("Specifies output sound device or sound card emulator"));
	else
		_midiPopUpDesc = new StaticTextWidget(boss, prefix + "auMidiPopupDesc", _domain == Common::ConfigManager::kApplicationDomain ? _c("Preferred dev.:", "lowres") : _c("Music device:", "lowres"), _domain == Common::ConfigManager::kApplicationDomain ? _("Specifies preferred sound device or sound card emulator") : _("Specifies output sound device or sound card emulator"));
	_midiPopUp = new PopUpWidget(boss, prefix + "auMidiPopup", _("Specifies output sound device or sound card emulator"));

	// Populate it
	const Common::String allFlags = MidiDriver::musicType2GUIO((uint32)-1);
	bool hasMidiDefined = (strpbrk(_guioptions.c_str(), allFlags.c_str()) != NULL);

	const PluginList p = MusicMan.getPlugins();
	for (PluginList::const_iterator m = p.begin(); m != p.end(); ++m) {
		MusicDevices i = (*m)->get<MusicPluginObject>().getDevices();
		for (MusicDevices::iterator d = i.begin(); d != i.end(); ++d) {
			Common::String deviceGuiOption = MidiDriver::musicType2GUIO(d->getMusicType());

			if ((_domain == Common::ConfigManager::kApplicationDomain && d->getMusicType() != MT_TOWNS  // global dialog - skip useless FM-Towns, C64, Amiga, AppleIIGS options there
				 && d->getMusicType() != MT_C64 && d->getMusicType() != MT_AMIGA && d->getMusicType() != MT_APPLEIIGS && d->getMusicType() != MT_PC98)
				|| (_domain != Common::ConfigManager::kApplicationDomain && !hasMidiDefined) // No flags are specified
				|| (_guioptions.contains(deviceGuiOption)) // flag is present
				// HACK/FIXME: For now we have to show GM devices, even when the game only has GUIO_MIDIMT32 set,
				// else we would not show for example external devices connected via ALSA, since they are always
				// marked as General MIDI device.
				|| (deviceGuiOption.contains(GUIO_MIDIGM) && (_guioptions.contains(GUIO_MIDIMT32)))
				|| d->getMusicDriverId() == "auto" || d->getMusicDriverId() == "null") // always add default and null device
				_midiPopUp->appendEntry(d->getCompleteName(), d->getHandle());
		}
	}

	// The OPL emulator popup & a label
	_oplPopUpDesc = new StaticTextWidget(boss, prefix + "auOPLPopupDesc", _("AdLib emulator:"), _("AdLib is used for music in many games"));
	_oplPopUp = new PopUpWidget(boss, prefix + "auOPLPopup", _("AdLib is used for music in many games"));

	// Populate it
	const OPL::Config::EmulatorDescription *ed = OPL::Config::getAvailable();
	while (ed->name) {
		_oplPopUp->appendEntry(_(ed->description), ed->id);
		++ed;
	}

	_enableAudioSettings = true;
}

void OptionsDialog::addMIDIControls(GuiObject *boss, const Common::String &prefix) {
	_gmDevicePopUpDesc = new StaticTextWidget(boss, prefix + "auPrefGmPopupDesc", _("GM device:"), _("Specifies default sound device for General MIDI output"));
	_gmDevicePopUp = new PopUpWidget(boss, prefix + "auPrefGmPopup");

	// Populate
	const PluginList p = MusicMan.getPlugins();
	// Make sure the null device is the first one in the list to avoid undesired
	// auto detection for users who don't have a saved setting yet.
	for (PluginList::const_iterator m = p.begin(); m != p.end(); ++m) {
		MusicDevices i = (*m)->get<MusicPluginObject>().getDevices();
		for (MusicDevices::iterator d = i.begin(); d != i.end(); ++d) {
			if (d->getMusicDriverId() == "null")
				_gmDevicePopUp->appendEntry(_("Don't use General MIDI music"), d->getHandle());
		}
	}
	// Now we add the other devices.
	for (PluginList::const_iterator m = p.begin(); m != p.end(); ++m) {
		MusicDevices i = (*m)->get<MusicPluginObject>().getDevices();
		for (MusicDevices::iterator d = i.begin(); d != i.end(); ++d) {
			if (d->getMusicType() >= MT_GM) {
				if (d->getMusicType() != MT_MT32)
					_gmDevicePopUp->appendEntry(d->getCompleteName(), d->getHandle());
			} else if (d->getMusicDriverId() == "auto") {
				_gmDevicePopUp->appendEntry(_("Use first available device"), d->getHandle());
			}
		}
	}

	if (!_domain.equals(Common::ConfigManager::kApplicationDomain)) {
		_gmDevicePopUpDesc->setEnabled(false);
		_gmDevicePopUp->setEnabled(false);
	}

	// SoundFont
	if (g_system->getOverlayWidth() > 320)
		_soundFontButton = new ButtonWidget(boss, prefix + "mcFontButton", _("SoundFont:"), _("SoundFont is supported by some audio cards, FluidSynth and Timidity"), kChooseSoundFontCmd);
	else
		_soundFontButton = new ButtonWidget(boss, prefix + "mcFontButton", _c("SoundFont:", "lowres"), _("SoundFont is supported by some audio cards, FluidSynth and Timidity"), kChooseSoundFontCmd);
	_soundFont = new StaticTextWidget(boss, prefix + "mcFontPath", _c("None", "soundfont"), _("SoundFont is supported by some audio cards, FluidSynth and Timidity"));

	_soundFontClearButton = addClearButton(boss, prefix + "mcFontClearButton", kClearSoundFontCmd);

	// Multi midi setting
	_multiMidiCheckbox = new CheckboxWidget(boss, prefix + "mcMixedCheckbox", _("Mixed AdLib/MIDI mode"), _("Use both MIDI and AdLib sound generation"));

	// MIDI gain setting (FluidSynth uses this)
	_midiGainDesc = new StaticTextWidget(boss, prefix + "mcMidiGainText", _("MIDI gain:"));
	_midiGainSlider = new SliderWidget(boss, prefix + "mcMidiGainSlider", 0, kMidiGainChanged);
	_midiGainSlider->setMinValue(0);
	_midiGainSlider->setMaxValue(1000);
	_midiGainLabel = new StaticTextWidget(boss, prefix + "mcMidiGainLabel", "1.00");

	_enableMIDISettings = true;
}

void OptionsDialog::addMT32Controls(GuiObject *boss, const Common::String &prefix) {
	_mt32DevicePopUpDesc = new StaticTextWidget(boss, prefix + "auPrefMt32PopupDesc", _("MT-32 Device:"), _("Specifies default sound device for Roland MT-32/LAPC1/CM32l/CM64 output"));
	_mt32DevicePopUp = new PopUpWidget(boss, prefix + "auPrefMt32Popup");

	// Native mt32 setting
	if (g_system->getOverlayWidth() > 320)
		_mt32Checkbox = new CheckboxWidget(boss, prefix + "mcMt32Checkbox", _("True Roland MT-32 (disable GM emulation)"), _("Check if you want to use your real hardware Roland-compatible sound device connected to your computer"));
	else
		_mt32Checkbox = new CheckboxWidget(boss, prefix + "mcMt32Checkbox", _c("True Roland MT-32 (no GM emulation)", "lowres"), _("Check if you want to use your real hardware Roland-compatible sound device connected to your computer"));

	// GS Extensions setting
	_enableGSCheckbox = new CheckboxWidget(boss, prefix + "mcGSCheckbox", _("Roland GS device (enable MT-32 mappings)"), _("Check if you want to enable patch mappings to emulate an MT-32 on a Roland GS device"));

	const PluginList p = MusicMan.getPlugins();
	// Make sure the null device is the first one in the list to avoid undesired
	// auto detection for users who don't have a saved setting yet.
	for (PluginList::const_iterator m = p.begin(); m != p.end(); ++m) {
		MusicDevices i = (*m)->get<MusicPluginObject>().getDevices();
		for (MusicDevices::iterator d = i.begin(); d != i.end(); ++d) {
			if (d->getMusicDriverId() == "null")
				_mt32DevicePopUp->appendEntry(_("Don't use Roland MT-32 music"), d->getHandle());
		}
	}
	// Now we add the other devices.
	for (PluginList::const_iterator m = p.begin(); m != p.end(); ++m) {
		MusicDevices i = (*m)->get<MusicPluginObject>().getDevices();
		for (MusicDevices::iterator d = i.begin(); d != i.end(); ++d) {
			if (d->getMusicType() >= MT_GM)
				_mt32DevicePopUp->appendEntry(d->getCompleteName(), d->getHandle());
			else if (d->getMusicDriverId() == "auto")
				_mt32DevicePopUp->appendEntry(_("Use first available device"), d->getHandle());
		}
	}

	if (!_domain.equals(Common::ConfigManager::kApplicationDomain)) {
		_mt32DevicePopUpDesc->setEnabled(false);
		_mt32DevicePopUp->setEnabled(false);
	} 

	_enableMT32Settings = true;
}

// The function has an extra slider range parameter, since both the launcher and SCUMM engine
// make use of the widgets. The launcher range is 0-255. SCUMM's 0-9
void OptionsDialog::addSubtitleControls(GuiObject *boss, const Common::String &prefix, int maxSliderVal) {

	if (g_system->getOverlayWidth() > 320) {
		_subToggleDesc = new StaticTextWidget(boss, prefix + "subToggleDesc", _("Text and speech:"));

		_subToggleGroup = new RadiobuttonGroup(boss, kSubtitleToggle);

		_subToggleSpeechOnly = new RadiobuttonWidget(boss, prefix + "subToggleSpeechOnly", _subToggleGroup, kSubtitlesSpeech, _("Speech"));
		_subToggleSubOnly = new RadiobuttonWidget(boss, prefix + "subToggleSubOnly", _subToggleGroup, kSubtitlesSubs, _("Subtitles"));
		_subToggleSubBoth = new RadiobuttonWidget(boss, prefix + "subToggleSubBoth", _subToggleGroup, kSubtitlesBoth, _("Both"));

		_subSpeedDesc = new StaticTextWidget(boss, prefix + "subSubtitleSpeedDesc", _("Subtitle speed:"));
	} else {
		_subToggleDesc = new StaticTextWidget(boss, prefix + "subToggleDesc", _c("Text and speech:", "lowres"));

		_subToggleGroup = new RadiobuttonGroup(boss, kSubtitleToggle);

		_subToggleSpeechOnly = new RadiobuttonWidget(boss, prefix + "subToggleSpeechOnly", _subToggleGroup, kSubtitlesSpeech, _("Spch"), _("Speech"));
		_subToggleSubOnly = new RadiobuttonWidget(boss, prefix + "subToggleSubOnly", _subToggleGroup, kSubtitlesSubs, _("Subs"), _("Subtitles"));
		_subToggleSubBoth = new RadiobuttonWidget(boss, prefix + "subToggleSubBoth", _subToggleGroup, kSubtitlesBoth, _c("Both", "lowres"), _("Show subtitles and play speech"));

		_subSpeedDesc = new StaticTextWidget(boss, prefix + "subSubtitleSpeedDesc", _c("Subtitle speed:", "lowres"));
	}

	// Subtitle speed
	_subSpeedSlider = new SliderWidget(boss, prefix + "subSubtitleSpeedSlider", 0, kSubtitleSpeedChanged);
	_subSpeedLabel = new StaticTextWidget(boss, prefix + "subSubtitleSpeedLabel", "100%");
	_subSpeedSlider->setMinValue(0); _subSpeedSlider->setMaxValue(maxSliderVal);
	_subSpeedLabel->setFlags(WIDGET_CLEARBG);

	_enableSubtitleSettings = true;
}

void OptionsDialog::addVolumeControls(GuiObject *boss, const Common::String &prefix) {

	// Volume controllers
	if (g_system->getOverlayWidth() > 320)
		_musicVolumeDesc = new StaticTextWidget(boss, prefix + "vcMusicText", _("Music volume:"));
	else
		_musicVolumeDesc = new StaticTextWidget(boss, prefix + "vcMusicText", _c("Music volume:", "lowres"));
	_musicVolumeSlider = new SliderWidget(boss, prefix + "vcMusicSlider", 0, kMusicVolumeChanged);
	_musicVolumeLabel = new StaticTextWidget(boss, prefix + "vcMusicLabel", "100%");
	_musicVolumeSlider->setMinValue(0);
	_musicVolumeSlider->setMaxValue(Audio::Mixer::kMaxMixerVolume);
	_musicVolumeLabel->setFlags(WIDGET_CLEARBG);

	_muteCheckbox = new CheckboxWidget(boss, prefix + "vcMuteCheckbox", _("Mute all"), 0, kMuteAllChanged);

	if (g_system->getOverlayWidth() > 320)
		_sfxVolumeDesc = new StaticTextWidget(boss, prefix + "vcSfxText", _("SFX volume:"), _("Special sound effects volume"));
	else
		_sfxVolumeDesc = new StaticTextWidget(boss, prefix + "vcSfxText", _c("SFX volume:", "lowres"), _("Special sound effects volume"));
	_sfxVolumeSlider = new SliderWidget(boss, prefix + "vcSfxSlider", _("Special sound effects volume"), kSfxVolumeChanged);
	_sfxVolumeLabel = new StaticTextWidget(boss, prefix + "vcSfxLabel", "100%");
	_sfxVolumeSlider->setMinValue(0);
	_sfxVolumeSlider->setMaxValue(Audio::Mixer::kMaxMixerVolume);
	_sfxVolumeLabel->setFlags(WIDGET_CLEARBG);

	if (g_system->getOverlayWidth() > 320)
		_speechVolumeDesc = new StaticTextWidget(boss, prefix + "vcSpeechText" , _("Speech volume:"));
	else
		_speechVolumeDesc = new StaticTextWidget(boss, prefix + "vcSpeechText" , _c("Speech volume:", "lowres"));
	_speechVolumeSlider = new SliderWidget(boss, prefix + "vcSpeechSlider", 0, kSpeechVolumeChanged);
	_speechVolumeLabel = new StaticTextWidget(boss, prefix + "vcSpeechLabel", "100%");
	_speechVolumeSlider->setMinValue(0);
	_speechVolumeSlider->setMaxValue(Audio::Mixer::kMaxMixerVolume);
	_speechVolumeLabel->setFlags(WIDGET_CLEARBG);

	_enableVolumeSettings = true;
}

void OptionsDialog::addEngineControls(GuiObject *boss, const Common::String &prefix, const ExtraGuiOptions &engineOptions) {
	// Note: up to 7 engine options can currently fit on screen (the most that
	// can fit in a 320x200 screen with the classic theme).
	// TODO: Increase this number by including the checkboxes inside a scroll
	// widget. The appropriate number of checkboxes will need to be added to
	// the theme files.

	uint i = 1;
	ExtraGuiOptions::const_iterator iter;
	for (iter = engineOptions.begin(); iter != engineOptions.end(); ++iter, ++i) {
		Common::String id = Common::String::format("%d", i);
		_engineCheckboxes.push_back(new CheckboxWidget(boss,
			prefix + "customOption" + id + "Checkbox", _(iter->label), _(iter->tooltip)));
	}
}

bool OptionsDialog::loadMusicDeviceSetting(PopUpWidget *popup, Common::String setting, MusicType preferredType) {
	if (!popup || !popup->isEnabled())
		return true;

	if (_domain != Common::ConfigManager::kApplicationDomain || ConfMan.hasKey(setting, _domain) || preferredType) {
		const Common::String drv = ConfMan.get(setting, (_domain != Common::ConfigManager::kApplicationDomain && !ConfMan.hasKey(setting, _domain)) ? Common::ConfigManager::kApplicationDomain : _domain);
		const PluginList p = MusicMan.getPlugins();

		for (PluginList::const_iterator m = p.begin(); m != p.end(); ++m) {
			MusicDevices i = (*m)->get<MusicPluginObject>().getDevices();
			for (MusicDevices::iterator d = i.begin(); d != i.end(); ++d) {
				if (setting.empty() ? (preferredType == d->getMusicType()) : (drv == d->getCompleteId())) {
					popup->setSelectedTag(d->getHandle());
					return popup->getSelected() != -1;
				}
			}
		}
	}

	return false;
}

void OptionsDialog::saveMusicDeviceSetting(PopUpWidget *popup, Common::String setting) {
	if (!popup || !_enableAudioSettings)
		return;

	const PluginList p = MusicMan.getPlugins();
	bool found = false;
	for (PluginList::const_iterator m = p.begin(); m != p.end() && !found; ++m) {
		MusicDevices i = (*m)->get<MusicPluginObject>().getDevices();
		for (MusicDevices::iterator d = i.begin(); d != i.end(); ++d) {
			if (d->getHandle() == popup->getSelectedTag()) {
				ConfMan.set(setting, d->getCompleteId(), _domain);
				found = true;
				break;
			}
		}
	}

	if (!found)
		ConfMan.removeKey(setting, _domain);
}

int OptionsDialog::getSubtitleMode(bool subtitles, bool speech_mute) {
	if (_guioptions.contains(GUIO_NOSUBTITLES))
		return kSubtitlesSpeech; // Speech only
	if (_guioptions.contains(GUIO_NOSPEECH))
		return kSubtitlesSubs; // Subtitles only

	if (!subtitles && !speech_mute) // Speech only
		return kSubtitlesSpeech;
	else if (subtitles && !speech_mute) // Speech and subtitles
		return kSubtitlesBoth;
	else if (subtitles && speech_mute) // Subtitles only
		return kSubtitlesSubs;
	else
		warning("Wrong configuration: Both subtitles and speech are off. Assuming subtitles only");
	return kSubtitlesSubs;
}

void OptionsDialog::updateMusicVolume(const int newValue) const {
	_musicVolumeLabel->setValue(newValue);
	_musicVolumeSlider->setValue(newValue);
	_musicVolumeLabel->markAsDirty();
	_musicVolumeSlider->markAsDirty();
}

void OptionsDialog::updateSfxVolume(const int newValue) const {
	_sfxVolumeLabel->setValue(newValue);
	_sfxVolumeSlider->setValue(newValue);
	_sfxVolumeLabel->markAsDirty();
	_sfxVolumeSlider->markAsDirty();
}

void OptionsDialog::updateSpeechVolume(const int newValue) const {
	_speechVolumeLabel->setValue(newValue);
	_speechVolumeSlider->setValue(newValue);
	_speechVolumeLabel->markAsDirty();
	_speechVolumeSlider->markAsDirty();
}

void OptionsDialog::reflowLayout() {
	if (_graphicsTabId != -1 && _tabWidget)
		_tabWidget->setTabTitle(_graphicsTabId, g_system->getOverlayWidth() > 320 ? _("Graphics") : _("GFX"));

	Dialog::reflowLayout();
	setupGraphicsTab();
}

void OptionsDialog::setupGraphicsTab() {
	if (!_fullscreenCheckbox)
		return;
	_gfxPopUpDesc->setVisible(true);
	_gfxPopUp->setVisible(true);
	if (g_system->hasFeature(OSystem::kFeatureStretchMode)) {
		_stretchPopUpDesc->setVisible(true);
		_stretchPopUp->setVisible(true);
	} else {
		_stretchPopUpDesc->setVisible(false);
		_stretchPopUp->setVisible(false);
	}
	_fullscreenCheckbox->setVisible(true);
	if (g_system->hasFeature(OSystem::kFeatureFilteringMode))
		_filteringCheckbox->setVisible(true);
	else
		_filteringCheckbox->setVisible(false);
	_aspectCheckbox->setVisible(true);
	_renderModePopUpDesc->setVisible(true);
	_renderModePopUp->setVisible(true);
}

#pragma mark -


GlobalOptionsDialog::GlobalOptionsDialog(LauncherDialog *launcher)
	: OptionsDialog(Common::ConfigManager::kApplicationDomain, "GlobalOptions"), _launcher(launcher) {
#ifdef GUI_ENABLE_KEYSDIALOG
	_keysDialog = 0;
#endif
#ifdef USE_FLUIDSYNTH
	_fluidSynthSettingsDialog = 0;
#endif
	_savePath = 0;
	_savePathClearButton = 0;
	_themePath = 0;
	_themePathClearButton = 0;
	_extraPath = 0;
	_extraPathClearButton = 0;
#ifdef DYNAMIC_MODULES
	_pluginsPath = 0;
#endif
	_curTheme = 0;
	_rendererPopUpDesc = 0;
	_rendererPopUp = 0;
	_autosavePeriodPopUpDesc = 0;
	_autosavePeriodPopUp = 0;
	_guiLanguagePopUpDesc = 0;
	_guiLanguagePopUp = 0;
	_guiLanguageUseGameLanguageCheckbox = nullptr;
	_useSystemDialogsCheckbox = 0;
#ifdef USE_UPDATES
	_updatesPopUpDesc = 0;
	_updatesPopUp = 0;
#endif
#ifdef USE_CLOUD
#ifdef USE_LIBCURL
	_selectedStorageIndex = CloudMan.getStorageIndex();
#else
	_selectedStorageIndex = 0;
#endif
	_storagePopUpDesc = 0;
	_storagePopUp = 0;
	_storageUsernameDesc = 0;
	_storageUsername = 0;
	_storageUsedSpaceDesc = 0;
	_storageUsedSpace = 0;
	_storageLastSyncDesc = 0;
	_storageLastSync = 0;
	_storageConnectButton = 0;
	_storageRefreshButton = 0;
	_storageDownloadButton = 0;
	_runServerButton = 0;
	_serverInfoLabel = 0;
	_rootPathButton = 0;
	_rootPath = 0;
	_rootPathClearButton = 0;
	_serverPortDesc = 0;
	_serverPort = 0;
	_serverPortClearButton = 0;
	_redrawCloudTab = false;
#ifdef USE_SDL_NET
	_serverWasRunning = false;
#endif
#endif
}

GlobalOptionsDialog::~GlobalOptionsDialog() {
#ifdef GUI_ENABLE_KEYSDIALOG
	delete _keysDialog;
#endif

#ifdef USE_FLUIDSYNTH
	delete _fluidSynthSettingsDialog;
#endif
}

void GlobalOptionsDialog::build() {
	// The tab widget
	TabWidget *tab = new TabWidget(this, "GlobalOptions.TabWidget");

	//
	// 1) The graphics tab
	//
	_graphicsTabId = tab->addTab(g_system->getOverlayWidth() > 320 ? _("Graphics") : _("GFX"));
	ScrollContainerWidget *graphicsContainer = new ScrollContainerWidget(tab, "GlobalOptions_Graphics.Container", kGraphicsTabContainerReflowCmd);
	graphicsContainer->setTarget(this);
	graphicsContainer->setBackgroundType(ThemeEngine::kDialogBackgroundNone);
	addGraphicControls(graphicsContainer, "GlobalOptions_Graphics_Container.");

	//
	// The shader tab (currently visible only for Vita platform), visibility checking by features
	//

	if (g_system->hasFeature(OSystem::kFeatureShader)) {
		tab->addTab(_("Shader"));
		addShaderControls(tab, "GlobalOptions_Shader.");
	}

	//
	// The control tab (currently visible only for AndroidSDL, SDL, and Vita platform, visibility checking by features
	//
	if (g_system->hasFeature(OSystem::kFeatureTouchpadMode) ||
		g_system->hasFeature(OSystem::kFeatureOnScreenControl) ||
		g_system->hasFeature(OSystem::kFeatureSwapMenuAndBackButtons) ||
		g_system->hasFeature(OSystem::kFeatureKbdMouseSpeed) ||
		g_system->hasFeature(OSystem::kFeatureJoystickDeadzone)) {
		tab->addTab(_("Control"));
		addControlControls(tab, "GlobalOptions_Control.");
	}

	//
	// 2) The audio tab
	//
	tab->addTab(_("Audio"));
	addAudioControls(tab, "GlobalOptions_Audio.");
	addSubtitleControls(tab, "GlobalOptions_Audio.");

	if (g_system->getOverlayWidth() > 320)
		tab->addTab(_("Volume"));
	else
		tab->addTab(_c("Volume", "lowres"));
	addVolumeControls(tab, "GlobalOptions_Volume.");

	// TODO: cd drive setting

	//
	// 3) The MIDI tab
	//
	_midiTabId = tab->addTab(_("MIDI"));
	addMIDIControls(tab, "GlobalOptions_MIDI.");

#ifdef USE_FLUIDSYNTH
	new ButtonWidget(tab, "GlobalOptions_MIDI.mcFluidSynthSettings", _("FluidSynth Settings"), 0, kFluidSynthSettingsCmd);
#endif

	//
	// 4) The MT-32 tab
	//
	tab->addTab(_("MT-32"));
	addMT32Controls(tab, "GlobalOptions_MT32.");

	//
	// 5) The Paths tab
	//
	if (g_system->getOverlayWidth() > 320)
		_pathsTabId = tab->addTab(_("Paths"));
	else
		_pathsTabId = tab->addTab(_c("Paths", "lowres"));

#if !defined(__DC__)
	// These two buttons have to be extra wide, or the text will be
	// truncated in the small version of the GUI.

	// Save game path
	if (g_system->getOverlayWidth() > 320)
		new ButtonWidget(tab, "GlobalOptions_Paths.SaveButton", _("Save Path:"), _("Specifies where your saved games are put"), kChooseSaveDirCmd);
	else
		new ButtonWidget(tab, "GlobalOptions_Paths.SaveButton", _c("Save Path:", "lowres"), _("Specifies where your saved games are put"), kChooseSaveDirCmd);
	_savePath = new StaticTextWidget(tab, "GlobalOptions_Paths.SavePath", "/foo/bar", _("Specifies where your saved games are put"));

	_savePathClearButton = addClearButton(tab, "GlobalOptions_Paths.SavePathClearButton", kSavePathClearCmd);

	if (g_system->getOverlayWidth() > 320)
		new ButtonWidget(tab, "GlobalOptions_Paths.ThemeButton", _("Theme Path:"), 0, kChooseThemeDirCmd);
	else
		new ButtonWidget(tab, "GlobalOptions_Paths.ThemeButton", _c("Theme Path:", "lowres"), 0, kChooseThemeDirCmd);
	_themePath = new StaticTextWidget(tab, "GlobalOptions_Paths.ThemePath", _c("None", "path"));

	_themePathClearButton = addClearButton(tab, "GlobalOptions_Paths.ThemePathClearButton", kThemePathClearCmd);

	if (g_system->getOverlayWidth() > 320)
		new ButtonWidget(tab, "GlobalOptions_Paths.ExtraButton", _("Extra Path:"), _("Specifies path to additional data used by all games or ScummVM"), kChooseExtraDirCmd);
	else
		new ButtonWidget(tab, "GlobalOptions_Paths.ExtraButton", _c("Extra Path:", "lowres"), _("Specifies path to additional data used by all games or ScummVM"), kChooseExtraDirCmd);
	_extraPath = new StaticTextWidget(tab, "GlobalOptions_Paths.ExtraPath", _c("None", "path"), _("Specifies path to additional data used by all games or ScummVM"));

	_extraPathClearButton = addClearButton(tab, "GlobalOptions_Paths.ExtraPathClearButton", kExtraPathClearCmd);

#ifdef DYNAMIC_MODULES
	if (g_system->getOverlayWidth() > 320)
		new ButtonWidget(tab, "GlobalOptions_Paths.PluginsButton", _("Plugins Path:"), 0, kChoosePluginsDirCmd);
	else
		new ButtonWidget(tab, "GlobalOptions_Paths.PluginsButton", _c("Plugins Path:", "lowres"), 0, kChoosePluginsDirCmd);
	_pluginsPath = new StaticTextWidget(tab, "GlobalOptions_Paths.PluginsPath", _c("None", "path"));
#endif
#endif

	//
	// 6) The miscellaneous tab
	//
	if (g_system->getOverlayWidth() > 320)
		tab->addTab(_("Misc"));
	else
		tab->addTab(_c("Misc", "lowres"));

	new ButtonWidget(tab, "GlobalOptions_Misc.ThemeButton", _("Theme:"), 0, kChooseThemeCmd);
	_curTheme = new StaticTextWidget(tab, "GlobalOptions_Misc.CurTheme", g_gui.theme()->getThemeName());


	_rendererPopUpDesc = new StaticTextWidget(tab, "GlobalOptions_Misc.RendererPopupDesc", _("GUI renderer:"));
	_rendererPopUp = new PopUpWidget(tab, "GlobalOptions_Misc.RendererPopup");

	if (g_system->getOverlayWidth() > 320) {
		for (uint i = 1; i < GUI::ThemeEngine::_rendererModesSize; ++i)
			_rendererPopUp->appendEntry(_(GUI::ThemeEngine::_rendererModes[i].name), GUI::ThemeEngine::_rendererModes[i].mode);
	} else {
		for (uint i = 1; i < GUI::ThemeEngine::_rendererModesSize; ++i)
			_rendererPopUp->appendEntry(_(GUI::ThemeEngine::_rendererModes[i].shortname), GUI::ThemeEngine::_rendererModes[i].mode);
	}

	if (g_system->getOverlayWidth() > 320)
		_autosavePeriodPopUpDesc = new StaticTextWidget(tab, "GlobalOptions_Misc.AutosavePeriodPopupDesc", _("Autosave:"));
	else
		_autosavePeriodPopUpDesc = new StaticTextWidget(tab, "GlobalOptions_Misc.AutosavePeriodPopupDesc", _c("Autosave:", "lowres"));
	_autosavePeriodPopUp = new PopUpWidget(tab, "GlobalOptions_Misc.AutosavePeriodPopup");

	for (int i = 0; savePeriodLabels[i]; i++) {
		_autosavePeriodPopUp->appendEntry(_(savePeriodLabels[i]), savePeriodValues[i]);
	}

#ifdef GUI_ENABLE_KEYSDIALOG
	new ButtonWidget(tab, "GlobalOptions_Misc.KeysButton", _("Keys"), 0, kChooseKeyMappingCmd);
#endif

	// TODO: joystick setting


#ifdef USE_TRANSLATION
	_guiLanguagePopUpDesc = new StaticTextWidget(tab, "GlobalOptions_Misc.GuiLanguagePopupDesc", _("GUI language:"), _("Language of ScummVM GUI"));
	_guiLanguagePopUp = new PopUpWidget(tab, "GlobalOptions_Misc.GuiLanguagePopup");
#ifdef USE_DETECTLANG
	_guiLanguagePopUp->appendEntry(_("<default>"), Common::kTranslationAutodetectId);
#endif // USE_DETECTLANG
	_guiLanguagePopUp->appendEntry("English", Common::kTranslationBuiltinId);
	_guiLanguagePopUp->appendEntry("", 0);
	Common::TLangArray languages = TransMan.getSupportedLanguageNames();
	Common::TLangArray::iterator lang = languages.begin();
	while (lang != languages.end()) {
		_guiLanguagePopUp->appendEntry(lang->name, lang->id);
		lang++;
	}

	// Select the currently configured language or default/English if
	// nothing is specified.
	if (ConfMan.hasKey("gui_language") && !ConfMan.get("gui_language").empty())
		_guiLanguagePopUp->setSelectedTag(TransMan.parseLanguage(ConfMan.get("gui_language")));
	else
#ifdef USE_DETECTLANG
		_guiLanguagePopUp->setSelectedTag(Common::kTranslationAutodetectId);
#else // !USE_DETECTLANG
		_guiLanguagePopUp->setSelectedTag(Common::kTranslationBuiltinId);
#endif // USE_DETECTLANG

	_guiLanguageUseGameLanguageCheckbox = new CheckboxWidget(tab, "GlobalOptions_Misc.GuiLanguageUseGameLanguage",
			_("Switch the GUI language to the game language"),
			_("When starting a game, change the GUI language to the game language."
			"That way, if a game uses the ScummVM save and load dialogs, they are "
			"in the same language as the game.")
	);

	if (ConfMan.hasKey("gui_use_game_language")) {
		_guiLanguageUseGameLanguageCheckbox->setState(ConfMan.getBool("gui_use_game_language", _domain));
	}

#endif // USE_TRANSLATION

	if (g_system->hasFeature(OSystem::kFeatureSystemBrowserDialog)) {
		_useSystemDialogsCheckbox = new CheckboxWidget(tab, "GlobalOptions_Misc.UseSystemDialogs",
			_("Use native system file browser"),
			_("Use the native system file browser instead of the ScummVM one to select a file or directory.")
		);

		_useSystemDialogsCheckbox->setState(ConfMan.getBool("gui_browser_native", _domain));
	}

#ifdef USE_UPDATES
	_updatesPopUpDesc = new StaticTextWidget(tab, "GlobalOptions_Misc.UpdatesPopupDesc", _("Update check:"), _("How often to check ScummVM updates"));
	_updatesPopUp = new PopUpWidget(tab, "GlobalOptions_Misc.UpdatesPopup");

	const int *vals = Common::UpdateManager::getUpdateIntervals();

	while (*vals != -1) {
		_updatesPopUp->appendEntry(Common::UpdateManager::updateIntervalToString(*vals), *vals);
		vals++;
	}

	_updatesPopUp->setSelectedTag(Common::UpdateManager::normalizeInterval(ConfMan.getInt("updates_check")));

	new ButtonWidget(tab, "GlobalOptions_Misc.UpdatesCheckManuallyButton", _("Check now"), 0, kUpdatesCheckCmd);
#endif

#ifdef USE_CLOUD
	//
	// 7) The cloud tab
	//
	if (g_system->getOverlayWidth() > 320)
		tab->addTab(_("Cloud"));
	else
		tab->addTab(_c("Cloud", "lowres"));

	ScrollContainerWidget *container = new ScrollContainerWidget(tab, "GlobalOptions_Cloud.Container", kCloudTabContainerReflowCmd);
	container->setTarget(this);
	container->setBackgroundType(ThemeEngine::kDialogBackgroundNone);

	_storagePopUpDesc = new StaticTextWidget(container, "GlobalOptions_Cloud_Container.StoragePopupDesc", _("Storage:"), _("Active cloud storage"));
	_storagePopUp = new PopUpWidget(container, "GlobalOptions_Cloud_Container.StoragePopup");
#ifdef USE_LIBCURL
	Common::StringArray list = CloudMan.listStorages();
	for (uint32 i = 0; i < list.size(); ++i)
		_storagePopUp->appendEntry(list[i], i);
#else
	_storagePopUp->appendEntry(_("<none>"), 0);
#endif
	_storagePopUp->setSelected(_selectedStorageIndex);

	_storageUsernameDesc = new StaticTextWidget(container, "GlobalOptions_Cloud_Container.StorageUsernameDesc", _("Username:"), _("Username used by this storage"));
	_storageUsername = new StaticTextWidget(container, "GlobalOptions_Cloud_Container.StorageUsernameLabel", "<none>");

	_storageUsedSpaceDesc = new StaticTextWidget(container, "GlobalOptions_Cloud_Container.StorageUsedSpaceDesc", _("Used space:"), _("Space used by ScummVM's saved games on this storage"));
	_storageUsedSpace = new StaticTextWidget(container, "GlobalOptions_Cloud_Container.StorageUsedSpaceLabel", "0 bytes");

	_storageLastSyncDesc = new StaticTextWidget(container, "GlobalOptions_Cloud_Container.StorageLastSyncDesc", _("Last sync time:"), _("When the last saved games sync for this storage occured"));
	_storageLastSync = new StaticTextWidget(container, "GlobalOptions_Cloud_Container.StorageLastSyncLabel", "<never>");

	_storageConnectButton = new ButtonWidget(container, "GlobalOptions_Cloud_Container.ConnectButton", _("Connect"), _("Open wizard dialog to connect your cloud storage account"), kConfigureStorageCmd);
	_storageRefreshButton = new ButtonWidget(container, "GlobalOptions_Cloud_Container.RefreshButton", _("Refresh"), _("Refresh current cloud storage information (username and usage)"), kRefreshStorageCmd);
	_storageDownloadButton = new ButtonWidget(container, "GlobalOptions_Cloud_Container.DownloadButton", _("Download"), _("Open downloads manager dialog"), kDownloadStorageCmd);

	_runServerButton = new ButtonWidget(container, "GlobalOptions_Cloud_Container.RunServerButton", _("Run server"), _("Run local webserver"), kRunServerCmd);
	_serverInfoLabel = new StaticTextWidget(container, "GlobalOptions_Cloud_Container.ServerInfoLabel", _("Not running"));

	// Root path
	if (g_system->getOverlayWidth() > 320)
		_rootPathButton = new ButtonWidget(container, "GlobalOptions_Cloud_Container.RootPathButton", _("/root/ Path:"), _("Specifies which directory the Files Manager can access"), kChooseRootDirCmd);
	else
		_rootPathButton = new ButtonWidget(container, "GlobalOptions_Cloud_Container.RootPathButton", _c("/root/ Path:", "lowres"), _("Specifies which directory the Files Manager can access"), kChooseRootDirCmd);
	_rootPath = new StaticTextWidget(container, "GlobalOptions_Cloud_Container.RootPath", "/foo/bar", _("Specifies which directory the Files Manager can access"));

	_rootPathClearButton = addClearButton(container, "GlobalOptions_Cloud_Container.RootPathClearButton", kRootPathClearCmd);

#ifdef USE_SDL_NET
	uint32 port = Networking::LocalWebserver::getPort();
#else
	uint32 port = 0; // the following widgets are hidden anyway
#endif
	_serverPortDesc = new StaticTextWidget(container, "GlobalOptions_Cloud_Container.ServerPortDesc", _("Server's port:"), _("Which port is used by the server\nAuth with server is not available with non-default port"));
	_serverPort = new EditTextWidget(container, "GlobalOptions_Cloud_Container.ServerPortEditText", Common::String::format("%u", port), 0);
	_serverPortClearButton = addClearButton(container, "GlobalOptions_Cloud_Container.ServerPortClearButton", kServerPortClearCmd);

	setupCloudTab();
#endif // USE_CLOUD

	// Activate the first tab
	tab->setActiveTab(0);
	_tabWidget = tab;

	// Add OK & Cancel buttons
	new ButtonWidget(this, "GlobalOptions.Cancel", _("Cancel"), 0, kCloseCmd);
	new ButtonWidget(this, "GlobalOptions.Apply", _("Apply"), 0, kApplyCmd);
	new ButtonWidget(this, "GlobalOptions.Ok", _("OK"), 0, kOKCmd);

#ifdef GUI_ENABLE_KEYSDIALOG
	_keysDialog = new KeysDialog();
#endif

#ifdef USE_FLUIDSYNTH
	_fluidSynthSettingsDialog = new FluidSynthSettingsDialog();
#endif

	OptionsDialog::build();

#if !defined(__DC__)
	// Set _savePath to the current save path
	Common::String savePath(ConfMan.get("savepath", _domain));
	Common::String themePath(ConfMan.get("themepath", _domain));
	Common::String extraPath(ConfMan.get("extrapath", _domain));

	if (savePath.empty() || !ConfMan.hasKey("savepath", _domain)) {
		_savePath->setLabel(_("Default"));
	} else {
		_savePath->setLabel(savePath);
	}

	if (themePath.empty() || !ConfMan.hasKey("themepath", _domain)) {
		_themePath->setLabel(_c("None", "path"));
	} else {
		_themePath->setLabel(themePath);
	}

	if (extraPath.empty() || !ConfMan.hasKey("extrapath", _domain)) {
		_extraPath->setLabel(_c("None", "path"));
	} else {
		_extraPath->setLabel(extraPath);
	}

#ifdef DYNAMIC_MODULES
	Common::String pluginsPath(ConfMan.get("pluginspath", _domain));
	if (pluginsPath.empty() || !ConfMan.hasKey("pluginspath", _domain)) {
		_pluginsPath->setLabel(_c("None", "path"));
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

#ifdef USE_CLOUD
	Common::String rootPath(ConfMan.get("rootpath", "cloud"));
	if (rootPath.empty() || !ConfMan.hasKey("rootpath", "cloud")) {
		_rootPath->setLabel(_c("None", "path"));
	} else {
		_rootPath->setLabel(rootPath);
	}
#endif
}

void GlobalOptionsDialog::clean() {
#ifdef GUI_ENABLE_KEYSDIALOG
	delete _keysDialog;
	_keysDialog = 0;
#endif

#ifdef USE_FLUIDSYNTH
	delete _fluidSynthSettingsDialog;
	_fluidSynthSettingsDialog = 0;
#endif

	OptionsDialog::clean();
}

void GlobalOptionsDialog::apply() {
	OptionsDialog::apply();

	bool isRebuildNeeded = false;

	Common::String savePath(_savePath->getLabel());
	if (!savePath.empty() && (savePath != _("Default")))
		ConfMan.set("savepath", savePath, _domain);
	else
		ConfMan.removeKey("savepath", _domain);

	Common::String themePath(_themePath->getLabel());
	if (!themePath.empty() && (themePath != _c("None", "path")))
		ConfMan.set("themepath", themePath, _domain);
	else
		ConfMan.removeKey("themepath", _domain);

	Common::String extraPath(_extraPath->getLabel());
	if (!extraPath.empty() && (extraPath != _c("None", "path")))
		ConfMan.set("extrapath", extraPath, _domain);
	else
		ConfMan.removeKey("extrapath", _domain);

#ifdef DYNAMIC_MODULES
	Common::String pluginsPath(_pluginsPath->getLabel());
	if (!pluginsPath.empty() && (pluginsPath != _c("None", "path")))
		ConfMan.set("pluginspath", pluginsPath, _domain);
	else
		ConfMan.removeKey("pluginspath", _domain);
#endif

#ifdef USE_CLOUD
	Common::String rootPath(_rootPath->getLabel());
	if (!rootPath.empty() && (rootPath != _c("None", "path")))
		ConfMan.set("rootpath", rootPath, "cloud");
	else
		ConfMan.removeKey("rootpath", "cloud");
#endif

	ConfMan.setInt("autosave_period", _autosavePeriodPopUp->getSelectedTag(), _domain);

#ifdef USE_UPDATES
	ConfMan.setInt("updates_check", _updatesPopUp->getSelectedTag());

	if (g_system->getUpdateManager()) {
		if (_updatesPopUp->getSelectedTag() == Common::UpdateManager::kUpdateIntervalNotSupported) {
			g_system->getUpdateManager()->setAutomaticallyChecksForUpdates(Common::UpdateManager::kUpdateStateDisabled);
		} else {
			g_system->getUpdateManager()->setAutomaticallyChecksForUpdates(Common::UpdateManager::kUpdateStateEnabled);
			g_system->getUpdateManager()->setUpdateCheckInterval(_updatesPopUp->getSelectedTag());
		}
	}
#endif

#ifdef USE_CLOUD
#ifdef USE_LIBCURL
	if (CloudMan.getStorageIndex() != _selectedStorageIndex) {
		if (!CloudMan.switchStorage(_selectedStorageIndex)) {
			bool anotherStorageIsWorking = CloudMan.isWorking();
			Common::String message = _("Failed to change cloud storage!");
			if (anotherStorageIsWorking) {
				message += "\n";
				message += _("Another cloud storage is already active.");
			}
			MessageDialog dialog(message);
			dialog.runModal();
		}
	}
#endif // USE_LIBCURL

#ifdef USE_SDL_NET
#ifdef NETWORKING_LOCALWEBSERVER_ENABLE_PORT_OVERRIDE
	// save server's port
	uint32 port = Networking::LocalWebserver::getPort();
	if (_serverPort) {
		uint64 contents = _serverPort->getEditString().asUint64();
		if (contents != 0)
			port = contents;
	}
	ConfMan.setInt("local_server_port", port);
#endif // NETWORKING_LOCALWEBSERVER_ENABLE_PORT_OVERRIDE
#endif // USE_SDL_NET
#endif // USE_CLOUD

	Common::String oldThemeId = g_gui.theme()->getThemeId();
	Common::String oldThemeName = g_gui.theme()->getThemeName();
	if (!_newTheme.empty()) {
		ConfMan.set("gui_theme", _newTheme);
	}

#ifdef USE_TRANSLATION
	int selectedLang = _guiLanguagePopUp->getSelectedTag();
	Common::String oldLang = ConfMan.get("gui_language");
	Common::String newLang = TransMan.getLangById(selectedLang);
	Common::String newCharset;
	if (newLang != oldLang) {
		TransMan.setLanguage(newLang);
		ConfMan.set("gui_language", newLang);
		newCharset = TransMan.getCurrentCharset();
		isRebuildNeeded = true;
	}

	bool guiUseGameLanguage = _guiLanguageUseGameLanguageCheckbox->getState();
	ConfMan.setBool("gui_use_game_language", guiUseGameLanguage, _domain);
#endif

	if (_useSystemDialogsCheckbox) {
		ConfMan.setBool("gui_browser_native", _useSystemDialogsCheckbox->getState(), _domain);
	}

	GUI::ThemeEngine::GraphicsMode gfxMode = (GUI::ThemeEngine::GraphicsMode)_rendererPopUp->getSelectedTag();
	Common::String oldGfxConfig = ConfMan.get("gui_renderer");
	Common::String newGfxConfig = GUI::ThemeEngine::findModeConfigName(gfxMode);
	if (newGfxConfig != oldGfxConfig) {
		ConfMan.set("gui_renderer", newGfxConfig, _domain);
	}

	if (_newTheme.empty())
		_newTheme = oldThemeId;

	if (!g_gui.loadNewTheme(_newTheme, gfxMode, true)) {
		Common::String errorMessage;

		_curTheme->setLabel(oldThemeName);
		_newTheme = oldThemeId;
		ConfMan.set("gui_theme", _newTheme);
		gfxMode = GUI::ThemeEngine::findMode(oldGfxConfig);
		_rendererPopUp->setSelectedTag(gfxMode);
		newGfxConfig = oldGfxConfig;
		ConfMan.set("gui_renderer", newGfxConfig, _domain);
#ifdef USE_TRANSLATION
		bool isCharsetEqual = (newCharset == TransMan.getCurrentCharset());
		TransMan.setLanguage(oldLang);
		_guiLanguagePopUp->setSelectedTag(selectedLang);
		ConfMan.set("gui_language", oldLang);

		if (!isCharsetEqual)
			errorMessage = _("Theme does not support selected language!");
		else
#endif
			errorMessage = _("Theme cannot be loaded!");

		g_gui.loadNewTheme(_newTheme, gfxMode, true);
		errorMessage += _("\nMisc settings will be restored.");
		MessageDialog error(errorMessage);
		error.runModal();
	}

	if (isRebuildNeeded) {
		rebuild();
		if (_launcher != 0)
			_launcher->rebuild();
	}

	_newTheme.clear();

	// Save config file
	ConfMan.flushToDisk();
}

void GlobalOptionsDialog::close() {
#if defined(USE_CLOUD) && defined(USE_SDL_NET)
	if (LocalServer.isRunning()) {
		LocalServer.stop();
	}
#endif
	OptionsDialog::close();
}

void GlobalOptionsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kChooseSaveDirCmd: {
		BrowserDialog browser(_("Select directory for saved games"), true);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(browser.getResult());
			if (dir.isWritable()) {
				_savePath->setLabel(dir.getPath());
			} else {
				MessageDialog error(_("The chosen directory cannot be written to. Please select another one."));
				error.runModal();
				return;
			}
			g_gui.scheduleTopDialogRedraw();
		}
		break;
	}
	case kChooseThemeDirCmd: {
		BrowserDialog browser(_("Select directory for GUI themes"), true);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(browser.getResult());
			_themePath->setLabel(dir.getPath());
			g_gui.scheduleTopDialogRedraw();
		}
		break;
	}
	case kChooseExtraDirCmd: {
		BrowserDialog browser(_("Select directory for extra files"), true);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(browser.getResult());
			_extraPath->setLabel(dir.getPath());
			g_gui.scheduleTopDialogRedraw();
		}
		break;
	}
#ifdef DYNAMIC_MODULES
	case kChoosePluginsDirCmd: {
		BrowserDialog browser(_("Select directory for plugins"), true);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(browser.getResult());
			_pluginsPath->setLabel(dir.getPath());
			g_gui.scheduleTopDialogRedraw();
		}
		break;
	}
#endif
#ifdef USE_CLOUD
	case kChooseRootDirCmd: {
		BrowserDialog browser(_("Select directory for Files Manager /root/"), true);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(browser.getResult());
			Common::String path = dir.getPath();
			if (path.empty())
				path = "/"; // absolute root
			_rootPath->setLabel(path);
			g_gui.scheduleTopDialogRedraw();
		}
		break;
	}
#endif
	case kThemePathClearCmd:
		_themePath->setLabel(_c("None", "path"));
		break;
	case kExtraPathClearCmd:
		_extraPath->setLabel(_c("None", "path"));
		break;
	case kSavePathClearCmd:
		_savePath->setLabel(_("Default"));
		break;
#ifdef USE_CLOUD
	case kRootPathClearCmd:
		_rootPath->setLabel(_c("None", "path"));
		break;
#endif
	case kChooseSoundFontCmd: {
		BrowserDialog browser(_("Select SoundFont"), false);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode file(browser.getResult());
			_soundFont->setLabel(file.getPath());

			if (!file.getPath().empty() && (file.getPath() != _c("None", "path")))
				_soundFontClearButton->setEnabled(true);
			else
				_soundFontClearButton->setEnabled(false);

			g_gui.scheduleTopDialogRedraw();
		}
		break;
	}
	case kChooseThemeCmd:
	{
		ThemeBrowser browser;
		if (browser.runModal() > 0) {
			// User made his choice...
			_newTheme = browser.getSelected();
			_curTheme->setLabel(browser.getSelectedName());
		}
		break;
	}
#ifdef USE_CLOUD
	case kCloudTabContainerReflowCmd:
		setupCloudTab();
		break;
#ifdef USE_LIBCURL
	case kPopUpItemSelectedCmd:
	{
		// update container's scrollbar
		reflowLayout();
		break;
	}
	case kConfigureStorageCmd:
	{
#ifdef NETWORKING_LOCALWEBSERVER_ENABLE_PORT_OVERRIDE
		// save server's port
		uint32 port = Networking::LocalWebserver::getPort();
		if (_serverPort) {
			uint64 contents = _serverPort->getEditString().asUint64();
			if (contents != 0)
				port = contents;
		}
		ConfMan.setInt("local_server_port", port);
		ConfMan.flushToDisk();
#endif // NETWORKING_LOCALWEBSERVER_ENABLE_PORT_OVERRIDE
		StorageWizardDialog dialog(_selectedStorageIndex);
		dialog.runModal();
		//update container's scrollbar
		reflowLayout();
		break;
	}
	case kRefreshStorageCmd:
	{
		CloudMan.info(
			new Common::Callback<GlobalOptionsDialog, Cloud::Storage::StorageInfoResponse>(this, &GlobalOptionsDialog::storageInfoCallback),
			new Common::Callback<GlobalOptionsDialog, Networking::ErrorResponse>(this, &GlobalOptionsDialog::storageErrorCallback)
		);
		Common::String dir = CloudMan.savesDirectoryPath();
		if (dir.lastChar() == '/')
			dir.deleteLastChar();
		CloudMan.listDirectory(
			dir,
			new Common::Callback<GlobalOptionsDialog, Cloud::Storage::ListDirectoryResponse>(this, &GlobalOptionsDialog::storageListDirectoryCallback),
			new Common::Callback<GlobalOptionsDialog, Networking::ErrorResponse>(this, &GlobalOptionsDialog::storageErrorCallback)
		);
		break;
	}
	case kDownloadStorageCmd:
		{
			DownloadDialog dialog(_selectedStorageIndex, _launcher);
			dialog.runModal();
			break;
		}
#endif // USE_LIBCURL
#ifdef USE_SDL_NET
	case kRunServerCmd:
		{
#ifdef NETWORKING_LOCALWEBSERVER_ENABLE_PORT_OVERRIDE
			// save server's port
			uint32 port = Networking::LocalWebserver::getPort();
			if (_serverPort) {
				uint64 contents = _serverPort->getEditString().asUint64();
				if (contents != 0)
					port = contents;
			}
			ConfMan.setInt("local_server_port", port);
			ConfMan.flushToDisk();
#endif // NETWORKING_LOCALWEBSERVER_ENABLE_PORT_OVERRIDE

			if (LocalServer.isRunning())
				LocalServer.stopOnIdle();
			else
				LocalServer.start();

			break;
		}

	case kServerPortClearCmd: {
		if (_serverPort) {
			_serverPort->setEditString(Common::String::format("%u", Networking::LocalWebserver::DEFAULT_SERVER_PORT));
		}
		g_gui.scheduleTopDialogRedraw();
		break;
	}
#endif // USE_SDL_NET
#endif // USE_CLOUD
#ifdef GUI_ENABLE_KEYSDIALOG
	case kChooseKeyMappingCmd:
		_keysDialog->runModal();
		break;
#endif
#ifdef USE_FLUIDSYNTH
	case kFluidSynthSettingsCmd:
		_fluidSynthSettingsDialog->runModal();
		break;
#endif
#ifdef USE_UPDATES
	case kUpdatesCheckCmd:
		if (g_system->getUpdateManager())
			g_system->getUpdateManager()->checkForUpdates();
		break;
#endif
	default:
		OptionsDialog::handleCommand(sender, cmd, data);
	}
}

void GlobalOptionsDialog::handleTickle() {
	OptionsDialog::handleTickle();
#ifdef USE_CLOUD
#ifdef USE_SDL_NET
	if (LocalServer.isRunning() != _serverWasRunning) {
		_serverWasRunning = !_serverWasRunning;
		_redrawCloudTab = true;
	}
#endif
	if (_redrawCloudTab) {
		setupCloudTab();
		g_gui.scheduleTopDialogRedraw();
		_redrawCloudTab = false;
	}
#endif
}

void GlobalOptionsDialog::reflowLayout() {
	int firstVisible = _tabWidget->getFirstVisible();
	int activeTab = _tabWidget->getActiveTab();

	if (_midiTabId != -1) {
		_tabWidget->setActiveTab(_midiTabId);

		_tabWidget->removeWidget(_soundFontClearButton);
		_soundFontClearButton->setNext(0);
		delete _soundFontClearButton;
		_soundFontClearButton = addClearButton(_tabWidget, "GlobalOptions_MIDI.mcFontClearButton", kClearSoundFontCmd);
	}

	if (_pathsTabId != -1) {
		_tabWidget->setActiveTab(_pathsTabId);

		_tabWidget->removeWidget(_savePathClearButton);
		_savePathClearButton->setNext(0);
		delete _savePathClearButton;
		_savePathClearButton = addClearButton(_tabWidget, "GlobalOptions_Paths.SavePathClearButton", kSavePathClearCmd);

		_tabWidget->removeWidget(_themePathClearButton);
		_themePathClearButton->setNext(0);
		delete _themePathClearButton;
		_themePathClearButton = addClearButton(_tabWidget, "GlobalOptions_Paths.ThemePathClearButton", kThemePathClearCmd);

		_tabWidget->removeWidget(_extraPathClearButton);
		_extraPathClearButton->setNext(0);
		delete _extraPathClearButton;
		_extraPathClearButton = addClearButton(_tabWidget, "GlobalOptions_Paths.ExtraPathClearButton", kExtraPathClearCmd);
	}

	_tabWidget->setActiveTab(activeTab);
	_tabWidget->setFirstVisible(firstVisible);

	OptionsDialog::reflowLayout();
#ifdef USE_CLOUD
	setupCloudTab();
#endif
}

#ifdef USE_CLOUD
void GlobalOptionsDialog::setupCloudTab() {
	int serverLabelPosition = -1; //no override
#ifdef USE_LIBCURL
	_selectedStorageIndex = (_storagePopUp ? _storagePopUp->getSelectedTag() : (uint32) Cloud::kStorageNoneId);

	if (_storagePopUpDesc) _storagePopUpDesc->setVisible(true);
	if (_storagePopUp) _storagePopUp->setVisible(true);

	bool shown = (_selectedStorageIndex != Cloud::kStorageNoneId);
	if (_storageUsernameDesc) _storageUsernameDesc->setVisible(shown);
	if (_storageUsername) {
		Common::String username = CloudMan.getStorageUsername(_selectedStorageIndex);
		if (username == "")
			username = _("<none>");
		_storageUsername->setLabel(username);
		_storageUsername->setVisible(shown);
	}
	if (_storageUsedSpaceDesc) _storageUsedSpaceDesc->setVisible(shown);
	if (_storageUsedSpace) {
		uint64 usedSpace = CloudMan.getStorageUsedSpace(_selectedStorageIndex);
		_storageUsedSpace->setLabel(Common::String::format(_("%llu bytes"), usedSpace));
		_storageUsedSpace->setVisible(shown);
	}
	if (_storageLastSyncDesc) _storageLastSyncDesc->setVisible(shown);
	if (_storageLastSync) {
		Common::String sync = CloudMan.getStorageLastSync(_selectedStorageIndex);
		if (sync == "") {
			if (_selectedStorageIndex == CloudMan.getStorageIndex() && CloudMan.isSyncing())
				sync = _("<right now>");
			else
				sync = _("<never>");
		}
		_storageLastSync->setLabel(sync);
		_storageLastSync->setVisible(shown);
	}
	if (_storageConnectButton)
		_storageConnectButton->setVisible(shown);
	if (_storageRefreshButton)
		_storageRefreshButton->setVisible(shown && _selectedStorageIndex == CloudMan.getStorageIndex());
	if (_storageDownloadButton)
		_storageDownloadButton->setVisible(shown && _selectedStorageIndex == CloudMan.getStorageIndex());
	if (!shown)
		serverLabelPosition = (_storageUsernameDesc ? _storageUsernameDesc->getRelY() : 0);
#else // USE_LIBCURL
	_selectedStorageIndex = 0;

	if (_storagePopUpDesc)
		_storagePopUpDesc->setVisible(false);
	if (_storagePopUp)
		_storagePopUp->setVisible(false);
	if (_storageUsernameDesc)
		_storageUsernameDesc->setVisible(false);
	if (_storageUsernameDesc)
		_storageUsernameDesc->setVisible(false);
	if (_storageUsername)
		_storageUsername->setVisible(false);
	if (_storageUsedSpaceDesc)
		_storageUsedSpaceDesc->setVisible(false);
	if (_storageUsedSpace)
		_storageUsedSpace->setVisible(false);
	if (_storageLastSyncDesc)
		_storageLastSyncDesc->setVisible(false);
	if (_storageLastSync)
		_storageLastSync->setVisible(false);
	if (_storageConnectButton)
		_storageConnectButton->setVisible(false);
	if (_storageRefreshButton)
		_storageRefreshButton->setVisible(false);
	if (_storageDownloadButton)
		_storageDownloadButton->setVisible(false);

	serverLabelPosition = (_storagePopUpDesc ? _storagePopUpDesc->getRelY() : 0);
#endif // USE_LIBCURL
#ifdef USE_SDL_NET
	//determine original widget's positions
	int16 x, y;
	uint16 w, h;
	int serverButtonY, serverInfoY;
	int serverRootButtonY, serverRootY, serverRootClearButtonY;
	int serverPortDescY, serverPortY, serverPortClearButtonY;
	if (!g_gui.xmlEval()->getWidgetData("GlobalOptions_Cloud_Container.RunServerButton", x, y, w, h))
		warning("GlobalOptions_Cloud_Container.RunServerButton's position is undefined");
	serverButtonY = y;
	if (!g_gui.xmlEval()->getWidgetData("GlobalOptions_Cloud_Container.ServerInfoLabel", x, y, w, h))
		warning("GlobalOptions_Cloud_Container.ServerInfoLabel's position is undefined");
	serverInfoY = y;

	if (!g_gui.xmlEval()->getWidgetData("GlobalOptions_Cloud_Container.RootPathButton", x, y, w, h))
		warning("GlobalOptions_Cloud_Container.RootPathButton's position is undefined");
	serverRootButtonY = y;
	if (!g_gui.xmlEval()->getWidgetData("GlobalOptions_Cloud_Container.RootPath", x, y, w, h))
		warning("GlobalOptions_Cloud_Container.RootPath's position is undefined");
	serverRootY = y;
	if (!g_gui.xmlEval()->getWidgetData("GlobalOptions_Cloud_Container.RootPathClearButton", x, y, w, h))
		warning("GlobalOptions_Cloud_Container.RootPathClearButton's position is undefined");
	serverRootClearButtonY = y;

	if (!g_gui.xmlEval()->getWidgetData("GlobalOptions_Cloud_Container.ServerPortDesc", x, y, w, h))
		warning("GlobalOptions_Cloud_Container.ServerPortDesc's position is undefined");
	serverPortDescY = y;
	if (!g_gui.xmlEval()->getWidgetData("GlobalOptions_Cloud_Container.ServerPortEditText", x, y, w, h))
		warning("GlobalOptions_Cloud_Container.ServerPortEditText's position is undefined");
	serverPortY = y;
	if (!g_gui.xmlEval()->getWidgetData("GlobalOptions_Cloud_Container.ServerPortClearButton", x, y, w, h))
		warning("GlobalOptions_Cloud_Container.ServerPortClearButton's position is undefined");
	serverPortClearButtonY = y;

	bool serverIsRunning = LocalServer.isRunning();

	if (serverLabelPosition < 0)
		serverLabelPosition = serverInfoY;
	if (_runServerButton) {
		_runServerButton->setVisible(true);
		_runServerButton->setPos(_runServerButton->getRelX(), serverLabelPosition + serverButtonY - serverInfoY);
		_runServerButton->setLabel(_(serverIsRunning ? "Stop server" : "Run server"));
		_runServerButton->setTooltip(_(serverIsRunning ? "Stop local webserver" : "Run local webserver"));
	}
	if (_serverInfoLabel) {
		_serverInfoLabel->setVisible(true);
		_serverInfoLabel->setPos(_serverInfoLabel->getRelX(), serverLabelPosition);
		if (serverIsRunning)
			_serverInfoLabel->setLabel(LocalServer.getAddress());
		else
			_serverInfoLabel->setLabel(_("Not running"));
	}
	if (_rootPathButton) {
		_rootPathButton->setVisible(true);
		_rootPathButton->setPos(_rootPathButton->getRelX(), serverLabelPosition + serverRootButtonY - serverInfoY);
	}
	if (_rootPath) {
		_rootPath->setVisible(true);
		_rootPath->setPos(_rootPath->getRelX(), serverLabelPosition + serverRootY - serverInfoY);
	}
	if (_rootPathClearButton) {
		_rootPathClearButton->setVisible(true);
		_rootPathClearButton->setPos(_rootPathClearButton->getRelX(), serverLabelPosition + serverRootClearButtonY - serverInfoY);
	}
#ifdef NETWORKING_LOCALWEBSERVER_ENABLE_PORT_OVERRIDE
	if (_serverPortDesc) {
		_serverPortDesc->setVisible(true);
		_serverPortDesc->setPos(_serverPortDesc->getRelX(), serverLabelPosition + serverPortDescY - serverInfoY);
		_serverPortDesc->setEnabled(!serverIsRunning);
	}
	if (_serverPort) {
		_serverPort->setVisible(true);
		_serverPort->setPos(_serverPort->getRelX(), serverLabelPosition + serverPortY - serverInfoY);
		_serverPort->setEnabled(!serverIsRunning);
	}
	if (_serverPortClearButton) {
		_serverPortClearButton->setVisible(true);
		_serverPortClearButton->setPos(_serverPortClearButton->getRelX(), serverLabelPosition + serverPortClearButtonY - serverInfoY);
		_serverPortClearButton->setEnabled(!serverIsRunning);
	}
#else // NETWORKING_LOCALWEBSERVER_ENABLE_PORT_OVERRIDE
	if (_serverPortDesc)
		_serverPortDesc->setVisible(false);
	if (_serverPort)
		_serverPort->setVisible(false);
	if (_serverPortClearButton)
		_serverPortClearButton->setVisible(false);
#endif // NETWORKING_LOCALWEBSERVER_ENABLE_PORT_OVERRIDE
#else // USE_SDL_NET
	if (_runServerButton)
		_runServerButton->setVisible(false);
	if (_serverInfoLabel) {
		_serverInfoLabel->setPos(_serverInfoLabel->getRelX(), serverLabelPosition); // Prevent compiler warning from serverLabelPosition being unused.
		_serverInfoLabel->setVisible(false);
	}
	if (_rootPathButton)
		_rootPathButton->setVisible(false);
	if (_rootPath)
		_rootPath->setVisible(false);
	if (_rootPathClearButton)
		_rootPathClearButton->setVisible(false);
	if (_serverPortDesc)
		_serverPortDesc->setVisible(false);
	if (_serverPort)
		_serverPort->setVisible(false);
	if (_serverPortClearButton)
		_serverPortClearButton->setVisible(false);
#endif // USE_SDL_NET
}

#ifdef USE_LIBCURL
void GlobalOptionsDialog::storageInfoCallback(Cloud::Storage::StorageInfoResponse response) {
	//we could've used response.value.email()
	//but Storage already notified CloudMan
	//so we just set the flag to redraw our cloud tab
	_redrawCloudTab = true;
}

void GlobalOptionsDialog::storageListDirectoryCallback(Cloud::Storage::ListDirectoryResponse response) {
	Common::Array<Cloud::StorageFile> &files = response.value;
	uint64 totalSize = 0;
	for (uint32 i = 0; i < files.size(); ++i)
		if (!files[i].isDirectory())
			totalSize += files[i].size();
	CloudMan.setStorageUsedSpace(CloudMan.getStorageIndex(), totalSize);
	_redrawCloudTab = true;
}

void GlobalOptionsDialog::storageErrorCallback(Networking::ErrorResponse response) {
	debug(9, "GlobalOptionsDialog: error response (%s, %ld):", (response.failed ? "failed" : "interrupted"), response.httpResponseCode);
	debug(9, "%s", response.response.c_str());

	if (!response.interrupted)
		g_system->displayMessageOnOSD(_("Request failed.\nCheck your Internet connection."));
}
#endif // USE_LIBCURL
#endif // USE_CLOUD

} // End of namespace GUI
