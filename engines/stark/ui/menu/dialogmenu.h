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
class DialogLineText;

/**
 * The conversation log menu
 */
class DialogScreen : public StaticLocationScreen {
public:
	DialogScreen(Gfx::Driver *gfx, Cursor *cursor);
	virtual ~DialogScreen();

	// StaticLocationScreen API
	void open() override;
	void close() override;
	void onScreenChanged() override;

	void onDialogClick(uint logIndex);

protected:
	// Window API
	void onRender() override;

private:
	static const uint _dialogTitleWidgetOffset = 8;

	enum WidgetIndex {
		kWidgetIndexBack = 3,
		kWidgetIndexNext = 4,
		kWidgetLogBack = 5,
		kWidgetIndex = 6,
		kWidgetLogNext = 7
	};

	Gfx::RenderEntry *_indexFrame, *_logFrame;
	uint _startTitleIndex, _nextTitleIndex, _startLineIndex, _nextLineIndex;
	uint _curMaxChapter, _curLogIndex;
	Common::Array<ChapterTitleText *> _chapterTitleTexts;
	Common::Array<uint> _prevTitleIndexStack;
	Common::Array<DialogLineText *> _dialogLineTexts;
	Common::Array<uint> _prevLineIndexStack;

	void loadIndex();
	void loadDialog();

	void backHandler();
	void indexBackHandler();
	void indexNextHandler();
	void logBackHandler();
	void backIndexHandler();
	void logNextHandler();

	void freeLogTitleWidgets();
	void freeChapterTitleTexts();
	void freeDialogLineTexts();
	void freeResources();
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
	static const Color _color;

	Common::Point _pos;
	VisualText _text;
};

/**
 * The dialog text displayed
 */
class DialogLineText {
public:
	DialogLineText(Gfx::Driver *gfx, uint logIndex, uint lineIndex, uint boxWidth);
	~DialogLineText() {}

	void setPosition(const Common::Point &pos);
	uint getHeight() { return _nameHeight + _lineHeight + 4; }

	void render() {
		_nameText.render(_namePos);
		_lineText.render(_linePos);
	}

	void onScreenChanged() {
		_nameText.resetTexture();
		_lineText.resetTexture();
	}

private:
	static const Color _textColorApril;
	static const Color _textColorNormal;

	Common::Point _namePos, _linePos;
	VisualText _nameText, _lineText;

	uint _nameWidth, _nameHeight, _lineHeight, _boxWidth;
};

/**
 * The dialog widget
 */
class DialogTitleWidget : public StaticLocationWidget {
public:
	DialogTitleWidget(DialogScreen *screen, Gfx::Driver *gfx, uint logIndex);
	virtual ~DialogTitleWidget() {}

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
	static const Color _textColorHovered;
	static const Color _textColorDefault;

	uint _logIndex, _chapter;
	int _width, _height;
	Common::Point _pos;
	VisualText _text;

	DialogScreen *_screen;
};

} // End of namespace Stark

#endif // STARK_UI_MENU_DIALOG_H
