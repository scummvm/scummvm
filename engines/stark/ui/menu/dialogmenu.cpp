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
#include "engines/stark/services/staticprovider.h"

#include "engines/stark/gfx/renderentry.h"

#include "engines/stark/resources/location.h"

namespace Stark {

DialogScreen::DialogScreen(Gfx::Driver *gfx, Cursor *cursor) :
		StaticLocationScreen(gfx, cursor, "DiaryLog", Screen::kScreenDialog),
		_startTitleIndex(0), _nextTitleIndex(0),
		_startLineIndex(0), _nextLineIndex(0),
		_curMaxChapter(0),
		_indexFrame(nullptr),
		_logFrame(nullptr),
		_chapterTitleTexts(),
		_prevTitleIndexStack() {
}

DialogScreen::~DialogScreen() {
	freeChapterTitleTexts();
	_prevTitleIndexStack.clear();
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
			CLICK_HANDLER(DialogScreen, backIndexHandler),
			nullptr));
			
	_widgets.push_back(new StaticLocationWidget(
			"IndexNext",
			CLICK_HANDLER(DialogScreen, nextIndexHandler),
			nullptr));
			
	_widgets.push_back(new StaticLocationWidget(
			"LogBack",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);
			
	_widgets.push_back(new StaticLocationWidget(
			"Index",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);
			
	_widgets.push_back(new StaticLocationWidget(
			"LogNext",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);
	
	for (uint i = 1; i < _widgets.size(); ++i) {
		_widgets[i]->setupSounds(0, 1);
	}

	Resources::Location *location = StarkStaticProvider->getLocation();
	_indexFrame = location->getRenderEntryByName("IndexFrame");
	_logFrame = location->getRenderEntryByName("LogFrame");

	_nextTitleIndex = 0;
	loadIndex();
}

void DialogScreen::close() {
	freeChapterTitleTexts();
	_prevTitleIndexStack.clear();
	StaticLocationScreen::close();
}

void DialogScreen::onScreenChanged() {
	StaticLocationScreen::onScreenChanged();
	for (uint i = 0; i < _chapterTitleTexts.size(); ++i) {
		_chapterTitleTexts[i]->onScreenChanged();
	}
}

void DialogScreen::onRender() {
	StaticLocationScreen::onRender();
	for (uint i = 0; i < _chapterTitleTexts.size(); ++i) {
		_chapterTitleTexts[i]->render();
	}
}

void DialogScreen::loadIndex() {
	freeDialogWidgets();
	freeChapterTitleTexts();

	_widgets[kWidgetIndexBack]->setVisible(_nextTitleIndex > 0);
	_startTitleIndex = _nextTitleIndex;

	Common::Point pos = _indexFrame->getPosition();
	uint bottom = _indexFrame->getText()->getTargetHeight() + pos.y;
	uint height;
	ChapterTitleText *chapterTitleText;
	DialogWidget *dialogWidget;

	_curMaxChapter = 99;
	while (_nextTitleIndex < StarkDiary->countDialog()) {
		height = 0;
		chapterTitleText = nullptr;

		dialogWidget = new DialogWidget(_gfx, _nextTitleIndex);
		height += dialogWidget->getHeight();

		if (dialogWidget->getChapter() != _curMaxChapter) {
			_curMaxChapter = dialogWidget->getChapter();
			chapterTitleText = new ChapterTitleText(_gfx, _curMaxChapter);
			height += chapterTitleText->getHeight() + 8;
		}

		if (pos.y + height > bottom) {
			break;
		}

		if (chapterTitleText) {
			pos.y += 4;
			chapterTitleText->setPosition(pos);
			pos.y += chapterTitleText->getHeight() + 4;
		}
		dialogWidget->setPosition(pos);
		pos.y += dialogWidget->getHeight() + 4;

		_widgets.push_back(dialogWidget);
		if (chapterTitleText) {
			_chapterTitleTexts.push_back(chapterTitleText);
		}

		++_nextTitleIndex;
	}

	_widgets[kWidgetIndexNext]->setVisible(_nextTitleIndex < StarkDiary->countDialog());
}

void DialogScreen::backHandler() {
	StarkUserInterface->backPrevScreen();
}

void DialogScreen::nextIndexHandler() {
	_prevTitleIndexStack.push_back(_startTitleIndex);
	loadIndex();
}

void DialogScreen::backIndexHandler() {
	_nextTitleIndex = _prevTitleIndexStack.back();
	_prevTitleIndexStack.pop_back();
	loadIndex();
}

void DialogScreen::freeDialogWidgets() {
	uint size = _widgets.size();
	for (uint i = 0; i < size - _dialogWidgetOffset; ++i) {
		delete _widgets.back();
		_widgets.pop_back();
	}
}

void DialogScreen::freeChapterTitleTexts() {
	for (uint i = 0; i < _chapterTitleTexts.size(); ++i) {
		delete _chapterTitleTexts[i];
	}
	_chapterTitleTexts.clear();
}

ChapterTitleText::ChapterTitleText(Gfx::Driver *gfx, uint chapter) :
		_pos(), _text(gfx) {
	Common::String text = Common::String::format(
			"%s: %s",
			StarkGameChapter->getChapterTitle(chapter).c_str(),
			StarkGameChapter->getChapterSubtitle(chapter).c_str());
	text.toUppercase();

	_text.setText(text);
	_text.setColor(_color);
	_text.setFont(FontProvider::kCustomFont, 5);
}

DialogWidget::DialogWidget(Gfx::Driver *gfx, uint logIndex) :
		StaticLocationWidget(nullptr, nullptr, nullptr),
		_logIndex(logIndex), _pos(), _text(gfx) {
	const Diary::ConversationLog &dialog = StarkDiary->getDialog(_logIndex);

	_chapter = dialog.chapter;

	_text.setText(dialog.title);
	_text.setColor(_textColorDefault);
	_text.setFont(FontProvider::kCustomFont, 3);

	Common::Rect rect = _text.getRect();
	_width = rect.right - rect.left;
	_height = rect.bottom - rect.top;
}

bool DialogWidget::isMouseInside(const Common::Point &mousePos) const {
	return mousePos.x >= _pos.x && mousePos.x <= _pos.x + _width &&
	       mousePos.y >= _pos.y && mousePos.y <= _pos.y + _height;
}

void DialogWidget::onClick() {
	debug("Click %d", _logIndex);
}

void DialogWidget::onScreenChanged() {
	_text.resetTexture();
}

} // End of namespace Stark
