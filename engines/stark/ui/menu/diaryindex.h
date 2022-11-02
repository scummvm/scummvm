/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef STARK_UI_MENU_DIARY_INDEX_H
#define STARK_UI_MENU_DIARY_INDEX_H

#include "engines/stark/ui/menu/locationscreen.h"

namespace Stark {

/**
 * The diary index is the in-game menu
 */
class DiaryIndexScreen : public StaticLocationScreen {
public:
	DiaryIndexScreen(Gfx::Driver *gfx, Cursor *cursor);
	virtual ~DiaryIndexScreen();

	// StaticLocationScreen API
	void open() override;

private:
	void widgetTextColorHandler(StaticLocationWidget &widget, const Common::Point &mousePos);
	void backHandler();
	void settingsHandler();
	void fmvHandler();
	void loadHandler();
	void saveHandler();
	void diaryHandler();
	void dialogHandler();
	void quitHandler();

	const Color _textColorHovered = Color(0x1E, 0x1E, 0x96);
	const Color _textColorDefault = Color(0x00, 0x00, 0x00);
};

} // End of namespace Stark

 #endif // STARK_UI_MENU_DIARY_INDEX_H
