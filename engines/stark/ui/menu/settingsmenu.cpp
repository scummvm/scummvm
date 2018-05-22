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

#include "engines/stark/ui/menu/settingsmenu.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/userinterface.h"

namespace Stark {

SettingsMenuScreen::SettingsMenuScreen(Gfx::Driver *gfx, Cursor *cursor):
		StaticLocationScreen(gfx, cursor, "OptionLocation", Screen::kScreenSettingsMenu) {
}

SettingsMenuScreen::~SettingsMenuScreen() {
}

void SettingsMenuScreen::open() {
	StaticLocationScreen::open();

	_widgets.push_back(new StaticLocationWidget(
			// This is the background image
			"The Longest Journey",
			nullptr,
			nullptr));
	
	_widgets.push_back(new StaticLocationWidget(
			"Return",
			CLICK_HANDLER(SettingsMenuScreen, backHandler),
			nullptr));
	_widgets.back()->setupSounds(3, 4);
	
	_widgets.push_back(new StaticLocationWidget(
			"Back",
			CLICK_HANDLER(SettingsMenuScreen, backHandler),
			nullptr));
	_widgets.back()->setupSounds(3, 4);
	
	_widgets.push_back(new StaticLocationWidget(
			"GSettings",
			nullptr,
			nullptr));
	
	_widgets.push_back(new StaticLocationWidget(
			"AprilHighRes",
			nullptr,
			MOVE_HANDLER(SettingsMenuScreen, textHandler<kHighRes>)));
	_widgets.back()->setupSounds(3, 4);
	
	_widgets.push_back(new StaticLocationWidget(
			"HighResHelp",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);
	
	_widgets.push_back(new StaticLocationWidget(
			"Subtitles",
			nullptr,
			MOVE_HANDLER(SettingsMenuScreen, textHandler<kSubtitles>)));
	_widgets.back()->setupSounds(3, 4);
	
	_widgets.push_back(new StaticLocationWidget(
			"SubtitleHelp",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);

	_widgets.push_back(new StaticLocationWidget(
			"SpecialFX",
			nullptr,
			MOVE_HANDLER(SettingsMenuScreen, textHandler<kSpecialFX>)));
	_widgets.back()->setupSounds(3, 4);
	
	_widgets.push_back(new StaticLocationWidget(
			"SpecialFXHelp",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);

	_widgets.push_back(new StaticLocationWidget(
			"Shadows",
			nullptr,
			MOVE_HANDLER(SettingsMenuScreen, textHandler<kShadows>)));
	_widgets.back()->setupSounds(3, 4);
	
	_widgets.push_back(new StaticLocationWidget(
			"ShadowsHelp",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);

	_widgets.push_back(new StaticLocationWidget(
			"HighResFMV",
			nullptr,
			MOVE_HANDLER(SettingsMenuScreen, textHandler<kHighResFMV>)));
	_widgets.back()->setupSounds(3, 4);
	
	_widgets.push_back(new StaticLocationWidget(
			"FMVHelp",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);

	_widgets.push_back(new StaticLocationWidget(
			"VSettings",
			nullptr,
			nullptr));
	
	_widgets.push_back(new StaticLocationWidget(
			"Voice",
			nullptr,
			MOVE_HANDLER(SettingsMenuScreen, textHandler<kVoice>)));
	
	_widgets.push_back(new StaticLocationWidget(
			"VoiceHelp",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);

	_widgets.push_back(new StaticLocationWidget(
			"Music",
			nullptr,
			MOVE_HANDLER(SettingsMenuScreen, textHandler<kMusic>)));
	
	_widgets.push_back(new StaticLocationWidget(
			"MusicHelp",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);

	_widgets.push_back(new StaticLocationWidget(
			"Sfx",
			nullptr,
			MOVE_HANDLER(SettingsMenuScreen, textHandler<kSfx>)));
	
	_widgets.push_back(new StaticLocationWidget(
			"SfxHelp",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);

	if (!isDemo()) {
		_widgets.push_back(new StaticLocationWidget(
				"AllowFF",
				nullptr,
				MOVE_HANDLER(SettingsMenuScreen, textHandler<kAllowFF>)));
		_widgets.back()->setupSounds(3, 4);
		
		_widgets.push_back(new StaticLocationWidget(
				"AllowFFHelp",
				nullptr,
				nullptr));
		_widgets.back()->setVisible(false);
	}
}

template<SettingsMenuScreen::HelpTextIndex N>
void SettingsMenuScreen::textHandler(StaticLocationWidget &widget, const Common::Point &mousePos) {
	if (widget.isVisible()) {
		if (widget.isMouseInside(mousePos)) {
			widget.setTextColor(_textColorHovered);
			_widgets[N]->setVisible(true);
		} else {
			widget.setTextColor(_textColorDefault);
			_widgets[N]->setVisible(false);
		}
	}
}

void SettingsMenuScreen::backHandler() {
	StarkUserInterface->backPrevScreen();
}

} // End of namespace Stark
