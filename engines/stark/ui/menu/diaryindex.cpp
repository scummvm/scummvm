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

#include "engines/stark/ui/menu/diaryindex.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/diary.h"
#include "engines/stark/services/userinterface.h"
#include "engines/stark/services/gamemessage.h"

#include "engines/stark/ui/cursor.h"

#include "engines/engine.h"
#include "gui/message.h"
#include "gui/saveload.h"

namespace Stark {

const Color DiaryIndexScreen::_textColorHovered = Color(0x1E, 0x1E, 0x96);
const Color DiaryIndexScreen::_textColorDefault = Color(0x00, 0x00, 0x00);

DiaryIndexScreen::DiaryIndexScreen(Gfx::Driver *gfx, Cursor *cursor) :
		StaticLocationScreen(gfx, cursor, "DiaryIndexLocation", Screen::kScreenDiaryIndex) {
}

DiaryIndexScreen::~DiaryIndexScreen() {
}

void DiaryIndexScreen::open() {
	StaticLocationScreen::open();

	_widgets.push_back(new StaticLocationWidget(
			"BGImage",
			nullptr,
			nullptr));

	_widgets.push_back(new StaticLocationWidget(
			"SaveGame",
			CLICK_HANDLER(DiaryIndexScreen, saveHandler),
			MOVE_HANDLER(DiaryIndexScreen, widgetTextColorHandler)));
	_widgets.back()->setVisible(g_engine->canSaveGameStateCurrently());

	_widgets.push_back(new StaticLocationWidget(
			"Continue",
			CLICK_HANDLER(DiaryIndexScreen, loadHandler),
			MOVE_HANDLER(DiaryIndexScreen, widgetTextColorHandler)));

	_widgets.push_back(new StaticLocationWidget(
			"Options",
			CLICK_HANDLER(DiaryIndexScreen, settingsHandler),
			MOVE_HANDLER(DiaryIndexScreen, widgetTextColorHandler)));

	_widgets.push_back(new StaticLocationWidget(
			"Log",
			CLICK_HANDLER(DiaryIndexScreen, dialogHandler),
			MOVE_HANDLER(DiaryIndexScreen, widgetTextColorHandler)));

	_widgets.push_back(new StaticLocationWidget(
			"Fmv",
			CLICK_HANDLER(DiaryIndexScreen, fmvHandler),
			MOVE_HANDLER(DiaryIndexScreen, widgetTextColorHandler)));

	_widgets.push_back(new StaticLocationWidget(
			"Diary",
			CLICK_HANDLER(DiaryIndexScreen, diaryHandler),
			MOVE_HANDLER(DiaryIndexScreen, widgetTextColorHandler)));
	_widgets.back()->setVisible(StarkDiary->isEnabled());

	_widgets.push_back(new StaticLocationWidget(
			"Return",
			CLICK_HANDLER(DiaryIndexScreen, backHandler),
			nullptr));

	_widgets.push_back(new StaticLocationWidget(
			"Quit",
			CLICK_HANDLER(DiaryIndexScreen, quitHandler),
			MOVE_HANDLER(DiaryIndexScreen, widgetTextColorHandler)));

	_widgets.push_back(new StaticLocationWidget(
			"Back",
			CLICK_HANDLER(DiaryIndexScreen, backHandler),
			nullptr));

	for (uint i = 1; i < _widgets.size(); i++) {
		// The background image is intentionally ignored
		_widgets[i]->setupSounds(0, 1);
	}
}

void DiaryIndexScreen::widgetTextColorHandler(StaticLocationWidget &widget, const Common::Point &mousePos) {
	if (widget.isVisible()) {
		Color textColor = widget.isMouseInside(mousePos) ? _textColorHovered : _textColorDefault;
		widget.setTextColor(textColor);
	}
}

void DiaryIndexScreen::settingsHandler() {
	StarkUserInterface->changeScreen(Screen::kScreenSettingsMenu);
}

void DiaryIndexScreen::fmvHandler() {
	StarkUserInterface->changeScreen(Screen::kScreenFMVMenu);
}

void DiaryIndexScreen::backHandler() {
	StarkUserInterface->backPrevScreen();
}

void DiaryIndexScreen::quitHandler() {
	StarkUserInterface->confirm(GameMessage::kQuitGamePrompt, StarkUserInterface,
	                            &UserInterface::requestQuitToMainMenu);
}

void DiaryIndexScreen::loadHandler() {
	StarkUserInterface->changeScreen(Screen::kScreenLoadMenu);
}

void DiaryIndexScreen::saveHandler() {
	StarkUserInterface->changeScreen(Screen::kScreenSaveMenu);
}

void DiaryIndexScreen::diaryHandler() {
	StarkUserInterface->changeScreen(Screen::kScreenDiaryPages);
}

void DiaryIndexScreen::dialogHandler() {
	StarkUserInterface->changeScreen(Screen::kScreenDialog);
}

} // End of namespace Stark
