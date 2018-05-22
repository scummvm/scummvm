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

#ifndef STARK_UI_MENU_SETTING_MENU_H
#define STARK_UI_MENU_SETTING_MENU_H

#include "engines/stark/ui/menu/locationscreen.h"
#include "engines/stark/services/services.h"
#include "engines/advancedDetector.h"

namespace Stark {

class VisualImageXMG;

/**
 * The setting menu of the game
 */
class SettingsMenuScreen : public StaticLocationScreen {
public:
	SettingsMenuScreen(Gfx::Driver *gfx, Cursor *cursor);
	virtual ~SettingsMenuScreen();

	// StaticLocationScreen API
	void open() override;

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

	template<HelpTextIndex N>
	void textHandler(StaticLocationWidget &widget, const Common::Point &mousePos);

	void backHandler();

private:
	static const uint32 _textColorHovered = 0xFF961E1E;
	static const uint32 _textColorDefault = 0xFF000000;

	bool isDemo() {
		return StarkGameDescription->flags & ADGF_DEMO;
	}
};

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
	int _width, _height;
	bool _isChecked;

	bool isMouseInsideCheckbox(const Common::Point &mousePos) const;
};

/*
class VolumeWidget : public StaticLocationWidget {
public:
	VolumeWidget(const char *renderEntryName,
				 WidgetOnClickCallback *onClickCallback,
	             WidgetOnMouseMoveCallback *onMouseMoveCallback);
	virtual ~VolumeWidget();

	// StaticLocationWidget API
	void render() override;
	bool isMouseInside(const Common::Point &mousePos) const override;
	void onClick() override;
	void onMouseMove(const Common::Point &mousePos) override;
};
*/

} // End of namespace Stark

#endif // STARK_UI_MENU_SETTING_MENU_H
