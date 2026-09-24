/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/rect.h"
#include "common/system.h"
#include "common/unicode-bidi.h"
#include "gui/widgets/editable.h"
#include "gui/gui-manager.h"
#include "graphics/font.h"

namespace GUI {

EditableWidget::EditableWidget(GuiObject *boss, int x, int y, int w, int h, bool scale, const Common::U32String &tooltip, uint32 cmd)
	: Widget(boss, x, y, w, h, scale, tooltip), CommandSender(boss), _cmd(cmd) {
	// WIDGET_HOOK_DRAG: prevent hooking by a container as we need drag for ourselves
	setFlags(WIDGET_TRACK_MOUSE | WIDGET_HOOK_DRAG);
	init();
}

EditableWidget::EditableWidget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String &tooltip, uint32 cmd)
	: EditableWidget(boss, x, y, w, h, false, tooltip, cmd) {
}

EditableWidget::EditableWidget(GuiObject *boss, const Common::String &name, const Common::U32String &tooltip, uint32 cmd)
	: Widget(boss, name, tooltip), CommandSender(boss), _cmd(cmd) {
	// WIDGET_HOOK_DRAG: prevent hooking by a container as we need drag for ourselves
	setFlags(WIDGET_TRACK_MOUSE | WIDGET_HOOK_DRAG);
	init();
}

void EditableWidget::init() {
	_caretVisible = false;
	_caretTime = 0;
	_caretPos = 0;

	_caretInverse = false;

	_editScrollOffset = 0;

	_selCaretPos = -1;
	_selOffset = 0;

	_shiftPressed = _isDragging = false;
	_disableSelection = g_gui.useRTL();

	_align = g_gui.useRTL() ? Graphics::kTextAlignRight : Graphics::kTextAlignLeft;
	_drawAlign = _align;

	_font = ThemeEngine::kFontStyleBold;
	_inversion = ThemeEngine::kTextInversionNone;
}

EditableWidget::~EditableWidget() {
}

void EditableWidget::drawWidget() {
	if (_caretVisible) {
		drawCaret(false, true);
		updateImeCompositionArea();
	}
}

void EditableWidget::reflowLayout() {
	Widget::reflowLayout();

	_editScrollOffset = g_gui.getStringWidth(getDisplayedEditString(), _font) - getEditRect().width();
	if (_editScrollOffset < 0) {
		_editScrollOffset = 0;
		_drawAlign = _align;
	} else {
		_drawAlign = Graphics::kTextAlignLeft;
	}
}

void EditableWidget::setEditString(const Common::U32String &str) {
	// TODO: We probably should filter the input string here,
	// e.g. using tryInsertChar.
	cancelImeComposition();
	_editString = str;
	clearSelection();
	setCaretPos(caretVisualPos(str.size()));
	markAsDirty();
}

bool EditableWidget::isCharAllowed(Common::u32char_type_t c) const {
	return (c >= 32 && c <= 127) || c >= 160;
}

bool EditableWidget::tryInsertChar(Common::u32char_type_t c, int pos) {
	if (!isCharAllowed(c))
		return false;
	_editString.insertChar(c, pos);
	return true;
}

Common::U32String EditableWidget::getDisplayedEditString() const {
	if (!hasImeComposition())
		return _editString;

	int baseBegin;
	int baseEnd;
	getImeCompositionBaseRange(baseBegin, baseEnd);
	Common::U32String displayedText = _editString.substr(0, baseBegin);
	displayedText += _imeComposition.text;
	displayedText += _editString.substr(baseEnd);
	return displayedText;
}

int EditableWidget::getDisplayedCaretPos() const {
	if (!hasImeComposition())
		return _caretPos;

	int baseBegin;
	int baseEnd;
	getImeCompositionBaseRange(baseBegin, baseEnd);
	const Common::U32String displayedText = getDisplayedEditString();
	const int logicalPos = MAX(0, MIN(baseBegin + _imeComposition.start, static_cast<int>(displayedText.size())));
	return static_cast<int>(Common::convertBiDiU32String(displayedText + " ").getVisualPosition(logicalPos));
}

