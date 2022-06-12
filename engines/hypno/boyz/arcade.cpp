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

void BoyzEngine::runBeforeArcade(ArcadeShooting *arc) {
	_checkpoint = _currentLevel;
	_lastStats = _stats;
	if (!_name.empty()) // if name is name, then we are testing some level
		saveProfile(_name, int(arc->id));

	if (arc->mode == "YM") {
		assert(!arc->player.empty());
		_playerFrames = decodeFrames(arc->player);
		_playerFrameSep = 0;

		Common::Rect healthBarBox(0, 3, 107, 18);
		Common::Rect ammoBarBox(0, 20, 103, 34);
		Common::Rect portraitBox(0, 40, 57, 94);

		for (int i = 0; i < int(_playerFrames.size()); i++) {
			_healthBar[i+1] = _playerFrames[i]->getSubArea(healthBarBox);
			_ammoBar[i+1] = _playerFrames[i]->getSubArea(ammoBarBox);
			_portrait[i+1] = _playerFrames[i]->getSubArea(portraitBox);
		}

		_playerFrameSep = _playerFrames.size();
		_playerFrameIdx = -1;
	} else {
		uint32 r = 1 + _rnd->getRandomNumber(1);
		arc->backgroundVideo = Common::String::format("c3/c35c0%ds.smk", r);
		_playerFrameSep = 0;
		_playerFrameIdx = -1;
		if (arc->mode != "YS")
			error("Invalid mode: %s", arc->mode.c_str());
	}

	if (!arc->beforeVideo.empty()) {
		MVideo video(arc->beforeVideo, Common::Point(0, 0), false, true, false);
		runIntro(video);
	}

	_currentScript = arc->script;
	// Reload all weapons
	for (Script::iterator it = _currentScript.begin(); it != _currentScript.end(); ++it) {
		_ammo = _weaponMaxAmmo[it->cursor];
	}

	updateFromScript();
	_shootsDestroyed.clear();
	_health = _previousHealth;
	_selectedCorrectBox = 0;
}

void BoyzEngine::runAfterArcade(ArcadeShooting *arc) {
	for (int i = 0; i < int(_playerFrames.size()); i++) {
		_playerFrames[i]->free();
		delete _playerFrames[i];
	}
	_playerFrames.clear();


	if (_health <= 0) {
		if (_arcadeMode == "YS")
			return;
		_stats = _lastStats;
		MVideo video(_deathDay[_currentActor], Common::Point(0, 0), false, true, false);
		disableCursor();
		runIntro(video);
		return;
	}

	if (_currentLevel == lastLevelTerritory(_currentLevel)) {
		byte *palette;
		int territory = getTerritory(_currentLevel) - 1;
		Graphics::Surface *stats = decodeFrame("preload/stats.smk", territory, &palette);
		loadPalette(palette, 0, 256);
		drawImage(*stats, 0, 0, true);
		uint32 enemyAvailable = _stats.targetsDestroyed + _stats.targetsMissed;
		drawString("scifi08.fgx", Common::String::format("%d", enemyAvailable), 240, 40, 0, kHypnoColorWhiteOrBlue);
		drawString("scifi08.fgx", Common::String::format("%d%%", 100 * _stats.targetsDestroyed / enemyAvailable), 240, 54, 0, kHypnoColorWhiteOrBlue);
		drawString("scifi08.fgx", Common::String::format("%d", _stats.shootsFired), 240, 77, 0, kHypnoColorWhiteOrBlue);
		drawString("scifi08.fgx", Common::String::format("%d%%", accuracyRatio()), 240, 92, 0, kHypnoColorWhiteOrBlue);
		drawString("scifi08.fgx", Common::String::format("%d", -uint32(-1) - _lives), 240, 117, 0, kHypnoColorWhiteOrBlue);
		drawString("scifi08.fgx", Common::String::format("%d", _stats.friendliesEncountered), 240, 142, 0, kHypnoColorWhiteOrBlue);
		drawString("scifi08.fgx", Common::String::format("%d", _stats.infoReceived), 240, 158, 0, kHypnoColorWhiteOrBlue);

		bool cont = true;
		while (!shouldQuit() && cont) {
			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				switch (event.type) {
					case Common::EVENT_KEYDOWN:
					cont = false;
					break;
					default:
					break;
				}
			}
			drawScreen();
			g_system->delayMillis(10);
		}

		// Merge current stats with the global ones
		_globalStats.shootsFired = _stats.shootsFired + _globalStats.shootsFired;
		_globalStats.enemyHits = _stats.enemyHits + _globalStats.enemyHits;
		_globalStats.enemyTargets = _stats.enemyTargets + _globalStats.enemyTargets;
		_globalStats.targetsDestroyed = _stats.targetsDestroyed + _globalStats.targetsDestroyed;
		_globalStats.targetsMissed = _stats.targetsMissed + _globalStats.targetsMissed;
		_globalStats.friendliesEncountered = _stats.friendliesEncountered + _globalStats.friendliesEncountered;
		_globalStats.infoReceived = _stats.infoReceived + _globalStats.infoReceived;
		// After that, we can reset the current stats
		resetStatistics();
	}

	_previousHealth = _health;
	_sceneState[Common::String::format("GS_SEQ_%d", _levelId)] = 1;
}

