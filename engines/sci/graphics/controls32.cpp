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
	_nextCursorFlashTick(0) {}

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
} // End of namespace Sci
