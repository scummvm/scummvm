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

#ifndef BADA_SYSTEM_H
#define BADA_SYSTEM_H

#include <FApp.h>
#include <FGraphics.h>
#include <FUi.h>
#include <FSystem.h>
#include <FBase.h>
#include <FIoFile.h>

#include "config.h"
#include "common/scummsys.h"
#include "common/events.h"
#include "base/main.h"
#include "backends/modular-backend.h"
#include "backends/mutex/mutex.h"
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "backends/events/default/default-events.h"
#include "backends/audiocd/default/default-audiocd.h"
#include "backends/fs/fs-factory.h"
#include "audio/mixer_intern.h"

#include "fs.h"
#include "form.h"

#if defined(_DEBUG)
#define logEntered() AppLog("%s entered (%s %d)", \
                             __FUNCTION__, __FILE__, __LINE__);
#define logLeaving() AppLog("%s leaving (%s %d)", \
                             __FUNCTION__, __FILE__, __LINE__);
#else
#define logEntered()
#define logLeaving()
#endif

BadaAppForm* systemStart(Osp::App::Application* app);
void systemError(const char* format, ...);

#define USER_MESSAGE_EXIT 1000

//
// BadaSystem
//
class BadaSystem : public ModularBackend, 
                   Common::EventSource {
 public:
  BadaSystem(BadaAppForm* appForm);
  ~BadaSystem();

  result Construct();
  void closeGraphics();

 private:
  void initBackend();
  result initModules();

  void updateScreen();
  bool pollEvent(Common::Event& event);
  uint32 getMillis();
  void delayMillis(uint msecs);
  void getTimeAndDate(TimeDate& t) const;
  void fatalError();
  void logMessage(LogMessageType::Type type, const char *message);

  Common::SeekableReadStream* createConfigReadStream();
  Common::WriteStream* createConfigWriteStream();
  
  BadaAppForm* appForm;
};

#endif
