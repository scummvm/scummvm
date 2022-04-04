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

#include "hypno/grammar.h"
#include "hypno/hypno.h"

#include "common/events.h"

namespace Hypno {

BoyzEngine::BoyzEngine(OSystem *syst, const ADGameDescription *gd) : HypnoEngine(syst, gd) {
	_screenW = 320;
	_screenH = 200;
	_lives = 2;
}

void BoyzEngine::loadAssets() {
	LibFile *missions = loadLib("", "preload/missions.lib", true);
	Common::ArchiveMemberList files;
	if (missions->listMembers(files) == 0)
		error("Failed to load any files from missions.lib");
	loadArcadeLevel("c11.mi_", "", "", "");
	loadLib("sound/", "misc/sound.lib", true);
	_nextLevel = "c11.mi_";
}

void BoyzEngine::runBeforeArcade(ArcadeShooting *arc) {
	_checkpoint = _currentLevel;
	assert(!arc->player.empty());
	_playerFrames = decodeFrames(arc->player);
	_playerFrameSep = 0;

	for (Frames::iterator it =_playerFrames.begin(); it != _playerFrames.end(); ++it) {
		if ((*it)->getPixel(0, 0) == 255)
			break;
		if ((*it)->getPixel(0, 0) == 252)
			break;

		_playerFrameSep++;
	}
	_playerFrameIdx = -1;
}

void BoyzEngine::drawPlayer() {
	drawImage(*_playerFrames[0], 0, 0, true);
}
void BoyzEngine::drawHealth() {}
void BoyzEngine::hitPlayer() {}
void BoyzEngine::drawShoot(const Common::Point &target) {}

void BoyzEngine::initSegment(ArcadeShooting *arc) {
	_segmentShootSequenceOffset = 0;
	_segmentShootSequenceMax = 0;

	uint32 randomSegmentShootSequence = _segmentShootSequenceOffset + _rnd->getRandomNumber(_segmentShootSequenceMax);
	SegmentShoots segmentShoots = arc->shootSequence[randomSegmentShootSequence];
	_shootSequence = segmentShoots.shootSequence;
	_segmentRepetitionMax = segmentShoots.segmentRepetition; // Usually zero
	_segmentRepetition = 0;
	_segmentOffset = 0;
	_segmentIdx = _segmentOffset;
}

} // namespace Hypno
