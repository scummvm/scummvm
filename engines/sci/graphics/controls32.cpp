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

#include "common/system.h"

#include "sci/sci.h"
#include "sci/console.h"
#include "sci/event.h"
#include "sci/engine/kernel.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/state.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/controls32.h"
#include "sci/graphics/font.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/text32.h"

namespace Sci {
GfxControls32::GfxControls32(SegManager *segMan, GfxCache *cache, GfxText32 *text) :
	_segMan(segMan),
	_gfxCache(cache),
	_gfxText32(text),
	_overwriteMode(false),
	_nextCursorFlashTick(0)
{
	// SSCI used a memory handle for a ScrollWindow object as ID.
	// We use a simple numeric handle instead.
	_lastScrollWindowId = make_reg(0, 9999);
}

GfxControls32::~GfxControls32() {
	Common::HashMap<int, ScrollWindow *>::iterator it;
	for (it = _scrollWindows.begin(); it != _scrollWindows.end(); ++it)
		delete it->_value;
}

Common::Array<reg_t> GfxControls32::listObjectReferences() {
	Common::Array<reg_t> ret;
	Common::HashMap<int, ScrollWindow *>::const_iterator it;
	for (it = _scrollWindows.begin(); it != _scrollWindows.end(); ++it)
		ret.push_back(it->_value->getBitmap());

	return ret;
}


reg_t GfxControls32::kernelEditText(const reg_t controlObject) {
	SegManager *segMan = _segMan;

	TextEditor editor;
	reg_t textObject = readSelector(_segMan, controlObject, SELECTOR(text));
	editor.text = _segMan->getString(textObject);
	editor.foreColor = readSelectorValue(_segMan, controlObject, SELECTOR(fore));
	editor.backColor = readSelectorValue(_segMan, controlObject, SELECTOR(back));
	editor.skipColor = readSelectorValue(_segMan, controlObject, SELECTOR(skip));
	editor.fontId = readSelectorValue(_segMan, controlObject, SELECTOR(font));
	editor.maxLength = readSelectorValue(_segMan, controlObject, SELECTOR(width));
	editor.bitmap = readSelector(_segMan, controlObject, SELECTOR(bitmap));
	editor.cursorCharPosition = 0;
	editor.cursorIsDrawn = false;
	editor.borderColor = readSelectorValue(_segMan, controlObject, SELECTOR(borderColor));

	reg_t titleObject = readSelector(_segMan, controlObject, SELECTOR(title));

	int16 titleHeight = 0;
	GuiResourceId titleFontId = readSelectorValue(_segMan, controlObject, SELECTOR(titleFont));
	if (!titleObject.isNull()) {
		GfxFont *titleFont = _gfxCache->getFont(titleFontId);
		titleHeight += _gfxText32->scaleUpHeight(titleFont->getHeight()) + 1;
		if (editor.borderColor != -1) {
			titleHeight += 2;
		}
	}

	int16 width = 0;
	int16 height = titleHeight;

	GfxFont *editorFont = _gfxCache->getFont(editor.fontId);
	height += _gfxText32->scaleUpHeight(editorFont->getHeight()) + 1;
	_gfxText32->setFont(editor.fontId);
	int16 emSize = _gfxText32->getCharWidth('M', true);
	width += editor.maxLength * emSize + 1;
	if (editor.borderColor != -1) {
		width += 4;
		height += 2;
	}

	Common::Rect editorPlaneRect(width, height);
	editorPlaneRect.translate(readSelectorValue(_segMan, controlObject, SELECTOR(x)), readSelectorValue(_segMan, controlObject, SELECTOR(y)));

	reg_t planeObj = readSelector(_segMan, controlObject, SELECTOR(plane));
	Plane *sourcePlane = g_sci->_gfxFrameout->getVisiblePlanes().findByObject(planeObj);
	if (sourcePlane == nullptr) {
		error("Could not find plane %04x:%04x", PRINT_REG(planeObj));
	}
	editorPlaneRect.translate(sourcePlane->_gameRect.left, sourcePlane->_gameRect.top);

	editor.textRect = Common::Rect(2, titleHeight + 2, width - 1, height - 1);
	editor.width = width;

	if (editor.bitmap.isNull()) {
		TextAlign alignment = (TextAlign)readSelectorValue(_segMan, controlObject, SELECTOR(mode));

		if (titleObject.isNull()) {
			bool dimmed = readSelectorValue(_segMan, controlObject, SELECTOR(dimmed));
			editor.bitmap = _gfxText32->createFontBitmap(width, height, editor.textRect, editor.text, editor.foreColor, editor.backColor, editor.skipColor, editor.fontId, alignment, editor.borderColor, dimmed, true);
		} else {
			Common::String title = _segMan->getString(titleObject);
			int16 titleBackColor = readSelectorValue(_segMan, controlObject, SELECTOR(titleBack));
			int16 titleForeColor = readSelectorValue(_segMan, controlObject, SELECTOR(titleFore));
			editor.bitmap = _gfxText32->createTitledBitmap(width, height, editor.textRect, editor.text, editor.foreColor, editor.backColor, editor.skipColor, editor.fontId, alignment, editor.borderColor, title, titleForeColor, titleBackColor, titleFontId, true);
		}
	}

	drawCursor(editor);

	Plane *plane = new Plane(editorPlaneRect, kPlanePicTransparent);
	plane->changePic();
	g_sci->_gfxFrameout->addPlane(*plane);

	CelInfo32 celInfo;
	celInfo.type = kCelTypeMem;
	celInfo.bitmap = editor.bitmap;

	ScreenItem *screenItem = new ScreenItem(plane->_object, celInfo, Common::Point(), ScaleInfo());
	plane->_screenItemList.add(screenItem);

	// frameOut must be called after the screen item is
	// created, and before it is updated at the end of the
	// event loop, otherwise it has both created and updated
	// flags set which crashes the engine (it runs updates
	// before creations)
	g_sci->_gfxFrameout->frameOut(true);

	EventManager *eventManager = g_sci->getEventManager();
	bool clearTextOnInput = true;
	bool textChanged = false;
	for (;;) {
		// We peek here because the last event needs to be allowed to
		// dispatch a second time to the normal event handling system.
		// In the actual engine, the event is always consumed and then
		// the last event just gets posted back to the event manager for
		// reprocessing, but instead, we only remove the event from the
		// queue *after* we have determined it is not a defocusing event
		const SciEvent event = eventManager->getSciEvent(SCI_EVENT_ANY | SCI_EVENT_PEEK);

		bool focused = true;
		// Original engine did not have a QUIT event but we have to handle it
		if (event.type == SCI_EVENT_QUIT) {
			focused = false;
			break;
		} else if (event.type == SCI_EVENT_MOUSE_PRESS && !editorPlaneRect.contains(event.mousePosSci)) {
			focused = false;
		} else if (event.type == SCI_EVENT_KEYBOARD) {
			switch (event.character) {
			case SCI_KEY_ESC:
			case SCI_KEY_UP:
			case SCI_KEY_DOWN:
			case SCI_KEY_TAB:
			case SCI_KEY_SHIFT_TAB:
			case SCI_KEY_ENTER:
				focused = false;
				break;
			}
		}

		if (!focused) {
			break;
		}

		// Consume the event now that we know it is not one of the
		// defocusing events above
		eventManager->getSciEvent(SCI_EVENT_ANY);

		// NOTE: In the original engine, the font and bitmap were
		// reset here on each iteration through the loop, but it
		// doesn't seem like this should be necessary since
		// control is not yielded back to the VM until input is
		// received, which means there is nothing that could modify
		// the GfxText32's state with a different font in the
		// meantime

		bool shouldDeleteChar = false;
		bool shouldRedrawText = false;
		uint16 lastCursorPosition = editor.cursorCharPosition;
 		if (event.type == SCI_EVENT_KEYBOARD) {
			switch (event.character) {
			case SCI_KEY_LEFT:
				clearTextOnInput = false;
				if (editor.cursorCharPosition > 0) {
					--editor.cursorCharPosition;
				}
				break;

			case SCI_KEY_RIGHT:
				clearTextOnInput = false;
				if (editor.cursorCharPosition < editor.text.size()) {
					++editor.cursorCharPosition;
				}
				break;

			case SCI_KEY_HOME:
				clearTextOnInput = false;
				editor.cursorCharPosition = 0;
				break;

			case SCI_KEY_END:
				clearTextOnInput = false;
				editor.cursorCharPosition = editor.text.size();
				break;

			case SCI_KEY_INSERT:
				clearTextOnInput = false;
				// Redrawing also changes the cursor rect to
				// reflect the new insertion mode
				shouldRedrawText = true;
				_overwriteMode = !_overwriteMode;
				break;

			case SCI_KEY_DELETE:
				clearTextOnInput = false;
				if (editor.cursorCharPosition < editor.text.size()) {
					shouldDeleteChar = true;
				}
				break;

			case SCI_KEY_BACKSPACE:
				clearTextOnInput = false;
				shouldDeleteChar = true;
				if (editor.cursorCharPosition > 0) {
					--editor.cursorCharPosition;
				}
				break;

			case SCI_KEY_ETX:
				editor.text.clear();
				editor.cursorCharPosition = 0;
				shouldRedrawText = true;
				break;

			default: {
				if (event.character >= 20 && event.character < 257) {
					if (clearTextOnInput) {
						clearTextOnInput = false;
						editor.text.clear();
					}

					if (
						(_overwriteMode && editor.cursorCharPosition < editor.maxLength) ||
						(editor.text.size() < editor.maxLength && _gfxText32->getCharWidth(event.character, true) + _gfxText32->getStringWidth(editor.text) < editor.textRect.width())
					) {
						if (_overwriteMode && editor.cursorCharPosition < editor.text.size()) {
							editor.text.setChar(event.character, editor.cursorCharPosition);
						} else {
							editor.text.insertChar(event.character, editor.cursorCharPosition);
						}

						++editor.cursorCharPosition;
						shouldRedrawText = true;
					}
				}
			}
			}
		}

		if (shouldDeleteChar) {
			shouldRedrawText = true;
			if (editor.cursorCharPosition < editor.text.size()) {
				editor.text.deleteChar(editor.cursorCharPosition);
			}
		}

		if (shouldRedrawText) {
			eraseCursor(editor);
			_gfxText32->erase(editor.textRect, true);
			_gfxText32->drawTextBox(editor.text);
			drawCursor(editor);
			textChanged = true;
			screenItem->_updated = g_sci->_gfxFrameout->getScreenCount();
		} else if (editor.cursorCharPosition != lastCursorPosition) {
			eraseCursor(editor);
			drawCursor(editor);
			screenItem->_updated = g_sci->_gfxFrameout->getScreenCount();
		} else {
			flashCursor(editor);
			screenItem->_updated = g_sci->_gfxFrameout->getScreenCount();
		}

		g_sci->_gfxFrameout->frameOut(true);
		g_sci->getSciDebugger()->onFrame();
		g_sci->getEngineState()->speedThrottler(16);
		g_sci->getEngineState()->_throttleTrigger = true;
	}

	g_sci->_gfxFrameout->deletePlane(*plane);
	if (readSelectorValue(segMan, controlObject, SELECTOR(frameOut))) {
		g_sci->_gfxFrameout->frameOut(true);
	}

	_segMan->freeHunkEntry(editor.bitmap);

	if (textChanged) {
		editor.text.trim();
		SciString *string = _segMan->lookupString(textObject);
		string->fromString(editor.text);
	}

	return make_reg(0, textChanged);
}

void GfxControls32::drawCursor(TextEditor &editor) {
	if (!editor.cursorIsDrawn) {
		editor.cursorRect.left = editor.textRect.left + _gfxText32->getTextWidth(editor.text, 0, editor.cursorCharPosition);

		const int16 scaledFontHeight = _gfxText32->scaleUpHeight(_gfxText32->_font->getHeight());

		// NOTE: The original code branched on borderColor here but
		// the two branches appeared to be identical, differing only
		// because the compiler decided to be differently clever
		// when optimising multiplication in each branch
		if (_overwriteMode) {
			editor.cursorRect.top = editor.textRect.top;
			editor.cursorRect.setHeight(scaledFontHeight);
		} else {
			editor.cursorRect.top = editor.textRect.top + scaledFontHeight - 1;
			editor.cursorRect.setHeight(1);
		}

		const char currentChar = editor.cursorCharPosition < editor.text.size() ? editor.text[editor.cursorCharPosition] : ' ';
		editor.cursorRect.setWidth(_gfxText32->getCharWidth(currentChar, true));

		_gfxText32->invertRect(editor.bitmap, editor.width, editor.cursorRect, editor.foreColor, editor.backColor, true);

		editor.cursorIsDrawn = true;
	}

	_nextCursorFlashTick = g_sci->getTickCount() + 30;
}

void GfxControls32::eraseCursor(TextEditor &editor) {
	if (editor.cursorIsDrawn) {
		_gfxText32->invertRect(editor.bitmap, editor.width, editor.cursorRect, editor.foreColor, editor.backColor, true);
		editor.cursorIsDrawn = false;
	}

	_nextCursorFlashTick = g_sci->getTickCount() + 30;
}

void GfxControls32::flashCursor(TextEditor &editor) {
	if (g_sci->getTickCount() > _nextCursorFlashTick) {
		_gfxText32->invertRect(editor.bitmap, editor.width, editor.cursorRect, editor.foreColor, editor.backColor, true);

		editor.cursorIsDrawn = !editor.cursorIsDrawn;
		_nextCursorFlashTick = g_sci->getTickCount() + 30;
	}
}


reg_t GfxControls32::registerScrollWindow(ScrollWindow *scrollWindow) {
	_lastScrollWindowId += 1;
	_scrollWindows[_lastScrollWindowId.getOffset()] = scrollWindow;
	return _lastScrollWindowId;
}


ScrollWindow *GfxControls32::getScrollWindow(reg_t id) {
	Common::HashMap<int, ScrollWindow *>::iterator it;
	it = _scrollWindows.find(id.getOffset());
	if (it == _scrollWindows.end())
		error("Invalid ScrollWindow ID");

	return it->_value;
}


void GfxControls32::deregisterScrollWindow(reg_t id) {
	_scrollWindows.erase(id.getOffset());
}



ScrollWindow::ScrollWindow(SegManager *segMan, const Common::Rect &rect,
                           const Common::Point &point, reg_t plane, uint8 fore,
                           uint8 back, GuiResourceId font, TextAlign align,
                           uint8 border)
: _gfxText32(segMan, g_sci->_gfxCache),
  _firstVisibleChar(0), _topVisibleLine(0),
  _lastVisibleChar(0), _bottomVisibleLine(0),
  _numLines(0), _numVisibleLines(0),
  _plane(plane), _foreColor(fore), _backColor(back),
  _borderColor(border), _fontId(font), _alignment(align),
  _visible(false), _position(point), _screenItem(nullptr) {

	_gfxText32.setFont(_fontId);

	_fontScaledWidth = _gfxText32._scaledWidth;
	_fontScaledHeight = _gfxText32._scaledHeight;

	int scriptWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
	int scriptHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;

	_screenRect.left = (rect.left * _fontScaledWidth) / scriptWidth;
	_screenRect.right = ((rect.right - 1) * _fontScaledWidth) / scriptWidth + 1;
	_screenRect.top = (rect.top * _fontScaledHeight) / scriptHeight;
	_screenRect.bottom = ((rect.bottom - 1) * _fontScaledHeight) / scriptHeight + 1;

	_textRect.left = 2;
	_textRect.top = 2;
	_textRect.right = _screenRect.width() - 2;
	_textRect.bottom = _screenRect.height() - 2;

	_pointSize = _gfxText32._font->getHeight();

	uint8 skipColor = 0;
	while (skipColor == _foreColor || skipColor == _backColor)
		skipColor++;

	assert(_screenRect.width() > 0 && _screenRect.height() > 0);
	_bitmap = _gfxText32.createFontBitmap(_screenRect.width(), _screenRect.height(),
	                                      _textRect, "", _foreColor, _backColor,
	                                      skipColor, _fontId, _alignment,
	                                      _borderColor, false, false);

	debugC(1, kDebugLevelGraphics, "New ScrollWindow: textRect size: %d x %d, bitmap: %04x:%04x", _textRect.width(), _textRect.height(), PRINT_REG(_bitmap));

	// We give lines handles starting at 10000. This is unlike SSCI, where
	// line handles were in fact TextIDs.
	_lastLineId = make_reg(0, 9999);
}


ScrollWindow::~ScrollWindow() {
	// _gfxText32._bitmap will get GCed once ScrollWindow is gone.
	// _screenItem will be deleted by GfxFrameout
}


Ratio ScrollWindow::where() const {
	return Ratio(_topVisibleLine, MAX(_numLines, 1));
}


void ScrollWindow::show() {
	if (_visible)
		return;

	if (_screenItem == nullptr) {
		CelInfo32 c;
		c.bitmap = _bitmap;

		ScaleInfo s;

		_screenItem = new ScreenItem(_plane, c, _position, s);
	}

	Plane *plane = g_sci->_gfxFrameout->getPlanes().findByObject(_plane);
	plane->_screenItemList.add(_screenItem);

	_visible = true;
}


void ScrollWindow::hide() {
	if (!_visible)
		return;

	Plane *plane = g_sci->_gfxFrameout->getPlanes().findByObject(_plane);

	// TODO: Remove duplication with GfxFrameout::kernelDeleteScreenItem
	if (_screenItem->_created == 0) {
		_screenItem->_created = 0;
		_screenItem->_updated = 0;
		_screenItem->_deleted = g_sci->_gfxFrameout->getScreenCount();
	} else {
		plane->_screenItemList.erase(_screenItem);
		plane->_screenItemList.pack();
	}

	_screenItem = nullptr;

	g_sci->_gfxFrameout->frameOut(true);

	_visible = false;
}


reg_t ScrollWindow::add(const Common::String &str, GuiResourceId font,
                        int fore, int align, bool scrollTo) {

	_lines.push_back(ScrollWindowLine());
	ScrollWindowLine line = _lines.back();
	line._alignment = align;
	line._foreColor = fore;
	line._fontId = font;

	// In SSCI the line ID was actually a memory handle for the
	// string of this line. We use a numeric ID instead.
	_lastLineId += 1;
	line._id = _lastLineId;


	// TODO: There are potential inconsistencies here, that seem to also exist
	// in SSCI. When line properties are -1, they in practice are displayed
	// with the default values from the ScrollWindow. However, if the
	// whole ScrollWindow is displayed at once, they might instead use the
	// properties of the previous line. Conversely, if there is a multi-line
	// entry with non-default properties, all lines except the first one might
	// be displayed with the defaults.

	Common::String s;
	s = Common::String::format("|s%d|", _lines.size() - 1);
	if (line._fontId != -1)
		s += Common::String::format("|f%d|", line._fontId);
	if (line._foreColor != -1)
		s += Common::String::format("|c%d|", line._foreColor);
	if (line._alignment != -1)
		s += Common::String::format("|a%d|", line._alignment);
	s += str;

	line._str = s;
	_text += s;


	if (scrollTo)
		_firstVisibleChar = _text.size() - s.size();

	computeLineIndices();

	update(true);

	return line._id;
}


void ScrollWindow::upArrow() {
	if (_topVisibleLine == 0)
		return;

	_topVisibleLine--;
	_bottomVisibleLine--;

	if (_bottomVisibleLine - _topVisibleLine + 1 < _numVisibleLines)
		_bottomVisibleLine = _numLines - 1;

	_firstVisibleChar = _startsOfLines[_topVisibleLine];
	_lastVisibleChar = _startsOfLines[_bottomVisibleLine + 1] - 1;

	_visibleText = Common::String(_text.c_str() + _firstVisibleChar, _text.c_str() + _lastVisibleChar + 1);

	// TODO: Double check this -1 at the end (for the \n)
	Common::String lineText(_text.c_str() + _startsOfLines[_topVisibleLine], _text.c_str() + _startsOfLines[_topVisibleLine + 1] - 1);

	debugC(3, kDebugLevelGraphics, "ScrollWindow::upArrow: top: %d, bottom: %d, num: %d, numvis: %d, lineText: %s", _topVisibleLine, _bottomVisibleLine, _numLines, _numVisibleLines, lineText.c_str());

	_gfxText32.scrollLine(lineText, _numVisibleLines, _foreColor, _alignment, _fontId, kScrollUp);

	if (_visible) {
		assert(_screenItem);

		_screenItem->update();
		g_sci->_gfxFrameout->frameOut(true);
	}
}


void ScrollWindow::downArrow() {
	if (_topVisibleLine + 1 >= _numLines)
		return;

	_topVisibleLine++;
	_bottomVisibleLine++;

	if (_bottomVisibleLine + 1 >= _numLines)
		_bottomVisibleLine = _numLines - 1;

	_firstVisibleChar = _startsOfLines[_topVisibleLine];
	_lastVisibleChar = _startsOfLines[_bottomVisibleLine + 1] - 1;

	_visibleText = Common::String(_text.c_str() + _firstVisibleChar, _text.c_str() + _lastVisibleChar + 1);

	Common::String lineText;
	if (_bottomVisibleLine - _topVisibleLine + 1 == _numVisibleLines) {
		// TODO: Double check this -1 at the end (for the \n)
		lineText = Common::String(_text.c_str() + _startsOfLines[_bottomVisibleLine], _text.c_str() + _startsOfLines[_bottomVisibleLine + 1] - 1);
	} else {
		// scroll in empty string
	}

	debugC(3, kDebugLevelGraphics, "ScrollWindow::downArrow: top: %d, bottom: %d, num: %d, numvis: %d, lineText: %s", _topVisibleLine, _bottomVisibleLine, _numLines, _numVisibleLines, lineText.c_str());


	_gfxText32.scrollLine(lineText, _numVisibleLines, _foreColor, _alignment, _fontId, kScrollDown);

	if (_visible) {
		assert(_screenItem);

		_screenItem->update();
		g_sci->_gfxFrameout->frameOut(true);
	}
}


void ScrollWindow::go(Ratio loc) {
	int line = (loc * _numLines).toInt();
	if (line < 0 || line > _numLines)
		error("Index is Out of Range in ScrollWindow");

	_firstVisibleChar = _startsOfLines[line];
	update(true);

	// HACK:
	// It usually isn't possible to set _topVisibleLine >= _numLines, and so
	// update() doesn't. However, in this case we should set _topVisibleLine
	// past the end. This is clearly visible in Phantasmagoria when dragging
	// the slider in the About dialog to the very end. The slider ends up lower
	// than were it can be moved by scrolling down with the arrows.
	if (loc.isOne())
		_topVisibleLine = _numLines;
}


void ScrollWindow::home() {
	if (_firstVisibleChar == 0)
		return;

	_firstVisibleChar = 0;
	update(true);
}


void ScrollWindow::pageUp() {
	if (_topVisibleLine == 0)
		return;

	_topVisibleLine -= _numVisibleLines;
	if (_topVisibleLine < 0)
		_topVisibleLine = 0;

	_firstVisibleChar = _startsOfLines[_topVisibleLine];
	update(true);
}


void ScrollWindow::pageDown() {
	if (_topVisibleLine + 1 >= _numLines)
		return;

	_topVisibleLine += _numVisibleLines;
	if (_topVisibleLine + 1 >= _numLines)
		_topVisibleLine = _numLines - 1;

	_firstVisibleChar = _startsOfLines[_topVisibleLine];
	update(true);
}


void ScrollWindow::computeLineIndices() {
	_gfxText32.setFont(_fontId);
	// set _gfxText32 foreColor, alignment?

	if (_gfxText32._font->getHeight() != _pointSize) {
		error("ScrollWindow font size mismatch");
	}

	Common::Rect lineRect(0, 0, _textRect.width() + 1, _pointSize + 3);

	_startsOfLines.clear();

	uint index = 0;

	while (index < _text.size()) {
		_startsOfLines.push_back(index);

		index += _gfxText32.getTextCount(_text, index, lineRect, false);
	}
	_numLines = _startsOfLines.size();

	_startsOfLines.push_back(_text.size());

	_lastVisibleChar = _gfxText32.getTextCount(_text, 0, _fontId, _textRect, false) - 1;

	_bottomVisibleLine = 0;
	while (_bottomVisibleLine < _numLines - 1 &&
	       _startsOfLines[_bottomVisibleLine + 1] < _lastVisibleChar)
		_bottomVisibleLine++;

	_numVisibleLines = _bottomVisibleLine + 1;
}


void ScrollWindow::update(bool doFrameOut) {
	_topVisibleLine = 0;
	while (_topVisibleLine < _numLines - 1 &&
	       _firstVisibleChar >= _startsOfLines[_topVisibleLine + 1])
		++_topVisibleLine;

	_bottomVisibleLine = _topVisibleLine + _numVisibleLines - 1;
	if (_bottomVisibleLine >= _numLines)
		_bottomVisibleLine = _numLines - 1;

	_firstVisibleChar = _startsOfLines[_topVisibleLine];

	if (_bottomVisibleLine >= 0)
		_lastVisibleChar = _startsOfLines[_bottomVisibleLine + 1] - 1;
	else
		_lastVisibleChar = -1;

	_visibleText = Common::String(_text.c_str() + _firstVisibleChar, _text.c_str() + _lastVisibleChar + 1);

	_gfxText32.erase(_textRect, false);

	_gfxText32.drawTextBox(_visibleText);

	if (_visible) {
		assert(_screenItem);

		_screenItem->update();
		if (doFrameOut)
			g_sci->_gfxFrameout->frameOut(true);
	}
}


} // End of namespace Sci
