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
	if (vs->number == kVerbVirtScreen && _macIndy3Gui->isActive())
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

MacIndy3Gui::MacIndy3Gui(OSystem *system, ScummEngine *vm) :
	_system(system), _vm(vm), _macScreen(vm->_macScreen), _visible(false) {
	Graphics::MacFontManager *mfm = _vm->_macFontManager;

	_fonts[0] = mfm->getFont(Graphics::MacFont(Graphics::kMacFontGeneva, 9));
	_fonts[1] = mfm->getFont(Graphics::MacFont(Graphics::kMacFontGeneva, 9, Graphics::kMacFontBold));
	_fonts[2] = mfm->getFont(Graphics::MacFont(Graphics::kMacFontGeneva, 9, Graphics::kMacFontBold | Graphics::kMacFontOutline | Graphics::kMacFontCondense));
	// This list has been arranged so that most of the standard verbs
	// have the same index as their verb number in the game. The rest
	// have been arranged tastefully.
	//
	// I think 101-106 are inventory items.

	initWidget( 0,  67, 292, 348, 18); // 100: Sentence line
	initWidget( 1, 137, 312,  68, 18); // 1: Open
	initWidget( 2, 137, 332,  68, 18); // 2: Close
	initWidget( 3,  67, 352,  68, 18); // 3: Give
	initWidget( 4, 277, 332,  68, 18); // 4: Turn on
	initWidget( 5, 277, 352,  68, 18); // 5: Turn off
	initWidget( 6,  67, 312,  68, 18); // 6: Push
	initWidget( 7,  67, 332,  68, 18); // 7: Pull
	initWidget( 8, 277, 312,  68, 18); // 8: Use
	initWidget( 9, 137, 352,  68, 18); // 9: Look at
	initWidget(10, 207, 312,  68, 18); // 10: Walk to
	initWidget(11, 207, 332,  68, 18); // 11: Pick up
	initWidget(12, 207, 352,  68, 18); // 12: What is
	initWidget(13, 347, 312,  68, 18); // 13: Talk
	initWidget(14,  97, 312, 121, 18); // 14: Never mind.
	initWidget(15, 347, 332,  68, 18); // 32: Travel
	initWidget(16, 324, 312,  91, 18); // 119: Take this:
	initWidget(17,  67, 292, 507, 18); // 120: Conversation 1
	initWidget(18,  67, 312, 507, 18); // 121: Conversation 2
	initWidget(19,  67, 332, 507, 18); // 122: Conversation 3
	initWidget(20,  67, 352, 507, 18); // 123: Conversation 4
	initWidget(21,  67, 352, 151, 18); // Conversation 5
	initWidget(22, 423, 352, 151, 18); // Conversation 6
}

MacIndy3Gui::~MacIndy3Gui() {
	for (int i = 0; i < ARRAYSIZE(_widgets); i++)
		free(_widgets[i].text);
}

void MacIndy3Gui::initWidget(int n, int x, int y, int width, int height) {
	Widget *w = &_widgets[n];

	w->bounds.left = x;
	w->bounds.top = y;
	w->bounds.right = x + width;
	w->bounds.bottom = y + height;
	w->text = nullptr;

	resetWidget(n);
}

void MacIndy3Gui::resetWidget(int n) {
	Widget *w = &_widgets[n];

	free(w->text);

	w->text = nullptr;
	w->slot = -1;
	w->timer = 0;
	w->visible = false;
	w->enabled = false;
	w->redraw = false;
	w->kill = false;
}

bool MacIndy3Gui::isActive() {
	int verbScript = _vm->VAR(_vm->VAR_VERB_SCRIPT);
	return verbScript == 4 || verbScript == 18;
}

void MacIndy3Gui::resetAfterLoad() {
	_visible = false;

	for (int i = 0; i < ARRAYSIZE(_widgets); i++)
		resetWidget(i);
}

