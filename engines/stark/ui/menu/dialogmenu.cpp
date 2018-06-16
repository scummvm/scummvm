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

#include "engines/stark/ui/menu/dialogmenu.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/userinterface.h"
#include "engines/stark/services/diary.h"
#include "engines/stark/services/gamechapter.h"

#include "engines/stark/gfx/renderentry.h"

namespace Stark {

DialogScreen::DialogScreen(Gfx::Driver *gfx, Cursor *cursor) :
		StaticLocationScreen(gfx, cursor, "DiaryLog", Screen::kScreenDialog), temp(nullptr) {
}

DialogScreen::~DialogScreen() {
}

void DialogScreen::open() {
	StaticLocationScreen::open();

	_widgets.push_back(new StaticLocationWidget(
			"BGImage",
			nullptr,
			nullptr));
			
	_widgets.push_back(new StaticLocationWidget(
			"Return",
			CLICK_HANDLER(DialogScreen, backHandler),
			nullptr));

	_widgets.push_back(new StaticLocationWidget(
			"Back",
			CLICK_HANDLER(DialogScreen, backHandler),
			nullptr));
			
	_widgets.push_back(new StaticLocationWidget(
			"IndexBack",
			nullptr,
			nullptr));
			
	_widgets.push_back(new StaticLocationWidget(
			"IndexNext",
			nullptr,
			nullptr));
			
	_widgets.push_back(new StaticLocationWidget(
			"LogBack",
			nullptr,
			nullptr));
			
	_widgets.push_back(new StaticLocationWidget(
			"Index",
			nullptr,
			nullptr));
			
	_widgets.push_back(new StaticLocationWidget(
			"LogNext",
			nullptr,
			nullptr));
	
	for (uint i = 1; i < _widgets.size(); ++i) {
		_widgets[i]->setupSounds(0, 1);
	}

	temp = new ChapterTitleText(_gfx, 0, Common::Point(200, 200));
	_widgets.push_back(new DialogWidget(0, Common::Point(200, 300)));
}

void DialogScreen::onScreenChanged() {
	StaticLocationScreen::onScreenChanged();
	temp->onScreenChanged();
}

void DialogScreen::onRender() {
	StaticLocationScreen::onRender();
	temp->render();
}

void DialogScreen::backHandler() {
	StarkUserInterface->backPrevScreen();
}

ChapterTitleText::ChapterTitleText(Gfx::Driver *gfx, uint chapter, const Common::Point &pos) :
		_pos(pos), _text(gfx) {
	Common::String text = Common::String::format(
			"%s: %s",
			StarkGameChapter->getChapterTitle(chapter).c_str(),
			StarkGameChapter->getChapterSubtitle(chapter).c_str());
	text.toUppercase();

	_text.setText(text);
	_text.setColor(_color);
	_text.setFont(FontProvider::kCustomFont, 5);
}

DialogWidget::DialogWidget(uint logIndex, const Common::Point &pos) :
		StaticLocationWidget("IndexFrame", nullptr, nullptr),
		_logIndex(logIndex) {
	const Diary::ConversationLog &dialog = StarkDiary->getDialog(_logIndex);
	VisualText *text = _renderEntry->getText();
	text->setText(dialog.title);
	_renderEntry->setPosition(pos);
}

void DialogWidget::onClick() {
	debug("Click %d", _logIndex);
}

} // End of namespace Stark
