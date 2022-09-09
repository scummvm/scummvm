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

#ifndef WATCHMAKER_WATCHMAKER_H
#define WATCHMAKER_WATCHMAKER_H

#include "engines/engine.h"

#include "common/platform.h"
#include "common/queue.h"
#include "common/rect.h"
#include "common/str.h"

#include "audio/mixer.h"

struct ADGameDescription;

namespace Watchmaker {

class Console;

enum WatchmakerDebugChannels {
	kDebugGeneral = 1 << 0
};

class WatchmakerGame : public Engine {
public:
	WatchmakerGame(OSystem *syst, const ADGameDescription *gameDesc);
	~WatchmakerGame() override;

	Common::Error run() override;

	// Detection related functions
	const ADGameDescription *_gameDescription;
	const char *getGameId() const;
	Common::Platform getPlatform() const;
private:
	Console *_console;
};
} // End of namespace Watchmaker

#endif