void MacIndy3Gui::update() {
	if (!isActive()) {
		if (_visible)
			hide();
		return;
	}

	if (!_visible)
		show();

	// Decrease all timers
	for (int i = 0; i < ARRAYSIZE(_widgets); i++) {
		Widget *w = &_widgets[i];
		if (w->timer > 0) {
			w->timer--;
			if (w->timer == 0 && w->visible)
				w->redraw = true;
		}
		w->kill = true;
	}

	bool keepGuiAlive = false;

	for (int i = 1; i < _vm->_numVerbs; i++) {
		VerbSlot *vs = &_vm->_verbs[i];

		if (!vs->saveid && vs->curmode && vs->verbid) {
			int id = -1;

			if (vs->verbid == 100) {
				id = 0;
			} else if (vs->verbid >= 1 && vs->verbid <= 14) {
				id = vs->verbid;
			} else if (vs->verbid == 32) {
				id = 15;
			} else if (vs->verbid >= 119 && vs->verbid <= 125) {
				id = vs->verbid - 103;
			} else {
				const byte *ptr = _vm->getResourceAddress(rtVerb, i);
				byte buf[270];
				_vm->convertMessageToString(ptr, buf, sizeof(buf));
				debug("Unknown verb: %d %s", vs->verbid, buf);
			}

			if (id != -1) {
				Widget *w = &_widgets[id];
				bool enabled = (vs->curmode == 1);

				w->slot = i;

				const byte *ptr = _vm->getResourceAddress(rtVerb, w->slot);
				byte buf[270];

				_vm->convertMessageToString(ptr, buf, sizeof(buf));
				if (w->text == nullptr || strcmp((char *)w->text, (char *)buf) != 0) {
					free(w->text);
					w->text = (byte *)scumm_strdup((const char *)buf);
					w->redraw = true;
				}

				if (!w->visible || w->redraw || w->enabled != enabled)
					w->redraw = true;
				w->kill = false;
				keepGuiAlive = true;
			}
		}
	}

	for (int i = 0; i < ARRAYSIZE(_widgets); i++) {
		Widget *w = &_widgets[i];

		if (w->kill && w->visible) {
			fill(w->bounds);
			free(w->text);
			w->text = nullptr;
			w->slot = -1;
			w->timer = 0;
			w->enabled = false;
			w->visible = false;
			w->redraw = false;
		}
	}

	for (int i = 0; i < ARRAYSIZE(_widgets); i++) {
		Widget *w = &_widgets[i];

		if (w->slot == -1)
			continue;

		if (w->redraw) {
			VerbSlot *vs = &_vm->_verbs[w->slot];
			debug("Drawing button %d: (%d) %s", i, vs->verbid, w->text);
			drawButton(i, w->text, vs->curmode != 2, w->timer);
			w->visible = true;
			w->enabled = (vs->curmode == 1);
			w->redraw = false;
		}
	}

	if (!keepGuiAlive)
		hide();
}

void MacIndy3Gui::handleEvent(Common::Event &event) {
	if (event.type != Common::EVENT_LBUTTONDOWN)
		return;

	int x = event.mouse.x;
	int y = event.mouse.y;

	for (int i = 0; i < ARRAYSIZE(_widgets); i++) {
		Widget *w = &_widgets[i];

		if (w->visible && w->enabled) {
			if (w->bounds.contains(x, y)) {
				w->redraw = true;
				w->timer = 15;

				if (w->slot != -1) {
					VerbSlot *vs = &_vm->_verbs[w->slot];
					_vm->runInputScript(kVerbClickArea, vs->verbid, 1);
				}
			}
		}
	}
}

void MacIndy3Gui::show() {
	if (_visible)
		return;

	clear();
	_visible = true;
}

void MacIndy3Gui::hide() {
	if (!_visible)
		return;

	_visible = false;

	for (int i = 0; i < ARRAYSIZE(_widgets); i++)
		resetWidget(i);

	_macScreen->fillRect(Common::Rect(0, 288, 640, 400), 0);
	_system->copyRectToScreen(_macScreen->getBasePtr(0, 288), _macScreen->pitch, 0, 288, 640, 112);
}

