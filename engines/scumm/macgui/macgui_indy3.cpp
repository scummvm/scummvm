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

#include "common/system.h"
#include "common/config-manager.h"
#include "common/macresman.h"

#include "graphics/macgui/macwindowmanager.h"

#include "scumm/scumm.h"
#include "scumm/scumm_v4.h"
#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/dialogs.h"
#include "scumm/macgui/macgui_impl.h"
#include "scumm/macgui/macgui_indy3.h"
#include "scumm/music.h"
#include "scumm/sound.h"
#include "scumm/verbs.h"

namespace Scumm {

// ===========================================================================
// The Mac GUI for Indiana Jones and the Last Crusade, including the infamous
// verb GUI.
//
// It's likely that the original interpreter used more hooks from the engine
// into the GUI. In particular, the inventory script uses a variable that I
// haven't been able to find in any of the early scripts of the game.
//
// But the design goal here is to keep things as simple as possible, even if
// that means using brute force. So the GUI figures out which verbs are active
// by scanning all the verbs, and which objects are in the inventory by
// scanning all objects.
//
// We used to coerce the Mac verb GUI into something that looked like the
// GUI from all other versions. This used a number of variables and hard-coded
// verbs. The only thing still used of all this is variable 67, to keep track
// of the inventory scroll position. The fake verbs are removed when loading
// old savegames, but the variables are assumed to be harmless.
// ===========================================================================

#define WIDGET_TIMER_JIFFIES	12
#define REPEAT_TIMER_JIFFIES	18

// ---------------------------------------------------------------------------
// The basic building block of the GUI is the Widget. It has a bounding box, a
// timer, a couple of drawing primitives, etc.
// ---------------------------------------------------------------------------

ScummEngine *MacIndy3Gui::Widget::_vm = nullptr;
Graphics::Surface *MacIndy3Gui::Widget::_surface = nullptr;
MacIndy3Gui *MacIndy3Gui::Widget::_gui = nullptr;

MacIndy3Gui::Widget::Widget(int x, int y, int width, int height) : MacGuiObject(Common::Rect(x, y, x + width, y + height), false) {
}

void MacIndy3Gui::Widget::reset() {
	clearTimer();
	_enabled = false;
	setRedraw(false);
}

void MacIndy3Gui::Widget::updateTimer(int delta) {
	if (hasTimer()) {
		if (delta > _timer)
			delta = _timer;

		_timer -= delta;

		if (_timer == 0)
			timeOut();
	}
}

void MacIndy3Gui::Widget::draw() {
	markScreenAsDirty(_bounds);
	_redraw = false;
}

void MacIndy3Gui::Widget::undraw() {
	fill(_bounds);
	markScreenAsDirty(_bounds);
	_redraw = false;
}

void MacIndy3Gui::Widget::markScreenAsDirty(Common::Rect r) const {
	_gui->markScreenAsDirty(r);
}

byte MacIndy3Gui::Widget::translateChar(byte c) const {
	if (c == '^')
		return 0xC9;
	return c;
}

void MacIndy3Gui::Widget::fill(Common::Rect r) {
	_gui->fill(r);
}

void MacIndy3Gui::Widget::drawBitmap(Common::Rect r, const uint16 *bitmap, byte color) const {
	_gui->drawBitmap(_surface, r, bitmap, color);
}

// The shadow box is the basic outline of the verb buttons and the inventory
// widget. A slightly rounded rectangle with a shadow and a highlight.

void MacIndy3Gui::Widget::drawShadowBox(Common::Rect r) const {
	_surface->hLine(r.left + 1, r.top, r.right - 3, kBlack);
	_surface->hLine(r.left + 1, r.bottom - 2, r.right - 3, kBlack);
	_surface->vLine(r.left, r.top + 1, r.bottom - 3, kBlack);
	_surface->vLine(r.right - 2, r.top + 1, r.bottom - 3, kBlack);

	_surface->hLine(r.left + 2, r.bottom - 1, r.right - 1, kBlack);
	_surface->vLine(r.right - 1, r.top + 2, r.bottom - 2, kBlack);

	_surface->hLine(r.left + 1, r.top + 1, r.right - 3, kWhite);
	_surface->vLine(r.left + 1, r.top + 2, r.bottom - 3, kWhite);
}

// The shadow frame is a rectangle with a highlight. It can be filled or
// unfilled.

void MacIndy3Gui::Widget::drawShadowFrame(Common::Rect r, byte shadowColor, byte fillColor) {
	_surface->hLine(r.left, r.top, r.right - 1, kBlack);
	_surface->hLine(r.left, r.bottom - 1, r.right - 1, kBlack);
	_surface->vLine(r.left, r.top + 1, r.bottom - 2, kBlack);
	_surface->vLine(r.right - 1, r.top + 1, r.bottom - 2, kBlack);

	_surface->hLine(r.left + 1, r.top + 1, r.right - 2, shadowColor);
	_surface->vLine(r.left + 1, r.top + 2, r.bottom - 2, shadowColor);

	if (fillColor != kTransparency) {
		Common::Rect fillRect(r.left + 2, r.top + 2, r.right - 1, r.bottom - 1);

		if (fillColor == kBackground)
			fill(fillRect);
		else
			_surface->fillRect(fillRect, fillColor);
	}
}

// ---------------------------------------------------------------------------
// The VerbWidget is what the user interacts with. Each one is connected to a
// verb id and slot. A VerbWidget can consist of several Widgets.
// ---------------------------------------------------------------------------

void MacIndy3Gui::VerbWidget::reset() {
	MacIndy3Gui::Widget::reset();
	_verbslot = -1;
	_visible = false;
	_kill = false;
}

void MacIndy3Gui::VerbWidget::updateVerb(int verbslot) {
	VerbSlot *vs = &_vm->_verbs[verbslot];
	bool enabled = (vs->curmode == 1);

	if (!_visible || _enabled != enabled)
		setRedraw(true);

	_verbslot = verbslot;
	_verbid = vs->verbid;
	_enabled = enabled;
	_kill = false;
}

void MacIndy3Gui::VerbWidget::draw() {
	Widget::draw();
	_visible = true;
}

void MacIndy3Gui::VerbWidget::undraw() {
	debug(1, "VerbWidget: Undrawing [%d]", _verbid);

	Widget::undraw();
	_visible = false;
}

// ---------------------------------------------------------------------------
// The most common type of VerbWidget is the button, which is used for verbs
// and conversation options.
// ---------------------------------------------------------------------------

MacIndy3Gui::Button::Button(int x, int y, int width, int height) : VerbWidget(x, y, width, height) {
}

void MacIndy3Gui::Button::reset() {
	MacIndy3Gui::VerbWidget::reset();
	_text.clear();
}

bool MacIndy3Gui::Button::handleEvent(Common::Event &event) {
	if (!_enabled || !_verbid)
		return false;

	VerbSlot *vs = &_vm->_verbs[_verbslot];

	if (vs->saveid)
		return false;

	bool caughtEvent = false;

	if (event.type == Common::EVENT_KEYDOWN) {
		if (!(event.kbd.flags & (Common::KBD_CTRL | Common::KBD_ALT | Common::KBD_META)) && event.kbd.keycode == vs->key)
			caughtEvent = true;
	} else if (event.type == Common::EVENT_LBUTTONDOWN) {
		if (_bounds.contains(event.mouse))
			caughtEvent = true;
	}

	// Events are handled at the end of the animation. This blatant attack
	// on speedrunners all over the world was done because that's what the
	// original seems to do, and it looks better. Based on tests in Mac
	// emulators, the speed of the animation depended on the speed of your
	// computer, so we just set something that looks good here.

	if (caughtEvent) {
		setRedraw(true);
		setTimer(WIDGET_TIMER_JIFFIES);
	}

	return caughtEvent;
}

void MacIndy3Gui::Button::timeOut() {
	if (_visible) {
		_vm->runInputScript(kVerbClickArea, _verbid, 1);
		setRedraw(true);
	}
}

void MacIndy3Gui::Button::updateVerb(int verbslot) {
	MacIndy3Gui::VerbWidget::updateVerb(verbslot);

	const byte *ptr = _vm->getResourceAddress(rtVerb, verbslot);
	byte buf[270];

	_vm->convertMessageToString(ptr, buf, sizeof(buf));
	if (_text != (char *)buf) {
		_text = (char *)buf;
		clearTimer();
		setRedraw(true);
	}
}

void MacIndy3Gui::Button::draw() {
	if (!getRedraw())
		return;

	debug(1, "Button: Drawing [%d] %s", _verbid, _text.c_str());

	MacIndy3Gui::VerbWidget::draw();
	fill(_bounds);

	if (!hasTimer()) {
		drawShadowBox(_bounds);
	} else {
		// I have only been able to capture a screenshot of the pressed
		// button in black and white, where the checkerboard background
		// makes it hard to see exactly which pixels should be drawn.
		// Basilisk II runs it too fast, and I haven't gotten Mini vMac
		// to run it in 16-color mode.
		//
		// All I can say for certain is that the upper left corner is
		// rounded while the lower right is not. I'm going to assume
		// that the shadow is always drawn, and the rest of the button
		// is just shifted down to the right. That would make the other
		// two corners rounded, so only the lower right one isn't.

		int x0 = _bounds.left + 1;
		int x1 = _bounds.right - 1;
		int y0 = _bounds.top + 1;
		int y1 = _bounds.bottom - 1;

		_surface->hLine(x0 + 1, y0, x1 - 1, kBlack);
		_surface->hLine(x0 + 1, y1, x1, kBlack);
		_surface->vLine(x0, y0 + 1, y1 - 1, kBlack);
		_surface->vLine(x1, y0 + 1, y1 - 1, kBlack);

		_surface->hLine(x0 + 1, y0 + 1, x1 - 1, kWhite);
		_surface->vLine(x0 + 1, y0 + 2, y1 - 1, kWhite);
	}

	// The text is drawn centered. Based on experimentation, I think the
	// width is always based on the outlined font, and the button shadow is
	// not counted as part of the button width.
	//
	// This gives us pixel perfect rendering for the English verbs.

	if (!_text.empty()) {
		const Graphics::Font *boldFont = _gui->getFont(kIndy3VerbFontBold);
		const Graphics::Font *outlineFont = _gui->getFont(kIndy3VerbFontOutline);

		int stringWidth = 0;
		for (uint i = 0; i < _text.size(); i++)
			stringWidth += outlineFont->getCharWidth(_text[i]);

		int x = (_bounds.left + (_bounds.width() - 1 - stringWidth) / 2) - 1;
		int y = _bounds.top + 2;
		byte color = _enabled ? kWhite : kBlack;

		if (hasTimer()) {
			x++;
			y++;
		}

		for (uint i = 0; i < _text.size() && x < _bounds.right; i++) {
			byte c = translateChar(_text[i]);
			if (x >= _bounds.left) {
				if (_enabled)
					outlineFont->drawChar(_surface, c, x, y, kBlack);
				boldFont->drawChar(_surface, c, x + 1, y, color);
			}
			x += boldFont->getCharWidth(c);
		}
	}
}

// ---------------------------------------------------------------------------
// Unlike in the DOS version, where each inventory object shown on screen is
// its own verb, in the Macintosh version the entire Inventory widget is one
// single verb. It consists of the widget itself, the inventory slots, and
// the scrollbar.
// ---------------------------------------------------------------------------

MacIndy3Gui::Inventory::Inventory(int x, int y, int width, int height) : MacIndy3Gui::VerbWidget(x, y, width, height) {
	x = _bounds.left + 6;
	y = _bounds.top + 6;

	// There are always six slots, no matter how many objects you are
	// carrying.
	//
	// Each slot is 12 pixels tall (as seen when they are highlighted),
	// which means they have to overlap slightly to fit. It is assumed
	// that this will never interfere with the text drawing.

	for (int i = 0; i < ARRAYSIZE(_slots); i++) {
		_slots[i] = new Slot(i, x, y, 128, 12);
		y += 11;
	}

	x = _bounds.right - 20;

	_scrollBar = new ScrollBar(x, _bounds.top + 19, 16, 40);

	_scrollButtons[0] = new ScrollButton(x, _bounds.top + 4, 16, 16, kScrollUp);
	_scrollButtons[1] = new ScrollButton(x, _bounds.bottom - 20, 16, 16, kScrollDown);
}

MacIndy3Gui::Inventory::~Inventory() {
	for (int i = 0; i < ARRAYSIZE(_slots); i++)
		delete _slots[i];

	delete _scrollBar;

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++)
		delete _scrollButtons[i];
}

