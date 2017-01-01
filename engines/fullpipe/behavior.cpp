/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "fullpipe/fullpipe.h"

#include "fullpipe/objects.h"
#include "fullpipe/behavior.h"
#include "fullpipe/statics.h"
#include "fullpipe/messages.h"

namespace Fullpipe {

BehaviorManager::BehaviorManager() {
	_scene = 0;
	_isActive = 1;
}

BehaviorManager::~BehaviorManager() {
	clear();
}

void BehaviorManager::clear() {
	for (uint i = 0; i < _behaviors.size(); i++) {
		for (int j = 0; j < _behaviors[i]->_animsCount; j++)
			delete _behaviors[i]->_behaviorAnims[j];

		delete _behaviors[i];
	}
	_behaviors.clear();
}

void BehaviorManager::initBehavior(Scene *sc, GameVar *var) {
	debugC(2, kDebugBehavior, "BehaviorManager::initBehavior(%d, %s)", sc->_sceneId, transCyrillic((byte *)var->_varName));

	clear();
	_scene = sc;

	BehaviorInfo *behinfo;

	GameVar *behvar = var->getSubVarByName("BEHAVIOR");
	if (!behvar)
		return;

	debugC(3, kDebugBehavior, "BehaviorManager::initBehavior. have Variable");

	for (GameVar *subvar = behvar->_subVars; subvar; subvar = subvar->_nextVarObj) {
		debugC(3, kDebugBehavior, "BehaviorManager::initBehavior. subVar %s", transCyrillic((byte *)subvar->_varName));
		if (!strcmp(subvar->_varName, "AMBIENT")) {
			behinfo = new BehaviorInfo;
			behinfo->initAmbientBehavior(subvar, sc);

			_behaviors.push_back(behinfo);
		} else {
			StaticANIObject *ani = sc->getStaticANIObject1ByName(subvar->_varName, -1);
			if (ani) {
				for (uint i = 0; i < sc->_staticANIObjectList1.size(); i++) {
					if (((StaticANIObject *)sc->_staticANIObjectList1[i])->_id == ani->_id) {
						behinfo = new BehaviorInfo;
						behinfo->initObjectBehavior(subvar, sc, ani);
						behinfo->_ani = (StaticANIObject *)sc->_staticANIObjectList1[i];

						_behaviors.push_back(behinfo);
					}
				}
			}
		}
	}
}

void BehaviorManager::updateBehaviors() {
	if (!_isActive)
		return;

	debugC(6, kDebugBehavior, "BehaviorManager::updateBehaviors()");
	for (uint i = 0; i < _behaviors.size(); i++) {
		BehaviorInfo *beh = _behaviors[i];

		if (!beh->_ani) {
			beh->_counter++;
			if (beh->_counter >= beh->_counterMax)
				updateBehavior(beh, beh->_behaviorAnims[0]);

			continue;
		}

		if (beh->_ani->_movement || !(beh->_ani->_flags & 4) || (beh->_ani->_flags & 2)) {
			beh->_staticsId = 0;
			continue;
		}

		if (beh->_ani->_statics->_staticsId == beh->_staticsId) {
			beh->_counter++;
			if (beh->_counter >= beh->_counterMax) {
				if (beh->_subIndex >= 0 && !(beh->_flags & 1) && beh->_ani->_messageQueueId <= 0)
					updateStaticAniBehavior(beh->_ani, beh->_counter, beh->_behaviorAnims[beh->_subIndex]);
			}
		} else {
			beh->_staticsId = beh->_ani->_statics->_staticsId;
			beh->_counter = 0;
			beh->_subIndex = -1;

			for (int j = 0; j < beh->_animsCount; j++)
				if (beh->_behaviorAnims[j]->_staticsId == beh->_staticsId) {
					beh->_subIndex = j;
					break;
				}

		}
	}
}

void BehaviorManager::updateBehavior(BehaviorInfo *behaviorInfo, BehaviorAnim *entry) {
	debugC(7, kDebugBehavior, "BehaviorManager::updateBehavior() moves: %d", entry->_movesCount);
	for (int i = 0; i < entry->_movesCount; i++) {
		BehaviorMove *bhi = entry->_behaviorMoves[i];
		if (!(bhi->_flags & 1)) {
			if (bhi->_flags & 2) {
				MessageQueue *mq = new MessageQueue(bhi->_messageQueue, 0, 1);

				mq->sendNextCommand();

				bhi->_flags &= 0xFFFFFFFD;
			} else if (behaviorInfo->_counter >= bhi->_delay && bhi->_percent && g_fp->_rnd->getRandomNumber(32767) <= entry->_behaviorMoves[i]->_percent) {
				MessageQueue *mq = new MessageQueue(bhi->_messageQueue, 0, 1);

				mq->sendNextCommand();

				behaviorInfo->_counter = 0;
			}
		}
	}
}

void BehaviorManager::updateStaticAniBehavior(StaticANIObject *ani, int delay, BehaviorAnim *bhe) {
	debugC(6, kDebugBehavior, "BehaviorManager::updateStaticAniBehavior(%s)", transCyrillic((byte *)ani->_objectName));

	MessageQueue *mq = 0;

	if (bhe->_flags & 1) {
		uint rnd = g_fp->_rnd->getRandomNumber(32767);
		uint runPercent = 0;
		for (int i = 0; i < bhe->_movesCount; i++) {
			if (!(bhe->_behaviorMoves[i]->_flags & 1) && bhe->_behaviorMoves[i]->_percent) {
				if ((rnd >= runPercent && rnd <= runPercent + bhe->_behaviorMoves[i]->_percent) || i == bhe->_movesCount - 1) {
					mq = new MessageQueue(bhe->_behaviorMoves[i]->_messageQueue, 0, 1);
					break;
				}
				runPercent += bhe->_behaviorMoves[i]->_percent;
			}
		}
	} else {
		for (int i = 0; i < bhe->_movesCount; i++) {
			if (!(bhe->_behaviorMoves[i]->_flags & 1) && delay >= bhe->_behaviorMoves[i]->_delay) {
				if (bhe->_behaviorMoves[i]->_percent) {
					if (g_fp->_rnd->getRandomNumber(32767) <= bhe->_behaviorMoves[i]->_percent) {
						mq = new MessageQueue(bhe->_behaviorMoves[i]->_messageQueue, 0, 1);
						break;
					}
				}
			}
		}
	}

	if (mq) {
		mq->setParamInt(-1, ani->_odelay);
		mq->chain(ani);
	}
}

bool BehaviorManager::setBehaviorEnabled(StaticANIObject *obj, int aniId, int quId, int flag) {
	BehaviorMove *entry = getBehaviorMoveByMessageQueueDataId(obj, aniId, quId);

	if (entry) {
		if (flag)
			entry->_flags &= 0xFFFFFFFE;
		else
			entry->_flags |= 1;
	} else
		return false;

	return true;
}

void BehaviorManager::setFlagByStaticAniObject(StaticANIObject *ani, int flag) {
	for (uint i = 0; i < _behaviors.size(); i++) {
		BehaviorInfo *beh = _behaviors[i];

		if (ani == beh->_ani) {
			if (flag)
				beh->_flags &= 0xfffffffe;
			else
				beh->_flags |= 1;
		}
	}
}

BehaviorMove *BehaviorManager::getBehaviorMoveByMessageQueueDataId(StaticANIObject *ani, int id1, int id2) {
	for (uint i = 0; i < _behaviors.size(); i++) {
		if (_behaviors[i]->_ani == ani) {
			for (uint j = 0; j < _behaviors[i]->_behaviorAnims.size(); j++) {
				if (_behaviors[i]->_behaviorAnims[j]->_staticsId == id1) {
					for (int k = 0; k < _behaviors[i]->_behaviorAnims[j]->_movesCount; k++) {
						if (_behaviors[i]->_behaviorAnims[j]->_behaviorMoves[k]->_messageQueue->_dataId == id2)
							return _behaviors[i]->_behaviorAnims[j]->_behaviorMoves[k];
					}
				}
			}
		}
	}

	return 0;
}

void BehaviorInfo::clear() {
	_ani = NULL;
	_staticsId = 0;
	_counter = 0;
	_counterMax = 0;
	_flags = 0;
	_subIndex = 0;
	_animsCount = 0;

	_behaviorAnims.clear();
}

void BehaviorInfo::initAmbientBehavior(GameVar *var, Scene *sc) {
	debugC(4, kDebugBehavior, "BehaviorInfo::initAmbientBehavior(%s)", transCyrillic((byte *)var->_varName));

	clear();
	_animsCount = 1;
	_counterMax = -1;

	BehaviorAnim *bi = new BehaviorAnim();

	_behaviorAnims.push_back(bi);

	bi->_movesCount = var->getSubVarsCount();

	bi->_behaviorMoves = (BehaviorMove **)calloc(bi->_movesCount, sizeof(BehaviorMove *));

	for (int i = 0; i < bi->_movesCount; i++) {
		int delay;
		bi->_behaviorMoves[i] = new BehaviorMove(var->getSubVarByIndex(i), sc, &delay);

		if (bi->_behaviorMoves[i]->_delay <_counterMax)
			_counterMax = bi->_behaviorMoves[i]->_delay;
	}
}

void BehaviorInfo::initObjectBehavior(GameVar *var, Scene *sc, StaticANIObject *ani) {
	Common::String s((char *)transCyrillic((byte *)var->_varName));
	debugC(4, kDebugBehavior, "BehaviorInfo::initObjectBehavior(%s, %d, %s)", s.c_str(), sc->_sceneId, transCyrillic((byte *)ani->_objectName));

	clear();

	_animsCount = var->getSubVarsCount();
	_counterMax = -1;

	while (var->_varType == 2) {
		if (strcmp(var->_value.stringValue, "ROOT"))
			break;

		GameVar *v1 = g_fp->getGameLoaderGameVar()->getSubVarByName("BEHAVIOR")->getSubVarByName(ani->getName());
		if (v1 == var)
			return;

		sc = g_fp->accessScene(ani->_sceneId);
		clear();
		var = v1;
		_animsCount = var->getSubVarsCount();
		_counterMax = -1;
	}

	for (int i = 0; i < _animsCount; i++) {
		int maxDelay = 0;

		_behaviorAnims.push_back(new BehaviorAnim(var->getSubVarByIndex(i), sc, ani, &maxDelay));

		if (maxDelay < _counterMax)
			_counterMax = maxDelay;
	}
}

BehaviorAnim::BehaviorAnim() {
	_staticsId = 0;
	_movesCount = 0;
	_flags = 0;
	_behaviorMoves = NULL;
}

BehaviorAnim::BehaviorAnim(GameVar *var, Scene *sc, StaticANIObject *ani, int *minDelay) {
	_staticsId = 0;
	_movesCount = 0;

	*minDelay = 0xffffffff;

	int totalPercent = 0;
	_flags = 0;
	_behaviorMoves = 0;

	Statics *st = ani->getStaticsByName(var->_varName);
	if (st)
		_staticsId = st->_staticsId;

	_movesCount = var->getSubVarsCount();
	if (_movesCount) {
		_behaviorMoves = (BehaviorMove **)calloc(_movesCount, sizeof(BehaviorMove *));

		for (int i = 0; i < _movesCount; i++) {
			GameVar *subvar = var->getSubVarByIndex(i);
			int delay = 0;

			_behaviorMoves[i] = new BehaviorMove(subvar, sc, &delay);
			totalPercent += delay;

			if (_behaviorMoves[i]->_delay < *minDelay)
				*minDelay = _behaviorMoves[i]->_delay;
		}

		if (!*minDelay && totalPercent == 1000)
			_flags |= 1;
	}
}

BehaviorMove::BehaviorMove(GameVar *subvar, Scene *sc, int *delay) {
	_messageQueue = 0;
	_delay = 0;
	_percent = 0;
	_flags = 0;
	_messageQueue = sc->getMessageQueueByName(subvar->_varName);

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

} // End of namespace Fullpipe
