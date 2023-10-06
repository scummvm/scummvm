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

#include "graphics/macega.h"
#include "graphics/fonts/macfont.h"
#include "graphics/macgui/macfontmanager.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/gfx_mac.h"
#include "scumm/usage_bits.h"
#include "scumm/verbs.h"

namespace Scumm {

void ScummEngine::mac_markScreenAsDirty(int x, int y, int w, int h) {
	// Mark the virtual screen as dirty. The top and left coordinates are
	// rounded down, while the bottom and right ones are rounded up.

	VirtScreen *vs = &_virtscr[kMainVirtScreen];

	int vsTop = y / 2 - vs->topline;
	int vsBottom = (y + h) / 2 - vs->topline;
	int vsLeft = x / 2;
	int vsRight = (x + w) / 2;

	if ((y + h) & 1)
		vsBottom++;

	if ((x + w) & 1)
		vsRight++;

	markRectAsDirty(kMainVirtScreen, vsLeft, vsRight, vsTop, vsBottom);
}

void ScummEngine::mac_drawStripToScreen(VirtScreen *vs, int top, int x, int y, int width, int height) {
	// The verb screen is completely replaced with a custom GUI. All
	// other drawing to that area is suspended.
	if (vs->number == kVerbVirtScreen && _macIndy3Gui->isVisible())
		return;

	const byte *pixels = vs->getPixels(x, top);
	const byte *ts = (byte *)_textSurface.getBasePtr(x * 2, y * 2);
	byte *mac = (byte *)_macScreen->getBasePtr(x * 2, y * 2);

	int pixelsPitch = vs->pitch;
	int tsPitch = _textSurface.pitch;
	int macPitch = _macScreen->pitch;

	// In b/w Mac rendering mode, the shadow palette is implemented here,
	// and not as a palette manipulation. See special cases in o5_roomOps()
	// and updatePalette().
	//
	// This is used at the very least for the lightning flashes at Castle
	// Brunwald in Indy 3, as well as the scene where the dragon finds
	// Rusty in Loom.
	//
	// Interestingly, the original Mac interpreter does not seem to do
	// this, and instead just renders the scene as if the palette was
	// unmodified. At least, that's what Mini vMac did when I tried it.

	if (_renderMode == Common::kRenderMacintoshBW) {
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				int color = _enableEnhancements ? _shadowPalette[pixels[w]] : pixels[w];
				if (ts[2 * w] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w] = Graphics::macEGADither[color][0];
				if (ts[2 * w + 1] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + 1] = Graphics::macEGADither[color][1];
				if (ts[2 * w + tsPitch] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + macPitch] = Graphics::macEGADither[color][2];
				if (ts[2 * w + tsPitch + 1] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + macPitch + 1] = Graphics::macEGADither[color][3];
			}

			pixels += pixelsPitch;
			ts += tsPitch * 2;
			mac += macPitch * 2;
		}
	} else {
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				if (ts[2 * w] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w] = pixels[w];
				if (ts[2 * w + 1] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + 1] = pixels[w];
				if (ts[2 * w + tsPitch] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + macPitch] = pixels[w];
				if (ts[2 * w + tsPitch + 1] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + macPitch + 1] = pixels[w];
			}

			pixels += pixelsPitch;
			ts += tsPitch * 2;
			mac += macPitch * 2;
		}
	}

	_system->copyRectToScreen(_macScreen->getBasePtr(x * 2, y * 2), _macScreen->pitch, x * 2, y * 2, width * 2, height * 2);
}

void ScummEngine::mac_drawLoomPracticeMode() {
	// In practice mode, the game shows the notes as they are being played.
	// In the DOS version, this is drawn by script 27 but the Mac version
	// just sets variables 50 and 54. The box is actually a verb, and it
	// seems that setting variable 50 is pretty much equal to turning verb
	// 53 on or off. I'm not sure what the purpose of variable 54 is.

	int x = 216;
	int y = 377;
	int width = 62;
	int height = 22;
	int var = 50;

	byte *ptr = (byte *)_macScreen->getBasePtr(x,  y);
	int pitch = _macScreen->pitch;

	int slot = getVerbSlot(53, 0);
	VerbSlot *vs = &_verbs[slot];

	vs->curmode = (VAR(var) != 0);
	vs->curRect.left = x / 2;
	vs->curRect.right = (x + width) / 2;
	vs->curRect.top = y / 22;
	vs->curRect.bottom = (y + height) / 2;

	_macScreen->fillRect(Common::Rect(x, y, x + width, y + height), 0);

	if (VAR(var)) {
		for (int w = 1; w < width - 1; w++) {
			ptr[w] = 7;
			ptr[w + pitch * (height - 1)] = 7;
		}

		for (int h = 1; h < height - 1; h++) {
			ptr[h * pitch] = 7;
			ptr[h * pitch + width - 1] = 7;
		}

		// Draw the notes
		int colors[] = { 4, 12, 14, 10, 11, 3, 9, 15 };

		for (int i = 0; i < 4; i++) {
			int note = (VAR(var) >> (4 * i)) & 0x0F;

			if (note >= 2 && note <= 9) {
				_charset->setColor(colors[note - 2]);
				_charset->drawChar(14 + note, *_macScreen, i * 13 + x + 8, y + 4);
			}
		}
	}

	_system->copyRectToScreen(ptr, pitch, x, y, width, height);
}