void MacIndy3Gui::Inventory::reset() {
	MacIndy3Gui::VerbWidget::reset();

	for (int i = 0; i < ARRAYSIZE(_slots); i++)
		_slots[i]->reset();

	_scrollBar->reset();

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++)
		_scrollButtons[i]->reset();
}

void MacIndy3Gui::Inventory::setRedraw(bool redraw) {
	MacIndy3Gui::Widget::setRedraw(redraw);

	for (int i = 0; i < ARRAYSIZE(_slots); i++)
		_slots[i]->setRedraw(redraw);

	_scrollBar->setRedraw(redraw);

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++)
		_scrollButtons[i]->setRedraw(redraw);
}

bool MacIndy3Gui::Inventory::handleEvent(Common::Event &event) {
	if (!_enabled || !_verbid)
		return false;

	if (_vm->enhancementEnabled(kEnhUIUX)) {
		if ((event.type == Common::EVENT_WHEELUP || event.type == Common::EVENT_WHEELDOWN) && _bounds.contains(event.mouse.x, event.mouse.y)) {
			if (event.type == Common::EVENT_WHEELUP) {
				_scrollBar->scroll(kScrollUp);
			} else {
				_scrollBar->scroll(kScrollDown);
			}
			return true;
		}
	}

	if (event.type != Common::EVENT_LBUTTONDOWN)
		return false;

	for (int i = 0; i < ARRAYSIZE(_slots); i++) {
		if (_slots[i]->handleEvent(event))
			return true;
	}

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++) {
		ScrollButton *b = _scrollButtons[i];

		// Scrolling is done by one object at a time, though you can
		// hold down the mouse button to repeat.

		if (b->handleEvent(event)) {
			_scrollBar->scroll(b->_direction);
			return true;
		}
	}

	if (_scrollBar->handleEvent(event))
		return true;

	return false;
}

bool MacIndy3Gui::Inventory::handleMouseHeld(Common::Point &pressed, Common::Point &held) {
	if (!_enabled || !_verbid)
		return false;

	for (int i = 0; i < ARRAYSIZE(_slots); i++) {
		if (_slots[i]->handleMouseHeld(pressed, held))
			return true;
	}

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++) {
		ScrollButton *b = _scrollButtons[i];

		if (b->handleMouseHeld(pressed, held)) {
			_scrollBar->scroll(b->_direction);
			return true;
		}
	}

	// It would be possible to handle dragging the scrollbar handle, but
	// the original didn't.

	if (_scrollBar->handleMouseHeld(pressed, held))
		return true;

	return false;
}

void MacIndy3Gui::Inventory::updateTimer(int delta) {
	Widget::updateTimer(delta);

	for (int i = 0; i < ARRAYSIZE(_slots); i++)
		_slots[i]->updateTimer(delta);

	_scrollBar->updateTimer(delta);

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++)
		_scrollButtons[i]->updateTimer(delta);
}

void MacIndy3Gui::Inventory::updateVerb(int verbslot) {
	MacIndy3Gui::VerbWidget::updateVerb(verbslot);

	int owner = _vm->VAR(_vm->VAR_EGO);

	int invCount = _vm->getInventoryCount(owner);
	int invOffset = _gui->getInventoryScrollOffset();
	int numSlots = ARRAYSIZE(_slots);

	// The scroll offset must be non-negative and if there are numSlots or
	// less objects in the inventory, the inventory is fixed in the top
	// position.

	if (invOffset < 0 || invCount <= numSlots)
		invOffset = 0;

	// If there are more than numSlots objects in the inventory, clamp the
	// scroll offset so that the inventory does not go past the last
	// numSlots objets.

	if (invCount > numSlots && invOffset > invCount - numSlots)
		invOffset = invCount - numSlots;

	_scrollButtons[0]->setEnabled(invOffset > 0);
	_scrollButtons[1]->setEnabled(invCount > numSlots && invOffset < invCount - numSlots);

	_scrollBar->setEnabled(invCount > numSlots);
	_scrollBar->setInventoryParameters(invCount, invOffset);

	_gui->setInventoryScrollOffset(invOffset);

	int invSlot = 0;

	// Assign the objects to the inventory slots

	for (int i = 0; i < _vm->_numInventory && invSlot < numSlots; i++) {
		int obj = _vm->_inventory[i];
		if (obj && _vm->getOwner(obj) == owner) {
			if (--invOffset < 0) {
				_slots[invSlot]->setObject(obj);
				invSlot++;
			}
		}
	}

	// Clear the remaining slots

	for (int i = invSlot; i < numSlots; i++)
		_slots[i]->clearObject();
}

