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

#ifndef BAGEL_HODJNPODJ_H
#define BAGEL_HODJNPODJ_H

#include "bagel/bagel.h"
#include "bagel/hodjnpodj/libs/types.h"

namespace Bagel {
namespace HodjNPodj {

#define GAME_WIDTH		640
#define GAME_HEIGHT		480

struct Minigame {
	const char *_name;
	void (*_run)();
};

class HodjNPodjEngine : public BagelEngine, public CBagel {
private:
	static const Minigame MINIGAMES[];

	ErrorCode InitializeSoundSystem(uint16 nChannels = 1, uint32 nFreq = 11025, uint16 nBitsPerSample = 8);
	ErrorCode ShutDownSoundSystem();
	void playMinigame(const Common::String &name);

protected:
	// Engine APIs
	Common::Error run() override;

	ErrorCode initialize() override;
	ErrorCode shutdown() override;
	bool shouldQuit() const override {
		return BagelEngine::shouldQuit();
	}

public:
	GAMESTRUCT gGameInfo;

public:
	HodjNPodjEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~HodjNPodjEngine() override;
};

extern HodjNPodjEngine *g_engine;

} // namespace HodjNPodj
} // namespace Bagel

#endif
