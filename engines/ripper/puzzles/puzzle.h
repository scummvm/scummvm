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

#ifndef RIPPER_PUZZLES_PUZZLE_H
#define RIPPER_PUZZLES_PUZZLE_H

#include "ripper/scenes/scene.h"

namespace Ripper {

class Puzzle : public Scene {
public:
	explicit Puzzle(RipperEngine *engine);

protected:
	bool serviceEngineEvents();
	void stopAudioHandles(Audio::SoundHandle *handles, uint count);
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_PUZZLE_H
