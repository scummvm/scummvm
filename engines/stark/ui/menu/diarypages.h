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

#ifndef STARK_UI_MENU_DIARY_PAGES_H
#define STARK_UI_MENU_DIARY_PAGES_H

#include "engines/stark/ui/menu/locationscreen.h"

namespace Stark {

/**
 * The screen where diary pages are shown in the game
 */
class DiaryPagesScreen : public StaticLocationScreen {
public:
	DiaryPagesScreen(Gfx::Driver *gfx, Cursor *cursor);
	virtual ~DiaryPagesScreen();

	// StaticLocationScreen API
	void open() override;
	void close() override;

private:
	enum WidgetIndex {
		kWidgetBack = 3,
		kWidgetNext = 4
	};

	uint _page;

	void backHandler();

	void prevPageHandler() { changePage(_page - 1); }
	void nextPageHandler() { changePage(_page + 1); }
	void changePage(uint page);
};

/**
 * The widget displaying diary text
 */
class DiaryWidget : public StaticLocationWidget {
public:
	DiaryWidget(uint diaryIndex);
	~DiaryWidget() {}
};

} // End of namespace Stark

#endif // STARK_UI_MENU_DIARY_PAGES_H
