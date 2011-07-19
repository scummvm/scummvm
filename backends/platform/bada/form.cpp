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

#include "common/translation.h"

#include "form.h"
#include "system.h"

using namespace Osp::App;
using namespace Osp::Base;
using namespace Osp::Base::Runtime;
using namespace Osp::Graphics;
using namespace Osp::Ui;
using namespace Osp::Ui::Controls;

#define SHORTCUT_TIMEOUT 4000
#define SHORTCUT_SWAP_MOUSE  0
#define SHORTCUT_ESCAPE      1
#define SHORTCUT_F5          2
#define SHORTCUT_PERIOD      3
#define SHORTCUT_SPACE       4

//
// BadaAppForm
//
BadaAppForm::BadaAppForm() : 
  gameThread(0), 
  state(InitState),
  shortcutTimer(0),
  shortcutIndex(-1),
  leftButton(true) {
  eventQueueLock = new Mutex();
  eventQueueLock->Create();
}

result BadaAppForm::Construct() {
  result r = Form::Construct(Controls::FORM_STYLE_NORMAL);
  if (IsFailed(r)) {
    return r;
  }

  BadaSystem* badaSystem = null;
  gameThread = null;

  badaSystem = new BadaSystem(this);
  r = badaSystem != null ? E_SUCCESS : E_OUT_OF_MEMORY;
  
  if (!IsFailed(r)) {
    r = badaSystem->Construct();
  }

  if (!IsFailed(r)) {
    gameThread = new Thread();
    r = gameThread != null ? E_SUCCESS : E_OUT_OF_MEMORY;
  }

  if (!IsFailed(r)) {
    r = gameThread->Construct(*this);
  }

  if (IsFailed(r)) {
    if (badaSystem != null) {
      delete badaSystem;
    }
    if (gameThread != null) {
      delete gameThread;
      gameThread = null;
    }
  }
  else {
    g_system = badaSystem;
  }

  return r;
}

BadaAppForm::~BadaAppForm() {
  logEntered();

  if (gameThread) {
    terminate();
    gameThread->Stop();
    gameThread->Join();
    delete gameThread;
    gameThread = null;
  }

  if (eventQueueLock) {
    delete eventQueueLock;
    eventQueueLock = null;
  }

  logLeaving();
}

//
// abort the game thread
//
void BadaAppForm::terminate() {
  if (state == ActiveState) {
    ((BadaSystem*) g_system)->setMute(true);

    eventQueueLock->Acquire();

    Common::Event e;
    e.type = Common::EVENT_QUIT;
    eventQueue.push(e);
    state = ClosingState;

    eventQueueLock->Release();

    // block while thread ends
    AppLog("waiting for shutdown");
    for (int i = 0; i < 10 && state == ClosingState; i++) {
      Thread::Sleep(250);
    }
  }
}

result BadaAppForm::OnInitializing(void) {
  logEntered();

  SetOrientation(ORIENTATION_LANDSCAPE);
  AddOrientationEventListener(*this);
  AddTouchEventListener(*this);
  AddKeyEventListener(*this);

  // set focus to enable receiving key events
  SetFocusable(true);
  SetFocus();

  return E_SUCCESS;
}

