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
#include "mohawk/zoombini_pages/easteregg_sb.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"

namespace Mohawk {

const Common::Point ZoombiniEasterEggSB::kDirections[8] = {
	Common::Point(0, -1),
	Common::Point(1, -1),
	Common::Point(1, 0),
	Common::Point(1, 1),
	Common::Point(0, 1),
	Common::Point(-1, 1),
	Common::Point(-1, 0),
	Common::Point(-1, -1),
};

ZoombiniEasterEggSB::ZoombiniEasterEggSB(MohawkEngine_Zoombini *vm)
	: ZoombiniInteractive(vm, ZoombiniPageType::kEasterEggSB) {
}

void ZoombiniEasterEggSB::open() {
	openArchive(ZMB_MHK_PICKER);
}

void ZoombiniEasterEggSB::setBackgroundMusic() {
	if (!_vm->getRelInfo().isFamily(ZmbReleaseInfo::Family::kTlcV20))
		_vm->_midi->playMidi(ZmbResource(ZmbResource::kSystem, kSysResMidi30035_MazeBgmBase));
}

void ZoombiniEasterEggSB::setBackgroundBitmap() {
	_vm->_gfx->setPalette(kResBackground2000);
	_vm->_gfx->fillArea(ZoombiniGraphics::kBackScreen, 44);
}

const ZoombiniPage::ScriptSoundPriorityRanges &ZoombiniEasterEggSB::getScriptSoundPriorityRanges() const {
	static const ScriptSoundPriorityRanges kRanges = {
		{kResSound3000_ObstacleHit, kResSound3002_Fire},
	};
	return kRanges;
}

void ZoombiniEasterEggSB::loadFeatures() {
	loadShapeOffsetRegs(ZmbResource::kPage, kResBitmapShape1000_Game);
	setRunnerZSortEnabled(false);
	resetShip();

	ZmbResource shapeRes = ZmbResource(ZmbResource::kPage, kResBitmapShape1000_Game);

	ZmbFeature *scorePanelFeature = loadScrbFeature(shapeRes, kResScrb1012_ScorePanel, 6, ZmbFeature::FLAG_00000000_TYPE_SHAPES);
	scorePanelFeature->setShapeOffsetRegs(_shapeOffsetRegsMap[kResBitmapShape1000_Game]);

	ZmbFeature::EventHooks scoreHooks;
	scoreHooks.setPreRenderShapeFunc(&ZoombiniEasterEggSB::updateScoreDigits);
	_scoreDigitsFeature = loadScrbFeature(shapeRes, kResScrb1014_ScoreDigits, 6, ZmbFeature::FLAG_00100000_PLAY_ONCE, scoreHooks);

	refreshScore();
}

ZoombiniEasterEggSB::Direction ZoombiniEasterEggSB::rotateDirection(Direction direction, int16 steps) {
	const uint16 index = static_cast<uint16>(direction + steps);
	return static_cast<Direction>(index & kDirectionMask07);
}

void ZoombiniEasterEggSB::resetShip() {
	if (_ship.motion.feature)
		unloadScrbFeature(_ship.motion.feature);

	_ship = Ship();
	_ship.motion.position = Common::Point(320, 240);
	_ship.motion.shape = 17;
	_ship.motion.feature = loadMovingFeature(kResScrb1010_Ship, 4);
}

ZmbFeature *ZoombiniEasterEggSB::loadMovingFeature(int16 script, uint32 interval) {
	ZmbFeature::EventHooks hooks;
	hooks.setPreRenderFunc(&ZoombiniEasterEggSB::tickMovingFeature);
	hooks.setPreRenderShapeFunc(&ZoombiniEasterEggSB::positionMovingShape);
	ZmbFeature *feature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1000_Game), script, interval, ZmbFeature::FLAG_00000000_TYPE_SHAPES, hooks);
	feature->setShapeOffsetRegs(_shapeOffsetRegsMap[kResBitmapShape1000_Game]);
	return feature;
}

void ZoombiniEasterEggSB::restartGame() {
	if (_gameOverFeature) {
		if (_specialObstacleFeature) {
			for (Obstacle &obstacle : _obstacles) {
				if (obstacle.motion.feature == _specialObstacleFeature) {
					unloadScrbFeature(_specialObstacleFeature);
					obstacle = Obstacle();
					_obstacleCount -= 1;
					break;
				}
			}
			if (_explosionFeature == _specialObstacleFeature) {
				unloadScrbFeature(_explosionFeature);
				_explosionFeature = nullptr;
				_explosionFinished = false;
			}
			_specialObstacleFeature = nullptr;
		}
		unloadScrbFeature(_gameOverFeature);
		_gameOverFeature = nullptr;
		_score = 0;
		_lives = 3;
		_nextBonusScore = 100;
		_alternateWave = false;
		refreshScore();
		resetShip();
	} else if (_ship.deathFrame) {
		resetShip();
	}
}

