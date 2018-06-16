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

#ifndef STARK_UI_MENU_DIALOG_H
#define STARK_UI_MENU_DIALOG_H

#include "engines/stark/ui/menu/locationscreen.h"

#include "engines/stark/visual/text.h"

namespace Stark {

class ChapterTitleText;

/**
 * The coversation log menu
 */
class DialogScreen : public StaticLocationScreen {
public:
	DialogScreen(Gfx::Driver *gfx, Cursor *cursor);
	virtual ~DialogScreen();

	// StaticLocationScreen API
	void open() override;
	void close() override;
	void onScreenChanged() override;

protected:
	// Window API
	void onRender() override;

private:
	static const uint _dialogWidgetOffset = 8;

	enum WidgetIndex {
		kWidgetIndexBack = 3,
		kWidgetIndexNext = 4
	};

	Gfx::RenderEntry *_indexFrame, *_logFrame;
	Common::Array<ChapterTitleText *> _chapterTitleTexts;
	uint _startTitleIndex, _nextTitleIndex, _startLineIndex, _nextLineIndex;
	uint _curMaxChapter;
	Common::Array<uint> _prevTitleIndexStack;

	void loadIndex();

	void backHandler();
	void nextIndexHandler();
	void backIndexHandler();

	void freeDialogWidgets();
	void freeChapterTitleTexts();
};

/**
 * The chapter title displayed on the menu
 */
class ChapterTitleText {
public:
	ChapterTitleText(Gfx::Driver *gfx, uint chapter);
	~ChapterTitleText() {}

	void setPosition(const Common::Point &pos) { _pos = pos; }
	uint getHeight() { return _text.getRect().bottom - _text.getRect().top; }

	void render() { _text.render(_pos); }
	void onScreenChanged() { _text.resetTexture(); }

private:
	static const uint32 _color = 0xFF040568;

	Common::Point _pos;
	VisualText _text;
};

/**
 * The dialog widget
 */
class DialogWidget : public StaticLocationWidget {
public:
	DialogWidget(Gfx::Driver *gfx, uint logIndex);
	virtual ~DialogWidget() {}

	void setPosition(const Common::Point &pos) { _pos = pos; }
	uint getHeight() { return _height; }
	uint getChapter() { return _chapter; }

	// StaticLocationWidget API
	void render() override { _text.render(_pos); }
	bool isMouseInside(const Common::Point &mousePos) const override;
	void onClick() override;
	void onMouseMove(const Common::Point &mousePos) override {
		_text.setColor(isMouseInside(mousePos) ? _textColorHovered : _textColorDefault);
	}
	void onScreenChanged() override;

private:
	static const uint32 _textColorHovered = 0xFF961E1E;
	static const uint32 _textColorDefault = 0xFF000000;

	uint _logIndex, _chapter, _width, _height;
	Common::Point _pos;
	VisualText _text;
};

} // End of namespace Stark

#endif // STARK_UI_MENU_DIALOG_H
