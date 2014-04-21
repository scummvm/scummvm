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
		for (int j = 0; j < _behaviors[i]->_itemsCount; j++)
			delete _behaviors[i]->_bheItems[j];

		delete _behaviors[i];
	}
	_behaviors.clear();
}

void BehaviorManager::initBehavior(Scene *sc, GameVar *var) {
	clear();
	_scene = sc;

	BehaviorInfo *behinfo;

	GameVar *behvar = var->getSubVarByName("BEHAVIOR");
	if (!behvar)
		return;

	for (GameVar *subvar = behvar->_subVars; subvar; subvar = subvar->_nextVarObj) {
		if (!strcmp(subvar->_varName, "AMBIENT")) {
			behinfo = new BehaviorInfo;
			behinfo->initAmbientBehavior(subvar, sc);

			_behaviors.push_back(behinfo);
		} else {
			StaticANIObject *ani = sc->getStaticANIObject1ByName(subvar->_varName, -1);
			if (ani)
				for (uint i = 0; i < sc->_staticANIObjectList1.size(); i++)
					if (((StaticANIObject *)sc->_staticANIObjectList1[i])->_id == ani->_id) {
						behinfo = new BehaviorInfo;
						behinfo->initObjectBehavior(subvar, sc, ani);
						behinfo->_ani = (StaticANIObject *)sc->_staticANIObjectList1[i];

						_behaviors.push_back(behinfo);
					}
		}
	}
}

void BehaviorManager::updateBehaviors() {
	if (!_isActive)
		return;

	debug(4, "BehaviorManager::updateBehaviors()");
	for (uint i = 0; i < _behaviors.size(); i++) {
		BehaviorInfo *beh = _behaviors[i];

		if (!beh->_ani) {
			beh->_counter++;
			if (beh->_counter >= beh->_counterMax)
				updateBehavior(beh, beh->_bheItems[0]);

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
					updateStaticAniBehavior(beh->_ani, beh->_counter, beh->_bheItems[beh->_subIndex]);
			}
		} else {
			beh->_staticsId = beh->_ani->_statics->_staticsId;
			beh->_counter = 0;
			beh->_subIndex = -1;

			for (int j = 0; j < beh->_itemsCount; j++)
				if (beh->_bheItems[j]->_staticsId == beh->_staticsId) {
					beh->_subIndex = j;
					break;
				}

		}
	}
}

void BehaviorManager::updateBehavior(BehaviorInfo *behaviorInfo, BehaviorEntry *entry) {
	debug(4, "BehaviorManager::updateBehavior() %d", entry->_itemsCount);
	for (int i = 0; i < entry->_itemsCount; i++) {
		BehaviorEntryInfo *bhi = entry->_items[i];
		if (!(bhi->_flags & 1)) {
			if (bhi->_flags & 2) {
				MessageQueue *mq = new MessageQueue(bhi->_messageQueue, 0, 1);

				mq->sendNextCommand();

				bhi->_flags &= 0xFFFFFFFD;
			} else if (behaviorInfo->_counter >= bhi->_delay && bhi->_percent && g_fp->_rnd->getRandomNumber(32767) <= entry->_items[i]->_percent) {
				MessageQueue *mq = new MessageQueue(bhi->_messageQueue, 0, 1);

				mq->sendNextCommand();

				behaviorInfo->_counter = 0;
			}
		}
	}
}

void BehaviorManager::updateStaticAniBehavior(StaticANIObject *ani, int delay, BehaviorEntry *bhe) {
	debug(4, "BehaviorManager::updateStaticAniBehavior(%s)", transCyrillic((byte *)ani->_objectName));

	MessageQueue *mq = 0;

	if (bhe->_flags & 1) {
		uint rnd = g_fp->_rnd->getRandomNumber(32767);
		uint runPercent = 0;
		for (int i = 0; i < bhe->_itemsCount; i++) {
			if (!(bhe->_items[i]->_flags & 1) && bhe->_items[i]->_percent) {
				if ((rnd >= runPercent && rnd <= runPercent + bhe->_items[i]->_percent) || i == bhe->_itemsCount - 1) {
					mq = new MessageQueue(bhe->_items[i]->_messageQueue, 0, 1);
					break;
				}
				runPercent += bhe->_items[i]->_percent;
			}
		}
	} else {
		for (int i = 0; i < bhe->_itemsCount; i++) {
			if (!(bhe->_items[i]->_flags & 1) && delay >= bhe->_items[i]->_delay) {
				if (bhe->_items[i]->_percent) {
					if (g_fp->_rnd->getRandomNumber(32767) <= bhe->_items[i]->_percent) {
						mq = new MessageQueue(bhe->_items[i]->_messageQueue, 0, 1);
						break;
					}
				}
			}
		}
	}

	if (mq) {
		mq->replaceKeyCode(-1, ani->_okeyCode);
		mq->chain(ani);
	}
}

