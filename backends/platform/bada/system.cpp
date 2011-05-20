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

#include "backends/modular-backend.h"
#include "base/main.h"
#include "backends/mutex/mutex.h"
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "backends/events/default/default-events.h"
#include "backends/audiocd/default/default-audiocd.h"
#include "backends/graphics/opengl/opengl-graphics.h"
#include "audio/mixer_intern.h"
#include "common/scummsys.h"
#include "common/events.h"
#include "fs-factory.h"

#include <FApp.h>
#include <FGraphics.h>
#include <FUi.h>
#include <FSystem.h>
#include <FBase.h>
#include <FIoFile.h>

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
// BadaGraphicsManager
//
struct BadaGraphicsManager : public OpenGLGraphicsManager {
  void setInternalMousePosition(int x, int y);
};

void BadaGraphicsManager::setInternalMousePosition(int x, int y) {
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
class BadaSystem : public ModularBackend, public IRunnable {
public:
  BadaSystem();
  ~BadaSystem();

  result Construct();
  Object* Run();

  void initBackend();
  //bool pollEvent(Common::Event &event);
  uint32 getMillis();
  void delayMillis(uint msecs);
  void getTimeAndDate(TimeDate &t) const;
  void fatalError();
  void logMessage(LogMessageType::Type type, const char *message);

  Common::SeekableReadStream* createConfigReadStream();
  Common::WriteStream* createConfigWriteStream();

private:
  Thread* pThread;
};

BadaSystem::BadaSystem() {
  Construct();
}

result BadaSystem::Construct(void) {
  pThread = new Thread;
  if (pThread == null) {
    return E_OUT_OF_MEMORY;
  }

  result r = pThread->Construct(*this);
  if (r != E_OUT_OF_MEMORY) {
    pThread->Start();
  }

  return r;
}

BadaSystem::~BadaSystem() {
  delete pThread;
}

Object* BadaSystem::Run(void) {
  scummvm_main(0, 0);
  delete this;
  return null;
}

void BadaSystem::initBackend() {
  _fsFactory = new BADAFilesystemFactory();
  _mutexManager = new BadaMutexManager();
  _timerManager = new DefaultTimerManager();
  //  _eventManager = new DefaultEventManager(this);
  _savefileManager = new BadaSaveFileManager();
  _graphicsManager = (GraphicsManager*) new BadaGraphicsManager();
  _audiocdManager = (AudioCDManager*) new DefaultAudioCDManager();
  _mixer = new Audio::MixerImpl(this, 22050);
  ((Audio::MixerImpl*) _mixer)->setReady(false);

  OSystem::initBackend();
}

//bool BadaSystem::pollEvent(Common::Event &event) {
// return false;
//}

uint32 BadaSystem::getMillis() {
  return 0;
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
  
}

void BadaSystem::logMessage(LogMessageType::Type type, const char *message) {
  
}

Common::SeekableReadStream* BadaSystem::createConfigReadStream() {
  Common::FSNode file(DEFAULT_CONFIG_FILE);
  return file.createReadStream();
}

Common::WriteStream* BadaSystem::createConfigWriteStream() {
  Common::FSNode file(DEFAULT_CONFIG_FILE);
  return file.createWriteStream();
}

// create the scummVM system
void systemStart() {
  g_system = new BadaSystem();
}

// prepares to halt the application
void systemStop() {
}

// adds a new event to the event queue
void systemPostEvent() {
}
