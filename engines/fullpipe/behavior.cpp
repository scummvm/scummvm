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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

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

void BehaviorManager::initBehavior(Scene *sc, CGameVar *var) {
	clear();
	_scene = sc;

	BehaviorInfo *behinfo;

	CGameVar *behvar = var->getSubVarByName("BEHAVIOR");
	if (!behvar)
		return;

	for (CGameVar *subvar = behvar->_subVars; subvar; subvar = subvar->_nextVarObj) {
		if (!strcmp(subvar->_varName, "AMBIENT")) {
			behinfo = new BehaviorInfo;
			behinfo->initAmbientBehavior(subvar);

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
	for (int i = 0; i < entry->_itemsCount; i++) {
		BehaviorEntryInfo *bhi = entry->_items[i];
		if (!(bhi->_flags & 1)) {
			if (bhi->_flags & 2) {
				MessageQueue *mq = new MessageQueue(entry->_items[i]->_messageQueue, 0, 1);

				mq->sendNextCommand();

				entry->_items[i]->_flags &= 0xFFFFFFFD;
			} else if (behaviorInfo->_counter >= bhi->_delay && bhi->_percent && g_fullpipe->_rnd->getRandomNumber(32767) <= entry->_items[i]->_percent) {
				MessageQueue *mq = new MessageQueue(entry->_items[i]->_messageQueue, 0, 1);

				mq->sendNextCommand();

				behaviorInfo->_counter = 0;
			}
		}
	}
}

void BehaviorManager::updateStaticAniBehavior(StaticANIObject *ani, unsigned int delay, BehaviorEntry *behaviorEntry) {
	warning("STUB: BehaviorManager::updateStaticAniBehavior()");
}

void BehaviorInfo::initAmbientBehavior(CGameVar *var) {
	warning("STUB: BehaviorInfo::initAmbientBehavior(%s)", transCyrillic((byte *)var->_varName));
}

void BehaviorInfo::initObjectBehavior(CGameVar *var, Scene *sceneObj, StaticANIObject *ani) {
	warning("STUB: BehaviorInfo::initObjectBehavior(%s)", transCyrillic((byte *)var->_varName));
}

} // End of namespace Fullpipe
