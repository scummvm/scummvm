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
#include "graphics/cursorman.h"

namespace Hypno {

void WetEngine::initSegment(ArcadeShooting *arc) {
	if (_arcadeMode == "Y1") {
		_segmentShootSequenceOffset = 0;
		_segmentShootSequenceMax = 3;
	} else if (_arcadeMode == "Y3") {
		_segmentShootSequenceOffset = 0;
		_segmentShootSequenceMax = 7;
	} else if (_arcadeMode == "Y5") {
		_segmentShootSequenceOffset = 1;
		_segmentShootSequenceMax = 9;
	} else if (_arcadeMode == "Y4") {
		_c40SegmentNext.clear();
		_c40SegmentNext.push_back(2);
		_c40SegmentPath.push_back(5);

		_c40SegmentNext.push_back(6);
		_c40SegmentPath.push_back(8);

		_c40SegmentNext.push_back(15);
		_c40SegmentPath.push_back(16);

		_c40SegmentNext.push_back(26);
		_c40SegmentIdx = 0;
		_c40lastTurn = -1;

		_segmentShootSequenceOffset = 0;
		_segmentShootSequenceMax = 3;
	} else {
		_segmentShootSequenceOffset = 0;
		_segmentShootSequenceMax = 0;
	}

	uint32 randomSegmentShootSequence = _segmentShootSequenceOffset + _rnd->getRandomNumber(_segmentShootSequenceMax);
	debugC(1, kHypnoDebugArcade, "Select random sequence %d", randomSegmentShootSequence);
	SegmentShoots segmentShoots = arc->shootSequence[randomSegmentShootSequence];
	_shootSequence = segmentShoots.shootSequence;
	_segmentRepetitionMax = segmentShoots.segmentRepetition; // Usually zero
	_segmentRepetition = 0;
	_segmentOffset = 0;
	_segmentIdx = _segmentOffset;

	if (_arcadeMode == "Y3") {
		ShootInfo si;
		si.name = "SP_CBREAKER_L";
		si.timestamp = 30 * (_segmentRepetitionMax + 1) - 3;
		_shootSequence.push_back(si);
	}
}

void WetEngine::findNextSegment(ArcadeShooting *arc) {
	debugC(1, kHypnoDebugArcade, "Repetition %d of %d", _segmentRepetition, _segmentRepetitionMax);
	Common::Point mousePos = g_system->getEventManager()->getMousePos();
	Segments segments = arc->segments;

	if (_segmentRepetition < _segmentRepetitionMax) {
		_segmentRepetition = _segmentRepetition + 1;
	} else {
		_segmentRepetition = 0;
		_segmentRepetitionMax = 0;
		if (segments[_segmentIdx].type == 0xb3) {
			if (_arcadeMode == "Y1") {
				if (_rnd->getRandomBit())
					_segmentIdx = _segmentIdx + 1;
				else
					_segmentIdx = _segmentIdx + 5;
			} else if (_arcadeMode == "Y4") {
				_segmentIdx = _c40SegmentNext[_c40SegmentIdx];
			} else if (_arcadeMode == "Y5") {
				int r = _rnd->getRandomNumber(4);
				if (r == 0)
					_segmentIdx = 1;
				else
					_segmentIdx = r + 4;

				if (segments[_segmentIdx].type == 'L') {
					_shootSequence = arc->shootSequence[11].shootSequence;
					_segmentRepetitionMax = 0;
				} else if (segments[_segmentIdx].type == 'R') {
					_shootSequence = arc->shootSequence[12].shootSequence;
					_segmentRepetitionMax = 0;
				} else if (segments[_segmentIdx].type == 'A') {
					_shootSequence = arc->shootSequence[15].shootSequence;
					_segmentRepetitionMax = 0;
				} else if (segments[_segmentIdx].type == 'P') {
					r = _rnd->getRandomNumber(1);
					_shootSequence = arc->shootSequence[13 + r].shootSequence; //13-14
					_segmentRepetitionMax = 0;
				}
			} else
				_segmentIdx = _segmentIdx + 1;

		} else if (segments[_segmentIdx].type == 0xc5) {
			if (_arcadeMode == "Y1") {
				if (mousePos.x <= 100)
					_segmentIdx = _segmentIdx + 1;
				else if (mousePos.x >= 300)
					_segmentIdx = _segmentIdx + 3;
				else
					_segmentIdx = _segmentIdx + 2;
			} else if (_arcadeMode == "Y5" || _arcadeMode == "Y4") {
				if (mousePos.x <= 100)
					_segmentIdx = _segmentIdx + 2;
				else if (mousePos.x >= 300)
					_segmentIdx = _segmentIdx + 3;
				else
					_segmentIdx = _segmentIdx + 1;
			} else
				error("Invalid segment type for mode: %s at the end of segment %x", _arcadeMode.c_str(), segments[_segmentIdx].type);

		} else if (segments[_segmentIdx].type == 0xc2) {
			if (mousePos.x <= 160)
				_segmentIdx = _segmentIdx + 1;
			else
				_segmentIdx = _segmentIdx + 2;
		} else if (segments[_segmentIdx].type == 0xcc) {
			if (mousePos.x <= 160)
				_segmentIdx = _segmentIdx + 1;
			else
				_segmentIdx = _segmentIdx + 2;
		} else if (segments[_segmentIdx].type == 'Y') {
			if (mousePos.x <= 160)
				_segmentIdx = _segmentIdx + 2;
			else
				_segmentIdx = _segmentIdx + 1;

		} else if (segments[_segmentIdx].type == 'a') {
			_segmentIdx = 1;
		} else if (segments[_segmentIdx].type == 's') {
			_segmentIdx = 11;
		} else {

			// Objective checking
			if (arc->objKillsRequired[_objIdx] > 0) {
				if (_objKillsCount[_objIdx] >= arc->objKillsRequired[_objIdx] && _objMissesCount[_objIdx] <= arc->objMissesAllowed[_objIdx]) {
					if (_objIdx == 0) {
						_objIdx = 1;
						if (_arcadeMode == "Y1") {
							_segmentOffset = 8;
							_segmentRepetition = 0;
							_segmentShootSequenceOffset = 8;
						}
					} else {
						_skipLevel = true; // RENAME
						return;
					}
				}
			}
			if (segments[_segmentIdx].type == 0xc9) {
				if (_arcadeMode == "Y3") {
					_segmentOffset = _segmentIdx + 1;
					_segmentShootSequenceOffset = 8;
					_segmentShootSequenceMax = 7;
				} else if (_arcadeMode == "Y4") {
					_segmentOffset = 13;
					_segmentShootSequenceOffset = 1; // TODO
					_segmentShootSequenceMax = 1;    // TODO
				} else
					error("Invalid segment type for mode: %s at the end of segment %x", _arcadeMode.c_str(), segments[_segmentIdx].type);
			} else if (segments[_segmentIdx].type == 0xbb) {
				_segmentOffset = 0;
				_segmentShootSequenceOffset = 0;
				_segmentShootSequenceMax = 7;
			}

			if (_arcadeMode == "Y4") {
				if (_c40SegmentPath[_c40SegmentIdx] == _segmentIdx) {
					_c40SegmentIdx++;
				} else {
					if (_c40lastTurn == int(_segmentIdx))
						_health = 0;
					else {
						// TODO: this should also add a barrier near the end
						_c40lastTurn = int(_segmentIdx);
					}
				}
			}

			_segmentIdx = _segmentOffset;
			// select a new shoot sequence
			uint32 randomSegmentShootSequence = _segmentShootSequenceOffset + _rnd->getRandomNumber(_segmentShootSequenceMax);
			debugC(1, kHypnoDebugArcade, "Selected random sequence %d", randomSegmentShootSequence);
			SegmentShoots segmentShoots = arc->shootSequence[randomSegmentShootSequence];
			_shootSequence = segmentShoots.shootSequence;
			_segmentRepetitionMax = segmentShoots.segmentRepetition; // Usually one

			if (_arcadeMode == "Y3") {
				ShootInfo si;
				if (_segmentOffset == 0)
					si.name = "SP_CBREAKER_L";
				else
					si.name = "SP_CBREAKER_U";

				si.timestamp = 30 * (_segmentRepetitionMax + 1) - 3;
				_shootSequence.push_back(si);
			}
		}
	}
}

void WetEngine::runAfterArcade(ArcadeShooting *arc) {
	_checkpoint = _currentLevel;
	if (!isDemo() || _variant == "Demo") {
		byte *palette;
		Graphics::Surface *frame = decodeFrame("c_misc/zones.smk", 12, &palette);
		loadPalette(palette, 0, 256);
		uint32 c = 251; // green
		int bonusCounter = 0;
		int scoreCounter = _score - _bonus;
		assert(scoreCounter >= 0);
		bool skip = false;
		Common::Event event;
		while (!shouldQuit() && !skip) {

			drawImage(*frame, 0, 0, false);
			drawString("scifi08.fgx", Common::String::format("Lives : %d", _lives), 36, 2, 0, c);
			drawString("scifi08.fgx", Common::String::format("%-20s = %7d", "SHOTS FIRED", _shootsFired), 60, 46, 0, c);
			drawString("scifi08.fgx", Common::String::format("%-20s = %7d", "ENEMY TARGETS", _enemyTargets), 60, 56, 0, c);
			drawString("scifi08.fgx", Common::String::format("%-20s = %7d", "TARGETS DESTROYED", _targetsDestroyed), 60, 66, 0, c);
			drawString("scifi08.fgx", Common::String::format("%-20s = %7d", "TARGETS MISSED", _targetsMissed), 60, 76, 0, c);
			drawString("scifi08.fgx", Common::String::format("%-20s = %5d %%", "KILL RATIO", killRatio()), 60, 86, 0, c);
			drawString("scifi08.fgx", Common::String::format("%-20s = %5d %%", "ACCURACY", accuracyRatio()), 60, 96, 0, c);
			drawString("scifi08.fgx", Common::String::format("%-20s = %5d %%", "ENERGY", _health), 60, 106, 0, c);

			while (g_system->getEventManager()->pollEvent(event)) {
				// Events
				switch (event.type) {

				case Common::EVENT_QUIT:
				case Common::EVENT_RETURN_TO_LAUNCHER:
					break;

				case Common::EVENT_KEYDOWN:
					bonusCounter = _bonus;
					drawString("scifi08.fgx", Common::String::format("%-20s = %3d pts", "BONUS", _bonus), 60, 116, 0, c);
					drawString("scifi08.fgx", Common::String::format("%-20s = %3d pts", "SCORE", _score), 60, 126, 0, c);
					skip = true;
					break;
				default:
					break;
				}
			}

			if (bonusCounter < _bonus) {
				bonusCounter++;
				scoreCounter++;
				drawString("scifi08.fgx", Common::String::format("%-20s = %3d pts", "BONUS", bonusCounter), 60, 116, 0, c);
				drawString("scifi08.fgx", Common::String::format("%-20s = %3d pts", "SCORE", scoreCounter), 60, 126, 0, c);
			}

			drawScreen();
			g_system->delayMillis(25);
		}
		frame->free();
		delete frame;
	}

}


void WetEngine::runBeforeArcade(ArcadeShooting *arc) {
	resetStatistics();
	_checkpoint = _currentLevel;
	MVideo *video;
	if (!isDemo()) {

		saveGame(int(arc->id));
		byte *palette;
		Graphics::Surface *frame = decodeFrame("c_misc/zones.smk", (arc->id / 10 - 1) * 2, &palette);
		loadPalette(palette, 0, 256);
		byte p[3] = {0xff, 0x00, 0x00}; // Always red?
		loadPalette((byte *) &p, 240 - arc->id % 10, 1);
		drawImage(*frame, 0, 0, false);
		frame->free();
		delete frame;
		bool showedBriefing = false;
		bool endedBriefing = false;
		Common::Event event;

		uint32 c = 251; // green
		drawString("scifi08.fgx", Common::String::format("Lives : %d", _lives), 36, 2, 0, c);
		while (!shouldQuit() && !endedBriefing) {

			while (g_system->getEventManager()->pollEvent(event)) {
				// Events
				switch (event.type) {

				case Common::EVENT_QUIT:
				case Common::EVENT_RETURN_TO_LAUNCHER:
					break;

				case Common::EVENT_KEYDOWN:
					if (showedBriefing) {
						endedBriefing = true;
						break;
					}
					if (!arc->briefingVideo.empty()) {
						video = new MVideo(arc->briefingVideo, Common::Point(44, 22), false, false, false);
						runIntro(*video);
						delete video;
					}
					showedBriefing = true;
					break;

				default:
					break;
				}
			}

			drawScreen();
			g_system->delayMillis(10);
		}
	}

	if (!arc->beforeVideo.empty()) {
		video = new MVideo(arc->beforeVideo, Common::Point(0, 0), false, true, false);
		runIntro(*video);
		delete video;
	}

	if (!arc->player.empty()) {
		_playerFrames = decodeFrames(arc->player);
	}

	if (arc->mode == "Y4" || arc->mode == "Y5")  { // These images are flipped, for some reason
		for (Frames::iterator it = _playerFrames.begin(); it != _playerFrames.end(); ++it) {
			for (int i = 0 ; i < (*it)->w ; i++)
				for (int j = 0 ; j < (*it)->h/2 ; j++) {
					uint32 p1 = (*it)->getPixel(i, j);
					uint32 p2 = (*it)->getPixel(i, (*it)->h - j - 1);
					(*it)->setPixel(i, j, p2);
					(*it)->setPixel(i, (*it)->h - j - 1, p1);
				}
		}
	}

	_playerFrameSep = 0;

	for (Frames::iterator it =_playerFrames.begin(); it != _playerFrames.end(); ++it) {
		if ((*it)->getPixel(0, 0) == 255)
			break;
		if ((*it)->getPixel(0, 0) == 252)
			break;

		_playerFrameSep++;
	}

	if (_playerFrameSep == (int)_playerFrames.size()) {
		debugC(1, kHypnoDebugArcade, "No player separator frame found in %s! (size: %d)", arc->player.c_str(), _playerFrames.size());
		//_playerFrameSep = -1;
	} else
		debugC(1, kHypnoDebugArcade, "Separator frame found at %d", _playerFrameSep);

	_playerFrameIdx = -1;
}

bool WetEngine::clickedSecondaryShoot(const Common::Point &mousePos) {
	incShotsFired();
	return clickedPrimaryShoot(mousePos);
}

void WetEngine::missTarget(Shoot *s, ArcadeShooting *arc, MVideo &background) {
	if (s->name == "SP_SWITCH_R" || s->name == "SP_SWITCH_L") {
		_health = 0;
	} else if (s->name == "SP_LIZARD1") {
		_health = _health - 15;
		background.decoder->pauseVideo(true);
		MVideo video(arc->additionalVideo, Common::Point(0, 0), false, true, false);
		runIntro(video);
		loadPalette(arc->backgroundPalette);
		background.decoder->pauseVideo(false);
		updateScreen(background);
		drawScreen();
	}
}

void WetEngine::hitPlayer() {
	if (_arcadeMode != "Y1" && _arcadeMode != "Y3" && _arcadeMode != "Y4" && _arcadeMode != "Y5") {
		assert( _playerFrameSep < (int)_playerFrames.size());
		if (_playerFrameIdx < _playerFrameSep)
			_playerFrameIdx = _playerFrameSep;
	}
}

void WetEngine::drawShoot(const Common::Point &mousePos) {
	uint32 c = 253;
	_compositeSurface->drawLine(0, _screenH, mousePos.x, mousePos.y, c);
	_compositeSurface->drawLine(0, _screenH, mousePos.x - 1, mousePos.y, c);
	_compositeSurface->drawLine(0, _screenH, mousePos.x - 2, mousePos.y, c);

	_compositeSurface->drawLine(_screenW, _screenH, mousePos.x, mousePos.y, c);
	_compositeSurface->drawLine(_screenW, _screenH, mousePos.x - 1, mousePos.y, c);
	_compositeSurface->drawLine(_screenW, _screenH, mousePos.x - 2, mousePos.y, c);
	playSound(_soundPath + _shootSound, 1);
}

void WetEngine::drawPlayer() {
	// TARGET ACQUIRED frame
	uint32 c = 251; // green
	_compositeSurface->drawLine(113, 1, 119, 1, c);
	_compositeSurface->drawLine(200, 1, 206, 1, c);

	_compositeSurface->drawLine(113, 1, 113, 9, c);
	_compositeSurface->drawLine(206, 1, 206, 9, c);

	_compositeSurface->drawLine(113, 9, 119, 9, c);
	_compositeSurface->drawLine(200, 9, 206, 9, c);

	c = 250; // red ?
	Common::Point mousePos = g_system->getEventManager()->getMousePos();
	int i = detectTarget(mousePos);
	if (i > 0)
		drawString("block05.fgx", "TARGET  ACQUIRED", 116, 3, 80, c);

	if (_arcadeMode == "Y1" || _arcadeMode == "Y3")
		return;

	if (_playerFrameIdx < _playerFrameSep) {
		_playerFrameIdx++;
		_playerFrameIdx = _playerFrameIdx % _playerFrameSep;
	} else {
		_playerFrameIdx++;
		if (_playerFrameIdx >= (int)_playerFrames.size())
			_playerFrameIdx = 0;
	}

	if (_arcadeMode == "Y5")
		_playerFrameIdx = 1;
	else if (_arcadeMode == "Y4")
		_playerFrameIdx = 2;

	drawImage(*_playerFrames[_playerFrameIdx], 0, 200 - _playerFrames[_playerFrameIdx]->h + 1, true);
}

void WetEngine::drawHealth() {
	uint32 c = 253;
	int p = (100 * _health) / _maxHealth;
	int s = _score;
	int mo = _objKillsCount[_objIdx];
	int mm = _objKillsRequired[_objIdx];
	if (_playerFrameIdx < _playerFrameSep) {
		const chapterEntry *entry = _chapterTable[_levelId];
		//uint32 id = _levelId;
		drawString("block05.fgx", Common::String::format("ENERGY  %d%%", p), entry->energyPos[0], entry->energyPos[1], 65, c);
		drawString("block05.fgx", Common::String::format("SCORE  %04d", s), entry->scorePos[0], entry->scorePos[1], 72, c);
		// Objectives are always in the zero in the demo
		if (entry->objectivesPos[0] > 0 && entry->objectivesPos[1] > 0)
			drawString("block05.fgx", Common::String::format("M.O.  %d/%d", mo, mm), entry->objectivesPos[0], entry->objectivesPos[1], 60, c);
	}
}

} // End of namespace Hypno
