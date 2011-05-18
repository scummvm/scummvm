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
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
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

using namespace Osp::Base;
using namespace Osp::Base::Runtime;

#define DEFAULT_CONFIG_FILE "scummvm.ini"

class BadaGraphicsManager : public OpenGLGraphicsManager {
public:
	BadaGraphicsManager() : OpenGLGraphicsManager() {
  }

  ~BadaGraphicsManager() {
  }
  
  void setInternalMousePosition(int x, int y) {
  }
};

class OSystemBada : public ModularBackend, public IRunnable {
public:
  OSystemBada();
  ~OSystemBada();

  result Construct();
  Object* Run();

  void initBackend();
  bool pollEvent(Common::Event &event);
  uint32 getMillis();
  void delayMillis(uint msecs);
  void getTimeAndDate(TimeDate &t) const;

  Common::SeekableReadStream *createConfigReadStream();
  Common::WriteStream *createConfigWriteStream();

private:
  Thread* pThread;
};

//
// OSystemBada constructor
//
OSystemBada::OSystemBada() {
  Construct();
}

result OSystemBada::Construct(void) {
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

OSystemBada::~OSystemBada() {
  delete pThread;
}

Object* OSystemBada::Run(void) {
  int res = scummvm_main(0, 0);
  delete this;
	return null;
}

void OSystemBada::initBackend() {
  _fsFactory = new BADAFilesystemFactory();
  //  _mutexManager = (MutexManager*) new NullMutexManager();
  //_timerManager = new DefaultTimerManager();
  //_eventManager = new DefaultEventManager(this);
  _savefileManager = new DefaultSaveFileManager();
  _graphicsManager = (GraphicsManager*) new BadaGraphicsManager();
  _audiocdManager = (AudioCDManager*) new DefaultAudioCDManager();
  _mixer = new Audio::MixerImpl(this, 22050);
  ((Audio::MixerImpl*) _mixer)->setReady(false);

  OSystem::initBackend();
}

bool OSystemBada::pollEvent(Common::Event &event) {
  return false;
}

uint32 OSystemBada::getMillis() {
  return 0;
}

void OSystemBada::delayMillis(uint msecs) {
  Thread::Sleep(msecs);
}

void OSystemBada::getTimeAndDate(TimeDate &t) const {
}

Common::SeekableReadStream *OSystemBada::createConfigReadStream() {
  Common::FSNode file(DEFAULT_CONFIG_FILE);
  return file.createReadStream();
}

Common::WriteStream *OSystemBada::createConfigWriteStream() {
  Common::FSNode file(DEFAULT_CONFIG_FILE);
  return file.createWriteStream();
}

//
//
//
void systemStart() {
  g_system = new OSystemBada();
  assert(g_system);
}

//
// Prepares to halt the application
//
void systemStop() {
}

//
// Adds a new event to the event queue
//
void systemPostEvent() {
}

