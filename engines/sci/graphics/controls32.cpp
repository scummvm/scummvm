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
#include "common/translation.h"
#include "gui/message.h"
#include "sci/sci.h"
#include "sci/console.h"
#include "sci/event.h"
#include "sci/engine/kernel.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/state.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/controls32.h"
#include "sci/graphics/scifont.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/text32.h"

namespace Sci {
GfxControls32::GfxControls32(SegManager *segMan, GfxCache *cache, GfxText32 *text) :
	_segMan(segMan),
	_gfxCache(cache),
	_gfxText32(text),
	_overwriteMode(false),
	_nextCursorFlashTick(0),
	// SSCI used a memory handle for a ScrollWindow object as ID. We use a
	// simple numeric handle instead.
	_nextScrollWindowId(10000) {}

GfxControls32::~GfxControls32() {
	ScrollWindowMap::iterator it;
	for (it = _scrollWindows.begin(); it != _scrollWindows.end(); ++it)
		delete it->_value;
}

#pragma mark -
#pragma mark Text input control

reg_t GfxControls32::kernelEditText(const reg_t controlObject) {
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
		sourcePlane = g_sci->_gfxFrameout->getPlanes().findByObject(planeObj);
		if (sourcePlane == nullptr) {
			error("Could not find plane %04x:%04x", PRINT_REG(planeObj));
		}
	}
	editorPlaneRect.translate(sourcePlane->_gameRect.left, sourcePlane->_gameRect.top);

	editor.textRect = Common::Rect(2, titleHeight + 2, width - 1, height - 1);
	editor.width = width;

	if (editor.bitmap.isNull()) {
		TextAlign alignment = (TextAlign)readSelectorValue(_segMan, controlObject, SELECTOR(mode));

		if (titleObject.isNull()) {
			bool dimmed = readSelectorValue(_segMan, controlObject, SELECTOR(dimmed));
			editor.bitmap = _gfxText32->createFontBitmap(width, height, editor.textRect, editor.text, editor.foreColor, editor.backColor, editor.skipColor, editor.fontId, alignment, editor.borderColor, dimmed, true, false);
		} else {
			error("Titled bitmaps are not known to be used by any game. Please submit a bug report with details about the game you were playing and what you were doing that triggered this error. Thanks!");
		}
	}

	drawCursor(editor);

	Plane *plane = new Plane(editorPlaneRect, kPlanePicTransparent);
	plane->changePic();
	g_sci->_gfxFrameout->addPlane(plane);

	CelInfo32 celInfo;
	celInfo.type = kCelTypeMem;
	celInfo.bitmap = editor.bitmap;

	ScreenItem *screenItem = new ScreenItem(plane->_object, celInfo, Common::Point(), ScaleInfo());
	plane->_screenItemList.add(screenItem);

	// frameOut must be called after the screen item is created, and before it
	// is updated at the end of the event loop, otherwise it has both created
	// and updated flags set which crashes the engine (updates are handled
	// before creations, but the screen item is not in the correct state for an
	// update)
	g_sci->_gfxFrameout->frameOut(true);

	EventManager *eventManager = g_sci->getEventManager();
	bool clearTextOnInput = true;
	bool textChanged = false;
	for (;;) {
		// We peek here because the last event needs to be allowed to dispatch a
		// second time to the normal event handling system. In SSCI, the event
		// is always consumed and then the last event just gets posted back to
		// the event manager for reprocessing, but instead, we only remove the
		// event from the queue *after* we have determined it is not a
		// defocusing event
		const SciEvent event = eventManager->getSciEvent(kSciEventAny | kSciEventPeek);

		bool focused = true;
		// SSCI did not have a QUIT event, but we do, so we have to handle it
		if (event.type == kSciEventQuit) {
			focused = false;
		} else if (event.type == kSciEventMousePress && !editorPlaneRect.contains(event.mousePosSci)) {
			focused = false;
		} else if (event.type == kSciEventKeyDown) {
			switch (event.character) {
			case kSciKeyEsc:
			case kSciKeyUp:
			case kSciKeyDown:
			case kSciKeyTab:
			case kSciKeyShiftTab:
			case kSciKeyEnter:
				focused = false;
				break;
			default:
				break;
			}
		}

		if (!focused) {
			break;
		}

		// Consume the event now that we know it is not one of the defocusing
		// events above
		if (event.type != kSciEventNone)
			eventManager->getSciEvent(kSciEventAny);

		if (processEditTextEvent(event, editor, screenItem, clearTextOnInput)) {
			textChanged = true;
		}
	}

