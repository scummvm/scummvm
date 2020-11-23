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

#include "gui/fluidsynth-dialog.h"
#include "gui/gui-manager.h"
#include "gui/message.h"
#include "gui/widgets/tab.h"
#include "gui/widgets/popup.h"

#include "common/config-manager.h"
#include "common/translation.h"
#include "common/debug.h"

namespace GUI {

enum {
	kActivateChorusCmd		= 'acho',
	kChorusVoiceCountChangedCmd	= 'cvcc',
	kChorusLevelChangedCmd		= 'clec',
	kChorusSpeedChangedCmd		= 'cspc',
	kChorusDepthChangedCmd		= 'cdec',

	kActivateReverbCmd		= 'arev',
	kReverbRoomSizeChangedCmd	= 'rrsc',
	kReverbDampingChangedCmd	= 'rdac',
	kReverbWidthChangedCmd		= 'rwic',
	kReverbLevelChangedCmd		= 'rlec',

	kResetSettingsCmd		= 'rese'
};

enum {
	kWaveFormTypeSine		= 0,
	kWaveFormTypeTriangle		= 1
};

enum {
	kInterpolationNone		= 0,
	kInterpolationLinear		= 1,
	kInterpolation4thOrder		= 2,
	kInterpolation7thOrder		= 3
};

FluidSynthSettingsDialog::FluidSynthSettingsDialog()
	: Dialog("FluidSynthSettings") {
	_domain = Common::ConfigManager::kApplicationDomain;

	_tabWidget = new TabWidget(this, "FluidSynthSettings.TabWidget");

	_tabWidget->addTab(_("Reverb"), "FluidSynthSettings_Reverb");

	_reverbActivate = new CheckboxWidget(_tabWidget, "FluidSynthSettings_Reverb.EnableTabCheckbox", _("Active"), Common::U32String(), kActivateReverbCmd);

	_reverbRoomSizeDesc = new StaticTextWidget(_tabWidget, "FluidSynthSettings_Reverb.RoomSizeText", _("Room:"));
	_reverbRoomSizeSlider = new SliderWidget(_tabWidget, "FluidSynthSettings_Reverb.RoomSizeSlider", Common::U32String(), kReverbRoomSizeChangedCmd);
	// 0.00 - 1.20, Default: 0.20
	_reverbRoomSizeSlider->setMinValue(0);
	_reverbRoomSizeSlider->setMaxValue(120);
	_reverbRoomSizeLabel = new StaticTextWidget(_tabWidget, "FluidSynthSettings_Reverb.RoomSizeLabel", Common::U32String("20"));

	_reverbDampingDesc = new StaticTextWidget(_tabWidget, "FluidSynthSettings_Reverb.DampingText", _("Damp:"));
	_reverbDampingSlider = new SliderWidget(_tabWidget, "FluidSynthSettings_Reverb.DampingSlider", Common::U32String(), kReverbDampingChangedCmd);
	// 0.00 - 1.00, Default: 0.00
	_reverbDampingSlider->setMinValue(0);
	_reverbDampingSlider->setMaxValue(100);
	_reverbDampingLabel = new StaticTextWidget(_tabWidget, "FluidSynthSettings_Reverb.DampingLabel", Common::U32String("0"));

	_reverbWidthDesc = new StaticTextWidget(_tabWidget, "FluidSynthSettings_Reverb.WidthText", _("Width:"));
	_reverbWidthSlider = new SliderWidget(_tabWidget, "FluidSynthSettings_Reverb.WidthSlider", Common::U32String(), kReverbWidthChangedCmd);
	// 0 - 100, Default: 1
	_reverbWidthSlider->setMinValue(0);
	_reverbWidthSlider->setMaxValue(100);
	_reverbWidthLabel = new StaticTextWidget(_tabWidget, "FluidSynthSettings_Reverb.WidthLabel", Common::U32String("1"));

	_reverbLevelDesc = new StaticTextWidget(_tabWidget, "FluidSynthSettings_Reverb.LevelText", _("Level:"));
	_reverbLevelSlider = new SliderWidget(_tabWidget, "FluidSynthSettings_Reverb.LevelSlider", Common::U32String(), kReverbLevelChangedCmd);
	// 0.00 - 1.00, Default: 0.90
	_reverbLevelSlider->setMinValue(0);
	_reverbLevelSlider->setMaxValue(100);
	_reverbLevelLabel = new StaticTextWidget(_tabWidget, "FluidSynthSettings_Reverb.LevelLabel", Common::U32String("90"));

	_tabWidget->addTab(_("Chorus"), "FluidSynthSettings_Chorus");

	_chorusActivate = new CheckboxWidget(_tabWidget, "FluidSynthSettings_Chorus.EnableTabCheckbox", _("Active"), Common::U32String(), kActivateChorusCmd);

	_chorusVoiceCountDesc = new StaticTextWidget(_tabWidget, "FluidSynthSettings_Chorus.VoiceCountText", _("N:"));
	_chorusVoiceCountSlider = new SliderWidget(_tabWidget, "FluidSynthSettings_Chorus.VoiceCountSlider", Common::U32String(), kChorusVoiceCountChangedCmd);
	// 0-99, Default: 3
	_chorusVoiceCountSlider->setMinValue(0);
	_chorusVoiceCountSlider->setMaxValue(99);
	_chorusVoiceCountLabel = new StaticTextWidget(_tabWidget, "FluidSynthSettings_Chorus.VoiceCountLabel", Common::U32String("3"));

	_chorusLevelDesc = new StaticTextWidget(_tabWidget, "FluidSynthSettings_Chorus.LevelText", _("Level:"));
	_chorusLevelSlider = new SliderWidget(_tabWidget, "FluidSynthSettings_Chorus.LevelSlider", Common::U32String(), kChorusLevelChangedCmd);
	// 0.00 - 1.00, Default: 1.00
	_chorusLevelSlider->setMinValue(0);
	_chorusLevelSlider->setMaxValue(100);
	_chorusLevelLabel = new StaticTextWidget(_tabWidget, "FluidSynthSettings_Chorus.LevelLabel", Common::U32String("100"));

	_chorusSpeedDesc = new StaticTextWidget(_tabWidget, "FluidSynthSettings_Chorus.SpeedText", _("Speed:"));
	_chorusSpeedSlider = new SliderWidget(_tabWidget, "FluidSynthSettings_Chorus.SpeedSlider", Common::U32String(), kChorusSpeedChangedCmd);
	// 0.30 - 5.00, Default: 0.30
	_chorusSpeedSlider->setMinValue(30);
	_chorusSpeedSlider->setMaxValue(500);
	_chorusSpeedLabel = new StaticTextWidget(_tabWidget, "FluidSynthSettings_Chorus.SpeedLabel", Common::U32String("30"));

	_chorusDepthDesc = new StaticTextWidget(_tabWidget, "FluidSynthSettings_Chorus.DepthText", _("Depth:"));
	_chorusDepthSlider = new SliderWidget(_tabWidget, "FluidSynthSettings_Chorus.DepthSlider", Common::U32String(), kChorusDepthChangedCmd);
	// 0.00 - 21.00, Default: 8.00
	_chorusDepthSlider->setMinValue(0);
	_chorusDepthSlider->setMaxValue(210);
	_chorusDepthLabel = new StaticTextWidget(_tabWidget, "FluidSynthSettings_Chorus.DepthLabel", Common::U32String("80"));

	_chorusWaveFormTypePopUpDesc = new StaticTextWidget(_tabWidget, "FluidSynthSettings_Chorus.WaveFormTypeText", _("Type:"));
	_chorusWaveFormTypePopUp = new PopUpWidget(_tabWidget, "FluidSynthSettings_Chorus.WaveFormType");

	_chorusWaveFormTypePopUp->appendEntry(_("Sine"), kWaveFormTypeSine);
	_chorusWaveFormTypePopUp->appendEntry(_("Triangle"), kWaveFormTypeTriangle);

	_tabWidget->addTab(_("Misc"), "FluidSynthSettings_Misc");

	_miscInterpolationPopUpDesc = new StaticTextWidget(_tabWidget, "FluidSynthSettings_Misc.InterpolationText", _("Interpolation:"));
	_miscInterpolationPopUp = new PopUpWidget(_tabWidget, "FluidSynthSettings_Misc.Interpolation");

	_miscInterpolationPopUp->appendEntry(_("None (fastest)"), kInterpolationNone);
	_miscInterpolationPopUp->appendEntry(_("Linear"), kInterpolationLinear);
	_miscInterpolationPopUp->appendEntry(_("Fourth-order"), kInterpolation4thOrder);
	_miscInterpolationPopUp->appendEntry(_("Seventh-order"), kInterpolation7thOrder);

	_tabWidget->setActiveTab(0);

	new ButtonWidget(this, "FluidSynthSettings.ResetSettings", _("Reset"), _("Reset all FluidSynth settings to their default values."), kResetSettingsCmd);

	new ButtonWidget(this, "FluidSynthSettings.Cancel", _("Cancel"), Common::U32String(), kCloseCmd);
	new ButtonWidget(this, "FluidSynthSettings.Ok", _("OK"), Common::U32String(), kOKCmd);
}

FluidSynthSettingsDialog::~FluidSynthSettingsDialog() {
}

void FluidSynthSettingsDialog::open() {
	Dialog::open();

	// Reset result value
	setResult(0);

	readSettings();
}

void FluidSynthSettingsDialog::close() {
	if (getResult()) {
		writeSettings();
	}

	Dialog::close();
}

void FluidSynthSettingsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kActivateChorusCmd:
		setChorusSettingsState(data);
		break;
	case kChorusVoiceCountChangedCmd:
		_chorusVoiceCountLabel->setLabel(Common::String::format("%d", _chorusVoiceCountSlider->getValue()));
		_chorusVoiceCountLabel->markAsDirty();
		break;
	case kChorusLevelChangedCmd:
		_chorusLevelLabel->setLabel(Common::String::format("%d", _chorusLevelSlider->getValue()));
		_chorusLevelLabel->markAsDirty();
		break;
	case kChorusSpeedChangedCmd:
		_chorusSpeedLabel->setLabel(Common::String::format("%d", _chorusSpeedSlider->getValue()));
		_chorusSpeedLabel->markAsDirty();
		break;
	case kChorusDepthChangedCmd:
		_chorusDepthLabel->setLabel(Common::String::format("%d", _chorusDepthSlider->getValue()));
		_chorusDepthLabel->markAsDirty();
		break;
	case kActivateReverbCmd:
		setReverbSettingsState(data);
		break;
	case kReverbRoomSizeChangedCmd:
		_reverbRoomSizeLabel->setLabel(Common::String::format("%d", _reverbRoomSizeSlider->getValue()));
		_reverbRoomSizeLabel->markAsDirty();
		break;
	case kReverbDampingChangedCmd:
		_reverbDampingLabel->setLabel(Common::String::format("%d", _reverbDampingSlider->getValue()));
		_reverbDampingLabel->markAsDirty();
		break;
	case kReverbWidthChangedCmd:
		_reverbWidthLabel->setLabel(Common::String::format("%d", _reverbWidthSlider->getValue()));
		_reverbWidthLabel->markAsDirty();
		break;
	case kReverbLevelChangedCmd:
		_reverbLevelLabel->setLabel(Common::String::format("%d", _reverbLevelSlider->getValue()));
		_reverbLevelLabel->markAsDirty();
		break;
	case kResetSettingsCmd: {
		MessageDialog alert(_("Do you really want to reset all FluidSynth settings to their default values?"), _("Yes"), _("No"));
		if (alert.runModal() == GUI::kMessageOK) {
			resetSettings();
			readSettings();
			g_gui.scheduleTopDialogRedraw();
		}
		break;
	}
	case kOKCmd:
		setResult(1);
		close();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
		break;
	}
}