void ScummEngine::mac_createIndy3TextBox(Actor *a) {
	int width = _macIndy3TextBox->w;
	int height = _macIndy3TextBox->h;

	_macIndy3TextBox->fillRect(Common::Rect(width, height), 0);

	int nameWidth = 0;

	if (a) {
		int oldID = _charset->getCurID();
		_charset->setCurID(2 | 0x80);

		const char *name = (const char *)a->getActorName();
		int charX = 25;

		for (int i = 0; name[i] && nameWidth < width - 50; i++) {
			_charset->drawChar(name[i], *_macIndy3TextBox, charX, 0);
			nameWidth += _charset->getCharWidth(name[i]);
			charX += _charset->getCharWidth(name[i]);
		}

		_charset->drawChar(':', *_macIndy3TextBox, charX, 0);
		_charset->setCurID(oldID);
	}

	if (nameWidth) {
		_macIndy3TextBox->hLine(2, 3, 20, 15);
		_macIndy3TextBox->hLine(32 + nameWidth, 3, width - 3, 15);
	} else
		_macIndy3TextBox->hLine(2, 3, width - 3, 15);

	_macIndy3TextBox->vLine(1, 4, height - 3, 15);
	_macIndy3TextBox->vLine(width - 2, 4, height - 3, 15);
	_macIndy3TextBox->hLine(2, height - 2, width - 3, 15);
}

void ScummEngine::mac_drawIndy3TextBox() {
	// The first two rows of the text box are padding for font rendering.
	// They are not drawn to the screen.

	int x = 96;
	int y = 32;
	int w = _macIndy3TextBox->w;
	int h = _macIndy3TextBox->h - 2;

	// The text box is drawn to the Mac screen and text surface, as if it
	// had been one giant glyph. Note that it will be drawn on the main
	// virtual screen, but we still pretend it's on the text one.

	byte *ptr = (byte *)_macIndy3TextBox->getBasePtr(0, 2);
	int pitch = _macIndy3TextBox->pitch;

	_macScreen->copyRectToSurface(ptr, pitch, x, y, w, h);
	_textSurface.fillRect(Common::Rect(x, y, x + w, y + h), 0);

	mac_markScreenAsDirty(x, y, w, h);
}

void ScummEngine::mac_undrawIndy3TextBox() {
	int x = 96;
	int y = 32;
	int w = _macIndy3TextBox->w;
	int h = _macIndy3TextBox->h - 2;

	_macScreen->fillRect(Common::Rect(x, y, x + w, y + h), 0);
	_textSurface.fillRect(Common::Rect(x, y, x + w, y + h), CHARSET_MASK_TRANSPARENCY);

	mac_markScreenAsDirty(x, y, w, h);
}

void ScummEngine::mac_undrawIndy3CreditsText() {
	if (_macScreen) {
		// Set _masMask to make the text clear, and _textScreenID to
		// ensure that it's the main area that's cleared. Note that
		// this only works with the high-resolution font.
		_charset->_hasMask = true;
		_charset->_textScreenID = kMainVirtScreen;
		restoreCharsetBg();
	} else {
		// The DOS VGA version clear the text by using the putState
		// opcode. I would have been more comfortable if I could have
		// compared it to the EGA version, but I don't have that.
		// Judging by the size and position of the object, they should
		// be the same.
		putState(946, 0);
		markObjectRectAsDirty(946);
		if (_bgNeedsRedraw)
			clearDrawObjectQueue();
	}
}

void ScummEngine::mac_drawBorder(int x, int y, int w, int h, byte color) {
	_macScreen->hLine(x + 2, y, x + w - 2, 0);
	_macScreen->hLine(x + 2, y + h - 1, x + w - 2, 0);
	_macScreen->vLine(x, y + 2, y + h - 3, 0);
	_macScreen->vLine(x + w, y + 2, y + h - 3, 0);
	_macScreen->setPixel(x + 1, y + 1, 0);
	_macScreen->setPixel(x + w - 1, y + 1, 0);
	_macScreen->setPixel(x + 1, y + h - 2, 0);
	_macScreen->setPixel(x + w - 1, y + h - 2, 0);
}

