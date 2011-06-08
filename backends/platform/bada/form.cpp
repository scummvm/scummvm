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

#include <FAppApplication.h>

#include "form.h"
#include "system.h"

using namespace Osp::App;
using namespace Osp::Base;
using namespace Osp::Base::Runtime;
using namespace Osp::Graphics;
using namespace Osp::Ui;
using namespace Osp::Ui::Controls;

//
// BadaAppForm
//
BadaAppForm::BadaAppForm() : pThread(0) {
  eventQueueLock = new Mutex();
  eventQueueLock->Create();
}

result BadaAppForm::Construct() {
  result r = Form::Construct(Controls::FORM_STYLE_NORMAL);
  if (IsFailed(r)) {
    return r;
  }

  BadaSystem* badaSystem = null;
  pThread = null;

  badaSystem = new BadaSystem(this);
  r = badaSystem != null ? E_SUCCESS : E_OUT_OF_MEMORY;
  
  if (!IsFailed(r)) {
    r = badaSystem->Construct();
  }

  if (!IsFailed(r)) {
    pThread = new Thread();
    r = pThread != null ? E_SUCCESS : E_OUT_OF_MEMORY;
  }

  if (!IsFailed(r)) {
    r = pThread->Construct(*this);
  }

  if (IsFailed(r)) {
    if (badaSystem != null) {
      delete badaSystem;
    }
    if (pThread != null) {
      delete pThread;
      pThread = null;
    }
  }
  else {
    g_system = badaSystem;
  }

  return r;
}

BadaAppForm::~BadaAppForm() {
  logEntered();

  if (pThread) {
    pThread->Stop();
    delete pThread;
    pThread = null;
  }

  if (eventQueueLock) {
    delete eventQueueLock;
    eventQueueLock = null;
  }

  if (g_system) {
    delete (BadaSystem*) g_system;
    g_system = null;
  }
}

result BadaAppForm::OnInitializing(void) {
  logEntered();

  SetOrientation(ORIENTATION_LANDSCAPE);
  AddOrientationEventListener(*this);
  AddTouchEventListener(*this);

  return E_SUCCESS;
}

result BadaAppForm::OnDraw(void) {
  logEntered();
  if (g_system) {
    g_system->updateScreen();
  }
  return E_SUCCESS;
}

bool BadaAppForm::pollEvent(Common::Event& event) {
  bool result = false;

  eventQueueLock->Acquire();

  if (!eventQueue.empty()) {
    event = eventQueue.pop();
    result = true;
  }

  eventQueueLock->Release();
  return result;
}

void BadaAppForm::pushEvent(Common::EventType type, 
                            const Point& currentPosition) {
  Common::Event e;
  e.type = type;
  e.mouse.x = currentPosition.x;
  e.mouse.y = currentPosition.y;
  g_system->warpMouse(currentPosition.x, currentPosition.y);

  eventQueueLock->Acquire();
  eventQueue.push(e);
  eventQueueLock->Release();
}

void BadaAppForm::OnOrientationChanged(const Control& source, 
                                       OrientationStatus orientationStatus) {
  logEntered();
  pThread->Start();
}

Object* BadaAppForm::Run(void) {
  scummvm_main(0, 0);

  AppLog("scummvm_main completed");
  Application::GetInstance()->SendUserEvent(USER_MESSAGE_EXIT, null);
  return null;
}

// TODO: use touchEventInfo, eg Flicked() to map commonly used keycodes
// such as the escape key, long press could be EVENT_MAINMENU etc.
// Can't experiment with this until the code is running on the device

void BadaAppForm::OnTouchDoublePressed(const Control& source, 
                                       const Point& currentPosition, 
                                       const TouchEventInfo& touchInfo) {
  // display the virtual keypad
  Common::Event e;
  e.type = Common::EVENT_KEYDOWN;
  e.kbd.keycode = Common::KEYCODE_F7;

  eventQueueLock->Acquire();
  eventQueue.push(e);
  eventQueueLock->Release();
}

void BadaAppForm::OnTouchFocusIn(const Control& source, 
                                 const Point& currentPosition, 
                                 const TouchEventInfo& touchInfo) {
}

void BadaAppForm::OnTouchFocusOut(const Control& source, 
                                  const Point& currentPosition, 
                                  const TouchEventInfo& touchInfo) {
}

void BadaAppForm::OnTouchLongPressed(const Control& source, 
                                     const Point& currentPosition, 
                                     const TouchEventInfo& touchInfo) {
}

void BadaAppForm::OnTouchMoved(const Control& source, 
                               const Point& currentPosition, 
                               const TouchEventInfo& touchInfo) {
  pushEvent(Common::EVENT_MOUSEMOVE, currentPosition);
}

void BadaAppForm::OnTouchPressed(const Control& source, 
                                 const Point& currentPosition, 
                                 const TouchEventInfo& touchInfo) {
  pushEvent(Common::EVENT_LBUTTONDOWN, currentPosition);
}

void BadaAppForm::OnTouchReleased(const Control& source, 
                                  const Point& currentPosition, 
                                  const TouchEventInfo& touchInfo) {
  pushEvent(Common::EVENT_LBUTTONUP, currentPosition);
}

//
// end of form.cpp 
//