void BoyzEngine::pressedKey(const int keycode) {
	if (keycode == Common::KEYCODE_c) {
		if (_cheatsEnabled) {
			_skipLevel = true;
			return;
		}
	} else if (keycode == Common::KEYCODE_k) { // Added for testing
		_health = 0;
	} else if (keycode == Common::KEYCODE_ESCAPE) {
		openMainMenuDialog();
	}
}

void BoyzEngine::updateFromScript() {
	if (_currentScript.size() > 0) {
		ScriptInfo si = *_currentScript.begin();
		//debug("%d %d %d", si.time, _background->decoder->getCurFrame(), si.actor);
		if (!_background || int(si.time) <= _background->decoder->getCurFrame()) {
			if (_currentActor != si.actor)
				_ammo = _weaponMaxAmmo[si.cursor];

			_currentActor = si.actor;
			_currentMode = si.mode;
			_currentWeapon = si.cursor;
			_currentScript.pop_front();

			if (_currentMode == NonInteractive)
				changeCursor(_crosshairsInactive[_currentWeapon], _crosshairsPalette, true);
			else
				changeCursor(_crosshairsActive[_currentWeapon], _crosshairsPalette, true);
		}
	}
}

void BoyzEngine::drawCursorArcade(const Common::Point &mousePos) {
	if (_currentMode == NonInteractive) {
		return;
	}

	int i = detectTarget(mousePos);
	if (i >= 0)
		changeCursor(_crosshairsTarget[_currentWeapon], _crosshairsPalette, true);
	else
		changeCursor(_crosshairsActive[_currentWeapon], _crosshairsPalette, true);
}

void BoyzEngine::drawPlayer() {
	updateFromScript();
	if (_arcadeMode != "YS")
		drawImage(_portrait[_currentActor], 0, 200 - _portrait[_currentActor].h, true);
}

void BoyzEngine::drawHealth() {
	updateFromScript();
	if(_arcadeMode == "YS")
		return;

	float w = float(_health) / float(_maxHealth);
	Common::Rect healthBarBox(0, 0, int((_healthBar[_currentActor].w - 3) * w), _healthBar[_currentActor].h / 2);

	uint32 c = kHypnoColorWhiteOrBlue; // white
	_compositeSurface->fillRect(healthBarBox, c);

	for (int i = 0; i < _maxHealth; i = i + 10) {
		int x = (_healthBar[_currentActor].w - 3) * float(i) / float(_maxHealth);
		_compositeSurface->drawLine(x, 2, x, 6, 0);
	}

	drawImage(_healthBar[_currentActor], 0, 0, true);
}