void MacIndy3Gui::Inventory::draw() {
	if (getRedraw()) {
		debug(1, "Inventory: Drawing [%d]", _verbid);

		MacIndy3Gui::VerbWidget::draw();

		drawShadowBox(_bounds);
		drawShadowFrame(Common::Rect(_bounds.left + 4, _bounds.top + 4, _bounds.right - 22, _bounds.bottom - 4), kBlack, kWhite);

		const uint16 upArrow[] = {
			0x0000, 0x0000, 0x0000, 0x0080,
			0x0140, 0x0220, 0x0410, 0x0808,
			0x1C1C, 0x0410, 0x0410, 0x0410,
			0x07F0, 0x0000, 0x0000, 0x0000
		};

		const uint16 downArrow[] = {
			0x0000, 0x0000, 0x0000, 0x0000,
			0x07F0, 0x0410, 0x0410, 0x0410,
			0x1C1C, 0x0808, 0x0410, 0x0220,
			0x0140, 0x0080, 0x0000, 0x0000
		};

		for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++) {
			ScrollButton *s = _scrollButtons[i];
			const uint16 *arrow = (s->_direction == kScrollUp) ? upArrow : downArrow;

			drawShadowFrame(s->getBounds(), kWhite, kTransparency);
			drawBitmap(s->getBounds(), arrow, kBlack);
			s->draw();
		}
	}

	// Since the inventory slots overlap, draw the highlighted ones (and
	// there should really only be one at a time) last.

	for (int i = 0; i < ARRAYSIZE(_slots); i++) {
		if (!_slots[i]->hasTimer())
			_slots[i]->draw();
	}

	for (int i = 0; i < ARRAYSIZE(_slots); i++) {
		if (_slots[i]->hasTimer())
			_slots[i]->draw();
	}

	_scrollBar->draw();

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++)
		_scrollButtons[i]->draw();
}

// ---------------------------------------------------------------------------
// Inventory::Slot is a widget for a single inventory object.
// ---------------------------------------------------------------------------

MacIndy3Gui::Inventory::Slot::Slot(int slot, int x, int y, int width, int height) : MacIndy3Gui::Widget(x, y, width, height) {
	_slot = slot;
}

void MacIndy3Gui::Inventory::Slot::reset() {
	Widget::reset();
	clearName();
	clearObject();
}

void MacIndy3Gui::Inventory::Slot::clearObject() {
	_obj = -1;
	setEnabled(false);

	if (hasName()) {
		clearName();
		setRedraw(true);
	}
}

void MacIndy3Gui::Inventory::Slot::setObject(int obj) {
	_obj = obj;

	const byte *ptr = _vm->getObjOrActorName(obj);

	if (ptr) {
		byte buf[270];
		_vm->convertMessageToString(ptr, buf, sizeof(buf));

		if (_name != (const char *)buf) {
			setEnabled(true);
			_name = (const char *)buf;
			clearTimer();
			setRedraw(true);
		}
	} else if (hasName()) {
		setEnabled(false);
		clearName();
		clearTimer();
		setRedraw(true);
	}
}

bool MacIndy3Gui::Inventory::Slot::handleEvent(Common::Event &event) {
	if (!_enabled || event.type != Common::EVENT_LBUTTONDOWN)
		return false;

	if (_bounds.contains(event.mouse)) {
		setRedraw(true);
		if (hasTimer())
			timeOut();
		setTimer(WIDGET_TIMER_JIFFIES);
		return true;
	}

	return false;
}

void MacIndy3Gui::Inventory::Slot::timeOut() {
	_vm->runInputScript(kInventoryClickArea, getObject(), 1);
	setRedraw(true);
}

void MacIndy3Gui::Inventory::Slot::draw() {
	if (!getRedraw())
		return;

	debug(1, "Inventory::Slot: Drawing [%d] %s", _slot, _name.c_str());

	Widget::draw();

	byte fg, bg;

	if (hasTimer()) {
		fg = kWhite;
		bg = kBlack;
	} else {
		fg = kBlack;
		bg = kWhite;
	}

	_surface->fillRect(_bounds, bg);

	if (hasName()) {
		const Graphics::Font *font = _gui->getFont(kIndy3VerbFontRegular);

		int y = _bounds.top - 1;
		int x = _bounds.left + 4;

		for (uint i = 0; i < _name.size() && x < _bounds.right; i++) {
			byte c = translateChar(_name[i]);

			font->drawChar(_surface, c, x, y, fg);
			x += font->getCharWidth(c);
		}
	}
}

// ---------------------------------------------------------------------------
// Inventory::ScrollBar is the slider which shows if the inventory contains
// more objects than are visible on screen.
// ---------------------------------------------------------------------------

// NB: This class makes several references to ARRAYSIZE(_slots), but accessing
//     members of the enclosing class like that should be ok in C++11.

MacIndy3Gui::Inventory::ScrollBar::ScrollBar(int x, int y, int width, int height) : MacIndy3Gui::Widget(x, y, width, height) {
}

bool MacIndy3Gui::Inventory::ScrollBar::handleEvent(Common::Event &event) {
	if (!_enabled || event.type != Common::EVENT_LBUTTONDOWN)
		return false;

	// Clicking on the scrollbar scrolls it to the top or the bottom, not
	// one page as one might suspect. Though you're rarely carrying enough
	// objects for this to make a difference.
	//
	// The direction depends on if you click above or below the handle.
	// Clicking on the handle also works, though the behavior strikes me
	// as a bit unintuitive. If you click on Y coordinate pos + 5, nothing
	// happens at all.

	if (_bounds.contains(event.mouse)) {
		int pos = _bounds.top + getHandlePosition();

		if (event.mouse.y <= pos + 4)
			_invOffset = 0;
		else if (event.mouse.y >= pos + 6)
			_invOffset = _invCount - ARRAYSIZE(_slots);

		_gui->setInventoryScrollOffset(_invOffset);
		setRedraw(true);
	}

	return false;
}

void MacIndy3Gui::Inventory::ScrollBar::setInventoryParameters(int invCount, int invOffset) {
	if (invOffset != _invOffset)
		setRedraw(true);

	if (invCount != _invCount && _invCount >= ARRAYSIZE(_slots))
		setRedraw(true);

	_invCount = invCount;
	_invOffset = invOffset;
}

void MacIndy3Gui::Inventory::ScrollBar::scroll(ScrollDirection dir) {
	int newOffset = _invOffset;
	int maxOffset = _invCount - ARRAYSIZE(_slots);

	if (dir == kScrollUp)
		newOffset--;
	else
		newOffset++;

	if (newOffset < 0)
		newOffset = 0;
	else if (newOffset > maxOffset)
		newOffset = maxOffset;

	if (newOffset != _invOffset) {
		_invOffset = newOffset;
		_gui->setInventoryScrollOffset(_invOffset);
		setRedraw(true);
	}
}

int MacIndy3Gui::Inventory::ScrollBar::getHandlePosition() {
	if (_invOffset == 0)
		return 0;

	// Hopefully this matches the original scroll handle position.

	int maxPos = _bounds.height() - 8;
	int maxOffset = _invCount - ARRAYSIZE(_slots);

	if (_invOffset >= maxOffset)
		return maxPos;

	return maxPos * _invOffset / maxOffset;
}

void MacIndy3Gui::Inventory::ScrollBar::reset() {
	MacIndy3Gui::Widget::reset();
	_invCount = 0;
	_invOffset = 0;
}

