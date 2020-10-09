/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 */

#ifndef STARK_UI_MENU_SETTINGS_MENU_H
#define STARK_UI_MENU_SETTINGS_MENU_H

#include "engines/stark/ui/menu/locationscreen.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/settings.h"

namespace Stark {

class VisualImageXMG;

/**
 * Manager of test sound
 */
class TestSoundManager {
public:
	TestSoundManager();
	~TestSoundManager() {}

	/** Load sounds **/
	void load();

	/** Close the sound manager and reset pointers **/
	void close();

	/** play a specific sound in a loop */
	void play(int index);

	/** request to end the playing loop */
	void endLoop();

	/** stop any currently playing sound */
	void stop();

	/** update on game frame */
	void update();

private:
	Resources::Sound *_currentSound;
	Resources::Sound *_sounds[3];
	bool _isLopping;
};

/**
 * The setting menu of the game
 */
class SettingsMenuScreen : public StaticLocationScreen {
public:
	SettingsMenuScreen(Gfx::Driver *gfx, Cursor *cursor);
	virtual ~SettingsMenuScreen();

	// StaticLocationScreen API
	void open() override;
	void close() override;
	void onGameLoop() override;

	void handleMouseUp();

private:
	enum HelpTextIndex {
		kHighRes = 5,
		kSubtitles = 7,
		kSpecialFX = 9,
		kShadows = 11,
		kHighResFMV = 13,
		kVoice = 16,
		kMusic = 18,
		kSfx = 20,
		kAllowFF = 22
	};

	enum WidgetIndex {
		kWidgetVoice = 15,
		kWidgetMusic = 17,
		kWidgetSfx = 19
	};

	template<HelpTextIndex N>
	void textHandler(StaticLocationWidget &widget, const Common::Point &mousePos);

	template<Settings::BoolSettingIndex N>
	void flipSettingHandler();

	void backHandler();

private:
	static const Color _textColorHovered;
	static const Color _textColorDefault;

	TestSoundManager _soundManager;
};

/**
 * Widget with a checkbox
 */
class CheckboxWidget : public StaticLocationWidget {
public:
	CheckboxWidget(const char *renderEntryName, bool isChecked,
				   WidgetOnClickCallback *onClickCallback,
	               WidgetOnMouseMoveCallback *onMouseMoveCallback);
	virtual ~CheckboxWidget() {};

	// StaticLocationWidget API
	void render() override;
	bool isMouseInside(const Common::Point &mousePos) const override;
	void onClick() override;

private:
	VisualImageXMG *_currentImage;
	VisualImageXMG *_checkBoxImage[2];
	Common::Point _position;
	int _checkboxWidth, _checkboxHeight;
	bool _isChecked;

	bool isMouseInsideCheckbox(const Common::Point &mousePos) const;
};

/**
 * Widget with a dragged slider for twisting the volume
 */
class VolumeWidget : public StaticLocationWidget {
public:
	VolumeWidget(const char *renderEntryName, Cursor *cursor,
				 TestSoundManager &soundManager, int soundIndex, 
				 Settings::IntSettingIndex settingIndex,
				 WidgetOnMouseMoveCallback *onMouseMoveCallback);
	virtual ~VolumeWidget() {};

	// StaticLocationWidget API
	void render() override;
	bool isMouseInside(const Common::Point &mousePos) const override;
	void onClick() override;
	void onMouseMove(const Common::Point &mousePos) override;
	void onMouseUp() override;

private:
	static const Color _textColorBgHovered;
	static const int _maxVolume = 256;

	VisualImageXMG *_sliderImage;
	VisualImageXMG *_bgImage;

	Cursor *_cursor;

	TestSoundManager &_soundManager;
	const int _soundIndex;

	Common::Point _sliderPosition, _bgPosition;
	int _bgWidth, _bgHeight, _sliderWidth, _minX, _maxX;
	
	bool _isDragged;
	const Settings::IntSettingIndex _settingIndex;

	bool isMouseInsideBg(const Common::Point &mousePos) const;

	int volumeToX(int volume) {
		return volume * (_maxX - _minX) / _maxVolume + _minX;
	}

	int xToVolume(int x) {
		return (x - _minX) * _maxVolume / (_maxX - _minX);	
	}
};

} // End of namespace Stark

#endif // STARK_UI_MENU_SETTING_MENU_H