void BoyzEngine::drawAmmo() {
	updateFromScript();
	if(_arcadeMode == "YS")
		return;

	float w = float(_ammoBar[_currentActor].w) / float(_weaponMaxAmmo[_currentWeapon]);

	Common::Rect ammoBarBox(320 - int(_ammo * w), 0, 320, _ammoBar[_currentActor].h / 2);
	uint32 c = kHypnoColorGreen; // green
	_compositeSurface->fillRect(ammoBarBox, c);

	drawImage(_ammoBar[_currentActor], 320 - _ammoBar[_currentActor].w, 0, true);
	for (int i = 1; i < _weaponMaxAmmo[_currentWeapon]; i++) {
		int x = 320 - _ammoBar[_currentActor].w + int (i * w);
		_compositeSurface->drawLine(x, 2, x, 6, 0);
	}
}

void BoyzEngine::hitPlayer() {
	if(_arcadeMode == "YS")
		return; // Should never happen?

	uint32 c = kHypnoColorRed; // red
	_compositeSurface->fillRect(Common::Rect(0, 0, _screenW, _screenH), c);
	drawScreen();
	if (!_infiniteHealthCheat) {
		_health = _health - 10;
	}
	if (!_hitSound.empty())
		playSound(_soundPath + _hitSound, 1, 11025);
}
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

void BoyzEngine::findNextSegment(ArcadeShooting *arc) {
	_segmentIdx = _segmentIdx + 1;
}

bool BoyzEngine::checkTransition(ArcadeTransitions &transitions, ArcadeShooting *arc) {
	ArcadeTransition at = *transitions.begin();
	int ttime = at.time;
	if (_background->decoder->getCurFrame() > ttime) {
		if (_background->decoder->getCurFrame() > ttime + 3) {
			debugC(1, kHypnoDebugArcade, "Skipped transition of %d at %d", ttime, _background->decoder->getCurFrame());
		} else if (at.video == "NONE") {
			if (!at.palette.empty()) {
				_background->decoder->pauseVideo(true);
				_currentPalette = at.palette;
				loadPalette(_currentPalette);
				_background->decoder->pauseVideo(false);
				drawPlayer();
				updateScreen(*_background);
				drawScreen();
			} else if (!arc->additionalSound.empty())
				playSound(arc->additionalSound, 1, arc->additionalSoundRate);
			else if (_levelId == 36) {
				if (!checkArcadeObjectives()) {
					_health = 0;
					// Not sure how to handle this
				}
			} else if (_levelId == 352) {
				// Objectives are never checked here, for some reason
				_skipLevel = true;
			} else if (_levelId == 51) {
				if (_selectedCorrectBox == 0) {
					_background->decoder->pauseVideo(true);
					_background->decoder->forceSeekToFrame(ttime - 2);
					_masks->decoder->forceSeekToFrame(ttime - 2);
					const Graphics::Surface *frame = _background->decoder->decodeNextFrame();
					Graphics::Surface *boxes = frame->convertTo(frame->format, _background->decoder->getPalette());
					drawImage(*boxes, 0, 0, false);
					drawScreen();
					_selectedCorrectBox = pickABox();
					if (_selectedCorrectBox == 1) {
						_background->decoder->forceSeekToFrame(582);
						_masks->decoder->forceSeekToFrame(582);
					} else if (_selectedCorrectBox == -1) {
						_background->decoder->forceSeekToFrame(525);
						_masks->decoder->forceSeekToFrame(525);
					} else
						error("Invalid value for _selectedCorrectBox: %d", _selectedCorrectBox);
					_background->decoder->pauseVideo(false);
					updateScreen(*_background);
					drawScreen();
				} else if (_selectedCorrectBox == -1) {
					_health = 0;
				}
			}
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
			if (!_music.empty())
				playSound(_music, 0, arc->musicRate); // restore music
			drawPlayer();
			updateScreen(*_background);
			drawScreen();
			drawCursorArcade(g_system->getEventManager()->getMousePos());
		} else if (!at.sound.empty()) {
			playSound(at.sound, 1, at.soundRate);
		} else if (at.jumpToTime > 0) {
			_background->decoder->forceSeekToFrame(at.jumpToTime);
			_masks->decoder->forceSeekToFrame(at.jumpToTime);
		} else if (at.loseLevel) {
			_health = 0;
		} else
			error ("Invalid transition at %d", ttime);

		transitions.pop_front();

		return true;
	}
	return false;
}

