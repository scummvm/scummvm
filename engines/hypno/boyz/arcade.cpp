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
		_healthBar[i] = _playerFrames[i]->getSubArea(healthBarBox);
		_ammoBar[i] = _playerFrames[i]->getSubArea(ammoBarBox);
		_portrait[i] = _playerFrames[i]->getSubArea(portraitBox);
	}

	_playerFrameSep = _playerFrames.size();
	_playerFrameIdx = -1;

	_currentScript = arc->script;
	ScriptInfo si = *_currentScript.begin();
	_currentActor = si.actor - 1;
	_currentMode = si.mode;
	_currentScript.pop_front();
}

void BoyzEngine::runAfterArcade(ArcadeShooting *arc) {
	for (int i = 0; i < int(_playerFrames.size()); i++) {
		_playerFrames[i]->free();
		delete _playerFrames[i];
	}
}

void BoyzEngine::updateFromScript() {
	if (_currentScript.size() > 0) {
		ScriptInfo si = *_currentScript.begin();
		//debug("%d %d %d", si.time, _background->decoder->getCurFrame(), si.actor);
		if (int(si.time) <= _background->decoder->getCurFrame()) {
			_currentActor = si.actor - 1;
			_currentMode = si.mode;
			_currentScript.pop_front();
		}
	}
}

void BoyzEngine::drawPlayer() {
	updateFromScript();
	drawImage(_portrait[_currentActor], 0, 200 - _portrait[_currentActor].h, true);
}

void BoyzEngine::drawHealth() {
	updateFromScript();
	drawImage(_healthBar[_currentActor], 0, 0, true);
	drawImage(_ammoBar[_currentActor], 320 - _ammoBar[_currentActor].w, 0, true);
}
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

void BoyzEngine::findNextSegment(ArcadeShooting *arc) {
	_segmentIdx = _segmentIdx + 1;
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

void BoyzEngine::shoot(const Common::Point &mousePos, ArcadeShooting *arc, MVideo &background) {
	incShotsFired();
	int i = detectTarget(mousePos);
	if (i < 0) {
		missNoTarget(arc, background);
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
		background.decoder->forceSeekToFrame(_shoots[i].explosionFrames[0].start - 3);
		_masks->decoder->forceSeekToFrame(_shoots[i].explosionFrames[0].start - 3);
		_shoots.clear();
	}
}

void BoyzEngine::missedTarget(Shoot *s, ArcadeShooting *arc, MVideo &background) {

	if (s->missedAnimation == 0)
		return;
	else if (s->missedAnimation == uint32(-1)) {
		uint32 last = background.decoder->getFrameCount()-1;
		background.decoder->forceSeekToFrame(last);
		_masks->decoder->forceSeekToFrame(last);
		return;
	}

	s->missedAnimation = s->missedAnimation + 3;
	background.decoder->forceSeekToFrame(s->missedAnimation);
	_masks->decoder->forceSeekToFrame(s->missedAnimation);
}

} // namespace Hypno