void EditableWidget::getDisplayedSelection(int &selectionBegin, int &selectionEnd) const {
	if (!hasImeComposition()) {
		selectionBegin = _selCaretPos;
		selectionEnd = _selCaretPos + _selOffset;
	} else {
		int baseBegin;
		int baseEnd;
		getImeCompositionBaseRange(baseBegin, baseEnd);
		const Common::U32String displayedText = getDisplayedEditString();
		const int logicalBegin = MAX(0, MIN(baseBegin + _imeComposition.start,
											static_cast<int>(displayedText.size())));
		const int logicalEnd = MAX(logicalBegin, MIN(logicalBegin + _imeComposition.length,
											static_cast<int>(displayedText.size())));
		const Common::UnicodeBiDiText bidi(displayedText + " ");
		selectionBegin = static_cast<int>(bidi.getVisualPosition(logicalBegin));
		selectionEnd = static_cast<int>(bidi.getVisualPosition(logicalEnd));
	}

	if (selectionBegin > selectionEnd)
		SWAP(selectionBegin, selectionEnd);

	const int displayedLength = static_cast<int>(getDisplayedEditString().size());
	selectionBegin = MAX(0, MIN(selectionBegin, displayedLength));
	selectionEnd = MAX(0, MIN(selectionEnd, displayedLength));
}

bool EditableWidget::hasImeComposition() const {
	return _imeComposition.state == Common::ImeComposition::kCompositing && !_imeComposition.text.empty();
}

void EditableWidget::getImeCompositionBaseRange(int &baseBegin, int &baseEnd) const {
	baseBegin = caretLogicalPos();
	baseEnd = baseBegin;
	if (_selCaretPos < 0 || _selOffset == 0)
		return;

	int selectionBegin = _selCaretPos;
	int selectionEnd = _selCaretPos + _selOffset;
	if (selectionBegin > selectionEnd)
		SWAP(selectionBegin, selectionEnd);

	const Common::UnicodeBiDiText bidi(_editString + " ");
	const int logicalBegin = static_cast<int>(bidi.getLogicalPosition(selectionBegin));
	const int logicalEnd = static_cast<int>(bidi.getLogicalPosition(selectionEnd));
	baseBegin = MAX(0, MIN(MIN(logicalBegin, logicalEnd), static_cast<int>(_editString.size())));
	baseEnd = MAX(baseBegin, MIN(MAX(logicalBegin, logicalEnd), static_cast<int>(_editString.size())));
}

bool EditableWidget::clearImeComposition() {
	if (!hasImeComposition())
		return false;

	_imeComposition = Common::ImeComposition();
	return true;
}

bool EditableWidget::cancelImeComposition() {
	g_system->cancelImeComposition();
	return clearImeComposition();
}

bool EditableWidget::commitImeComposition(const Common::U32String &text) {
	int baseBegin;
	int baseEnd;
	getImeCompositionBaseRange(baseBegin, baseEnd);

	Common::U32String acceptedText;
	for (uint32 i = 0; i < text.size(); i++) {
		if (isCharAllowed(text[i]))
			acceptedText.insertChar(text[i], acceptedText.size());
	}

	clearImeComposition();
	if (acceptedText.empty())
		return false;

	_editString.replace(baseBegin, baseEnd - baseBegin, acceptedText);
	clearSelection();
	setCaretPos(caretVisualPos(baseBegin + static_cast<int>(acceptedText.size())));
	sendCommand(_cmd, 0);
	return true;
}

void EditableWidget::handleImeComposition(const Common::ImeComposition &composition) {
	if (composition.state == Common::ImeComposition::kComplete) {
		commitImeComposition(composition.text);
		return;
	}

	if (composition.state != Common::ImeComposition::kCompositing || composition.text.empty()) {
		clearImeComposition();
		return;
	}

	const int compositionLength = static_cast<int>(composition.text.size());
	_imeComposition = composition;
	if (_imeComposition.start < 0)
		_imeComposition.start = compositionLength;
	_imeComposition.start = MAX(0, MIN(_imeComposition.start, compositionLength));
	if (_imeComposition.length < 0)
		_imeComposition.length = 0;
	_imeComposition.length = MIN(_imeComposition.length, compositionLength - _imeComposition.start);
}

