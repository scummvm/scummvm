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

#ifndef SLUDGE_H
#define SLUDGE_H
 
#include "common/random.h"
#include "engines/engine.h"
#include "gui/debugger.h"
 
#include "sludge/console.h"

namespace Sludge {
 
class SludgeConsole;
 
struct SludgeGameDescription;

// debug channels
enum {
	kSludgeDebugScript = 1 << 0,
	kSludgeDebugDataLoad = 1 << 1,
};
 
class SludgeEngine : public Engine {
protected:
	// Engine APIs
	virtual Common::Error run();

public:
	SludgeEngine(OSystem *syst, const SludgeGameDescription *gameDesc);
	virtual ~SludgeEngine();
 
	int getGameType() const;
	const char *getGameId() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;

	const SludgeGameDescription *_gameDescription;
 
private:
	SludgeConsole *_console;
 	Common::RandomSource *_rnd;
};

} // End of namespace Sludge
 
#endif