	g_sci->_gfxFrameout->deletePlane(*plane);
	if (readSelectorValue(_segMan, controlObject, SELECTOR(frameOut))) {
		g_sci->_gfxFrameout->frameOut(true);
	}

	_segMan->freeBitmap(editor.bitmap);

	if (textChanged) {
		editor.text.trim();
		SciArray &string = *_segMan->lookupArray(textObject);
		string.fromString(editor.text);
	}

	return make_reg(0, textChanged);
}

reg_t GfxControls32::kernelInputText(const reg_t textObject, const reg_t titleObject, const int16 maxTextLength) {
	// kInputText is only known to be used by Phantasmagoria 2 easter eggs

	TextEditor editor;
	editor.text = _segMan->getString(textObject);
	editor.foreColor = 0;
	editor.backColor = 255;
	editor.skipColor = 250;
	editor.fontId = -1;
	editor.maxLength = maxTextLength;
	editor.cursorCharPosition = 0;
	editor.cursorIsDrawn = false;
	editor.borderColor = 0;

	Common::String title = _segMan->getString(titleObject);
	_gfxText32->setFont(editor.fontId);
	GfxFont *systemFont = _gfxCache->getFont(editor.fontId);
	int16 textWidth = _gfxText32->getCharWidth('M', true) * maxTextLength;
	int16 titleWidth = _gfxText32->getStringWidth(title);
	int16 textHeight = _gfxText32->scaleUpHeight(systemFont->getHeight());
	int16 width = MAX(textWidth, titleWidth) + 4;
	int16 height = (textHeight * 2) + 7;

	// SSCI doesn't scale the position when centering, so the dialog is
	//  centered on the left side of the screen in Phantasmagoria 2.
	Common::Rect editorPlaneRect(width, height);
	editorPlaneRect.translate((320 - width) / 2, (200 - height) / 2);
	editor.textRect = Common::Rect(1, (height / 2) + 1, width - 1, height - 1);
	editor.width = width;

	editor.bitmap = _gfxText32->createTitledFontBitmap(width, height, editor.textRect, editor.text,
						editor.foreColor, editor.backColor, editor.skipColor, editor.fontId,
						kTextAlignLeft, editor.borderColor, title,
						editor.backColor, editor.foreColor, // title colors are inverse
						editor.fontId, true, true);

	drawCursor(editor);

	Plane *plane = new Plane(editorPlaneRect, kPlanePicTransparent);
	plane->changePic();
	g_sci->_gfxFrameout->addPlane(plane);

	CelInfo32 celInfo;
	celInfo.type = kCelTypeMem;
	celInfo.bitmap = editor.bitmap;

	ScreenItem *screenItem = new ScreenItem(plane->_object, celInfo, Common::Point(), ScaleInfo());
	plane->_screenItemList.add(screenItem);

	g_sci->_gfxFrameout->frameOut(true);

	EventManager *eventManager = g_sci->getEventManager();
	bool wasEnterPressed = false;
	bool clearTextOnInput = true;
	for (;;) {
		const SciEvent event = eventManager->getSciEvent(kSciEventAny | kSciEventPeek);

		bool exitEventLoop = false;
		// SSCI did not have a QUIT event, but we do, so we have to handle it
		if (event.type == kSciEventQuit) {
			exitEventLoop = true;
		} else if (event.type == kSciEventKeyDown) {
			switch (event.character) {
			case kSciKeyEsc:
				exitEventLoop = true;
				break;
			case kSciKeyEnter:
				wasEnterPressed = true;
				exitEventLoop = true;
				break;
			default:
				break;
			}
		}

		// consume all events except QUIT so that the engine quits
		if (event.type != kSciEventNone && event.type != kSciEventQuit) {
			eventManager->getSciEvent(kSciEventAny);
		}

		if (exitEventLoop) {
			break;
		}

		processEditTextEvent(event, editor, screenItem, clearTextOnInput);
	}

	g_sci->_gfxFrameout->deletePlane(*plane);
	g_sci->_gfxFrameout->frameOut(true);
	_segMan->freeBitmap(editor.bitmap);

	editor.text.trim();
	SciArray &string = *_segMan->lookupArray(textObject);
	string.fromString(editor.text);

	return make_reg(0, wasEnterPressed);
}