Common::KeyState ScummEngine::mac_showOldStyleBannerAndPause(const char *msg, int32 waitTime) {
	char bannerMsg[512];

	_messageBannerActive = true;

	// Fetch the translated string for the message...
	convertMessageToString((const byte *)msg, (byte *)bannerMsg, sizeof(bannerMsg));

	// Backup the surfaces...
	int x = 70;
	int y = 189;
	int w = 499;
	int h = 22;

	Graphics::Surface backupTextSurface;
	Graphics::Surface backupMacScreen;

	backupTextSurface.create(w + 1, h, Graphics::PixelFormat::createFormatCLUT8());
	backupMacScreen.create(w + 1, h, Graphics::PixelFormat::createFormatCLUT8());

	backupTextSurface.copyRectToSurface(_textSurface, 0, 0, Common::Rect(x, y, x + w + 1, y + h));
	backupMacScreen.copyRectToSurface(*_macScreen, 0, 0, Common::Rect(x, y, x + w + 1, y + h));

	// Pause shake effect
	_shakeTempSavedState = _shakeEnabled;
	setShake(0);

	// Pause the engine
	PauseToken pt = pauseEngine();

	// Backup the current charsetId...
	int oldId = _charset->getCurID();
	_charset->setCurID(1 | 0x80);
	_charset->setColor(0);

	_textSurface.fillRect(Common::Rect(x, y, x + w + 1, y + h), 0);
	_macScreen->fillRect(Common::Rect(x + 1, y + 1, x + w, y + h - 1), 15);
	mac_drawBorder(x, y, w, h, 0);
	mac_drawBorder(x + 2, y + 2, w - 4, h - 4, 0);

	int stringWidth = 0;

	for (int i = 0; msg[i]; i++)
		stringWidth += _charset->getCharWidth(msg[i]);

	int stringX = 1 + x + (w - stringWidth) / 2;

	for (int i = 0; msg[i]; i++) {
		_charset->drawChar(msg[i], *_macScreen, stringX, y + 4);
		stringX += _charset->getCharWidth(msg[i]);
	}

	mac_markScreenAsDirty(x, y, w, h);
	ScummEngine::drawDirtyScreenParts();

	Common::KeyState ks = Common::KEYCODE_INVALID;
	bool leftBtnPressed = false, rightBtnPressed = false;
	if (waitTime) {
		waitForBannerInput(waitTime, ks, leftBtnPressed, rightBtnPressed);
	}

	// Restore the surfaces...
	_textSurface.copyRectToSurface(backupTextSurface, x, y, Common::Rect(0, 0, w + 1, h));
	_macScreen->copyRectToSurface(backupMacScreen, x, y, Common::Rect(0, 0, w + 1, h));

	backupTextSurface.free();
	backupMacScreen.free();

	// Notify the gfx system that we restored the surfaces...
	mac_markScreenAsDirty(x, y, w + 1, h);
	ScummEngine::drawDirtyScreenParts();

	// Finally, resume the engine, clear the input state, and restore the charset.
	pt.clear();
	clearClickedStatus();

	_charset->setCurID(oldId);

	_messageBannerActive = false;

	return ks;
}

/*
 * The infamous verb GUI for the Macintosh version of Indiana Jones and the
 * Last Crusade.
 *
 * It's likely that the original interpreter used more hooks from the engine
 * into the GUI. In particular, the inventory script uses a variable that I
 * haven't been able to find in any of the early scripts of the game.
 *
 * But the design goal here is to keep things as simple as possible, even if
 * that means using brute force. So the GUI figures out which verbs are active
 * by scanning all the verbs, and which objects are in the inventory by
 * scanning all objects.
 *
 * We used to coerce the Mac verb GUI into something that looked like the
 * GUI from all other versions. This used a number of variables and hard-coded
 * verbs. The only thing still used of all this is variable 67, to keep track
 * of the inventory scroll position. The fake verbs are removed when loading
 * old savegames, but the variables are assumed to be harmless.
 */

ScummEngine *MacIndy3Gui::Widget::_vm = nullptr;
Graphics::Surface *MacIndy3Gui::Widget::_surface = nullptr;
MacIndy3Gui *MacIndy3Gui::Widget::_gui = nullptr;

MacIndy3Gui::Widget::Widget(int x, int y, int width, int height) {
	_bounds.left = x;
	_bounds.top = y;
	_bounds.right = x + width;
	_bounds.bottom = y + height;
}

void MacIndy3Gui::Widget::reset() {
	_timer = 0;
	_enabled = false;
	setRedraw(false);
}

bool MacIndy3Gui::Widget::updateTimer(int delta) {
	if (_timer == 0)
		return false;

	if (delta > _timer)
		delta = _timer;

	_timer -= MIN(delta, _timer);
	return _timer == 0;
}

void MacIndy3Gui::Widget::copyRectToScreen(Common::Rect r) {
	_gui->copyRectToScreen(r);
}

void MacIndy3Gui::Widget::fill(Common::Rect r) {
	_gui->fill(r);
}

void MacIndy3Gui::Widget::drawBitmap(Common::Rect r, const uint16 *bitmap, byte color) {
	_gui->drawBitmap(r, bitmap, color);
}

void MacIndy3Gui::Widget::drawShadowBox(Common::Rect r) {
	_surface->hLine(r.left + 1, r.top, r.right - 3, 0);
	_surface->hLine(r.left + 1, r.bottom - 2, r.right - 3, 0);
	_surface->vLine(r.left, r.top + 1, r.bottom - 3, 0);
	_surface->vLine(r.right - 2, r.top + 1, r.bottom - 3, 0);

	_surface->hLine(r.left + 2, r.bottom - 1, r.right - 1, 0);
	_surface->vLine(r.right - 1, r.top + 2, r.bottom - 2, 0);

	_surface->hLine(r.left + 1, r.top + 1, r.right - 3, 15);
	_surface->vLine(r.left + 1, r.top + 2, r.bottom - 3, 15);
}