void FluidSynthSettingsDialog::setChorusSettingsState(bool enabled) {
	_chorusVoiceCountDesc->setEnabled(enabled);
	_chorusVoiceCountSlider->setEnabled(enabled);
	_chorusVoiceCountLabel->setEnabled(enabled);
	_chorusLevelDesc->setEnabled(enabled);
	_chorusLevelSlider->setEnabled(enabled);
	_chorusLevelLabel->setEnabled(enabled);
	_chorusSpeedDesc->setEnabled(enabled);
	_chorusSpeedSlider->setEnabled(enabled);
	_chorusSpeedLabel->setEnabled(enabled);
	_chorusDepthDesc->setEnabled(enabled);
	_chorusDepthSlider->setEnabled(enabled);
	_chorusDepthLabel->setEnabled(enabled);
	_chorusWaveFormTypePopUpDesc->setEnabled(enabled);
	_chorusWaveFormTypePopUp->setEnabled(enabled);
}

void FluidSynthSettingsDialog::setReverbSettingsState(bool enabled) {
	_reverbRoomSizeDesc->setEnabled(enabled);
	_reverbRoomSizeSlider->setEnabled(enabled);
	_reverbRoomSizeLabel->setEnabled(enabled);
	_reverbDampingDesc->setEnabled(enabled);
	_reverbDampingSlider->setEnabled(enabled);
	_reverbDampingLabel->setEnabled(enabled);
	_reverbWidthDesc->setEnabled(enabled);
	_reverbWidthSlider->setEnabled(enabled);
	_reverbWidthLabel->setEnabled(enabled);
	_reverbLevelDesc->setEnabled(enabled);
	_reverbLevelSlider->setEnabled(enabled);
	_reverbLevelLabel->setEnabled(enabled);
}

