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
 */

#ifndef M4_VIEWMGR_H
#define M4_VIEWMGR_H

#include "common/algorithm.h"
#include "common/array.h"
#include "common/list.h"
#include "common/events.h"
#include "common/rect.h"

#include "m4/font.h"
#include "m4/globals.h"
#include "m4/events.h"
#include "m4/graphics.h"

namespace M4 {

class View;
class ViewManager;

enum SceneTransition {
	kTransitionNone = 0,
	kTransitionFadeIn = 1,
	kTransitionFadeIn2 = 2,
	kTransitionBoxInBottomLeft = 3,
	kTransitionBoxInBottomRight = 4,
	kTransitionBoxInTopLeft = 5,
	kTransitionBoxInTopRight = 6,
	kTransitionPanLeftToRight = 7,
	kTransitionPanRightToLeft = 8,
	kTransitionCircleIn = 9
};

enum {SCREEN_DIALOG, SCREEN_BUFFER, SCREEN_TEXT, SCREEN_TRANSPARENT};
enum ScreenEventType {SCREVENT_NONE = 0, SCREVENT_KEY = 1, SCREVENT_MOUSE = 2, SCREVENT_ALL = 3};
enum ScreenLayers {
	LAYER_BACKGROUND = 0, LAYER_DRIFTER = 1, LAYER_INTERFACE = 1, LAYER_FLOATER = 2,
	LAYER_SURFACE = 3, LAYER_MENU = 9, LAYER_DIALOG = 10, LAYER_MOUSE = 15
};

enum ViewIds {
	VIEWID_MAINMENU = 1,
	VIEWID_SCENE = 2,
	VIEWID_TEXTVIEW = 3,
	VIEWID_ANIMVIEW = 4,
	VIEWID_MENU = 69,
	VIEWID_CONVERSATION = 48,
	VIEWID_INTERFACE = 49
};

struct ScreenFlags {
	bool visible:1;
	bool transparent:1;
	bool immovable:1;

	enum ScreenEventType blocks:2;
	enum ScreenEventType get:2;

	uint layer:4;
};

#define SCREEN_FLAGS_DEFAULT _screenFlags.layer = LAYER_DRIFTER; \
	_screenFlags.get = SCREVENT_ALL; _screenFlags.blocks = SCREVENT_NONE; \
	_screenFlags.visible = true;
#define SCREEN_FLAGS_ALERT _screenFlags.layer = LAYER_FLOATER \
	_screenFlags.get = SCREVENT_ALL; _screenFlags.blocks = SCREVENT_ALL; \
	_screenFlags.visible = true;

class RectList : public Common::Array<Common::Rect> {
public:
	RectList();
	~RectList();
	void addRect(int x1, int y1, int x2, int y2);
	void addRect(const Common::Rect &rect);

//	Common::Rect& operator [](int idx) { return _rects[idx]; }
	int find(const Common::Point &pt);
};

struct Hotkey {
public:
	typedef void (*Callback)(MadsM4Engine *vm, View *view, uint32 key);
	Hotkey(uint32 keyVal, Hotkey::Callback callbackFn) : key(keyVal), callback(callbackFn) {}
	uint32 key;
	Hotkey::Callback callback;
};

class HotkeyList {
public:
	HotkeyList(View *owner);
	~HotkeyList();
	void add(uint32 key, Hotkey::Callback callback);
	void remove(uint32 key);
	bool call(uint32 key);
private:
	Common::Array<Hotkey*> _hotkeys;
	View *_view;
};

class View : public M4Surface {
public:
	View(MadsM4Engine *vm, const Common::Rect &viewBounds, bool transparent = false);
	View(MadsM4Engine *vm, int x = 0, int y = 0, bool transparent = false);
	virtual ~View() {}

	void getCoordinates(Common::Rect &rect);
	void extract(int *status);
	virtual void show();
	virtual void hide();
	void moveToFront() {}
	void moveToBack();
	void moveAbsolute(int x, int y);
	void moveRelative(int x, int y);
	void resize(int newWidth, int newHeight);
	void restore(int x1, int y1, int x2, int y2);

	Common::Rect bounds() const { return _coords; }
	bool isInside(int x, int y) const { return _coords.contains(x, y); }
	ScreenFlags screenFlags() const { return _screenFlags; }
	int screenType() const { return _screenType; }
	bool isOffscreen() const { return !_screenFlags.visible; }
	bool isTransparent() const { return _screenFlags.transparent; }
	bool isVisible() const { return _screenFlags.visible; }
	uint layer() const { return _screenFlags.layer; }
	HotkeyList &hotkeys() { return _hotkeys; }

	virtual void onRefresh(RectList *rects, M4Surface *destSurface);
	virtual bool onEvent(M4EventType eventType, int32 param, int x, int y, bool &captureEvents) { return false; }
	virtual void updateState() {}

protected:
	MadsM4Engine *_vm;
	Common::Rect _coords;
	HotkeyList _hotkeys;
	int _screenType;
	ScreenFlags _screenFlags;
	bool _transparent;
};

class ViewManager {
private:
	MadsM4Engine *_vm;
	HotkeyList _systemHotkeys;
	Common::List<View *> _views;
	View *_captureScreen;
	bool _captureEvents;
public:
	typedef Common::List<View *>::iterator ListIterator;

	ViewManager(MadsM4Engine *vm);
	~ViewManager();

	void addView(View *view);
	void deleteView(View *view);

	void handleEvents(const Common::Event &event);
	void handleKeyboardEvents(uint32 keycode);
	void handleMouseEvents(M4EventType event);
	void restore(int x1, int y1, int x2, int y2);
	void restore(const Common::Rect &rect);

	void moveToFront(View *view);
	void moveToBack(View *view);

	Common::List<View *> views() const { return _views; }
	bool contains(View *key) const {
		return Common::find(_views.begin(), _views.end(), key) != _views.end();
	}
	bool contains(int screenType) { return getView(screenType) != NULL; }
	View *getView(int screenType);
	int containsViews() { return !_views.empty(); }

	void showTextView(const char *textViewName, bool returnToMainMenu = true);
	void showAnimView(const char *animViewName, bool returnToMainMenu = true);

	void updateState();
	void refreshAll();
	HotkeyList &systemHotkeys() { return _systemHotkeys; }
};

}

#endif