void MacIndy3Gui::Inventory::ScrollBar::draw() {
	if (!getRedraw())
		return;

	debug(1, "Inventory::Scrollbar: Drawing");

	Widget::draw();
	drawShadowFrame(_bounds, kBlack, kBackground);

	// The scrollbar handle is only drawn when there are enough inventory
	// objects to scroll.

	if (_enabled) {
		debug(1, "Inventory::Scrollbar: Drawing handle");

		int y = _bounds.top + getHandlePosition();

		// The height of the scrollbar handle never changes, regardless
		// of how many items you are carrying.
		drawShadowFrame(Common::Rect(_bounds.left, y, _bounds.right, y + 8), kWhite, kTransparency);
	}

	setRedraw(false);
	markScreenAsDirty(_bounds);
}

// ---------------------------------------------------------------------------
// Inventory::ScrollButton is the buttons used to scroll the inventory up and
// down. They're only responsible for drawing the filled part of the arrow,
// since the rest of the buttons are drawn by the Inventory widget itself.
// ---------------------------------------------------------------------------

MacIndy3Gui::Inventory::ScrollButton::ScrollButton(int x, int y, int width, int height, ScrollDirection direction) : MacIndy3Gui::Widget(x, y, width, height) {
	_direction = direction;
}

bool MacIndy3Gui::Inventory::ScrollButton::handleEvent(Common::Event &event) {
	if (!_enabled || event.type != Common::EVENT_LBUTTONDOWN)
		return false;

	if (_bounds.contains(event.mouse)) {
		setRedraw(true);
		setTimer(WIDGET_TIMER_JIFFIES);
		return true;
	}

	return false;
}

bool MacIndy3Gui::Inventory::ScrollButton::handleMouseHeld(Common::Point &pressed, Common::Point &held) {
	if (!_enabled)
		return false;

	// The scroll button doesn't care if the mouse has moved outside while
	// being held.

	return _bounds.contains(pressed);
}

void MacIndy3Gui::Inventory::ScrollButton::timeOut() {
	// Nothing happens, but the button changes state.
	setRedraw(true);
}

void MacIndy3Gui::Inventory::ScrollButton::draw() {
	if (!getRedraw())
		return;

	debug(1, "Inventory::ScrollButton: Drawing [%d]", _direction);

	Widget::draw();

	const uint16 upArrow[] = {
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0080, 0x01C0, 0x03E0, 0x07F0,
		0x03E0, 0x03E0, 0x03E0, 0x03E0,
		0x0000, 0x0000, 0x0000, 0x0000
	};

	const uint16 downArrow[] = {
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x03E0, 0x03E0, 0x03E0,
		0x03E0, 0x07F0,	0x03E0, 0x01C0,
		0x0080, 0x0000, 0x0000, 0x0000
	};

	const uint16 *arrow = (_direction == kScrollUp) ? upArrow : downArrow;
	byte color = hasTimer() ? kBlack : kWhite;

	drawBitmap(_bounds, arrow, color);

	setRedraw(false);
	markScreenAsDirty(_bounds);
}

// ---------------------------------------------------------------------------
// The MacIndy3Gui class ties the whole thing together, mostly by delegating
// the work to the individual widgets.
// ---------------------------------------------------------------------------

MacIndy3Gui::MacIndy3Gui(ScummEngine *vm, const Common::Path &resourceFile) :
	MacGuiImpl(vm, resourceFile), _visible(false) {

	Common::Rect verbGuiArea(640, 112);
	verbGuiArea.translate(0, 288 + _vm->_macScreenDrawOffset * 2);

	_verbGuiTop = verbGuiArea.top;
	_verbGuiSurface = _surface->getSubArea(verbGuiArea);

	// There is one widget for every verb in the game. Verbs include the
	// inventory widget and conversation options.

	Widget::_vm = _vm;
	Widget::_surface = &_verbGuiSurface;
	Widget::_gui = this;

	_widgets[  1] = new Button(137, 24,  68, 18); // Open
	_widgets[  2] = new Button(137, 44,  68, 18); // Close
	_widgets[  3] = new Button( 67, 64,  68, 18); // Give
	_widgets[  4] = new Button(277, 44,  68, 18); // Turn on
	_widgets[  5] = new Button(277, 64,  68, 18); // Turn off
	_widgets[  6] = new Button( 67, 24,  68, 18); // Push
	_widgets[  7] = new Button( 67, 44,  68, 18); // Pull
	_widgets[  8] = new Button(277, 24,  68, 18); // Use
	_widgets[  9] = new Button(137, 64,  68, 18); // Look at
	_widgets[ 10] = new Button(207, 24,  68, 18); // Walk to
	_widgets[ 11] = new Button(207, 44,  68, 18); // Pick up
	_widgets[ 12] = new Button(207, 64,  68, 18); // What is
	_widgets[ 13] = new Button(347, 24,  68, 18); // Talk
	_widgets[ 14] = new Button( 97, 24, 121, 18); // Never mind.
	_widgets[ 32] = new Button(347, 44,  68, 18); // Travel
	_widgets[ 33] = new Button(347, 64,  68, 18); // To Indy
	_widgets[ 34] = new Button(347, 64,  68, 18); // To Henry
	_widgets[ 90] = new Button( 67,  4, 507, 18); // Travel 1
	_widgets[ 91] = new Button( 67, 24, 507, 18); // Travel 2
	_widgets[ 92] = new Button( 67, 44, 507, 18); // Travel 3
	_widgets[100] = new Button( 67,  4, 348, 18); // Sentence
	_widgets[101] = new Inventory(417, 4, 157, 78);
	_widgets[119] = new Button(324, 24,  91, 18); // Take this:
	_widgets[120] = new Button( 67,  4, 507, 18); // Converse 1
	_widgets[121] = new Button( 67, 24, 507, 18); // Converse 2
	_widgets[122] = new Button( 67, 44, 507, 18); // Converse 3
	_widgets[123] = new Button( 67, 64, 507, 18); // Converse 4
	_widgets[124] = new Button( 67, 64, 151, 18); // Converse 5
	_widgets[125] = new Button(423, 64, 151, 18); // Converse 6

	for (auto &it: _widgets)
		it._value->setVerbid(it._key);

	_dirtyRects.clear();
	_textArea.create(448, 47, Graphics::PixelFormat::createFormatCLUT8());
}

MacIndy3Gui::~MacIndy3Gui() {
	for (auto &it: _widgets)
		delete it._value;
	_textArea.free();
}

void MacIndy3Gui::setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate) {
	const byte buf[15 * 15] = {
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		0, 0, 0, 0, 0, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 0, 3, 0, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3
	};

	width = height = 15;
	hotspotX = hotspotY = 7;
	animate = 0;

	_windowManager->replaceCustomCursor(buf, width, height, hotspotX, hotspotY, 3);
}

const Graphics::Font *MacIndy3Gui::getFontByScummId(int32 id) {
	// The game seems to use font 0 most of the time, but during the intro
	// it switches to font 1 to print "BARNETT COLLEGE,", "NEW YORK," and
	// "1938". By the look of it, these map to the same font.
	//
	// This is different from the DOS version, where font 1 is bolder.
	switch (id) {
	case 0:
	case 1:
		return getFont(kIndy3FontMedium);

	default:
		error("MacIndy3Gui::getFontByScummId: Invalid font id %d", id);
	}
}

bool MacIndy3Gui::getFontParams(FontId fontId, int &id, int &size, int &slant) const {
	if (MacGuiImpl::getFontParams(fontId, id, size, slant))
		return true;

	// Indy 3 provides an "Indy" font in two sizes, 9 and 12, which are
	// used for the text boxes. The smaller font can be used for a
	// headline. The rest of the Indy 3 verb GUI uses Geneva.

	switch (fontId) {
	case kIndy3FontSmall:
		id = _gameFontId;
		size = 9;
		slant = Graphics::kMacFontRegular;
		return true;

	case kIndy3FontMedium:
		id = _gameFontId;
		size = 12;
		slant = Graphics::kMacFontRegular;
		return true;

	case kIndy3VerbFontRegular:
		id = Graphics::kMacFontGeneva;
		size = 9;
		slant = Graphics::kMacFontRegular;
		return true;

	case kIndy3VerbFontBold:
		id = Graphics::kMacFontGeneva;
		size = 9;
		slant = Graphics::kMacFontBold;
		return true;

	case kIndy3VerbFontOutline:
		id = Graphics::kMacFontGeneva;
		size = 9;
		slant = Graphics::kMacFontBold | Graphics::kMacFontOutline | Graphics::kMacFontCondense;
		return true;

	default:
		error("MacIndy3Gui: getFontParams: Unknown font id %d", (int)fontId);
	}

	return false;
}

