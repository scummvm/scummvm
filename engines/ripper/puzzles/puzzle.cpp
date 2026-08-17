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
 */

#include "ripper/puzzles/puzzle.h"

#include "ripper/input.h"
#include "ripper/ripper.h"

namespace Ripper {

Puzzle::Puzzle(RipperEngine *engine) : Scene(engine) {
}

bool Puzzle::serviceEngineEvents() {
	if (!_engine->getInput()->pollEvents())
		return true;
	_engine->quitGame();
	return false;
}

} // End of namespace Ripper
