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
#include "common/macresman.h"

#include "graphics/blit.h"
#include "graphics/cursorman.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/paletteman.h"
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

MacGuiImpl::MacDialogWindow::MacDialogWindow(MacGuiImpl *gui, OSystem *system, Graphics::Surface *from, Common::Rect bounds, MacDialogWindowStyle windowStyle, MacDialogMenuStyle menuStyle) : _gui(gui), _system(system), _from(from), _bounds(bounds) {
	// Only apply menu style if the menu is open.
	Graphics::MacMenu *menu = _gui->_windowManager->getMenu();

	if (!menu->_active)
		menuStyle = kMenuStyleNone;

	_pauseToken = _gui->_vm->pauseEngine();

	// Remember if the screen was shaking. We don't use the SCUMM engine's
	// own _shakeTempSavedState to remember this, because there may be
	// nested dialogs. They each need their own.

	_shakeWasEnabled = _gui->_vm->_shakeEnabled;
	_gui->_vm->setShake(0);

	_black = _gui->getBlack();
	_white = _gui->getWhite();

	_backup = new Graphics::Surface();
	_backup->create(bounds.width(), bounds.height(), Graphics::PixelFormat::createFormatCLUT8());
	_backup->copyRectToSurface(*_from, 0, 0, bounds);

	_margin = (windowStyle == kWindowStyleNormal) ? 6 : 4;

	_surface = _from->getSubArea(bounds);
	bounds.grow(-_margin);
	_innerSurface = _from->getSubArea(bounds);

	_dirtyRects.clear();

	Graphics::Surface *s = surface();
	Common::Rect r = Common::Rect(s->w, s->h);

	r.grow(-1);
	s->fillRect(r, _white);

	if (windowStyle == kWindowStyleNormal) {
		if (_gui->_vm->_game.version < 6) {
			int growths[] = { 1, -3, -1 };

			for (int i = 0; i < ARRAYSIZE(growths); i++) {
				r.grow(growths[i]);
				s->frameRect(r, _black);
			}
		} else {
			uint32 light = _gui->_windowManager->findBestColor(0xCC, 0xCC, 0xFF);
			uint32 medium = _gui->_windowManager->findBestColor(0xBB, 0xBB, 0xBB);
			uint32 dark = _gui->_windowManager->findBestColor(0x66, 0x66, 0x99);

			s->frameRect(r, _black);
			s->frameRect(Common::Rect(r.left, r.top, r.right, r.bottom), dark);
			s->frameRect(Common::Rect(r.left, r.top, r.right - 1, r.bottom - 1), light);
			s->frameRect(Common::Rect(r.left + 1, r.top + 1, r.right - 1, r.bottom - 1), medium);
			s->frameRect(Common::Rect(r.left + 2, r.top + 2, r.right - 2, r.bottom - 2), dark);
			s->frameRect(Common::Rect(r.left + 3, r.top + 3, r.right - 2, r.bottom - 2), light);
			s->frameRect(Common::Rect(r.left + 3, r.top + 3, r.right - 3, r.bottom - 3), _black);
		}
	} else if (windowStyle == kWindowStyleRounded) {
		r.grow(1);

		for (int i = 0; i < 2; i++) {
			s->hLine(r.left + 2, r.top, r.right - 3, _black);
			s->hLine(r.left + 2, r.bottom - 1, r.right - 3, _black);
			s->vLine(r.left, r.top + 2, r.bottom - 3, _black);
			s->vLine(r.right - 1, r.top + 2, r.bottom - 3, _black);
			s->setPixel(r.left + 1, r.top + 1, _black);
			s->setPixel(r.left + 1, r.bottom - 2, _black);
			s->setPixel(r.right - 2, r.top + 1, _black);
			s->setPixel(r.right - 2, r.bottom - 2, _black);
			r.grow(-2);
		}
	}

	if (menuStyle != kMenuStyleNone) {
		// The menu bar isn't part of the Mac screen. We copy it to the
		// Mac screen so that the beam cursor is correctly drawn if it
		// ever moves that far up. There's no reason for it to, but it
		// can happen.

		Graphics::Surface *screen = _gui->surface();
		Graphics::Surface *realScreen = _system->lockScreen();

		// On a real Mac, the menu stays interactable. But the only
		// thing that is actually used for is the Edit menu, which we
		// don't implement. In ScummVM, the menu bar is just a drawing
		// at the point, so we can retouch it to look correct.
		//
		// However, the Mac Window Manager's ideas of what's black and
		// what's white may no longer be valid.

		uint32 macWhite = _gui->_macWhite;
		uint32 macBlack = _gui->_macBlack;

		if (macWhite != _white || macBlack != _black) {
			for (int y = 0; y < 20; y++) {
				for (int x = 0; x < realScreen->w; x++) {
					uint32 color = realScreen->getPixel(x, y);
					if (color == macWhite)
						realScreen->setPixel(x, y, _white);
					else if (color == macBlack)
						realScreen->setPixel(x, y, _black);
				}
			}
		}

		if (menuStyle == kMenuStyleDisabled) {
			for (int y = 0; y < 19; y++) {
				for (int x = 5; x < 635; x++) {
					if (((x + y) & 1) == 0)
						realScreen->setPixel(x, y, _white);
				}
			}
		} else if (menuStyle == kMenuStyleApple) {
			for (int y = 1; y < 19; y++) {
				for (int x = 10; x < 38; x++) {
					uint32 color = realScreen->getPixel(x, y);
					if (color == _black)
						realScreen->setPixel(x, y, _white);
					else
						realScreen->setPixel(x, y, _black);
				}
			}
		}

		screen->copyRectToSurface(*realScreen, 0, 0, Common::Rect(0, 0, 640, 19));

		_system->unlockScreen();
	}
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

	_gui->restoreScreen();
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

void MacGuiImpl::MacDialogWindow::addWidget(MacWidget *widget, MacWidgetType type) {
	widget->setId(_widgets.size());
	widget->setType(type);
	_widgets.push_back(widget);
}

MacGuiImpl::MacButton *MacGuiImpl::MacDialogWindow::addButton(Common::Rect bounds, Common::String text, bool enabled) {
	MacGuiImpl::MacButton *button = new MacButton(this, bounds, text, enabled);
	addWidget(button, kWidgetButton);
	return button;
}

MacGuiImpl::MacCheckbox *MacGuiImpl::MacDialogWindow::addCheckbox(Common::Rect bounds, Common::String text, bool enabled) {
	MacGuiImpl::MacCheckbox *checkbox = new MacCheckbox(this, bounds, text, enabled);
	addWidget(checkbox, kWidgetCheckbox);
	return checkbox;
}

MacGuiImpl::MacStaticText *MacGuiImpl::MacDialogWindow::addStaticText(Common::Rect bounds, Common::String text, bool enabled, Graphics::TextAlign alignment) {
	MacGuiImpl::MacStaticText *staticText = new MacStaticText(this, bounds, text, enabled, alignment);
	addWidget(staticText, kWidgetStaticText);
	return staticText;
}

MacGuiImpl::MacEditText *MacGuiImpl::MacDialogWindow::addEditText(Common::Rect bounds, Common::String text, bool enabled) {
	MacGuiImpl::MacEditText *editText = new MacEditText(this, bounds, text, enabled);
	addWidget(editText, kWidgetEditText);
	return editText;
}

MacGuiImpl::MacImage *MacGuiImpl::MacDialogWindow::addIcon(int x, int y, int id, bool enabled) {
	Graphics::Surface *icon = nullptr;
	Graphics::Surface *mask = nullptr;
	MacGuiImpl::MacImage *image = nullptr;

	if (_gui->loadIcon(id, &icon, &mask)) {
		image = new MacImage(this, Common::Rect(x, y, x + icon->w, y + icon->h), icon, mask, false);
		addWidget(image, kWidgetIcon);
	}

	return image;
}

MacGuiImpl::MacImage *MacGuiImpl::MacDialogWindow::addPicture(Common::Rect bounds, int id, bool enabled) {
	MacGuiImpl::MacImage *image = new MacImage(this, bounds, _gui->loadPict(id), nullptr, false);
	addWidget(image, kWidgetImage);
	return image;
}

MacGuiImpl::MacSlider *MacGuiImpl::MacDialogWindow::addSlider(int x, int y, int h, int minValue, int maxValue, int pageSize, bool enabled) {
	MacGuiImpl::MacSlider *slider = new MacSlider(this, Common::Rect(x, y, x + 16, y + h), minValue, maxValue, pageSize, enabled);
	addWidget(slider, kWidgetSlider);
	return slider;
}

MacGuiImpl::MacImageSlider *MacGuiImpl::MacDialogWindow::addImageSlider(int backgroundId, int handleId, bool enabled, int minX, int maxX, int minValue, int maxValue, int leftMargin, int rightMargin) {
	MacImage *background = (MacImage *)_widgets[backgroundId];
	MacImage *handle = (MacImage *)_widgets[handleId];

	background->setVisible(false);
	handle->setVisible(false);

	MacGuiImpl::MacImageSlider *slider = new MacImageSlider(this, background, handle, enabled, minX, maxX, minValue, maxValue, leftMargin, rightMargin);
	addWidget(slider, kWidgetImageSlider);
	return slider;
}

MacGuiImpl::MacImageSlider *MacGuiImpl::MacDialogWindow::addImageSlider(Common::Rect bounds, MacImage *handle, bool enabled, int minX, int maxX, int minValue, int maxValue) {
	handle->setVisible(false);

	MacGuiImpl::MacImageSlider *slider = new MacImageSlider(this, bounds, handle, enabled, minX, maxX, minValue, maxValue);
	addWidget(slider, kWidgetImageSlider);
	return slider;
}

MacGuiImpl::MacListBox *MacGuiImpl::MacDialogWindow::addListBox(Common::Rect bounds, Common::StringArray texts, bool enabled, bool contentUntouchable) {
	MacGuiImpl::MacListBox *listBox = new MacListBox(this, bounds, texts, enabled, contentUntouchable);
	addWidget(listBox, kWidgetListBox);
	return listBox;
}

MacGuiImpl::MacPopUpMenu *MacGuiImpl::MacDialogWindow::addPopUpMenu(Common::Rect bounds, Common::String text, int textWidth, Common::StringArray texts, bool enabled) {
	MacGuiImpl::MacPopUpMenu *popUpMenu = new MacPopUpMenu(this, bounds, text, textWidth, texts, enabled);
	addWidget(popUpMenu, kWidgetPopUpMenu);
	return popUpMenu;
}

void MacGuiImpl::MacDialogWindow::addControl(Common::Rect bounds, uint16 controlId) {
	Common::MacResManager resource;

	resource.open(_gui->_resourceFile);

	Common::SeekableReadStream *cntl = resource.getResource(MKTAG('C', 'N', 'T', 'L'), controlId);
	if (cntl) {
		byte cntlHeader[22];

		cntl->read(cntlHeader, sizeof(cntlHeader));
		Common::String cntlText = cntl->readPascalString();

		uint16 procId = READ_BE_UINT16(cntlHeader + 16);

		if ((procId & 0xFFF0) == 0x03F0) {
			uint16 textWidth = READ_BE_UINT16(cntlHeader + 12);
			uint16 menuId = READ_BE_UINT16(cntlHeader + 14);

			Common::SeekableReadStream *menu = resource.getResource(MKTAG('M', 'E', 'N', 'U'), menuId);
			if (menu) {
				menu->skip(14);
				menu->skip(menu->readByte());

				Common::StringArray items;

				while (true) {
					Common::String str;

					str = menu->readPascalString();

					if (str.empty())
						break;

					items.push_back(str);
					menu->skip(4);
				}

				delete menu;

				addPopUpMenu(bounds, cntlText, textWidth, items, true);
			} else
				warning("MacGuiImpl::addPopUpMenu: Could not load MENU %d", menuId);
		} else
			warning("MacGuiImpl::addPopUpMenu: Unknown control ProcID: %d", procId);
		delete cntl;
	} else
		warning("MacGuiImpl::addPopUpMenu: Could not load CNTL %d", controlId);

	resource.close();
}

void MacGuiImpl::MacDialogWindow::markRectAsDirty(Common::Rect r) {
	_dirtyRects.push_back(r);
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

		color = _gui->_windowManager->inverter(color);
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
	if (_dirtyPalette) {
		_gui->_vm->updatePalette();
		_dirtyPalette = false;
	}

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

	uint32 color[2];

	// The dotted line is used by the default save/load dialogs as a
	// separator between buttons. Surprisingly, this is only drawn using
	// shades of gray in 16 color mode. Not, as you might think, in 256
	// color mode.
	//
	// At least not in the version of MacOS that was used for reference.

	if (_gui->_vm->_renderMode == Common::kRenderMacintoshBW || !(_gui->_vm->_game.features & GF_16COLOR)) {
		color[0] = _black;
		color[1] = _white;
	} else {
		color[0] = _gui->_windowManager->findBestColor(0x75, 0x75, 0x75);
		color[1] = _gui->_windowManager->findBestColor(0xBE, 0xBE, 0xBE);
	}

	for (int x = x0; x <= x1; x++)
		s->setPixel(x, y, color[x & 1]);
}

void MacGuiImpl::MacDialogWindow::fillPattern(Common::Rect r, uint16 pattern, bool fillBlack, bool fillWhite) {
	for (int y = r.top; y < r.bottom; y++) {
		for (int x = r.left; x < r.right; x++) {
			int bit = 0x8000 >> (4 * (y % 4) + (x % 4));

			if (pattern & bit) {
				if (fillBlack)
					_innerSurface.setPixel(x, y, _black);
			} else {
				if (fillWhite)
					_innerSurface.setPixel(x, y, _white);
			}
		}
	}

	markRectAsDirty(r);
}

void MacGuiImpl::MacDialogWindow::queueEvent(MacWidget *widget, MacDialogEventType type) {
	MacDialogEvent event;

	event.widget = widget;
	event.type = type;

	_eventQueue.push(event);
}

bool MacGuiImpl::MacDialogWindow::runDialog(MacGuiImpl::MacDialogEvent &dialogEvent) {
	for (uint i = 0; i < _widgets.size(); i++)
		_widgets[i]->rememberValue();

	// The first time the function is called, show the dialog and redraw
	// all widgets completely.

	if (!_visible) {
		show();

		for (uint i = 0; i < _widgets.size(); i++) {
			if (_widgets[i]->isVisible()) {
				_widgets[i]->setRedraw(true);
				_widgets[i]->draw();
			}
		}
	}

	// Handle all incoming events and turn them into dialog events.

	bool buttonPressed = false;
	uint32 nextMouseRepeat = 0;

	Common::Event event;

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
			// When a widget is clicked, it becomes the focused
			// widget. Focused widgets are often indicated by some
			// sort of highlight, e.g. buttons become inverted.
			//
			// This highlight is usually only shown while the mouse
			// is within the widget bounds, but as long as it
			// remains focused it can regain the highlight by
			// moving the cursor back into the widget bounds again.
			//
			// It's unclear to me if Macs should handle double
			// clicks on mouse down, mouse up or both.

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
					queueEvent(_focusedWidget, kDialogClick);
			}
			break;

		case Common::EVENT_LBUTTONUP:
			buttonPressed = false;

			// Only the focused widget receives the button up
			// event. If the widget handles the event, it produces
			// a dialog click event.

			if (_focusedWidget) {
				MacWidget *widget = _focusedWidget;

				if (widget->findWidget(event.mouse.x, event.mouse.y)) {
					if (widget->handleMouseUp(event)) {
						clearFocusedWidget();
						queueEvent(widget, kDialogClick);
					}
				}

				clearFocusedWidget();
			}

			updateCursor();
			break;

		case Common::EVENT_MOUSEMOVE:
			// The "beam" cursor can be hidden, but will become
			// visible again when the user moves the mouse.

			if (_beamCursor)
				_beamCursorVisible = true;

			// Only the focused widget receives mouse move events,
			// and then only if the mouse is within the widget's
			// area of control. This are of control is usually the
			// widget bounds, but widgets can redefine findWidget()
			// to change this, e.g. for slider widgets.

			if (_focusedWidget) {
				bool wasActive = _focusedWidget->findWidget(_oldMousePos.x, _oldMousePos.y);
				bool isActive = _focusedWidget->findWidget(_mousePos.x, _mousePos.y);

				if (wasActive != isActive)
					_focusedWidget->setRedraw();

				// The widget gets mouse events while it's
				// active, but also one last one when it
				// becomes inactive.

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

					queueEvent(widget, kDialogClick);
				}
			}

			// Otherwise, give widgets a chance to react to key
			// presses. All widgets get a chance, whether or not
			// they are focused. This may be a bad idea if there
			// is ever more than one edit text widget in the
			// window.
			//
			// Typing hides the "beam" cursor.

			for (uint i = 0; i < _widgets.size(); i++) {
				if (_widgets[i]->isVisible() && _widgets[i]->isEnabled() && _widgets[i]->handleKeyDown(event)) {
					if (_beamCursor) {
						_beamCursorVisible = false;
						undrawBeamCursor();
					}

					if (_widgets[i]->reactsToKeyDown())
						queueEvent(_widgets[i], kDialogKeyDown);

					break;
				}
			}

			break;

		default:
			break;
		}
	}

	// A focused widget implies that the mouse button is being held down.
	// It must be active and visible, so it can receive mouse repeat
	// events, e.g. for holding down scroll buttons on a slider widget.

	if (_focusedWidget && _system->getMillis() > nextMouseRepeat) {
		nextMouseRepeat = _system->getMillis() + 40;
		_focusedWidget->handleMouseHeld();
	}

	for (uint i = 0; i < _widgets.size(); i++) {
		if (_widgets[i]->valueHasChanged())
			queueEvent(_widgets[i], kDialogValueChange);
	}

	if (!_eventQueue.empty()) {
		dialogEvent = _eventQueue.pop();
		return true;
	}

	return false;
}

