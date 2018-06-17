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
#include "engines/stark/services/global.h"

#include "engines/stark/gfx/renderentry.h"

#include "engines/stark/resources/location.h"

namespace Stark {

DialogScreen::DialogScreen(Gfx::Driver *gfx, Cursor *cursor) :
		StaticLocationScreen(gfx, cursor, "DiaryLog", Screen::kScreenDialog),
		_startTitleIndex(0), _nextTitleIndex(0),
		_startLineIndex(0), _nextLineIndex(0),
		_curMaxChapter(0), _curLogIndex(0),
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
			CLICK_HANDLER(DialogScreen, indexBackHandler),
			nullptr));
			
	_widgets.push_back(new StaticLocationWidget(
			"IndexNext",
			CLICK_HANDLER(DialogScreen, indexNextHandler),
			nullptr));
			
	_widgets.push_back(new StaticLocationWidget(
			"LogBack",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);
			
	_widgets.push_back(new StaticLocationWidget(
			"Index",
			CLICK_HANDLER(DialogScreen, backIndexHandler),
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
	freeDialogLineTexts();
	_prevTitleIndexStack.clear();
	StaticLocationScreen::close();
}

void DialogScreen::onScreenChanged() {
	StaticLocationScreen::onScreenChanged();
	for (uint i = 0; i < _chapterTitleTexts.size(); ++i) {
		_chapterTitleTexts[i]->onScreenChanged();
	}
	for (uint i = 0; i < _dialogLineTexts.size(); ++i) {
		_dialogLineTexts[i]->onScreenChanged();
	}
}

void DialogScreen::onDialogClick(uint logIndex) {
	freeLogTitleWidgets();
	freeChapterTitleTexts();

	_widgets[kWidgetIndexBack]->setVisible(false);
	_widgets[kWidgetIndexNext]->setVisible(false);
	_widgets[kWidgetIndex]->setVisible(true);

	_nextLineIndex = 0;
	_curLogIndex = logIndex;
	loadDialog();
}

void DialogScreen::onRender() {
	StaticLocationScreen::onRender();
	for (uint i = 0; i < _chapterTitleTexts.size(); ++i) {
		_chapterTitleTexts[i]->render();
	}
	for (uint i = 0; i < _dialogLineTexts.size(); ++i) {
		_dialogLineTexts[i]->render();
	}
}

void DialogScreen::loadIndex() {
	freeLogTitleWidgets();
	freeChapterTitleTexts();

	_startTitleIndex = _nextTitleIndex;

	Common::Point pos = _indexFrame->getPosition();
	uint bottom = _indexFrame->getText()->getTargetHeight() + pos.y;
	uint height;
	ChapterTitleText *chapterTitleText;
	DialogTitleWidget *dialogTitleWidget;

	_curMaxChapter = 99;
	while (_nextTitleIndex < StarkDiary->countDialog()) {
		height = 0;
		chapterTitleText = nullptr;

		dialogTitleWidget = new DialogTitleWidget(this, _gfx, _nextTitleIndex);
		height += dialogTitleWidget->getHeight();

		if (dialogTitleWidget->getChapter() != _curMaxChapter) {
			_curMaxChapter = dialogTitleWidget->getChapter();
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
		dialogTitleWidget->setPosition(pos);
		pos.y += dialogTitleWidget->getHeight() + 4;

		_widgets.push_back(dialogTitleWidget);
		if (chapterTitleText) {
			_chapterTitleTexts.push_back(chapterTitleText);
		}

		if (pos.y > bottom) {
			break;
		}

		++_nextTitleIndex;
	}

	_widgets[kWidgetIndexBack]->setVisible(_startTitleIndex > 0);
	_widgets[kWidgetIndexNext]->setVisible(_nextTitleIndex < StarkDiary->countDialog());
}

void DialogScreen::loadDialog() {
	freeDialogLineTexts();

	Common::Point pos = _logFrame->getPosition();
	uint boxWidth = _logFrame->getText()->getTargetWidth();

	_dialogLineTexts.push_back(new DialogLineText(_gfx, _curLogIndex, _nextLineIndex));
	_dialogLineTexts.back()->setPosition(pos, boxWidth);
}

void DialogScreen::backHandler() {
	StarkUserInterface->backPrevScreen();
}

void DialogScreen::indexNextHandler() {
	_prevTitleIndexStack.push_back(_startTitleIndex);
	loadIndex();
}

void DialogScreen::indexBackHandler() {
	_nextTitleIndex = _prevTitleIndexStack.back();
	_prevTitleIndexStack.pop_back();
	loadIndex();
}

void DialogScreen::backIndexHandler() {
	freeDialogLineTexts();

	_widgets[kWidgetLogBack]->setVisible(false);
	_widgets[kWidgetLogNext]->setVisible(false);
	_widgets[kWidgetIndex]->setVisible(false);

	_nextTitleIndex = _startTitleIndex;
	loadIndex();
}

void DialogScreen::freeLogTitleWidgets() {
	uint size = _widgets.size();
	for (uint i = 0; i < size - _dialogTitleWidgetOffset; ++i) {
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

void DialogScreen::freeDialogLineTexts() {
	for (uint i = 0; i < _dialogLineTexts.size(); ++i) {
		delete _dialogLineTexts[i];
	}
	_dialogLineTexts.clear();
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

DialogLineText::DialogLineText(Gfx::Driver *gfx, uint logIndex, uint lineIndex) :
		_namePos(), _linePos(),
		_nameText(gfx), _lineText(gfx) {
	Diary::ConversationLogLine logLine = StarkDiary->getDialog(logIndex).lines[lineIndex];

	Common::String name = StarkGlobal->getCharacterName(logLine.characterId);
	name.toUppercase();

	uint color = name == "APRIL" ? _textColorApril : _textColorNormal;

	_nameText.setText(name);
	_nameText.setColor(color);
	_nameText.setFont(FontProvider::kCustomFont, 5);

	_lineText.setText(logLine.line);
	_lineText.setColor(color);
	_lineText.setFont(FontProvider::kCustomFont, 3);

	Common::Rect rect = _nameText.getRect();
	_nameWidth = rect.right - rect.left;
	_nameHeight = rect.bottom - rect.top;

	rect = _lineText.getRect();
	_lineHeight = rect.bottom - rect.top;
}

void DialogLineText::setPosition(const Common::Point &pos, uint boxWidth) {
	_namePos.x = pos.x + (boxWidth - _nameWidth) / 2;
	_namePos.y = pos.y;

	_linePos.x = pos.x;
	_linePos.y = pos.y + _nameHeight + 4;
}

DialogTitleWidget::DialogTitleWidget(DialogScreen *screen, Gfx::Driver *gfx, uint logIndex) :
		StaticLocationWidget(nullptr, nullptr, nullptr),
		_logIndex(logIndex), _pos(), _text(gfx),
		_screen(screen) {
	const Diary::ConversationLog &dialog = StarkDiary->getDialog(_logIndex);

	_chapter = dialog.chapter;

	_text.setText(dialog.title);
	_text.setColor(_textColorDefault);
	_text.setFont(FontProvider::kCustomFont, 3);

	Common::Rect rect = _text.getRect();
	_width = rect.right - rect.left;
	_height = rect.bottom - rect.top;
}

bool DialogTitleWidget::isMouseInside(const Common::Point &mousePos) const {
	return mousePos.x >= _pos.x && mousePos.x <= _pos.x + _width &&
	       mousePos.y >= _pos.y && mousePos.y <= _pos.y + _height;
}

void DialogTitleWidget::onClick() {
	_screen->onDialogClick(_logIndex);
}

void DialogTitleWidget::onScreenChanged() {
	_text.resetTexture();
}

} // End of namespace Stark
