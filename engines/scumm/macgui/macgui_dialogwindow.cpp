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

#include "graphics/cursorman.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/primitives.h"
#include "graphics/surface.h"

#include "scumm/scumm.h"
#include "scumm/detection.h"
#include "scumm/macgui/macgui_impl.h"

namespace Scumm {

// ---------------------------------------------------------------------------
// Dialog window
//
// This can either be used as a modal dialog (options, etc.), or as a framed
// drawing area (about and pause). It can not be dragged.
// ---------------------------------------------------------------------------

MacGuiImpl::MacDialogWindow::MacDialogWindow(MacGuiImpl *gui, OSystem *system, Graphics::Surface *from, Common::Rect bounds, MacDialogWindowStyle style) : _gui(gui), _system(system), _from(from), _bounds(bounds) {
	_pauseToken = _gui->_vm->pauseEngine();

	// Remember if the screen was shaking. We don't use the SCUMM engine's
	// own _shakeTempSavedState to remember this, because there may be
	// nested dialogs. They each need their own.

	_shakeWasEnabled = _gui->_vm->_shakeEnabled;
	_gui->_vm->setShake(0);

	_backup = new Graphics::Surface();
	_backup->create(bounds.width(), bounds.height(), Graphics::PixelFormat::createFormatCLUT8());
	_backup->copyRectToSurface(*_from, 0, 0, bounds);

	_margin = (style == kStyleNormal) ? 6 : 4;

	_surface = _from->getSubArea(bounds);
	bounds.grow(-_margin);
	_innerSurface = _from->getSubArea(bounds);

	_dirtyRects.clear();

	Graphics::Surface *s = surface();
	Common::Rect r = Common::Rect(s->w, s->h);

	r.grow(-1);
	s->fillRect(r, kWhite);

	if (style == kStyleNormal) {
		int growths[] = { 1, -3, -1 };

		for (int i = 0; i < ARRAYSIZE(growths); i++) {
			r.grow(growths[i]);

			s->hLine(r.left, r.top, r.right - 1, kBlack);
			s->hLine(r.left, r.bottom - 1, r.right - 1, kBlack);
			s->vLine(r.left, r.top + 1, r.bottom - 2, kBlack);
			s->vLine(r.right - 1, r.top + 1, r.bottom - 2, kBlack);
		}
	} else if (style == kStyleRounded) {
		r.grow(1);

		for (int i = 0; i < 2; i++) {
			s->hLine(r.left + 2, r.top, r.right - 3, kBlack);
			s->hLine(r.left + 2, r.bottom - 1, r.right - 3, kBlack);
			s->vLine(r.left, r.top + 2, r.bottom - 3, kBlack);
			s->vLine(r.right - 1, r.top + 2, r.bottom - 3, kBlack);
			s->setPixel(r.left + 1, r.top + 1, kBlack);
			s->setPixel(r.left + 1, r.bottom - 2, kBlack);
			s->setPixel(r.right - 2, r.top + 1, kBlack);
			s->setPixel(r.right - 2, r.bottom - 2, kBlack);
			r.grow(-2);
		}
	}

	// The menu bar isn't part of the Mac screen. We copy it to the Mac
	// screen so that the beam cursor is correctly drawn if it ever moves
	// that far up. There's no reason for it to, but it can happen.

	Graphics::Surface *screen = _gui->surface();
	Graphics::Surface *realScreen = _system->lockScreen();

	screen->copyRectToSurface(*realScreen, 0, 0, Common::Rect(0, 0, 640, 19));

	_system->unlockScreen();
}

MacGuiImpl::MacDialogWindow::~MacDialogWindow() {
	if (!CursorMan.isVisible())
		CursorMan.showMouse(true);

	CursorMan.showMouse(_cursorWasVisible);
	_gui->_windowManager->popCursor();

	copyToScreen(_backup);
	_backup->free();
	delete _backup;

	for (uint i = 0; i < _widgets.size(); i++)
		delete _widgets[i];

	_widgets.clear();
	_pauseToken.clear();
	_gui->_vm->setShake(_shakeWasEnabled);
}

void MacGuiImpl::MacDialogWindow::copyToScreen(Graphics::Surface *s) const {
	if (s) {
		_from->copyRectToSurface(*s, _bounds.left, _bounds.top, Common::Rect(_bounds.width(), _bounds.height()));
	}
	_system->copyRectToScreen(_from->getBasePtr(_bounds.left, _bounds.top), _from->pitch, _bounds.left, _bounds.top, _bounds.width(), _bounds.height());
}

void MacGuiImpl::MacDialogWindow::show() {
	_visible = true;
	copyToScreen();
	_dirtyRects.clear();
	_gui->_windowManager->pushCursor(Graphics::MacGUIConstants::kMacCursorArrow);
	_cursorWasVisible = CursorMan.showMouse(true);
}

void MacGuiImpl::MacDialogWindow::setFocusedWidget(int x, int y) {
	int nr = findWidget(x, y);
	if (nr >= 0) {
		_focusedWidget = _widgets[nr];
		_focusClick.x = x;
		_focusClick.y = y;
		_focusedWidget->getFocus();
	} else
		clearFocusedWidget();
}

void MacGuiImpl::MacDialogWindow::clearFocusedWidget() {
	if (_focusedWidget) {
		_focusedWidget->loseFocus();
		_focusedWidget = nullptr;
		_focusClick.x = -1;
		_focusClick.y = -1;
	}
}

int MacGuiImpl::MacDialogWindow::findWidget(int x, int y) const {
	for (uint i = 0; i < _widgets.size(); i++) {
		if (_widgets[i]->isEnabled() && _widgets[i]->isVisible() && _widgets[i]->findWidget(x, y))
			return i;
	}

	return -1;
}

MacGuiImpl::MacButton *MacGuiImpl::MacDialogWindow::addButton(Common::Rect bounds, Common::String text, bool enabled) {
	MacGuiImpl::MacButton *button = new MacButton(this, bounds, text, enabled);
	_widgets.push_back(button);
	return button;
}

MacGuiImpl::MacCheckbox *MacGuiImpl::MacDialogWindow::addCheckbox(Common::Rect bounds, Common::String text, bool enabled) {
	MacGuiImpl::MacCheckbox *checkbox = new MacCheckbox(this, bounds, text, enabled);
	_widgets.push_back(checkbox);
	return checkbox;
}

MacGuiImpl::MacStaticText *MacGuiImpl::MacDialogWindow::addStaticText(Common::Rect bounds, Common::String text, bool enabled, Graphics::TextAlign alignment) {
	MacGuiImpl::MacStaticText *staticText = new MacStaticText(this, bounds, text, enabled, alignment);
	_widgets.push_back(staticText);
	return staticText;
}

MacGuiImpl::MacEditText *MacGuiImpl::MacDialogWindow::addEditText(Common::Rect bounds, Common::String text, bool enabled) {
	MacGuiImpl::MacEditText *editText = new MacEditText(this, bounds, text, enabled);
	_widgets.push_back(editText);
	return editText;
}

MacGuiImpl::MacPicture *MacGuiImpl::MacDialogWindow::addPicture(Common::Rect bounds, int id, bool enabled) {
	MacGuiImpl::MacPicture *picture = new MacPicture(this, bounds, id, false);
	_widgets.push_back(picture);
	return picture;
}

MacGuiImpl::MacSlider *MacGuiImpl::MacDialogWindow::addSlider(int x, int y, int h, int minValue, int maxValue, int pageSize, bool enabled) {
	MacGuiImpl::MacSlider *slider = new MacSlider(this, Common::Rect(x, y, x + 16, y + h), minValue, maxValue, pageSize, enabled);
	_widgets.push_back(slider);
	return slider;
}

MacGuiImpl::MacPictureSlider *MacGuiImpl::MacDialogWindow::addPictureSlider(int backgroundId, int handleId, bool enabled, int minX, int maxX, int minValue, int maxValue, int leftMargin, int rightMargin) {
	MacPicture *background = (MacPicture *)_widgets[backgroundId];
	MacPicture *handle = (MacPicture *)_widgets[handleId];

	background->setVisible(false);
	handle->setVisible(false);

	MacGuiImpl::MacPictureSlider *slider = new MacPictureSlider(this, background, handle, enabled, minX, maxX, minValue, maxValue, leftMargin, rightMargin);
	_widgets.push_back(slider);
	return slider;
}

void MacGuiImpl::MacDialogWindow::markRectAsDirty(Common::Rect r) {
	_dirtyRects.push_back(r);
}

MacGuiImpl::MacListBox *MacGuiImpl::MacDialogWindow::addListBox(Common::Rect bounds, Common::StringArray texts, bool enabled, bool contentUntouchable) {
	MacGuiImpl::MacListBox *listBox = new MacListBox(this, bounds, texts, enabled, contentUntouchable);
	_widgets.push_back(listBox);
	return listBox;
}

void MacGuiImpl::MacDialogWindow::drawBeamCursor() {
	int x0 = _beamCursorPos.x - _beamCursorHotspotX;
	int y0 = _beamCursorPos.y - _beamCursorHotspotY;
	int x1 = x0 + _beamCursor->w;
	int y1 = y0 + _beamCursor->h;

	Graphics::Surface *screen = _gui->surface();

	_beamCursor->copyRectToSurface(*screen, 0, 0, Common::Rect(x0, y0, x1, y1));

	const byte beam[] = {
		0,  0,  1,  0,  5,  0,  6,  0,  2,  1,  4,  1,  3,  2,  3,  3,
		3,  4,  3,  5,  3,  6,  3,  7,  3,  8,  3,  9,  3, 10,  3, 11,
		3, 12,  3, 13,  2, 14,  4, 14,  0, 15,  1, 15,  5, 15,  6, 15
	};

	for (int i = 0; i < ARRAYSIZE(beam); i += 2) {
		uint32 color = _beamCursor->getPixel(beam[i], beam[i + 1]);

		// From looking at a couple of colors, it seems this is how
		// the colors are inverted for 0-15. I'm just going to assume
		// that the same method will work reasonably well for the
		// custom colors. If there's anything else, just make it black.

		if (color <= 15)
			color = 15 - color;
		else if (color > kCustomColor && color <= kCustomColor + 15)
			color = kCustomColor + 15 - color;
		else
			color = kBlack;

		_beamCursor->setPixel(beam[i], beam[i + 1], color);
	}

	int srcX = 0;
	int srcY = 0;

	if (x0 < 0) {
		srcX = -x0;
		x0 = 0;
	}

	x1 = MIN<int>(x1, screen->w);

	if (y0 < 0) {
		srcY = -y0;
		y0 = 0;
	}

	y1 = MIN<int>(y1, screen->h);

	_system->copyRectToScreen(_beamCursor->getBasePtr(srcX, srcY), _beamCursor->pitch, x0, y0, x1 - x0, y1 - y0);
}

void MacGuiImpl::MacDialogWindow::undrawBeamCursor() {
		int x0 = _beamCursorPos.x - _beamCursorHotspotX;
		int y0 = _beamCursorPos.y - _beamCursorHotspotY;
		int x1 = x0 + _beamCursor->w;
		int y1 = y0 + _beamCursor->h;

		Graphics::Surface *screen = _gui->surface();

		x0 = MAX(x0, 0);
		x1 = MIN<int>(x1, screen->w);
		y0 = MAX(y0, 0);
		y1 = MIN<int>(y1, screen->h);

		_system->copyRectToScreen(screen->getBasePtr(x0, y0), screen->pitch, x0, y0, x1 - x0, y1 - y0);
}

void MacGuiImpl::MacDialogWindow::update(bool fullRedraw) {
	for (uint i = 0; i < _widgets.size(); i++) {
		if (_widgets[i]->isVisible())
			_widgets[i]->draw();
	}

	if (fullRedraw) {
		_dirtyRects.clear();
		markRectAsDirty(Common::Rect(_innerSurface.w, _innerSurface.h));
	}

	for (uint i = 0; i < _dirtyRects.size(); i++) {
		_system->copyRectToScreen(
			_innerSurface.getBasePtr(_dirtyRects[i].left, _dirtyRects[i].top),
			_innerSurface.pitch,
			_bounds.left + _margin + _dirtyRects[i].left, _bounds.top + _margin + _dirtyRects[i].top,
			_dirtyRects[i].width(), _dirtyRects[i].height());
	}

	_dirtyRects.clear();

	if (_beamCursor) {
		if (_beamCursorVisible)
			undrawBeamCursor();

		_beamCursorPos = _realMousePos;

		if (_beamCursorVisible)
			drawBeamCursor();
	}
}

void MacGuiImpl::MacDialogWindow::drawDottedHLine(int x0, int y, int x1) {
	Graphics::Surface *s = innerSurface();

	Color color[2];

	if (_gui->_vm->_renderMode == Common::kRenderMacintoshBW) {
		color[0] = kBlack;
		color[1] = kWhite;
	} else {
		color[0] = kDarkGray;
		color[1] = kLightGray;
	}

	for (int x = x0; x <= x1; x++)
		s->setPixel(x, y, color[x & 1]);
}

void MacGuiImpl::MacDialogWindow::fillPattern(Common::Rect r, uint16 pattern) {
	for (int y = r.top; y < r.bottom; y++) {
		for (int x = r.left; x < r.right; x++) {
			int bit = 0x8000 >> (4 * (y % 4) + (x % 4));
			_innerSurface.setPixel(x, y, (pattern & bit) ? kBlack : kWhite);
		}
	}

	markRectAsDirty(r);
}

int MacGuiImpl::MacDialogWindow::runDialog(Common::Array<int> &deferredActionIds) {
	// The first time the function is called, show the dialog and redraw
	// all widgets completely.

	deferredActionIds.clear();

	if (!_visible) {
		show();

		Common::Rect windowBounds(_innerSurface.w, _innerSurface.h);

		for (uint i = 0; i < _widgets.size(); i++) {
			_widgets[i]->setId(i);

			if (_widgets[i]->isVisible()) {
				_widgets[i]->setRedraw(true);
				_widgets[i]->draw();
			}
		}
	}

	// Run the dialog until something interesting happens to a widget. It's
	// up to the caller to repeat the calls to runDialog() until the dialog
	// has ended.

	bool buttonPressed = false;
	uint32 nextMouseRepeat = 0;

	while (!_gui->_vm->shouldQuit()) {
		Common::Event event;
		int widgetId = -1;

		while (_system->getEventManager()->pollEvent(event)) {
			// Adjust mouse coordinates to the dialog window

			if (Common::isMouseEvent(event)) {
				_realMousePos.x = event.mouse.x;
				_realMousePos.y = event.mouse.y;

				event.mouse.x -= (_bounds.left + _margin);
				event.mouse.y -= (_bounds.top + _margin);

				_oldMousePos = _mousePos;

				_mousePos.x = event.mouse.x;
				_mousePos.y = event.mouse.y;

				// Update engine mouse position
				_gui->_vm->_mouse.x = _realMousePos.x / 2;
				_gui->_vm->_mouse.y = _realMousePos.y / 2;
			}

			int w;

			switch (event.type) {
			case Common::EVENT_LBUTTONDOWN:
				// When a widget is clicked, it becomes the
				// focused widget. Focused widgets are often
				// indicated by some sort of highlight, e.g.
				// buttons become inverted.
				//
				// This highlight is usually only shown while
				// the mouse is within the widget bounds, but
				// as long as it remains focused it can regain
				// the highlight by moving the cursor back into
				// the widget bounds again.
				//
				// It's unclear to me if Macs should handle
				// double clicks on mouse down, mouse up or
				// both.

				buttonPressed = true;
				nextMouseRepeat = _system->getMillis() + 40;
				setFocusedWidget(event.mouse.x, event.mouse.y);
				if (_focusedWidget) {
					_focusedWidget->handleMouseDown(event);

					uint32 now = _system->getMillis();
					bool doubleClick =
						(now - _lastClickTime < 500 &&
						ABS(event.mouse.x - _lastClickPos.x) < 5 &&
						ABS(event.mouse.y - _lastClickPos.y) < 5);

					_lastClickTime = _system->getMillis();
					_lastClickPos.x = event.mouse.x;
					_lastClickPos.y = event.mouse.y;

					if (doubleClick && _focusedWidget->handleDoubleClick(event))
						return _focusedWidget->getId();
				}
				break;

			case Common::EVENT_LBUTTONUP:
				buttonPressed = false;

				// Only the focused widget receives the button
				// up event. If the widget handles the event,
				// control is passed back to the caller of
				// runDialog() so that it can react, e.g. to
				// the user clicking the "Okay" button.

				if (_focusedWidget) {
					MacWidget *widget = _focusedWidget;

					if (widget->findWidget(event.mouse.x, event.mouse.y)) {
						widgetId = widget->getId();
						if (widget->handleMouseUp(event)) {
							clearFocusedWidget();
							return widgetId;
						}
					}

					clearFocusedWidget();
				}

				updateCursor();
				break;

			case Common::EVENT_MOUSEMOVE:
				// The "beam" cursor can be hidden, but will
				// become visible again when the user moves
				// the mouse.

				if (_beamCursor)
					_beamCursorVisible = true;

				// Only the focused widget receives mouse move
				// events, and then only if the mouse is within
				// the widget's area of control. This are of
				// control is usually the widget bounds, but
				// widgets can redefine findWidget() to change
				// this, e.g. for slider widgets.

				if (_focusedWidget) {
					bool wasActive = _focusedWidget->findWidget(_oldMousePos.x, _oldMousePos.y);
					bool isActive = _focusedWidget->findWidget(_mousePos.x, _mousePos.y);

					if (wasActive != isActive)
						_focusedWidget->setRedraw();

					// The widget gets mouse events while
					// it's active, but also one last one
					// when it becomes inactive.

					if (isActive || wasActive)
						_focusedWidget->handleMouseMove(event);
				} else {
					updateCursor();
				}

				break;

			case Common::EVENT_WHEELUP:
				if (!_gui->_vm->enhancementEnabled(kEnhUIUX) || _focusedWidget)
					break;

				w = findWidget(event.mouse.x, event.mouse.y);
				if (w >= 0)
					_widgets[w]->handleWheelUp();

				break;

			case Common::EVENT_WHEELDOWN:
				if (!_gui->_vm->enhancementEnabled(kEnhUIUX) || _focusedWidget)
					break;

				w = findWidget(event.mouse.x, event.mouse.y);
				if (w >= 0)
					_widgets[w]->handleWheelDown();

				break;

			case Common::EVENT_KEYDOWN:
				// Ignore keyboard while mouse is pressed
				if (buttonPressed)
					break;

				// Handle default button
				if (event.kbd.keycode == Common::KEYCODE_RETURN) {
					MacWidget *widget = getDefaultWidget();
					if (widget && widget->isEnabled() && widget->isVisible()) {
						for (int i = 0; i < 2; i++) {
							widget->setRedraw();
							widget->draw(i == 0);
							update();

							for (int j = 0; j < 10; j++) {
								_system->delayMillis(10);
								_system->updateScreen();
							}
						}

						return widget->getId();
					}
				}

				// Otherwise, give widgets a chance to react
				// to key presses. All widgets get a chance,
				// whether or not they are focused. This may
				// be a bad idea, if there is ever more than
				// one edit text widget in the window.
				//
				// Typing hides the "beam" cursor.

				for (uint i = 0; i < _widgets.size(); i++) {
					if (_widgets[i]->isVisible() && _widgets[i]->isEnabled() && _widgets[i]->handleKeyDown(event)) {
						if (_beamCursor) {
							_beamCursorVisible = false;
							undrawBeamCursor();
						}

						if (_widgets[i]->shouldDeferAction())
							deferredActionIds.push_back(_widgets[i]->getId());

						break;
					}
				}

				if (!deferredActionIds.empty())
					return -2;

				break;

			default:
				break;
			}
		}

		// A focused widget implies that the mouse button is being
		// held down. It must be active and visible, so it can receive
		// mouse repeat events, e.g. for holding down scroll buttons
		// on a slider widget.

		if (_focusedWidget && _system->getMillis() > nextMouseRepeat) {
			nextMouseRepeat = _system->getMillis() + 40;
			_focusedWidget->handleMouseHeld();
		}

		_system->delayMillis(10);
		update();
		_system->updateScreen();
	}

	return -1;
}

void MacGuiImpl::MacDialogWindow::updateCursor() {
	int mouseOverWidget = findWidget(_mousePos.x, _mousePos.y);
	bool useBeamCursor = (mouseOverWidget >= 0 && _widgets[mouseOverWidget]->useBeamCursor());

	if (useBeamCursor && !_beamCursor) {
		CursorMan.showMouse(false);
		_beamCursor = new Graphics::Surface();
		_beamCursor->create(7, 16, Graphics::PixelFormat::createFormatCLUT8());
		_beamCursorVisible = true;
		_beamCursorPos = _realMousePos;
	} else if (!useBeamCursor && _beamCursor) {
		CursorMan.showMouse(true);

		undrawBeamCursor();
		_beamCursor->free();
		delete _beamCursor;
		_beamCursor = nullptr;
		_beamCursorVisible = false;
	}
}

void MacGuiImpl::MacDialogWindow::drawSprite(const Graphics::Surface *sprite, int x, int y) {
	_innerSurface.copyRectToSurface(*sprite, x, y, Common::Rect(sprite->w, sprite->h));
	markRectAsDirty(Common::Rect(x, y, x + sprite->w, y + sprite->h));
}

void MacGuiImpl::MacDialogWindow::drawSprite(const Graphics::Surface *sprite, int x, int y, Common::Rect(clipRect)) {
	Common::Rect subRect(sprite->w, sprite->h);

	if (x < clipRect.left) {
		subRect.left += (clipRect.left - x);
		x = clipRect.left;
	}

	if (y < clipRect.top) {
		subRect.top += (clipRect.top - y);
		y = clipRect.top;
	}

	if (x + sprite->w >= clipRect.right) {
		subRect.right -= (x + sprite->w - clipRect.right);
	}

	if (y + sprite->h >= clipRect.bottom) {
		subRect.bottom -= (y + sprite->h - clipRect.bottom);
	}

	if (subRect.width() > 0 && subRect.height() > 0) {
		_innerSurface.copyRectToSurface(*sprite, x, y, subRect);
		markRectAsDirty(Common::Rect(x, y, x + subRect.width(), y + subRect.height()));
	}
}

void MacGuiImpl::MacDialogWindow::plotPixel(int x, int y, int color, void *data) {
	MacGuiImpl::MacDialogWindow *window = (MacGuiImpl::MacDialogWindow *)data;
	Graphics::Surface *s = window->innerSurface();
	s->setPixel(x, y, color);
}

// I don't know if the original actually used two different plot functions, one
// to fill and one to darken (used to draw over the text screens). It's such a
// subtle effect that I suspect it was just doing some different magic, maybe
// with XOR, but I couldn't get that to work by eye only.

void MacGuiImpl::MacDialogWindow::plotPattern(int x, int y, int pattern, void *data) {
	const uint16 patterns[] = {
		0x0000, 0x2828, 0xA5A5, 0xD7D7,
		0xFFFF,	0xD7D7, 0xA5A5, 0x2828
	};

	MacGuiImpl::MacDialogWindow *window = (MacGuiImpl::MacDialogWindow *)data;
	Graphics::Surface *s = window->innerSurface();
	int bit = 0x8000 >> (4 * (y % 4) + (x % 4));
	if (patterns[pattern] & bit)
		s->setPixel(x, y, kBlack);
	else
		s->setPixel(x, y, kWhite);
}

void MacGuiImpl::MacDialogWindow::plotPatternDarkenOnly(int x, int y, int pattern, void *data) {
	const uint16 patterns[] = {
		0x0000, 0x2828, 0xA5A5, 0xD7D7, 0xFFFF
	};

	MacGuiImpl::MacDialogWindow *window = (MacGuiImpl::MacDialogWindow *)data;
	Graphics::Surface *s = window->innerSurface();
	int bit = 0x8000 >> (4 * (y % 4) + (x % 4));
	if (patterns[pattern] & bit)
		s->setPixel(x, y, kBlack);
}

void MacGuiImpl::MacDialogWindow::drawTexts(Common::Rect r, const TextLine *lines) {
	if (!lines)
		return;

	Graphics::Surface *s = innerSurface();

	for (int i = 0; lines[i].str; i++) {
		const Graphics::Font *f1 = nullptr;
		const Graphics::Font *f2 = nullptr;

		switch (lines[i].style) {
		case kStyleHeader:
			f1 = _gui->getFont(kAboutFontHeaderOutside);
			f2 = _gui->getFont(kAboutFontHeaderInside);
			break;
		case kStyleBold:
			f1 = _gui->getFont(kAboutFontBold);
			break;
		case kStyleExtraBold:
			f1 = _gui->getFont(kAboutFontExtraBold);
			break;
		case kStyleRegular:
			f1 = _gui->getFont(kAboutFontRegular);
			break;
		}

		const char *msg = lines[i].str;
		int x = r.left + lines[i].x;
		int y = r.top + lines[i].y;
		Graphics::TextAlign align = lines[i].align;
		int width = r.right - x;

		if (lines[i].style == kStyleHeader) {
			f1->drawString(s, msg, x - 1, y + 1, width, kBlack, align);
			f2->drawString(s, msg, x + 1, y + 1, width, kBlack, align);
			f1->drawString(s, msg, x - 2, y, width, kBlack, align);
			f2->drawString(s, msg, x, y, width, kWhite, align);
		} else {
			f1->drawString(s, msg, x, y, width, kBlack, align);

			if (lines[i].style == kStyleExtraBold)
				f1->drawString(s, msg, x + 1, y, width, kBlack, align);
		}
	}
}

void MacGuiImpl::MacDialogWindow::drawTextBox(Common::Rect r, const TextLine *lines, int arc) {
	Graphics::drawRoundRect(r, arc, kWhite, true, plotPixel, this);
	Graphics::drawRoundRect(r, arc, kBlack, false, plotPixel, this);
	markRectAsDirty(r);

	drawTexts(r, lines);
}

} // End of namespace Scumm
