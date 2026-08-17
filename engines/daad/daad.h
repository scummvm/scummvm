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

#ifndef DAAD_DAAD_H
#define DAAD_DAAD_H

#include "common/scummsys.h"
#include "engines/engine.h"

struct ADGameDescription;

namespace DAAD {

/**
 * DAAD engine
 *
 * Wraps the ADP (ADventure Player) interpreter, which plays databases
 * produced by DAAD (Diseñador de Aventuras AD) for all of its targets:
 * Spectrum, CPC, C64, MSX, PCW, Atari ST, Amiga and IBM PC.
 *
 * ADP owns its presentation completely: it renders text, graphics and the
 * input line into a single 8 bit paletted framebuffer, because the original
 * interpreters did the same and the games rely on the exact cell metrics,
 * character set and attribute behaviour of the machine they targeted. The
 * backend (daad_vid.cpp) therefore implements ADP's VID_* ABI on top of
 * OSystem directly, and this class only owns startup and teardown.
 */
class DAADEngine : public Engine {
public:
	DAADEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~DAADEngine() override;

	/**
	 * Execute the game
	 */
	Common::Error run() override;

	bool hasFeature(EngineFeature f) const override;

	/**
	 * Returns the game Id the game was detected as
	 */
	const char *getGameId() const;

private:
	const ADGameDescription *_gameDescription;
};

extern DAADEngine *g_daad;

} // End of namespace DAAD

#endif
