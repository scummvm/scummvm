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

#include <FUiCtrlMessageBox.h>

#include "common/config-manager.h"
#include <stdarg.h>

#include "form.h"
#include "system.h"
#include "graphics.h"
#include "audio.h"

using namespace Osp::Base;
using namespace Osp::Base::Runtime;
using namespace Osp::Ui::Controls;

#define DEFAULT_CONFIG_FILE "/Home/scummvm.ini"

//
// BadaFilesystemFactory
//
class BadaFilesystemFactory : public FilesystemFactory {
  AbstractFSNode *makeRootFileNode() const;
  AbstractFSNode *makeCurrentDirectoryFileNode() const;
  AbstractFSNode *makeFileNodePath(const Common::String &path) const;
};

AbstractFSNode *BadaFilesystemFactory::makeRootFileNode() const {
	return new BadaFilesystemNode("/");
}

AbstractFSNode *BadaFilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new BadaFilesystemNode("/Home");
}

AbstractFSNode *BadaFilesystemFactory::makeFileNodePath(const Common::String &path) const {
  AppAssert(!path.empty());
  return new BadaFilesystemNode(path);
}

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

  String unicodeFileName;
  StringUtil::Utf8ToString(filename.c_str(), unicodeFileName);

  switch (Osp::Io::File::Remove(unicodeFileName)) {
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
  Mutex* mutex = new Mutex();
  mutex->Create();
  return (OSystem::MutexRef) mutex;
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
BadaSystem::BadaSystem(BadaAppForm* appForm) : 
  appForm(appForm),
  audioThread(0), 
  epoch(0) {
}

result BadaSystem::Construct(void) {
  logEntered();

  _fsFactory = new BadaFilesystemFactory();
  if (!_fsFactory) {
    return E_OUT_OF_MEMORY;
  }

  return E_SUCCESS;
}

BadaSystem::~BadaSystem() {
  if (audioThread) {
    audioThread->Stop();
    delete audioThread;
    audioThread = null;
  }
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

  _savefileManager = new BadaSaveFileManager();
  if (!_savefileManager) {
    return E_OUT_OF_MEMORY;
  }

  _graphicsManager = (GraphicsManager*) new BadaGraphicsManager(appForm);
  if (!_graphicsManager) {
    return E_OUT_OF_MEMORY;
  }

  // depends on _graphicsManager when ENABLE_VKEYBD enabled
  _eventManager = new DefaultEventManager(this); 
  if (!_eventManager) {
    return E_OUT_OF_MEMORY;
  }

  audioThread = new AudioThread();
  if (!audioThread) {
    return E_OUT_OF_MEMORY;
  }

  _mixer = audioThread->Construct(this);
  if (!_mixer) {
    return E_OUT_OF_MEMORY;
  }

  _audiocdManager = (AudioCDManager*) new DefaultAudioCDManager();
  if (!_audiocdManager) {
    return E_OUT_OF_MEMORY;
  }

  if (IsFailed(audioThread->Start())) {
    AppLog("Failed to start audio thread");
    return E_OUT_OF_MEMORY;
  }

  logLeaving();
  return E_SUCCESS;
}

void BadaSystem::initBackend() {
  logEntered();

  // allow translations to be found
  ConfMan.set("themepath", "/Res/scummmobile");

  // use the mobile device theme
  ConfMan.set("gui_theme", "mobile");

  // allow virtual keypad pack to be found
  ConfMan.set("vkeybdpath", "/Res/vkeybd_default");

  // set default save path to writable area
  if (!ConfMan.hasKey("savepath")) {
    ConfMan.set("savepath", "/Home");
  }

	ConfMan.registerDefault("fullscreen", true);
	ConfMan.registerDefault("aspect_ratio", true);

  Osp::System::SystemTime::GetTicks(epoch);

  if (E_SUCCESS != initModules()) {
    systemError("initModules failed");
  }
  else {
    OSystem::initBackend();
  }

  logLeaving();
}

bool BadaSystem::pollEvent(Common::Event& event) {
  return appForm->pollEvent(event);
}

uint32 BadaSystem::getMillis() {
  long long result, ticks = 0;
  Osp::System::SystemTime::GetTicks(ticks);
  result = ticks - epoch;
  return result;
}

void BadaSystem::delayMillis(uint msecs) {
  Thread::Sleep(msecs);
}

void BadaSystem::updateScreen() {
  if (_graphicsManager) {
    _graphicsManager->updateScreen();
  }
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
}

void BadaSystem::logMessage(LogMessageType::Type /*type*/, const char *message) {
  AppLog(message);
}

Common::SeekableReadStream* BadaSystem::createConfigReadStream() {
  BadaFilesystemNode file(DEFAULT_CONFIG_FILE);
  return file.createReadStream();
}

Common::WriteStream* BadaSystem::createConfigWriteStream() {
  BadaFilesystemNode file(DEFAULT_CONFIG_FILE);
  return file.createWriteStream();
}

void BadaSystem::closeGraphics() {
  if (_graphicsManager) {
    delete _graphicsManager;
    _graphicsManager = null;
  }
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
// display a fatal error notification
//
void systemError(const char* format, ...) {
  va_list ap;
  char buffer[255];

  va_start(ap, format);
  vsnprintf(buffer, sizeof(buffer), format, ap);
  va_end(ap);
  
  AppLog(buffer);

  // prevent graphicsmanager from overwriting the popup
  if (g_system) {
    ((BadaSystem*) g_system)->closeGraphics();
  }

  MessageBox messageBox;
  messageBox.Construct(L"Fatal Error", buffer, MSGBOX_STYLE_OK);
  int modalResult;
  messageBox.ShowAndWait(modalResult);
  Application::GetInstance()->SendUserEvent(USER_MESSAGE_EXIT, null);
}

//
// end of system.cpp 
//