void MacIndy3Gui::clear() {
	_macScreen->fillRect(Common::Rect(0, 288, 640, 289), 0);
	fill(Common::Rect(0, 290, 640, 373));
	_macScreen->fillRect(Common::Rect(0, 373, 640, 400), 0);

	const byte corner[] = {
		1, 1, 1, 1,
		1, 1, 0, 0,
		1, 0, 0, 0,
		1, 0, 0, 0
	};

	byte *ul = (byte *)_macScreen->getBasePtr(0, 290);
	byte *ur = (byte *)_macScreen->getBasePtr(639, 290);
	byte *ll = (byte *)_macScreen->getBasePtr(0, 372);
	byte *lr = (byte *)_macScreen->getBasePtr(639, 372);

	int pitch = _macScreen->pitch;

	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			if (corner[y * 4 + x]) {
				*(ul + y * pitch + x) = 0;
				*(ur + y * pitch - x) = 0;
				*(ll - y * pitch + x) = 0;
				*(lr - y * pitch - x) = 0;
			}
		}
	}

	_system->copyRectToScreen(_macScreen->getBasePtr(0, 288), _macScreen->pitch, 0, 288, 640, 112);
}

void MacIndy3Gui::fill(Common::Rect r) {
	int pitch = _macScreen->pitch;

	if (_vm->_renderMode == Common::kRenderMacintoshBW) {
		byte *row = (byte *)_macScreen->getBasePtr(r.left, r.top);

		for (int y = r.top; y < r.bottom; y++) {
			byte *ptr = row;
			for (int x = r.left; x < r.right; x++) {
				*ptr++ = ((x + y) & 1) ? 15 : 0;
			}
			row += pitch;
		}
	} else
		_macScreen->fillRect(r, 7);

	_system->copyRectToScreen(_macScreen->getBasePtr(r.left, r.top), pitch, r.left, r.top, r.width(), r.height());
}