bool BehaviorManager::setBehaviorEnabled(StaticANIObject *obj, int aniId, int quId, int flag) {
	BehaviorEntryInfo *entry = getBehaviorEntryInfoByMessageQueueDataId(obj, aniId, quId);

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
				beh->_flags &= 0xfe;
			else
				beh->_flags |= 1;
		}
	}
}

BehaviorEntryInfo *BehaviorManager::getBehaviorEntryInfoByMessageQueueDataId(StaticANIObject *ani, int id1, int id2) {
	for (uint i = 0; i < _behaviors.size(); i++) {
		if (_behaviors[i]->_ani == ani) {
			for (uint j = 0; j < _behaviors[i]->_bheItems.size(); j++) {
				if (_behaviors[i]->_bheItems[j]->_staticsId == id1) {
					for (int k = 0; k < _behaviors[i]->_bheItems[j]->_itemsCount; k++) {
						if (_behaviors[i]->_bheItems[j]->_items[k]->_messageQueue->_dataId == id2)
							return _behaviors[i]->_bheItems[j]->_items[k];
					}
				}
			}
		}
	}

	return 0;
}

void BehaviorInfo::clear() {
	_ani = 0;
	_staticsId = 0;
	_counter = 0;
	_counterMax = 0;
	_flags = 0;
	_subIndex = 0;
	_itemsCount = 0;

	_bheItems.clear();
}

void BehaviorInfo::initAmbientBehavior(GameVar *var, Scene *sc) {
	debug(4, "BehaviorInfo::initAmbientBehavior(%s)", transCyrillic((byte *)var->_varName));

	clear();
	_itemsCount = 1;
	_counterMax = -1;

	BehaviorEntry *bi = new BehaviorEntry();

	_bheItems.push_back(bi);

	bi->_itemsCount = var->getSubVarsCount();

	bi->_items = (BehaviorEntryInfo**)calloc(bi->_itemsCount, sizeof(BehaviorEntryInfo *));

	for (int i = 0; i < bi->_itemsCount; i++) {
		int delay;
		bi->_items[i] = new BehaviorEntryInfo(var->getSubVarByIndex(i), sc, &delay);

		if (bi->_items[i]->_delay <_counterMax)
			_counterMax = bi->_items[i]->_delay;
	}
}

void BehaviorInfo::initObjectBehavior(GameVar *var, Scene *sc, StaticANIObject *ani) {
	debug(4, "BehaviorInfo::initObjectBehavior(%s)", transCyrillic((byte *)var->_varName));

	clear();

	_itemsCount = var->getSubVarsCount();
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
		_itemsCount = var->getSubVarsCount();
		_counterMax = -1;
	}

	for (int i = 0; i < _itemsCount; i++) {
		int maxDelay = 0;

		_bheItems.push_back(new BehaviorEntry(var->getSubVarByIndex(i), sc, ani, &maxDelay));

		if (maxDelay < _counterMax)
			_counterMax = maxDelay;
	}
}

BehaviorEntry::BehaviorEntry() {
	_staticsId = 0;
	_itemsCount = 0;
	_flags = 0;
	_items = 0;
}

BehaviorEntry::BehaviorEntry(GameVar *var, Scene *sc, StaticANIObject *ani, int *minDelay) {
	_staticsId = 0;
	_itemsCount = 0;

	*minDelay = 100000000;

	int totalPercent = 0;
	_flags = 0;
	_items = 0;

	Statics *st = ani->getStaticsByName(var->_varName);
	if (st)
		_staticsId = st->_staticsId;

	_itemsCount = var->getSubVarsCount();
	if (_itemsCount) {
		_items = (BehaviorEntryInfo**)calloc(_itemsCount, sizeof(BehaviorEntryInfo *));

		for (int i = 0; i < _itemsCount; i++) {
			GameVar *subvar = var->getSubVarByIndex(i);
			int delay = 0;

			_items[i] = new BehaviorEntryInfo(subvar, sc, &delay);
			totalPercent += delay;

			if (_items[i]->_delay < *minDelay)
				*minDelay = _items[i]->_delay;
		}

		if (!*minDelay && totalPercent == 1000)
			_flags |= 1;
	}
}

BehaviorEntryInfo::BehaviorEntryInfo(GameVar *subvar, Scene *sc, int *delay) {
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
