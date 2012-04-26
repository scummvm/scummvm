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

#ifndef TONY_H
#define TONY_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/random.h"
#include "common/util.h"
#include "engines/engine.h"

#include "tony/mpal/mpal.h"

/**
 * This is the namespace of the Tony engine.
 *
 * Status of this engine: In Development
 *
 * Games using this engine:
 * - Tony Tough
 */
namespace Tony {

using namespace MPAL;

enum {
	kTonyDebugAnimations = 1 << 0,
	kTonyDebugActions = 1 << 1,
	kTonyDebugSound = 1 << 2,
	kTonyDebugMusic = 2 << 3
};

#define DEBUG_BASIC 1
#define DEBUG_INTERMEDIATE 2
#define DEBUG_DETAILED 3

struct TonyGameDescription;

class TonyEngine : public Engine {
private:
	Common::ErrorCode Init();
protected:
	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
public:
	LPCUSTOMFUNCTION FuncList[300];
	Common::RandomSource _randomSource;
public:
	TonyEngine(OSystem *syst, const TonyGameDescription *gameDesc);
	virtual ~TonyEngine();

	const TonyGameDescription *_gameDescription;
	uint32 getGameID() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	uint16 getVersion() const;
	uint32 getFlags() const;
	Common::Platform getPlatform() const;

	void GUIError(const Common::String &msg);
};

// Global reference to the TonyEngine object
extern TonyEngine *_vm;

} // End of namespace Tony

#endif /* TONY_H */
