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

#include "ngi/ngi.h"

#include "ngi/objects.h"
#include "ngi/behavior.h"
#include "ngi/statics.h"
#include "ngi/messages.h"

namespace NGI {

BehaviorController::BehaviorController() {
	_scene = nullptr;
	_isActive = true;
}

BehaviorController::~BehaviorController() {
	clear();
}

void BehaviorController::clear() {
	_behaviors.clear();
	_sounds.clear();
	_music.clear();
}

void BehaviorController::addSound(const BehaviorSound &sound, bool music) {
	(music ? _music : _sounds).push_back(sound);
}

void BehaviorController::enableAllMusic() {
	for (BehaviorSound &track : _music)
		track._enabled = true;
}

BehaviorAni &BehaviorController::addAni(int16 objectId, int16 sceneId, int flags) {
	_behaviors.push_back(BehaviorAni());
	BehaviorAni &behavior = _behaviors.back();
	behavior._objectId = objectId;
	behavior._sourceSceneId = sceneId;
	behavior._flags = flags;
	return behavior;
}

BehaviorMove &BehaviorAni::addMove(int16 staticsId) {
	// Keep XML order, including repeated groups for the same pose.
	if (_behaviorAnims.empty() || _behaviorAnims.back()._staticsId != staticsId) {
		_behaviorAnims.push_back(BehaviorAnim());
		_behaviorAnims.back()._staticsId = staticsId;
	}
	Common::Array<BehaviorMove> &moves = _behaviorAnims.back()._behaviorMoves;
	moves.push_back(BehaviorMove());
	return moves.back();
}

void BehaviorController::onShow(Scene &scene) {
	for (BehaviorAni &behavior : _behaviors) {
		AniObject *ani = scene.findAniObjectById(behavior._objectId, 0);
		if (ani && ani->_sceneId != behavior._sourceSceneId)
			ani = nullptr;
		if (behavior._ani != ani) {
			behavior._ani = ani;
			behavior._activeMove = nullptr;
			behavior._elapsed = 0;
		}
	}
	for (int i = 0; i < g_nmi->_currSoundListCount; ++i) {
		SoundList &list = *g_nmi->_currSoundList1[i];
		for (int j = 0; j < list.getCount(); ++j) {
			Sound &sound = list.getSoundByIndex(j);
			if (sound.getFlags() != Sound::kMusic || !sound.isPlaying())
				continue;
			for (const BehaviorSound &track : _music)
				if (track._soundId == sound.getId() && track._enabled)
					return;
			sound.stop();
			return;
		}
	}
}

bool BehaviorAni::updateIdle(uint32 elapsed) {
	if (!_ani)
		return false;
	if (_activeMove) {
		if (_activeMove->_movementId) {
			if (_ani->_movement && _ani->_movement->_id == _activeMove->_movementId)
				return false;
		} else {
			for (const MessageQueue *queue : *g_nmi->_globalMessageQueueList)
				if (queue->_queueId == _activeMove->_queueId && !queue->_isFinished)
					return false;
		}
		_activeMove = nullptr;
		return false;
	}
	if (_ani->_movement || !_ani->_statics || !(_ani->_flags & 4) || _ani->_messageQueueId)
		return false;
	if (_flags & 1) {
		for (const MessageQueue *queue : *g_nmi->_globalMessageQueueList)
			if (!queue->_isFinished && !(queue->_flags & kBehaviorQueue))
				return false;
	}

	_elapsed += elapsed;
	for (const BehaviorAnim &anim : _behaviorAnims) {
		if (anim._staticsId != _ani->_statics->_staticsId)
			continue;
		for (const BehaviorMove &move : anim._behaviorMoves) {
			if ((move._flags & 1) || move._mirror != ((_ani->_flags & 8) != 0) ||
					_elapsed < (uint32)move._delay || !move._percent || g_nmi->_rnd.getRandomNumber(99) > move._percent)
				continue;
			const bool started = move._movementId ? _ani->startMovement(move._movementId, 0, -1) :
					runQueueByIdWithAni(_ani, move._queueId, kBehaviorQueue);
			if (started) {
				_elapsed = 0;
				_activeMove = &move;
				return true;
			}
		}
	}
	return false;
}

bool BehaviorController::update(uint32 elapsed) {
	bool changed = false;
	for (BehaviorAni &behavior : _behaviors)
		changed |= behavior.updateIdle(elapsed);
	updateSounds(elapsed);
	return changed;
}

bool BehaviorSound::update(uint32 elapsed) {
	Sound *sound = g_nmi->findSound(_soundId);
	if (!sound)
		return false;
	if (sound->isPlaying())
		return true;
	_elapsed += elapsed;
	// The original roll is inclusive, so zero still matches one roll out of 100.
	if (!_enabled || _elapsed < _delay || g_nmi->_rnd.getRandomNumber(99) > _percent)
		return false;
	_elapsed = 0;
	sound->play(_loop);
	return sound->isPlaying();
}

void BehaviorController::updateSounds(uint32 elapsed) {
	for (BehaviorSound &sound : _sounds)
		sound.update(elapsed);

	bool musicPlaying = false;
	for (const BehaviorSound &track : _music) {
		Sound *sound = g_nmi->findSound(track._soundId);
		if (sound && sound->isPlaying()) {
			musicPlaying = true;
			break;
		}
	}
	for (BehaviorSound &track : _music) {
		if (musicPlaying)
			track._elapsed = 0;
		else
			musicPlaying = track.update(elapsed);
	}
}

void BehaviorController::initBehavior(Scene *sc, GameVar *var) {
	debugC(2, kDebugBehavior, "BehaviorController::initBehavior(%d, %s)", sc->_sceneId, transCyrillic(var->_varName));

	clear();
	_scene = sc;

	GameVar *behvar = var->getSubVarByName("BEHAVIOR");
	if (!behvar)
		return;

	debugC(3, kDebugBehavior, "BehaviorController::initBehavior. have Variable");

	for (GameVar *subvar = behvar->_subVars; subvar; subvar = subvar->_nextVarObj) {
		debugC(3, kDebugBehavior, "BehaviorController::initBehavior. subVar %s", transCyrillic(subvar->_varName));
		if (subvar->_varName == "AMBIENT") {
			_behaviors.push_back(BehaviorAni());
			_behaviors.back().initAmbientBehavior(subvar, sc);
		} else {
			AniObject *ani = sc->getStaticANIObject1ByName(subvar->_varName, -1);
			if (ani) {
				for (uint i = 0; i < sc->_aniObjects.size(); i++) {
					if (sc->_aniObjects[i]->_id == ani->_id) {
						_behaviors.push_back(BehaviorAni());
						BehaviorAni &behinfo = _behaviors.back();
						behinfo.initObjectBehavior(subvar, sc, ani);
						behinfo._ani = sc->_aniObjects[i];
					}
				}
			}
		}
	}
}

void BehaviorController::update() {
	if (!_isActive)
		return;

	debugC(6, kDebugBehavior, "BehaviorController::update()");
	for (uint i = 0; i < _behaviors.size(); i++) {
		BehaviorAni &beh = _behaviors[i];

		if (!beh._ani) {
			beh._counter++;
			if (beh._counter >= beh._counterMax)
				updateBehavior(beh, beh._behaviorAnims[0]);

			continue;
		}

		if (beh._ani->_movement || !(beh._ani->_flags & 4) || (beh._ani->_flags & 2)) {
			beh._staticsId = 0;
			continue;
		}

		if (beh._ani->_statics->_staticsId == beh._staticsId) {
			beh._counter++;
			if (beh._counter >= beh._counterMax) {
				if (beh._subIndex >= 0 && !(beh._flags & 1) && beh._ani->_messageQueueId <= 0) {
					assert(beh._ani);
					updateStaticAniBehavior(*beh._ani, beh._counter, beh._behaviorAnims[beh._subIndex]);
				}
			}
		} else {
			beh._staticsId = beh._ani->_statics->_staticsId;
			beh._counter = 0;
			beh._subIndex = -1;

			for (int j = 0; j < beh._animsCount; j++)
				if (beh._behaviorAnims[j]._staticsId == beh._staticsId) {
					beh._subIndex = j;
					break;
				}

		}
	}
}

void BehaviorController::updateBehavior(BehaviorAni &behaviorInfo, BehaviorAnim &entry) {
	debugC(7, kDebugBehavior, "BehaviorController::updateBehavior() moves: %d", entry._behaviorMoves.size());
	for (uint i = 0; i < entry._behaviorMoves.size(); i++) {
		BehaviorMove &bhi = entry._behaviorMoves[i];
		if (!(bhi._flags & 1)) {
			if (bhi._flags & 2) {
				MessageQueue *mq = new MessageQueue(bhi._messageQueue, 0, 1);

				mq->next();

				bhi._flags &= 0xFFFFFFFD;
			} else if (behaviorInfo._counter >= bhi._delay && bhi._percent && g_nmi->_rnd.getRandomNumber(32767) <= entry._behaviorMoves[i]._percent) {
				MessageQueue *mq = new MessageQueue(bhi._messageQueue, 0, 1);

				mq->next();

				behaviorInfo._counter = 0;
			}
		}
	}
}

void BehaviorController::updateStaticAniBehavior(AniObject &ani, int delay, const BehaviorAnim &beh) {
	debugC(6, kDebugBehavior, "BehaviorController::updateStaticAniBehavior(%s)", transCyrillic(ani._title));

	MessageQueue *mq = nullptr;

	if (beh._flags & 1) {
		uint rnd = g_nmi->_rnd.getRandomNumber(32767);
		uint runPercent = 0;
		for (uint i = 0; i < beh._behaviorMoves.size(); i++) {
			if (!(beh._behaviorMoves[i]._flags & 1) && beh._behaviorMoves[i]._percent) {
				if ((rnd >= runPercent && rnd <= runPercent + beh._behaviorMoves[i]._percent) || i == beh._behaviorMoves.size() - 1) {
					mq = new MessageQueue(beh._behaviorMoves[i]._messageQueue, 0, 1);
					break;
				}
				runPercent += beh._behaviorMoves[i]._percent;
			}
		}
	} else {
		for (uint i = 0; i < beh._behaviorMoves.size(); i++) {
			if (!(beh._behaviorMoves[i]._flags & 1) && delay >= beh._behaviorMoves[i]._delay) {
				if (beh._behaviorMoves[i]._percent) {
					if (g_nmi->_rnd.getRandomNumber(32767) <= beh._behaviorMoves[i]._percent) {
						mq = new MessageQueue(beh._behaviorMoves[i]._messageQueue, 0, 1);
						break;
					}
				}
			}
		}
	}

	if (mq) {
		mq->setObjectCopy(-1, ani._copyId);
		if (!mq->run(&ani)) {
			g_nmi->_globalMessageQueueList->deleteQueueById(mq->_id);
		}
	}
}

bool BehaviorController::enableAni(AniObject *obj, int aniId, int quId, int flag) {
	BehaviorMove *entry = getAniQueue(obj, aniId, quId);

	if (entry) {
		if (flag)
			entry->_flags &= 0xFFFFFFFE;
		else
			entry->_flags |= 1;
	} else
		return false;

	return true;
}

void BehaviorController::enableAllQueuesForAni(AniObject *ani, int flag) {
	for (uint i = 0; i < _behaviors.size(); i++) {
		BehaviorAni &beh = _behaviors[i];

		if (ani == beh._ani) {
			if (flag)
				beh._flags &= 0xfffffffe;
			else
				beh._flags |= 1;
		}
	}
}

BehaviorMove *BehaviorController::getAniQueue(AniObject *ani, int id1, int id2) {
	for (uint i = 0; i < _behaviors.size(); i++) {
		if (_behaviors[i]._ani == ani) {
			for (uint j = 0; j < _behaviors[i]._behaviorAnims.size(); j++) {
				if (_behaviors[i]._behaviorAnims[j]._staticsId == id1) {
					for (uint k = 0; k < _behaviors[i]._behaviorAnims[j]._behaviorMoves.size(); k++) {
						BehaviorMove &move = _behaviors[i]._behaviorAnims[j]._behaviorMoves[k];
						if (move._queueId && move._queueId == id2)
							return &move;
					}
				}
			}
		}
	}

	return nullptr;
}

void BehaviorAni::clear() {
	_ani = nullptr;
	_staticsId = 0;
	_counter = 0;
	_counterMax = 0;
	_flags = 0;
	_subIndex = 0;
	_animsCount = 0;
	_elapsed = 0;
	_activeMove = nullptr;
	_behaviorAnims.clear();
}

void BehaviorAni::initAmbientBehavior(GameVar *var, Scene *sc) {
	debugC(4, kDebugBehavior, "BehaviorAni::initAmbientBehavior(%s)", transCyrillic(var->_varName));

	clear();
	_animsCount = 1;
	_counterMax = -1;

	_behaviorAnims.push_back(BehaviorAnim());
	BehaviorAnim &bi = _behaviorAnims.back();

	int movesCount = var->getSubVarsCount();
	bi._behaviorMoves.reserve(movesCount);
	for (int i = 0; i < movesCount; i++) {
		int delay;
		bi._behaviorMoves.push_back(BehaviorMove(var->getSubVarByIndex(i), sc, &delay));
		BehaviorMove &move = bi._behaviorMoves.back();

		if (move._delay < _counterMax)
			_counterMax = move._delay;
	}
}

void BehaviorAni::initObjectBehavior(GameVar *var, Scene *sc, AniObject *ani) {
	Common::String s((char *)transCyrillic(var->_varName));
	debugC(4, kDebugBehavior, "BehaviorAni::initObjectBehavior(%s, %d, %s)", s.c_str(), sc->_sceneId, transCyrillic(ani->_title));

	clear();

	_animsCount = var->getSubVarsCount();
	_counterMax = -1;

	while (var->_varType == 2) {
		if (strcmp(var->_value.stringValue, "ROOT"))
			break;

		GameVar *v1 = g_nmi->getGameLoaderGameVar()->getSubVarByName("BEHAVIOR")->getSubVarByName(ani->getName());
		if (v1 == var)
			return;

		sc = g_nmi->findScene(ani->_sceneId);
		clear();
		var = v1;
		_animsCount = var->getSubVarsCount();
		_counterMax = -1;
	}

	for (int i = 0; i < _animsCount; i++) {
		int maxDelay = 0;

		_behaviorAnims.push_back(BehaviorAnim(var->getSubVarByIndex(i), sc, ani, &maxDelay));

		if (maxDelay < _counterMax)
			_counterMax = maxDelay;
	}
}

BehaviorAnim::BehaviorAnim() {
	_staticsId = 0;
	_flags = 0;
}

BehaviorAnim::BehaviorAnim(GameVar *var, Scene *sc, AniObject *ani, int *minDelay) {
	_staticsId = 0;

	*minDelay = 0xffffffff;

	int totalPercent = 0;
	_flags = 0;

	Statics *st = ani->getStaticsByName(var->_varName);
	if (st)
		_staticsId = st->_staticsId;

	int movesCount = var->getSubVarsCount();
	if (movesCount) {
		_behaviorMoves.reserve(movesCount);
		for (int i = 0; i < movesCount; i++) {
			GameVar *subvar = var->getSubVarByIndex(i);
			int delay = 0;

			_behaviorMoves.push_back(BehaviorMove(subvar, sc, &delay));
			BehaviorMove &move = _behaviorMoves.back();
			totalPercent += delay;

			if (move._delay < *minDelay)
				*minDelay = move._delay;
		}

		if (!*minDelay && totalPercent == 1000)
			_flags |= 1;
	}
}

BehaviorMove::BehaviorMove(GameVar *subvar, Scene *sc, int *delay) {
	_messageQueue = sc->getMessageQueueByName(subvar->_varName);
	if (_messageQueue)
		_queueId = _messageQueue->_queueId;

	GameVar *vart = subvar->getSubVarByName("dwDelay");
	if (vart)
		_delay = vart->_value.intValue;

	*delay = 0;
	vart = subvar->getSubVarByName("dwPercent");
	if (vart) {
		_percent = 0x7FFF * vart->_value.intValue / 1000;
		*delay = vart->_value.intValue;
	}

	vart = subvar->getSubVarByName("dwFlags");
	if (vart && vart->_varType == 2 && strstr(vart->_value.stringValue, "QDESC_AUTOSTART"))
		_flags |= 2;
}

} // End of namespace NGI