int EditableWidget::caretVisualPos(int logicalPos) const {
	return Common::convertBiDiU32String(_editString + " ").getVisualPosition(logicalPos);
}

int EditableWidget::caretLogicalPos() const {
	return Common::convertBiDiU32String(_editString + " ").getLogicalPosition(_caretPos);
}

void EditableWidget::handleTickle() {
	uint32 time = g_system->getMillis();
	if (_caretTime < time && isEnabled()) {
		_caretTime = time + kCaretBlinkTime;
		drawCaret(_caretVisible);
	}
}

void EditableWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	if (!isEnabled())
		return;
	if (cancelImeComposition())
		markAsDirty();

	_isDragging = true;
	// Select all text incase of double press
	if (clickCount > 1) {
		_selCaretPos = 0;
		setCaretPos(caretVisualPos(_editString.size()));
		setSelectionOffset(_editString.size() - _selCaretPos);
		markAsDirty();
		return;
	}

	// Clear any selection
	if (_selOffset != 0 && !_shiftPressed)
		clearSelection();
	else if (_shiftPressed && _selCaretPos < 0 && !_disableSelection)
		_selCaretPos = _caretPos;

	if (g_gui.useRTL()) {
		x = _w - x;
	}

	x += _editScrollOffset;
	int width = 0;
	if (_drawAlign == Graphics::kTextAlignRight)
		width = _editScrollOffset + getEditRect().width() - g_gui.getStringWidth(_editString, _font);

	uint i, last = 0;
	for (i = 0; i < _editString.size(); ++i) {
		const uint cur = _editString[i];
		width += g_gui.getCharWidth(cur, _font) + g_gui.getKerningOffset(last, cur, _font);
		if (width >= x && width > _editScrollOffset)
			break;
		last = cur;
	}
	setCaretPos(i);
	if (_selCaretPos >= 0 && !_disableSelection)
		setSelectionOffset(i - _selCaretPos);
	markAsDirty();
}

void EditableWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	if(isEnabled())
		_isDragging = false;
}

void EditableWidget::handleMouseMoved(int x, int y, int button) {
	if (_isDragging && isEnabled() && !_disableSelection) {
		if (_selCaretPos < 0)
			_selCaretPos = _caretPos;

		if (g_gui.useRTL()) {
			x = _w - x;
		}

		if (x < 0 && _editScrollOffset > 0) {
			_editScrollOffset += x;
			if(_editScrollOffset < 0)
				_editScrollOffset = 0;
		}

		x += _editScrollOffset;
		int width = 0;
		if (_drawAlign == Graphics::kTextAlignRight)
			width = _editScrollOffset + getEditRect().width() - g_gui.getStringWidth(_editString, _font);
		uint i, last = 0;
		for (i = 0; i < _editString.size(); ++i) {
			const uint cur = _editString[i];
			width += g_gui.getCharWidth(cur, _font) + g_gui.getKerningOffset(last, cur, _font);
			if (width >= x && width > _editScrollOffset)
				break;
			last = cur;
		}

		setCaretPos(i);
		if(_selCaretPos >= 0)
			setSelectionOffset(i - _selCaretPos);
		markAsDirty();
	}
}

bool EditableWidget::handleKeyUp(Common::KeyState state) {
	_shiftPressed = state.hasFlags(Common::KBD_SHIFT);
	return false;
}

