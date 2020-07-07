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

namespace BladeRunner {

const Color256 UIScrollBox::k3DFrameColors[] = {
	{ 32, 32, 24 },
	{ 40, 40, 40 },
	{ 40, 40, 48 },
	{ 72, 64, 64 },
	{ 160, 136, 128 },
	{ 160, 136, 128 },
	{ 0, 0, 0 },
	{ 0, 0, 0 }
};
const Color256 UIScrollBox::kTextBackgroundColors[] = {
	{ 40, 56, 80 },
	{ 48, 64, 96 },
	{ 56, 72, 112 },
	{ 72, 88, 128 },
	{ 152, 192, 248 },
	{ 0, 0, 0 }
};
const Color256 UIScrollBox::kTextColors1[] = {
	{ 72, 104, 152 },
	{ 96, 120, 184 },
	{ 112, 144, 216 },
	{ 136, 168, 248 },
	{ 152, 192, 248 }
};
const Color256 UIScrollBox::kTextColors2[] = {
	{ 200, 216, 248 },
	{ 216, 224, 248 },
	{ 224, 232, 248 },
	{ 232, 240, 248 },
	{ 248, 248, 248 }
};
const Color256 UIScrollBox::kTextColors3[] = {
	{ 240, 232, 192 },
	{ 240, 232, 208 },
	{ 240, 240, 216 },
	{ 248, 240, 232 },
	{ 248, 248, 248 }
};
const Color256 UIScrollBox::kTextColors4[] = {
	{ 152, 112, 56 },
	{ 184, 144, 88 },
	{ 216, 184, 112 },
	{ 232, 208, 136 },
	{ 248, 224, 144 }
};

UIScrollBox::UIScrollBox(BladeRunnerEngine *vm,
	                     UIScrollBoxClickedCallback *lineSelectedCallback,
	                     void *callbackData,
	                     int maxLineCount,
	                     int style,
	                     bool center,
	                     Common::Rect rect,
	                     Common::Rect scrollBarRect) : UIComponent(vm) {

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
	_style      = style; // 0, 1 or (new) 2. "2" is similar to "1" but with solid background for main area and scroll bar
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
		_lines[i]->checkboxFrame = 5u;
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

bool UIScrollBox::isVisible() {
	return _isVisible;
}

bool UIScrollBox::hasFocus() {
	return _mouseOver;
}

void UIScrollBox::setBoxTop(int top) {
	_rect.moveTo(_rect.left, top);

	_rect.bottom = _rect.top + kLineHeight * _maxLinesVisible - 1;
}

void UIScrollBox::setBoxLeft(int left) {
	_rect.moveTo(left, _rect.top);
}

void UIScrollBox::setBoxWidth(uint16 width) {
	_rect.setWidth(width);
}

int UIScrollBox::getBoxLeft() {
	return _rect.left;
}

uint16 UIScrollBox::getBoxWidth() {
	return _rect.width();
}

void UIScrollBox::setScrollbarTop(int top) {
	_scrollBarRect.moveTo(_scrollBarRect.left, top);
}

void UIScrollBox::setScrollbarLeft(int left) {
	_scrollBarRect.moveTo(left, _scrollBarRect.top);
}

void UIScrollBox::setScrollbarWidth(uint16 width) {
	_scrollBarRect.setWidth(width);
	_scrollBarRect.right += 15; // right side was not used, but it's useful for determining if the control is selected
}

void UIScrollBox::clearLines() {
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
			_timeLastScroll = _vm->_time->currentSystem() - 160u;
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
			_timeLastScroll = _vm->_time->currentSystem() - 160u;
		} else {
			_scrollAreaUpState = 1;
		}
		if (_scrollAreaDownHover) {
			_scrollAreaDownState = 2;
			_timeLastScroll = _vm->_time->currentSystem() - 160u;
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

Common::String UIScrollBox::getLineText(int lineData) {
	if (hasLine(lineData)) {
		return _lines[_hoveredLine]->text;
	}
	return "";
}

int UIScrollBox::getMaxLinesVisible() {
	return _maxLinesVisible;
}

int UIScrollBox::getLineCount() {
	return _lineCount;
}

void UIScrollBox::draw(Graphics::Surface &surface) {
	if (!_isVisible) {
		return;
	}

	uint32 timeNow = _vm->_time->currentSystem();

	// update scrolling
	if (_scrollUpButtonState == 2 && _scrollUpButtonHover) {
		// unsigned difference is intentional
		if ((timeNow - _timeLastScroll) > 160u) {
			scrollUp();
			_timeLastScroll = timeNow;
		}
	} else if (_scrollDownButtonState == 2 && _scrollDownButtonHover) {
		// unsigned difference is intentional
		if ((timeNow - _timeLastScroll) > 160u) {
			scrollDown();
			_timeLastScroll = timeNow;
		}
	} else if (_scrollAreaUpState == 2 && _scrollAreaUpHover) {
		// unsigned difference is intentional
		if ((timeNow - _timeLastScroll) > 160u) {
			_firstLineVisible -= _maxLinesVisible - 1;
			_firstLineVisible = CLIP(_firstLineVisible, 0, _lineCount - _maxLinesVisible);
			_timeLastScroll = timeNow;
		}
	} else if (_scrollAreaDownState == 2 && _scrollAreaDownHover) {
		// unsigned difference is intentional
		if ((timeNow - _timeLastScroll) > 160u) {
			_firstLineVisible += _maxLinesVisible - 1;
			_firstLineVisible = CLIP(_firstLineVisible, 0, _lineCount - _maxLinesVisible);
			_timeLastScroll = timeNow;
		}
	}

	// update checkboxes
	// unsigned difference is intentional
	uint32 timeDiffCheckBox = timeNow - _timeLastCheckbox;
	if (timeDiffCheckBox > 67u) {
		_timeLastCheckbox = timeNow;
		for (int i = 0; i < _lineCount; ++i) {
			if (_lines[i]->flags & 0x01) { // has checkbox
				if (_lines[i]->flags & 0x02) { // checkbox checked
					if (_lines[i]->checkboxFrame < 5u) {
						_lines[i]->checkboxFrame += timeDiffCheckBox / 67u;
					}
					if (_lines[i]->checkboxFrame > 5u) {
						_lines[i]->checkboxFrame = 5u;
					}
				} else { // checkbox not checked
					if (_lines[i]->checkboxFrame > 0u) {
						_lines[i]->checkboxFrame =  (_lines[i]->checkboxFrame < (timeDiffCheckBox / 67u)) ? 0u : _lines[i]->checkboxFrame - (timeDiffCheckBox / 67u);
					}
					if (_lines[i]->checkboxFrame == 0u) { // original was < 0, int
						_lines[i]->checkboxFrame = 0u;
					}
				}
			}
		}
	}


	// update highlight
	// unsigned difference is intentional
	if ((timeNow - _timeLastHighlight) > 67u) {
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
					color = surface.format.RGBToColor(kTextColors2[colorIndex].r, kTextColors2[colorIndex].g, kTextColors2[colorIndex].b);
				} else {
					color = surface.format.RGBToColor(kTextColors3[colorIndex].r, kTextColors3[colorIndex].g, kTextColors3[colorIndex].b);
				}
			}
			else {
				if (_style) {
					color = surface.format.RGBToColor(kTextColors1[colorIndex].r, kTextColors1[colorIndex].g, kTextColors1[colorIndex].b);
				} else {
					color = surface.format.RGBToColor(kTextColors4[colorIndex].r, kTextColors4[colorIndex].g, kTextColors4[colorIndex].b);
				}
			}

			int x = _rect.left;

			if (_lines[i]->flags & 0x01) { // has checkbox
				int checkboxShapeId = 0;
				if (_style == 0) {
					if (_lines[i]->checkboxFrame || v35) {
						if (_lines[i]->checkboxFrame != 5u || v35) {
							checkboxShapeId = _lines[i]->checkboxFrame + 62u;
						} else {
							checkboxShapeId = 61;
						}
					} else {
						checkboxShapeId = 60;
					}
				} else if (_lines[i]->checkboxFrame || v35) {
					if (_lines[i]->checkboxFrame != 5u || v35) {
						checkboxShapeId = _lines[i]->checkboxFrame + 54u;
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
				if (_style == 2) {
					colorBackground = surface.format.RGBToColor(kTextBackgroundColors[colorIndex].r / 8, kTextBackgroundColors[colorIndex].g / 8, kTextBackgroundColors[colorIndex].b / 8);
				} else if (_style > 0) {
					colorBackground = surface.format.RGBToColor(kTextBackgroundColors[colorIndex].r, kTextBackgroundColors[colorIndex].g, kTextBackgroundColors[colorIndex].b);
				} else {
					colorBackground = surface.format.RGBToColor(80, 56, 32);
				}

				if (_style == 2) {
					// New: style = 2 (original unused)
					// original behavior -- No padding between the colored background of lines, simulate solid background (gradient)
					surface.fillRect(Common::Rect(CLIP(x - 1, 0, 639), y, _rect.right + 1, y + kLineHeight), colorBackground);
				} else {
					// original behavior -- there is padding between the colored background of lines
					surface.fillRect(Common::Rect(x, y, _rect.right + 1, y1 + 1), colorBackground);
				}
			}

			if (_center) {
				x = _rect.left + (_rect.width() - _vm->_mainFont->getStringWidth(_lines[i]->text)) / 2;
			}

			_vm->_mainFont->drawString(&surface, _lines[i]->text, x, y, surface.w, color);

			y1 += kLineHeight;
			y2 += kLineHeight;
			y += kLineHeight;
			++i;
		} while (i < lastLineVisible);
	}

	if (_style == 2 && getLineCount() >= getMaxLinesVisible()) {
		// New: style = 2 (original unused)
		// Solid background color for scrollbar
		int scrollBarFillColor = surface.format.RGBToColor(k3DFrameColors[0].r / 2, k3DFrameColors[0].g / 2, k3DFrameColors[0].b / 2);
		surface.fillRect(Common::Rect(_scrollBarRect.left, _scrollBarRect.top, CLIP(_scrollBarRect.left + 15, 0, 639), _scrollBarRect.bottom), scrollBarFillColor);
	}

	if (_style != 2
	    || (_style == 2 && getLineCount() >= getMaxLinesVisible())
	) {
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
		color1 = surface.format.RGBToColor(k3DFrameColors[style + 6].r, k3DFrameColors[style + 6].g, k3DFrameColors[style + 6].b);
		color2 = surface.format.RGBToColor(k3DFrameColors[style + 4].r, k3DFrameColors[style + 4].g, k3DFrameColors[style + 4].b);
	} else {
		color1 = surface.format.RGBToColor(k3DFrameColors[style + 4].r, k3DFrameColors[style + 4].g, k3DFrameColors[style + 4].b);
		color2 = surface.format.RGBToColor(k3DFrameColors[style + 6].r, k3DFrameColors[style + 6].g, k3DFrameColors[style + 6].b);
	}

	int color3 = surface.format.RGBToColor(k3DFrameColors[style].r, k3DFrameColors[style].g, k3DFrameColors[style].b);
	int fillColor = surface.format.RGBToColor(k3DFrameColors[style + 2].r, k3DFrameColors[style + 2].g, k3DFrameColors[style + 2].b);

	surface.fillRect(Common::Rect(rect.left + 1, rect.top + 1, rect.right - 1, rect.bottom - 1), fillColor);

	surface.hLine(rect.left + 1,  rect.top,        rect.right - 2,  color1);
	surface.hLine(rect.left + 1,  rect.bottom - 1, rect.right - 2,  color2);
	surface.vLine(rect.left,      rect.top,        rect.bottom - 2, color1);
	surface.vLine(rect.right - 1, rect.top + 1,    rect.bottom - 1, color2);
	surface.hLine(rect.right - 1, rect.top,        rect.right - 1,  color3);
	surface.hLine(rect.left,      rect.bottom - 1, rect.left,       color3);
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