int BoyzEngine::detectTarget(const Common::Point &mousePos) {
	Common::Point target = computeTargetPosition(mousePos);
	if (!_mask)
		return -1;

	uint32 m = _mask->getPixel(target.x, target.y);
	if (m == 0)
		return -1;

	int i = 0;
	for (Shoots::iterator it = _shoots.begin(); it != _shoots.end(); ++it) {
		if (!it->bodyFrames.empty() && _background->decoder->getCurFrame() > int(it->bodyFrames.back().start)) {
			i++;
			continue;  // This shoot is old!
		}

		if (m == it->paletteOffset && !_shoots[i].destroyed)
			return i;
		i++;
	}
	if (i == int(_shoots.size()))
		return -1;

	error("Invalid mask state (%d)!", m);
}

char BoyzEngine::selectDirection() {
	Common::Event event;
	Common::Rect button(252, 158, 315, 195);
	Graphics::Surface *screen = _compositeSurface->convertTo(_compositeSurface->format, _background->decoder->getPalette());
	Frames mapFrames = decodeFrames("c4/minemap.smk");
	bool showMap = _sceneState["GS_MINEMAP"];
	bool viewingMap = false;
	if (showMap)
		drawImage(*mapFrames[0], 0, 0, true);

	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Point mousePos = g_system->getEventManager()->getMousePos();
			switch (event.type) {
				case Common::EVENT_MOUSEMOVE:
					if (showMap && button.contains(mousePos))
						defaultCursor();
					else if (!viewingMap && mousePos.x <= _screenW / 3)
						changeCursor(_leftArrowPointer, _crosshairsPalette, true);
					else if (!viewingMap && mousePos.x >= 2 * _screenW / 3)
						changeCursor(_rightArrowPointer, _crosshairsPalette, true);
					else if (!viewingMap)
						changeCursor(_crossPointer, _crosshairsPalette, true);
					break;

				case Common::EVENT_LBUTTONDOWN:
					if (showMap && button.contains(mousePos)) {
						if (viewingMap) {
							drawImage(*screen, 0, 0, false);
							drawImage(*mapFrames[0], 0, 0, true);
						} else {
							drawImage(*mapFrames[1], 0, 0, true);
						}
						viewingMap = !viewingMap;
					} else if (!viewingMap && mousePos.x <= _screenH / 2) {
						return 'L';
					} else if (!viewingMap)
						return 'R';
					break;

				default:
					break;
			}
		}
		drawScreen();
		g_system->delayMillis(10);
	}
	return 0;
}


void BoyzEngine::waitForUserClick(uint32 timeout) {
	Common::Event event;
	bool cont = true;
	Common::Rect button(252, 158, 315, 195);
	while (!shouldQuit() && cont) {
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Point mousePos = g_system->getEventManager()->getMousePos();
			switch (event.type) {
				case Common::EVENT_QUIT:
				case Common::EVENT_RETURN_TO_LAUNCHER:
					cont = false;
					break;

				case Common::EVENT_LBUTTONDOWN:
					if (button.contains(mousePos))
						cont = false;
					break;

				default:
					break;
			}
		}
		drawScreen();
		g_system->delayMillis(10);
	}
}