bool EditableWidget::handleKeyDown(Common::KeyState state) {
	bool handled = true;
	bool dirty = false;
	bool forcecaret = false;
	int deleteIndex;

	if (!isEnabled())
		return false;
	if (hasImeComposition()) {
		// The native IME owns key interpretation until it completes or cancels
		// the composition. Raw key events must not modify the committed text.
		_shiftPressed = state.hasFlags(Common::KBD_SHIFT);
		return true;
	}

	// First remove caret
	if (_caretVisible)
		drawCaret(true);

	_shiftPressed = state.hasFlags(Common::KBD_SHIFT);

	// Remap numeric keypad if NUM lock is *not* active.
	// This code relies on the fact that the various KEYCODE_KP* values are
	// consecutive.
	if (0 == (state.flags & Common::KBD_NUM)
		&& Common::KEYCODE_KP0 <= state.keycode
		&& state.keycode <= Common::KEYCODE_KP_PERIOD) {
		const Common::KeyCode remap[11] = {
			Common::KEYCODE_INSERT, 	// KEYCODE_KP0
			Common::KEYCODE_END,	 	// KEYCODE_KP1
			Common::KEYCODE_DOWN, 		// KEYCODE_KP2
			Common::KEYCODE_PAGEDOWN, 	// KEYCODE_KP3
			Common::KEYCODE_LEFT, 		// KEYCODE_KP4
			Common::KEYCODE_INVALID, 	// KEYCODE_KP5
			Common::KEYCODE_RIGHT,	 	// KEYCODE_KP6
			Common::KEYCODE_HOME,	 	// KEYCODE_KP7
			Common::KEYCODE_UP, 		// KEYCODE_KP8
			Common::KEYCODE_PAGEUP, 	// KEYCODE_KP9
			Common::KEYCODE_DELETE,	 	// KEYCODE_KP_PERIOD
		};
		state.keycode = remap[state.keycode - Common::KEYCODE_KP0];
	}

	switch (state.keycode) {
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		// confirm edit and exit editmode
		endEditMode();
		dirty = true;
		break;

	case Common::KEYCODE_ESCAPE:
		abortEditMode();
		dirty = true;
		break;

	case Common::KEYCODE_BACKSPACE:
		deleteIndex = caretLogicalPos();
		if (deleteIndex > 0 && _selOffset == 0) {
			deleteIndex--;
			_editString.deleteChar(deleteIndex);
			setCaretPos(caretVisualPos(deleteIndex));
			_selCaretPos = -1;
			dirty = true;

			sendCommand(_cmd, 0);
		} else if (deleteIndex >= 0 && _selOffset != 0) {
			int selBegin = _selCaretPos;
			int selEnd = _selCaretPos + _selOffset;
			if (selBegin > selEnd)
				SWAP(selBegin, selEnd);
			_editString.erase(selBegin, selEnd - selBegin);
			setCaretPos(caretVisualPos(selBegin));
			_selCaretPos = -1;
			_selOffset = 0;
			dirty = true;

			sendCommand(_cmd, 0);
		}
		forcecaret = true;
		break;

	case Common::KEYCODE_DELETE:
		deleteIndex = caretLogicalPos();
		if (deleteIndex < (int)_editString.size()) {
			_editString.deleteChar(deleteIndex);
			setCaretPos(caretVisualPos(deleteIndex));
			_selCaretPos = -1;
			_selOffset = 0;
			dirty = true;

			sendCommand(_cmd, 0);
		}
		forcecaret = true;
		break;

	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_END:
		moveCaretToEnd(state.hasFlags(Common::KBD_SHIFT));
		forcecaret = true;
		dirty = true;
		break;

	case Common::KEYCODE_LEFT:
		if (state.hasFlags(Common::KBD_SHIFT)) {
			if (_disableSelection)
				break;
			if (_selCaretPos < 0)
				_selCaretPos = _caretPos;
			if (_caretPos > 0)
				_selOffset--;
		} else {
			clearSelection();
		}
		// Move caret one left (if possible)
		if (_caretPos > 0) {
			dirty = setCaretPos(_caretPos - 1);
		}
		forcecaret = true;
		dirty = true;
		break;

	case Common::KEYCODE_RIGHT:
		if (state.hasFlags(Common::KBD_SHIFT)) {
			if (_disableSelection)
				break;
			if (_selCaretPos < 0)
				_selCaretPos = _caretPos;
			if (_selOffset + _selCaretPos < (int)_editString.size())
				_selOffset++;
		} else {
			clearSelection();
		}
		// Move caret one right (if possible)
		if (_caretPos < (int)_editString.size()) {
			dirty = setCaretPos(_caretPos + 1);
		}
		forcecaret = true;
		dirty = true;
		break;

	case Common::KEYCODE_UP:
	case Common::KEYCODE_HOME:
		moveCaretToStart(state.hasFlags(Common::KBD_SHIFT));
		forcecaret = true;
		dirty = true;
		break;

	default:
		defaultKeyDownHandler(state, dirty, forcecaret, handled);
	}

	if (dirty)
		markAsDirty();

	if (forcecaret)
		makeCaretVisible();

	return handled;
}

