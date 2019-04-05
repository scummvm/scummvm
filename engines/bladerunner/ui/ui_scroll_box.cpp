/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#include "bladerunner/ui/ui_scroll_box.h"

#include "bladerunner/audio_player.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/font.h"
#include "bladerunner/game_info.h"
#include "bladerunner/shape.h"
#include "bladerunner/time.h"
#include "bladerunner/game_constants.h"
#include "bladerunner/ui/kia.h"
#include "bladerunner/ui/kia_shapes.h"

namespace BladeRunner {

const int UIScrollBox::k3DFrameColors[]        = { 0x1083, 0x14A5, 0x14A6, 0x2508, 0x5230, 0x5230, 0x0000, 0x0000 };
const int UIScrollBox::kTextBackgroundColors[] = { 0x14EA, 0x190C, 0x1D2E, 0x2570, 0x4F1F, 0x0000 };
const int UIScrollBox::kTextColors1[]          = { 0x25B3, 0x31F7, 0x3A5B, 0x46BF, 0x4F1F };
const int UIScrollBox::kTextColors2[]          = { 0x677F, 0x6F9F, 0x73BF, 0x77DF, 0x7FFF };
const int UIScrollBox::kTextColors3[]          = { 0x7BB8, 0x7BBA, 0x7BDB, 0x7FDD, 0x7FFF };
const int UIScrollBox::kTextColors4[]          = { 0x4DC7, 0x5E4B, 0x6EEE, 0x7751, 0x7F92 };

UIScrollBox::UIScrollBox(BladeRunnerEngine *vm, UIScrollBoxCallback *lineSelectedCallback, void *callbackData, int maxLineCount, int style, bool center, Common::Rect rect, Common::Rect scrollBarRect) : UIComponent(vm) {
	_selectedLineState     = 0;
	_scrollUpButtonState   = 0;
	_scrollDownButtonState = 0;
	_scrollAreaUpState     = 0;
	_scrollAreaDownState   = 0;
	_scrollBarState        = 0;

	_scrollUpButtonHover   = false;
	_scrollDownButtonHover = false;
	_scrollAreaUpHover     = false;
	_scrollAreaDownHover   = false;
	_scrollBarHover        = false;

	_hoveredLine          = -1;
	_selectedLineIndex    = -1;

	_lineSelectedCallback = lineSelectedCallback;
	_callbackData         = callbackData;

	_isVisible  = false;
	_style      = style;
	_center     = center;
	_timeLastScroll    = _vm->_time->currentSystem();
	_timeLastCheckbox  = _vm->_time->currentSystem();
	_timeLastHighlight = _vm->_time->currentSystem();

	_highlightFrame = 0;

	_rect          = rect;
	_scrollBarRect = scrollBarRect;
	_scrollBarRect.right += 15; // right side was not used, but it's useful for determining if the control is selected

	_lineCount    = 0;
	_maxLineCount = maxLineCount;

	_firstLineVisible = 0;
	_maxLinesVisible  = _rect.height() / kLineHeight;

	_mouseButton = false;

	_rect.bottom = _rect.top + kLineHeight * _maxLinesVisible - 1;

	_lines.resize(_maxLineCount);
	for (int i = 0; i < _maxLineCount; ++i) {
		_lines[i] = new Line();
		_lines[i]->lineData = -1;
		_lines[i]->flags = 0x00;
		_lines[i]->checkboxFrame = 5;
	}

	_mouseOver = false;
}

UIScrollBox::~UIScrollBox() {
	for (int i = 0; i < _maxLineCount; ++i) {
		delete _lines[i];
	}
}

void UIScrollBox::show() {
	_selectedLineState     = 0;
	_scrollUpButtonState   = 0;
	_scrollDownButtonState = 0;
	_scrollAreaUpState     = 0;
	_scrollAreaDownState   = 0;
	_scrollBarState        = 0;

	_hoveredLine       = -1;
	_selectedLineIndex = -1;

	_scrollUpButtonHover   = false;
	_scrollDownButtonHover = false;
	_scrollAreaUpHover     = false;
	_scrollAreaDownHover   = false;
	_scrollBarHover        = false;

	_timeLastScroll    = _vm->_time->currentSystem();
	_timeLastCheckbox  = _vm->_time->currentSystem();
	_timeLastHighlight = _vm->_time->currentSystem();

	_highlightFrame = 0;
	_isVisible = true;

	_mouseOver = false;
}

void UIScrollBox::hide() {
	_isVisible = false;
}

void UIScrollBox::clearLines(){
	_lineCount = 0;
	_firstLineVisible = 0;

}

void UIScrollBox::addLine(const Common::String &text, int lineData, int flags) {
	_lines[_lineCount]->text = text;
	_lines[_lineCount]->lineData = lineData;
	_lines[_lineCount]->flags = flags;

	++_lineCount;
}

void UIScrollBox::addLine(const char *text, int lineData, int flags) {
	_lines[_lineCount]->text = text;
	_lines[_lineCount]->lineData = lineData;
	_lines[_lineCount]->flags = flags;

	++_lineCount;
}

void UIScrollBox::sortLines() {
	qsort(_lines.data(), _lineCount, sizeof(Line *), &sortFunction);
}

void UIScrollBox::handleMouseMove(int mouseX, int mouseY) {
	if (!_isVisible) {
		return;
	}

	_mouseOver = _rect.contains(mouseX, mouseY) || _scrollBarRect.contains(mouseX, mouseY);

	if (_rect.contains(mouseX, mouseY)) {
		int newHoveredLine = (mouseY - _rect.top) / 10 + _firstLineVisible;
		if (newHoveredLine >= _lineCount) {
			newHoveredLine = -1;
		}

		if (newHoveredLine != _hoveredLine && newHoveredLine >= 0 && newHoveredLine < _lineCount) {
			if (_lines[newHoveredLine]->lineData >= 0 && _selectedLineState == 0) {
				int soundId = kSfxTEXT1;
				if (_lines[newHoveredLine]->flags & 0x01 ) {
					soundId = kSfxTEXT3;
				}
				_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(soundId), 100, 0, 0, 50, 0);
			}
		}
		_hoveredLine = newHoveredLine;
	} else {
		_hoveredLine = -1;
	}

