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

#include "hypno/grammar.h"
#include "hypno/hypno.h"

namespace Hypno {

extern int parse_arc(const char *);

void HypnoEngine::splitArcadeFile(const Common::String &filename, Common::String &arc, Common::String &list) {
	Common::File file;
	if (!file.open(filename.c_str()))
		error("Failed to open %s", filename.c_str());

	while (!file.eos()) {
		byte x = file.readByte();
		arc += x;
		if (x == 'X') {
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
	g_parsedArc->backgroundVideo.clear();
	g_parsedArc->transitionPalette.clear();
	g_parsedArc->player.clear();
	g_parsedArc->shoots.clear();
	g_parsedArc->intros.clear();
	g_parsedArc->defeatNoEnergyFirstVideo.clear();
	g_parsedArc->defeatMissBossVideo.clear();
	g_parsedArc->defeatNoEnergySecondVideo.clear();
	g_parsedArc->beforeVideo.clear();
	g_parsedArc->briefingVideo.clear();
	g_parsedArc->segments.clear();
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
	SegmentShootsSequence seq;
	// Parsing

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
		Common::StringTokenizer tok(pdata, " ,.\t");
		while (!tok.empty()) {
			t = tok.nextToken();
			if (t[0] == '\n')
				continue;
			n = tok.nextToken();
			if (t == "Z")
				break;

			Common::replace(n, "\nS", "");
			Common::replace(n, "\nZ\n", "");
			si.name = n;
			si.timestamp = atoi(t.c_str());
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
void HypnoEngine::drawShoot(const Common::Point &target) { error("Function \"%s\" not implemented", __FUNCTION__); }
void HypnoEngine::hitPlayer() { error("Function \"%s\" not implemented", __FUNCTION__); }

void HypnoEngine::runBeforeArcade(ArcadeShooting *arc) {}

void HypnoEngine::initSegment(ArcadeShooting *arc) { error("Function \"%s\" not implemented", __FUNCTION__); }
void HypnoEngine::findNextSegment(ArcadeShooting *arc) { error("Function \"%s\" not implemented", __FUNCTION__); }

void HypnoEngine::runArcade(ArcadeShooting *arc) {
	_arcadeMode = arc->mode;
	Common::Point mousePos;
	Common::List<uint32> shootsToRemove;

	// segment/shoots
	Segments segments = arc->segments;
	initSegment(arc);

	_levelId = arc->id;
	_shootSound = arc->shootSound;
	_hitSound = arc->hitSound;
	_health = arc->health;
	_maxHealth = _health;
	debugC(1, kHypnoDebugArcade, "Starting segment of type %x", segments[_segmentIdx].type);
	changeCursor("arcade");
	_shoots.clear();
	if (!arc->player.empty())
		_playerFrames = decodeFrames(arc->player);
	_playerFrameSep = 0;
	// Only used in spider
	_currentPlayerPosition = kPlayerLeft;
	_lastPlayerPosition = kPlayerLeft;
	_skipLevel = false;

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

	MVideo background = MVideo(arc->backgroundVideo, Common::Point(0, 0), false, false, false);

	changeCursor("arcade");
	playVideo(background);
	float rate = background.decoder->getFrameRate().toDouble();
	if (rate < 10) {
		debugC(1, kHypnoDebugArcade, "Used frame rate looks odd: %f, increasing x 10", rate);
		background.decoder->setRate(10.0);
	}
	loadPalette(arc->backgroundPalette);
	bool shootingPrimary = false;
	bool shootingSecondary = false;
	bool needsUpdate = true;
	bool transition = false;

	_objIdx = 0;
	_objKillsCount[0] = 0;
	_objKillsCount[1] = 0;
	_objMissesCount[0] = 0;
	_objMissesCount[1] = 0;

	debugC(1, kHypnoDebugArcade, "Using frame delay: %d", arc->frameDelay);

	Common::Event event;
	while (!shouldQuit()) {
		//debug("frame: %d", background.decoder->getCurFrame());
		needsUpdate = background.decoder->needsUpdate();
		while (g_system->getEventManager()->pollEvent(event)) {
			mousePos = g_system->getEventManager()->getMousePos();
			// Events
			switch (event.type) {

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_c) {
					background.decoder->pauseVideo(true);
					showCredits();
					changeScreenMode("320x200");
					background.decoder->pauseVideo(false);
				} else if (event.kbd.keycode == Common::KEYCODE_k) { // Added for testing
					_health = 0;
				} else if (event.kbd.keycode == Common::KEYCODE_LEFT) {
					_lastPlayerPosition = _currentPlayerPosition; 
					_currentPlayerPosition = kPlayerLeft;
				} else if (event.kbd.keycode == Common::KEYCODE_DOWN) {
					_lastPlayerPosition = _currentPlayerPosition;
					_currentPlayerPosition = kPlayerBottom;
				} else if (event.kbd.keycode == Common::KEYCODE_RIGHT) {
					_lastPlayerPosition = _currentPlayerPosition;
					_currentPlayerPosition = kPlayerRight;
				} else if (event.kbd.keycode == Common::KEYCODE_UP) {
					_lastPlayerPosition = _currentPlayerPosition;
					_currentPlayerPosition = kPlayerTop;
				}
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
				break;

			default:
				break;
			}
		}

		if (needsUpdate) {
			drawScreen();
			updateScreen(background);
		}

		if (_health <= 0) {
			skipVideo(background);
			if (!arc->defeatNoEnergySecondVideo.empty() && transition) {
				MVideo video(arc->defeatNoEnergySecondVideo, Common::Point(0, 0), false, true, false);
				runIntro(video);
			} else if (!arc->defeatNoEnergyFirstVideo.empty()) {
				MVideo video(arc->defeatNoEnergyFirstVideo, Common::Point(0, 0), false, true, false);
				runIntro(video);
			}
			assert(!arc->levelIfLose.empty());
			_nextLevel = arc->levelIfLose;
			debugC(1, kHypnoDebugArcade, "Losing level and jumping to %s", _nextLevel.c_str());
			_lives = _lives - 1;
			break;
		}

		if (!arc->transitionVideo.empty() && !transition && background.decoder->getCurFrame() >= (int)arc->transitionTime) {
			transition = true;
			background.decoder->pauseVideo(true);

			debugC(1, kHypnoDebugArcade, "Playing transition %s", arc->transitionVideo.c_str());
			MVideo video(arc->transitionVideo, Common::Point(0, 0), false, true, false);
			runIntro(video);

			if (!arc->transitionPalette.empty())
				loadPalette(arc->transitionPalette);
			else
				loadPalette(arc->backgroundPalette);
			background.decoder->pauseVideo(false);
			updateScreen(background);
			drawScreen();
		}

		if (background.decoder && background.decoder->getCurFrame() >= int(segments[_segmentIdx].start + segments[_segmentIdx].size - 1)) {
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
				background.decoder->forceSeekToFrame(segments[_segmentIdx].start+2);
				needsUpdate = true;
				continue;
			} else 
				_skipLevel = true;
		}

		if (segments[_segmentIdx].end || _skipLevel) {
			skipVideo(background);
			// Objectives
			if ((_objKillsCount[_objIdx] > 0 || _objMissesCount[_objIdx] > 0) && !_skipLevel) {
				if (_objKillsCount[_objIdx] < arc->objKillsRequired[_objIdx] || _objMissesCount[_objIdx] > arc->objMissesAllowed[_objIdx]) {
					if (!arc->defeatMissBossVideo.empty()) {
						MVideo video(arc->defeatMissBossVideo, Common::Point(0, 0), false, true, false);
						runIntro(video);
					}
					assert(!arc->levelIfLose.empty());
					_nextLevel = arc->levelIfLose;
					_lives = _lives - 1;
					_arcadeMode = "";
					debugC(1, kHypnoDebugArcade, "Losing level (objectives) and jumping to %s", _nextLevel.c_str());
					break;
				}
			}

			if (!arc->nextLevelVideo.empty()) {
				MVideo video(arc->nextLevelVideo, Common::Point(0, 0), false, true, false);
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
					+ background.decoder->getCurFrame() \
					- (int)segments[_segmentIdx].start; 
			//debug("%d %d", si.timestamp, idx);
			if ((int)si.timestamp <= idx) {
				_shootSequence.pop_front();
				for (Shoots::iterator it = arc->shoots.begin(); it != arc->shoots.end(); ++it) {
					if (it->name == si.name) {
						Shoot s = *it;
						if (it->animation == "NONE") {
							if ((uint32)(it->name[0]) == _currentPlayerPosition) {
								_health = _health - it->attackWeight;
								hitPlayer();
							}
							byte p[3] = {0xff, 0x00, 0x00}; // Always red?
							assert(s.paletteSize == 1 || s.paletteSize == 0);
							loadPalette((byte *) &p, s.paletteOffset, s.paletteSize);
							_shoots.push_back(s);
						} else {
							s.video = new MVideo(it->animation, it->position, true, false, false);
							playVideo(*s.video);
							s.video->currentFrame = s.video->decoder->decodeNextFrame(); // Skip the first frame
							if (s.attackFrames.size() == 0) {
								uint32 lastFrame = s.bodyFrames.back().lastFrame();
								s.attackFrames.push_back(lastFrame - 3);
							}
							s.lastFrame = s.bodyFrames[s.bodyFrames.size() - 1].lastFrame();
							loadPalette(s.video->decoder->getPalette() + 3*s.paletteOffset, s.paletteOffset, s.paletteSize);
							_shoots.push_back(s);
							playSound(_soundPath + arc->enemySound, 1);
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
						_health = _health - it->attackWeight;
						hitPlayer();
						it->attackFrames.pop_front();
					}
				}

				uint32 bodyLastFrame = it->bodyFrames[it->bodyFrames.size() - 1].lastFrame();
				if (frame > 0 && frame >= (int)(bodyLastFrame - 3) && !it->destroyed) {
					// No need to pop attackFrames or explosionFrames
					skipVideo(*it->video);
				} else if (frame > 0 && frame >= (int)(it->lastFrame)) {
					skipVideo(*it->video);
					shootsToRemove.push_back(i);
				} else if (it->video->decoder->needsUpdate() && needsUpdate) {
					updateScreen(*it->video);
				}
			}
			i++;
		}
		if (shootsToRemove.size() > 0) {
			for (Common::List<uint32>::iterator it = shootsToRemove.begin(); it != shootsToRemove.end(); ++it) {
				debugC(1, kHypnoDebugArcade, "Removing %d from %d size", *it, _shoots.size());
				delete _shoots[*it].video;
				_shoots.remove_at(*it);
			}
		}

		if (_music.empty() && !arc->music.empty()) {
			_music = _soundPath + arc->music;
			playSound(_music, 0, arc->musicRate); // music loop forever
		}

		if (needsUpdate) {
			if (shootingPrimary || shootingSecondary) {
				shoot(mousePos);
				drawShoot(mousePos);
				shootingPrimary = false;
			}

			drawPlayer();
			drawHealth();
		}

		g_system->delayMillis(arc->frameDelay);
	}

	// Deallocate shoots
	for (Shoots::iterator it = _shoots.begin(); it != _shoots.end(); ++it) {
		if (it->video && it->video->decoder)
			skipVideo(*it->video);
		delete it->video;
	}

	if (background.decoder)
		skipVideo(background);

	stopSound();
	_music.clear();
}

int HypnoEngine::detectTarget(const Common::Point &mousePos) {
	int i = -1;
	int x = mousePos.x;
	int y = mousePos.y;
	for (Shoots::iterator it = _shoots.begin(); it != _shoots.end(); ++it) {
		i++;
		if (it->destroyed)
			continue;

		if (it->animation != "NONE" && !it->video->decoder)
			continue;

		uint32 c =  _compositeSurface->getPixel(x, y);
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
		
	g_system->copyRectToScreen(_compositeSurface->getPixels(), _compositeSurface->pitch, 0, 0, _screenW, _screenH);
}

bool HypnoEngine::clickedPrimaryShoot(const Common::Point &mousePos) { return true; }

void HypnoEngine::shoot(const Common::Point &mousePos) {
	int i = detectTarget(mousePos);
	if (i >= 0) {
		if (!_shoots[i].hitSound.empty())
			playSound(_soundPath + _shoots[i].hitSound, 1);
		if (!_shoots[i].deathSound.empty())
			playSound(_soundPath + _shoots[i].deathSound, 1);
		_score = _score + _shoots[i].pointsToShoot;
		_shoots[i].destroyed = true;

		if (_shoots[i].animation != "NONE") {
			if (_shoots[i].deathPosition.x != 0 && _shoots[i].deathPosition.y != 0)
				_shoots[i].video->position = Common::Point(mousePos.x, mousePos.y) - _shoots[i].deathPosition;

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
			byte p[3] = {0x00, 0x00, 0x00}; // Always black?
			assert(_shoots[i].paletteSize == 1 || _shoots[i].paletteSize == 0);
			loadPalette((byte *) &p, _shoots[i].paletteOffset, _shoots[i].paletteSize);
			_objKillsCount[_objIdx] = _objKillsCount[_objIdx] + _shoots[i].objKillsCount;
			if (!_shoots[i].explosionAnimation.empty()) {
				_shoots[i].video = new MVideo(_shoots[i].explosionAnimation, mousePos, true, false, false);
				playVideo(*_shoots[i].video);
				int w = _shoots[i].video->decoder->getWidth();
				int h = _shoots[i].video->decoder->getHeight();
				_shoots[i].video->position = Common::Point(mousePos.x - w / 2, mousePos.y - h / 2);
			}
		}
	}
}

bool HypnoEngine::checkArcadeLevelCompleted(MVideo &background, Segment segment) {
	return !background.decoder || background.decoder->endOfVideo() || segment.end || _skipLevel;
}

bool HypnoEngine::clickedSecondaryShoot(const Common::Point &mousePos) {
	return false;
}

} // End of namespace Hypno

