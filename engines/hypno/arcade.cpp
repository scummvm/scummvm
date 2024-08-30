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

#include "common/tokenizer.h"
#include "common/events.h"
#include "graphics/cursorman.h"
#include "graphics/framelimiter.h"

#include "hypno/grammar.h"
#include "hypno/hypno.h"

namespace Hypno {

extern int parse_arc(const char *);

void HypnoEngine::splitArcadeFile(const Common::String &filename, Common::String &arc, Common::String &list) {
	debugC(1, kHypnoDebugParser, "Splitting %s", filename.c_str());
	Common::File file;
	if (!file.open(filename.c_str()))
		error("Failed to open %s", filename.c_str());

	while (!file.eos()) {
		byte x = file.readByte();
		byte p = arc.lastChar();
		arc += x;
		if (x == 'X' && p == '\n') {
			while (!file.eos()) {
				x = file.readByte();
				if (x == 'Y' && list.size() > 0 && list[list.size()-1] == '\n')
					break;
				list += x;
			}
			break; // No need to keep parsing
		}
	}
	file.close();
}

void HypnoEngine::parseArcadeShooting(const Common::String &prefix, const Common::String &filename, const Common::String &data) {
	debugC(1, kHypnoDebugParser, "Parsing %s/%s", prefix.c_str(), filename.c_str());
	parse_arc(data.c_str());
	ArcadeShooting *arcade = new ArcadeShooting();
	*arcade = *g_parsedArc;
	_levels[filename] = (Level*) arcade;
	g_parsedArc->clear();
}

SegmentShootsSequence HypnoEngine::parseShootList(const Common::String &filename, const Common::String &data) {
	debugC(1, kHypnoDebugParser, "Parsing %s", filename.c_str());
	debugC(1, kHypnoDebugParser, "%s", data.c_str());
	// Preparsing
	Common::String pdata;
	Common::StringTokenizer lines(data, "\n");
	Common::String t;
	while (!lines.empty()) {
		t = lines.nextToken();
		if (t[0] == ';')
			continue;
		if (t.size() == 0)
			continue;
		pdata += "\n" + t;
	}
	Common::String n;
	ShootInfo si;
	si.timestamp = 0;
	si.name = "";
	SegmentShootsSequence seq;

	// Patch to fix an issue in the parsing of the c3 level in Spiderman
	if (filename == "c3.mi_" || filename == "c3h.mi_")
		Common::replace(pdata, "92.B", "92,B");

	// Parsing
	pdata.trim();
	pdata = "\n" + pdata;

	if (pdata[1] == 'L') { // List of elements
		SegmentShoots ss;
		ss.segmentRepetition = 0;
		Common::StringTokenizer tok(pdata, " ,.\n\t");
		while (!tok.empty()) {
		t = tok.nextToken();
			while (t == "L") {
				if (ss.segmentRepetition > 0)
					seq.push_back(ss);
				t = tok.nextToken();
				ss.segmentRepetition = atoi(t.c_str());
				ss.shootSequence.clear();
				t = tok.nextToken();
			}

			n = tok.nextToken();
			if (t == "Z") {
				seq.push_back(ss);
				break;
			}
			si.name = n;
			si.timestamp = atoi(t.c_str());
			if (si.timestamp == 0 && si.name != "0") // 0,0 is a special case
				error("Error at parsing '%s' with timestamp: %s", n.c_str(), t.c_str());
			ss.shootSequence.push_back(si);
			debugC(1, kHypnoDebugParser, "%d -> %s", si.timestamp, si.name.c_str());
		}
	} else if (pdata[1] == 'S' ) { // Single element
		SegmentShoots ss;
		Common::StringTokenizer tok(pdata, " ,\t\r");
		while (!tok.empty()) {
			t = tok.nextToken();
			if (t[0] == '\n')
				continue;
			n = tok.nextToken();
			if (t == "Z")
				break;

			Common::replace(n, "\nS", "");
			Common::replace(n, "\nZ\n", "");
			Common::replace(n, "\nZ", "");
			uint32 timestamp = atoi(t.c_str());
			if (timestamp < si.timestamp) {
				debugC(1, kHypnoDebugParser, "WARNING: stopping the sequence earlier than expected");
				break;
			}

			si.name = n;
			si.timestamp = timestamp;
			if (si.timestamp == 0)
				error("Error at parsing '%s' with timestamp: %s", n.c_str(), t.c_str());
			ss.shootSequence.push_back(si);
			debugC(1, kHypnoDebugParser, "%d -> %s", si.timestamp, si.name.c_str());
		}
		seq.push_back(ss);
	} else
		error("Invalid shoot sequence to parse: %c", pdata[1]);

	return seq;
}

void HypnoEngine::loadArcadeLevel(const Common::String &arclevel, const Common::String &nextWin, const Common::String &nextLose, const Common::String &prefix) {
	debugC(1, kHypnoDebugParser, "Parsing %s", arclevel.c_str());
	Common::String arc;
	Common::String list;
	splitArcadeFile(arclevel, arc, list);
	debugC(1, kHypnoDebugParser, "%s", arc.c_str());
	parseArcadeShooting("", arclevel, arc);
	ArcadeShooting *arcade = (ArcadeShooting *) _levels[arclevel];
	arcade->shootSequence = parseShootList(arclevel, list);
	arcade->prefix = prefix;
	arcade->levelIfWin = nextWin;
	arcade->levelIfLose = nextLose;
}

void HypnoEngine::drawPlayer() { error("Function \"%s\" not implemented", __FUNCTION__); }
void HypnoEngine::drawHealth() { error("Function \"%s\" not implemented", __FUNCTION__); }
void HypnoEngine::drawAmmo() {}
void HypnoEngine::drawShoot(const Common::Point &target) { error("Function \"%s\" not implemented", __FUNCTION__); }
void HypnoEngine::hitPlayer() { error("Function \"%s\" not implemented", __FUNCTION__); }
void HypnoEngine::missedTarget(Shoot *s, ArcadeShooting *arc) {}
void HypnoEngine::missNoTarget(ArcadeShooting *arc) {}

void HypnoEngine::runBeforeArcade(ArcadeShooting *arc) {}
void HypnoEngine::runAfterArcade(ArcadeShooting *arc) {}

void HypnoEngine::pressedKey(const int keycode) {}

void HypnoEngine::initSegment(ArcadeShooting *arc) { error("Function \"%s\" not implemented", __FUNCTION__); }
void HypnoEngine::findNextSegment(ArcadeShooting *arc) { error("Function \"%s\" not implemented", __FUNCTION__); }

byte *HypnoEngine::getTargetColor(Common::String name, int levelId) { error("Function \"%s\" not implemented", __FUNCTION__); }

bool HypnoEngine::availableObjectives() {
	return (_objKillsRequired[_objIdx] > 0);
}

bool HypnoEngine::checkArcadeObjectives() {
	debugC(1, kHypnoDebugArcade, "Checking objective %d (%d/%d)", _objIdx, _objKillsCount[_objIdx], _objKillsRequired[_objIdx]);
	if (_objKillsRequired[_objIdx] > 0)
		return (_objKillsCount[_objIdx] >= _objKillsRequired[_objIdx] && \
		        _objMissesCount[_objIdx] <= _objMissesAllowed[_objIdx]);
	return true;
}

bool HypnoEngine::checkTransition(ArcadeTransitions &transitions, ArcadeShooting *arc) {
	error("Function \"%s\" not implemented", __FUNCTION__);
}

void HypnoEngine::runArcade(ArcadeShooting *arc) {
	_arcadeMode = arc->mode;
	Common::Point mousePos;
	Common::List<uint32> shootsToRemove;


	// segment/shoots
	Segments segments = arc->segments;
	initSegment(arc);

	// Transitions
	_transitions = arc->transitions;

	_levelId = arc->id;
	_shootSound = arc->shootSound;
	_hitSound = arc->hitSound;
	_additionalSound = arc->additionalSound;
	debugC(1, kHypnoDebugArcade, "Starting segment of type %x of size %d", segments[_segmentIdx].type, segments[_segmentIdx].size);
	_shoots.clear();
	_skipLevel = false;
	_loseLevel = false;
	_skipDefeatVideo = false;
	_skipNextVideo = false;
	_mask = nullptr;
	_masks = nullptr;

	if (arc->mouseBox == Common::Rect(0, 0, 0, 0))
		error("Invalid or missing mouse box");

	Common::Point offset;
	Common::Point anchor = arc->anchor;
	anchor.x = 0;	// This is almost always zero, except when the screen starts at the middle
					// We don't really need it
	anchor.y = MAX(0, anchor.y - arc->mouseBox.bottom);

	// Correct mouseBox
	arc->mouseBox.moveTo(anchor.x, anchor.y);
	_background = new MVideo(arc->backgroundVideo, anchor, false, false, false);

	drawCursorArcade(mousePos);
	playVideo(*_background);

	if (!arc->maskVideo.empty()) {
		_masks = new MVideo(arc->maskVideo, offset, false, false, false);
		playVideo(*_masks);
		_mask = _masks->decoder->decodeNextFrame();
	}

	float rate = _background->decoder->getFrameRate().toDouble();
	if (rate < 10) {
		debugC(1, kHypnoDebugArcade, "Used frame rate looks odd: %f, increasing x 10", rate);
		_background->decoder->setRate(10.0);
	}
	_currentPalette = arc->backgroundPalette;
	loadPalette(_currentPalette);
	int firstFrame = segments[_segmentIdx].start;
	if (firstFrame > 1) {
		_background->decoder->forceSeekToFrame(firstFrame);
		_masks->decoder->forceSeekToFrame(firstFrame);
		segments[_segmentIdx].start = 1;
	}

	bool shootingPrimary = false;
	bool shootingSecondary = false;
	bool needsUpdate = true;
	bool transition = false;

	_objIdx = 0;
	_objKillsCount[0] = 0;
	_objKillsCount[1] = 0;
	_objMissesCount[0] = 0;
	_objMissesCount[1] = 0;

	_objKillsRequired[0] = arc->objKillsRequired[0];
	_objKillsRequired[1] = arc->objKillsRequired[1];
	_objMissesAllowed[0] = arc->objMissesAllowed[0];
	_objMissesAllowed[1] = arc->objMissesAllowed[1];

	bool vsync = g_system->getFeatureState(OSystem::kFeatureVSync);
	// Disable vsync for arcade sequences, since these require a fixed frame rate
	g_system->beginGFXTransaction();
	g_system->setFeatureState(OSystem::kFeatureVSync, false);
	g_system->endGFXTransaction();

	debugC(1, kHypnoDebugArcade, "Using frame delay: %d", arc->frameDelay);
	Graphics::FrameLimiter limiter(g_system, 1000.0 / arc->frameDelay);
	limiter.startFrame();

	Common::Event event;
	while (!shouldQuit()) {
		if (_timerStarted) {
			if (_countdown <= 0) {
				_loseLevel = true;
				debugC(1, kHypnoDebugArcade, "Finishing level (timeout)");
				_timerStarted = false;
				removeTimers();
			}
		}
		needsUpdate = _background->decoder->needsUpdate();
		while (g_system->getEventManager()->pollEvent(event)) {
			mousePos = getPlayerPosition(false);

			// Events
			switch (event.type) {

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_KEYDOWN:
				pressedKey(event.kbd.keycode);
				if (event.kbd.keycode == Common::KEYCODE_LCTRL)
					if (clickedPrimaryShoot(mousePos))
						shootingPrimary = true;
				break;

			case Common::EVENT_LBUTTONDOWN:
				if (clickedPrimaryShoot(mousePos))
					shootingPrimary = true;
				break;

			case Common::EVENT_RBUTTONDOWN:
				if (clickedSecondaryShoot(mousePos))
					shootingSecondary = true;
				break;

			case Common::EVENT_RBUTTONUP:
				shootingSecondary = false;
				break;

			case Common::EVENT_MOUSEMOVE:
				drawCursorArcade(mousePos);
				if (mousePos.x >= arc->mouseBox.right-1) {
					g_system->warpMouse(arc->mouseBox.right-1, mousePos.y);
				} else if (mousePos.y < arc->mouseBox.top) { // Usually top is zero
					g_system->warpMouse(mousePos.x, arc->mouseBox.top + 1);
				} else if (mousePos.y >= arc->mouseBox.bottom-1) {
					g_system->warpMouse(mousePos.x, arc->mouseBox.bottom-1);
				} else if (mousePos.x <= 40 && offset.x < 0) {
					for (Shoots::iterator it = _shoots.begin(); it != _shoots.end(); ++it) {
						if (it->video && it->video->decoder)
							it->video->position.x = it->video->position.x + 1;
					}
					offset.x = offset.x + 1;
					needsUpdate = true;
				} else if (mousePos.x >= 280 && offset.x > 320 - _background->decoder->getWidth()) {
					for (Shoots::iterator it = _shoots.begin(); it != _shoots.end(); ++it) {
						if (it->video && it->video->decoder)
							it->video->position.x = it->video->position.x - 1;
					}
					offset.x = offset.x - 1;
					needsUpdate = true;
				}
				_background->position.x = offset.x;
				break;

			default:
				break;
			}
		}

		if (needsUpdate) {
			getPlayerPosition(true);
			if (_background->decoder->getCurFrame() > firstFrame)
				drawScreen();
			updateScreen(*_background);
			if (!arc->maskVideo.empty() && _masks->decoder->needsUpdate())
				_mask = _masks->decoder->decodeNextFrame();
			if (_additionalVideo && _additionalVideo->decoder->needsUpdate())
				_additionalVideo->decoder->decodeNextFrame(); // only audio?
		}

		if (_health <= 0) {
			skipVideo(*_background);
			if (_skipDefeatVideo)
				; // No video
			else if (!arc->defeatNoEnergySecondVideo.empty() && transition) {
				disableCursor();
				MVideo video(arc->defeatNoEnergySecondVideo, Common::Point(0, 0), false, true, false);
				runIntro(video);
			} else if (!arc->defeatNoEnergyFirstVideo.empty()) {
				disableCursor();
				MVideo video(arc->defeatNoEnergyFirstVideo, Common::Point(0, 0), false, true, false);
				runIntro(video);
			}
			assert(!arc->levelIfLose.empty());
			_nextLevel = arc->levelIfLose;
			debugC(1, kHypnoDebugArcade, "Losing level and jumping to %s", _nextLevel.c_str());
			_lives = _lives - 1;
			break;
		}

		if (!_transitions.empty()) {
			transition = checkTransition(_transitions, arc);
		}

		if (_background->decoder && _background->decoder->getCurFrame() >= int(segments[_segmentIdx].start + segments[_segmentIdx].size - 2)) {
			debugC(1, kHypnoDebugArcade, "Finished segment %d of type %x", _segmentIdx, segments[_segmentIdx].type);

			// Clear shoots
			/*for (Shoots::iterator it = _shoots.begin(); it != _shoots.end(); ++it) {
				if (it->video && it->video->decoder)
					skipVideo(*it->video);
				delete it->video;
			}
			_shoots.clear();*/
			findNextSegment(arc);

			if (_segmentIdx >= segments.size())
				error("Invalid segment %d", _segmentIdx);

			debugC(1, kHypnoDebugArcade, "Starting segment %d of type %x at %d", _segmentIdx, segments[_segmentIdx].type, segments[_segmentIdx].start);
			if (!segments[_segmentIdx].end) { // If it is not the end segment
				_background->decoder->forceSeekToFrame(segments[_segmentIdx].start);
				continue;
			}
		}

		if (segments[_segmentIdx].end || _skipLevel || _loseLevel) {
			skipVideo(*_background);
			// Objectives
			if (!checkArcadeObjectives() && !_skipLevel) {
				if (!arc->defeatMissBossVideo.empty()) {
					MVideo video(arc->defeatMissBossVideo, Common::Point(0, 0), false, true, false);
					disableCursor();
					runIntro(video);
				}
				assert(!arc->levelIfLose.empty());
				_nextLevel = arc->levelIfLose;
				_lives = _lives - 1;
				_arcadeMode = "";
				debugC(1, kHypnoDebugArcade, "Losing level (objectives) and jumping to %s", _nextLevel.c_str());
				break;
			}

			if (!arc->nextLevelVideo.empty() && !_skipNextVideo) {
				MVideo video(arc->nextLevelVideo, Common::Point(0, 0), false, true, false);
				disableCursor();
				runIntro(video);
			}
			assert(!arc->levelIfWin.empty());
			_nextLevel = arc->levelIfWin;
			_checkpoint = _nextLevel;
			_arcadeMode = "";
			_skipLevel = false;
			debugC(1, kHypnoDebugArcade, "Wining level and jumping to %s", _nextLevel.c_str());
			break;
		}

		if (_shootSequence.size() > 0) {
			ShootInfo si = _shootSequence.front();
			int idx = (int)segments[_segmentIdx].size * _segmentRepetition \
					+ _background->decoder->getCurFrame() \
					- (int)segments[_segmentIdx].start + 3;
			//debug("%d %d", si.timestamp, idx);
			if ((int)si.timestamp <= idx) {
				_shootSequence.pop_front();
				incEnemyTargets();
				for (Shoots::iterator it = arc->shoots.begin(); it != arc->shoots.end(); ++it) {
					if (it->name == si.name) {
						Shoot s = *it;
						s.startFrame = si.timestamp;
						if (_masks) {
							s.startFrame = 0;
							_shoots.push_back(s);
						} else if (it->animation == "NONE") {
							byte *c = getTargetColor(it->name, _levelId);
							assert(s.paletteSize == 1 || s.paletteSize == 0);
							loadPalette(c, s.paletteOffset, s.paletteSize);
							_shoots.push_back(s);
						} else {
							s.video = new MVideo(it->animation, offset + it->position, true, false, false);
							playVideo(*s.video);
							s.video->decoder->decodeNextFrame(); // Make sure the palette is loaded
							if (s.attackFrames.size() == 0) {
								uint32 lastFrame = s.bodyFrames.back().lastFrame();
								s.attackFrames.push_back(lastFrame - 3);
							}
							s.lastFrame = s.bodyFrames[s.bodyFrames.size() - 1].lastFrame();
							loadPalette(s.video->decoder->getPalette() + 3*s.paletteOffset, s.paletteOffset, s.paletteSize);
							_shoots.push_back(s);
						}
						if (!s.noEnemySound) {
							if (!s.enemySound.empty())
								playSound(_soundPath + s.enemySound, 1, s.enemySoundRate);
							else if (!arc->enemySound.empty())
								playSound(_soundPath + arc->enemySound, 1, arc->enemySoundRate);
						}
					}
				}
			}
		}

		uint32 i = 0;
		shootsToRemove.clear();

		for (Shoots::iterator it = _shoots.begin(); it != _shoots.end(); ++it) {
			if (it->video && it->video->decoder) {
				int frame = it->video->decoder->getCurFrame();
				if (it->attackFrames.size() > 0) {
					uint32 attackFrame = it->attackFrames.front();
					if (frame > 0 && frame >= (int)(attackFrame - 2) && !it->destroyed) {
						if (!_infiniteHealthCheat)
							_health = _health - it->attackWeight;
						hitPlayer();
						it->attackFrames.pop_front();
					}
				}

				uint32 bodyLastFrame = it->bodyFrames[it->bodyFrames.size() - 1].lastFrame();
				if (frame > 0 && frame >= (int)(bodyLastFrame - 3) && !it->destroyed) {
					incTargetsMissed();
					missedTarget(it, arc);
					// No need to pop attackFrames or explosionFrames
					skipVideo(*it->video);
					shootsToRemove.push_back(i);
				} else if (frame > 0 && frame >= (int)(it->lastFrame)) {
					skipVideo(*it->video);
					shootsToRemove.push_back(i);
				} else if (it->video->decoder->needsUpdate() && needsUpdate) {
					updateScreen(*it->video);
				}
			} else if (!it->video && it->bodyFrames.size() > 0) {
				uint32 frame = _background->decoder->getCurFrame();
				uint32 bodyLastFrame = it->bodyFrames[it->bodyFrames.size() - 1].lastFrame();
				if (frame > it->startFrame && frame - it->startFrame >= bodyLastFrame)
					if (!it->destroyed) {
						incTargetsMissed();
						missedTarget(it, arc);
						shootsToRemove.push_back(i);
					}
			}
			i++;
		}
		if (shootsToRemove.size() > 0) {
			debugC(1, kHypnoDebugArcade, "Shoots to remove: %d", shootsToRemove.size());
			Common::sort(shootsToRemove.begin(), shootsToRemove.end());
			for (Common::List<uint32>::iterator it = shootsToRemove.reverse_begin(); it != shootsToRemove.end(); --it) {
				debugC(1, kHypnoDebugArcade, "Removing %d from %d size", *it, _shoots.size());
				delete _shoots[*it].video;
				_shoots.remove_at(*it);
			}
		}

		if (_music.empty() && !arc->music.empty()) {
			_music = _soundPath + arc->music;
			_musicRate = arc->musicRate;
			_musicStereo = arc->musicStereo;
			playSound(_music, 0, _musicRate, _musicStereo); // music loop forever
		}

		if (needsUpdate) {
			if (shootingPrimary) {
				shootingPrimary = shoot(mousePos, arc, false);
			} else if (shootingSecondary) {
				shootingSecondary = shoot(mousePos, arc, true);
			}

			drawPlayer();
			drawHealth();
			drawAmmo();
		}

		limiter.delayBeforeSwap();
		drawScreen();
		limiter.startFrame();
	}

	g_system->beginGFXTransaction();
	// Restore vsync state
	g_system->setFeatureState(OSystem::kFeatureVSync, vsync);
	g_system->endGFXTransaction();

	// Deallocate shoots
	for (Shoots::iterator it = _shoots.begin(); it != _shoots.end(); ++it) {
		if (it->video && it->video->decoder)
			skipVideo(*it->video);
		delete it->video;
	}

	if (_background->decoder) {
		skipVideo(*_background);
	}

	delete _background;
	_background = nullptr;

	if (_masks) {
		skipVideo(*_masks);
		delete _masks;
		_mask = nullptr;
		_masks = nullptr;
	}

	if (_additionalVideo) {
		skipVideo(*_additionalVideo);
		delete _additionalVideo;
		_additionalVideo = nullptr;
	}

	_timerStarted = false;
	removeTimers();
	stopSound();
	_music.clear();
}

Common::Point HypnoEngine::computeTargetPosition(const Common::Point &mousePos) {
	return mousePos;
}

Common::Point HypnoEngine::getPlayerPosition(bool needsUpdate) {
	return g_system->getEventManager()->getMousePos();
}

int HypnoEngine::detectTarget(const Common::Point &mousePos) {
	int i = -1;
	Common::Point target = computeTargetPosition(mousePos);
	if (target.x >= _compositeSurface->w || target.y >= _compositeSurface->h)
		return -1;

	if (target.x < 0 || target.y < 0)
		return -1;

	for (Shoots::iterator it = _shoots.begin(); it != _shoots.end(); ++it) {
		i++;
		if (it->destroyed)
			continue;

		if (it->animation != "NONE" && !it->video->decoder)
			continue;

		uint32 c =  _compositeSurface->getPixel(target.x, target.y);
		if (c >= it->paletteOffset && c < it->paletteOffset + it->paletteSize) {
			return i;
		}
	}
	return -1;
}

void HypnoEngine::drawCursorArcade(const Common::Point &mousePos) {
	int i = detectTarget(mousePos);
	if (i >= 0)
		changeCursor("target");
	else
		changeCursor("arcade");

}

bool HypnoEngine::clickedPrimaryShoot(const Common::Point &mousePos) { return true; }

bool HypnoEngine::shoot(const Common::Point &mousePos, ArcadeShooting *arc, bool secondary) {
	incShotsFired();
	int i = detectTarget(mousePos);
	if (i < 0) {
		missNoTarget(arc);
	} else {
		if (!_shoots[i].hitSound.empty())
			playSound(_soundPath + _shoots[i].hitSound, 1);

		incEnemyHits();
		if (_shoots[i].timesToShoot > 1) {
			_shoots[i].timesToShoot = _shoots[i].timesToShoot - 1;
			// Redraw cursor
			drawCursorArcade(mousePos);
			goto end;
		}

		if (!_shoots[i].deathSound.empty())
			playSound(_soundPath + _shoots[i].deathSound, 1);

		incTargetsDestroyed();
		incScore(_shoots[i].pointsToShoot);
		incBonus(_shoots[i].pointsToShoot);
		_shoots[i].destroyed = true;

		if (_shoots[i].animation != "NONE") {
			if (_shoots[i].deathPosition.x != 0 && _shoots[i].deathPosition.y != 0) {
				Common::Point position = computeTargetPosition(mousePos);
				_shoots[i].video->position = Common::Point(position.x, position.y) - _shoots[i].deathPosition;
			}
			int currentFrame = _shoots[i].video->decoder->getCurFrame();
			uint32 explosionIdx;
			for (explosionIdx = 0; explosionIdx < _shoots[i].bodyFrames.size(); explosionIdx++) {
				if (int(_shoots[i].bodyFrames[explosionIdx].lastFrame()) >= currentFrame)
					break;
			}
			if (explosionIdx > 0)
				explosionIdx = explosionIdx - 1;

			uint32 explosionStartFrame = _shoots[i].explosionFrames[explosionIdx].start;
			uint32 explosionLastFrame = _shoots[i].explosionFrames[explosionIdx].lastFrame();
			_objKillsCount[_objIdx] = _objKillsCount[_objIdx] + _shoots[i].objKillsCount;
			_shoots[i].video->decoder->forceSeekToFrame(explosionStartFrame - 2);
			_shoots[i].lastFrame = explosionLastFrame - 2;
		} else {
			if (!_shoots[i].explosionAnimation.empty()) {
				Common::Point position = computeTargetPosition(mousePos);
				_shoots[i].video = new MVideo(_shoots[i].explosionAnimation, position, true, false, false);
				playVideo(*_shoots[i].video);
				int w = _shoots[i].video->decoder->getWidth();
				int h = _shoots[i].video->decoder->getHeight();
				uint32 explosionLastFrame = _shoots[i].video->decoder->getFrameCount() - 1;
				_shoots[i].video->position = Common::Point(position.x - w / 2, position.y - h / 2);
				_shoots[i].lastFrame = explosionLastFrame - 1;
			} else if (_objIdx == 0 && !arc->hitBoss1Video.empty()) {
				_background->decoder->pauseVideo(true);
				MVideo video(arc->hitBoss1Video, Common::Point(0, 0), false, true, false);
				disableCursor();
				runIntro(video);
				// Should be currentPalette?
				loadPalette(arc->backgroundPalette);
				_background->decoder->pauseVideo(false);
				updateScreen(*_background);
				drawScreen();
				if (!_music.empty())
					playSound(_music, 0, _musicRate, _musicStereo); // restore music
			} else if (_objIdx == 1 && !arc->hitBoss2Video.empty()) {
				_background->decoder->pauseVideo(true);
				MVideo video(arc->hitBoss2Video, Common::Point(0, 0), false, true, false);
				runIntro(video);
				// Should be currentPalette?
				loadPalette(arc->backgroundPalette);
				_background->decoder->pauseVideo(false);
				updateScreen(*_background);
				drawScreen();
				drawCursorArcade(mousePos);
				if (!_music.empty())
					playSound(_music, 0, _musicRate, _musicStereo); // restore music
			}
			byte p[3] = {0x00, 0x00, 0x00}; // Always black?
			assert(_shoots[i].paletteSize == 1 || _shoots[i].paletteSize == 0);
			loadPalette((byte *) &p, _shoots[i].paletteOffset, _shoots[i].paletteSize);
			_objKillsCount[_objIdx] = _objKillsCount[_objIdx] + _shoots[i].objKillsCount;
		}
		// Redraw cursor
		drawCursorArcade(mousePos);
	}
	end:
	if (secondary) {
		if (_background->decoder->getCurFrame() % 2 == 0)
			drawShoot(mousePos);
		return clickedSecondaryShoot(mousePos);
	} else {
		drawShoot(mousePos);
		return false;
	}
}

void HypnoEngine::incBonus(int inc) {
	_bonus = _bonus + inc;
}

void HypnoEngine::incScore(int inc) {
	_score = _score + inc;
}

void HypnoEngine::incLivesUsed() {
	_stats.livesUsed++;
}

void HypnoEngine::incShotsFired() {
	_stats.shootsFired++;
}

void HypnoEngine::incEnemyHits() {
	_stats.enemyHits++;
}

void HypnoEngine::incEnemyTargets() {
	_stats.enemyTargets++;
}

void HypnoEngine::incTargetsDestroyed() {
	_stats.targetsDestroyed++;
}

void HypnoEngine::incTargetsMissed() {
	_stats.targetsMissed++;
}

uint32 HypnoEngine::killRatio() {
	if (_stats.enemyTargets == 0)
		return 0;
	return 100 * _stats.targetsDestroyed / _stats.enemyTargets;
}

uint32 HypnoEngine::accuracyRatio() {
	if (_stats.shootsFired == 0)
		return 0;
	return 100 * _stats.enemyHits / _stats.shootsFired;
}

void HypnoEngine::incFriendliesEncountered() {
	_stats.friendliesEncountered++;
}

void HypnoEngine::incInfoReceived() {
	_stats.infoReceived++;
}

void HypnoEngine::resetStatistics() {
	_stats = ArcadeStats();
	_bonus = 0;
}

bool HypnoEngine::clickedSecondaryShoot(const Common::Point &mousePos) {
	return false;
}

} // End of namespace Hypno