	_scrollUpButtonHover =
		(mouseX >= _scrollBarRect.left)
		&& (mouseX < _scrollBarRect.left + 15)
		&& (mouseY >= _scrollBarRect.top)
		&& (mouseY < _scrollBarRect.top + 8);

	_scrollDownButtonHover =
		(mouseX >= _scrollBarRect.left)
		&& (mouseX < _scrollBarRect.left + 15)
		&& (mouseY > _scrollBarRect.bottom - 8)
		&& (mouseY <= _scrollBarRect.bottom);

	int scrollAreaHeight = _scrollBarRect.bottom - _scrollBarRect.top - 15;

	int scrollBarHeight = scrollAreaHeight;
	if (_lineCount > _maxLinesVisible) {
		scrollBarHeight = _maxLinesVisible * scrollAreaHeight / _lineCount;
	}
	if (scrollBarHeight < 16) {
		scrollBarHeight = 16;
	}

	int scrollAreaEmptySize = scrollAreaHeight - scrollBarHeight;

	int scrollBarY = 0;
	if (_lineCount > _maxLinesVisible) {
		scrollBarY = scrollAreaEmptySize * _firstLineVisible / (_lineCount - _maxLinesVisible);
	}

	if (_scrollBarState == 2) {
		int v12 = scrollBarHeight / 2 + 8;
		if (mouseY - _scrollBarRect.top > v12 && _lineCount > _maxLinesVisible && scrollAreaEmptySize > 0) {
			_firstLineVisible = (_lineCount - _maxLinesVisible) * (mouseY - _scrollBarRect.top - v12) / scrollAreaEmptySize;
			if (_firstLineVisible > _lineCount - _maxLinesVisible) {
				_firstLineVisible = _lineCount - _maxLinesVisible;
			}
		} else {
			_firstLineVisible = 0;
		}

		if (_lineCount <= _maxLinesVisible) {
			scrollBarY = 0;
		} else {
			scrollBarY = scrollAreaEmptySize * _firstLineVisible/ (_lineCount - _maxLinesVisible);
		}
	}
	scrollBarY = scrollBarY + _scrollBarRect.top + 8;

