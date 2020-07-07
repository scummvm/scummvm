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

#include "bladerunner/bladerunner.h" // for BLADERUNNER_ORIGINAL_SETTINGS macro
#include "bladerunner/color.h"
#include "bladerunner/ui/kia_section_base.h"

#include "common/config-manager.h"
#include "common/rect.h"

namespace BladeRunner {

class BladeRunnerEngine;
class UIContainer;
class UICheckBox;
class UIImagePicker;
class UISlider;
class UIScrollBox;
class UIDropDown;

class KIASectionSettings : public KIASectionBase {
	enum State {
		kStateNormal         = 0,
		kStateLanguageSelect = 1
	};

	static const char *kLeary;
	static const Color256 kColors[];

	UIContainer   *_uiContainer;
	UISlider      *_musicVolume;
	UISlider      *_soundEffectVolume;
	UISlider      *_speechVolume;
#if BLADERUNNER_ORIGINAL_SETTINGS
	UISlider      *_ambientSoundVolume;
	UISlider      *_gammaCorrection;
#endif
	UICheckBox    *_directorsCut;
	UICheckBox    *_subtitlesEnable;
	Common::String _selectedTextLanguageStr;
	int            _selectedTextLanguageId;

	UIDropDown    *_textLanguageDropdown;
	UIImagePicker *_playerAgendaSelector;

	int            _mouseX;
	int            _mouseY;

	int            _learyPos;

	State          _state;

public:
	KIASectionSettings(BladeRunnerEngine *vm);
	~KIASectionSettings() override;

	void open() override;
	void close() override;

	void draw(Graphics::Surface &surface) override;

	void handleKeyDown(const Common::KeyState &kbd) override;
	void handleMouseMove(int mouseX, int mouseY) override;
	void handleMouseDown(bool mainButton) override;
	void handleMouseUp(bool mainButton) override;
	void handleMouseScroll(int direction) override;

	void showTextSelectionDropdown(bool showToggle);

private:
	static void sliderCallback(void *callbackData, void *source);
	static void checkBoxCallback(void *callbackData, void *source);
	static void mouseInCallback(int buttonId, void *callbackData);
	static void mouseUpCallback(int buttonId, void *callbackData);
	static void dropdownSelectedCallback(void *callbackData, void *source, int lineData, int mouseButton);
	static void dropdownCancelledCallback(void *callbackData, void *source);
	static void dropdownClickedTopFrameCallback(void *callbackData, void *source);

	void onButtonPressed(int buttonId) override;

	void initConversationChoices();
	void populateLanguageSelection();

	void changeState(State state);
};

} // End of namespace BladeRunner

#endif
