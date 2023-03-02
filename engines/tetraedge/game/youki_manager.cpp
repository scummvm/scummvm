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

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/youki_manager.h"
#include "tetraedge/game/character.h"
#include "tetraedge/game/game.h"

namespace Tetraedge {

YoukiManager::YoukiManager() : _followKate(false), _allowUpdate(true),
	_isAssitAnim(false), _isWalking(false) {
}

void YoukiManager::reset() {
	_timer.start();
	_followKate = false;
	_allowUpdate = true;
	_isAssitAnim = false;
	_isWalking = false;
	Game *game = g_engine->getGame();
	Character *youki = game->scene().character("Youki");
	if (!youki)
		return;

	youki->_onCharacterAnimFinishedSignal.remove(this, &YoukiManager::onAnimFinished);
	youki->onFinished().remove(this, &YoukiManager::onMoveFinished);
	youki->deleteAllCallback();
	youki->stop();
	youki->setFreeMoveZone(nullptr);
}

void YoukiManager::update() {
	if (g_engine->gameType() != TetraedgeEngine::kSyberia2)
		return;

	Game *game = g_engine->getGame();
	Character *youki = game->scene().character("Youki");
	if (!youki)
		return;

	if (_timer.getTimeFromStart() <= 3000000.0 || !_allowUpdate)
		return;

	if (game->_movePlayerCharacterDisabled)
		return;

	Character *player = game->scene()._character;
	if (!player) {
		warning("YoukiManager::update: Couldn't get player");
		return;
	}

	_timer.stop();
	_timer.start();
	game->_sceneCharacterVisibleFromLoad = false;

	const TeVector3f32 youkipos = youki->_model->position();
	const TeVector3f32 playerpos = player->_model->position();
	const TeVector3f32 dirtoplayer = playerpos - youkipos;
	if (dirtoplayer.squaredLength() >= 4.0f) {
		TeVector3f32 destPos = playerpos - dirtoplayer.getNormalized() * 1.5f;
        TeIntrusivePtr<TeBezierCurve> curve = youki->freeMoveZone()->curve(youkipos, destPos);
        if (curve) {
			youki->setCurveStartLocation(TeVector3f32(0, 0, 0));
			youki->placeOnCurve(curve);
            youki->setCurveOffset(0);
			if (dirtoplayer.getSquareMagnitude() >= 25.0f) {
                youki->walkMode("Jog");
			} else {
                youki->walkMode("WalkComp");
			}
			youki->setAnimation(youki->walkAnim(Character::WalkPart_Loop), true);
			youki->walkTo(1.0f, false);
            _isWalking = true;
            youki->_onCharacterAnimFinishedSignal.remove(this, &YoukiManager::onAnimFinished);
            youki->_onCharacterAnimFinishedSignal.add(this, &YoukiManager::onAnimFinished);
            youki->onFinished().remove(this, &YoukiManager::onMoveFinished);
            youki->onFinished().add(this, &YoukiManager::onMoveFinished);
		}
	}
}

bool YoukiManager::onAnimFinished(const Common::String &anim) {
	if (!_followKate)
		return false;

	Game *game = g_engine->getGame();
	Character *youki = game->scene().character("Youki");
	Character *player = game->scene()._character;
	if (!youki || !player) {
		warning("YoukiManager::onAnimFinished: Couldn't get both Youki and player");
		return false;
	}

	const TeVector3f32 youkipos = youki->_model->position();
	const TeVector3f32 playerpos = player->_model->position();
	const TeVector3f32 dirtoplayer = playerpos - youkipos;
	float squareDistToPlayer = dirtoplayer.squaredLength();
	if (!_isAssitAnim) {
		int youkiAnimFrame = youki->_model->anim()->curFrame2();
		int youkiAnimLastFrame = youki->_model->anim()->lastFrame();
		if (youkiAnimFrame == youkiAnimLastFrame) {
			if (squareDistToPlayer <= 4.0) {
				int randVal = g_engine->getRandomNumber(9);
				if (randVal < 7) {
					youki->setAnimation("Youki/y_assit.te3da", false);
				} else {
					youki->setAnimation("Youki/y_gratte1.te3da", false);
				}
			} else {
				youki->setAnimation("Youki/y_assit_debout.te3da", false);
				_isAssitAnim = false;
			}
			return false;
		}
	}

	if (!_isAssitAnim && !_isWalking) {
		if (squareDistToPlayer <= 4.0) {
			int randVal = g_engine->getRandomNumber(9);
			if (randVal < 3) {
				youki->setAnimation("Youki/y_assit.te3da", false);
				_isAssitAnim = true;
			} else if (randVal < 4) {
				youki->setAnimation("Youki/y_gratte2.te3da", false);
			} else if (randVal < 7) {
				youki->setAnimation("Youki/y_sent2.te3da", false);
			} else {
				_allowUpdate = true;
				youki->setAnimation("Youki/y_att_debout.te3da", false);
			}
		} else {
			_allowUpdate = true;
			youki->setAnimation("Youki/y_att_debout.te3da", false);
		}
	}
	return false;
}

bool YoukiManager::onMoveFinished() {
	if (!_followKate)
		return false;

	Game *game = g_engine->getGame();
	Character *youki = game->scene().character("Youki");
	Character *player = game->scene()._character;
	if (!youki || !player) {
		warning("YoukiManager::onMoveFinished: Couldn't get both Youki and player");
		return false;
	}

	const TeVector3f32 youkipos = youki->_model->position();
	const TeVector3f32 playerpos = player->_model->position();
	const TeVector3f32 dirtoplayer = playerpos - youkipos;
	if (dirtoplayer.squaredLength() >= 4.0) {
        _isWalking = false;
        _allowUpdate = true;
        youki->setAnimation("Youki/y_att_debout.te3da", false);
        _timer.setTime(3000000.0);
	} else {
		int randVal = g_engine->getRandomNumber(9);
		_isWalking = false;
		if (randVal < 3) {
			_allowUpdate = true;
			youki->setAnimation("Youki/y_att_debout.te3da", false);
		} else if (randVal < 6) {
			_allowUpdate = false;
			youki->setAnimation("Youki/y_sent2.te3da", false);
		} else {
			_allowUpdate = false;
			youki->setAnimation("Youki/y_assit.te3da", false);
			_isAssitAnim = true;
        }
	}
	return false;
}

} // end namespace Tetraedge