	_scrollBarHover =
		(mouseX >= _scrollBarRect.left)
		&& (mouseX < _scrollBarRect.left + 15)
		&& (mouseY >= scrollBarY)
		&& (mouseY < scrollBarY + scrollBarHeight);

	_scrollAreaUpHover =
		(mouseX >= _scrollBarRect.left)
		&& (mouseX < _scrollBarRect.left + 15)
		&& (mouseY >= _scrollBarRect.top + 8)
		&& (mouseY < scrollBarY);

	_scrollAreaDownHover =
		(mouseX >= _scrollBarRect.left)
		&& (mouseX < _scrollBarRect.left + 15)
		&& (mouseY >= scrollBarY + scrollBarHeight)
		&& (mouseY < _scrollBarRect.bottom - 8);
}

void UIScrollBox::handleMouseDown(bool alternateButton) {
	if (!_isVisible) {
		return;
	}

	_mouseButton = alternateButton;
	if (_hoveredLine == -1) {
		_selectedLineState = 1;
	} else if (_selectedLineIndex == -1) {
		_selectedLineIndex = _hoveredLine;
		_selectedLineState = 2;
		if (_hoveredLine < _lineCount) {
			if (_lineSelectedCallback) {
				_lineSelectedCallback(_callbackData, this, _lines[_selectedLineIndex]->lineData, _mouseButton);
			}

			if (_lines[_selectedLineIndex]->flags & 0x01) {
				_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxBEEP10), 100, 0, 0, 50, 0);
			}
		}
	}
	if (!alternateButton) {
		if (_scrollUpButtonHover) {
			_scrollUpButtonState = 2;
			_timeLastScroll = _vm->_time->currentSystem() - 160;
		} else {
			_scrollUpButtonState = 1;
		}
		if (_scrollDownButtonHover) {
			_scrollDownButtonState = 2;
		} else {
			_scrollDownButtonState = 1;
		}
		if (_scrollBarHover) {
			_scrollBarState = 2;
		} else {
			_scrollBarState = 1;
		}
		if (_scrollAreaUpHover) {
			_scrollAreaUpState = 2;
			_timeLastScroll = _vm->_time->currentSystem() - 160;
		} else {
			_scrollAreaUpState = 1;
		}
		if (_scrollAreaDownHover) {
			_scrollAreaDownState = 2;
			_timeLastScroll = _vm->_time->currentSystem() - 160;
		} else {
			_scrollAreaDownState = 1;
		}
	}
}

void UIScrollBox::handleMouseUp(bool alternateButton) {
	if (_isVisible) {
		if ( alternateButton == _mouseButton) {
			_selectedLineState = 0;
			_selectedLineIndex = -1;
		}

		if (!alternateButton) {
			_scrollUpButtonState = 0;
			_scrollDownButtonState = 0;
			_scrollAreaUpState = 0;
			_scrollAreaDownState = 0;
			_scrollBarState = 0;
		}
	}
}

void UIScrollBox::handleMouseScroll(int direction) {
	if (_mouseOver) {
		if (direction > 0) {
			scrollDown();
		} else if (direction < 0) {
			scrollUp();
		}
	}
}

int UIScrollBox::getSelectedLineData() {
	if (_hoveredLine >= 0 && _selectedLineState != 1 && _hoveredLine < _lineCount) {
		return _lines[_hoveredLine]->lineData;
	}
	return -1;
}