void MacIndy3Gui::Widget::drawShadowFrame(Common::Rect r, byte shadowColor, byte fillColor) {
	_surface->hLine(r.left, r.top, r.right - 1, 0);
	_surface->hLine(r.left, r.bottom - 1, r.right - 1, 0);
	_surface->vLine(r.left, r.top + 1, r.bottom - 2, 0);
	_surface->vLine(r.right - 1, r.top + 1, r.bottom - 2, 0);

	_surface->hLine(r.left + 1, r.top + 1, r.right - 2, shadowColor);
	_surface->vLine(r.left + 1, r.top + 2, r.bottom - 2, shadowColor);

	if (fillColor != 255) {
		Common::Rect fillRect(r.left + 2, r.top + 2, r.right - 1, r.bottom - 1);

		if (fillColor == 0)
			fill(fillRect);
		else
			_surface->fillRect(fillRect, fillColor);
	}
}

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
	// Clear the area used by the widget
	fill(_bounds);
	_visible = true;

	// Don't copy to screen, because widgets are assumed to keep drawing
	// after calling this. Also, don't call setRedraw() because there
	// may be sub-widgets that still need drawing.
}

void MacIndy3Gui::VerbWidget::undraw() {
	fill(_bounds);
	copyRectToScreen(_bounds);
}

void MacIndy3Gui::Button::reset() {
	MacIndy3Gui::VerbWidget::reset();
	_text.clear();
}

bool MacIndy3Gui::Button::handleEvent(Common::Event &event) {
	if (!_verbid || !_enabled)
		return false;

	VerbSlot *vs = &_vm->_verbs[_verbslot];

	if (vs->saveid)
		return false;

	bool caughtEvent = false;

	if (event.type == Common::EVENT_KEYDOWN) {
		if (!event.kbd.hasFlags(Common::KBD_NON_STICKY) && event.kbd.keycode == vs->key) {
			caughtEvent = true;
		}
	} else if (event.type == Common::EVENT_LBUTTONDOWN) {
		int x = event.mouse.x;
		int y = event.mouse.y;

		if (_bounds.contains(x, y))
			caughtEvent = true;
	}

	// Events are handled at the end of the animation. This blatant attack
	// on speedrunners all over the world was done because that's what the
	// original seems to do, and it looks better. Based on tests in Mac
	// emulators, the speed of the animation depended on the speed of your
	// computer, so we just set something that looks good here.

	if (caughtEvent) {
		setRedraw(true);
		_timer = 12;
	}

	return caughtEvent;
}

bool MacIndy3Gui::Button::updateTimer(int delta) {
	bool ret = VerbWidget::updateTimer(delta);

	if (ret) {
		if (_visible) {
			_vm->runInputScript(kVerbClickArea, _verbid, 1);
			setRedraw(true);
		}
	}

	return ret;
}

void MacIndy3Gui::Button::updateVerb(int verbslot) {
	MacIndy3Gui::VerbWidget::updateVerb(verbslot);

	const byte *ptr = _vm->getResourceAddress(rtVerb, verbslot);
	byte buf[270];

	_vm->convertMessageToString(ptr, buf, sizeof(buf));
	if (_text != (char *)buf) {
		_text = (char *)buf;
		_timer = 0;
		setRedraw(true);
	}
}

void MacIndy3Gui::Button::draw() {
	if (!_redraw)
		return;

	debug("Drawing button [%d] %s", _verbid, _text.c_str());

	MacIndy3Gui::VerbWidget::draw();

	if (_timer == 0) {
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
		// two corners rounded.
		_surface->hLine(_bounds.left + 2, _bounds.top + 1, _bounds.right - 2, 0);
		_surface->hLine(_bounds.left + 2, _bounds.bottom - 1, _bounds.right - 1, 0);
		_surface->vLine(_bounds.left + 1, _bounds.top + 2, _bounds.bottom - 2, 0);
		_surface->vLine(_bounds.right - 1, _bounds.top + 2, _bounds.bottom - 2, 0);

		_surface->hLine(_bounds.left + 2, _bounds.top + 2, _bounds.right - 2, 15);
		_surface->vLine(_bounds.left + 2, _bounds.top + 3, _bounds.bottom - 2, 15);
	}

	// The text is drawn centered. Based on experimentation, I think the
	// width is always based on the outlined font, and the button shadow is
	// not counted as part of the button width.
	//
	// This gives us pixel perfect rendering for the English verbs.

	if (!_text.empty()) {
		const Graphics::Font *boldFont = _gui->getFont(1);
		const Graphics::Font *outlineFont = _gui->getFont(2);

		int stringWidth = 0;
		for (uint i = 0; i < _text.size(); i++)
			stringWidth += outlineFont->getCharWidth(_text[i]);

		int x = (_bounds.left + (_bounds.width() - 1 - stringWidth) / 2) - 1;
		int y = _bounds.top + 2;
		int color = _enabled ? 15 : 0;

		if (_timer) {
			x++;
			y++;
		}

		for (uint i = 0; i < _text.size() && x < _bounds.right; i++) {
			if (x >= _bounds.left) {
				if (_enabled)
					outlineFont->drawChar(_surface, _text[i], x, y, 0);
				boldFont->drawChar(_surface, _text[i], x + 1, y, color);
			}
			x += boldFont->getCharWidth(_text[i]);
		}
	}

	setRedraw(false);
	copyRectToScreen(_bounds);
}

