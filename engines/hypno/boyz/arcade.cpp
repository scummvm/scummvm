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

	if (!arc->beforeVideo.empty()) {
		MVideo video(arc->beforeVideo, Common::Point(0, 0), false, true, false);
		runIntro(video);
	}

	_currentScript = arc->script;
	// Reload all weapons
	for (Script::iterator it = _currentScript.begin(); it != _currentScript.end(); ++it) {
		_ammoTeam[it->actor] = _weaponMaxAmmo[it->cursor];
	}

	updateFromScript();
}

void BoyzEngine::runAfterArcade(ArcadeShooting *arc) {
	for (int i = 0; i < int(_playerFrames.size()); i++) {
		_playerFrames[i]->free();
		delete _playerFrames[i];
	}
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
	drawImage(_portrait[_currentActor], 0, 200 - _portrait[_currentActor].h, true);
}

void BoyzEngine::drawHealth() {
	updateFromScript();

	float w = float(_healthTeam[_currentActor]) / float(_maxHealth);
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

	float w = float(_ammoBar[_currentWeapon].w) / float(_weaponMaxAmmo[_currentWeapon]);

	Common::Rect ammoBarBox(320 - int(_ammoTeam[_currentActor] * w), 0, 320, _ammoBar[_currentActor].h / 2);
	uint32 c = kHypnoColorGreen; // green
	_compositeSurface->fillRect(ammoBarBox, c);

	drawImage(_ammoBar[_currentActor], 320 - _ammoBar[_currentWeapon].w, 0, true);
	for (int i = 1; i < _weaponMaxAmmo[_currentWeapon]; i++) {
		int x = 320 - _ammoBar[_currentWeapon].w + int (i * w);
		_compositeSurface->drawLine(x, 2, x, 6, 0);
	}
}

void BoyzEngine::hitPlayer() {
	uint32 c = kHypnoColorRed; // red
	_compositeSurface->fillRect(Common::Rect(0, 0, _screenW, _screenH), c);
	drawScreen();
	if (!_infiniteHealthCheat)
		_healthTeam[_currentActor] = _healthTeam[_currentActor] - 10;
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
		if (!at.video.empty()) {
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
		} else if (!at.sound.empty()) {
			playSound(at.sound, 1);
		} else
			error ("Invalid transition at %d", ttime);

		transitions.pop_front();
		if (!_music.empty())
			playSound(_music, 0, arc->musicRate); // restore music
		return true;
	}
	return false;
}

int BoyzEngine::detectTarget(const Common::Point &mousePos) {
	Common::Point target = computeTargetPosition(mousePos);
	assert(_shoots.size() <= 1);
	for (Shoots::iterator it = _shoots.begin(); it != _shoots.end(); ++it) {
		if (_mask->getPixel(target.x, target.y) == 1)
			return 0;
	}
	return -1;
}

void BoyzEngine::shoot(const Common::Point &mousePos, ArcadeShooting *arc) {
	if (_currentMode == NonInteractive) {
		return;
	}

	if (_ammoTeam[_currentActor] == 0)
		return; // TODO: out of ammo sound is missing
	if (!_infiniteAmmoCheat)
		_ammoTeam[_currentActor]--;
	playSound(_soundPath + _weaponShootSound[_currentWeapon], 1);
	incShotsFired();
	int i = detectTarget(mousePos);
	if (i < 0) {
		missNoTarget(arc);
	} else {
		if (!_shoots[i].hitSound.empty())
			playSound(_soundPath + _shoots[i].hitSound, 1);

		incEnemyHits();
		if (!_shoots[i].deathSound.empty())
			playSound(_soundPath + _shoots[i].deathSound, 1);

		incTargetsDestroyed();
		incScore(_shoots[i].pointsToShoot);
		incBonus(_shoots[i].pointsToShoot);
		_shoots[i].destroyed = true;
		_background->decoder->forceSeekToFrame(_shoots[i].explosionFrames[0].start - 3);
		_masks->decoder->forceSeekToFrame(_shoots[i].explosionFrames[0].start - 3);
		_shoots.clear();
		changeCursor(_crosshairsActive[_currentWeapon], _crosshairsPalette, true);
	}
}

void BoyzEngine::missedTarget(Shoot *s, ArcadeShooting *arc) {
	hitPlayer();
	if (s->missedAnimation == 0)
		return;
	else if (s->missedAnimation == uint32(-1)) {
		uint32 last = _background->decoder->getFrameCount()-1;
		_background->decoder->forceSeekToFrame(last);
		_masks->decoder->forceSeekToFrame(last);
		return;
	}

	s->missedAnimation = s->missedAnimation + 3;
	_background->decoder->forceSeekToFrame(s->missedAnimation);
	_masks->decoder->forceSeekToFrame(s->missedAnimation);
}

} // namespace Hypno