void ZoombiniEasterEggSB::onMapButtonActivated() {
	_vm->setNextPage(ZoombiniPageType::kRodMap);
	close();
}

ZmbEventHandleResult ZoombiniEasterEggSB::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	onMapButtonActivated();
	return ZmbEventHandleResult::kConsumed;
}

ZmbEventHandleResult ZoombiniEasterEggSB::onKeyDown(const Common::KeyState &kbd, bool kbdRepeat) {
	if (kbd.keycode == Common::KEYCODE_SPACE) {
		restartGame();
	} else if (!_ship.deathFrame) {
		switch (kbd.ascii) {
		case '4': // Turn Left
			_ship.direction = rotateDirection(_ship.direction, -1);
			break;
		case '5': // Fire
			fireProjectile();
			break;
		case '6': // Turn Right
			_ship.direction = rotateDirection(_ship.direction, 1);
			break;
		case '8': { // Add velocity
			const Common::Point &step = kDirections[_ship.direction];
			_ship.motion.velocity.x = CLIP<int16>(_ship.motion.velocity.x + 4 * step.x, -12, 12);
			_ship.motion.velocity.y = CLIP<int16>(_ship.motion.velocity.y + 4 * step.y, -12, 12);
			break;
		}
		default:
			break;
		}
	}
	return ZoombiniInteractive::onKeyDown(kbd, kbdRepeat);
}

void ZoombiniEasterEggSB::fireProjectile() {
	for (Projectile &projectile : _projectiles) {
		if (projectile.motion.feature)
			continue;

		const Common::Point &heading = kDirections[_ship.direction];
		projectile.motion.velocity = Common::Point(14 * heading.x, 14 * heading.y);
		projectile.motion.position = _ship.motion.position + projectile.motion.velocity;
		projectile.motion.shape = 25;
		projectile.motion.feature = loadMovingFeature(kResScrb1011_Projectile, 3);
		queueScriptSoundForNextRenderPass(ZmbResource(ZmbResource::kPage, kResSound3002_Fire));
		return;
	}
}

ZmbFeature *ZoombiniEasterEggSB::spawnObstacle(int16 baseScript, const Common::Point *splitPosition, Direction splitDirection) {
	for (Obstacle &obstacle : _obstacles) {
		Motion &motion = obstacle.motion;
		if (motion.feature)
			continue;

		const int16 speedRoll = _vm->_rnd->getRandomNumber(1, 10);
		int16 speed = 12;
		if (speedRoll <= 2)
			speed = 8;
		else if (9 <= speedRoll)
			speed = 16;
		Direction direction = static_cast<Direction>(_vm->_rnd->getRandomNumber(kDirection00_North, kDirection07_Northwest));
		const int16 script = baseScript + _vm->_rnd->getRandomNumber(0, 4);
		if (baseScript == kResScrb1021_LeftObstacleBase) {
			direction = kDirection06_West;
			speed = 20;
		} else if (baseScript == kResScrb1026_RightObstacleBase) {
			direction = kDirection02_East;
			speed = 20;
		}

		if (splitPosition) {
			motion.position = *splitPosition;
			direction = splitDirection;
		} else if (direction == kDirection00_North || direction == kDirection04_South) {
			motion.position = Common::Point(_vm->_rnd->getRandomNumber(20, 620), -10);
		} else {
			motion.position = Common::Point(-10, _vm->_rnd->getRandomNumber(20, 460));
		}
		obstacle.direction = direction;
		motion.velocity = Common::Point(kDirections[direction].x * speed, kDirections[direction].y * speed);

		motion.feature = loadMovingFeature(script, 5);
		const ZmbScriptDecoder::DecodedScrb *decoded = getDecodedScrb(ZmbResource(ZmbResource::kPage, script));
		if (decoded->frames.empty() || decoded->frames[0].hotspots.empty())
			error("sb: obstacle SCRB %d has no initial shape", script);
		motion.shape = decoded->frames[0].hotspots[0]._shapeIdx;
		_obstacleCount += 1;
		return motion.feature;
	}
	return nullptr;
}

void ZoombiniEasterEggSB::onPostRenderFrame() {
	if (!_obstacleCount) {
		spawnObstacle(kResScrb1016_LargeObstacleBase);
		if (_vm->_rnd->getRandomNumber(1, 10) <= 4)
			spawnObstacle(kResScrb1016_LargeObstacleBase);
	}
	if (_expiredProjectile) {
		unloadScrbFeature(_expiredProjectile->motion.feature);
		*_expiredProjectile = Projectile();
		_expiredProjectile = nullptr;
	}
	if (_explosionFinished) {
		if (_explosionFeature == _specialObstacleFeature) {
			_alternateWave = false;
			_specialObstacleFeature = nullptr;
		}
		unloadScrbFeature(_explosionFeature);
		_explosionFeature = nullptr;
		_explosionFinished = false;
	} else if (!_explosionFeature && 0 <= _hitObstacleSlot) {
		splitHitObstacle();
	}
}

