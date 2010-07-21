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
#include "common/translation.h"

#include "graphics/scaler.h"

#include "sound/mididrv.h"
#include "sound/musicplugin.h"
#include "sound/mixer.h"
#include "sound/fmopl.h"

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
	kChooseThemeDirCmd		= 'chth',
	kChooseExtraDirCmd		= 'chex',
	kChoosePluginsDirCmd	= 'chpl',
	kChooseThemeCmd			= 'chtf'
};

enum {
	kSubtitlesSpeech,
	kSubtitlesSubs,
	kSubtitlesBoth
};

#ifdef SMALL_SCREEN_DEVICE
enum {
	kChooseKeyMappingCmd    = 'chma'
};
#endif

static const char *savePeriodLabels[] = { _s("Never"), _s("every 5 mins"), _s("every 10 mins"), _s("every 15 mins"), _s("every 30 mins"), 0 };
static const int savePeriodValues[] = { 0, 5 * 60, 10 * 60, 15 * 60, 30 * 60, -1 };
static const char *outputRateLabels[] = { _s("<default>"), _s("8 kHz"), _s("11kHz"), _s("22 kHz"), _s("44 kHz"), _s("48 kHz"), 0 };
static const int outputRateValues[] = { 0, 8000, 11025, 22050, 44100, 48000, -1 };



OptionsDialog::OptionsDialog(const Common::String &domain, int x, int y, int w, int h)
	: Dialog(x, y, w, h), _domain(domain), _graphicsTabId(-1), _tabWidget(0) {
	init();
}

OptionsDialog::OptionsDialog(const Common::String &domain, const Common::String &name)
	: Dialog(name), _domain(domain), _graphicsTabId(-1), _tabWidget(0) {
	init();
}

void OptionsDialog::init() {
	_enableGraphicSettings = false;
	_gfxPopUp = 0;
	_renderModePopUp = 0;
	_fullscreenCheckbox = 0;
	_aspectCheckbox = 0;
	_enableAudioSettings = false;
	_midiPopUp = 0;
	_mt32DevicePopUp = 0;
	_gmDevicePopUp = 0;
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
	_subToggleGroup = 0;
	_subToggleSubOnly = 0;
	_subToggleSpeechOnly = 0;
	_subToggleSubBoth = 0;
	_subSpeedDesc = 0;
	_subSpeedSlider = 0;
	_subSpeedLabel = 0;

	// Retrieve game GUI options
	_guioptions = 0;
	if (ConfMan.hasKey("guioptions", _domain)) {
		_guioptionsString = ConfMan.get("guioptions", _domain);
		_guioptions = parseGameGUIOptions(_guioptionsString);
	}
}

void OptionsDialog::open() {
	Dialog::open();

	// Reset result value
	setResult(0);

	// Retrieve game GUI options
	_guioptions = 0;
	if (ConfMan.hasKey("guioptions", _domain)) {
		_guioptionsString = ConfMan.get("guioptions", _domain);
		_guioptions = parseGameGUIOptions(_guioptionsString);
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
					sel = i + 2;
			}
			_renderModePopUp->setSelected(sel);
		}

#ifdef SMALL_SCREEN_DEVICE
		_fullscreenCheckbox->setState(true);
		_fullscreenCheckbox->setEnabled(false);
		_aspectCheckbox->setEnabled(false);
#else // !SMALL_SCREEN_DEVICE
		// Fullscreen setting
		_fullscreenCheckbox->setState(ConfMan.getBool("fullscreen", _domain));

		// Aspect ratio setting
		_aspectCheckbox->setState(ConfMan.getBool("aspect_ratio", _domain));