bool GfxControls32::processEditTextEvent(const SciEvent &event, TextEditor &editor, ScreenItem *screenItem, bool &clearTextOnInput) {
	// In SSCI, the font and bitmap were reset here on each iteration
	// through the loop, but this is not necessary since control is not
	// yielded back to the VM until input is received, which means there is
	// nothing that could modify the GfxText32's state with a different font
	// in the meantime

	bool textChanged = false;
	bool shouldDeleteChar = false;
	bool shouldRedrawText = false;
	uint16 lastCursorPosition = editor.cursorCharPosition;
	if (event.type == kSciEventKeyDown) {
		switch (event.character) {
		case kSciKeyLeft:
			clearTextOnInput = false;
			if (editor.cursorCharPosition > 0) {
				--editor.cursorCharPosition;
			}
			break;

		case kSciKeyRight:
			clearTextOnInput = false;
			if (editor.cursorCharPosition < editor.text.size()) {
				++editor.cursorCharPosition;
			}
			break;

		case kSciKeyHome:
			clearTextOnInput = false;
			editor.cursorCharPosition = 0;
			break;

		case kSciKeyEnd:
			clearTextOnInput = false;
			editor.cursorCharPosition = editor.text.size();
			break;

		case kSciKeyInsert:
			clearTextOnInput = false;
			// Redrawing also changes the cursor rect to reflect the new
			// insertion mode
			shouldRedrawText = true;
			_overwriteMode = !_overwriteMode;
			break;

		case kSciKeyDelete:
			clearTextOnInput = false;
			if (editor.cursorCharPosition < editor.text.size()) {
				shouldDeleteChar = true;
			}
			break;

		case kSciKeyBackspace:
			clearTextOnInput = false;
			shouldDeleteChar = true;
			if (editor.cursorCharPosition > 0) {
				--editor.cursorCharPosition;
			}
			break;

		case kSciKeyEtx:
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
	g_sci->_gfxFrameout->throttle();

	return textChanged;
}

void GfxControls32::drawCursor(TextEditor &editor) {
	if (!editor.cursorIsDrawn) {
		editor.cursorRect.left = editor.textRect.left + _gfxText32->getTextWidth(editor.text, 0, editor.cursorCharPosition);

		const int16 scaledFontHeight = _gfxText32->scaleUpHeight(_gfxText32->_font->getHeight());

		// SSCI branched on borderColor here but the two branches appeared to be
		// identical, differing only because the compiler decided to be
		// differently clever when optimising multiplication in each branch
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

#pragma mark -
#pragma mark Scrollable window control

ScrollWindow::ScrollWindow(SegManager *segMan, const Common::Rect &gameRect, const Common::Point &position, const reg_t plane, const uint8 defaultForeColor, const uint8 defaultBackColor, const GuiResourceId defaultFontId, const TextAlign defaultAlignment, const int16 defaultBorderColor, const uint16 maxNumEntries) :
	_segMan(segMan),
	_gfxText32(segMan, g_sci->_gfxCache),
	_maxNumEntries(maxNumEntries),
	_firstVisibleChar(0),
	_topVisibleLine(0),
	_lastVisibleChar(0),
	_bottomVisibleLine(0),
	_numLines(0),
	_numVisibleLines(0),
	_plane(plane),
	_foreColor(defaultForeColor),
	_backColor(defaultBackColor),
	_borderColor(defaultBorderColor),
	_fontId(defaultFontId),
	_alignment(defaultAlignment),
	_visible(false),
	_position(position),
	_screenItem(nullptr),
	_nextEntryId(1) {

	_entries.reserve(maxNumEntries);

	_gfxText32.setFont(_fontId);
	_pointSize = _gfxText32._font->getHeight();

	const uint16 scriptWidth = g_sci->_gfxFrameout->getScriptWidth();
	const uint16 scriptHeight = g_sci->_gfxFrameout->getScriptHeight();

	Common::Rect bitmapRect(gameRect);
	mulinc(bitmapRect, Ratio(_gfxText32._xResolution, scriptWidth), Ratio(_gfxText32._yResolution, scriptHeight));

	_textRect.left = 2;
	_textRect.top = 2;
	_textRect.right = bitmapRect.width() - 2;
	_textRect.bottom = bitmapRect.height() - 2;

	uint8 skipColor = 0;
	while (skipColor == _foreColor || skipColor == _backColor) {
		skipColor++;
	}

	assert(bitmapRect.width() > 0 && bitmapRect.height() > 0);
	_bitmap = _gfxText32.createFontBitmap(bitmapRect.width(), bitmapRect.height(), _textRect, "", _foreColor, _backColor, skipColor, _fontId, _alignment, _borderColor, false, false, false);

	debugC(1, kDebugLevelGraphics, "New ScrollWindow: textRect size: %d x %d, bitmap: %04x:%04x", _textRect.width(), _textRect.height(), PRINT_REG(_bitmap));
}

ScrollWindow::~ScrollWindow() {
	_segMan->freeBitmap(_bitmap);
	// _screenItem will be deleted by GfxFrameout
}

Ratio ScrollWindow::where() const {
	return Ratio(_topVisibleLine, MAX(_numLines, 1));
}

void ScrollWindow::show() {
	if (_visible) {
		return;
	}

	if (_screenItem == nullptr) {
		CelInfo32 celInfo;
		celInfo.type = kCelTypeMem;
		celInfo.bitmap = _bitmap;

		_screenItem = new ScreenItem(_plane, celInfo, _position, ScaleInfo());
	}

	Plane *plane = g_sci->_gfxFrameout->getPlanes().findByObject(_plane);

	if (plane == nullptr) {
		error("[ScrollWindow::show]: Plane %04x:%04x not found", PRINT_REG(_plane));
	}

	plane->_screenItemList.add(_screenItem);

	_visible = true;
}

void ScrollWindow::hide() {
	if (!_visible) {
		return;
	}

	g_sci->_gfxFrameout->deleteScreenItem(*_screenItem, _plane);
	_screenItem = nullptr;
	g_sci->_gfxFrameout->frameOut(true);

	_visible = false;
}

reg_t ScrollWindow::add(const Common::String &text, const GuiResourceId fontId, const int16 foreColor, const TextAlign alignment, const bool scrollTo) {
	if (_entries.size() == _maxNumEntries) {
		ScrollWindowEntry removedEntry = _entries.remove_at(0);
		_text.erase(0, removedEntry.text.size());
		// `_firstVisibleChar` will be reset shortly if `scrollTo` is true, so
		// there is no reason to update it
		if (!scrollTo) {
			_firstVisibleChar -= removedEntry.text.size();
		}
	}

	_entries.push_back(ScrollWindowEntry());
	ScrollWindowEntry &entry = _entries.back();

	// In SSCI, the line ID was a memory handle for the string of this line. We
	// use a numeric ID instead.
	entry.id = make_reg(0, _nextEntryId++);

	if (_nextEntryId > _maxNumEntries) {
		_nextEntryId = 1;
	}

	// In SSCI, this was updated after _text was updated, which meant there was
	// an extra unnecessary subtraction operation (subtracting `entry.text`
	// size)
	if (scrollTo) {
		_firstVisibleChar = _text.size();
	}

	fillEntry(entry, text, fontId, foreColor, alignment);
	_text += entry.text;

	computeLineIndices();
	update(true);

	return entry.id;
}

void ScrollWindow::fillEntry(ScrollWindowEntry &entry, const Common::String &text, const GuiResourceId fontId, const int16 foreColor, const TextAlign alignment) {
	entry.alignment = alignment;
	entry.foreColor = foreColor;
	entry.fontId = fontId;

	Common::String formattedText;

	// NB: There are inconsistencies here.
	// If there is a multi-line entry with non-default properties, and it
	// is only partially displayed, it may not be displayed right, since the
	// property directives are only added to the first line.
	// (Verified by trying this in SSCI SQ6 with a custom ScrollWindowAdd call.)
	//
	// The converse is also a potential issue (but unverified), where lines
	// with properties -1 can inherit properties from the previously rendered
	// line instead of the defaults.

	// SSCI added "|s<lineIndex>|" here, but |s| is not a valid control code, so
	// it just always ended up getting skipped by the text rendering code
	if (entry.fontId != -1) {
		formattedText += Common::String::format("|f%d|", entry.fontId);
	}
	if (entry.foreColor != -1) {
		formattedText += Common::String::format("|c%d|", entry.foreColor);
	}
	if (entry.alignment != -1) {
		formattedText += Common::String::format("|a%d|", entry.alignment);
	}
	formattedText += text;
	entry.text = formattedText;
}

reg_t ScrollWindow::modify(const reg_t id, const Common::String &text, const GuiResourceId fontId, const int16 foreColor, const TextAlign alignment, const bool scrollTo) {

	EntriesList::iterator it = _entries.begin();
	uint firstCharLocation = 0;
	for ( ; it != _entries.end(); ++it) {
		if (it->id == id) {
			break;
		}
		firstCharLocation += it->text.size();
	}

	if (it == _entries.end()) {
		return make_reg(0, 0);
	}

	ScrollWindowEntry &entry = *it;

	uint oldTextLength = entry.text.size();

	fillEntry(entry, text, fontId, foreColor, alignment);
	_text.replace(firstCharLocation, oldTextLength, entry.text);

	if (scrollTo) {
		_firstVisibleChar = firstCharLocation;
	}

	computeLineIndices();
	update(true);

	return entry.id;
}

void ScrollWindow::upArrow() {
	if (_topVisibleLine == 0) {
		return;
	}

	_topVisibleLine--;
	_bottomVisibleLine--;

	if (_bottomVisibleLine - _topVisibleLine + 1 < _numVisibleLines) {
		_bottomVisibleLine = _numLines - 1;
	}

	_firstVisibleChar = _startsOfLines[_topVisibleLine];
	_lastVisibleChar = _startsOfLines[_bottomVisibleLine + 1] - 1;

	_visibleText = Common::String(_text.c_str() + _firstVisibleChar, _text.c_str() + _lastVisibleChar + 1);

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
	if (_topVisibleLine + 1 >= _numLines) {
		return;
	}

	_topVisibleLine++;
	_bottomVisibleLine++;

	if (_bottomVisibleLine + 1 >= _numLines) {
		_bottomVisibleLine = _numLines - 1;
	}

	_firstVisibleChar = _startsOfLines[_topVisibleLine];
	_lastVisibleChar = _startsOfLines[_bottomVisibleLine + 1] - 1;

	_visibleText = Common::String(_text.c_str() + _firstVisibleChar, _text.c_str() + _lastVisibleChar + 1);

	Common::String lineText;
	if (_bottomVisibleLine - _topVisibleLine + 1 == _numVisibleLines) {
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

void ScrollWindow::go(const Ratio location) {
	const int line = (location * _numLines).toInt();
	if (line < 0 || line > _numLines) {
		error("Index is Out of Range in ScrollWindow");
	}

	_firstVisibleChar = _startsOfLines[line];
	update(true);

	// HACK:
	// It usually isn't possible to set _topVisibleLine >= _numLines, and so
	// update() doesn't. However, in this case we should set _topVisibleLine
	// past the end. This is clearly visible in Phantasmagoria when dragging
	// the slider in the About dialog to the very end. The slider ends up lower
	// than where it can be moved by scrolling down with the arrows.
	if (location.isOne()) {
		_topVisibleLine = _numLines;
	}
}

void ScrollWindow::home() {
	if (_firstVisibleChar == 0) {
		return;
	}

	_firstVisibleChar = 0;
	update(true);
}

void ScrollWindow::end() {
	if (_bottomVisibleLine + 1 >= _numLines) {
		return;
	}

	int line = _numLines - _numVisibleLines;
	if (line < 0) {
		line = 0;
	}
	_firstVisibleChar = _startsOfLines[line];
	update(true);
}

void ScrollWindow::pageUp() {
	if (_topVisibleLine == 0) {
		return;
	}

	_topVisibleLine -= _numVisibleLines;
	if (_topVisibleLine < 0) {
		_topVisibleLine = 0;
	}

	_firstVisibleChar = _startsOfLines[_topVisibleLine];
	update(true);
}

void ScrollWindow::pageDown() {
	if (_topVisibleLine + 1 >= _numLines) {
		return;
	}

	_topVisibleLine += _numVisibleLines;
	if (_topVisibleLine + 1 >= _numLines) {
		_topVisibleLine = _numLines - 1;
	}

	_firstVisibleChar = _startsOfLines[_topVisibleLine];
	update(true);
}

void ScrollWindow::computeLineIndices() {
	_gfxText32.setFont(_fontId);
	// Unlike SSCI, foreColor and alignment are not set since these properties
	// do not affect the width of lines

	if (_gfxText32._font->getHeight() != _pointSize) {
		error("Illegal font size font = %d pointSize = %d, should be %d.", _fontId, _gfxText32._font->getHeight(), _pointSize);
	}

	Common::Rect lineRect(0, 0, _textRect.width(), _pointSize + 3);

	_startsOfLines.clear();

	// SSCI had a 1000-line limit; we do not enforce any limit since we use
	// dynamic containers
	for (uint charIndex = 0; charIndex < _text.size(); ) {
		_startsOfLines.push_back(charIndex);
		charIndex += _gfxText32.getTextCount(_text, charIndex, lineRect, false);
	}

	_numLines = _startsOfLines.size();

	_startsOfLines.push_back(_text.size());

	_lastVisibleChar = _gfxText32.getTextCount(_text, 0, _fontId, _textRect, false) - 1;

	_bottomVisibleLine = 0;
	while (
		_bottomVisibleLine < _numLines - 1 &&
		_startsOfLines[_bottomVisibleLine + 1] < _lastVisibleChar
	) {
		++_bottomVisibleLine;
	}

	_numVisibleLines = _bottomVisibleLine + 1;
}

void ScrollWindow::update(const bool doFrameOut) {
	_topVisibleLine = 0;
	while (
		_topVisibleLine < _numLines - 1 &&
		_firstVisibleChar >= _startsOfLines[_topVisibleLine + 1]
	) {
		++_topVisibleLine;
	}

	_bottomVisibleLine = _topVisibleLine + _numVisibleLines - 1;
	if (_bottomVisibleLine >= _numLines) {
		_bottomVisibleLine = _numLines - 1;
	}

	_firstVisibleChar = _startsOfLines[_topVisibleLine];

	if (_bottomVisibleLine >= 0) {
		_lastVisibleChar = _startsOfLines[_bottomVisibleLine + 1] - 1;
	} else {
		_lastVisibleChar = -1;
	}

	_visibleText = Common::String(_text.c_str() + _firstVisibleChar, _text.c_str() + _lastVisibleChar + 1);

	_gfxText32.erase(_textRect, false);
	_gfxText32.drawTextBox(_visibleText);

	if (_visible) {
		assert(_screenItem);

		_screenItem->update();
		if (doFrameOut) {
			g_sci->_gfxFrameout->frameOut(true);
		}
	}
}

reg_t GfxControls32::makeScrollWindow(const Common::Rect &gameRect, const Common::Point &position, const reg_t planeObj, const uint8 defaultForeColor, const uint8 defaultBackColor, const GuiResourceId defaultFontId, const TextAlign defaultAlignment, const int16 defaultBorderColor, const uint16 maxNumEntries) {

	ScrollWindow *scrollWindow = new ScrollWindow(_segMan, gameRect, position, planeObj, defaultForeColor, defaultBackColor, defaultFontId, defaultAlignment, defaultBorderColor, maxNumEntries);

	const uint16 id = _nextScrollWindowId++;
	_scrollWindows[id] = scrollWindow;
	return make_reg(0, id);
}

ScrollWindow *GfxControls32::getScrollWindow(const reg_t id) {
	ScrollWindowMap::iterator it;
	it = _scrollWindows.find(id.toUint16());
	if (it == _scrollWindows.end())
		error("Invalid ScrollWindow ID");

	return it->_value;
}

void GfxControls32::destroyScrollWindow(const reg_t id) {
	ScrollWindow *scrollWindow = getScrollWindow(id);
	scrollWindow->hide();
	_scrollWindows.erase(id.getOffset());
	delete scrollWindow;
}

#pragma mark -
#pragma mark Message box

int16 GfxControls32::showMessageBox(const Common::U32String &message, const Common::U32String &okLabel, const Common::U32String &altLabel, const int16 okValue, const int16 altValue) {
	GUI::MessageDialog dialog(message, okLabel, altLabel);
	return (dialog.runModal() == GUI::kMessageOK) ? okValue : altValue;
}

reg_t GfxControls32::kernelMessageBox(const Common::String &message, const Common::String &title, const uint16 style) {
	PauseToken pt;
	if (g_engine) {
		pt = g_engine->pauseEngine();
	}

	int16 result;

	switch (style & 0xF) {
	case kMessageBoxOK:
		result = showMessageBox(message, _("OK"), Common::U32String(), 1, 1);
	break;
	case kMessageBoxYesNo:
		result = showMessageBox(message, _("Yes"), _("No"), 6, 7);
	break;
	default:
		error("Unsupported MessageBox style 0x%x", style & 0xF);
	}

	return make_reg(0, result);
}

} // End of namespace Sci