void UIScrollBox::draw(Graphics::Surface &surface) {
	int timeNow = _vm->_time->currentSystem();

	// update scrolling
	if (_scrollUpButtonState == 2 && _scrollUpButtonHover) {
		if ((timeNow - _timeLastScroll) > 160) {
			scrollUp();
			_timeLastScroll = timeNow;
		}
	} else if (_scrollDownButtonState == 2 && _scrollDownButtonHover) {
		if ((timeNow - _timeLastScroll) > 160) {
			scrollDown();
			_timeLastScroll = timeNow;
		}
	} else if (_scrollAreaUpState == 2 && _scrollAreaUpHover) {
		if ((timeNow - _timeLastScroll) > 160) {
			_firstLineVisible -= _maxLinesVisible - 1;
			_firstLineVisible = CLIP(_firstLineVisible, 0, _lineCount - _maxLinesVisible);
			_timeLastScroll = timeNow;
		}
	} else if (_scrollAreaDownState == 2 && _scrollAreaDownHover) {
		if ((timeNow - _timeLastScroll) > 160) {
			_firstLineVisible += _maxLinesVisible - 1;
			_firstLineVisible = CLIP(_firstLineVisible, 0, _lineCount - _maxLinesVisible);
			_timeLastScroll = timeNow;
		}
	}

	// update checkboxes
	int timeDiffCheckBox = timeNow - _timeLastCheckbox;
	if (timeDiffCheckBox > 67) {
		_timeLastCheckbox = timeNow;
		for (int i = 0; i < _lineCount; ++i) {
			if (_lines[i]->flags & 0x01) { // has checkbox
				if (_lines[i]->flags & 0x02) { // checkbox checked
					if (_lines[i]->checkboxFrame < 5) {
						_lines[i]->checkboxFrame += timeDiffCheckBox / 67;
					}
					if (_lines[i]->checkboxFrame > 5) {
						_lines[i]->checkboxFrame = 5;
					}
				} else { // checkbox not checked
					if (_lines[i]->checkboxFrame > 0) {
						_lines[i]->checkboxFrame -= timeDiffCheckBox / 67;
					}
					if (_lines[i]->checkboxFrame < 0) {
						_lines[i]->checkboxFrame = 0;
					}
				}
			}
		}
	}


	// update highlight
	if ((timeNow - _timeLastHighlight) > 67) {
		_timeLastHighlight = timeNow;
		_highlightFrame = (_highlightFrame + 1) % 8;
	}

	// draw text lines
	int linesVisible = 0;
	int lastLineVisible = 0;

	if (_maxLinesVisible < _lineCount - _firstLineVisible) {
		linesVisible = _maxLinesVisible;
		lastLineVisible = _firstLineVisible + _maxLinesVisible;
	} else {
		linesVisible = _lineCount - _firstLineVisible;
		lastLineVisible = _lineCount;
	}

	if (_firstLineVisible < lastLineVisible) {
		int y = _rect.top;
		int y1 = _rect.top + 8;
		int y2 = _rect.top + 2;
		int i = _firstLineVisible;
		do {
			int startingColorIndex = 3;
			if (i - _firstLineVisible < 3) {
				startingColorIndex = i - _firstLineVisible;
			}

			int endingColorIndex = 3;
			if (i - _firstLineVisible >= linesVisible - 3) {
				endingColorIndex = linesVisible - (i - _firstLineVisible + 1);
			}

			int colorIndex = endingColorIndex;
			if (startingColorIndex < endingColorIndex) {
				colorIndex = startingColorIndex;
			}

			bool v35 = false;
			int color = 0;

			if ((((_selectedLineState == 0 && i == _hoveredLine) || (_selectedLineState == 2 && i == _selectedLineIndex && _selectedLineIndex == _hoveredLine)) && _lines[i]->lineData != -1) || _lines[i]->flags & 0x04) {
				v35 = true;
				if (_style) {
					color = kTextColors2[colorIndex];
				} else {
					color = kTextColors3[colorIndex];
				}
			}
			else {
				if (_style) {
					color = kTextColors1[colorIndex];
				} else {
					color = kTextColors4[colorIndex];
				}
			}

			int x = _rect.left;

			if (_lines[i]->flags & 0x01) { // has checkbox
				int checkboxShapeId = 0;
				if (_style == 0) {
					if (_lines[i]->checkboxFrame || v35) {
						if (_lines[i]->checkboxFrame != 5 || v35) {
							checkboxShapeId = _lines[i]->checkboxFrame + 62;
						} else {
							checkboxShapeId = 61;
						}
					} else {
						checkboxShapeId = 60;
					}
				} else if (_lines[i]->checkboxFrame || v35) {
					if (_lines[i]->checkboxFrame != 5 || v35) {
						checkboxShapeId = _lines[i]->checkboxFrame + 54;
					} else {
						checkboxShapeId = 53;
					}
				} else {
					checkboxShapeId = 52;
				}
				_vm->_kia->_shapes->get(checkboxShapeId)->draw(surface, x - 1, y);
				x += 11;
			}

			if (_lines[i]->flags & 0x10) { // highlighted line
				if (_lines[i]->flags & 0x20) {
					int highlightShapeId = _highlightFrame;
					if (highlightShapeId > 4) {
						highlightShapeId = 8 - highlightShapeId;
					}
					_vm->_kia->_shapes->get(highlightShapeId + 85)->draw(surface, x, y2);
				}
				x += 6;
			}

			if (_lines[i]->flags & 0x08) { // has background rectangle
				int colorBackground = 0;
				if (_style) {
					colorBackground = kTextBackgroundColors[colorIndex];
				} else {
					colorBackground = 0x28E4;
				}
				surface.fillRect(Common::Rect(x, y, _rect.right + 1, y1 + 1), colorBackground);
			}

			if (_center) {
				x = _rect.left + (_rect.width() - _vm->_mainFont->getTextWidth(_lines[i]->text)) / 2;
			}

			_vm->_mainFont->drawColor(_lines[i]->text, surface, x, y, color);

			y1 += kLineHeight;
			y2 += kLineHeight;
			y += kLineHeight;
			++i;
		} while (i < lastLineVisible);
	}

	// draw scroll up button
	int scrollUpButtonShapeId = 0;
	if (_scrollUpButtonState) {
		if (_scrollUpButtonState == 2) {
			if (_scrollUpButtonHover) {
				scrollUpButtonShapeId = 72;
			} else {
				scrollUpButtonShapeId = 71;
			}
		} else {
			scrollUpButtonShapeId = 70;
		}
	} else if (_scrollUpButtonHover) {
		scrollUpButtonShapeId = 71;
	} else {
		scrollUpButtonShapeId = 70;
	}
	_vm->_kia->_shapes->get(scrollUpButtonShapeId)->draw(surface, _scrollBarRect.left, _scrollBarRect.top);

	// draw scroll down button
	int scrollDownButtonShapeId = 0;
	if (_scrollDownButtonState) {
		if (_scrollDownButtonState == 2) {
			if (_scrollDownButtonHover) {
				scrollDownButtonShapeId = 75;
			} else {
				scrollDownButtonShapeId = 74;
			}
		} else {
			scrollDownButtonShapeId = 73;
		}
	} else if (_scrollDownButtonHover) {
		scrollDownButtonShapeId = 74;
	} else {
		scrollDownButtonShapeId = 73;
	}
	_vm->_kia->_shapes->get(scrollDownButtonShapeId)->draw(surface, _scrollBarRect.left, _scrollBarRect.bottom - 7);

	int scrollAreaSize = _scrollBarRect.bottom - (_scrollBarRect.top + 15);
	int scrollBarHeight = 0;
	if (_lineCount <= _maxLinesVisible) {
		scrollBarHeight = _scrollBarRect.bottom - (_scrollBarRect.top + 15);
	} else {
		scrollBarHeight = _maxLinesVisible * scrollAreaSize / _lineCount;
	}
	scrollBarHeight = MAX(scrollBarHeight, 16);

	int v56 = 0;
	if (_lineCount <= _maxLinesVisible) {
		v56 = 0;
	} else {
		v56 = _firstLineVisible * (scrollAreaSize - scrollBarHeight) / (_lineCount - _maxLinesVisible);
	}

	int v58 = v56 + _scrollBarRect.top + 8;

	if (_scrollBarState == 2) {
		draw3DFrame(surface, Common::Rect(_scrollBarRect.left, v58, _scrollBarRect.left + 15, v58 + scrollBarHeight), 1, 1);
	} else if (!_scrollBarState && _scrollBarHover) {
		draw3DFrame(surface, Common::Rect(_scrollBarRect.left, v56 + _scrollBarRect.top + 8, _scrollBarRect.left + 15, v58 + scrollBarHeight), 0, 1);
	} else {
		draw3DFrame(surface, Common::Rect(_scrollBarRect.left, v58, _scrollBarRect.left + 15, v58 + scrollBarHeight), 0, 0);
	}
}