void MacIndy3Gui::initTextAreaForActor(Actor *a, byte color) {
	int width = _textArea.w;
	int height = _textArea.h;

	_textArea.fillRect(Common::Rect(width, height), kBlack);

	int nameWidth = 0;

	if (a) {
		const Graphics::Font *font = getFont(kIndy3FontSmall);

		const char *name = (const char *)a->getActorName();
		int charX = 25;

		if (_vm->_renderMode == Common::kRenderMacintoshBW)
			color = kWhite;

		for (int i = 0; name[i] && nameWidth < width - 50; i++) {
			font->drawChar(&_textArea, name[i], charX, 0, color);
			nameWidth += font->getCharWidth(name[i]);
			charX += font->getCharWidth(name[i]);
		}

		font->drawChar(&_textArea, ':', charX, 0, color);
	}

	if (nameWidth) {
		_textArea.hLine(2, 3, 20, 15);
		_textArea.hLine(32 + nameWidth, 3, width - 3, 15);
	} else
		_textArea.hLine(2, 3, width - 3, 15);

	_textArea.vLine(1, 4, height - 3, 15);
	_textArea.vLine(width - 2, 4, height - 3, 15);
	_textArea.hLine(2, height - 2, width - 3, 15);
}

void MacIndy3Gui::printCharToTextArea(int chr, int x, int y, int color) {
	// In black and white mode, all text is white. Text is never disabled.
	if (_vm->_renderMode == Common::kRenderMacintoshBW)
		color = 15;

	// Since we're working with unscaled coordinates most of the time, the
	// lines of the text box weren't spaced quite as much as in the
	// original. I thought no one would notice, but I was wrong. This is
	// the best way I can think of to fix that.

	if (y > 0)
		y = 17;

	const Graphics::Font *font = getFont(kIndy3FontMedium);

	font->drawChar(&_textArea, chr, x + 5, y + 11, color);
}

bool MacIndy3Gui::handleMenu(int id, Common::String &name) {
	if (MacGuiImpl::handleMenu(id, name))
		return true;

	Common::StringArray substitutions;

	switch (id) {
	case 204:	// IQ Points
		runIqPointsDialog();
		break;

	case 205:	// Options
		runOptionsDialog();
		break;

	case 206:	// Quit
		if (runQuitDialog())
			_vm->quitGame();
		break;

	default:
		debug("MacIndy3Gui::handleMenu: Unknown menu command: %d", id);
		break;
	}

	return false;
}

void MacIndy3Gui::runAboutDialog() {
	// The About window is not a a dialog resource. Its size appears to be
	// hard-coded (416x166), and it's drawn centered. The graphics are in
	// PICT 2000.

	int width = 416;
	int height = 166;
	int x = (640 - width) / 2;
	int y = (400 - height) / 2;

	Common::Rect bounds(x, y, x + width, y + height);
	MacDialogWindow *window = createWindow(bounds, kWindowStyleNormal, kMenuStyleApple);
	Graphics::Surface *pict = loadPict(2000);

	// For the background of the sprites to match the background of the
	// window, we have to move them at multiples of 4 pixels each step. We
	// cut out enough of the background so that each time they are drawn,
	// the visible remains of the previous frame is overdrawn.

	Graphics::Surface train = pict->getSubArea(Common::Rect(249, 93));

	Graphics::Surface trolley[3];

	for (int i = 0; i < 3; i++)
		trolley[i] = pict->getSubArea(Common::Rect(251 + 92 * i, 38, 335 + 92 * i, 93));

	clearAboutDialog(window);
	window->show();

	Graphics::Surface *s = window->innerSurface();
	Common::Rect clipRect(2, 2, s->w - 4, s->h - 4);

	Common::Rect r1(22, 6, 382, 102);
	Common::Rect r2(22, 6, 382, 70);

	// Judging by recordings of Basilisk II, the internal frame rate is
	// 10 fps.

	int scene = 0;
	DelayStatus status = kDelayDone;

	int trainX = -2;
	int trolleyX = width + 1;
	int trolleyFrame = 1;
	int trolleyFrameDelta = 1;
	int trolleyWaitFrames = 20;	// ~2 seconds
	int waitFrames = 0;

	const char *subVers = (const char *)_vm->getStringAddress(24);

	Common::String version = Common::String::format(_strsStrings[kMSIAboutString3].c_str(), subVers, '5', '1', '6');

	const TextLine page1[] = {
		{ 0, 4, kStyleHeader1, Graphics::kTextAlignCenter, _strsStrings[kMSIAboutString1].c_str() }, // "Indiana Jones and the Last Crusade"
		{ 0, 22, kStyleBold, Graphics::kTextAlignCenter, _strsStrings[kMSIAboutString2].c_str() }, // "The Graphic Adventure"
		{ 0, 49, kStyleBold, Graphics::kTextAlignCenter, version.c_str() }, // "Mac 1.7 8/17/90, Interpreter version 5.1.6"
		{ 1, 82, kStyleRegular, Graphics::kTextAlignCenter, _strsStrings[kMSIAboutString4].c_str() }, // "TM & \xA9 1990 LucasArts Entertainment Company.  All rights reserved."
		TEXT_END_MARKER
	};

	const TextLine page2[] = {
		{ 1, 7, kStyleBold, Graphics::kTextAlignCenter, _strsStrings[kMSIAboutString5].c_str() }, // "Macintosh version by"
		{ 70, 21, kStyleHeader1, Graphics::kTextAlignLeft, _strsStrings[kMSIAboutString7].c_str() }, // "Eric Johnston"
		{ 194, 32, kStyleBold, Graphics::kTextAlignLeft, _strsStrings[kMSIAboutString6].c_str() }, // "and"
		{ 216, 41, kStyleHeader1, Graphics::kTextAlignLeft, _strsStrings[kMSIAboutString8].c_str() }, // "Dan Filner"
		TEXT_END_MARKER
	};

	const TextLine page3[] = {
		{ 1, 7, kStyleBold, Graphics::kTextAlignCenter, _strsStrings[kMSIAboutString9].c_str() }, // "Macintosh scripting by"
		{ 75, 21, kStyleHeader1, Graphics::kTextAlignLeft, _strsStrings[kMSIAboutString11].c_str() }, // "Ron Baldwin"
		{ 186, 32, kStyleBold, Graphics::kTextAlignLeft, _strsStrings[kMSIAboutString10].c_str() }, // "and"
		{ 214, 41, kStyleHeader1, Graphics::kTextAlignLeft, _strsStrings[kMSIAboutString12].c_str() }, // "David Fox"
		TEXT_END_MARKER
	};

	const TextLine page4[] = {
		{ 1, 7, kStyleBold, Graphics::kTextAlignCenter, _strsStrings[kMSIAboutString13].c_str() }, // "Designed and scripted by"
		{ 77, 24, kStyleHeader1, Graphics::kTextAlignLeft, _strsStrings[kMSIAboutString14].c_str() }, // "Noah Falstein"
		{ 134, 44, kStyleHeader1, Graphics::kTextAlignLeft, _strsStrings[kMSIAboutString15].c_str() }, // "David Fox"
		{ 167, 64, kStyleHeader1, Graphics::kTextAlignLeft, _strsStrings[kMSIAboutString16].c_str() }, // "Ron Gilbert"
		TEXT_END_MARKER
	};

	const TextLine page5[] = {
		{ 1, 7, kStyleBold, Graphics::kTextAlignCenter, _strsStrings[kMSIAboutString17].c_str() }, // "SCUMM Story System"
		{ 1, 17, kStyleBold, Graphics::kTextAlignCenter, _strsStrings[kMSIAboutString18].c_str() }, // "created by"
		{ 107, 36, kStyleHeader1, Graphics::kTextAlignLeft, _strsStrings[kMSIAboutString20].c_str() }, // "Ron Gilbert"
		{ 170, 52, kStyleBold, Graphics::kTextAlignLeft, _strsStrings[kMSIAboutString19].c_str() }, // "and"
		{ 132, 66, kStyleHeader1, Graphics::kTextAlignLeft, _strsStrings[kMSIAboutString21].c_str() }, // "Aric Wilmunder"
		TEXT_END_MARKER
	};

	const TextLine page6[] = {
		{ 1, 19, kStyleBold, Graphics::kTextAlignCenter, _strsStrings[kMSIAboutString22].c_str() }, // "Stumped?  Indy hint books are available!"
		{ 86, 36, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[kMSIAboutString25].c_str() }, // "In the U.S. call"
		{ 160, 37, kStyleBold, Graphics::kTextAlignLeft, _strsStrings[kMSIAboutString23].c_str() }, // "1 (800) STAR-WARS"
		{ 160, 46, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[kMSIAboutString27].c_str() }, // "that\xD5s  1 (800) 782-7927"
		{ 90, 66, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[kMSIAboutString26].c_str() }, // "In Canada call"
		{ 160, 67, kStyleBold, Graphics::kTextAlignLeft, _strsStrings[kMSIAboutString24].c_str() }, // "1 (800) 828-7927"
		TEXT_END_MARKER
	};

	const TextLine page7[] = {
		{ 1, 17, kStyleBold, Graphics::kTextAlignCenter, _strsStrings[kMSIAboutString28].c_str() }, // "Need a hint NOW?  Having problems?"
		{ 53, 31, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[kMSIAboutString30].c_str() }, // "For hints or technical support call"
		{ 215, 32, kStyleBold, Graphics::kTextAlignLeft, _strsStrings[kMSIAboutString29].c_str() }, // "1 (900) 740-JEDI"
		{ 1, 46, kStyleRegular, Graphics::kTextAlignCenter, _strsStrings[kMSIAboutString31].c_str() }, // "The charge is 75\xA2 per minute."
		{ 1, 56, kStyleRegular, Graphics::kTextAlignCenter, _strsStrings[kMSIAboutString32].c_str() }, // "(You must have your parents\xD5 permission to"
		{ 1, 66, kStyleRegular, Graphics::kTextAlignCenter, _strsStrings[kMSIAboutString33].c_str() }, // "call this number if you are under 18.)"
		TEXT_END_MARKER
	};

	const TextLine page8[] = {
		{ 1, 1, kStyleBold, Graphics::kTextAlignCenter, _strsStrings[kMSIAboutString34].c_str() }, // "Click to continue"
		TEXT_END_MARKER
	};

	bool changeScene = false;

	while (!_vm->shouldQuit()) {
		switch (scene) {
		case 0:
			window->drawSprite(&train, trainX, 40, clipRect);
			trainX -= 4;

			if (trainX < -train.w)
				changeScene = true;

			break;

		case 1:
		case 4:
		case 5:
		case 6:
		case 7:
			if (--waitFrames <= 0)
				changeScene = true;
			break;

		case 2:
		case 3:
			window->drawSprite(&trolley[trolleyFrame], trolleyX, 78, clipRect);

			if (scene == 2 && trolleyX == 161 && trolleyWaitFrames > 0) {
				if (--trolleyWaitFrames == 0)
					changeScene = true;
			} else {
				trolleyX -= 4;
				trolleyFrame += trolleyFrameDelta;
				if (trolleyFrame < 0 || trolleyFrame > 2) {
					trolleyFrame = 1;
					trolleyFrameDelta = -trolleyFrameDelta;
				}

				if (trolleyX < -85)
					changeScene = true;
			}

			break;
		}

		window->update();
		status = delay((scene == 0) ? 33 : 100);

		if (status == kDelayAborted)
			break;

		if (status == kDelayInterrupted || changeScene) {
			changeScene = false;
			scene++;
			waitFrames = 50;	// ~5 seconds

			switch (scene) {
			case 1:
				clearAboutDialog(window);
				window->drawTextBox(r1, page1);
				break;

			case 2:
				clearAboutDialog(window);
				window->drawTextBox(r2, page2);
				break;

			case 3:
				// Don't clear. The trolley is still on screen
				// and only the text changes.
				window->drawTextBox(r2, page3);
				break;

			case 4:
				clearAboutDialog(window);
				window->drawTextBox(r1, page4);
				break;

			case 5:
				window->drawTextBox(r1, page5);
				break;

			case 6:
				waitFrames = 100;	// ~10 seconds
				window->drawTextBox(r1, page6);
				break;

			case 7:
				waitFrames = 30;	// ~3 seconds
				window->drawTextBox(r1, page7);
				break;

			case 8:
				window->drawTextBox(Common::Rect(142, 106, 262, 119), page8, false, 3);
				break;
			}

			window->update();

			if (scene >= 8)
				break;
		}
	}

	if (status != kDelayAborted)
		delay();

	_windowManager->popCursor();

	pict->free();
	delete pict;
	delete window;
}