void FluidSynthSettingsDialog::readSettings() {
	_chorusVoiceCountSlider->setValue(ConfMan.getInt("fluidsynth_chorus_nr", _domain));
	_chorusVoiceCountLabel->setLabel(Common::String::format("%d", _chorusVoiceCountSlider->getValue()));
	_chorusLevelSlider->setValue(ConfMan.getInt("fluidsynth_chorus_level", _domain));
	_chorusLevelLabel->setLabel(Common::String::format("%d", _chorusLevelSlider->getValue()));
	_chorusSpeedSlider->setValue(ConfMan.getInt("fluidsynth_chorus_speed", _domain));
	_chorusSpeedLabel->setLabel(Common::String::format("%d", _chorusSpeedSlider->getValue()));
	_chorusDepthSlider->setValue(ConfMan.getInt("fluidsynth_chorus_depth", _domain));
	_chorusDepthLabel->setLabel(Common::String::format("%d", _chorusDepthSlider->getValue()));

	Common::String waveForm = ConfMan.get("fluidsynth_chorus_waveform", _domain);
	if (waveForm == "sine") {
		_chorusWaveFormTypePopUp->setSelectedTag(kWaveFormTypeSine);
	} else if (waveForm == "triangle") {
		_chorusWaveFormTypePopUp->setSelectedTag(kWaveFormTypeTriangle);
	}

	_reverbRoomSizeSlider->setValue(ConfMan.getInt("fluidsynth_reverb_roomsize", _domain));
	_reverbRoomSizeLabel->setLabel(Common::String::format("%d", _reverbRoomSizeSlider->getValue()));
	_reverbDampingSlider->setValue(ConfMan.getInt("fluidsynth_reverb_damping", _domain));
	_reverbDampingLabel->setLabel(Common::String::format("%d", _reverbDampingSlider->getValue()));
	_reverbWidthSlider->setValue(ConfMan.getInt("fluidsynth_reverb_width", _domain));
	_reverbWidthLabel->setLabel(Common::String::format("%d", _reverbWidthSlider->getValue()));
	_reverbLevelSlider->setValue(ConfMan.getInt("fluidsynth_reverb_level", _domain));
	_reverbLevelLabel->setLabel(Common::String::format("%d", _reverbLevelSlider->getValue()));

	Common::String interpolation = ConfMan.get("fluidsynth_misc_interpolation", _domain);
	if (interpolation == "none") {
		_miscInterpolationPopUp->setSelectedTag(kInterpolationNone);
	} else if (interpolation == "linear") {
		_miscInterpolationPopUp->setSelectedTag(kInterpolationLinear);
	} else if (interpolation == "4th") {
		_miscInterpolationPopUp->setSelectedTag(kInterpolation4thOrder);
	} else if (interpolation == "7th") {
		_miscInterpolationPopUp->setSelectedTag(kInterpolation7thOrder);
	}

	// This may trigger redrawing, so don't do it until all sliders have
	// their proper values. Otherwise, the dialog may crash because of
	// invalid slider values.
	_chorusActivate->setState(ConfMan.getBool("fluidsynth_chorus_activate", _domain));
	_reverbActivate->setState(ConfMan.getBool("fluidsynth_reverb_activate", _domain));
}

