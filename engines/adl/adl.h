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

#ifndef ADL_ADL_H
#define ADL_ADL_H

#include "common/random.h"
#include "engines/engine.h"
#include "gui/debugger.h"

namespace Common {
class ReadStream;
class SeekableReadStream;
}

namespace Adl {

class Display;
class Parser;
class Console;
struct AdlGameDescription;

enum GameType {
	kGameTypeNone = 0,
	kGameTypeHires1
};

Common::String asciiToApple(Common::String str);
Common::String appleToAscii(Common::String str);

enum {
	STR_COMMON_ENTERCMD,
	STR_COMMON_VERBERR,
	STR_COMMON_NOUNERR,
	STR_CUSTOM_START
};

#define A2CHAR(C) ((C) | 0x80)

class AdlEngine : public Engine {
public:
	AdlEngine(OSystem *syst, const AdlGameDescription *gd);
	virtual ~AdlEngine();

	const AdlGameDescription *_gameDescription;
	uint32 getFeatures() const;
	const char *getGameId() const;

	static AdlEngine *create(GameType type, OSystem *syst, const AdlGameDescription *gd);

	Common::Error run();
	virtual Common::String getExeString(uint id) = 0;

protected:
	virtual void runGame() = 0;
	Common::String readString(Common::ReadStream &stream, byte until = 0);
	void printStrings(Common::SeekableReadStream &stream, int count = 1);
	Display *_display;
	Parser *_parser;

private:
	Console *_console;

	// We need random numbers
	Common::RandomSource *_rnd;
};

// Example console class
class Console : public GUI::Debugger {
public:
	Console(AdlEngine *vm) {}
	virtual ~Console(void) {}
};

AdlEngine *AdlEngine_v1__create(OSystem *syst, const AdlGameDescription *gd);

} // End of namespace Adl

#endif
