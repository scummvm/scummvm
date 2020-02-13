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

#ifndef FLUIDSYNTH_DIALOG_H
#define FLUIDSYNTH_DIALOG_H

#include "common/str.h"
#include "gui/dialog.h"

namespace GUI {

class TabWidget;
class CheckboxWidget;
class SliderWidget;
class StaticTextWidget;
class PopUpWidget;

class FluidSynthSettingsDialog : public Dialog {
public:
	FluidSynthSettingsDialog();
	~FluidSynthSettingsDialog() override;

	void open() override;
	void close() override;
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;

protected:
	void setChorusSettingsState(bool enabled);
	void setReverbSettingsState(bool enabled);

	void readSettings();
	void writeSettings();

	void resetSettings();

private:
	Common::String _domain;

	TabWidget *_tabWidget;

	CheckboxWidget *_chorusActivate;

	StaticTextWidget *_chorusVoiceCountDesc;
	SliderWidget *_chorusVoiceCountSlider;
	StaticTextWidget *_chorusVoiceCountLabel;

	StaticTextWidget *_chorusLevelDesc;
	SliderWidget *_chorusLevelSlider;
	StaticTextWidget *_chorusLevelLabel;

	StaticTextWidget *_chorusSpeedDesc;
	SliderWidget *_chorusSpeedSlider;
	StaticTextWidget *_chorusSpeedLabel;

	StaticTextWidget *_chorusDepthDesc;
	SliderWidget *_chorusDepthSlider;
	StaticTextWidget *_chorusDepthLabel;

	StaticTextWidget *_chorusWaveFormTypePopUpDesc;
	PopUpWidget *_chorusWaveFormTypePopUp;

	CheckboxWidget *_reverbActivate;

	StaticTextWidget *_reverbRoomSizeDesc;
	SliderWidget *_reverbRoomSizeSlider;
	StaticTextWidget *_reverbRoomSizeLabel;

	StaticTextWidget *_reverbDampingDesc;
	SliderWidget *_reverbDampingSlider;
	StaticTextWidget *_reverbDampingLabel;

	StaticTextWidget *_reverbWidthDesc;
	SliderWidget *_reverbWidthSlider;
	StaticTextWidget *_reverbWidthLabel;

	StaticTextWidget *_reverbLevelDesc;
	SliderWidget *_reverbLevelSlider;
	StaticTextWidget *_reverbLevelLabel;

	StaticTextWidget *_miscInterpolationPopUpDesc;
	PopUpWidget *_miscInterpolationPopUp;
};

} // End of namespace GUI

#endif
