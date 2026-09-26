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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_PUZZLES_ROLODEX_H
#define RIPPER_PUZZLES_ROLODEX_H

#include "audio/mixer.h"
#include "common/rect.h"

#include "ripper/media.h"
#include "ripper/puzzles/puzzle.h"

namespace Ripper {

class RipperEngine;

class RolodexPuzzle : public Puzzle, public MediaSequenceCallback {
public:
	explicit RolodexPuzzle(RipperEngine *engine);

	Result run(uint completionFlag) override;
	uint16 service(uint frame) override;

private:
	void serviceAudio(uint frame);
	uint16 serviceInput();
	void updateCursor(const Common::Point &point);
	void stopAudio(uint index);
	void stopAllAudio();
	void startAudio(uint index, uint volumePercent = 50, bool loop = false);

	Audio::SoundHandle _audioHandles[4];
	uint _activeSequence;
	bool _acceptInput;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_ROLODEX_H
