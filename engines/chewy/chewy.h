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

#ifndef CHEWY_CHEWY_H
#define CHEWY_CHEWY_H

// FIXME
#define AIL
#define ENGLISCH

#include "common/scummsys.h"
#include "common/error.h"
#include "common/memstream.h"
#include "common/random.h"
#include "engines/engine.h"
#include "graphics/screen.h"
#include "chewy/temp_file.h"

namespace Chewy {

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

#define SHOULD_QUIT g_engine->shouldQuit()

struct ChewyGameDescription;
class EventsManager;
class Sound;

class ChewyEngine : public Engine {
protected:
	// Engine APIs
	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;

	void initialize();
	void shutdown();

public:
	TempFileArchive _tempFiles;
	EventsManager *_events;
	Sound *_sound;
	Graphics::Screen *_screen;
public:
	ChewyEngine(OSystem *syst, const ChewyGameDescription *gameDesc);
	~ChewyEngine() override;

	int getGameType() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;

	const ChewyGameDescription *_gameDescription;
	Common::RandomSource _rnd;
};

extern ChewyEngine *g_engine;
extern Graphics::Screen *g_screen;

} // End of namespace Chewy

#endif
