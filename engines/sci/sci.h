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
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_H
#define SCI_H

#include "engines/engine.h"
#include "engines/advancedDetector.h"

#include "sci/console.h"

namespace Sci {

// our engine debug levels
enum kDebugLevels {
	kDebugLevelError      = 1 << 0,
	kDebugLevelNodes      = 1 << 1,
	kDebugLevelGraphics   = 1 << 2,
	kDebugLevelStrings    = 1 << 3,
	kDebugLevelMem        = 1 << 4,
	kDebugLevelFuncCheck  = 1 << 5,
	kDebugLevelBresen     = 1 << 6,
	kDebugLevelSound      = 1 << 7,
	kDebugLevelGfxDriver  = 1 << 8,
	kDebugLevelBaseSetter = 1 << 9,
	kDebugLevelParser     = 1 << 10,
	kDebugLevelMenu       = 1 << 11,
	kDebugLevelSaid       = 1 << 12,
	kDebugLevelFile       = 1 << 13,
	kDebugLevelTime       = 1 << 14,
	kDebugLevelRoom       = 1 << 15,
	kDebugLevelAvoidPath  = 1 << 16
};

struct GameFlags {
	//int gameType;
	//int gameId;
	//uint32 features;
	// SCI Version
	// Resource Map Version
	// etc...
};

struct SciGameDescription {
	ADGameDescription desc;
	GameFlags flags;
	int version;
};

class SciEngine : public Engine {
public:
	SciEngine(OSystem *syst, const SciGameDescription *desc);
	~SciEngine();

	virtual Common::Error init(void);
	virtual Common::Error go(void);

	GUI::Debugger *getDebugger() { return _console; }

	const char* getGameID() const;
	int getVersion() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	uint32 getFlags() const;

	Common::String getSavegameName(int nr) const;

private:
	const SciGameDescription *_gameDescription;
	Console *_console;
};

} // End of namespace Sci

#endif // SCI_H