void MacIndy3Gui::clearAboutDialog(MacDialogWindow *window) {
	Graphics::Surface *s = window->innerSurface();

	window->fillPattern(Common::Rect(2, 2, s->w - 2, 132), 0x8020);
	window->fillPattern(Common::Rect(2, 130, s->w - 2, 133), 0xA5A5);
	window->fillPattern(Common::Rect(2, 133, s->w - 2, 136), 0xFFFF);
	window->fillPattern(Common::Rect(2, 136, s->w - 2, s->h - 4), 0xA5A5);
}

bool MacIndy3Gui::runOpenDialog(int &saveSlotToHandle) {
	// Widgets:
	//
	// 0 - Open button
	// 1 - Weird button outside the dialog (folder dropdown?)
	// 2 - Cancel button
	// [skipped] - User item (disk label?)
	// 3 - Eject button
	// 4 - Drive button
	// [skipped] - User item (file list?)
	// [skipped] - User item (scrollbar?)
	// [skipped] - User item (line between Desktop and Open buttons?)
	// 5 - Empty text
	// 6 - "IQ" picture
	// 7 - "Episode: ^0" text
	// 8 - "Series: ^1" text
	// 9 - "(Indy Quotient)" text

	MacDialogWindow *window = createDialog((_vm->_renderMode == Common::kRenderMacintoshBW) ? 4000 : 4001);

	MacButton *buttonSave = (MacButton *)window->getWidget(kWidgetButton, 0);
	MacButton *buttonCancel = (MacButton *)window->getWidget(kWidgetButton, 2);

	window->setDefaultWidget(buttonSave);
	window->addSubstitution(Common::String::format("%d", _vm->VAR(244)));
	window->addSubstitution(Common::String::format("%d", _vm->VAR(245)));

	bool availSlots[100];
	int slotIds[100];
	Common::StringArray savegameNames;
	prepareSaveLoad(savegameNames, availSlots, slotIds, ARRAYSIZE(availSlots));

	MacListBox *listBox = window->addListBox(Common::Rect(14, 41, 232, 187), savegameNames, true);

	drawFakePathList(window, Common::Rect(14, 18, 231, 37), "Indy Last Crusade");

	// When quitting, the default action is to not open a saved game
	bool ret = false;
	Common::Array<int> deferredActionsIds;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == buttonSave->getId() || clicked == listBox->getId()) {
			ret = true;
			saveSlotToHandle =
				listBox->getValue() < ARRAYSIZE(slotIds) ? slotIds[listBox->getValue()] : -1;
			break;
		}

		if (clicked == buttonCancel->getId())
			break;
	}

	delete window;
	return ret;
}