// Desired behavior:
//
// The drag handle of the scrollbar never changes size. It's only drawn when
// there are enough inventory items to scroll.
//
// The size of the scroll handle is fixed, not scaled to indicate the number of
// objects in your inventory.
//
// The exact positions of the scroll handle are not yet known. Probably just a
// simple calculation.
//
// Clicking on an arrow scrolls up or down by one row. Clicking and holding
// scrolls the inventory. The time between scrolling is constant, i.e. the
// first delay is not different. The delay seems to depend on the speed of the
// Mac, so pick something that feels right.
//
// Clicking above or below the handle scrolls up or down by - probably - one
// page. I've never seen the inventory full enough for this to mean anything
// else than scrolling to the top or bottom.
//
// Dragging the handle is not possible. Clicking on the handle is probably
// indistinguishable from clicking above or below.

const uint16 MacIndy3Gui::Inventory::_upArrow[16] = {
	0x0000, 0x0000, 0x0000, 0x0080,	0x0140, 0x0220, 0x0410, 0x0808,
	0x1C1C, 0x0410, 0x0410, 0x0410,	0x07F0, 0x0000, 0x0000, 0x0000
};

const uint16 MacIndy3Gui::Inventory::_downArrow[16] = {
	0x0000, 0x0000, 0x0000, 0x0000,	0x07F0, 0x0410, 0x0410, 0x0410,
	0x1C1C, 0x0808, 0x0410, 0x0220,	0x0140, 0x0080, 0x0000, 0x0000
};

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

	_scrollBar = new ScrollBar(_bounds.right - 20, _bounds.top + 19, 16, 40);

	x = _bounds.right - 20;
	y = _bounds.top + 4;

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
	MacIndy3Gui::Widget::reset();

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

void MacIndy3Gui::Inventory::updateVerb(int verbslot) {
	MacIndy3Gui::VerbWidget::updateVerb(verbslot);

	int owner = _vm->VAR(_vm->VAR_EGO);

	int invCount = _vm->getInventoryCount(owner);
	int invOffset = _vm->VAR(67);

	// The scroll offset must be non-negative and if there are six or less
	// items in the inventory, the inventory is fixed in the top position.

	if (invOffset < 0 || invCount <= 6)
		invOffset = 0;

	// If there are more than six items in the inventory, clamp the scroll
	// offset to be at most invCount - 6.

	if (invCount > 6 && invOffset >= invCount - 6)
		invOffset = invCount - 6;

	_vm->VAR(67) = invOffset;

	int invSlot = 0;
	invCount = 0;

	for (int i = 0; i < _vm->_numInventory && invSlot < ARRAYSIZE(_slots); i++) {
		int obj = _vm->_inventory[i];
		if (obj && _vm->getOwner(obj) == owner) {
			if (++invCount >= invOffset) {
				Slot *s = _slots[invSlot];
				const byte *name = _vm->getObjOrActorName(obj);

				if (name) {
					if (s->_name != (const char *)name) {
						s->_name = (const char *)name;
						s->_timer = 0;
						s->setRedraw(true);
					}
				} else {
					if (!s->_name.empty()) {
						s->_name.clear();
						s->_timer = 0;
						s->setRedraw(true);
					}
				}

				s->_obj = obj;
				invSlot++;
			}
		}
	}

	for (int i = invSlot; i < ARRAYSIZE(_slots); i++) {
		Slot *s = _slots[i];

		if (!s->_name.empty()) {
			s->_name.clear();
			s->_obj = -1;
			s->_timer = 0;
			s->setRedraw(true);
		}
	}
}

bool MacIndy3Gui::Inventory::handleEvent(Common::Event &event) {
	if (!_verbid || !_enabled)
		return false;

	if (event.type != Common::EVENT_LBUTTONDOWN)
		return false;

	for (int i = 0; i < ARRAYSIZE(_slots); i++) {
		if (_slots[i]->handleEvent(event))
			return true;
	}

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++) {
		if (_scrollButtons[i]->handleEvent(event))
			return true;
	}

	return false;
}

bool MacIndy3Gui::Inventory::updateTimer(int delta) {
	VerbWidget::updateTimer(delta);

	for (int i = 0; i < ARRAYSIZE(_slots); i++) {
		Slot *s = _slots[i];

		if (s->updateTimer(delta)) {
			s->setRedraw(true);
			_vm->runInputScript(kInventoryClickArea, s->_obj, 1);
		}
	}

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++) {
		ScrollButton *b = _scrollButtons[i];

		if (b->updateTimer(delta)) {
			b->setRedraw(true);
		}
	}

	return false;
}

void MacIndy3Gui::Inventory::draw() {
	if (_redraw) {
		debug("Drawing button [%d] Inventory", _verbid);

		MacIndy3Gui::VerbWidget::draw();

		drawShadowBox(_bounds);
		drawShadowFrame(Common::Rect(_bounds.left + 4, _bounds.top + 4, _bounds.right - 22, _bounds.bottom - 4), 0, 15);

		for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++) {
			ScrollButton *s = _scrollButtons[i];
			const uint16 *arrow = (s->_direction == kScrollUp) ? _upArrow : _downArrow;

			drawShadowFrame(s->_bounds, 15, 255);
			drawBitmap(s->_bounds, arrow, 0);
			s->draw();
		}

		copyRectToScreen(_bounds);
	}

	// Since the inventory slots overlap, draw the highlighted ones (and
	// there should really only be one at a time) last.

	for (int i = 0; i < ARRAYSIZE(_slots); i++) {
		if (!_slots[i]->_timer)
			_slots[i]->draw();
	}

	for (int i = 0; i < ARRAYSIZE(_slots); i++) {
		if (_slots[i]->_timer)
			_slots[i]->draw();
	}

	_scrollBar->draw();

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++)
		_scrollButtons[i]->draw();

	setRedraw(false);
}

