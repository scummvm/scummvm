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

#ifndef TIZEN_FORM_H
#define TIZEN_FORM_H

#include <FApp.h>
#include <FUi.h>
#include <FSystem.h>
#include <FBase.h>
#include <FUiITouchEventListener.h>
#include <FUiITextEventListener.h>
#include <FUiCtrlIFormBackEventListener.h>
#include <FUiCtrlIFormMenuEventListener.h>

#include "config.h"
#include "common/scummsys.h"
#include "common/events.h"
#include "common/queue.h"
#include "common/mutex.h"
#include "engines/engine.h"

using namespace Tizen::Ui;
using namespace Tizen::Graphics;
using namespace Tizen::Base::Runtime;
using namespace Tizen::Ui::Controls;

//
// TizenAppForm
//
class TizenAppForm :
	public Controls::Form,
	public IRunnable,
	public IOrientationEventListener,
	public ITouchEventListener,
	public IFormBackEventListener,
	public IFormMenuEventListener {

public:
	TizenAppForm();
	virtual ~TizenAppForm();

	result Construct();
	bool pollEvent(Common::Event &event);
	bool isClosing() { return _state == kClosingState; }
	bool isStarting() { return _state == kInitState; }
	void pushKey(Common::KeyCode keycode);
	void exitSystem();
	void showKeypad();

private:
	Tizen::Base::Object *Run();
	result OnInitializing(void);
	result OnDraw(void);
	void OnOrientationChanged(const Control &source,
			OrientationStatus orientationStatus);
	void OnTouchDoublePressed(const Control &source,
			const Point &currentPosition,
			const TouchEventInfo &touchInfo);
	void OnTouchFocusIn(const Control &source,
			const Point &currentPosition,
			const TouchEventInfo &touchInfo);
	void OnTouchFocusOut(const Control &source,
			const Point &currentPosition,
			const TouchEventInfo &touchInfo);
	void OnTouchLongPressed(const Control &source,
			const Point &currentPosition,
			const TouchEventInfo &touchInfo);
	void OnTouchMoved(const Control &source,
			const Point &currentPosition,
			const TouchEventInfo &touchInfo);
	void OnTouchPressed(const Control &source,
			const Point &currentPosition,
			const TouchEventInfo &touchInfo);
	void OnTouchReleased(const Control &source,
			const Point &currentPosition,
			const TouchEventInfo &touchInfo);
	void OnFormBackRequested(Form &source);
	void OnFormMenuRequested(Form &source);

	void pushEvent(Common::EventType type, const Point &currentPosition);
	void terminate();
	void setButtonShortcut();
	void setMessage(const char *message);
	void setShortcut();
	void invokeShortcut();
	int  getTouchCount();
	bool gameActive() { return _state == kActiveState && g_engine != NULL && !g_engine->isPaused(); }

	// event handling
	bool _gestureMode;
	const char *_osdMessage;
	Tizen::Base::Runtime::Thread *_gameThread;
	Tizen::Base::Runtime::Mutex *_eventQueueLock;
	Common::Queue<Common::Event> _eventQueue;
	enum { kInitState, kActiveState, kClosingState, kDoneState, kErrorState } _state;
	enum { kLeftButton, kRightButtonOnce, kRightButton, kMoveOnly } _buttonState;
	enum { kControlMouse, kEscapeKey, kGameMenu, kShowKeypad } _shortcut;
};

#endif