bool MacIndy3Gui::runSaveDialog(int &saveSlotToHandle, Common::String &saveName) {
	// Widgets:
	//
	// 0 - Save button
	// 1 - Cancel button
	// 2 - "Save Game File as..." text
	// [skipped] - User item (disk label?)
	// 3 - Eject button
	// 4 - Drive button
	// 5 - Editable text (save file name)
	// [skipped] - User item (file list?)
	// 6 - "IQ" picture
	// 7 - "Episode: ^0" text
	// 8 - "Series: ^1" text
	// 9 - "(Indy Quotient)" text

	MacDialogWindow *window = createDialog((_vm->_renderMode == Common::kRenderMacintoshBW) ? 3998 : 3999);

	MacButton *buttonSave = (MacButton *)window->getWidget(kWidgetButton, 0);
	MacButton *buttonCancel = (MacButton *)window->getWidget(kWidgetButton, 1);
	MacEditText *editText = (MacEditText *)window->getWidget(kWidgetEditText);

	window->setDefaultWidget(buttonSave);
	window->addSubstitution(Common::String::format("%d", _vm->VAR(244)));
	window->addSubstitution(Common::String::format("%d", _vm->VAR(245)));

	bool busySlots[100];
	int slotIds[100];
	Common::StringArray savegameNames;
	prepareSaveLoad(savegameNames, busySlots, slotIds, ARRAYSIZE(busySlots));

	drawFakePathList(window, Common::Rect(16, 8, 198, 27), "Indy Last Crusade");

	int firstAvailableSlot = -1;
	for (int i = 0; i < ARRAYSIZE(busySlots); i++) {
		if (!busySlots[i]) {
			firstAvailableSlot = i;
			break;
		}
	}

	window->addListBox(Common::Rect(16, 31, 199, 129), savegameNames, true, true);

	// When quitting, the default action is not to save a game
	bool ret = false;
	Common::Array<int> deferredActionsIds;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == buttonSave->getId()) {
			ret = true;
			saveName = editText->getText();
			saveSlotToHandle = firstAvailableSlot;
			break;
		}

		if (clicked == buttonCancel->getId())
			break;

		if (clicked == kDialogWantsAttention) {
			// Cycle through deferred actions
			for (uint i = 0; i < deferredActionsIds.size(); i++) {
				if (deferredActionsIds[i] == editText->getId()) {
					// Disable "Save" button when text is empty
					buttonSave->setEnabled(!editText->getText().empty());
				}
			}
		}
	}

	delete window;
	return ret;
}

bool MacIndy3Gui::runOptionsDialog() {
	// Widgets:
	//
	// 0 - Okay button
	// 1 - Cancel button
	// 2 - Sound checkbox
	// 3 - Music checkbox
	// 4 - Picture (text speed background)
	// 5 - Picture (text speed handle)
	// 6 - "Machine speed rating:" text
	// 7 - "^0" text
	// 8 - Scrolling checkbox
	// 9 - Text speed slider (manually created)

	int sound = (!ConfMan.hasKey("mute") || !ConfMan.getBool("mute")) ? 1 : 0;
	int music = (!ConfMan.hasKey("music_mute") || !ConfMan.getBool("music_mute")) ? 1 : 0;
	int scrolling = _vm->_snapScroll == 0;
	int textSpeed = _vm->_defaultTextSpeed;

	MacDialogWindow *window = createDialog(1000);

	MacButton *buttonOk = (MacButton *)window->getWidget(kWidgetButton, 0);
	MacButton *buttonCancel = (MacButton *)window->getWidget(kWidgetButton, 1);

	MacCheckbox *checkboxSound = (MacCheckbox *)window->getWidget(kWidgetCheckbox, 0);
	MacCheckbox *checkboxMusic = (MacCheckbox *)window->getWidget(kWidgetCheckbox, 1);
	MacCheckbox *checkboxScrolling = (MacCheckbox *)window->getWidget(kWidgetCheckbox, 2);

	checkboxSound->setValue(sound);
	checkboxMusic->setValue(music);
	checkboxScrolling->setValue(scrolling);

	if (!sound)
		checkboxMusic->setEnabled(false);

	MacPictureSlider *sliderTextSpeed = window->addPictureSlider(4, 5, true, 5, 105, 0, 9);
	sliderTextSpeed->setValue(textSpeed);

	window->addSubstitution(Common::String::format("%d", _vm->VAR(_vm->VAR_MACHINE_SPEED)));

	// When quitting, the default action is not to not apply options
	bool ret = false;
	Common::Array<int> deferredActionsIds;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == buttonOk->getId()) {
			ret = true;
			break;
		}

		if (clicked == buttonCancel->getId())
			break;

		if (clicked == checkboxSound->getId())
			checkboxMusic->setEnabled(checkboxSound->getValue() != 0);
	}

	if (ret) {
		// Update settings

		// TEXT SPEED
		_vm->_defaultTextSpeed = CLIP<int>(sliderTextSpeed->getValue(), 0, 9);
		ConfMan.setInt("original_gui_text_speed", _vm->_defaultTextSpeed);
		_vm->setTalkSpeed(_vm->_defaultTextSpeed);

		// SOUND&MUSIC ACTIVATION
		// 0 - Sound&Music on
		// 1 - Sound on, music off
		// 2 - Sound&Music off
		bool disableSound = checkboxSound->getValue() == 0;
		bool disableMusic = checkboxMusic->getValue() == 0;

		_vm->_musicEngine->toggleMusic(!disableMusic);
		_vm->_musicEngine->toggleSoundEffects(!disableSound);
		ConfMan.setBool("music_mute", disableMusic);
		ConfMan.setBool("mute", disableSound);
		ConfMan.flushToDisk();

		_vm->syncSoundSettings();

		// SCROLLING ACTIVATION
		_vm->_snapScroll = checkboxScrolling->getValue() == 0;
	}

	delete window;
	return ret;
}

bool MacIndy3Gui::runIqPointsDialog() {
	// Widgets
	//
	// 0 - Done button
	// 1 - Reset Series IQ button
	// 2 - "(Indy Quotient)" text
	// 3 - "Episode: ^0" text
	// 4 - "Series: ^1" text
	// 5 - "IQ" picture

	MacDialogWindow *window = createDialog((_vm->_renderMode == Common::kRenderMacintoshBW) ? 1001 : 1002);

	MacButton *buttonOk = (MacButton *)window->getWidget(kWidgetButton, 0);
	MacButton *buttonCancel = (MacButton *)window->getWidget(kWidgetButton, 1);

	MacStaticText *textSeriesIQ = (MacStaticText *)window->getWidget(kWidgetStaticText, 2);

	((ScummEngine_v4 *)_vm)->updateIQPoints();
	window->addSubstitution(Common::String::format("%d", _vm->VAR(244)));
	window->addSubstitution(Common::String::format("%d", _vm->VAR(245)));

	Common::Array<int> deferredActionsIds;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == buttonOk->getId())
			break;

		if (clicked == buttonCancel->getId()) {
			if (!_vm->enhancementEnabled(kEnhUIUX) || runOkCancelDialog("Are you sure you want to reset the series IQ score?")) {
				((ScummEngine_v4 *)_vm)->clearSeriesIQPoints();
				window->replaceSubstitution(1, Common::String::format("%d", _vm->VAR(245)));
				textSeriesIQ->setRedraw(true);
			}
		}
	}

	delete window;
	return true;
}

// Before the GUI rewrite, the scroll offset was saved in variable 67. Let's
// continue that tradition, just in case. If nothing else, it gives us an easy
// way to still store it in savegames.

int MacIndy3Gui::getInventoryScrollOffset() const {
	return _vm->VAR(67);
}

void MacIndy3Gui::setInventoryScrollOffset(int n) const {
	_vm->VAR(67) = n;
}

bool MacIndy3Gui::isVerbGuiAllowed() const {
	// The GUI is only allowed if the verb area has the expected size. That
	// really seems to be all that's needed.

	VirtScreen *vs = &_vm->_virtscr[kVerbVirtScreen];
	if (vs->topline != 144 || vs->h != 56)
		return false;

	// HACK: Don't allow the GUI during fist fights. Usually this is not a
	//       problem, in my experience, but I've had it happening when
	//       offering an item to a guard led directly to one.

	if (_vm->VAR(_vm->VAR_VERB_SCRIPT) == 19)
		return false;

	return true;
}

bool MacIndy3Gui::isVerbGuiActive() const {
	// The visibility flag may not have been updated yet, so better check
	// that the GUI is still allowed.

	return _visible && isVerbGuiAllowed();
}

void MacIndy3Gui::reset() {
	_visible = false;

	for (auto &it: _widgets)
		it._value->reset();
}

void MacIndy3Gui::resetAfterLoad() {
	reset();

	// In the DOS version, verb ID 102-106 were used for the visible
	// inventory items, and 107-108 for inventory arrow buttons. In the
	// Macintosh version, the entire inventory widget is verb ID 101.
	//
	// In old savegames, the DOS verb IDs may still be present, and have
	// to be removed.

	for (int i = 0; i < _vm->_numVerbs; i++) {
		if (_vm->_verbs[i].verbid >= 102 && _vm->_verbs[i].verbid <= 108)
			_vm->killVerb(i);
	}

	int charsetId = _vm->_charset->getCurID();
	if (charsetId < 0 || charsetId > 1)
		_vm->_charset->setCurID(0);
}