void EditableWidget::defaultKeyDownHandler(Common::KeyState &state, bool &dirty, bool &forcecaret, bool &handled) {
	if (isCharAllowed(state.ascii)) {
		// Incase of a selection, replace the selection with the character
		if (_selCaretPos >= 0) {
			int selBegin = _selCaretPos;
			int selEnd = _selCaretPos + _selOffset;
			if (selBegin > selEnd)
				SWAP(selBegin, selEnd);
			_editString.replace(selBegin, selEnd - selBegin, Common::U32String(state.ascii));
			if (_editString.size() > 0)
				selBegin++;
			setCaretPos(caretVisualPos(selBegin));
			_selCaretPos = -1;
			_selOffset = 0;
		} else {
			// Insert char normally at caretPos
			const int logicalPosition = caretLogicalPos();
			_editString.insertChar(state.ascii, logicalPosition);
			setCaretPos(caretVisualPos(logicalPosition + 1));
		}
		dirty = true;
		forcecaret = true;
		sendCommand(_cmd, 0);
	} else {
		handled = false;
	}
}

void EditableWidget::moveCaretToStart(bool shiftPressed) {
	if (shiftPressed) {
		if (_selCaretPos < 0)
			_selCaretPos = _caretPos;

		setSelectionOffset(0 - _selCaretPos);
	} else {
		clearSelection();
	}
	// Move caret to start
	setCaretPos(caretVisualPos(0));
}

void EditableWidget::moveCaretToEnd(bool shiftPressed) {
	if (_shiftPressed) {
		if (_selCaretPos < 0)
			_selCaretPos = _caretPos;

		setSelectionOffset(_editString.size() - _selCaretPos);
	} else {
		clearSelection();
	}

	// Move caret to end
	setCaretPos(caretVisualPos(_editString.size()));
}

