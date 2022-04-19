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

#ifndef MTROPOLIS_MTROPOLIS_H
#define MTROPOLIS_MTROPOLIS_H

#include "mtropolis/detection.h"

#include "engines/engine.h"

#include "common/random.h"

/**
 * This is the namespace of the mTropolis engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Obsidian
 */
namespace MTropolis {

class Runtime;

class MTropolisEngine : public ::Engine {
protected:

	// Engine APIs
	Common::Error run() override;

public:
	MTropolisEngine(OSystem *syst, const MTropolisGameDescription *gameDesc);
	~MTropolisEngine() override;

	bool hasFeature(EngineFeature f) const override;
	//void syncSoundSettings() override;

	const MTropolisGameDescription *_gameDescription;
	uint32 getGameID() const;
	uint16 getVersion() const;
	Common::Platform getPlatform() const;

public:
	void handleEvents();

protected:
	void pauseEngineIntern(bool pause) override;

private:
	Common::ScopedPtr<Runtime> _runtime;
};

} // End of namespace MTropolis

#endif /* MTROPOLIS_H */