void MacGuiImpl::MacDialogWindow::delayAndUpdate() {
	_system->delayMillis(10);
	update();
	_system->updateScreen();
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

MacGuiImpl::MacWidget *MacGuiImpl::MacDialogWindow::getWidget(uint nr) const {
	if (nr < _widgets.size())
		return _widgets[nr];
	return nullptr;
}

MacGuiImpl::MacWidget *MacGuiImpl::MacDialogWindow::getWidget(MacWidgetType type, uint nr) const {
	for (uint i = 0; i < _widgets.size(); i++) {
		if (_widgets[i]->getType() == type) {
			if (nr == 0)
				return _widgets[i];
			nr--;
		}
	}

	return nullptr;
}

void MacGuiImpl::MacDialogWindow::drawSprite(const MacImage *image, int x, int y) {
	Graphics::Surface *dstSurface = innerSurface();
	const Graphics::Surface *srcSurface = image->getImage();
	const Graphics::Surface *maskSurface = image->getMask();

	if (maskSurface) {
		byte *dst = (byte*)dstSurface->getBasePtr(x, y);
		const byte *src = (const byte *)srcSurface->getBasePtr(0, 0);
		const byte *mask = (const byte *)maskSurface->getBasePtr(0, 0);

		assert(srcSurface->format == dstSurface->format);

		Graphics::maskBlit(dst, src, mask,
			dstSurface->pitch, srcSurface->pitch, maskSurface->pitch,
			srcSurface->w, srcSurface->h,
			dstSurface->format.bytesPerPixel);
		markRectAsDirty(Common::Rect(x, y, x + srcSurface->w, y + srcSurface->h));
	} else
		drawSprite(srcSurface, x, y);
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
		s->setPixel(x, y, window->_gui->getBlack());
	else
		s->setPixel(x, y, window->_gui->getWhite());
}

void MacGuiImpl::MacDialogWindow::plotPatternDarkenOnly(int x, int y, int pattern, void *data) {
	const uint16 patterns[] = {
		0x0000, 0x2828, 0xA5A5, 0xD7D7, 0xFFFF
	};

	MacGuiImpl::MacDialogWindow *window = (MacGuiImpl::MacDialogWindow *)data;
	Graphics::Surface *s = window->innerSurface();
	int bit = 0x8000 >> (4 * (y % 4) + (x % 4));
	if (patterns[pattern] & bit)
		s->setPixel(x, y, window->_gui->getBlack());
}

void MacGuiImpl::MacDialogWindow::drawTexts(Common::Rect r, const TextLine *lines, bool inverse) {
	if (!lines)
		return;

	uint32 fg, bg;

	if (inverse) {
		fg = _white;
		bg = _black;
	} else {
		fg = _black;
		bg = _white;
	}

	Graphics::Surface *s = innerSurface();

	for (int i = 0; lines[i].str; i++) {
		const Graphics::Font *f1 = nullptr;
		const Graphics::Font *f2 = nullptr;

		switch (lines[i].style) {
		case kStyleHeader1:
			f1 = _gui->getFont(kAboutFontHeaderOutside);
			f2 = _gui->getFont(kAboutFontHeaderInside);
			break;

		case kStyleHeader2:
			f1 = _gui->getFont(kAboutFontHeader);
			break;

		case kStyleHeaderSimple1:
			f1 = _gui->getFont(kAboutFontHeaderSimple1);
			break;

		case kStyleHeaderSimple2:
			f1 = _gui->getFont(kAboutFontHeaderSimple2);
			break;

		case kStyleBold:
			f1 = _gui->getFont(kAboutFontBold);
			break;

		case kStyleBold2:
			f1 = _gui->getFont(kAboutFontBold2);
			break;

		case kStyleExtraBold:
			f1 = _gui->getFont(kAboutFontExtraBold);
			break;

		case kStyleRegular:
			f1 = _gui->getFont(kAboutFontRegular);
			break;

		default:
			return;
		}

		const char *msg = lines[i].str;
		int x = r.left + lines[i].x;
		int y = r.top + lines[i].y;
		Graphics::TextAlign align = lines[i].align;
		int width = r.right - x;

		switch (lines[i].style) {
		case kStyleHeader1:
			f1->drawString(s, msg, x - 1, y + 1, width, fg, align);
			f2->drawString(s, msg, x + 1, y + 1, width, fg, align);
			f1->drawString(s, msg, x - 2, y, width, fg, align);
			f2->drawString(s, msg, x, y, width, bg, align);
			break;

		case kStyleHeader2:
			f1->drawString(s, msg, x, y, width, fg, align);
			f1->drawString(s, msg, x + 1, y, width, fg, align);
			break;

		default:
			f1->drawString(s, msg, x, y, width, fg, align);
			break;
		}
	}
}

void MacGuiImpl::MacDialogWindow::drawTextBox(Common::Rect r, const TextLine *lines, bool inverse, int arc) {
	uint32 fg, bg;

	if (inverse) {
		fg = _white;
		bg = _black;
	} else {
		fg = _black;
		bg = _white;
	}

	Graphics::drawRoundRect(r, arc, bg, true, plotPixel, this);
	Graphics::drawRoundRect(r, arc, fg, false, plotPixel, this);
	markRectAsDirty(r);

	drawTexts(r, lines, inverse);
}

} // End of namespace Scumm