void EditableWidget::handleOtherEvent(const Common::Event &evt) {
	bool dirty = false;
	bool forcecaret = false;

	if (!isEnabled())
		return;

	// First remove caret
	if (_caretVisible)
		drawCaret(true);

	switch (evt.type) {
	case Common::EVENT_IME_COMPOSITION:
		handleImeComposition(evt.imeComposition);
		dirty = true;
		forcecaret = true;
		break;
	case Common::EVENT_FOCUS_LOST:
		dirty = clearImeComposition();
		break;
	case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
		switch (evt.customType) {
		case kActionHome:
			if (cancelImeComposition())
				dirty = true;
			moveCaretToStart(false);
			forcecaret = true;
			dirty = true;
			break;
		case kActionShiftHome:
			if (cancelImeComposition())
				dirty = true;
			moveCaretToStart(true);
			forcecaret = true;
			dirty = true;
			break;
		case kActionEnd:
			if (cancelImeComposition())
				dirty = true;
			moveCaretToEnd(false);
			forcecaret = true;
			dirty = true;
			break;
		case kActionShiftEnd:
			if (cancelImeComposition())
				dirty = true;
			moveCaretToEnd(true);
			forcecaret = true;
			dirty = true;
			break;
		case kActionCut:
			if (!getEditString().empty() && _selOffset != 0) {
				if (cancelImeComposition())
					dirty = true;
				int selBegin = _selCaretPos;
				int selEnd = _selCaretPos + _selOffset;
				if (selBegin > selEnd)
					SWAP(selBegin, selEnd);
				const Common::U32String selected(getEditString().begin() + selBegin, getEditString().begin() + selEnd);
				g_system->setTextInClipboard(selected);

				_editString.erase(selBegin, selEnd - selBegin);
				setCaretPos(caretVisualPos(selBegin));
				_selCaretPos = -1;
				_selOffset = 0;
				dirty = true;
			}
			break;

		case kActionCopy:
			if (!getEditString().empty()) {
				int selBegin = _selCaretPos;
				int selEnd = _selCaretPos + _selOffset;
				if (selBegin > selEnd)
					SWAP(selBegin, selEnd);
				const Common::U32String selected(getEditString().begin() + selBegin, getEditString().begin() + selEnd);
				g_system->setTextInClipboard(selected);
			}
			break;

		case kActionPaste:
			if (g_system->hasTextInClipboard()) {
				if (cancelImeComposition())
					dirty = true;
				Common::U32String text = g_system->getTextFromClipboard();
				if (_selOffset != 0) {
					int selBegin = _selCaretPos;
					int selEnd = _selCaretPos + _selOffset;
					if (selBegin > selEnd)
						SWAP(selBegin, selEnd);
					_editString.replace(selBegin, selEnd - selBegin, text);
					setCaretPos(caretVisualPos(selBegin));
					const int logicalPosition = caretLogicalPos();
					setCaretPos(caretVisualPos(logicalPosition + text.size()));
					clearSelection();
				} else {
					for (uint32 i = 0; i < text.size(); ++i) {
						const int logicalPosition = caretLogicalPos();
						if (tryInsertChar(text[i], logicalPosition))
								setCaretPos(caretVisualPos(logicalPosition + 1));
					}
				}
				dirty = true;
			}
			break;
		default:
			break;
	}
	default:
		break;
	}

	if (dirty)
		markAsDirty();

	if (forcecaret)
		makeCaretVisible();
}

int EditableWidget::getCaretOffset() const {
	const Common::U32String displayedText = getDisplayedEditString();
	const Common::UnicodeBiDiText utxt(displayedText);
	const int caretPos = getDisplayedCaretPos();
	Common::U32String substr(utxt.visual.begin(), utxt.visual.begin() + caretPos);
	return g_gui.getStringWidth(substr, _font) - _editScrollOffset;
}

int EditableWidget::getSelectionCarretOffset() const {
	const Common::U32String displayedText = getDisplayedEditString();
	const Common::UnicodeBiDiText utxt(displayedText);
	int selectionBegin;
	int selectionEnd;
	getDisplayedSelection(selectionBegin, selectionEnd);
	Common::U32String substr(utxt.visual.begin(), utxt.visual.begin() + selectionBegin);
	return g_gui.getStringWidth(substr, _font) - _editScrollOffset;
}

Common::Rect EditableWidget::getCaretRect(bool useRelativeCoordinates) {
	// Only calculate the caret area if the widget is visible.
	if (!isVisible() || !_boss->isVisible())
		return Common::Rect();

	Common::Rect editRect = getEditRect();

	int xOff;
	int yOff;

	if (useRelativeCoordinates) {
		xOff = getRelX();
		yOff = getRelY();
	} else {
		xOff = getAbsX();
		yOff = getAbsY();
	}

	int x = editRect.left;
	int y = editRect.top;

	const Common::U32String displayedText = getDisplayedEditString();

	if (_align == Graphics::kTextAlignRight) {
		int strVisibleWidth = g_gui.getStringWidth(displayedText, _font) - _editScrollOffset;
		if (strVisibleWidth > editRect.width()) {
			_drawAlign = Graphics::kTextAlignLeft;
			strVisibleWidth = editRect.width();
		} else {
			_drawAlign = _align;
		}
		x = editRect.right - strVisibleWidth;
	}

	const int caretOffset = getCaretOffset();
	x += caretOffset;

	if (y < 0 || y + editRect.height() > _h)
		return Common::Rect();

	if (g_gui.useRTL())
		x += g_system->getOverlayWidth() - _w - xOff;
	else
		x += xOff;
	y += yOff;

	return Common::Rect(x, y, x + 1, y + editRect.height());
}