result BadaAppForm::OnDraw(void) {
  logEntered();

  if (g_system) {
    BadaSystem* system = (BadaSystem*) g_system;
    BadaGraphicsManager* graphics = system->getGraphics();
    if (graphics && graphics->isReady()) {
      g_system->updateScreen();
    }
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

void BadaAppForm::pushKey(Common::KeyCode keycode) {
  Common::Event e;
  e.synthetic = false;
  e.kbd.keycode = keycode;
  e.kbd.ascii = keycode;
  e.kbd.flags = 0;

  eventQueueLock->Acquire();

  e.type = Common::EVENT_KEYDOWN;
  eventQueue.push(e);
  e.type = Common::EVENT_KEYUP;
  eventQueue.push(e);

  eventQueueLock->Release();
}

void BadaAppForm::OnOrientationChanged(const Control& source, 
                                       OrientationStatus orientationStatus) {
  logEntered();
  if (state == InitState) {
    state = ActiveState;
    gameThread->Start();
  }
}

Object* BadaAppForm::Run(void) {
  scummvm_main(0, 0);

  AppLog("scummvm_main completed");

  if (state == ActiveState) {
    Application::GetInstance()->SendUserEvent(USER_MESSAGE_EXIT, null);
  }
  state = DoneState;
  return null;
}

int BadaAppForm::getShortcutIndex() {
  if (shortcutTimer) {
    uint32 nextTimer = g_system->getMillis();
    if (shortcutTimer + SHORTCUT_TIMEOUT < nextTimer) {
      // double tap has expired
      shortcutTimer = 0;
      shortcutIndex = -1;
    }
  }
  return shortcutIndex;
}

void BadaAppForm::OnTouchDoublePressed(const Control& source, 
                                       const Point& currentPosition, 
                                       const TouchEventInfo& touchInfo) {
  int index = getShortcutIndex();
  shortcutIndex = (index == -1 ? 0 : index + 1);
  shortcutTimer = g_system->getMillis();

  switch (shortcutIndex) {
  case SHORTCUT_SWAP_MOUSE:
    g_system->displayMessageOnOSD(_("Sweep = Swap Left/Right Mouse"));
    break;

  case SHORTCUT_F5:
    g_system->displayMessageOnOSD(_("Sweep = Game Menu"));
    break;

  case SHORTCUT_ESCAPE:
    g_system->displayMessageOnOSD(_("Sweep = Escape"));
    break;

  case SHORTCUT_PERIOD:
    g_system->displayMessageOnOSD(_("Sweep = Dot Key"));
    break;

  case SHORTCUT_SPACE:
    g_system->displayMessageOnOSD(_("Sweep = Space Key"));
    break;
    
  default:
    shortcutIndex = -1;
  }
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
  pushKey(Common::KEYCODE_RETURN);
}

void BadaAppForm::OnTouchMoved(const Control& source, 
                               const Point& currentPosition, 
                               const TouchEventInfo& touchInfo) {
  if (getShortcutIndex() == -1) {
    pushEvent(Common::EVENT_MOUSEMOVE, currentPosition);
  }
}

void BadaAppForm::OnTouchPressed(const Control& source, 
                                 const Point& currentPosition, 
                                 const TouchEventInfo& touchInfo) {
  if (getShortcutIndex() == -1) {
    pushEvent(leftButton ? Common::EVENT_LBUTTONDOWN : Common::EVENT_RBUTTONDOWN,
              currentPosition);
  }
}

void BadaAppForm::OnTouchReleased(const Control& source, 
                                  const Point& currentPosition, 
                                  const TouchEventInfo& touchInfo) {
  if (getShortcutIndex() == -1) {
    pushEvent(leftButton ? Common::EVENT_LBUTTONUP : Common::EVENT_RBUTTONUP,
              currentPosition);
  }
  else if (touchInfo.IsFlicked()) {
    bool repeat = false;
    switch (shortcutIndex) {
    case SHORTCUT_SWAP_MOUSE:
      leftButton = !leftButton;
      g_system->displayMessageOnOSD(leftButton ? _("Left Button") : _("Right Button"));
      break;

    case SHORTCUT_F5:
      pushKey(Common::KEYCODE_F5);
      break;

    case SHORTCUT_ESCAPE:
      pushKey(Common::KEYCODE_ESCAPE);
      repeat = true;
      break;
      
    case SHORTCUT_PERIOD:
      pushKey(Common::KEYCODE_PERIOD);
      repeat = true;
      break;

    case SHORTCUT_SPACE:
      pushKey(Common::KEYCODE_SPACE);
      repeat = true;
      break;
    }

    if (repeat) {
      // allow key repeat
      shortcutTimer = g_system->getMillis();
    }
  }
}

void BadaAppForm::OnKeyLongPressed(const Control& source, KeyCode keyCode) {
  logEntered();
  switch (keyCode) {
  case KEY_SIDE_UP:
    g_system->displayMessageOnOSD(_("Volume Up"));
    ((BadaSystem*) g_system)->setVolume(true, true);
    return;

  case KEY_SIDE_DOWN:
    g_system->displayMessageOnOSD(_("Volume Down"));
    ((BadaSystem*) g_system)->setVolume(false, true);
    return;

  case KEY_CAMERA:
    pushKey(Common::KEYCODE_F5);
    return;

  default:
    break;
  }
}

void BadaAppForm::OnKeyPressed(const Control& source, KeyCode keyCode) {
  switch (keyCode) {
  case KEY_SIDE_UP:
    g_system->displayMessageOnOSD(_("Volume Up"));
    ((BadaSystem*) g_system)->setVolume(true, false);
    return;

  case KEY_SIDE_DOWN:
    g_system->displayMessageOnOSD(_("Volume Down"));
    ((BadaSystem*) g_system)->setVolume(false, false);
    return;

  case KEY_CAMERA:
    // display the soft keyboard
    pushKey(Common::KEYCODE_F7);
    return;

  default:
    break;
  }
}

void BadaAppForm::OnKeyReleased(const Control& source, KeyCode keyCode) {
  logEntered();
}

//
// end of form.cpp 
//