#endif // SMALL_SCREEN_DEVICE
	}

	// Audio options
	if (!loadMusicDeviceSetting(_midiPopUp, "music_driver"))
		_midiPopUp->setSelected(0);

	if (!loadMusicDeviceSetting(_mt32DevicePopUp, "mt32_device")) {
		if (_domain.equals(Common::ConfigManager::kApplicationDomain)) {
			if (!loadMusicDeviceSetting(_mt32DevicePopUp, Common::String(), MT_MT32))
				_mt32DevicePopUp->setSelected(0);
		} else {
			_mt32DevicePopUp->setSelected(0);
		}
	}

	if (!loadMusicDeviceSetting(_gmDevicePopUp, "gm_device")) {
		if (_domain.equals(Common::ConfigManager::kApplicationDomain)) {
			if (!loadMusicDeviceSetting(_gmDevicePopUp, Common::String(), MT_GM))
				_gmDevicePopUp->setSelected(0);
		} else {
			_gmDevicePopUp->setSelected(0);
		}
	}

	if (_oplPopUp) {
		OPL::Config::DriverId id = MAX<OPL::Config::DriverId>(OPL::Config::parse(ConfMan.get("opl_driver", _domain)), 0);
		_oplPopUp->setSelectedTag(id);
	}

	if (_outputRatePopUp) {
		_outputRatePopUp->setSelected(1);
		int value = ConfMan.getInt("output_rate", _domain);
		for	(int i = 0; outputRateLabels[i]; i++) {
			if (value == outputRateValues[i])
				_outputRatePopUp->setSelected(i);
		}
	}

	if (_multiMidiCheckbox) {

		// Multi midi setting
		_multiMidiCheckbox->setState(ConfMan.getBool("multi_midi", _domain));

		// Native mt32 setting
		_mt32Checkbox->setState(ConfMan.getBool("native_mt32", _domain));

		// GS extensions setting
		_enableGSCheckbox->setState(ConfMan.getBool("enable_gs", _domain));

		Common::String soundFont(ConfMan.get("soundfont", _domain));
		if (soundFont.empty() || !ConfMan.hasKey("soundfont", _domain)) {
			_soundFont->setLabel(_("None"));
			_soundFontClearButton->setEnabled(false);
		} else {
			_soundFont->setLabel(soundFont);
			_soundFontClearButton->setEnabled(true);
		}

		// MIDI gain setting
		char buf[10];

		_midiGainSlider->setValue(ConfMan.getInt("midi_gain", _domain));
		sprintf(buf, "%.2f", (double)_midiGainSlider->getValue() / 100.0);
		_midiGainLabel->setLabel(buf);
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
				saveMusicDeviceSetting(_mt32DevicePopUp, "mt32_device");
				saveMusicDeviceSetting(_gmDevicePopUp, "gm_device");
			} else {
				ConfMan.removeKey("music_driver", _domain);
				ConfMan.removeKey("mt32_device", _domain);
				ConfMan.removeKey("gm_device", _domain);
			}
		}

		if (_oplPopUp) {
			if (_enableAudioSettings) {
				const OPL::Config::EmulatorDescription *ed = OPL::Config::getAvailable();
				while (ed->name && ed->id != (int)_oplPopUp->getSelectedTag())
					++ed;

				if (ed->name)
					ConfMan.set("opl_driver", ed->name, _domain);
				else
					ConfMan.removeKey("opl_driver", _domain);
			} else {
				ConfMan.removeKey("opl_driver", _domain);
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

		// MIDI options
		if (_multiMidiCheckbox) {
			if (_enableMIDISettings) {
				ConfMan.setBool("multi_midi", _multiMidiCheckbox->getState(), _domain);
				ConfMan.setBool("native_mt32", _mt32Checkbox->getState(), _domain);
				ConfMan.setBool("enable_gs", _enableGSCheckbox->getState(), _domain);
				ConfMan.setInt("midi_gain", _midiGainSlider->getValue(), _domain);

				Common::String soundFont(_soundFont->getLabel());
				if (!soundFont.empty() && (soundFont != _("None")))
					ConfMan.set("soundfont", soundFont, _domain);
				else
					ConfMan.removeKey("soundfont", _domain);
			} else {
				ConfMan.removeKey("multi_midi", _domain);
				ConfMan.removeKey("native_mt32", _domain);
				ConfMan.removeKey("enable_gs", _domain);
				ConfMan.removeKey("midi_gain", _domain);
				ConfMan.removeKey("soundfont", _domain);
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
	case kMuteAllChanged:
		// 'true' because if control is disabled then event do not pass
		setVolumeSettingsState(true);
		break;
	case kSubtitleSpeedChanged:
		_subSpeedLabel->setValue(_subSpeedSlider->getValue());
		_subSpeedLabel->draw();
		break;
	case kClearSoundFontCmd:
		_soundFont->setLabel(_("None"));
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
	_mt32DevicePopUpDesc->setEnabled(_domain.equals(Common::ConfigManager::kApplicationDomain) ? enabled : false);
	_mt32DevicePopUp->setEnabled(_domain.equals(Common::ConfigManager::kApplicationDomain) ? enabled : false);
	_gmDevicePopUpDesc->setEnabled(_domain.equals(Common::ConfigManager::kApplicationDomain) ? enabled : false);
	_gmDevicePopUp->setEnabled(_domain.equals(Common::ConfigManager::kApplicationDomain) ? enabled : false);

	uint32 allFlags = MidiDriver::musicType2GUIO((uint32)-1);

	if (_domain != Common::ConfigManager::kApplicationDomain && // global dialog
			(_guioptions & allFlags) && // No flags are specified
				!(_guioptions & Common::GUIO_MIDIADLIB)) {
		_oplPopUpDesc->setEnabled(false);
		_oplPopUp->setEnabled(false);
	} else {
		_oplPopUpDesc->setEnabled(enabled);
		_oplPopUp->setEnabled(enabled);
	}
	_outputRatePopUpDesc->setEnabled(enabled);
	_outputRatePopUp->setEnabled(enabled);
}

void OptionsDialog::setMIDISettingsState(bool enabled) {
	if (_guioptions & Common::GUIO_NOMIDI)
		enabled = false;

	_enableMIDISettings = enabled;

	_soundFontButton->setEnabled(enabled);
	_soundFont->setEnabled(enabled);

	if (enabled && !_soundFont->getLabel().empty() && (_soundFont->getLabel() != _("None")))
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

	ena = enabled && !_muteCheckbox->getState();
	if (_guioptions & Common::GUIO_NOMUSIC)
		ena = false;

	_musicVolumeDesc->setEnabled(ena);
	_musicVolumeSlider->setEnabled(ena);
	_musicVolumeLabel->setEnabled(ena);

	ena = enabled && !_muteCheckbox->getState();
	if (_guioptions & Common::GUIO_NOSFX)
		ena = false;

	_sfxVolumeDesc->setEnabled(ena);
	_sfxVolumeSlider->setEnabled(ena);
	_sfxVolumeLabel->setEnabled(ena);

	ena = enabled && !_muteCheckbox->getState();
	if (_guioptions & Common::GUIO_NOSPEECH)
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
	if ((_guioptions & Common::GUIO_NOSUBTITLES) || (_guioptions & Common::GUIO_NOSPEECH))
		ena = false;

	_subToggleGroup->setEnabled(ena);
	_subToggleDesc->setEnabled(ena);

	ena = enabled;
	if (_guioptions & Common::GUIO_NOSUBTITLES)
		ena = false;

	_subSpeedDesc->setEnabled(ena);
	_subSpeedSlider->setEnabled(ena);
	_subSpeedLabel->setEnabled(ena);
}

void OptionsDialog::addGraphicControls(GuiObject *boss, const Common::String &prefix) {
	const OSystem::GraphicsMode *gm = g_system->getSupportedGraphicsModes();

	// The GFX mode popup
	_gfxPopUpDesc = new StaticTextWidget(boss, prefix + "grModePopupDesc", _("Graphics mode:"));
	_gfxPopUp = new PopUpWidget(boss, prefix + "grModePopup");

	_gfxPopUp->appendEntry(_("<default>"));
	_gfxPopUp->appendEntry("");
	while (gm->name) {
		_gfxPopUp->appendEntry(_(gm->description), gm->id);
		gm++;
	}

	// RenderMode popup
	_renderModePopUpDesc = new StaticTextWidget(boss, prefix + "grRenderPopupDesc", _("Render mode:"), _("Special dithering modes supported by some games"));
	_renderModePopUp = new PopUpWidget(boss, prefix + "grRenderPopup", _("Special dithering modes supported by some games"));
	_renderModePopUp->appendEntry(_("<default>"), Common::kRenderDefault);
	_renderModePopUp->appendEntry("");
	const Common::RenderModeDescription *rm = Common::g_renderModes;
	for (; rm->code; ++rm) {
		_renderModePopUp->appendEntry(_(rm->description), rm->id);
	}

	// Fullscreen checkbox
	_fullscreenCheckbox = new CheckboxWidget(boss, prefix + "grFullscreenCheckbox", _("Fullscreen mode"));

	// Aspect ratio checkbox
	_aspectCheckbox = new CheckboxWidget(boss, prefix + "grAspectCheckbox", _("Aspect ratio correction"), _("Correct aspect ratio for 320x200 games"));

	_enableGraphicSettings = true;
}

void OptionsDialog::addAudioControls(GuiObject *boss, const Common::String &prefix) {
	// The MIDI mode popup & a label
	_midiPopUpDesc = new StaticTextWidget(boss, prefix + "auMidiPopupDesc", _domain == Common::ConfigManager::kApplicationDomain ? _("Preferred Device:") : _("Music Device:"), _domain == Common::ConfigManager::kApplicationDomain ? _("Specifies preferred sound device or sound card emulator") : _("Specifies output sound device or sound card emulator"));
	_midiPopUp = new PopUpWidget(boss, prefix + "auMidiPopup", _("Specifies output sound device or sound card emulator"));

	_mt32DevicePopUpDesc = new StaticTextWidget(boss, prefix + "auPrefMt32PopupDesc", _("MT-32 Device:"), _("Specifies default sound device for Roland MT-32/LAPC1/CM32l/CM64 output"));
	_mt32DevicePopUp = new PopUpWidget(boss, prefix + "auPrefMt32Popup");
	_gmDevicePopUpDesc = new StaticTextWidget(boss, prefix + "auPrefGmPopupDesc", _("GM Device:"), _("Specifies default sound device for General MIDI output"));
	_gmDevicePopUp = new PopUpWidget(boss, prefix + "auPrefGmPopup");

	// Populate it
	uint32 allFlags = MidiDriver::musicType2GUIO((uint32)-1);

	const MusicPlugin::List p = MusicMan.getPlugins();
	for (MusicPlugin::List::const_iterator m = p.begin(); m != p.end(); ++m) {
		MusicDevices i = (**m)->getDevices();
		for (MusicDevices::iterator d = i.begin(); d != i.end(); ++d) {
			if ((_domain == Common::ConfigManager::kApplicationDomain && d->getMusicType() != MT_TOWNS) // global dialog - skip useless FM-Towns option there
			    || (_domain != Common::ConfigManager::kApplicationDomain && !(_guioptions & allFlags)) // No flags are specified
			    || _guioptions & (MidiDriver::musicType2GUIO(d->getMusicType())) // flag is present
			    || d->getMusicDriverId() == "auto" || d->getMusicDriverId() == "null") // always add default and null device
					_midiPopUp->appendEntry(d->getCompleteName(), d->getHandle());

			if (d->getMusicType() >= MT_GM || d->getMusicDriverId() == "auto") {
				_mt32DevicePopUp->appendEntry(d->getCompleteName(), d->getHandle());
				if (d->getMusicType() != MT_MT32)
					_gmDevicePopUp->appendEntry(d->getCompleteName(), d->getHandle());
			}
		}
	}

	if (!_domain.equals(Common::ConfigManager::kApplicationDomain)) {
		_mt32DevicePopUpDesc->setEnabled(false);
		_mt32DevicePopUp->setEnabled(false);
		_gmDevicePopUpDesc->setEnabled(false);
		_gmDevicePopUp->setEnabled(false);
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

	// Sample rate settings
	_outputRatePopUpDesc = new StaticTextWidget(boss, prefix + "auSampleRatePopupDesc", _("Output rate:"), _("Higher value specifies better sound quality but may be not supported by your soundcard"));
	_outputRatePopUp = new PopUpWidget(boss, prefix + "auSampleRatePopup", _("Higher value specifies better sound quality but may be not supported by your soundcard"));

	for (int i = 0; outputRateLabels[i]; i++) {
		_outputRatePopUp->appendEntry(_(outputRateLabels[i]), outputRateValues[i]);
	}

	_enableAudioSettings = true;
}

void OptionsDialog::addMIDIControls(GuiObject *boss, const Common::String &prefix) {
	// SoundFont
	_soundFontButton = new ButtonWidget(boss, prefix + "mcFontButton", _("SoundFont:"), _("SoundFont is supported by some audio cards, Fluidsynth and Timidity"), kChooseSoundFontCmd);
	_soundFont = new StaticTextWidget(boss, prefix + "mcFontPath", _("None"), _("SoundFont is supported by some audio cards, Fluidsynth and Timidity"));
	_soundFontClearButton = new ButtonWidget(boss, prefix + "mcFontClearButton", "C", _("Clear value"), kClearSoundFontCmd);

	// Multi midi setting
	_multiMidiCheckbox = new CheckboxWidget(boss, prefix + "mcMixedCheckbox", _("Mixed AdLib/MIDI mode"), _("Use both MIDI and AdLib sound generation"));

	// Native mt32 setting
	_mt32Checkbox = new CheckboxWidget(boss, prefix + "mcMt32Checkbox", _("True Roland MT-32 (disable GM emulation)"), _("Check if you want to use your real hardware Roland-compatible sound device connected to your computer"));

	// GS Extensions setting
	_enableGSCheckbox = new CheckboxWidget(boss, prefix + "mcGSCheckbox", _("Enable Roland GS Mode"), _("Turns off General MIDI mapping for games with Roland MT-32 soundtrack"));

	// MIDI gain setting (FluidSynth uses this)
	_midiGainDesc = new StaticTextWidget(boss, prefix + "mcMidiGainText", _("MIDI gain:"));
	_midiGainSlider = new SliderWidget(boss, prefix + "mcMidiGainSlider", 0, kMidiGainChanged);
	_midiGainSlider->setMinValue(0);
	_midiGainSlider->setMaxValue(1000);
	_midiGainLabel = new StaticTextWidget(boss, prefix + "mcMidiGainLabel", "1.00");

	_enableMIDISettings = true;
}

// The function has an extra slider range parameter, since both the launcher and SCUMM engine
// make use of the widgets. The launcher range is 0-255. SCUMM's 0-9
void OptionsDialog::addSubtitleControls(GuiObject *boss, const Common::String &prefix, int maxSliderVal) {

	_subToggleDesc = new StaticTextWidget(boss, prefix + "subToggleDesc", _("Text and Speech:"));

	if (g_system->getOverlayWidth() > 320) {
		_subToggleGroup = new RadiobuttonGroup(boss, kSubtitleToggle);

		_subToggleSpeechOnly = new RadiobuttonWidget(boss, prefix + "subToggleSpeechOnly", _subToggleGroup, kSubtitlesSpeech, _("Speech"));
		_subToggleSubOnly = new RadiobuttonWidget(boss, prefix + "subToggleSubOnly", _subToggleGroup, kSubtitlesSubs, _("Subtitles"));
		_subToggleSubBoth = new RadiobuttonWidget(boss, prefix + "subToggleSubBoth", _subToggleGroup, kSubtitlesBoth, _("Both"));
	} else {
		_subToggleGroup = new RadiobuttonGroup(boss, kSubtitleToggle);

		_subToggleSpeechOnly = new RadiobuttonWidget(boss, prefix + "subToggleSpeechOnly", _subToggleGroup, kSubtitlesSpeech, _("Spch"), _("Speech"));
		_subToggleSubOnly = new RadiobuttonWidget(boss, prefix + "subToggleSubOnly", _subToggleGroup, kSubtitlesSubs, _("Subs"), _("Subtitles"));
		_subToggleSubBoth = new RadiobuttonWidget(boss, prefix + "subToggleSubBoth", _subToggleGroup, kSubtitlesBoth, _("Both"), _("Show subtitles and play speech"));
	}

	// Subtitle speed
	_subSpeedDesc = new StaticTextWidget(boss, prefix + "subSubtitleSpeedDesc", _("Subtitle speed:"));
	_subSpeedSlider = new SliderWidget(boss, prefix + "subSubtitleSpeedSlider", 0, kSubtitleSpeedChanged);
	_subSpeedLabel = new StaticTextWidget(boss, prefix + "subSubtitleSpeedLabel", "100%");
	_subSpeedSlider->setMinValue(0); _subSpeedSlider->setMaxValue(maxSliderVal);
	_subSpeedLabel->setFlags(WIDGET_CLEARBG);

	_enableSubtitleSettings = true;
}

void OptionsDialog::addVolumeControls(GuiObject *boss, const Common::String &prefix) {

	// Volume controllers
	_musicVolumeDesc = new StaticTextWidget(boss, prefix + "vcMusicText", _("Music volume:"));
	_musicVolumeSlider = new SliderWidget(boss, prefix + "vcMusicSlider", 0, kMusicVolumeChanged);
	_musicVolumeLabel = new StaticTextWidget(boss, prefix + "vcMusicLabel", "100%");
	_musicVolumeSlider->setMinValue(0);
	_musicVolumeSlider->setMaxValue(Audio::Mixer::kMaxMixerVolume);
	_musicVolumeLabel->setFlags(WIDGET_CLEARBG);

	_muteCheckbox = new CheckboxWidget(boss, prefix + "vcMuteCheckbox", _("Mute All"), 0, kMuteAllChanged);


	_sfxVolumeDesc = new StaticTextWidget(boss, prefix + "vcSfxText", _("SFX volume:"), _("Special sound effects volume"));
	_sfxVolumeSlider = new SliderWidget(boss, prefix + "vcSfxSlider", _("Special sound effects volume"), kSfxVolumeChanged);
	_sfxVolumeLabel = new StaticTextWidget(boss, prefix + "vcSfxLabel", "100%");
	_sfxVolumeSlider->setMinValue(0);
	_sfxVolumeSlider->setMaxValue(Audio::Mixer::kMaxMixerVolume);
	_sfxVolumeLabel->setFlags(WIDGET_CLEARBG);

	_speechVolumeDesc = new StaticTextWidget(boss, prefix + "vcSpeechText" , _("Speech volume:"));
	_speechVolumeSlider = new SliderWidget(boss, prefix + "vcSpeechSlider", 0, kSpeechVolumeChanged);
	_speechVolumeLabel = new StaticTextWidget(boss, prefix + "vcSpeechLabel", "100%");
	_speechVolumeSlider->setMinValue(0);
	_speechVolumeSlider->setMaxValue(Audio::Mixer::kMaxMixerVolume);
	_speechVolumeLabel->setFlags(WIDGET_CLEARBG);

	_enableVolumeSettings = true;
}

bool OptionsDialog::loadMusicDeviceSetting(PopUpWidget *popup, Common::String setting, MusicType preferredType) {
	if (!popup || !popup->isEnabled())
		return true;

	if (_domain != Common::ConfigManager::kApplicationDomain || ConfMan.hasKey(setting, _domain) || preferredType) {
		const Common::String drv = ConfMan.get(setting, (_domain != Common::ConfigManager::kApplicationDomain && !ConfMan.hasKey(setting, _domain)) ? Common::ConfigManager::kApplicationDomain : _domain);
		const MusicPlugin::List p = MusicMan.getPlugins();

		for (MusicPlugin::List::const_iterator m = p.begin(); m != p.end(); ++m) {
			MusicDevices i = (**m)->getDevices();
			for (MusicDevices::iterator d = i.begin(); d != i.end(); ++d) {
				if (setting.empty() ? (preferredType == d->getMusicType()) : (drv == d->getCompleteId())) {
					popup->setSelectedTag(d->getHandle());
					return popup->getSelected() == -1 ? false : true;
				}
			}
		}
	}

	return false;
}

void OptionsDialog::saveMusicDeviceSetting(PopUpWidget *popup, Common::String setting) {
	if (!popup || !_enableAudioSettings)
		return;

	const MusicPlugin::List p = MusicMan.getPlugins();
	bool found = false;
	for (MusicPlugin::List::const_iterator m = p.begin(); m != p.end() && !found; ++m) {
		MusicDevices i = (**m)->getDevices();
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
	if (_guioptions & Common::GUIO_NOSUBTITLES)
		return kSubtitlesSpeech; // Speech only
	if (_guioptions & Common::GUIO_NOSPEECH)
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

void OptionsDialog::reflowLayout() {
	if (_graphicsTabId != -1 && _tabWidget)
		_tabWidget->setTabTitle(_graphicsTabId, g_system->getOverlayWidth() > 320 ? _("Graphics") : _("GFX"));

	Dialog::reflowLayout();
}

#pragma mark -


GlobalOptionsDialog::GlobalOptionsDialog()
	: OptionsDialog(Common::ConfigManager::kApplicationDomain, "GlobalOptions") {

	// The tab widget
	TabWidget *tab = new TabWidget(this, "GlobalOptions.TabWidget");

	//
	// 1) The graphics tab
	//
	_graphicsTabId = tab->addTab(g_system->getOverlayWidth() > 320 ? _("Graphics") : _("GFX"));
	addGraphicControls(tab, "GlobalOptions_Graphics.");

	//
	// 2) The audio tab
	//
	tab->addTab(_("Audio"));
	addAudioControls(tab, "GlobalOptions_Audio.");
	addSubtitleControls(tab, "GlobalOptions_Audio.");

	tab->addTab(_("Volume"));
	addVolumeControls(tab, "GlobalOptions_Volume.");

	// TODO: cd drive setting

	//
	// 3) The MIDI tab
	//
	tab->addTab(_("MIDI"));
	addMIDIControls(tab, "GlobalOptions_MIDI.");

	//
	// 4) The miscellaneous tab
	//
	tab->addTab(_("Paths"));

#if !( defined(__DC__) || defined(__GP32__) )
	// These two buttons have to be extra wide, or the text will be
	// truncated in the small version of the GUI.

	// Save game path
	new ButtonWidget(tab, "GlobalOptions_Paths.SaveButton", _("Save Path: "), _("Specifies where your savegames are put"), kChooseSaveDirCmd);
	_savePath = new StaticTextWidget(tab, "GlobalOptions_Paths.SavePath", "/foo/bar", _("Specifies where your savegames are put"));

	new ButtonWidget(tab, "GlobalOptions_Paths.ThemeButton", _("Theme Path:"), 0, kChooseThemeDirCmd);
	_themePath = new StaticTextWidget(tab, "GlobalOptions_Paths.ThemePath", _("None"));

	new ButtonWidget(tab, "GlobalOptions_Paths.ExtraButton", _("Extra Path:"), _("Specifies path to additional data used by all games or ScummVM"), kChooseExtraDirCmd);
	_extraPath = new StaticTextWidget(tab, "GlobalOptions_Paths.ExtraPath", _("None"), _("Specifies path to additional data used by all games or ScummVM"));

#ifdef DYNAMIC_MODULES
	new ButtonWidget(tab, "GlobalOptions_Paths.PluginsButton", _("Plugins Path:"), 0, kChoosePluginsDirCmd);
	_pluginsPath = new StaticTextWidget(tab, "GlobalOptions_Paths.PluginsPath", _("None"));
#endif
#endif

	tab->addTab(_("Misc"));

	new ButtonWidget(tab, "GlobalOptions_Misc.ThemeButton", _("Theme:"), 0, kChooseThemeCmd);
	_curTheme = new StaticTextWidget(tab, "GlobalOptions_Misc.CurTheme", g_gui.theme()->getThemeName());


	_rendererPopUpDesc = new StaticTextWidget(tab, "GlobalOptions_Misc.RendererPopupDesc", _("GUI Renderer:"));
	_rendererPopUp = new PopUpWidget(tab, "GlobalOptions_Misc.RendererPopup");

	for (uint i = 1; i < GUI::ThemeEngine::_rendererModesSize; ++i)
		_rendererPopUp->appendEntry(_(GUI::ThemeEngine::_rendererModes[i].name), GUI::ThemeEngine::_rendererModes[i].mode);

	_autosavePeriodPopUpDesc = new StaticTextWidget(tab, "GlobalOptions_Misc.AutosavePeriodPopupDesc", _("Autosave:"));
	_autosavePeriodPopUp = new PopUpWidget(tab, "GlobalOptions_Misc.AutosavePeriodPopup");

	for (int i = 0; savePeriodLabels[i]; i++) {
		_autosavePeriodPopUp->appendEntry(_(savePeriodLabels[i]), savePeriodValues[i]);
	}

#ifdef SMALL_SCREEN_DEVICE
	new ButtonWidget(tab, "GlobalOptions_Misc.KeysButton", _("Keys"), 0, kChooseKeyMappingCmd);
#endif

	// TODO: joystick setting


#ifdef USE_TRANSLATION
	_guiLanguagePopUpDesc = new StaticTextWidget(tab, "GlobalOptions_Misc.GuiLanguagePopupDesc", _("GUI Language:"), _("Language of ScummVM GUI"));
	_guiLanguagePopUp = new PopUpWidget(tab, "GlobalOptions_Misc.GuiLanguagePopup");
#ifdef USE_DETECTLANG
	_guiLanguagePopUp->appendEntry(_("<default>"), Common::kTranslationAutodetectId);
#endif // USE_DETECTLANG
	_guiLanguagePopUp->appendEntry(_("English"), Common::kTranslationBuiltinId);
	_guiLanguagePopUp->appendEntry("", 0);
	Common::TLangArray languages = TransMan.getSupportedLanguages();
	Common::TLangArray::iterator lang = languages.begin();
	while (lang != languages.end()) {
		_guiLanguagePopUp->appendEntry(lang->name, lang->id);
		lang++;
	}

	// Select the currently configured language or default/English if
	// nothing is specified.
	if (ConfMan.hasKey("gui_language"))
		_guiLanguagePopUp->setSelectedTag(TransMan.parseLanguage(ConfMan.get("gui_language")));
	else
#ifdef USE_DETECTLANG
		_guiLanguagePopUp->setSelectedTag(Common::kTranslationAutodetectId);
#else // !USE_DETECTLANG
		_guiLanguagePopUp->setSelectedTag(Common::kTranslationBuiltinId);
#endif // USE_DETECTLANG

#endif // USE_TRANSLATION

	// Activate the first tab
	tab->setActiveTab(0);
	_tabWidget = tab;

	// Add OK & Cancel buttons
	new ButtonWidget(this, "GlobalOptions.Cancel", _("Cancel"), 0, kCloseCmd);
	new ButtonWidget(this, "GlobalOptions.Ok", _("OK"), 0, kOKCmd);

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
		_savePath->setLabel(_("None"));
	} else {
		_savePath->setLabel(savePath);
	}

	if (themePath.empty() || !ConfMan.hasKey("themepath", _domain)) {
		_themePath->setLabel(_("None"));
	} else {
		_themePath->setLabel(themePath);
	}

	if (extraPath.empty() || !ConfMan.hasKey("extrapath", _domain)) {
		_extraPath->setLabel(_("None"));
	} else {
		_extraPath->setLabel(extraPath);
	}

#ifdef DYNAMIC_MODULES
	Common::String pluginsPath(ConfMan.get("pluginspath", _domain));
	if (pluginsPath.empty() || !ConfMan.hasKey("pluginspath", _domain)) {
		_pluginsPath->setLabel(_("None"));
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
		Common::String savePath(_savePath->getLabel());
		if (!savePath.empty() && (savePath != _("None")))
			ConfMan.set("savepath", savePath, _domain);

		Common::String themePath(_themePath->getLabel());
		if (!themePath.empty() && (themePath != _("None")))
			ConfMan.set("themepath", themePath, _domain);
		else
			ConfMan.removeKey("themepath", _domain);

		Common::String extraPath(_extraPath->getLabel());
		if (!extraPath.empty() && (extraPath != _("None")))
			ConfMan.set("extrapath", extraPath, _domain);
		else
			ConfMan.removeKey("extrapath", _domain);

#ifdef DYNAMIC_MODULES
		Common::String pluginsPath(_pluginsPath->getLabel());
		if (!pluginsPath.empty() && (pluginsPath != _("None")))
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
#ifdef USE_TRANSLATION
		Common::String oldLang = ConfMan.get("gui_language");
		int selLang = _guiLanguagePopUp->getSelectedTag();

		ConfMan.set("gui_language", TransMan.getLangById(selLang));

		Common::String newLang = ConfMan.get("gui_language").c_str();
		if (newLang != oldLang) {
#if 0
			// Activate the selected language
			TransMan.setLanguage(selLang);

			// FIXME: Actually, any changes (including the theme change) should
			// only become active *after* the options dialog has closed.
			g_gui.loadNewTheme(g_gui.theme()->getThemeId(), ThemeEngine::kGfxDisabled, true);
#else
			MessageDialog error(_("You have to restart ScummVM to take the effect."));
			error.runModal();
#endif
		}
#endif // USE_TRANSLATION

	}
	OptionsDialog::close();
}

void GlobalOptionsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kChooseSaveDirCmd: {
		BrowserDialog browser(_("Select directory for savegames"), true);
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
			draw();
		}
		break;
	}
	case kChooseThemeDirCmd: {
		BrowserDialog browser(_("Select directory for GUI themes"), true);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(browser.getResult());
			_themePath->setLabel(dir.getPath());
			draw();
		}
		break;
	}
	case kChooseExtraDirCmd: {
		BrowserDialog browser(_("Select directory for extra files"), true);
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
		BrowserDialog browser(_("Select directory for plugins"), true);
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
		BrowserDialog browser(_("Select SoundFont"), false);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode file(browser.getResult());
			_soundFont->setLabel(file.getPath());

			if (!file.getPath().empty() && (file.getPath() != _("None")))
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
