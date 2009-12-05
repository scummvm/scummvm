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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// TODO: Views have a _coords rect, so I'm not sure if x/y is needed in the onRefresh

#include "m4/m4.h"
#include "m4/viewmgr.h"
#include "m4/mads_anim.h"

namespace M4 {

void returnToMainMenuFn(M4Engine *vm) {
	vm->_palette->resetColorCounts();
	vm->_palette->setMadsSystemPalette();

	vm->loadMenu(MAIN_MENU);
}

RectList::RectList() {
}

RectList::~RectList() {
}

void RectList::addRect(int x1, int y1, int x2, int y2) {
	addRect(Common::Rect(x1, y1, x2, y2));
}

void RectList::addRect(const Common::Rect &rect) {
	/* TODO:
		Implement the following:
			- Don't add the Rect if it's contained in any Rect in the list
			- Split up the Rect if it intersects any Rect in the list
			  and add the resulting partial Rects instead
	*/
	push_back(rect);
}

int RectList::find(const Common::Point &pt) {
	for (int idx = 0; idx < size(); ++idx) {
		if (this->operator [](idx).contains(pt.x, pt.y))
			return idx;
	}
	return -1;	
}

//--------------------------------------------------------------------------

HotkeyList::HotkeyList(View *owner) {
	_view = owner;
}

HotkeyList::~HotkeyList() {
	for (uint32 i = 0; i < _hotkeys.size(); i++)
		delete _hotkeys[i];
}

void HotkeyList::add(uint32 key, Hotkey::Callback callback) {
	_hotkeys.push_back(new Hotkey(key, callback));
}

void HotkeyList::remove(uint32 key) {
	for (uint32 i = 0; i < _hotkeys.size(); i++) {
		if (_hotkeys[i]->key == key) {
			delete _hotkeys[i];
			_hotkeys.remove_at(i);
			break;
		}
	}
}

bool HotkeyList::call(uint32 key) {
	for (uint32 i = 0; i < _hotkeys.size(); i++) {
		if (_hotkeys[i]->key == key) {
			if (_hotkeys[i]->callback)
				(_hotkeys[i]->callback)(_vm, _view, key);
			return true;
		}
	}
	return false;
}

//--------------------------------------------------------------------------

// View constructor

View::View(M4Engine *vm, const Common::Rect &viewBounds, bool transparent):
	_hotkeys(HotkeyList(this)), M4Surface(viewBounds.width(), viewBounds.height()), _vm(vm) {
	SCREEN_FLAGS_DEFAULT;
	_coords = viewBounds;
	_transparent = transparent;
}

View::View(M4Engine *vm, int x, int y, bool transparent): _hotkeys(HotkeyList(this)), M4Surface(), _vm(vm) {
	SCREEN_FLAGS_DEFAULT;
	_coords.left = x;
	_coords.top = y;
	_coords.right = _vm->_screen->width();
	_coords.bottom = _vm->_screen->height();
	_transparent = transparent;
}

void View::getCoordinates(Common::Rect &rect) {
	rect = _coords;
}

void View::extract(int *status) {
}

void View::show() {
	_screenFlags.visible = true;
	_vm->_viewManager->moveToFront(this);
	_vm->_viewManager->restore(_coords);
}

void View::hide() {
	_screenFlags.visible = false;
	_vm->_viewManager->restore(_coords);
}

void View::moveToBack() {
	_vm->_viewManager->moveToBack(this);
}

void View::moveAbsolute(int x, int y) {
	// TODO: Handle clipping and offscreen
	Common::Rect oldCoords = _coords;
	_coords.moveTo(x, y);
	_vm->_viewManager->restore(oldCoords);
	_vm->_viewManager->restore(_coords);
}

void View::moveRelative(int x, int y) {
	// TODO: Handle clipping and offscreen
	Common::Rect oldCoords = _coords;
	_coords.translate(x, y);
	_vm->_viewManager->restore(oldCoords);
	_vm->_viewManager->restore(_coords);
}

void View::resize(int newWidth, int newHeight) {
	Common::Rect oldCoords = _coords;
	if (newWidth >= 0)
		_coords.setWidth(newWidth);
	if (newHeight >= 0)
		_coords.setHeight(newHeight);
	_vm->_viewManager->restore(oldCoords);
	_vm->_viewManager->restore(_coords);
}

void View::restore(int x1, int y1, int x2, int y2) {
	_vm->_viewManager->restore(_coords.left + x1, _coords.top + y1, _coords.left + x2, _coords.top + y2);
}

void View::onRefresh(RectList *rects, M4Surface *destSurface) {
	assert(destSurface);

	if (rects == NULL)
		// No rect list specified, so copy entire surface
		copyTo(destSurface, _coords.left, _coords.top, _transparent ? 0 : -1);
	else {
		// Loop through the set of specified rectangles
		RectList::iterator i;
		for (i = rects->begin(); i != rects->end(); ++i) {
			Common::Rect &destRect = *i;
			Common::Rect srcBounds(destRect.left - _coords.left, destRect.top - _coords.top,
				destRect.right - _coords.left, destRect.bottom - _coords.top);
			copyTo(destSurface, srcBounds, destRect.left, destRect.top, _transparent ? 0 : -1);
		}
	}
}

//--------------------------------------------------------------------------

ViewManager::ViewManager(M4Engine *vm): _systemHotkeys(HotkeyList(NULL)), _vm(vm) {
	_captureScreen = NULL;
	_captureEvents = false;
}

ViewManager::~ViewManager() {
	// Delete any remaining active views
	ListIterator i;
	for (i = _views.begin(); i != _views.end(); ++i)
		delete (*i);
}

void ViewManager::addView(View *view) {
	_views.push_back(view);
	moveToFront(view);
}

// Warning: After calling this method, the passed view object will no longer be valid

void ViewManager::deleteView(View *view) {
	_views.remove(view);
	delete view;
}

void ViewManager::handleEvents(const Common::Event &event) {
}

void ViewManager::handleKeyboardEvents(uint32 keycode) {
	Common::Point mousePos = _vm->_mouse->currentPos();
	View *view;
	bool blockedFlag;
	bool foundFlag;
	bool handledFlag;

	// Scan view list for one which accepts or blocks keyboard events. If one is found,
	// then the event is passed to it

	view = NULL;
	handledFlag = false;
	foundFlag = false;
	blockedFlag = false;

	// Loop from the front to back view
	ListIterator i;
	for (i = _views.reverse_begin(); (i != _views.end()) && !foundFlag; --i) {
		view = *i;
		if (!view->isVisible()) continue;

		if (view->screenFlags().blocks & SCREVENT_KEY)
			blockedFlag = true;
		if (view->screenFlags().get & SCREVENT_KEY) {
			foundFlag = true;
			handledFlag = (view->onEvent)(KEVENT_KEY, keycode, mousePos.x, mousePos.y, _captureEvents);
		}
	}

	// Scan view list for one with a hotkey list, aborting if a view is found that either
	// blocks keyboard events, or has a hotkey list that includes the keycode

	blockedFlag = false;
	for (i = _views.reverse_begin(); (i != _views.end()) && !foundFlag && !blockedFlag; --i) {
		view = *i;
		if (!view->isVisible()) continue;

		if (view->screenFlags().blocks & SCREVENT_KEY)
			blockedFlag = true;
		if (view->screenFlags().get & SCREVENT_KEY) {
			if (view->hotkeys().call(keycode)) {
				handledFlag = true;
				_captureEvents = false;
				//_vm->_dialogs->keyMouseCollision();	// TODO
			}
		}
	}

	// Final check: if no view handled or blocked the key, check against the system hotkey list

	if (!handledFlag && !blockedFlag) {
		handledFlag = _systemHotkeys.call(keycode);
		if (handledFlag) {
			_captureEvents = false;
			//_vm->_dialogs->keyMouseCollision();	// TODO
		}
	}
}

void ViewManager::handleMouseEvents(M4EventType event) {
	Common::Point mousePos = _vm->_mouse->currentPos();
	ListIterator i;
	View *view;
	bool blockedFlag;
	bool foundFlag;

	// If a window sets the _captureEvents flag to true, it will receive all events until
	// it sets it to false, even if it's not the top window
	if (_captureEvents) {
		if (_captureScreen->screenFlags().get & SCREVENT_MOUSE)
			(_captureScreen->onEvent)(event, 0, mousePos.x, mousePos.y, _captureEvents);

	} else {
		blockedFlag = false;
		foundFlag = false;
		view = NULL;

		// Loop from the front to back view
		for (i = _views.reverse_begin(); (i != _views.end()) && !foundFlag && !blockedFlag; --i) {
			view = *i;
			if (!view->isVisible()) continue;

			if (view->screenFlags().blocks & SCREVENT_MOUSE)
				blockedFlag = true;
			if ((view->screenFlags().get & SCREVENT_MOUSE) && view->isInside(mousePos.x, mousePos.y))
				foundFlag = true;
		}

		if (foundFlag)
			view->onEvent(event, 0, mousePos.x, mousePos.y, _captureEvents);
		else
			_captureEvents = false;
		if (_captureEvents)
			_captureScreen = view;
	}
}

void ViewManager::restore(int x1, int y1, int x2, int y2) {
	RectList *rl = new RectList();
	Common::Rect redrawBounds(x1, y1, x2, y2);
	rl->addRect(x1, y1, x2, y2);

	for (ListIterator i = _views.begin(); i != _views.end(); ++i) {
		View *v = *i;

		if (v->isVisible() && v->bounds().intersects(redrawBounds))
			v->onRefresh(rl, _vm->_screen);
	}

	_vm->_screen->update();

}

void ViewManager::restore(const Common::Rect &rect) {
	restore(rect.left, rect.top, rect.right, rect.bottom);
}

void ViewManager::moveToFront(View *view) {
	if (_views.size() < 2)
		return;

	_views.remove(view);

	ListIterator i = _views.begin();
	while ((i != _views.end()) && ((*i)->layer() <= view->layer()))
		++i;

	_views.insert(i, view);
}

void ViewManager::moveToBack(View *view) {
	if (_views.size() < 2)
		return;

	_views.remove(view);

	ListIterator i = _views.begin();
	while ((i != _views.end()) && ((*i)->layer() < view->layer()))
		++i;

	_views.insert(i, view);
}

View *ViewManager::getView(int screenType) {
	ListIterator i = _views.begin();
	while (i != _views.end()) {
		if ((*i)->screenType() == screenType)
			return *i;
		++i;
	}

	return NULL;
}

void ViewManager::updateState() {
	Common::List<View *> viewList = _views;

	for (ListIterator i = viewList.begin(); i != viewList.end(); ++i) {
		if (_vm->_events->quitFlag)
			return;

		View *v = *i;
		v->updateState();
	}
}

void ViewManager::refreshAll() {
	_vm->_screen->clear();

	for (ListIterator i = _views.begin(); i != _views.end(); ++i) {
		View *v = *i;

		if (v->isVisible())
			v->onRefresh(NULL, _vm->_screen);
	}

	_vm->_screen->update();
}

void ViewManager::showTextView(const char *textViewName, bool returnToMainMenu) {
	// Deactivate the scene if it's currently active
	View *view = _vm->_viewManager->getView(VIEWID_SCENE);
	if (view != NULL)
		_vm->_viewManager->deleteView(view);

	// Deactivate the main menu if it's currently active
	view = _vm->_viewManager->getView(VIEWID_MAINMENU);
	if (view != NULL)
		_vm->_viewManager->deleteView(view);

	// Activate the textview view
	_vm->_font->setFont(FONT_CONVERSATION_MADS);
	TextviewView *textView = new TextviewView(_vm);
	_vm->_viewManager->addView(textView);
	if (returnToMainMenu)
		textView->setScript(textViewName, returnToMainMenuFn);
	else
		textView->setScript(textViewName, NULL);
}

void ViewManager::showAnimView(const char *animViewName, bool returnToMainMenu) {
	// Deactivate the scene if it's currently active
	View *view = _vm->_viewManager->getView(VIEWID_SCENE);
	if (view != NULL)
		_vm->_viewManager->deleteView(view);

	// Deactivate the main menu if it's currently active
	view = _vm->_viewManager->getView(VIEWID_MAINMENU);
	if (view != NULL)
		_vm->_viewManager->deleteView(view);

	// Activate the animview view
	AnimviewView *animView = new AnimviewView(_vm);
	_vm->_viewManager->addView(animView);
	if (returnToMainMenu)
		animView->setScript(animViewName, returnToMainMenuFn);
	else
		animView->setScript(animViewName, NULL);
}

} // End of namespace M4