void MacIndy3Gui::update(int delta) {
	if (isVerbGuiAllowed() && updateVerbs(delta)) {
		if (!_visible)
			show();

		updateMouseHeldTimer(delta);
		drawVerbs();
	} else {
		if (_visible)
			hide();
	}

	copyDirtyRectsToScreen();
}

bool MacIndy3Gui::updateVerbs(int delta) {
	// Tentatively mark the verb widgets for removal. Any widget that wants
	// to stay has to say so.

	for (auto &it: _widgets) {
		VerbWidget *w = it._value;

		if (delta > 0)
			w->updateTimer(delta);

		w->threaten();
	}

	bool hasActiveVerbs = false;

	// Collect all active verbs. Verb slot 0 is special, apparently, so we
	// don't look at that one.

	for (int i = 1; i < _vm->_numVerbs; i++) {
		VerbSlot *vs = &_vm->_verbs[i];

		if (!vs->saveid && vs->curmode && vs->verbid) {
			VerbWidget *w = _widgets.getValOrDefault(vs->verbid);

			if (w) {
				w->updateVerb(i);
				hasActiveVerbs = true;
			} else {
				const byte *ptr = _vm->getResourceAddress(rtVerb, i);
				byte buf[270];
				_vm->convertMessageToString(ptr, buf, sizeof(buf));
				warning("MacIndy3Gui: Unknown verb: %d %s", vs->verbid, buf);
			}
		}
	}

	return hasActiveVerbs;
}

void MacIndy3Gui::updateMouseHeldTimer(int delta) {
	if (delta > 0 && _leftButtonIsPressed) {
		_timer -= delta;

		if (_timer <= 0) {
			debug(2, "MacIndy3Gui: Left button still down");

			_timer = REPEAT_TIMER_JIFFIES;

			for (auto &it: _widgets) {
				if (it._value->handleMouseHeld(_leftButtonPressed, _leftButtonHeld))
					break;
			}
		}
	}
}

void MacIndy3Gui::drawVerbs() {
	// The possible verbs overlap each other. Remove the dead ones first, then draw the live ones.

	for (auto &it: _widgets) {
		VerbWidget *w = it._value;

		if (w->isDying() && w->isVisible()) {
			w->undraw();
			w->reset();
		}
	}

	for (auto &it: _widgets) {
		VerbWidget *w = it._value;

		if (w->hasVerb())
			w->draw();
	}
}

bool MacIndy3Gui::handleEvent(Common::Event event) {
	if (MacGuiImpl::handleEvent(event))
		return true;

	if (_vm->isPaused())
		return false;

	bool isPauseEvent = event.type == Common::EVENT_KEYDOWN &&
		event.kbd == Common::KEYCODE_SPACE;

	if (!isPauseEvent && (!isVerbGuiActive() || _vm->_userPut <= 0))
		return false;

	// Make all mouse events relative to the verb GUI area

	if (Common::isMouseEvent(event))
		event.mouse.y -= _verbGuiTop;

	if (event.type == Common::EVENT_LBUTTONDOWN) {
		if (!_leftButtonIsPressed) {
			debug(2, "MacIndy3Gui: Left button down");

			_leftButtonIsPressed = true;
			_leftButtonPressed = event.mouse;
			_leftButtonHeld = event.mouse;
			_timer = REPEAT_TIMER_JIFFIES;
		}
	} else if (event.type == Common::EVENT_LBUTTONUP) {
		if (_leftButtonIsPressed) {
			debug(2, "MacIndy3Gui: Left button up");

			_leftButtonIsPressed = false;
			_timer = 0;
		}
	} else if (event.type == Common::EVENT_MOUSEMOVE) {
		if (_leftButtonIsPressed) {
			_leftButtonHeld.x = event.mouse.x;
			_leftButtonHeld.y = event.mouse.y;
		}
	}

	// It probably doesn't make much of a difference, but if a widget
	// responds to an event, and marks itself as wanting to be redrawn,
	// we do that redrawing immediately, not on the next update.

	for (auto &it: _widgets) {
		Widget *w = it._value;

		if (w->handleEvent(event)) {
			if (w->getRedraw()) {
				w->draw();
				copyDirtyRectsToScreen();
			}
			return true;
		}
	}

	return false;
}

void MacIndy3Gui::show() {
	if (_visible)
		return;

	debug(1, "MacIndy3Gui: Showing");

	_visible = true;

	// The top and bottom of the verb GUI area black. On a screen that's
	// 400 pixels tall, the verb GUI extends all the way to the bottom. On
	// a 480 pixel tall screen there will be an additional 40 pixels below
	// that, but nothing should ever be drawn there and if it ever is, it's
	// not the verb GUI's responsibility to clear it.

	_verbGuiSurface.fillRect(Common::Rect(0, 0, 640, 1), kBlack);
	_verbGuiSurface.fillRect(Common::Rect(0, 85, 640, _verbGuiSurface.h), kBlack);

	fill(Common::Rect(0, 2, 640, 85));

	const uint16 ulCorner[] = { 0xF000, 0xC000, 0x8000, 0x8000 };
	const uint16 urCorner[] = { 0xF000, 0x3000, 0x1000, 0x1000 };
	const uint16 llCorner[] = { 0x8000, 0x8000, 0xC000, 0xF000 };
	const uint16 lrCorner[] = { 0x1000, 0x1000, 0x3000, 0xF000 };

	drawBitmap(&_verbGuiSurface, Common::Rect(  0,  2,   4,  6), ulCorner, kBlack);
	drawBitmap(&_verbGuiSurface, Common::Rect(636,  2, 640,  6), urCorner, kBlack);
	drawBitmap(&_verbGuiSurface, Common::Rect(  0, 81,   4, 85), llCorner, kBlack);
	drawBitmap(&_verbGuiSurface, Common::Rect(636, 81, 640, 85), lrCorner, kBlack);

	markScreenAsDirty(Common::Rect(_verbGuiSurface.w, _verbGuiSurface.h));
}

void MacIndy3Gui::hide() {
	if (!_visible)
		return;

	debug(1, "MacIndy3Gui: Hiding");

	_leftButtonIsPressed = false;
	_timer = 0;

	reset();

	// If the verb GUI is allowed, the area should be cleared. If the verb
	// GUI is not allowed, the game is presumably using it for its own
	// drawing, and we should leave it alone.

	if (isVerbGuiAllowed()) {
		_verbGuiSurface.fillRect(Common::Rect(_verbGuiSurface.w, _verbGuiSurface.h), kBlack);
		markScreenAsDirty(Common::Rect(_verbGuiSurface.w, _verbGuiSurface.h));
	}
}

void MacIndy3Gui::markScreenAsDirty(Common::Rect r) {
	// As long as we always call this with the most encompassing rect
	// first, it is trivial to filter out unnecessary calls.

	for (uint i = 0; i < _dirtyRects.size(); i++) {
		if (_dirtyRects[i].contains(r))
			return;
	}

	_dirtyRects.push_back(r);
}

void MacIndy3Gui::copyDirtyRectsToScreen() {
	for (uint i = 0; i < _dirtyRects.size(); i++) {
		_system->copyRectToScreen(
			_verbGuiSurface.getBasePtr(_dirtyRects[i].left, _dirtyRects[i].top),
			_verbGuiSurface.pitch,
			_dirtyRects[i].left, _verbGuiTop + _dirtyRects[i].top,
			_dirtyRects[i].width(), _dirtyRects[i].height());
	}

	_dirtyRects.clear();
}

void MacIndy3Gui::fill(Common::Rect r) {
	int pitch = _verbGuiSurface.pitch;

	// Fill the screen with either gray of a checkerboard pattern.

	if (_vm->_renderMode == Common::kRenderMacintoshBW) {
		byte *row = (byte *)_verbGuiSurface.getBasePtr(r.left, r.top);

		for (int y = r.top; y < r.bottom; y++) {
			byte *ptr = row;
			for (int x = r.left; x < r.right; x++)
				*ptr++ = ((x + y) & 1) ? kWhite : kBlack;
			row += pitch;
		}
	} else
		_verbGuiSurface.fillRect(r, kLightGray);
}

} // End of namespace Scumm
