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

#include "fullpipe/utils.h"
#include "fullpipe/statics.h"
#include "fullpipe/motion.h"
#include "fullpipe/messages.h"

namespace Fullpipe {

void MGM::clear() {
	_items.clear();
}

MessageQueue *MGM::genMQ(StaticANIObject *ani, int staticsIndex, int staticsId, int *resStatId, Common::Point **pointArr) {
	int idx = getItemIndexById(ani->_id);

	if (idx == -1)
		return 0;

	int stid = staticsId;

	if (!staticsId) {
		if (ani->_movement) {
			stid = ani->_movement->_staticsObj2->_staticsId;
		} else {
			if (!ani->_statics)
				return 0;

			stid = ani->_statics->_staticsId;
		}
	}

	if (stid == staticsIndex)
		return new MessageQueue(g_fp->_globalMessageQueueList->compact());

	int startidx = getStaticsIndexById(idx, stid);
	int endidx = getStaticsIndexById(idx, staticsIndex);
	int subidx = startidx + endidx * _items[idx]->statics.size();

	if (!_items[idx]->subItems[subidx]->movement) {
		clearMovements2(idx);
		recalcOffsets(idx, startidx, endidx, 0, 1);
	}

	if (!_items[idx]->subItems[subidx]->movement)
		return 0;

    MessageQueue *mq = new MessageQueue(g_fp->_globalMessageQueueList->compact());
	Common::Point point;
	ExCommand *ex;

	int i = 0;
	do {
		subidx = startidx + endidx * _items[idx]->statics.size();

		_items[idx]->subItems[subidx]->movement->calcSomeXY(point, 0, -1);

		if (pointArr) {
			int sz;

			if (_items[idx]->subItems[subidx]->movement->_currMovement)
				sz = _items[idx]->subItems[subidx]->movement->_currMovement->_dynamicPhases.size();
			else
				sz = _items[idx]->subItems[subidx]->movement->_dynamicPhases.size();

			ex = new ExCommand2(20, ani->_id, &pointArr[i], sz);

			ex->_messageNum = _items[idx]->subItems[subidx]->movement->_id;
		} else {
			ex = new ExCommand(ani->_id, 1, _items[idx]->subItems[subidx]->movement->_id, 0, 0, 0, 1, 0, 0, 0);
		}

		ex->_keyCode = ani->_okeyCode;
		ex->_field_3C = 1;
		ex->_field_24 = 1;

		mq->addExCommandToEnd(ex);

		if (resStatId)
			*resStatId = _items[idx]->subItems[subidx]->movement->_id;

		startidx = _items[idx]->subItems[subidx]->staticsIndex;

		uint step;

		if (_items[idx]->subItems[subidx]->movement->_currMovement)
			step = _items[idx]->subItems[subidx]->movement->_currMovement->_dynamicPhases.size();
		else
			step = _items[idx]->subItems[subidx]->movement->_dynamicPhases.size();

		i += step;
	} while (startidx != endidx);

	return mq;
}

MGMItem::MGMItem() {
	objId = 0;
}

MGMSubItem::MGMSubItem() {
	movement = 0;
	staticsIndex = 0;
	field_8 = 0;
	field_C = 0;
	x = 0;
	y = 0;
}

void MGM::addItem(int objId) {
	if (getItemIndexById(objId) == -1) {
		MGMItem *item = new MGMItem();

		item->objId = objId;
		_items.push_back(item);
	}
	rebuildTables(objId);
}

void MGM::rebuildTables(int objId) {
	int idx = getItemIndexById(objId);

	if (idx == -1)
		return;

	_items[idx]->subItems.clear();
	_items[idx]->statics.clear();
	_items[idx]->movements1.clear();
	_items[idx]->movements2.clear();

	StaticANIObject *obj = g_fp->_currentScene->getStaticANIObject1ById(objId, -1);

	if (!obj)
		return;

	for (uint i = 0; i < obj->_staticsList.size(); i++) {
		_items[idx]->statics.push_back((Statics *)obj->_staticsList[i]);

		_items[idx]->subItems.push_back(new MGMSubItem);
	}

	for (uint i = 0; i < obj->_movements.size(); i++)
		_items[idx]->movements1.push_back((Movement *)obj->_movements[i]);
}

int MGM::getItemIndexById(int objId) {
	for (uint i = 0; i < _items.size(); i++)
		if (_items[i]->objId == objId)
			return i;

	return -1;
}

MessageQueue *MGM::genMovement(MGMInfo *mgminfo) {
	if (!mgminfo->ani)
		return 0;

	Movement *mov = mgminfo->ani->_movement;

	if (!mov && !mgminfo->ani->_statics)
		return 0;

	if (!(mgminfo->flags & 1)) {
		if (mov)
			mgminfo->staticsId1 = mov->_staticsObj2->_staticsId;
		else
			mgminfo->staticsId1 = mgminfo->ani->_statics->_staticsId;
	}

	Common::Point point;

	if (!(mgminfo->flags & 0x10) || !(mgminfo->flags & 0x20)) {
		int nx = mgminfo->ani->_ox;
		int ny = mgminfo->ani->_oy;

		if (mgminfo->ani->_movement) {
			mgminfo->ani->calcNextStep(&point);

			nx += point.x;
			ny += point.y;
		}

		if (!(mgminfo->flags & 0x10))
			mgminfo->x2 = nx;

		if (!(mgminfo->flags & 0x20))
			mgminfo->y2 = ny;
	}

	mov = mgminfo->ani->getMovementById(mgminfo->movementId);

	if (!mov)
		return 0;

	int itemIdx = getItemIndexById(mgminfo->ani->_id);
	int subIdx = getStaticsIndexById(itemIdx, mgminfo->staticsId1);
	int st2idx = getStaticsIndexById(itemIdx, mov->_staticsObj1->_staticsId);
	int st1idx = getStaticsIndexById(itemIdx, mov->_staticsObj2->_staticsId);
	int subOffset = getStaticsIndexById(itemIdx, mgminfo->staticsId2);

	clearMovements2(itemIdx);
	recalcOffsets(itemIdx, subIdx, st2idx, 0, 1);
	clearMovements2(itemIdx);
	recalcOffsets(itemIdx, st1idx, subOffset, 0, 1);

	MGMSubItem *sub1 = _items[itemIdx]->subItems[subIdx + st2idx * _items[itemIdx]->statics.size()];
	MGMSubItem *sub2 = _items[itemIdx]->subItems[st1idx + subOffset * _items[itemIdx]->statics.size()];

	if (subIdx != st2idx && !sub1->movement)
		return 0;

	if (st1idx != subOffset && !sub2->movement)
		return 0;

	int n1x = mgminfo->x1 - mgminfo->x2 - sub1->x - sub2->x;
	int n1y = mgminfo->y1 - mgminfo->y2 - sub1->y - sub2->y;

	Common::Point point1;

	mov->calcSomeXY(point1, 0, -1);

	int n2x = point1.x;
	int n2y = point1.y;
	int mult;
	int len = -1;

	if (mgminfo->flags & 0x40) {
		mult = mgminfo->field_10;
		len = -1;
		n2x *= mult;
		n2y *= mult;
	} else {
		calcLength(&point, mov, n1x, n1y, &mult, &len, 1);
		n2x = point.x;
		n2y = point.y;
	}

	if (!(mgminfo->flags & 2)) {
		len = -1;
		n2x = mult * point1.x;
		n1x = mult * point1.x;
		mgminfo->x1 = mgminfo->x2 + mult * point1.x + sub1->x + sub2->x;
	}

	if (!(mgminfo->flags & 4)) {
		n2y = mult * point1.y;
		n1y = mult * point1.y;
		len = -1;
		mgminfo->y1 = mgminfo->y2 + mult * point1.y + sub1->y + sub2->y;
	}

	int px = 0;
	int py = 0;

	if (sub1->movement) {
		px = countPhases(itemIdx, subIdx, st2idx, 1);
		py = countPhases(itemIdx, subIdx, st2idx, 2);
	}

	if (mult > 1) {
		px += (mult - 1) * mov->countPhasesWithFlag(-1, 1);
		py += (mult - 1) * mov->countPhasesWithFlag(-1, 2);
	}

	if (mult > 0) {
		px += mov->countPhasesWithFlag(len, 1);
		py += mov->countPhasesWithFlag(len, 2);
	}

	if (sub2->movement) {
		px += countPhases(itemIdx, st1idx, subOffset, 1);
		py += countPhases(itemIdx, st1idx, subOffset, 2);
	}

	int dx1 = n1x - n2x;
	int dy1 = n1y - n2y;
	int x1, y1;

	if (px) {
		x1 = (int)((double)dx1 / (double)px);
	} else {
		x1 = 0;
	}

	if (py) {
		y1 = (int)((double)dy1 / (double)py);
	} else {
		y1 = 0;
	}

	Common::Point x2, y2;

	y2.x = dx1 - px * x1;
	y2.y = dy1 - py * y1;

	if (n1x - n2x == px * x1)
		x2.x = 0;
	else
		x2.x = (dx1 - px * x1) / abs(dx1 - px * x1);

	if (dy1 == py * y1)
		x2.y = 0;
	else
		x2.y = (dy1 - py * y1) / abs(dy1 - py * y1);

	MessageQueue *mq = new MessageQueue(g_fp->_globalMessageQueueList->compact());
	ExCommand2 *ex2;

	for (int i = subIdx; i != st2idx;) {
		MGMSubItem *s = _items[itemIdx]->subItems[i + subOffset * _items[itemIdx]->statics.size()];

		ex2 = buildExCommand2(s->movement, mgminfo->ani->_id, x1, y1, &x2, &y2, -1);
		ex2->_parId = mq->_id;
		ex2->_keyCode = mgminfo->ani->_okeyCode;

		mq->addExCommandToEnd(ex2);

		i = s->staticsIndex;
	}

	for (int i = 0; i < mult; ++i) {
		int plen;

		if (i == mult - 1)
			plen = len;
		else
			plen = -1;

		ex2 = buildExCommand2(mov, mgminfo->ani->_id, x1, y1, &x2, &y2, plen);
		ex2->_parId = mq->_id;
		ex2->_keyCode = mgminfo->ani->_okeyCode;

		mq->addExCommandToEnd(ex2);
	}

	for (int j = st1idx; j != subOffset;) {
		MGMSubItem *s = _items[itemIdx]->subItems[j + subOffset * _items[itemIdx]->statics.size()];

		ex2 = buildExCommand2(s->movement, mgminfo->ani->_id, x1, y1, &x2, &y2, -1);
		ex2->_parId = mq->_id;
		ex2->_keyCode = mgminfo->ani->_okeyCode;

		mq->addExCommandToEnd(ex2);

		j = s->staticsIndex;
	}

	ExCommand *ex = new ExCommand(mgminfo->ani->_id, 5, -1, mgminfo->x1, mgminfo->y1, 0, 1, 0, 0, 0);

	ex->_field_14 = mgminfo->field_1C;
	ex->_keyCode = mgminfo->ani->_okeyCode;
	ex->_field_24 = 0;
	ex->_excFlags |= 3;

	mq->addExCommandToEnd(ex);

	return mq;
}

int MGM::countPhases(int idx, int subIdx, int endIdx, int flag) {
	int res = 0;

	if (endIdx < 0)
		return 0;

	while (subIdx != endIdx) {
		if (subIdx < 0)
			break;

		res += _items[idx]->subItems[subIdx + endIdx * _items[idx]->statics.size()]->movement->countPhasesWithFlag(-1, flag);

		subIdx = _items[idx]->subItems[subIdx + 6 * endIdx * _items[idx]->statics.size()]->staticsIndex;
	}

	return res;
}
void MGM::updateAnimStatics(StaticANIObject *ani, int staticsId) {
	if (getItemIndexById(ani->_id) == -1)
		return;

	if (ani->_movement) {
		ani->queueMessageQueue(0);
		ani->_movement->gotoLastFrame();
		ani->_statics = ani->_movement->_staticsObj2;

		int x = ani->_movement->_ox;
		int y = ani->_movement->_oy;

		ani->_movement = 0;

		ani->setOXY(x, y);
	}

	if (ani->_statics) {
		Common::Point point;

		getPoint(&point, ani->_id, ani->_statics->_staticsId, staticsId);

		ani->setOXY(ani->_ox + point.x, ani->_oy + point.y);

		ani->_statics = ani->getStaticsById(staticsId);
	}
}

Common::Point *MGM::getPoint(Common::Point *point, int objectId, int staticsId1, int staticsId2) {
	int idx = getItemIndexById(objectId);

	if (idx == -1) {
		point->x = -1;
		point->y = -1;
	} else {
		int st1idx = getStaticsIndexById(idx, staticsId1);
		int st2idx = getStaticsIndexById(idx, staticsId2);

		if (st1idx == st2idx) {
			point->x = 0;
			point->y = 0;
		} else {
			int subidx = st1idx + st2idx * _items[idx]->statics.size();

			if (!_items[idx]->subItems[subidx]->movement) {
				clearMovements2(idx);
				recalcOffsets(idx, st1idx, st2idx, false, true);

				if (!_items[idx]->subItems[subidx]->movement) {
					clearMovements2(idx);
					recalcOffsets(idx, st1idx, st2idx, true, false);
				}
			}

			MGMSubItem *sub = _items[idx]->subItems[subidx];

			if (sub->movement) {
				point->x = sub->x;
				point->y = sub->y;
			} else {
				point->x = 0;
				point->y = 0;
			}
		}
	}

	return point;
}

int MGM::getStaticsIndexById(int idx, int16 id) {
	if (!_items[idx]->statics.size())
		return -1;

	for (uint i = 0; i < _items[idx]->statics.size(); i++) {
		if (_items[idx]->statics[i]->_staticsId == id)
			return i;
	}

	return 0;
}

int MGM::getStaticsIndex(int idx, Statics *st) {
	if (!_items[idx]->statics.size())
		return -1;

	for (uint i = 0; i < _items[idx]->statics.size(); i++) {
		if (_items[idx]->statics[i] == st)
			return i;
	}

	return 0;
}

void MGM::clearMovements2(int idx) {
	_items[idx]->movements2.clear();
}

int MGM::recalcOffsets(int idx, int st1idx, int st2idx, bool flip, bool flop) {
	MGMItem *item = _items[idx];
	int subIdx = st1idx + st2idx * item->statics.size();

	if (st1idx == st2idx) {
		memset(&item->subItems[subIdx], 0, sizeof(item->subItems[subIdx]));
		return 0;
	}

	if (item->subItems[subIdx])
		return item->subItems[subIdx]->field_8;

	Common::Point point;

	for (uint i = 0; i < item->movements1.size(); i++) {
		Movement *mov = item->movements1[i];

		if (mov->_staticsObj1 == item->statics[st1idx]) {
			if (!item->movements2[i] && (!flop || mov->_field_50)) {
				item->movements2[i] = 1;

				int stidx = getStaticsIndex(idx, item->movements1[i]->_staticsObj2);
				int recalc = recalcOffsets(idx, stidx, st2idx, flip, flop);
				int sz = mov->_currMovement ? mov->_currMovement->_dynamicPhases.size() : mov->_dynamicPhases.size();
				int newsz = sz + item->subItems[stidx + 6 * st2idx * _items[idx]->statics.size()]->field_C;

				if (recalc >= 0) {
					if (!item->subItems[subIdx]->movement || item->subItems[subIdx]->field_8 > recalc + 1 ||
						(item->subItems[subIdx]->field_8 == recalc + 1 && item->subItems[subIdx]->field_C > newsz)) {
						item->subItems[subIdx]->movement = mov;
						item->subItems[subIdx]->staticsIndex = stidx;
						item->subItems[subIdx]->field_8 = recalc + 1;
						item->subItems[subIdx]->field_C = newsz;

						mov->calcSomeXY(point, 0, -1);

						item->subItems[subIdx]->x = item->subItems[stidx + 6 * st2idx * _items[idx]->statics.size()]->x + point.x;
						item->subItems[subIdx]->y = item->subItems[stidx + 6 * st2idx * _items[idx]->statics.size()]->y + point.y;
					}
				}
			}
		} else if (flip) {
			if (mov->_staticsObj2 == item->statics[st1idx]) {
				if (!item->movements2[i] && (!flop || mov->_field_50)) {
					item->movements2[i] = 1;

					int stidx = getStaticsIndex(idx, mov->_staticsObj1);
					int recalc = recalcOffsets(idx, stidx, st2idx, flip, flop);

					if (recalc >= 0) {
						if (!item->subItems[subIdx]->movement || item->subItems[subIdx]->field_8 > recalc + 1) {
							item->subItems[subIdx]->movement = mov;
							item->subItems[subIdx]->staticsIndex = stidx;
							item->subItems[subIdx]->field_8 = recalc + 1;

							int sz = mov->_currMovement ? mov->_currMovement->_dynamicPhases.size() : mov->_dynamicPhases.size();

							item->subItems[subIdx]->field_C = sz + item->subItems[stidx + 6 * st2idx * _items[idx]->statics.size()]->field_C;

							mov->calcSomeXY(point, 0, -1);

							item->subItems[subIdx]->x = item->subItems[stidx + 6 * st2idx * _items[idx]->statics.size()]->x - point.x;
							item->subItems[subIdx]->y = item->subItems[stidx + 6 * st2idx * _items[idx]->statics.size()]->y - point.y;
						}
					}
				}
			}
		}
	}

	if (item->subItems[subIdx]->movement)
		return item->subItems[subIdx]->field_8;

	return -1;
}

int MGM::refreshOffsets(int objectId, int idx1, int idx2) {
	int idx = getItemIndexById(objectId);

	if (idx != -1) {
		int from = getStaticsIndexById(idx, idx1);
		int to = getStaticsIndexById(idx, idx2);

		MGMSubItem *sub = _items[idx]->subItems[from + to * _items[idx]->statics.size()];

		if (sub->movement) {
			idx = sub->field_8;
		} else {
			clearMovements2(idx);
			idx = recalcOffsets(idx, from, to, 0, 1);
		}
	}

	return idx;
}

Common::Point *MGM::calcLength(Common::Point *pRes, Movement *mov, int x, int y, int *mult, int *len, int flag) {
	Common::Point point;

	mov->calcSomeXY(point, 0, -1);
	int p1x = point.x;
	int p1y = point.y;

	int newmult = 0;
	int oldlen = *len;

	if (abs(p1y) > abs(p1x)) {
		if (mov->calcSomeXY(point, 0, -1)->y)
			newmult = (int)((double)y / mov->calcSomeXY(point, 0, -1)->y);
	} else if (mov->calcSomeXY(point, 0, -1)->x) {
		newmult = (int)((double)x / mov->calcSomeXY(point, 0, -1)->x);
	}

	if (newmult < 0)
		newmult = 0;

	*mult = newmult;

	int phase = 1;
	int sz;

	if (flag) {
		if (abs(p1y) > abs(p1x)) {
			while (abs(p1y * newmult + mov->calcSomeXY(point, 0, phase)->y) < abs(y)) {
				sz = mov->_currMovement ? mov->_currMovement->_dynamicPhases.size() : mov->_dynamicPhases.size();

				if (phase > sz)
					break;

				phase++;
			}
		} else {
			while (abs(p1x * newmult + mov->calcSomeXY(point, 0, phase)->x) < abs(x)) {
				sz = mov->_currMovement ? mov->_currMovement->_dynamicPhases.size() : mov->_dynamicPhases.size();

				if (phase >= sz)
					break;

				phase++;
			}
		}

		*len = phase - 1;
	} else {
		*len = -1;
	}

	int p2x = 0;
	int p2y = 0;

	if (!oldlen)
		oldlen = -1;

	if (oldlen > 0) {
		++*mult;

		mov->calcSomeXY(point, 0, oldlen);
		p2x = point.x;
		p2y = point.y;

		if (abs(p1y) > abs(p1x))
			p2x = p1x;
		else
			p2y = p1y;
	}

	pRes->x = p2x + p1x * newmult;
	pRes->y = p2y + p1y * newmult;

	return pRes;
}

ExCommand2 *MGM::buildExCommand2(Movement *mov, int objId, int x1, int y1, Common::Point *x2, Common::Point *y2, int len) {
	uint cnt;

	if (mov->_currMovement)
		cnt = mov->_currMovement->_dynamicPhases.size();
	else
		cnt = mov->_dynamicPhases.size();

	if (len > 0 && cnt > (uint)len)
		cnt = len;

	Common::Point **points = (Common::Point **)malloc(sizeof(Common::Point *) * cnt);

	for (uint i = 0; i < cnt; i++) {
		int flags = mov->getDynamicPhaseByIndex(i)->getDynFlags();

		points[i] = new Common::Point;

		if (flags & 1) {
			points[i]->x = x1 + x2->x;

			y2->x -= x2->x;

			if (!y2->x)
				x2->x = 0;
		}

		if (flags & 2) {
			points[i]->y = y1 + x2->y;

			y2->y -= x2->y;

			if (!y2->y)
				x2->y = 0;
		}
	}

	ExCommand2 *ex = new ExCommand2(20, objId, points, cnt);
	ex->_excFlags = 2;
	ex->_messageNum = mov->_id;
	ex->_field_14 = len;
	ex->_field_24 = 1;
	ex->_keyCode = -1;

	for (uint i = 0; i < cnt; i++)
		delete points[i];

	free(points);

	return ex;
}

} // End of namespace Fullpipe
