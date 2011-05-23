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

#include "common/scummsys.h"
#include "common/events.h"
#include "fs-factory.h"
#include "backends/modular-backend.h"
#include "base/main.h"
#include "backends/mutex/mutex.h"
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "backends/events/default/default-events.h"
#include "backends/audiocd/default/default-audiocd.h"
#include "audio/mixer_intern.h"

#include <stdarg.h>

#include "system.h"
#include "graphics.h"

using namespace Osp::Base;
using namespace Osp::Base::Runtime;

#define DEFAULT_CONFIG_FILE "scummvm.ini"

//
// BadaSaveFileManager
//
struct BadaSaveFileManager : public DefaultSaveFileManager {
  bool removeSavefile(const Common::String &filename);
};

bool BadaSaveFileManager::removeSavefile(const Common::String &filename) {
  Common::String savePathName = getSavePath();

  checkPath(Common::FSNode(savePathName));
  if (getError().getCode() != Common::kNoError) {
    return false;
  }

  // recreate FSNode since checkPath may have changed/created the directory
  Common::FSNode savePath(savePathName);
  Common::FSNode file = savePath.getChild(filename);

  switch (Osp::Io::File::Remove(filename.c_str())) {
  case E_SUCCESS:
    return true;

  case E_ILLEGAL_ACCESS:
    setError(Common::kWritePermissionDenied, "Search or write permission denied: " + 
             file.getName());
    break;

  default:
    setError(Common::kPathDoesNotExist, "removeSavefile: '" + file.getName() +
             "' does not exist or path is invalid");
    break;
  }

  return false;
}

//
// BadaMutexManager
//
struct BadaMutexManager : public MutexManager {
  OSystem::MutexRef createMutex();
  void lockMutex(OSystem::MutexRef mutex);
  void unlockMutex(OSystem::MutexRef mutex);
  void deleteMutex(OSystem::MutexRef mutex);
};

OSystem::MutexRef BadaMutexManager::createMutex() {
  return (OSystem::MutexRef) new Mutex();
}

void BadaMutexManager::lockMutex(OSystem::MutexRef mutex) {
  Mutex* m = (Mutex*) mutex;
  m->Acquire();
}

void BadaMutexManager::unlockMutex(OSystem::MutexRef mutex) {
  Mutex* m = (Mutex*) mutex;
  m->Release();
}

void BadaMutexManager::deleteMutex(OSystem::MutexRef mutex) {
  Mutex* m = (Mutex*) mutex;
  delete m;
}

//
// BadaSystem
//
struct BadaSystem : public ModularBackend, 
                    Common::EventSource {
  BadaSystem(BadaAppForm* appForm);
  ~BadaSystem();

  result Construct();
  void initBackend();
  result initModules();

  bool pollEvent(Common::Event &event);
  uint32 getMillis();
  void delayMillis(uint msecs);
  void getTimeAndDate(TimeDate &t) const;
  void fatalError();
  void logMessage(LogMessageType::Type type, const char *message);

  Common::SeekableReadStream* createConfigReadStream();
  Common::WriteStream* createConfigWriteStream();

  BadaAppForm* appForm;
};

BadaSystem::BadaSystem(BadaAppForm* appForm) : appForm(appForm) {
}

result BadaSystem::Construct(void) {
  logEntered();

  _fsFactory = new BADAFilesystemFactory();
  if (!_fsFactory) {
    return E_OUT_OF_MEMORY;
  }

  return E_SUCCESS;
}

BadaSystem::~BadaSystem() {
}

