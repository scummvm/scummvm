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
#include "gui/message.h"
#include "graphics/cursorman.h"

namespace Hypno {

void WetEngine::initSegment(ArcadeShooting *arc) {
	if (_arcadeMode == "Y1") {
		_segmentShootSequenceOffset = 0;
		_segmentShootSequenceMax = 3;
	} else if (_arcadeMode == "Y3") {
		_segmentShootSequenceOffset = 0;
		_segmentShootSequenceMax = 7;
	} else if (_arcadeMode == "Y4") {
		_c40SegmentNext.clear();

		_c40SegmentNext.push_back(2);  // Road fork (down)
		_c40SegmentPath.push_back(3);  // Go straight

		_c40SegmentNext.push_back(9);  // Tunnel entry
		_c40SegmentPath.push_back(9);  // Go straight
		_c40SegmentNext.push_back(1);  // Tunnel section
		_c40SegmentPath.push_back(11); // Tunnel exit

		_c40SegmentNext.push_back(2);  // Road fork
		_c40SegmentPath.push_back(5);  // Turn left

		_c40SegmentNext.push_back(6);  // Ramp
		_c40SegmentPath.push_back(8);  // Take ramp

		_c40SegmentNext.push_back(15);  // Road fork (up)
		_c40SegmentPath.push_back(18);  // Take right

		_c40SegmentNext.push_back(22);  // Tunnel entry
		_c40SegmentPath.push_back(22);  // Go straight
		_c40SegmentNext.push_back(14);  // Tunnel section
		_c40SegmentPath.push_back(24);  // Tunnel exit

		_c40SegmentNext.push_back(15);  // Road fork (up)
		_c40SegmentPath.push_back(17);  // Take left

		_c40SegmentNext.push_back(19);  // Ramp
		_c40SegmentPath.push_back(21);  // Take ramp

		_c40SegmentNext.push_back(2);  // Road fork (down)
		_c40SegmentPath.push_back(3);  // Go straight

		_c40SegmentNext.push_back(26);
		_c40SegmentIdx = 0;
		_c40lastTurn = -1;

		_segmentShootSequenceOffset = 0;
		_segmentShootSequenceMax = 5;
	} else if (_arcadeMode == "Y5") {
		_c50LeftTurns = 0;
		_c50RigthTurns = 0;

		_segmentShootSequenceOffset = 1;
		_segmentShootSequenceMax = 9;
	} else {
		_segmentShootSequenceOffset = 0;
		_segmentShootSequenceMax = 0;
	}

	uint32 randomSegmentShootSequence = _segmentShootSequenceOffset + _rnd->getRandomNumber(_segmentShootSequenceMax);
	debugC(1, kHypnoDebugArcade, "Select random sequence %d", randomSegmentShootSequence);
	SegmentShoots segmentShoots = arc->shootSequence[randomSegmentShootSequence];
	_segments = arc->segments;
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

	if (_segmentRepetition < _segmentRepetitionMax) {
		_segmentRepetition = _segmentRepetition + 1;
	} else {
		_segmentRepetition = 0;
		_segmentRepetitionMax = 0;
		if (_segments[_segmentIdx].type == 0xb3) {
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

				if (_segments[_segmentIdx].type == 'L') {
					_shootSequence = arc->shootSequence[11].shootSequence;
					_segmentRepetitionMax = 0;
				} else if (_segments[_segmentIdx].type == 'R') {
					_shootSequence = arc->shootSequence[12].shootSequence;
					_segmentRepetitionMax = 0;
				} else if (_segments[_segmentIdx].type == 'A') {
					_shootSequence = arc->shootSequence[15].shootSequence;
					_segmentRepetitionMax = 0;
				} else if (_segments[_segmentIdx].type == 'P') {
					r = _rnd->getRandomNumber(1);
					_shootSequence = arc->shootSequence[13 + r].shootSequence; //13-14
					_segmentRepetitionMax = 0;
				}
			} else
				_segmentIdx = _segmentIdx + 1;

		} else if (_segments[_segmentIdx].type == 0xc5) {
			if (_arcadeMode == "Y1") {
				if (mousePos.x <= 106)
					_segmentIdx = _segmentIdx + 1;
				else if (mousePos.x >= 213)
					_segmentIdx = _segmentIdx + 3;
				else
					_segmentIdx = _segmentIdx + 2;
			} else if (_arcadeMode == "Y4") {
				if (mousePos.x <= 106)
					_segmentIdx = _segmentIdx + 2;
				else if (mousePos.x >= 213)
					_segmentIdx = _segmentIdx + 3;
				else
					_segmentIdx = _segmentIdx + 1;

			} else if (_arcadeMode == "Y5") {
				if (mousePos.x <= 106) {
					_segmentIdx = _segmentIdx + 2;
					_c50LeftTurns++;
				} else if (mousePos.x >= 213) {
					_segmentIdx = _segmentIdx + 3;
					_c50RigthTurns++;
				} else
					_segmentIdx = _segmentIdx + 1;
			} else
				error("Invalid segment type for mode: %s at the end of segment %x", _arcadeMode.c_str(), _segments[_segmentIdx].type);

		} else if (_segments[_segmentIdx].type == 0xc2) {
			if (mousePos.x <= 160)
				_segmentIdx = _segmentIdx + 1;
			else
				_segmentIdx = _segmentIdx + 2;
		} else if (_segments[_segmentIdx].type == 0xcc) {
			if (mousePos.x <= 160)
				_segmentIdx = _segmentIdx + 1;
			else {
				_segmentIdx = _segmentIdx + 2;
				if (_arcadeMode == "Y3") {
					ShootInfo si;
					si.name = "SP_WALKER_U";
					si.timestamp = 25;
					_shootSequence.push_back(si);
				}
			}
		} else if (_segments[_segmentIdx].type == 'Y') {
			if (mousePos.x <= 160) {
				_segmentIdx = _segmentIdx + 2;
				if (_arcadeMode == "Y3") {
					ShootInfo si;
					si.name = "SP_WALKER_D";
					si.timestamp = 25;
					_shootSequence.push_back(si);
				}
			} else
				_segmentIdx = _segmentIdx + 1;

		/*} else if (_segments[_segmentIdx].type == 'a') {
			_segmentIdx = 1;*/
		} else if (_segments[_segmentIdx].type == 's') {
			_segmentIdx = _segmentIdx + 10; // _segmentIdx = 11;
		} else {

			// Objective checking
			if (availableObjectives() && checkArcadeObjectives()) {
				if (_objIdx == 0) {
					_objIdx = 1;
					if (_arcadeMode == "Y1") {
						_segmentOffset = 8;
						_segmentRepetition = 0;
						_segmentShootSequenceOffset = 8;

						MVideo video(arc->hitBoss1Video, Common::Point(0, 0), false, true, false);
						disableCursor();
						runIntro(video);
					} else if (_arcadeMode == "Y3")
						_skipLevel = true;
				} else {
					_loseLevel = true;
					return;
				}
			}
			if (_segments[_segmentIdx].type == 0xc9) {
				if (_arcadeMode == "Y3") {
					_segmentOffset = _segmentIdx + 1;
					_segmentShootSequenceOffset = 8;
					_segmentShootSequenceMax = 7;
				} else if (_arcadeMode == "Y4") {
					if (_segmentOffset == 0) {
						_segmentOffset = 13;
						_segmentShootSequenceOffset = 10; // TODO
						_segmentShootSequenceMax = 5;    // TODO
					} else {
						_segmentOffset = 0;
						_segmentShootSequenceOffset = 0;
						_segmentShootSequenceMax = 5;
					}
				} else
					error("Invalid segment type for mode: %s at the end of segment %x", _arcadeMode.c_str(), _segments[_segmentIdx].type);
			} else if (_segments[_segmentIdx].type == 0xbb) {
				_segmentOffset = 0;
				_segmentShootSequenceOffset = 0;
				_segmentShootSequenceMax = 7;
			} else if (_segments[_segmentIdx].type == 0x61) {
				_segmentOffset = _segmentOffset + 1; // _segmentOffset = 1;
				_segmentShootSequenceOffset = 6;
				_segmentShootSequenceMax = 4;
			} else if (_segments[_segmentIdx].type == 0x63) {
				_segmentOffset = _segmentOffset - 1; // _segmentOffset = 0;
				//_segmentShootSequenceOffset = 0; // TODO
				//_segmentShootSequenceMax = 7; // TODO
			}
			// TODO: refactor the following code to avoid using a variable here
			bool addBarrier = false;
			if (_arcadeMode == "Y4") {
				if (_c40SegmentPath[_c40SegmentIdx] == _segmentIdx) {
					_c40SegmentIdx++;
				} else {
					if (_c40lastTurn == int(_segmentIdx))
						_health = 0;
					else {
						addBarrier = true;
						_c40lastTurn = int(_segmentIdx);
					}
				}
			} else if (_arcadeMode == "Y5") {
				if (_c50LeftTurns >= 1 && _c50RigthTurns >= 3) {
					_segmentIdx = 9; // last segment
					return;
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
			} else if (_arcadeMode == "Y4" && addBarrier) {
				// Add a barrier near the end
				ShootInfo si;
				si.name = "SP_BLOCKADE";
				si.timestamp = (30 * _segmentRepetitionMax) - 20;
				_shootSequence.push_back(si);
			}
		}
	}
}

bool WetEngine::checkTransition(ArcadeTransitions &transitions, ArcadeShooting *arc) {
	ArcadeTransition at = *transitions.begin();
	int ttime = at.time;
	if (ttime == 0) { // This special case is only reachable in c33
		assert(_objIdx == 1);
		transitions.pop_front();
	} else if (_background->decoder->getCurFrame() > ttime) {

		if (at.video == "NONE") {
			//TODO
			return true;
		}

		if (_playerFrameSeps.size() == 1) {
			_playerFrameStart = _playerFrameEnd + 1;
			_playerFrameSep = *_playerFrameSeps.begin();
			_playerFrameSeps.pop_front();
			_playerFrameEnd = _playerFrames.size();
			_playerFrameIdx = _playerFrameStart;
			debugC(1, kHypnoDebugArcade, "New separator frames %d %d %d", _playerFrameStart, _playerFrameSep, _playerFrameEnd);
		} else if (_playerFrameSeps.size() >= 2) {
			_playerFrameStart = _playerFrameEnd + 1;
			_playerFrameSep = *_playerFrameSeps.begin();
			_playerFrameSeps.pop_front();
			_playerFrameEnd = *_playerFrameSeps.begin();
			_playerFrameSeps.pop_front();
			_playerFrameIdx = _playerFrameStart;
			debugC(1, kHypnoDebugArcade, "New separator frames %d %d %d", _playerFrameStart, _playerFrameSep, _playerFrameEnd);
		}

		if (_levelId == 33) {
			if (checkArcadeObjectives()) {
				_objIdx = 1;
			} else {
				// We do not play the transition, just skip the level
				_loseLevel = true;
				return true;
			}
		} else if (_levelId == 52) {
			// Ignore the first objective, this will be checked when the targets are missed
			// just go to the second one
			_objIdx = 1;
		} else if (_levelId == 61 && transitions.size() == 1) {
			// Check the first objective during the second transition
			if (checkArcadeObjectives()) {
				_objIdx = 1;
			} else {
				// We do not play the transition, just skip the level
				_loseLevel = true;
				return true;
			}
		}
		if (at.video.empty() && !at.palette.empty()) {
			_background->decoder->pauseVideo(true);
			_currentPalette = at.palette;
			loadPalette(_currentPalette);
			_background->decoder->pauseVideo(false);
			drawPlayer();
			updateScreen(*_background);
			drawScreen();
		} else if (!at.video.empty()) {
			_background->decoder->pauseVideo(true);
			debugC(1, kHypnoDebugArcade, "Playing transition %s", at.video.c_str());
			MVideo video(at.video, Common::Point(0, 0), false, true, false);
			disableCursor();
			runIntro(video);

			if (!at.palette.empty())
				_currentPalette = at.palette;

			loadPalette(_currentPalette);
			_background->decoder->pauseVideo(false);
			drawPlayer();
			updateScreen(*_background);
			drawScreen();
			drawCursorArcade(g_system->getEventManager()->getMousePos());
			if (!_music.empty())
				playSound(_music, 0, _musicRate); // restore music
		} else
			error ("Invalid transition at %d", ttime);

		transitions.pop_front();
		return true;
	}
	return false;
}


void WetEngine::runAfterArcade(ArcadeShooting *arc) {
	_checkpoint = _currentLevel;

	_playerFrameSeps.clear();
	for (Frames::iterator it =_playerFrames.begin(); it != _playerFrames.end(); ++it) {
		(*it)->free();
		delete (*it);
	}
	_playerFrames.clear();

	if (_health < 0)
		_health = 0;

	if (arc->mode == "YT") {
		for (Frames::iterator it = _c33PlayerCursor.begin(); it != _c33PlayerCursor.end(); ++it) {
			(*it)->free();
			delete (*it);
		}
	}

	if (isDemo() && _variant != "Demo" && _variant != "M&MCD" && _restoredContentEnabled) {
		showDemoScore();
	} else if (!isDemo() || _variant == "Demo" || _variant == "M&MCD" || _variant == "Gen4") {
		byte *palette;
		Graphics::Surface *frame = decodeFrame("c_misc/zones.smk", 12, &palette);
		loadPalette(palette, 0, 256);
		free(palette);
		uint32 c = kHypnoColorGreen; // green
		int bonusCounter = 0;
		int scoreCounter = _score - _bonus;
		bool extraLife = false;
		assert(scoreCounter >= 0);
		bool skip = false;
		Common::Event event;
		while (!shouldQuit() && !skip) {

			drawImage(*frame, 0, 0, false);
			drawString("scifi08.fgx", Common::String::format("Lives : %d", _lives), 36, 2, 0, c);
			drawString("scifi08.fgx", Common::String::format("%-20s = %7d", "SHOTS FIRED", _stats.shootsFired), 60, 46, 0, c);
			drawString("scifi08.fgx", Common::String::format("%-20s = %7d", "ENEMY TARGETS", _stats.enemyTargets), 60, 56, 0, c);
			drawString("scifi08.fgx", Common::String::format("%-20s = %7d", "TARGETS DESTROYED", _stats.targetsDestroyed), 60, 66, 0, c);
			drawString("scifi08.fgx", Common::String::format("%-20s = %7d", "TARGETS MISSED", _stats.targetsMissed), 60, 76, 0, c);
			drawString("scifi08.fgx", Common::String::format("%-20s = %5d %%", "KILL RATIO", killRatio()), 60, 86, 0, c);
			drawString("scifi08.fgx", Common::String::format("%-20s = %5d %%", "ACCURACY", accuracyRatio()), 60, 96, 0, c);
			drawString("scifi08.fgx", Common::String::format("%-20s = %5d %%", "ENERGY", _health), 60, 106, 0, c);

			while (g_system->getEventManager()->pollEvent(event)) {
				// Events
				switch (event.type) {

				case Common::EVENT_QUIT:
				case Common::EVENT_RETURN_TO_LAUNCHER:
					break;

				case Common::EVENT_LBUTTONDOWN:
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

			extraLife |= checkScoreMilestones(scoreCounter); // This increase the number of lives, if necessary
			if (extraLife) {
				drawString("scifi08.fgx", "EXTRA LIFE", 164, 140, 0, kHypnoColorRed);
			}

			drawScreen();
			g_system->delayMillis(25);
		}
		frame->free();
		delete frame;
	}

	if (!arc->postStatsVideo.empty()) {
		MVideo video(arc->postStatsVideo, Common::Point(0, 0), false, true, false);
		disableCursor();
		runIntro(video);
	}
}
void WetEngine::restoreScoreMilestones(int score) {
	if (score == 0) {
		_scoreMilestones.clear();
		_scoreMilestones.push_back(10000);
		_scoreMilestones.push_back(25000);
		_scoreMilestones.push_back(50000);
		_scoreMilestones.push_back(100000);
	} else {
		while (true) {
			if (_scoreMilestones.empty() || score < *_scoreMilestones.begin())
				break;
			_scoreMilestones.pop_front();
		}
	}
}

bool WetEngine::checkScoreMilestones(int score) {
	bool extraLife = false;
	while (true) {
		if (_scoreMilestones.empty() || score < *_scoreMilestones.begin())
			break;
		_scoreMilestones.pop_front();
		_lives = _lives + 1;
		extraLife = true;
	}
	return extraLife;
}

uint32 WetEngine::findPaletteIndexZones(uint32 id) {
	switch (id) {
	case 11:
		return 237;
	case 10:
		return 239;
	case 21:
		return 240;
	case 22:
		return 237;
	case 23:
		return 238;
	case 20:
		return 239;
	default:
	break;
	}

	switch (id / 10) {
	case 3:
		if (id == 30)
			return 241;
		else
			return 237 + id % 10;
	case 4:
		if (id == 40)
			return 241;
		else
			return 236 + id % 10;
	case 5:
		if (id == 50)
			return 240;
		else
			return 237 + id % 10;
	case 6:
		if (id == 60)
			return 238;
		else
			return 237;
	default:
	error("Invalid level id: %d", id);
	}
}

void WetEngine::runBeforeArcade(ArcadeShooting *arc) {
	_health = arc->health;
	_maxHealth = _health;
	resetStatistics();
	_checkpoint = _currentLevel;
	MVideo *video;
	if (!isDemo() || ((_variant == "Demo" || _variant == "M&MCD") && _restoredContentEnabled)) {

		saveProfile(_name, int(arc->id));
		byte *palette;
		Graphics::Surface *frame = decodeFrame("c_misc/zones.smk", (arc->id / 10 - 1) * 2, &palette);
		loadPalette(palette, 0, 256);
		free(palette);
		byte red[3] = {0xff, 0x00, 0x00};
		for (int i = 0; i < 5; i++)
			loadPalette((byte *) &red, 237 + i, 1);

		byte blue[3] = {0x00, 0x00, 0xff};
		for (uint32 id = 10 * (arc->id / 10) + 1; id < arc->id; id++)
			loadPalette((byte *) &blue, findPaletteIndexZones(id), 1);

		byte green[3] = {0x00, 0xff, 0x00};
		uint32 idx = findPaletteIndexZones(arc->id);
		loadPalette((byte *) &green, idx, 1);
		drawImage(*frame, 0, 0, false);
		frame->free();
		delete frame;
		bool showedBriefing = false;
		bool endedBriefing = false;
		Common::Event event;

		uint32 c = kHypnoColorGreen; // green
		drawString("scifi08.fgx", Common::String::format("Lives : %d", _lives), 36, 2, 0, c);
		while (!shouldQuit() && !endedBriefing) {

			while (g_system->getEventManager()->pollEvent(event)) {
				// Events
				switch (event.type) {

				case Common::EVENT_QUIT:
				case Common::EVENT_RETURN_TO_LAUNCHER:
					break;

				case Common::EVENT_LBUTTONDOWN:
				case Common::EVENT_KEYDOWN:
					if (showedBriefing) {
						endedBriefing = true;
						break;
					}
					if (!arc->briefingVideo.empty()) {
						Graphics::Surface *bframe = decodeFrame(arc->briefingVideo, 1, &palette);
						loadPalette(palette, 0, 256);
						free(palette);
						video = new MVideo(arc->briefingVideo, Common::Point(44, 22), false, false, false);
						runIntro(*video);
						delete video;
						bframe->free();
						delete bframe;
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

	if (arc->mode == "Y2" && !arc->additionalVideo.empty()) {
		video = new MVideo(arc->additionalVideo, Common::Point(0, 0), false, true, false);
		runIntro(*video);
		delete video;
	}

	if (!arc->player.empty()) {
		_playerFrames = decodeFrames(arc->player);
	}

	if (_variant == "EarlyDemo" && (arc->id == 31 || arc->id == 41)) {
		int cutX = 36;
		for (int i = 0; i < int(_playerFrames.size()); i++) {
			Graphics::Surface *frame = _playerFrames[i];
			Graphics::Surface *newFrame = new Graphics::Surface();
			newFrame->create(320, 200, frame->format);
			newFrame->fillRect(Common::Rect(0, 0, 320, 200), frame->format.ARGBToColor(0, 0, 0, 0));
			newFrame->copyRectToSurfaceWithKey(*frame, 0, 0, Common::Rect(0, 0, 320, cutX), 0);
			newFrame->copyRectToSurfaceWithKey(*frame, 0, 200 - (frame->h - cutX - 1), Common::Rect(0, cutX, 320, frame->h - 1), 0);

			frame->free();
			delete frame;
			_playerFrames[i] = newFrame;
		}

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


	for (int i = 0; i < int(_playerFrames.size()); i++) {
		if (_playerFrames[i]->getPixel(0, 0) == 255)
			_playerFrameSeps.push_back(i);
		else if (_playerFrames[i]->getPixel(0, 0) == 252)
			_playerFrameSeps.push_back(i);
	}

	if (_playerFrameSeps.size() == 0) {
		debugC(1, kHypnoDebugArcade, "No player separator frame found in %s! (size: %d)", arc->player.c_str(), _playerFrames.size());
	} else
		debugC(1, kHypnoDebugArcade, "Number of separator frames: %d", _playerFrameSeps.size());

	_playerFrameStart = 0;
	_playerFrameSep = _playerFrames.size();

	if (_playerFrameSeps.size() > 0) {
		_playerFrameSep = *_playerFrameSeps.begin();
		_playerFrameSeps.pop_front();
	}

	if (_playerFrameSeps.size() > 0) {
		_playerFrameEnd = *_playerFrameSeps.begin();
		_playerFrameSeps.pop_front();
	} else {
		_playerFrameEnd = _playerFrames.size();
	}

	_playerFrameIdx = -1;

	_ammo = 150;
	_maxAmmo = 150;
	_c33PlayerPosition = Common::Point(_screenW/2, _screenH/2);

	if (arc->mode == "YT") {
		_c33PlayerCursor = decodeFrames("c33/c33i2.smk");
		_c33UseMouse = true;
		_c33PlayerDirection.clear();
	}
	if (arc->mode == "Y3") {
		bool started = startCountdown(420); // 7 minutes
		if (!started)
			error("Failed to start countdown in level %d!", arc->id);
	} else
		_timerStarted = false;

}

void WetEngine::pressedKey(const int keycode) {
	if (keycode == Common::KEYCODE_c) {
		_background->decoder->pauseVideo(true);
		showCredits();
		loadPalette(_currentPalette);
		changeScreenMode("320x200");
		_background->decoder->pauseVideo(false);
		updateScreen(*_background);
		drawScreen();
		if (!_music.empty())
			playSound(_music, 0, _musicRate); // restore music
	} else if (keycode == Common::KEYCODE_s) { // Added for testing
		if (_cheatsEnabled) {
			_skipLevel = true;
		}
	} else if (keycode == Common::KEYCODE_k) { // Added for testing
		_health = 0;
	} else if (keycode == Common::KEYCODE_ESCAPE) {
		openMainMenuDialog();
	} else if (keycode == Common::KEYCODE_LEFT) {
		if (_arcadeMode == "YT" && _c33PlayerPosition.x > 0) {
			_c33UseMouse = false;
			if (_c33PlayerDirection.size() < 3)
				_c33PlayerDirection.push_back(kPlayerLeft);
		}
	} else if (keycode == Common::KEYCODE_DOWN) {
		if (_arcadeMode == "YT" && _c33PlayerPosition.y < 130) { // Viewport value minus 30
			_c33UseMouse = false;
			if (_c33PlayerDirection.size() < 3)
				_c33PlayerDirection.push_back(kPlayerBottom);
		}
	} else if (keycode == Common::KEYCODE_RIGHT) {
		if (_arcadeMode == "YT" && _c33PlayerPosition.x < _screenW) {
			_c33UseMouse = false;
			if (_c33PlayerDirection.size() < 3)
				_c33PlayerDirection.push_back(kPlayerRight);
		}
	} else if (keycode == Common::KEYCODE_UP) {
		if (_arcadeMode == "YT" && _c33PlayerPosition.y > 0) {
			_c33UseMouse = false;
			if (_c33PlayerDirection.size() < 3)
				_c33PlayerDirection.push_back(kPlayerTop);
		}
	}
}

Common::Point WetEngine::getPlayerPosition(bool needsUpdate) {
	Common::Point mousePos = g_system->getEventManager()->getMousePos();
	if (_arcadeMode == "YT") {
		if (needsUpdate) {
			if (!_c33UseMouse) {
				disableCursor();
				if (_c33PlayerDirection.size() == 0)
					drawImage(*_c33PlayerCursor[10], _c33PlayerPosition.x - 10, _c33PlayerPosition.y, true);
				else if (_c33PlayerDirection.front() == kPlayerRight) {
					_c33PlayerPosition.x = _c33PlayerPosition.x + 4;
					drawImage(*_c33PlayerCursor[4], _c33PlayerPosition.x - 10, _c33PlayerPosition.y, true);
				} else if (_c33PlayerDirection.front() == kPlayerLeft) {
					_c33PlayerPosition.x = _c33PlayerPosition.x - 4;
					drawImage(*_c33PlayerCursor[8], _c33PlayerPosition.x - 10, _c33PlayerPosition.y, true);
				} else if (_c33PlayerDirection.front() == kPlayerBottom) {
					_c33PlayerPosition.y = _c33PlayerPosition.y + 4;
					drawImage(*_c33PlayerCursor[12], _c33PlayerPosition.x - 10,  _c33PlayerPosition.y, true);
				} else if (_c33PlayerDirection.front() == kPlayerTop) {
					_c33PlayerPosition.y = _c33PlayerPosition.y - 4;
					drawImage(*_c33PlayerCursor[10], _c33PlayerPosition.x - 10, _c33PlayerPosition.y, true);
				} else
					error("Invalid condition in getPlayerPosition");

				if (_c33PlayerDirection.size() > 0)
					_c33PlayerDirection.pop_front();
			} else {
				Common::Point diff = mousePos - _c33PlayerPosition;
				if (abs(diff.x) > 1 || abs(diff.y) > 1)
					diff = diff / 10;

				if (abs(diff.x) >= 10)
					diff.x = (diff.x / abs(diff.x)) * 10;

				if (abs(diff.y) >= 10)
					diff.y = (diff.x / abs(diff.x)) * 10;

				_c33PlayerPosition = _c33PlayerPosition + diff;

				if (diff.x > 0 && abs(diff.x) > abs(diff.y))
					drawImage(*_c33PlayerCursor[4], _c33PlayerPosition.x - 10, _c33PlayerPosition.y, true);
				else if (diff.x < 0 && abs(diff.x) > abs(diff.y))
					drawImage(*_c33PlayerCursor[8], _c33PlayerPosition.x - 10, _c33PlayerPosition.y, true);
				else if (diff.y > 0)
					drawImage(*_c33PlayerCursor[12], _c33PlayerPosition.x - 10, _c33PlayerPosition.y, true);
				else
					drawImage(*_c33PlayerCursor[10], _c33PlayerPosition.x - 10, _c33PlayerPosition.y, true);
			}
		}
		uint32 c = _compositeSurface->getPixel(_c33PlayerPosition.x, _c33PlayerPosition.y);
		if (c >= 225 && c <= 231) {
			if (!_infiniteHealthCheat)
				_health = _health - 1;
			generateStaticEffect();
		}
		return _c33PlayerPosition;
	}
	return mousePos;
}

void WetEngine::drawCursorArcade(const Common::Point &mousePos) {
	int i = detectTarget(mousePos);
	if (_arcadeMode == "YT") {
		if (_c33UseMouse)
			changeCursor("arcade");
		else
			disableCursor();
		return;
	}

	if (i >= 0)
		changeCursor("target");
	else
		changeCursor("arcade");
}

bool WetEngine::clickedSecondaryShoot(const Common::Point &mousePos) {
	if (_ammo <= 0)
		return false;

	if (!_infiniteAmmoCheat)
		_ammo--;
	incShotsFired();
	return clickedPrimaryShoot(mousePos);
}

void WetEngine::missedTarget(Shoot *s, ArcadeShooting *arc) {
	if (s->name == "SP_SWITCH_R" || s->name == "SP_SWITCH_L") {
		_health = 0;
	} else if (s->name == "SP_LIZARD1") {
		if (!_infiniteHealthCheat)
			_health = _health - 15;
		_background->decoder->pauseVideo(true);
		MVideo video(arc->additionalVideo, Common::Point(0, 0), false, true, false);
		runIntro(video);
		loadPalette(arc->backgroundPalette);
		_background->decoder->pauseVideo(false);
		updateScreen(*_background);
		drawScreen();
	} else if (s->name.hasPrefix("DOOR")) {
		_health = 0;
		_background->decoder->pauseVideo(true);
		// In some levels, the hit boss video is used to store this ending
		MVideo video(arc->hitBoss1Video, Common::Point(0, 0), false, true, false);
		runIntro(video);
		loadPalette(_currentPalette);
		_background->decoder->pauseVideo(false);
		updateScreen(*_background);
		drawScreen();
		_skipDefeatVideo = true;
	} else if (s->attackFrames.empty()) {
		if (!_infiniteHealthCheat)
			_health = _health - s->attackWeight;
		hitPlayer();
	}
}

void WetEngine::missNoTarget(ArcadeShooting *arc) {
	for (int i = _shoots.size() - 1; i >= 0; --i) {
		Shoot *it = &_shoots[i];
		if ((it->name == "SP_BOSS" || it->name == "SP_BOSS1") && !arc->missBoss1Video.empty()) {
			_background->decoder->pauseVideo(true);
			MVideo video(arc->missBoss1Video, Common::Point(0, 0), false, true, false);
			disableCursor();
			runIntro(video);
			// Should be currentPalette?
			loadPalette(arc->backgroundPalette);
			_background->decoder->pauseVideo(false);
			updateScreen(*_background);
			drawScreen();
			if (!_music.empty())
				playSound(_music, 0, _musicRate); // restore music
			break;
		} else if (it->name == "SP_BOSS2" && !arc->missBoss2Video.empty()) {
			_background->decoder->pauseVideo(true);
			MVideo video(arc->missBoss2Video, Common::Point(0, 0), false, true, false);
			disableCursor();
			runIntro(video);
			// Should be currentPalette?
			loadPalette(arc->backgroundPalette);
			_background->decoder->pauseVideo(false);
			updateScreen(*_background);
			drawScreen();
			if (!_music.empty())
				playSound(_music, 0, _musicRate); // restore music
			break;
		}
	}
}

void WetEngine::generateStaticEffect() {
	// random static
	uint8 c = _compositeSurface->getPixel(150, 120); // some pixel in the middle
	if (c != 0 && c != 254) {
		for (int i = 0; i < _screenW; i++) {
			for (int j = 50; j < 60; j++) {
				c = _rnd->getRandomBit() ? 254 : 0;
				_compositeSurface->setPixel(i, j, c);
			}
		}

		for (int i = 0; i < _screenW; i++) {
			for (int j = 80; j < 90; j++) {
				c = _rnd->getRandomBit() ? 254 : 0;
				_compositeSurface->setPixel(i, j, c);
			}
		}

		for (int i = 0; i < _screenW; i++) {
			for (int j = 120; j < 150; j++) {
				c = _rnd->getRandomBit() ? 254 : 0;
				_compositeSurface->setPixel(i, j, c);
			}
		}
		drawScreen();
		if (!_additionalSound.empty()) {
			playSound(_soundPath + _additionalSound, 1, 11025);
		}
	}
}

void WetEngine::hitPlayer() {
	if (_arcadeMode != "YT" && _arcadeMode != "Y1" && _arcadeMode != "Y2" && _arcadeMode != "Y3" && _arcadeMode != "Y4" && _arcadeMode != "Y5") {
		assert( _playerFrameSep < (int)_playerFrames.size());
		if (_playerFrameIdx < _playerFrameSep)
			_playerFrameIdx = _playerFrameSep;
	}
	if (!_hitSound.empty()) {
		playSound(_soundPath + _hitSound, 1, 11025);
	}
}

Common::Point WetEngine::computeTargetPosition(const Common::Point &mousePos) {
	if (_arcadeMode == "YT") {
		return Common::Point(mousePos.x, mousePos.y - 20);
	}
	return mousePos;
}

void WetEngine::drawShoot(const Common::Point &mousePos) {
	uint32 c = kHypnoColorYellow;

	if (_arcadeMode == "YT") {
		_compositeSurface->drawLine(mousePos.x, mousePos.y - 20, mousePos.x + 5, mousePos.y, c);
		_compositeSurface->drawLine(mousePos.x, mousePos.y - 20, mousePos.x + 5, mousePos.y, c);

		_compositeSurface->drawLine(mousePos.x, mousePos.y - 20, mousePos.x - 5, mousePos.y, c);
		_compositeSurface->drawLine(mousePos.x, mousePos.y - 20, mousePos.x - 5, mousePos.y, c);
	} else if (_arcadeMode == "Y4") {
		_compositeSurface->drawLine(_screenW/2 - 50, _screenH, mousePos.x, mousePos.y, c);
		_compositeSurface->drawLine(_screenW/2 - 50, _screenH, mousePos.x - 1, mousePos.y, c);

		_compositeSurface->drawLine(_screenW/2 + 50, _screenH, mousePos.x, mousePos.y, c);
		_compositeSurface->drawLine(_screenW/2 + 50, _screenH, mousePos.x - 1, mousePos.y, c);
	} else {
		_compositeSurface->drawLine(0, _screenH, mousePos.x, mousePos.y, c);
		_compositeSurface->drawLine(0, _screenH, mousePos.x - 1, mousePos.y, c);
		_compositeSurface->drawLine(0, _screenH, mousePos.x - 2, mousePos.y, c);

		_compositeSurface->drawLine(_screenW, _screenH, mousePos.x, mousePos.y, c);
		_compositeSurface->drawLine(_screenW, _screenH, mousePos.x - 1, mousePos.y, c);
		_compositeSurface->drawLine(_screenW, _screenH, mousePos.x - 2, mousePos.y, c);
	}

	playSound(_soundPath + _shootSound, 1);

	if (_arcadeMode == "Y4") {
		if (mousePos.x <= 25)
			_playerFrameIdx = 10;
		else if (mousePos.x <= 50)
			_playerFrameIdx = 12;
		else if (mousePos.x >= 295)
			_playerFrameIdx = 18;
		else if (mousePos.x >= 270)
			_playerFrameIdx = 16;
		else
			_playerFrameIdx = 14;

		drawImage(*_playerFrames[_playerFrameIdx], 0, 200 - _playerFrames[_playerFrameIdx]->h, true);
	}
}

void WetEngine::drawPlayer() {
	uint8 segmentType = _segments[_segmentIdx].type;
	if (segmentType == 0xc5 || segmentType == 0xc2 || segmentType == 0xcc)
		if (_background->decoder->getCurFrame() % 3 > 0) // Avoid flashing too much
			drawString("block05.fgx", _directionString, 113, 13, 80, kHypnoColorCyan);

	// TARGET ACQUIRED frame
	uint32 c = kHypnoColorGreen; // green
	_compositeSurface->drawLine(113, 1, 119, 1, c);
	_compositeSurface->drawLine(200, 1, 206, 1, c);

	_compositeSurface->drawLine(113, 1, 113, 9, c);
	_compositeSurface->drawLine(206, 1, 206, 9, c);

	_compositeSurface->drawLine(113, 9, 119, 9, c);
	_compositeSurface->drawLine(200, 9, 206, 9, c);

	if (_timerStarted) {
		assert(_arcadeMode == "Y3");
		c = kHypnoColorYellow;
		uint32 minutes = _countdown / 60;
		uint32 seconds = _countdown % 60;
		drawString("block05.fgx", Common::String::format("CLOCK %02d:%02d", minutes, seconds), 19, 11, 0, c);

		const chapterEntry *entry = _chapterTable[_levelId];
		Common::Point ep(entry->energyPos[0], entry->energyPos[1]);

		Common::Rect r = Common::Rect(ep.x - 2, ep.y + 6, ep.x + 69, ep.y + 15);
		_compositeSurface->frameRect(r, kHypnoColorGreen);

	}

	c = kHypnoColorRed; // red ?
	Common::Point mousePos = g_system->getEventManager()->getMousePos();
	int i = detectTarget(mousePos);
	if (i >= 0)
		drawString("block05.fgx", _targetString.c_str(), 116, 3, 80, c);

	if (_arcadeMode == "Y1" || _arcadeMode == "Y3")
		return;

	_playerFrameIdx++;
	if (_playerFrameIdx == _playerFrameSep || _playerFrameIdx >= _playerFrameEnd) {
		_playerFrameIdx = _playerFrameStart;
	}

	if (_arcadeMode == "Y5") {
		int x = mousePos.x / (_screenW / 3);
		int y = mousePos.y / (_screenH / 3);
		_playerFrameIdx = 2 * (x + 3*y) + 1;
	} else if (_arcadeMode == "Y4") {
		if (mousePos.x <= 25)
			_playerFrameIdx = 0;
		else if (mousePos.x <= 50)
			_playerFrameIdx = 2;
		else if (mousePos.x >= 295)
			_playerFrameIdx = 8;
		else if (mousePos.x >= 270)
			_playerFrameIdx = 6;
		else
			_playerFrameIdx = 4;
	}


	int offset = 0;
	// Ugly, but seems to be necessary
	if (_levelId == 31 && _variant != "EarlyDemo")
		offset = 2;
	else if (_levelId == 52)
		offset = 2;

	drawImage(*_playerFrames[_playerFrameIdx], 0, 200 - _playerFrames[_playerFrameIdx]->h + offset, true);
}

void WetEngine::drawHealth() {
	uint32 c = kHypnoColorYellow;
	int p = (100 * _health) / _maxHealth;
	int s = _score;
	int mo = _objKillsCount[_objIdx];
	int mm = _objKillsRequired[_objIdx];
	if (_playerFrameIdx < _playerFrameSep) {
		const chapterEntry *entry = _chapterTable[_levelId];
		Common::Point ep(entry->energyPos[0], entry->energyPos[1]);
		Common::Point sp(entry->scorePos[0], entry->scorePos[1]);
		Common::Point op(entry->objectivesPos[0], entry->objectivesPos[1]);
		Common::String healthFormat = _healthString + " %d%%";
		drawString("block05.fgx", Common::String::format(healthFormat.c_str(), p), ep.x, ep.y, 65, c);
		Common::String scoreFormat = _scoreString + "  %04d";
		Common::String moFormat = _objString + " %d/%d";

		if (_arcadeMode == "Y1" || _arcadeMode == "Y3" || _arcadeMode == "Y4" || _arcadeMode == "Y5") {
			Common::Rect r;
			r = Common::Rect(ep.x - 2, ep.y - 2, ep.x + 69, sp.y + 7);
			_compositeSurface->frameRect(r, kHypnoColorGreen);

			r = Common::Rect(sp.x - 2, sp.y - 2, sp.x + 74, sp.y + 7);
			_compositeSurface->frameRect(r, kHypnoColorGreen);

			r = Common::Rect(op.x - 2, op.y - 2, op.x + 74, op.y + 7);
			_compositeSurface->frameRect(r, kHypnoColorGreen);

			scoreFormat = _scoreString + " %04d";
			moFormat = _objString + "   %d/%d";
		}

		drawString("block05.fgx", Common::String::format(scoreFormat.c_str(), s), sp.x, sp.y, 72, c);
		if (op.x > 0 && op.y > 0)
			drawString("block05.fgx", Common::String::format(moFormat.c_str(), mo, mm), op.x, op.y, 60, c);
	}
}

void WetEngine::drawAmmo() {
	const chapterEntry *entry = _chapterTable[_levelId];
	if (entry->ammoPos[0] == 0 && entry->ammoPos[1] == 0)
		return;

	if (_playerFrameIdx > _playerFrameSep) // Player was hit
		return;

	int d = (13 * (_maxAmmo - _ammo) / _maxAmmo);
	if (d >= 13)
		return;
	Common::Point p(entry->ammoPos[0], entry->ammoPos[1]);
	int ao = entry->ammoOffset;
	Common::Rect r;
	if (_arcadeMode == "Y1" || _arcadeMode == "Y3" || _arcadeMode == "Y4" || _arcadeMode == "Y5") {
		r = Common::Rect(p.x - 1, p.y - 1, p.x + 16, p.y + 14);
		_compositeSurface->frameRect(r, kHypnoColorGreen);
	}

	if (_levelId / 10 == 5 && _arcadeMode != "Y5") {
		r = Common::Rect(p.x, p.y + d, p.x + 13, p.y + 13);
		_compositeSurface->fillRect(r, kHypnoColorWhiteOrBlue); // blue

		if (ao > 0) {
			r = Common::Rect(p.x + ao, p.y + d, p.x + 13 + ao, p.y + 13);
			_compositeSurface->fillRect(r, kHypnoColorWhiteOrBlue); // blue
		}
	} else {
		r = Common::Rect(p.x, p.y + d, p.x + 15, p.y + 13);
		_compositeSurface->fillRect(r, kHypnoColorWhiteOrBlue); // blue

		if (ao > 0) {
			r = Common::Rect(p.x + ao, p.y + d, p.x + 15 + ao, p.y + 13);
			_compositeSurface->fillRect(r, kHypnoColorWhiteOrBlue); // blue
		}
	}
}


byte *WetEngine::getTargetColor(Common::String name, int levelId) {
	if (name == "BOSS1" ||  name == "BOSS2" || name == "BOSS3" || name == "BOSS4")
		return getPalette(kHypnoColorGreen);

	const chapterEntry *entry = _chapterTable[levelId];
	if (entry->targetColor < 0)
		error ("No target color specified for level %d", levelId);
	return getPalette(entry->targetColor);
}

} // End of namespace Hypno