void ZoombiniEasterEggSB::splitHitObstacle() {
	Obstacle &obstacle = _obstacles[_hitObstacleSlot];
	ZmbFeature *feature = obstacle.motion.feature;
	const int16 script = feature->getScrbId();
	int16 fragmentBase = 0;
	if (script < kResScrb1005_SmallObstacleBase)
		fragmentBase = kResScrb1005_SmallObstacleBase;
	else if (kResScrb1016_LargeObstacleBase <= script && script < kResScrb1021_LeftObstacleBase)
		fragmentBase = kResScrb1000_MediumObstacleBase;

	if (fragmentBase) {
		const int16 spread = _vm->_rnd->getRandomNumber(1, 100) <= 33 ? 2 : 1;
		const Common::Point &position = obstacle.motion.position;
		const Direction direction = obstacle.direction;
		spawnObstacle(fragmentBase, &position, rotateDirection(direction, spread));
		spawnObstacle(fragmentBase, &position, rotateDirection(direction, -spread));
		if (fragmentBase == kResScrb1005_SmallObstacleBase && _vm->_rnd->getRandomNumber(1, 100) <= 33)
			spawnObstacle(fragmentBase, &position, rotateDirection(direction, 4));
	}

	feature->setFlags(ZmbFeature::FLAG_00100000_PLAY_ONCE);
	feature->setEventHooks(ZmbFeature::EventHooks());
	loadScrbOntoFeature(feature, kResScrb1015_Explosion);
	_explosionFeature = feature;
	obstacle = Obstacle();
	_hitObstacleSlot = -1;
	_obstacleCount -= 1;
	if (!_obstacleCount) {
		_alternateWave = !_alternateWave;
		if (_alternateWave) {
			const int16 base = _vm->_rnd->getRandomNumber(1, 10) <= 5 ? kResScrb1021_LeftObstacleBase : kResScrb1026_RightObstacleBase;
			_specialObstacleFeature = spawnObstacle(base);
		}
	}
}

bool ZoombiniEasterEggSB::beginMotionStep(Motion &object) const {
	const uint32 frame = getCurrentFrameCounter();
	if (frame < object.nextStep)
		return false;
	object.nextStep = frame + object.feature->getFrameInterval();
	object.feature->setNeedsRedraw(true);
	return true;
}

void ZoombiniEasterEggSB::advanceMotion(Motion &motion) {
	motion.position += motion.velocity;
	if (650 < motion.position.x)
		motion.position.x = -10;
	else if (motion.position.x < -10)
		motion.position.x = 650;
	if (490 < motion.position.y)
		motion.position.y = -10;
	else if (motion.position.y < -10)
		motion.position.y = 490;
}

int16 ZoombiniEasterEggSB::findObstacleHit(const ZmbFeature *feature) const {
	const Common::Rect hitRect = feature->getClickRect();
	if (hitRect.isEmpty())
		return -1;
	for (uint16 i = 0; i < ARRAYSIZE(_obstacles); i++) {
		const ZmbFeature *obstacle = _obstacles[i].motion.feature;
		if (obstacle && !obstacle->getClickRect().isEmpty() && hitRect.intersects(obstacle->getClickRect()))
			return i;
	}
	return -1;
}

bool ZoombiniEasterEggSB::tickMovingFeature(ZmbFeature *feature) {
	if (feature == _ship.motion.feature) {
		if (beginMotionStep(_ship.motion))
			tickShip();
		return true;
	}
	for (Obstacle &obstacle : _obstacles) {
		if (obstacle.motion.feature == feature) {
			if (beginMotionStep(obstacle.motion))
				advanceMotion(obstacle.motion);
			return true;
		}
	}
	for (Projectile &projectile : _projectiles) {
		if (projectile.motion.feature == feature) {
			if (beginMotionStep(projectile.motion))
				tickProjectile(projectile);
			return true;
		}
	}
	return true;
}

void ZoombiniEasterEggSB::tickShip() {
	if (_ship.deathFrame) {
		if (4 < _ship.deathFrame) {
			_ship.motion.shape = 0;
			return;
		}
		advanceMotion(_ship.motion);
		_ship.motion.shape = 10 + _ship.deathFrame;
		_ship.deathFrame += 1;
		return;
	}

	if (0 <= findObstacleHit(_ship.motion.feature)) {
		_ship.deathFrame = 1;
		_lives -= 1;
		queueScriptSoundForNextRenderPass(ZmbResource(ZmbResource::kPage, kResSound3001_ShipHit));
		if (!_lives) {
			_gameOverFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, kResBitmapShape1000_Game), kResScrb1013_GameOver, 6, ZmbFeature::FLAG_00000000_TYPE_SHAPES);
			_gameOverFeature->setShapeOffsetRegs(_shapeOffsetRegsMap[kResBitmapShape1000_Game]);
		}
		refreshScore();
	}
	advanceMotion(_ship.motion);
	_ship.motion.shape = 17 + _ship.direction;
}