result BadaSystem::initModules() {
  logEntered();

  _mutexManager = new BadaMutexManager();
  if (!_mutexManager) {
    return E_OUT_OF_MEMORY;
  }

  _timerManager = new DefaultTimerManager();
  if (!_timerManager) {
    return E_OUT_OF_MEMORY;
  }

  _eventManager = new DefaultEventManager(this);
  if (!_eventManager) {
    return E_OUT_OF_MEMORY;
  }

  _savefileManager = new BadaSaveFileManager();
  if (!_savefileManager) {
    return E_OUT_OF_MEMORY;
  }

  _graphicsManager = (GraphicsManager*) new BadaGraphicsManager(appForm);
  if (!_graphicsManager) {
    return E_OUT_OF_MEMORY;
  }

  _mixer = new Audio::MixerImpl(this, 22050);
  if (!_mixer) {
    return E_OUT_OF_MEMORY;
  }

  _audiocdManager = (AudioCDManager*) new DefaultAudioCDManager();
  if (!_audiocdManager) {
    return E_OUT_OF_MEMORY;
  }

  ((Audio::MixerImpl*) _mixer)->setReady(false);
  logLeaving();
  return E_SUCCESS;
}

void BadaSystem::initBackend() {
  logEntered();

  if (E_SUCCESS != initModules()) {
    systemError("initModules failed");
  }
  else {
    OSystem::initBackend();
  }

  logLeaving();
}

bool BadaSystem::pollEvent(Common::Event &event) {
  Thread::Sleep(100);
  return false;
}

uint32 BadaSystem::getMillis() {
  uint32 result = 0;
  DateTime currentTime;
  if (E_SUCCESS == Osp::System::SystemTime::GetCurrentTime(currentTime)) {
    result = currentTime.GetTime().GetMilliseconds();
  }
  else {
    fatalError();
  }
  return result;
}

void BadaSystem::delayMillis(uint msecs) {
  Thread::Sleep(msecs);
}

void BadaSystem::getTimeAndDate(TimeDate &td) const {
  DateTime currentTime;

  if (E_SUCCESS == Osp::System::SystemTime::GetCurrentTime(currentTime)) {
    td.tm_sec = currentTime.GetSecond();
    td.tm_min = currentTime.GetMinute();
    td.tm_hour = currentTime.GetHour();
    td.tm_mday = currentTime.GetDay();
    td.tm_mon = currentTime.GetMonth();
    td.tm_year = currentTime.GetYear();
  }
}

void BadaSystem::fatalError() {
  systemError("ScummVM: Fatal internal error.");
  exit(1);
}

void BadaSystem::logMessage(LogMessageType::Type /*type*/, const char *message) {
  AppLog(message);
}

Common::SeekableReadStream* BadaSystem::createConfigReadStream() {
  Common::FSNode file(DEFAULT_CONFIG_FILE);
  return file.createReadStream();
}

Common::WriteStream* BadaSystem::createConfigWriteStream() {
  Common::FSNode file(DEFAULT_CONFIG_FILE);
  return file.createWriteStream();
}

//
// BadaAppForm
//
result BadaAppForm::Construct() {
  result r = Form::Construct(Osp::Ui::Controls::FORM_STYLE_NORMAL);
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
    }
  }
  else {
    g_system = badaSystem;
    pThread->Start();
  }

  return r;
}

BadaAppForm::~BadaAppForm() {
  if (pThread) {
    delete pThread;
  }
}

result BadaAppForm::OnDraw(void) {
  logEntered();
  //  g_system->updateScreen();
  return E_SUCCESS;
}

Object* BadaAppForm::Run(void) {
  scummvm_main(0, 0);
  AppLog("scummvm_main completed");
  delete this;
  return null;
}

//
// create the scummVM system
//
BadaAppForm* systemStart(Osp::App::Application* app) {
  logEntered();

  BadaAppForm* appForm = new BadaAppForm();
  if (!appForm) {
    systemError("Failed to create appForm");
    return null;
  }

  if (E_SUCCESS != appForm->Construct() ||
      E_SUCCESS != app->GetAppFrame()->GetFrame()->AddControl(*appForm)) {
    delete appForm;
    systemError("Failed to construct appForm");
    return null;
  }

  return appForm;
}

//
// prepares to halt the application
//
void systemStop(BadaAppForm* appForm) {
}

//
// adds a new event to the event queue
//
void systemPostEvent() {
}

//
// display a fatal error notification
//
void systemError(const char* format, ...) {
  va_list ap;
  char buffer[255];

  va_start(ap, format);
  vsnprintf(buffer, sizeof(buffer), format, ap);
  va_end(ap);
  
  AppLog(buffer);
  // TODO: display in a popup
}