MacIndy3Gui::Inventory::Slot::Slot(int slot, int x, int y, int width, int height) : MacIndy3Gui::Widget(x, y, width, height) {
	_slot = slot;
}

void MacIndy3Gui::Inventory::Slot::reset() {
	Widget::reset();
	_name.clear();
	_obj = -1;
}

bool MacIndy3Gui::Inventory::Slot::handleEvent(Common::Event &event) {
	if (event.type != Common::EVENT_LBUTTONDOWN)
		return false;

	if (_obj != -1 && _bounds.contains(event.mouse.x, event.mouse.y)) {
		setRedraw(true);
		_timer = 12;
		return true;
	}

	return false;
}

bool MacIndy3Gui::Inventory::Slot::updateTimer(int delta) {
	bool ret = Widget::updateTimer(delta);

	if (ret) {
		_vm->runInputScript(kInventoryClickArea, _obj, 1);
		setRedraw(true);
	}

	return ret;
}

void MacIndy3Gui::Inventory::Slot::draw() {
	if (!_redraw)
		return;

	debug("Drawing inventory slot: [%d] %s", _slot, _name.c_str());
	int fg, bg;

	if (_timer) {
		fg = 15;
		bg = 0;
	} else {
		fg = 0;
		bg = 15;
	}

	_surface->fillRect(_bounds, bg);

	if (!_name.empty()) {
		const Graphics::Font *font = _gui->getFont(0);

		int y = _bounds.top - 1;
		int x = _bounds.left + 4;

		for (uint i = 0; i < _name.size() && x < _bounds.right; i++) {
			if (_name[i] != '@') {
				font->drawChar(_surface, _name[i], x, y, fg);
				x += font->getCharWidth(_name[i]);
			}
		}
	}

	setRedraw(false);
	copyRectToScreen(_bounds);
}

MacIndy3Gui::Inventory::ScrollBar::ScrollBar(int x, int y, int width, int height) : MacIndy3Gui::Widget(x, y, width, height) {
}

bool MacIndy3Gui::Inventory::ScrollBar::handleEvent(Common::Event &event) {
	if (event.type != Common::EVENT_LBUTTONDOWN)
		return false;

	return false;
}

void MacIndy3Gui::Inventory::ScrollBar::draw() {
	if (!_redraw)
		return;

	debug("Drawing scrollbar");
	drawShadowFrame(_bounds, 0, 255);
	copyRectToScreen(_bounds);
	setRedraw(false);
}

const uint16 MacIndy3Gui::Inventory::ScrollButton::_upArrow[16] = {
	0x0000, 0x0000, 0x0000, 0x0000,	0x0080, 0x01C0, 0x03E0, 0x07F0,
	0x03E0, 0x03E0, 0x03E0, 0x03E0,	0x0000, 0x0000, 0x0000, 0x0000
};

const uint16 MacIndy3Gui::Inventory::ScrollButton::_downArrow[16] = {
	0x0000, 0x0000, 0x0000, 0x0000,	0x0000, 0x03E0, 0x03E0, 0x03E0,
	0x03E0, 0x07F0,	0x03E0, 0x01C0,	0x0080, 0x0000, 0x0000, 0x0000
};

MacIndy3Gui::Inventory::ScrollButton::ScrollButton(int x, int y, int width, int height, ScrollDirection direction) : MacIndy3Gui::Widget(x, y, width, height) {
	_direction = direction;
}

bool MacIndy3Gui::Inventory::ScrollButton::handleEvent(Common::Event &event) {
	if (event.type != Common::EVENT_LBUTTONDOWN)
		return false;

	if (_enabled && _bounds.contains(event.mouse.x, event.mouse.y)) {
		setRedraw(true);
		_timer = 12;
		return true;
	}

	return false;
}

void MacIndy3Gui::Inventory::ScrollButton::draw() {
	if (!_redraw)
		return;

	debug("Drawing inventory arrow %d", _direction);

	setRedraw(false);

	const uint16 *arrow = (_direction == kScrollUp) ? _upArrow : _downArrow;
	byte color = _timer ? 0 : 15;

	drawBitmap(_bounds, arrow, color);
	setRedraw(false);
	copyRectToScreen(_bounds);
}