int BoyzEngine::pickABox() {
	Common::Event event;

	Common::Rect correctBox(84, 14, 135, 66);
	Common::Rect incorrectBoxes[6];
	incorrectBoxes[0] = Common::Rect(15, 17, 77, 66);
	incorrectBoxes[1] = Common::Rect(2, 69, 84, 92);
	incorrectBoxes[2] = Common::Rect(74, 108, 242, 138);
	incorrectBoxes[3] = Common::Rect(62, 134, 245, 160);
	incorrectBoxes[4] = Common::Rect(59, 161, 239, 190);
	incorrectBoxes[5] = Common::Rect(135, 29, 223, 101);
	int i;
	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Point mousePos = g_system->getEventManager()->getMousePos();
			switch (event.type) {
				case Common::EVENT_MOUSEMOVE:
					if (correctBox.contains(mousePos)) {
						changeCursor(_crosshairsTarget[_currentWeapon], _crosshairsPalette, true);
						break;
					}
					for (i = 0; i < 6; i++)
						if (incorrectBoxes[i].contains(mousePos)) {
							changeCursor(_crosshairsTarget[_currentWeapon], _crosshairsPalette, true);
							break;
						}
					if (i == 6)
						changeCursor(_crosshairsActive[_currentWeapon], _crosshairsPalette, true);
					break;

				case Common::EVENT_LBUTTONDOWN:
					if (correctBox.contains(mousePos))
						return 1;
					for (i = 0; i < 6; i++)
						if (incorrectBoxes[i].contains(mousePos))
							return -1;
					break;

				default:
					break;
			}
		}
		drawScreen();
		g_system->delayMillis(10);
	}
	return -1;
}


