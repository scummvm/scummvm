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

#ifndef ULTIMA2_H
#define ULTIMA2_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/util.h"
#include "engines/engine.h"
#include "ultima/detection.h"
#include "ultima/shared/engine/events.h"

namespace Ultima {
namespace Ultima2 {

struct Ultima2GameDescription;

class Ultima2Engine : public Engine, public Shared::Events {
private:
	const Ultima::UltimaGameDescription *_gameDescription;

protected:
	// Engine APIs
	Common::Error run() override;

	/**
	 * Returns true if the game should quit
	 */
	bool shouldQuit() const override {
		return Engine::shouldQuit();
	}

public:
	Ultima2Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc);
	~Ultima2Engine() override;

	uint32 getFeatures() const;

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

	bool hasFeature(EngineFeature f) const override {
		return
			(f == kSupportsReturnToLauncher);
	};
};

extern Ultima2Engine *g_engine;

} // namespace Ultima2
} // namespace Ultima

#endif
