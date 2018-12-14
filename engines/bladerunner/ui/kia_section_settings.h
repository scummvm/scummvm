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

#ifndef BLADERUNNER_KIA_SECTION_SETTINGS_H
#define BLADERUNNER_KIA_SECTION_SETTINGS_H

#include "bladerunner/ui/kia_section_base.h"

namespace BladeRunner {

class BladeRunnerEngine;
class UIContainer;
class UICheckBox;
class UIImagePicker;
class UISlider;

class KIASectionSettings : public KIASectionBase {
	static const char *kLeary;

	UIContainer   *_uiContainer;
	UISlider      *_musicVolume;
	UISlider      *_soundEffectVolume;
	UISlider      *_ambientSoundVolume;
	UISlider      *_speechVolume;
	UISlider      *_gammaCorrection;
	UICheckBox    *_directorsCut;
	UICheckBox    *_subtitlesEnable;
	UIImagePicker *_playerAgendaSelector;

	int            _mouseX;
	int            _mouseY;

	int            _learyPos;

public:
	KIASectionSettings(BladeRunnerEngine *vm);
	~KIASectionSettings();

	void open();
	void close();

	void draw(Graphics::Surface &surface);

	void handleKeyUp(const Common::KeyState &kbd);
	void handleMouseMove(int mouseX, int mouseY);
	void handleMouseDown(bool mainButton);
	void handleMouseUp(bool mainButton);

private:
	static void sliderCallback(void *callbackData, void *source);
	static void checkBoxCallback(void *callbackData, void *source);
	static void mouseInCallback(int buttonId, void *callbackData);
	static void mouseUpCallback(int buttonId, void *callbackData);

	void onButtonPressed(int buttonId);

	void initConversationChoices();
};

} // End of namespace BladeRunner

#endif
