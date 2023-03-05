/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef PLATFORM_ATARI_H
#define PLATFORM_ATARI_H

#include "backends/modular-backend.h"

class OSystem_Atari : public ModularMixerBackend, public ModularGraphicsBackend {
public:
	OSystem_Atari();
	virtual ~OSystem_Atari();

	void initBackend() override;

	Common::MutexInternal *createMutex() override;
	uint32 getMillis(bool skipRecord = false) override;
	void delayMillis(uint msecs) override;
	void getTimeAndDate(TimeDate &td, bool skipRecord = false) const override;

	Common::KeymapArray getGlobalKeymaps() override;
	Common::HardwareInputSet *getHardwareInputSet() override;

	void quit() override;

	void logMessage(LogMessageType::Type type, const char *message) override;

	void addSysArchivesToSearchSet(Common::SearchSet &s, int priority) override;
	Common::String getDefaultConfigFileName() override;

	void update();

private:
	long _startTime;

	bool _video_initialized = false;
	bool _ikbd_initialized = false;
};

#endif
