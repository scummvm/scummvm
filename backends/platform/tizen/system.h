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

#ifndef TIZEN_SYSTEM_H
#define TIZEN_SYSTEM_H

#include <FApp.h>
#include <FGraphics.h>
#include <FUi.h>
#include <FSystem.h>
#include <FBase.h>
#include <FIoFile.h>

#include "config.h"
#include "common/scummsys.h"
#include "backends/modular-backend.h"

#include "backends/platform/tizen/fs.h"
#include "backends/platform/tizen/form.h"
#include "backends/platform/tizen/audio.h"
#include "backends/platform/tizen/graphics.h"

#if defined(_DEBUG)
#define logEntered() AppLog("%s entered (%s %d)", __FUNCTION__, __FILE__, __LINE__);
#define logLeaving() AppLog("%s leaving (%s %d)", __FUNCTION__, __FILE__, __LINE__);
#else
#define logEntered()
#define logLeaving()
#endif

TizenAppForm *systemStart(Tizen::App::Application *app);
void systemError(const char *message);

#define USER_MESSAGE_EXIT		1000
#define USER_MESSAGE_EXIT_ERR		1001
#define USER_MESSAGE_EXIT_ERR_CONFIG 1002

//
// TizenSystem
//
class TizenSystem :
	public ModularBackend,
	Common::EventSource {
public:
	TizenSystem(TizenAppForm *appForm);
	~TizenSystem();

	result Construct();
	void closeAudio();
	void closeGraphics();
	void destroyBackend();
	void setMute(bool on);
	void exitSystem();
	bool isClosing() { return _appForm->isClosing(); }

	TizenGraphicsManager *getGraphics() {
		return dynamic_cast<TizenGraphicsManager *>(_graphicsManager);
	}

private:
	void initBackend();
	result initModules();

	void updateScreen();
	bool pollEvent(Common::Event &event);
	uint32 getMillis(bool skipRecord = false);
	void delayMillis(uint msecs);
	void getTimeAndDate(TimeDate &t) const;
	void fatalError();
	void quit();
	void logMessage(LogMessageType::Type type, const char *message);
	void addSysArchivesToSearchSet(Common::SearchSet &s, int priority);

	Common::EventSource *getDefaultEventSource() { return this; }
	Common::SeekableReadStream *createConfigReadStream();
	Common::WriteStream *createConfigWriteStream();

	TizenAppForm *_appForm;
	AudioThread *_audioThread;
	long long _epoch;
	Common::String _resourcePath;
};

#endif
