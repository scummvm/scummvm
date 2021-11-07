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

#ifndef EFH_EFH_H
#define EFH_EFH_H

#include "efh/detection.h"

#include "common/file.h"
#include "common/rect.h"
#include "common/events.h"

#include "engines/engine.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

namespace Common {
class RandomSource;
}

/**
 * This is the namespace of the Efh engine.
 *
 * Status of this engine:
 * - Skeletton
 *
 * Games using this engine:
 * - Escape From Hell
 * 
 * Escape From Hell is based on a modified Wasteland engine, so this engine could eventually, one day, also support:
 * - Wasteland
 * - Fountain of Dreams
 */
namespace Efh {

static const int kSavegameVersion = 1;

struct EfhGameDescription;

class EfhEngine : public Engine {
public:
	EfhEngine(OSystem *syst, const EfhGameDescription *gd);
	~EfhEngine() override;

	OSystem *_system;
	Graphics::Surface *_mainSurface;
	Common::RandomSource *_rnd;


	const EfhGameDescription *_gameDescription;
	uint32 getFeatures() const;
	const char *getGameId() const;

	void initGame(const EfhGameDescription *gd);
	GameType getGameType() const;
	Common::Platform getPlatform() const;

	bool hasFeature(EngineFeature f) const override;
	const char *getCopyrightString() const;

	Common::String getSavegameFilename(int slot);
	void syncSoundSettings() override;

	bool _shouldQuit;

protected:
	Common::EventManager *_eventMan;
	int _lastTime;

	// Engine APIs
	Common::Error run() override;
	void handleMenu();

private:
	static EfhEngine *s_Engine;

	GameType _gameType;
	Common::Platform _platform;

	void initialize();
};

} // End of namespace Efh

#endif