bool BoyzEngine::shoot(const Common::Point &mousePos, ArcadeShooting *arc, bool secondary) {
	if (_currentMode == NonInteractive) {
		return false;
	}

	if (!secondary && _currentWeapon > 0) {
		if (_ammo == 0) {
			if (!arc->noAmmoSound.empty())
				playSound(_soundPath + arc->noAmmoSound, 1, arc->noAmmoSoundRate);
			return false;
		}
		if (!_infiniteAmmoCheat)
			_ammo--;
		playSound(_soundPath + _weaponShootSound[_currentWeapon], 1);
		incShotsFired();
	}
	int i = detectTarget(mousePos);
	if (i < 0) {
		missNoTarget(arc);
	} else {
		debugC(1, kHypnoDebugArcade, "Hit target %s", _shoots[i].name.c_str());
		if (_shoots[i].nonHostile && secondary) {
			playSound(_soundPath + _heySound[_currentActor], 1);

			if (_shoots[i].isAnimal) {
				playSound(_soundPath + _shoots[i].animalSound, 1);
				return false;
			}
			incFriendliesEncountered();

			if (!_shoots[i].additionalVideo.empty()) {
				incInfoReceived();
				_background->decoder->pauseVideo(true);
				MVideo video(_shoots[i].additionalVideo, Common::Point(0, 0), false, true, false);
				disableCursor();
				runIntro(video);
				defaultCursor();

				if (_shoots[i].waitForClickAfterInteraction > 0) {
					waitForUserClick(_shoots[i].waitForClickAfterInteraction);
					if (_shoots[i].name == "LILKID")
						_sceneState["GS_MINEMAP"] = true;
					else if (_shoots[i].name == "HO3") {
						_sceneState["GS_C5MAP"] = true;
					}
				}

				loadPalette(_currentPalette);
				_background->decoder->pauseVideo(false);
				// Skip the rest of the interaction
				if (_shoots[i].explosionFrames[0].start == uint32(-1))
					_skipLevel = true;
				else {
					_background->decoder->forceSeekToFrame(_shoots[i].explosionFrames[0].start + 3);
					_masks->decoder->forceSeekToFrame(_shoots[i].explosionFrames[0].start + 3);
					_shoots[i].destroyed = true;
					_shootsDestroyed[_shoots[i].name] = true;
					updateScreen(*_background);
					drawScreen();
					if (!_music.empty())
						playSound(_music, 0, arc->musicRate); // restore music
				}
			} else if (_shoots[i].interactionFrame > 0) {
				incInfoReceived();
				_background->decoder->forceSeekToFrame(_shoots[i].interactionFrame);
				_masks->decoder->forceSeekToFrame(_shoots[i].interactionFrame);
				_additionalVideo = new MVideo(arc->missBoss2Video, Common::Point(0, 0), true, false, false);
				playVideo(*_additionalVideo);
				//_shoots[i].lastFrame = _background->decoder->getFrameCount();
				_shoots[i].destroyed = true;
				_shootsDestroyed[_shoots[i].name] = true;
				updateScreen(*_background);
				drawScreen();
			}
			return false;
		} else if (_shoots[i].nonHostile && !secondary) {

			if (checkCup(_shoots[i].name))
				return false;

			incFriendliesEncountered();
			uint32 idx = _shoots[i].warningVideoIdx;
			idx = idx == 0 ? 2 : idx;
			Common::String filename = _warningVideosDay[idx];
			_civiliansShoot++;

			_background->decoder->pauseVideo(true);
			MVideo video(filename, Common::Point(0, 0), false, true, false);
			disableCursor();
			runIntro(video);
			// Should be currentPalette?
			loadPalette(arc->backgroundPalette);
			_background->decoder->pauseVideo(false);
			updateScreen(*_background);
			drawScreen();
			if (!_music.empty())
				playSound(_music, 0, arc->musicRate); // restore music

			hitPlayer();

			_background->decoder->forceSeekToFrame(_shoots[i].explosionFrames[0].start - 3);
			_masks->decoder->forceSeekToFrame(_shoots[i].explosionFrames[0].start - 3);


			if (_shoots[i].jumpToTimeAfterKilled == -1000) {
				ArcadeTransition at("", 0, "", 0, _shoots[i].explosionFrames[0].lastFrame() - 1);
				at.loseLevel = true;
				_transitions.push_front(at);
			}

			return false;
		} else if (!_shoots[i].nonHostile && secondary) {
			if (_shoots[i].interactionFrame > 0) {
				_background->decoder->forceSeekToFrame(_shoots[i].interactionFrame);
				_masks->decoder->forceSeekToFrame(_shoots[i].interactionFrame);
				_shoots[i].destroyed = true;

				updateScreen(*_background);
				drawScreen();
			}
			return false;
		}

		if (!_shoots[i].hitSound.empty())
			playSound(_soundPath + _shoots[i].hitSound, 1);

		incEnemyHits();
		if (!_shoots[i].deathSound.empty())
			playSound(_soundPath + _shoots[i].deathSound, 1);

		if (_shoots[i].playInteractionAudio) {
			incInfoReceived();
			_additionalVideo = new MVideo(arc->missBoss2Video, Common::Point(0, 0), true, false, false);
			playVideo(*_additionalVideo);
		}

		incTargetsDestroyed();
		incScore(_shoots[i].pointsToShoot);
		incBonus(_shoots[i].pointsToShoot);
		_shoots[i].destroyed = true;
		_objKillsCount[_objIdx] = _objKillsCount[_objIdx] + _shoots[i].objKillsCount;
		_shootsDestroyed[_shoots[i].name] = true;

		if (_shoots[i].name == "HELICOPTER") {
			_background->decoder->pauseVideo(true);
			MVideo video(arc->hitBoss2Video, Common::Point(0, 0), false, true, false);
			disableCursor();
			runIntro(video);
			_skipLevel = true;
			_skipNextVideo = true;
			return false;
		}

		_background->decoder->forceSeekToFrame(_shoots[i].explosionFrames[0].start - 3);
		_masks->decoder->forceSeekToFrame(_shoots[i].explosionFrames[0].start - 3);
		changeCursor(_crosshairsActive[_currentWeapon], _crosshairsPalette, true);

		if (_shoots[i].jumpToTimeAfterKilled > 0) {
			ArcadeTransition at("", 0, "", 0, _shoots[i].explosionFrames[0].lastFrame() - 1);
			at.jumpToTime = _shoots[i].jumpToTimeAfterKilled;
			_transitions.push_front(at);
		}
	}
	return false;
}

