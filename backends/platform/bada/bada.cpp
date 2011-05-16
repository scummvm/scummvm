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

#if defined(BADA)
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "audio/mixer_intern.h"
#include "common/scummsys.h"

#define DEFAULT_CONFIG_FILE "scummvm.ini"

#include "backends/fs/bada/bada-fs-factory.h"

class OSystemBada : public ModularBackend {
public:
  OSystemBada();
  virtual ~OSystemBada();

  virtual void initBackend();
  virtual bool pollEvent(Common::Event &event);
  virtual uint32 getMillis();
  virtual void delayMillis(uint msecs);
  virtual void getTimeAndDate(TimeDate &t) const {}

  virtual Common::SeekableReadStream *createConfigReadStream();
  virtual Common::WriteStream *createConfigWriteStream();
};

OSystemBada::OSystemBada() {
  _fsFactory = new BADAFilesystemFactory();
}

OSystemBada::~OSystemBada() {
}

void OSystemBada::initBackend() {
  _mutexManager = (MutexManager *)new NullMutexManager();
  _timerManager = new DefaultTimerManager();
  _eventManager = new DefaultEventManager(this);
  _savefileManager = new DefaultSaveFileManager();
  _graphicsManager = (GraphicsManager *)new NullGraphicsManager();
  _audiocdManager = (AudioCDManager *)new DefaultAudioCDManager();
  _mixer = new Audio::MixerImpl(this, 22050);

  ((Audio::MixerImpl *)_mixer)->setReady(false);

  // Note that both the mixer and the timer manager are useless
  // this way; they need to be hooked into the system somehow to
  // be functional. Of course, can't do that in a NULL backend :).

  OSystem::initBackend();
}

bool OSystemBada::pollEvent(Common::Event &event) {
  return false;
}

uint32 OSystemBada::getMillis() {
  return 0;
}

void OSystemBada::delayMillis(uint msecs) {
}

Common::SeekableReadStream *OSystemBada::createConfigReadStream() {
  Common::FSNode file(DEFAULT_CONFIG_FILE);
  return file.createReadStream();
}

Common::WriteStream *OSystemBada::createConfigWriteStream() {
  Common::FSNode file(DEFAULT_CONFIG_FILE);
  return file.createWriteStream();
}

OSystem *OSystemBadaCreate() {
  return new OSystemBada();
}

int main(int argc, char *argv[]) {
  g_system = OSystemBadaCreate();
  assert(g_system);

  // Invoke the actual ScummVM main entry point:
  int res = scummvm_main(argc, argv);
  delete (OSystemBada*)g_system;
  return res;
}

#endif
