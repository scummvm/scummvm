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
				if (x == 'Y')
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
	g_parsedArc->background.clear();
	g_parsedArc->player.clear();
	g_parsedArc->shoots.clear();
}

ShootSequence HypnoEngine::parseShootList(const Common::String &filename, const Common::String &data) {
	debugC(1, kHypnoDebugParser, "Parsing %s", filename.c_str());
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

	// Parsing
	Common::StringTokenizer tok(pdata, " ,.\t");
	Common::String n;
	ShootInfo si;
	ShootSequence seq;
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
			error("Error at parsing '%s'", n.c_str());
		seq.push_back(si);
		debugC(1, kHypnoDebugParser, "%d -> %s", si.timestamp, si.name.c_str());
	}
	return seq;
}

void HypnoEngine::loadArcadeLevel(const Common::String &arclevel, const Common::String &next, const Common::String &prefix) {
	debugC(1, kHypnoDebugParser, "Parsing %s", arclevel.c_str());
	Common::String arc;
	Common::String list;
	splitArcadeFile(arclevel, arc, list);
	debugC(1, kHypnoDebugParser, "%s", arc.c_str());
	parseArcadeShooting("", arclevel, arc);
	ArcadeShooting *arcade = (ArcadeShooting *) _levels[arclevel]; 
	arcade->shootSequence = parseShootList(arclevel, list);
	arcade->prefix = prefix;
	arcade->levelIfWin = next;
}

void HypnoEngine::drawPlayer() { error("Function \"%s\" not implemented", __FUNCTION__); }
void HypnoEngine::drawHealth() { error("Function \"%s\" not implemented", __FUNCTION__); }
void HypnoEngine::drawShoot(const Common::Point &target) { error("Function \"%s\" not implemented", __FUNCTION__); }

void HypnoEngine::hitPlayer() {
	if ( _playerFrameSep < (int)_playerFrames.size()){
		if (_playerFrameIdx < _playerFrameSep)
			_playerFrameIdx = _playerFrameSep;
	} else {
		uint32 c = 251; // red
		_compositeSurface->fillRect(Common::Rect(0, 0, 640, 480), c);
		drawScreen();
	}
	//if (!_hitSound.empty())
	//	playSound(_soundPath + _hitSound, 1);
}

void HypnoEngine::runArcade(ArcadeShooting *arc) {
	_arcadeMode = arc->mode;
	Common::Point mousePos;
	Common::List<uint32> shootsToRemove;
	ShootSequence shootSequence = arc->shootSequence;
	_levelId = arc->id;
	_shootSound = arc->shootSound;
	_hitSound = arc->hitSound;
	_score = 0;
	_health = arc->health;
	_maxHealth = _health;
	changeCursor("arcade");
	_shoots.clear();
	_playerFrames = decodeFrames(arc->player);
	_playerFrameSep = 0;
	// Only used in spider
	_currentPlayerPosition = PlayerLeft;
	_lastPlayerPosition = PlayerLeft;
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

	MVideo background = MVideo(arc->background, Common::Point(0, 0), false, false, false);

	changeCursor("arcade");
	playVideo(background);
	loadPalette(arc->palette);
	bool shootingPrimary = false;
	bool shootingSecondary = false;
	bool needsUpdate = true;

	Common::Event event;
	while (!shouldQuit()) {
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
					_currentPlayerPosition = PlayerLeft;
				} else if (event.kbd.keycode == Common::KEYCODE_DOWN) {
					_lastPlayerPosition = _currentPlayerPosition;
					_currentPlayerPosition = PlayerBottom;
				} else if (event.kbd.keycode == Common::KEYCODE_RIGHT) {
					_lastPlayerPosition = _currentPlayerPosition;
					_currentPlayerPosition = PlayerRight;
				} else if (event.kbd.keycode == Common::KEYCODE_UP) {
					_lastPlayerPosition = _currentPlayerPosition;
					_currentPlayerPosition = PlayerTop;
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
			if (!arc->defeatNoEnergyVideo.empty()) {
				MVideo video(arc->defeatNoEnergyVideo, Common::Point(0, 0), false, false, false);
				runIntro(video);
			}
			assert(!arc->levelIfLose.empty());
			_nextLevel = arc->levelIfLose;
			debugC(1, kHypnoDebugArcade, "Losing level and jumping to %s", _nextLevel.c_str());
			break;
		}

		if (!arc->transitionVideo.empty() && background.decoder->getCurFrame() >= (int)arc->transitionTime) {
			debugC(1, kHypnoDebugArcade, "Playing transition %s", arc->transitionVideo.c_str());
			arc->transitionTime = background.decoder->getFrameCount() + 1;
			MVideo video(arc->transitionVideo, Common::Point(0, 0), false, false, false);
			runIntro(video);
			skipVideo(background);
		}

		if (checkArcadeLevelCompleted(background)) {
			skipVideo(background);
			if (!arc->nextLevelVideo.empty()) {
				MVideo video(arc->nextLevelVideo, Common::Point(0, 0), false, false, false);
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

		if (shootSequence.size() > 0) {
			ShootInfo si = shootSequence.front();
			if ((int)si.timestamp <= background.decoder->getCurFrame()) {
				shootSequence.pop_front();
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
				if (frame > 0 && frame >= (int)(it->attackFrame) && !it->destroyed) {
					_health = _health - it->attackWeight;
					hitPlayer();
					it->attackFrame = it->video->decoder->getFrameCount() + 1; // It will never attack again
				}

				if (frame > 0 && frame >= (int)(it->explosionFrame - 3) && !it->destroyed) {
					skipVideo(*it->video);
				} else if (frame > 0 && frame >= (int)(it->video->decoder->getFrameCount() - 2)) {
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

		if (_music.empty()) {
			_music = _soundPath + arc->music;
			playSound(_music, 0); // music loop forever
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

		g_system->delayMillis(10);
	}

	// Deallocate shoots
	for (Shoots::iterator it = _shoots.begin(); it != _shoots.end(); ++it) {
		if (it->video && it->video->decoder)
			skipVideo(*it->video);
		delete it->video;
	}

	if (background.decoder)
		skipVideo(background);

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
		playSound(_soundPath + _shoots[i].hitSound, 1);
		playSound(_soundPath + _shoots[i].deathSound, 1);
		_score = _score + _shoots[i].pointsToShoot;
		_shoots[i].destroyed = true;

		if (_shoots[i].animation != "NONE") {
			int w = _shoots[i].video->decoder->getWidth();
			int h = _shoots[i].video->decoder->getHeight();
			_shoots[i].video->position = Common::Point(mousePos.x - w / 2, mousePos.y - h / 2);
			_shoots[i].video->decoder->forceSeekToFrame(_shoots[i].explosionFrame + 2);
		} else {
			byte p[3] = {}; // Always black?
			assert(_shoots[i].paletteSize == 1 || _shoots[i].paletteSize == 0);
			loadPalette((byte *) &p, _shoots[i].paletteOffset, _shoots[i].paletteSize);
		}

	}
}

bool HypnoEngine::checkArcadeLevelCompleted(MVideo &background) {
	return !background.decoder || background.decoder->endOfVideo() || _skipLevel;
}

bool HypnoEngine::clickedSecondaryShoot(const Common::Point &mousePos) {
	return false;
}

} // End of namespace Hypno