void UIScrollBox::checkAll() {
	for (int i = 0; i < _lineCount; ++i) {
		if (_lines[i]->flags & 0x01) {
			_lines[i]->flags |= 0x02;
		}
	}
}

void UIScrollBox::uncheckAll() {
	for (int i = 0; i < _lineCount; ++i) {
		if (_lines[i]->flags & 0x01) {
			_lines[i]->flags &= ~0x02;
		}
	}
}

void UIScrollBox::toggleCheckBox(int lineData) {
	int i = findLine(lineData);
	if (i != -1) {
		if (_lines[i]->flags & 0x02) {
			_lines[i]->flags &= ~0x02;
		} else {
			_lines[i]->flags |= 0x02;
		}
	}
}

bool UIScrollBox::hasLine(int lineData) {
	return findLine(lineData) != -1;
}

void UIScrollBox::resetHighlight(int lineData) {
	int i = findLine(lineData);
	if (i != -1) {
		_lines[i]->flags &= ~0x20;
	}
}

void UIScrollBox::setFlags(int lineData, int flags) {
	int i = findLine(lineData);
	if (i != -1) {
		_lines[i]->flags |= flags;
	}
}

void UIScrollBox::resetFlags(int lineData, int flags) {
	int i = findLine(lineData);
	if (i != -1) {
		_lines[i]->flags &= ~flags;
	}
}