void ZoombiniEasterEggSB::tickProjectile(Projectile &projectile) {
	projectile.age += 1;
	if (15 < projectile.age) {
		projectile.motion.shape = 0;
		if (!_expiredProjectile)
			_expiredProjectile = &projectile;
		return;
	}
	advanceMotion(projectile.motion);
	if (13 <= projectile.age) {
		projectile.motion.shape = 12 + projectile.age;
		projectile.age += 1;
		return;
	}
	if (_explosionFeature || 0 <= _hitObstacleSlot)
		return;

	const int16 slot = findObstacleHit(projectile.motion.feature);
	if (slot < 0)
		return;
	_hitObstacleSlot = slot;
	projectile.age = 12;
	queueScriptSoundForNextRenderPass(ZmbResource(ZmbResource::kPage, kResSound3000_ObstacleHit));
	const int16 script = _obstacles[slot].motion.feature->getScrbId();
	if (script < kResScrb1005_SmallObstacleBase)
		_score += 5;
	else if (script < kResScrb1016_LargeObstacleBase)
		_score += 10;
	else if (script < kResScrb1021_LeftObstacleBase)
		_score += 1;
	else
		_score += 15;
	if (_nextBonusScore <= _score) {
		_nextBonusScore += 100;
		if (_lives < 9)
			_lives += 1;
	}
	refreshScore();
}

const ZoombiniEasterEggSB::Motion *ZoombiniEasterEggSB::getMotion(const ZmbFeature *feature) const {
	if (feature == _ship.motion.feature)
		return &_ship.motion;
	for (const Obstacle &obstacle : _obstacles) {
		if (obstacle.motion.feature == feature)
			return &obstacle.motion;
	}
	for (const Projectile &projectile : _projectiles) {
		if (projectile.motion.feature == feature)
			return &projectile.motion;
	}
	return nullptr;
}

void ZoombiniEasterEggSB::positionMovingShape(ZmbFeature *feature, ZmbHotspotGroup *group, Common::Array<ZmbHotspot> &hotspots) {
	const Motion *motion = getMotion(feature);
	if (!motion || hotspots.empty())
		return;
	hotspots[0]._shapeIdx = motion->shape;
	hotspots[0]._x = motion->position.x;
	hotspots[0]._y = motion->position.y;

	if (motion->shape == ZmbHotspot::kShapeNone)
		return;

	// Collision uses the full sprite bounds, including the space beyond the screen edges where objects wrap.
	const Common::Point origin = motion->position - feature->getShapeOffsetRegs()->getShapeDelta(motion->shape);
	const Common::Rect shapeSize = _vm->_gfx->getShapeSize(feature->getResource(), motion->shape);
	feature->setClickRect(Common::Rect(origin, shapeSize.width(), shapeSize.height()));
}

void ZoombiniEasterEggSB::onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) {
	if (feature == _explosionFeature && eventCode == kAnimEventM1_End)
		_explosionFinished = true;
}

void ZoombiniEasterEggSB::refreshScore() {
	if (999 < _score)
		_score = 0;
	ZmbStateFile &state = _vm->_state->getCurrentState();
	if (state._mudballHighScore < _score) {
		state._mudballHighScore = _score;
		_vm->_state->markSaveBeforeQuitPending();
	}
	loadScrbOntoFeature(_scoreDigitsFeature, kResScrb1014_ScoreDigits);
}

void ZoombiniEasterEggSB::updateScoreDigits(ZmbFeature *feature, ZmbHotspotGroup *group, Common::Array<ZmbHotspot> &hotspots) {
	if (hotspots.size() < 7)
		return;
	const uint16 best = MIN<uint16>(999, _vm->_state->getCurrentState()._mudballHighScore);
	const int16 zero = hotspots[0]._shapeIdx;
	hotspots[0]._shapeIdx = zero + best / 100;
	hotspots[1]._shapeIdx = zero + best / 10 % 10;
	hotspots[2]._shapeIdx = zero + best % 10;
	hotspots[3]._shapeIdx = zero + _score / 100;
	hotspots[4]._shapeIdx = zero + _score / 10 % 10;
	hotspots[5]._shapeIdx = zero + _score % 10;
	hotspots[6]._shapeIdx = zero + _lives;
}

} // End of namespace Mohawk