void BoyzEngine::missedTarget(Shoot *s, ArcadeShooting *arc) {
	debugC(1, kHypnoDebugArcade, "Missed target %s!", s->name.c_str());
	if (!s->checkIfDestroyed.empty()) {
		if (_shootsDestroyed.contains(s->checkIfDestroyed))
			return;  // Precondition was destroyed, so we ignore the missed shoot
	}

	if (s->nonHostile) {
		incFriendliesEncountered();
		_stats.targetsMissed--; // If the target was not hostile, it should *not* count as missed
	}

	if (s->name == "CAPTOR") {
		_background->decoder->pauseVideo(true);
		MVideo video(_warningHostage, Common::Point(0, 0), false, true, false);
		disableCursor();
		runIntro(video);
		hitPlayer();
		if (_health > 0)
			_skipLevel = true;
		return;
	} else if (s->name.hasPrefix("ALARM")) {
		if (s->missedAnimation != uint32(-1) && uint32(_background->decoder->getCurFrame()) > s->missedAnimation)
			return;
		_background->decoder->pauseVideo(true);
		MVideo video(_warningAlarmVideos.front(), Common::Point(0, 0), false, true, false);
		disableCursor();
		runIntro(video);
		_health = 0;
		return;
	} else if (s->direction > 0) {
		char selected = selectDirection();
		defaultCursor();

		if (selected == s->direction) {
			int missedAnimation = s->missedAnimation;
			debugC(1, kHypnoDebugArcade, "Jumping to: %d", missedAnimation);
			_background->decoder->forceSeekToFrame(missedAnimation);
			_masks->decoder->forceSeekToFrame(missedAnimation);
		} else {
			_background->decoder->forceSeekToFrame(s->explosionFrames[0].start - 3);
			_masks->decoder->forceSeekToFrame(s->explosionFrames[0].start - 3);

			if (s->jumpToTimeAfterKilled == -1000) {
				ArcadeTransition at("", 0, "", 0, s->explosionFrames[0].lastFrame() - 1);
				at.loseLevel = true;
				_transitions.push_front(at);
			}
		}
		return;
	}

	if (s->missedAnimation == 0) {
		return;
	} else if (s->missedAnimation == uint32(-1)) {
		debugC(1, kHypnoDebugArcade, "Jumping to end of level");
		_skipLevel = true;
	} else if (s->missedAnimation == uint32(-1000)) {
		_health = 0;
	} else {
		int missedAnimation = s->missedAnimation;
		if (missedAnimation + 3 > int(_background->decoder->getFrameCount()) - 1) {
			_skipLevel = true;
			return;
		}
		if (_background->decoder->getCurFrame() > missedAnimation)
			return; // Too late for this
		debugC(1, kHypnoDebugArcade, "Jumping to: %d", missedAnimation);
		_background->decoder->forceSeekToFrame(missedAnimation);
		_masks->decoder->forceSeekToFrame(missedAnimation);
	}
	if (!s->nonHostile)
		hitPlayer();
}

bool BoyzEngine::checkCup(const Common::String &name) {
	if (name == "CUP1") {
		if (_background->path == "c3/c35c01s.smk") {
			MVideo video("c3/c35c07s.smk", Common::Point(0, 0), false, true, false);
			disableCursor();
			runIntro(video);
			_skipLevel = true;
			_sceneState["GS_WONSHELLGAME"] = 1;
		} else {
			MVideo video("c3/c35c06s.smk", Common::Point(0, 0), false, true, false);
			disableCursor();
			runIntro(video);
			_health = 0;
		}
		return true;
	} else if (name == "CUP2") {
		if (_background->path == "c3/c35c02s.smk") {
			MVideo video("c3/c35c07s.smk", Common::Point(0, 0), false, true, false);
			disableCursor();
			runIntro(video);
			_skipLevel = true;
			_sceneState["GS_WONSHELLGAME"] = 1;
		} else {
			MVideo video("c3/c35c06s.smk", Common::Point(0, 0), false, true, false);
			disableCursor();
			runIntro(video);
			_health = 0;
		}
		return true;
	}
	return false;
}

bool BoyzEngine::clickedSecondaryShoot(const Common::Point &mousePos) {
	if (_currentMode == NonInteractive) {
		return false;
	}

	Common::Rect ammoBarBox(320 - _ammoBar[_currentActor].w, 0, 320, _ammoBar[_currentActor].h);
	if (ammoBarBox.contains(mousePos)) {
		_ammo = _weaponMaxAmmo[_currentWeapon];
		playSound(_soundPath + _weaponReloadSound[_currentWeapon], 1);
		return false;
	}
	return true;
}

} // namespace Hypno