int UIScrollBox::sortFunction(const void *item1, const void *item2) {
	Line *line1 = *(Line * const *)item1;
	Line *line2 = *(Line * const *)item2;
	return line1->text.compareToIgnoreCase(line2->text);
}

void UIScrollBox::draw3DFrame(Graphics::Surface &surface, Common::Rect rect, bool pressed, int style) {
	int color1, color2;

	if (pressed) {
		color1 = k3DFrameColors[style + 6];
		color2 = k3DFrameColors[style + 4];
	} else {
		color1 = k3DFrameColors[style + 4];
		color2 = k3DFrameColors[style + 6];
	}

	int fillColor = k3DFrameColors[style + 2];

	surface.fillRect(Common::Rect(rect.left + 1, rect.top + 1, rect.right - 1, rect.bottom - 1), fillColor);

	surface.hLine(rect.left + 1,  rect.top,        rect.right - 2,  color1);
	surface.hLine(rect.left + 1,  rect.bottom - 1, rect.right - 2,  color2);
	surface.vLine(rect.left,      rect.top,        rect.bottom - 2, color1);
	surface.vLine(rect.right - 1, rect.top + 1,    rect.bottom - 1, color2);
	surface.hLine(rect.right - 1, rect.top,        rect.right - 1,  k3DFrameColors[style]);
	surface.hLine(rect.left,      rect.bottom - 1, rect.left,       k3DFrameColors[style]);
}

void UIScrollBox::scrollUp() {
	if (_firstLineVisible > 0) {
		--_firstLineVisible;
	}
}

void UIScrollBox::scrollDown() {
	if (_lineCount - _firstLineVisible > _maxLinesVisible) {
		++_firstLineVisible;
	}
}

int UIScrollBox::findLine(int lineData) {
	for (int i = 0; i < _lineCount; ++i) {
		if (_lines[i]->lineData == lineData) {
			return i;
		}
	}
	return -1;
}

} // End of namespace BladeRunner