void EditableWidget::updateImeCompositionArea() {
	const Common::Rect caretRect = getCaretRect();
	if (!caretRect.isEmpty())
		g_system->setImeCompositionArea(caretRect);
}

void EditableWidget::drawCaret(bool erase, bool useRelativeCoordinates) {
	const Common::Rect caretRect = getCaretRect(useRelativeCoordinates);
	if (caretRect.isEmpty())
		return;

	const Common::Rect editRect = getEditRect();
	const Common::U32String displayedText = getDisplayedEditString();
	const int caretPos = getDisplayedCaretPos();
	const int caretOffset = getCaretOffset();
	int x = caretRect.left;
	const int y = caretRect.top;

	if (!erase && !useRelativeCoordinates)
		g_system->setImeCompositionArea(caretRect);

	g_gui.theme()->drawCaret(caretRect, erase);

	if (erase) {
		Common::U32String character;
		int width;

		if ((uint)caretPos < displayedText.size()) {
			Common::UnicodeBiDiText utxt(displayedText);
			const Common::u32char_type_t chr = utxt.visual[caretPos];
			width = g_gui.getCharWidth(chr, _font);
			character = Common::U32String(chr);

			const uint32 last = (caretPos > 0) ?  utxt.visual[caretPos - 1] : 0;
			x += g_gui.getKerningOffset(last, chr, _font);
		} else {
			// We draw a fake space here to assure that removing the caret
			// does not result in color glitches in case the edit rect is
			// drawn with an inversion.
			width = g_gui.getCharWidth(' ', _font);
			character = " ";
		}

		// TODO: Right now we manually prevent text from being drawn outside
		// the edit area here. We might want to consider to use
		// setTextDrawableArea for this. However, it seems that only
		// EditTextWidget uses that but not ListWidget. Thus, one should check
		// whether we can unify the drawing in the text area first to avoid
		// possible glitches due to different methods used.

		ThemeEngine::TextInversionState inversion = _inversion;

		if (!_disableSelection)
			inversion = (_selOffset < 0) ? ThemeEngine::kTextInversionFocus : ThemeEngine::kTextInversionNone;

		width = MIN(editRect.width() - caretOffset, width);
		if (width > 0) {
			g_gui.theme()->drawText(Common::Rect(x, y, x + width, y + editRect.height()), character,
			                        _state, _drawAlign, inversion, 0, false, _font,
			                        ThemeEngine::kFontColorNormal, true, _textDrawableArea);
		}
	}

	_caretVisible = !erase;
}

bool EditableWidget::setCaretPos(int newPos) {
	assert(newPos >= 0 && newPos <= (int)_editString.size());
	_caretPos = newPos;
	return adjustOffset();
}

bool EditableWidget::adjustOffset() {
	// check if the caret is still within the textbox; if it isn't,
	// adjust _editScrollOffset

	int caretpos = getCaretOffset();
	const int editWidth = getEditRect().width();

	if (caretpos < 0) {
		// scroll left
		_editScrollOffset += caretpos;
		return true;
	} else if (caretpos >= editWidth) {
		// scroll right
		_editScrollOffset -= (editWidth - caretpos);
		return true;
	} else if (_editScrollOffset > 0) {
		const int strWidth = g_gui.getStringWidth(getDisplayedEditString(), _font);
		if (strWidth - _editScrollOffset < editWidth) {
			// scroll right
			_editScrollOffset = (strWidth - editWidth);
			if (_editScrollOffset < 0)
				_editScrollOffset = 0;
		}
	}

	return false;
}

void EditableWidget::makeCaretVisible() {
	_caretTime = g_system->getMillis() + kCaretBlinkTime;
	_caretVisible = true;
	drawCaret(false);
}

void EditableWidget::clearSelection() {
	_selCaretPos = -1;
	_selOffset = 0;
	markAsDirty();
}

void EditableWidget::setSelectionOffset(int newOffset) {
	_selOffset = newOffset;
}

} // End of namespace GUI