MacIndy3Gui::MacIndy3Gui(OSystem *system, ScummEngine *vm) :
	_system(system), _vm(vm), _macScreen(vm->_macScreen), _visible(false) {
	Graphics::MacFontManager *mfm = _vm->_macFontManager;

	_fonts[0] = mfm->getFont(Graphics::MacFont(Graphics::kMacFontGeneva, 9));
	_fonts[1] = mfm->getFont(Graphics::MacFont(Graphics::kMacFontGeneva, 9, Graphics::kMacFontBold));
	_fonts[2] = mfm->getFont(Graphics::MacFont(Graphics::kMacFontGeneva, 9, Graphics::kMacFontBold | Graphics::kMacFontOutline | Graphics::kMacFontCondense));

	// There is one widget for every verb in the game. Verbs include the
	// inventory widget and conversation options.

	Widget::_vm = _vm;
	Widget::_surface = _macScreen;
	Widget::_gui = this;

	_widgets[  1] = new Button(137, 312,  68, 18); // Open
	_widgets[  2] = new Button(137, 332,  68, 18); // Close
	_widgets[  3] = new Button( 67, 352,  68, 18); // Give
	_widgets[  4] = new Button(277, 332,  68, 18); // Turn on
	_widgets[  5] = new Button(277, 352,  68, 18); // Turn off
	_widgets[  6] = new Button( 67, 312,  68, 18); // Push
	_widgets[  7] = new Button( 67, 332,  68, 18); // Pull
	_widgets[  8] = new Button(277, 312,  68, 18); // Use
	_widgets[  9] = new Button(137, 352,  68, 18); // Look at
	_widgets[ 10] = new Button(207, 312,  68, 18); // Walk to
	_widgets[ 11] = new Button(207, 332,  68, 18); // Pick up
	_widgets[ 12] = new Button(207, 352,  68, 18); // What is
	_widgets[ 13] = new Button(347, 312,  68, 18); // Talk
	_widgets[ 14] = new Button( 97, 312, 121, 18); // Never mind.
	_widgets[ 32] = new Button(347, 332,  68, 18); // Travel
	_widgets[ 33] = new Button(347, 352,  68, 18); // To Indy
	_widgets[ 34] = new Button(347, 352,  68, 18); // To Henry
	_widgets[ 90] = new Button( 67, 292, 507, 18); // Travel 1
	_widgets[ 91] = new Button( 67, 312, 507, 18); // Travel 2
	_widgets[ 92] = new Button( 67, 332, 507, 18); // Travel 3
	_widgets[100] = new Button( 67, 292, 348, 18); // Sentence
	_widgets[101] = new Inventory(417, 292, 157, 78);
	_widgets[119] = new Button(324, 312,  91, 18); // Take this:
	_widgets[120] = new Button( 67, 292, 507, 18); // Converse 1
	_widgets[121] = new Button( 67, 312, 507, 18); // Converse 2
	_widgets[122] = new Button( 67, 332, 507, 18); // Converse 3
	_widgets[123] = new Button( 67, 352, 507, 18); // Converse 4
	_widgets[124] = new Button( 67, 352, 151, 18); // Converse 5
	_widgets[125] = new Button(423, 352, 151, 18); // Converse 6

	for (Common::HashMap<int, VerbWidget *>::iterator i = _widgets.begin(); i != _widgets.end(); ++i)
		i->_value->_verbid = i->_key;
}

MacIndy3Gui::~MacIndy3Gui() {
	for (Common::HashMap<int, VerbWidget *>::iterator i = _widgets.begin(); i != _widgets.end(); ++i)
		delete i->_value;
}

bool MacIndy3Gui::isActive() {
	// The GUI is only allowed if the verb area has the expected size. There
	// are scenes (e.g. the flight path to Venice) where it's not.

	VirtScreen *vs = &_vm->_virtscr[kVerbVirtScreen];
	if (vs->topline != 144 || vs->h != 56)
		return false;

	// The GUI is only allowed for certain verb scripts:
	//
	//   4 - Regular verb GUI
	//  18 - Conversations
	// 200 - Venice, outdoors
	// 201 - Travel to Henry's or Venice
	// 205 - Travel from Venice
	//
	// Other known verb scripts where the GUI is not allowed:
	//
	//  19 - Boxing

	int verbScript = _vm->VAR(_vm->VAR_VERB_SCRIPT);
	int guiVerbScripts[] = { 4, 18, 200, 201, 205 };

	for (int i = 0; i < ARRAYSIZE(guiVerbScripts); i++) {
		if (verbScript == guiVerbScripts[i])
			return true;
	}

	return false;
}

void MacIndy3Gui::resetAfterLoad() {
	_visible = false;

	for (Common::HashMap<int, VerbWidget *>::iterator i = _widgets.begin(); i != _widgets.end(); ++i)
		i->_value->reset();

	// In the DOS version, verb ID 102-106 were used for the visible
	// inventory items, and 107-108 for inventory arrow buttons. In the
	// Macintosh version, the entire inventory widget is verb ID 101.
	//
	// In old savegames, the DOS verb IDs may still be present, and have
	// to be removed.

	bool oldSavegame = false;

	for (int i = 0; i < _vm->_numVerbs; i++) {
		if (_vm->_verbs[i].verbid >= 102 && _vm->_verbs[i].verbid <= 108) {
			_vm->killVerb(i);
			oldSavegame = true;
		}
	}

	// The old GUI adjusted the offset by 2 when scrolling the inventory.

	if (oldSavegame)
		_vm->VAR(67) /= 2;
}