void MacIndy3Gui::drawButton(int n, byte *text, bool enabled, bool pressed) {
	Common::Rect r = _widgets[n].bounds;

	fill(r);

	if (!pressed) {
		_macScreen->hLine(r.left + 1, r.top, r.right - 3, 0);
		_macScreen->hLine(r.left + 1, r.bottom - 2, r.right - 3, 0);
		_macScreen->vLine(r.left, r.top + 1, r.bottom - 3, 0);
		_macScreen->vLine(r.right - 2, r.top + 1, r.bottom - 3, 0);

		_macScreen->hLine(r.left + 2, r.bottom - 1, r.right - 1, 0);
		_macScreen->vLine(r.right - 1, r.top + 2, r.bottom - 2, 0);

		_macScreen->hLine(r.left + 1, r.top + 1, r.right - 3, 15);
		_macScreen->vLine(r.left + 1, r.top + 2, r.bottom - 3, 15);
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
		_macScreen->hLine(r.left + 2, r.top + 1, r.right - 2, 0);
		_macScreen->hLine(r.left + 2, r.bottom - 1, r.right - 1, 0);
		_macScreen->vLine(r.left + 1, r.top + 2, r.bottom - 2, 0);
		_macScreen->vLine(r.right - 1, r.top + 2, r.bottom - 2, 0);

		_macScreen->hLine(r.left + 2, r.top + 2, r.right - 2, 15);
		_macScreen->vLine(r.left + 2, r.top + 3, r.bottom - 2, 15);
	}

	// The text is drawn centered. Based on experimentation, I think the
	// width is always based on the outlined font, and the button shadow is
	// not counted as part of the button width.
	//
	// This gives us pixel perfect rendering for the English verbs.

	if (text) {
		int stringWidth = 0;
		for (int i = 0; text[i]; i++)
			stringWidth += _fonts[2]->getCharWidth(text[i]);

		int x = (r.left + (r.width() - 1 - stringWidth) / 2) - 1;
		int y = r.top + 2;
		int color = enabled ? 15 : 0;

		if (pressed) {
			x++;
			y++;
		}

		for (int i = 0; text[i]; i++) {
			if (enabled)
				_fonts[2]->drawChar(_macScreen, text[i], x, y, 0);
			_fonts[1]->drawChar(_macScreen, text[i], x + 1, y, color);
			x += _fonts[2]->getCharWidth(text[i]);
		}
	}

	_system->copyRectToScreen(_macScreen->getBasePtr(r.left, r.top), _macScreen->pitch, r.left, r.top, r.width(), r.height());
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

void MacIndy3Gui::drawInventoryWidget() {
	fill(Common::Rect(417, 292, 574, 370));

	// Main outline and shadow
	_macScreen->hLine(418, 292, 571, 0);
	_macScreen->hLine(418, 368, 571, 0);
	_macScreen->vLine(417, 293, 367, 0);
	_macScreen->vLine(572, 293, 367, 0);

	_macScreen->hLine(419, 369, 573, 0);
	_macScreen->vLine(573, 294, 368, 0);

	_macScreen->hLine(418, 293, 571, 15);
	_macScreen->vLine(418, 294, 367, 15);

	// Inner frame
	_macScreen->hLine(421, 296, 551, 0);
	_macScreen->hLine(421, 297, 551, 0);
	_macScreen->hLine(421, 365, 551, 0);
	_macScreen->vLine(421, 298, 364, 0);
	_macScreen->vLine(422, 298, 364, 0);
	_macScreen->vLine(551, 298, 364, 0);

	_macScreen->fillRect(Common::Rect(423, 298, 551, 365), 15);

	// Scrollbar, outer frame
	_macScreen->hLine(554, 296, 569, 0);
	_macScreen->hLine(554, 365, 569, 0);
	_macScreen->vLine(554, 297, 364, 0);
	_macScreen->vLine(569, 297, 364, 0);

	drawInventoryScrollbar();

	_macScreen->hLine(555, 297, 568, 15);
	_macScreen->vLine(555, 298, 310, 15);
	_macScreen->hLine(555, 351, 568, 15);
	_macScreen->vLine(555, 352, 364, 15);

	drawInventoryArrowUp(false);
	drawInventoryArrowDown(false);
}

void MacIndy3Gui::drawInventoryArrow(int arrowX, int arrowY, bool highlighted, bool flipped) {
	const byte arrow[110] = {
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0,
		0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0,
		0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0,
		0, 1, 2, 2, 2, 2, 2, 2, 2, 1, 0,
		1, 1, 1, 2, 2, 2, 2, 2, 1, 1, 1,
		0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0,
		0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0,
		0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0,
		0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0
	};

	byte palette[] = { 0, 15 };

	if (highlighted)
		palette[1] = 0;

	int y0, y1, yd;

	if (flipped) {
		y0 = 9;
		y1 = 0;
		yd = -1;
	} else {
		y0 = 0;
		y1 = 9;
		yd = 1;
	}

	byte *ptr = (byte *)_macScreen->getBasePtr(arrowX, arrowY);
	int pitch = _macScreen->pitch;

	int y = y0 - yd;
	do {
		y += yd;
		for (int x = 0; x < 11; x++) {
			byte color = arrow[11 * y + x];
			if (color)
				ptr[x] = palette[color - 1];
		}
		ptr += pitch;
	} while (y != y1);
}

void MacIndy3Gui::drawInventoryArrowUp(bool highlighted) {
	drawInventoryArrow(557, 299, highlighted, false);
}

void MacIndy3Gui::drawInventoryArrowDown(bool highlighted) {
	drawInventoryArrow(557, 354, highlighted, true);
}

void MacIndy3Gui::drawInventoryScrollbar() {
	_macScreen->hLine(555, 311, 568, 0);
	_macScreen->hLine(555, 350, 568, 0);

	_macScreen->hLine(555, 312, 568, 0);
	_macScreen->vLine(555, 313, 349, 0);

	fill(Common::Rect(556, 313, 569, 350));
}

void MacIndy3Gui::drawInventoryText(int slot, char *text, bool highlighted) {
	int slotX = 423;
	int slotY = 298 + slot * 11;

	int fg, bg;

	if (highlighted) {
		fg = 15;
		bg = 0;
	} else {
		fg = 0;
		bg = 15;
	}

#if 0
	// The inventory slots overlap slightly, so we have to clear the entire
	// area, then highlight the at most single one that's highlighted.
	_macScreen->fillRect(Common::Rect(423, 298, 551, 365), 15);
#endif

	int height = 12;
	int width = 128;

	if (highlighted)
		_macScreen->fillRect(Common::Rect(slotX, slotY, slotX + width, slotY + height), bg);

	int y = slotY - 1;
	int x = slotX + 4;

	for (int i = 0; text[i]; i++) {
		_fonts[0]->drawChar(_macScreen, text[i], x, y, fg);
		x += _fonts[0]->getCharWidth(text[i]);
	}
}

} // End of namespace Scumm