void FluidSynthSettingsDialog::writeSettings() {
	ConfMan.setBool("fluidsynth_chorus_activate", _chorusActivate->getState());
	ConfMan.setInt("fluidsynth_chorus_nr", _chorusVoiceCountSlider->getValue(), _domain);
	ConfMan.setInt("fluidsynth_chorus_level", _chorusLevelSlider->getValue(), _domain);
	ConfMan.setInt("fluidsynth_chorus_speed", _chorusSpeedSlider->getValue(), _domain);
	ConfMan.setInt("fluidsynth_chorus_depth", _chorusDepthSlider->getValue(), _domain);

	uint32 waveForm = _chorusWaveFormTypePopUp->getSelectedTag();
	if (waveForm == kWaveFormTypeSine) {
		ConfMan.set("fluidsynth_chorus_waveform", "sine", _domain);
	} else if (waveForm == kWaveFormTypeTriangle) {
		ConfMan.set("fluidsynth_chorus_waveform", "triangle", _domain);
	} else {
		ConfMan.removeKey("fluidsynth_chorus_waveform", _domain);
	}

	ConfMan.setBool("fluidsynth_reverb_activate", _reverbActivate->getState());
	ConfMan.setInt("fluidsynth_reverb_roomsize", _reverbRoomSizeSlider->getValue(), _domain);
	ConfMan.setInt("fluidsynth_reverb_damping", _reverbDampingSlider->getValue(), _domain);
	ConfMan.setInt("fluidsynth_reverb_width", _reverbWidthSlider->getValue(), _domain);
	ConfMan.setInt("fluidsynth_reverb_level", _reverbLevelSlider->getValue(), _domain);

	uint32 interpolation = _miscInterpolationPopUp->getSelectedTag();
	if (interpolation == kInterpolationNone) {
		ConfMan.set("fluidsynth_misc_interpolation", "none", _domain);
	} else if (interpolation == kInterpolationLinear) {
		ConfMan.set("fluidsynth_misc_interpolation", "linear", _domain);
	} else if (interpolation == kInterpolation4thOrder) {
		ConfMan.set("fluidsynth_misc_interpolation", "4th", _domain);
	} else if (interpolation == kInterpolation7thOrder) {
		ConfMan.set("fluidsynth_misc_interpolation", "7th", _domain);
	} else {
		ConfMan.removeKey("fluidsynth_misc_interpolation", _domain);
	}

	// The main options dialog is responsible for writing the config file.
	// That's why we don't actually flush the settings to the file here.
}

void FluidSynthSettingsDialog::resetSettings() {
	ConfMan.removeKey("fluidsynth_chorus_activate", _domain);
	ConfMan.removeKey("fluidsynth_chorus_nr", _domain);
	ConfMan.removeKey("fluidsynth_chorus_level", _domain);
	ConfMan.removeKey("fluidsynth_chorus_speed", _domain);
	ConfMan.removeKey("fluidsynth_chorus_depth", _domain);
	ConfMan.removeKey("fluidsynth_chorus_waveform", _domain);

	ConfMan.removeKey("fluidsynth_reverb_activate", _domain);
	ConfMan.removeKey("fluidsynth_reverb_roomsize", _domain);
	ConfMan.removeKey("fluidsynth_reverb_damping", _domain);
	ConfMan.removeKey("fluidsynth_reverb_width", _domain);
	ConfMan.removeKey("fluidsynth_reverb_level", _domain);

	ConfMan.removeKey("fluidsynth_misc_interpolation", _domain);
}

} // End of namespace GUI