void MacIndy3Gui::update(int delta) {
	static int lastVerbScript = -1;

	if (_vm->VAR(_vm->VAR_VERB_SCRIPT) != lastVerbScript) {
		debug("VAR_VERB_SCRIPT = %d", _vm->VAR(_vm->VAR_VERB_SCRIPT));
		lastVerbScript = _vm->VAR(_vm->VAR_VERB_SCRIPT);
	}

	if (!isActive()) {
		if (isVisible())
			hide();
		return;
	}

	// Tentatively mark the verb widgets for removal. Any widget that wants
	// to stay has to say so.

	for (Common::HashMap<int, VerbWidget *>::iterator i = _widgets.begin(); i != _widgets.end(); ++i) {
		VerbWidget *w = i->_value;

		w->updateTimer(delta);
		w->threaten();
	}

	bool keepGuiAlive = false;

	// Collect all active verbs. Verb slot 0 is special, apparently, so we
	// don't look at that one.

	for (int i = 1; i < _vm->_numVerbs; i++) {
		VerbSlot *vs = &_vm->_verbs[i];

		if (!vs->saveid && vs->curmode && vs->verbid) {
			VerbWidget *w = _widgets.getValOrDefault(vs->verbid);

			if (w) {
				w->updateVerb(i);
				keepGuiAlive = true;
			} else {
				const byte *ptr = _vm->getResourceAddress(rtVerb, i);
				byte buf[270];
				_vm->convertMessageToString(ptr, buf, sizeof(buf));
				debug("Unknown verb: %d %s", vs->verbid, buf);
			}
		}
	}

	if (!keepGuiAlive) {
		hide();
		return;
	}

	if (!isVisible())
		show();

	for (Common::HashMap<int, VerbWidget *>::iterator i = _widgets.begin(); i != _widgets.end(); ++i) {
		VerbWidget *w = i->_value;

		if (w->_kill && w->_visible) {
			w->undraw();
			w->reset();
		}
	}

	for (Common::HashMap<int, VerbWidget *>::iterator i = _widgets.begin(); i != _widgets.end(); ++i) {
		VerbWidget *w = i->_value;

		if (w->_verbslot != -1) {
			w->draw();
		}
	}
}

void MacIndy3Gui::handleEvent(Common::Event &event) {
	if (!isVisible() || _vm->_userPut <= 0)
		return;

	for (Common::HashMap<int, VerbWidget *>::iterator i = _widgets.begin(); i != _widgets.end(); ++i) {
		if (i->_value->handleEvent(event))
			break;
	}
}

void MacIndy3Gui::show() {
	if (isVisible())
		return;

	debug("SHOW");

	clear();
	_visible = true;
}

void MacIndy3Gui::hide() {
	if (!isVisible())
		return;

	_visible = false;

	for (Common::HashMap<int, VerbWidget *>::iterator i = _widgets.begin(); i != _widgets.end(); ++i)
		i->_value->reset();

	if (_vm->_virtscr[kVerbVirtScreen].topline != 144)
		return;

	debug("HIDE");

	_macScreen->fillRect(Common::Rect(0, 288, 640, 400), 0);
	_system->copyRectToScreen(_macScreen->getBasePtr(0, 288), _macScreen->pitch, 0, 288, 640, 112);
}

void MacIndy3Gui::clear() {
	_macScreen->fillRect(Common::Rect(0, 288, 640, 289), 0);
	_macScreen->fillRect(Common::Rect(0, 373, 640, 400), 0);

	fill(Common::Rect(0, 290, 640, 373));
	drawBitmap(Common::Rect(  0, 290,   4, 294), _ulCorner, 0);
	drawBitmap(Common::Rect(636, 290, 640, 294), _urCorner, 0);
	drawBitmap(Common::Rect(  0, 369,   4, 373), _llCorner, 0);
	drawBitmap(Common::Rect(636, 369, 640, 373), _lrCorner, 0);

	copyRectToScreen(Common::Rect(0, 288, 640, 400));
}

void MacIndy3Gui::copyRectToScreen(Common::Rect r) {
	_system->copyRectToScreen(_macScreen->getBasePtr(r.left, r.top), _macScreen->pitch, r.left, r.top, r.width(), r.height());
}

void MacIndy3Gui::fill(Common::Rect r) {
	int pitch = _macScreen->pitch;

	// Fill the screen with either gray of a checkerboard pattern.

	if (_vm->_renderMode == Common::kRenderMacintoshBW) {
		byte *row = (byte *)_macScreen->getBasePtr(r.left, r.top);

		for (int y = r.top; y < r.bottom; y++) {
			byte *ptr = row;
			for (int x = r.left; x < r.right; x++)
				*ptr++ = ((x + y) & 1) ? 15 : 0;
			row += pitch;
		}
	} else
		_macScreen->fillRect(r, 7);
}

void MacIndy3Gui::drawBitmap(Common::Rect r, const uint16 *bitmap, byte color) {
	byte *ptr = (byte *)_macScreen->getBasePtr(r.left, r.top);
	int pitch = _macScreen->pitch;

	assert(r.width() <= 16);

	for (int y = 0; y < r.height(); y++) {
		uint16 bit = 0x8000;
		for (int x = 0; x < r.width(); x++) {
			if (bitmap[y] & bit)
				ptr[x] = color;
			bit >>= 1;
		}
		ptr += pitch;
	}
}

} // End of namespace Scumm
