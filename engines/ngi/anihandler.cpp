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

#include "ngi/utils.h"
#include "ngi/statics.h"
#include "ngi/motion.h"
#include "ngi/messages.h"

namespace NGI {

void AniHandler::detachAllObjects() {
	_items.clear();
}

MessageQueue *AniHandler::makeQueue(AniObject *ani, int staticsIndex, int, int *, Common::Point **) {
	debugC(4, kDebugPathfinding, "AniHandler::makeQueue(*%d, %d, id, res, point)", ani->_id, staticsIndex);

	int idx = getIndex(ani->_id);

	if (idx == -1)
		return nullptr;

#if 0
	int stid = staticsId;

	if (!staticsId) {
#else
	int stid;
#endif
		if (ani->_movement) {
			stid = ani->_movement->_nextStatic->_staticsId;
		} else {
			if (!ani->_statics)
				return nullptr;

			stid = ani->_statics->_staticsId;
		}
#if 0
	}
#endif

	if (stid == staticsIndex)
		return new MessageQueue(g_nmi->_globalMessageQueueList->compact());

	int startidx = getStaticIndex(idx, stid);
	int endidx = getStaticIndex(idx, staticsIndex);
	int subidx = startidx + endidx * _items[idx].statics.size();

	if (!_items[idx].transitions[subidx].movement) {
		clearVisitsList(idx);
		seekWay(idx, startidx, endidx, 0, 1);
	}

	if (!_items[idx].transitions[subidx].movement)
		return nullptr;

	MessageQueue *mq = new MessageQueue(g_nmi->_globalMessageQueueList->compact());
	Common::Point point;
	Command *ex;

#if 0
	int i = 0;
#endif

	do {
		subidx = startidx + endidx * _items[idx].statics.size();

		point = _items[idx].transitions[subidx].movement->getMovementSize(0, -1);

#if 0
		if (pointArr) {
			int sz;

			if (_items[idx].transitions[subidx].movement->_currMovement)
				sz = _items[idx].transitions[subidx].movement->_currMovement->_phases.size();
			else
				sz = _items[idx].transitions[subidx].movement->_phases.size();

			ex = new ExCommand2(20, ani->_id, &pointArr[i], sz);

			ex->_messageNum = _items[idx].transitions[subidx].movement->_id;
		} else {
#endif
			ex = new Command(ani->_id, 1, _items[idx].transitions[subidx].movement->_id, 0, 0, 0, 1, 0, 0, 0);
#if 0
		}
#endif

		ex->_param = ani->_copyId;
		ex->_field_3C = 1;
		ex->_wait = 1;

		mq->addCommandToEnd(ex);

#if 0
		if (resStatId)
			*resStatId = _items[idx].transitions[subidx].movement->_id;
#endif

		startidx = _items[idx].transitions[subidx].staticsIndex;

#if 0
		uint step;

		if (_items[idx].transitions[subidx].movement->_currMovement)
			step = _items[idx].transitions[subidx].movement->_currMovement->_phases.size();
		else
			step = _items[idx].transitions[subidx].movement->_phases.size();

		i += step;
#endif
	} while (startidx != endidx);

	return mq;
}

MGMItem::MGMItem() {
	objId = 0;
}

MGMSubItem::MGMSubItem() {
	movement = nullptr;
	staticsIndex = 0;
	movementCount = 0;
	frameCount = 0;
	x = 0;
	y = 0;
}

void AniHandler::attachObject(int objId) {
	debugC(4, kDebugPathfinding, "AniHandler::addItem(%d)", objId);

	if (getIndex(objId) == -1) {
		_items.push_back(MGMItem());
		_items.back().objId = objId;
	}
	resetData(objId);
}

void AniHandler::resetData(int objId) {
	int idx = getIndex(objId);

	if (idx == -1)
		return;

	debugC(3, kDebugPathfinding, "AniHandler::resetData. (1) movements sz: %d visitedMovements sz: %d", _items[idx].movements.size(), _items[idx].visitedMovements.size());

	_items[idx].transitions.clear();
	_items[idx].statics.clear();
	_items[idx].movements.clear();
	_items[idx].visitedMovements.clear();

	AniObject *obj = g_nmi->_currentScene->findAniObjectById(objId, -1);

	if (!obj)
		return;

	debugC(1, kDebugPathfinding, "WWW rebuild. idx: %d, size: %d", idx, obj->_staticsList.size() * obj->_staticsList.size());
	for (uint i = 0; i < obj->_staticsList.size(); i++) {
		_items[idx].statics.push_back(obj->_staticsList[i]);

		for (uint j = 0; j < obj->_staticsList.size(); j++) // Yes, square
			_items[idx].transitions.push_back(MGMSubItem());
	}

	for (uint i = 0; i < obj->_movements.size(); i++) {
		_items[idx].movements.push_back(obj->_movements[i]);
		_items[idx].visitedMovements.push_back(0);
	}

	debugC(3, kDebugPathfinding, "AniHandler::resetData. (2) movements sz: %d visitedMovements sz: %d", _items[idx].movements.size(), _items[idx].visitedMovements.size());
}

int AniHandler::getIndex(int objId) {
	for (uint i = 0; i < _items.size(); i++)
		if (_items[i].objId == objId)
			return i;

	return -1;
}

MessageQueue *AniHandler::makeRunQueue(MakeQueueStruct *mkQueue) {
	debugC(4, kDebugPathfinding, "AniHandler::makeRunQueue(*%d)", mkQueue->ani ? mkQueue->ani->_id : -1);

	if (!mkQueue->ani)
		return nullptr;

	Movement *mov = mkQueue->ani->_movement;

	if (!mov && !mkQueue->ani->_statics)
		return nullptr;

	if (!(mkQueue->flags & 1)) {
		if (mov)
			mkQueue->staticsId1 = mov->_nextStatic->_staticsId;
		else
			mkQueue->staticsId1 = mkQueue->ani->_statics->_staticsId;
	}

	Common::Point point;

	if (!(mkQueue->flags & 0x10) || !(mkQueue->flags & 0x20)) {
		int nx = mkQueue->ani->_x;
		int ny = mkQueue->ani->_y;

		if (mkQueue->ani->_movement) {
			mkQueue->ani->calcNextStep(&point);

			nx += point.x;
			ny += point.y;
		}

		if (!(mkQueue->flags & 0x10))
			mkQueue->x2 = nx;

		if (!(mkQueue->flags & 0x20))
			mkQueue->y2 = ny;
	}

	mov = mkQueue->ani->getMovement(mkQueue->movementId);

	if (!mov)
		return nullptr;


	int itemIdx = getIndex(mkQueue->ani->_id);
	int subIdx = getStaticIndex(itemIdx, mkQueue->staticsId1);
	int st2idx = getStaticIndex(itemIdx, mov->_prevStatic->_staticsId);
	int st1idx = getStaticIndex(itemIdx, mov->_nextStatic->_staticsId);
	int subOffset = getStaticIndex(itemIdx, mkQueue->staticsId2);

	debugC(3, kDebugPathfinding, "AniHandler::genMovement. (1) movements sz: %d visitedMovements sz: %d", _items[itemIdx].movements.size(), _items[itemIdx].visitedMovements.size());

	clearVisitsList(itemIdx);
	seekWay(itemIdx, subIdx, st2idx, 0, 1);
	clearVisitsList(itemIdx);
	seekWay(itemIdx, st1idx, subOffset, 0, 1);

	const MGMSubItem &sub1 = _items[itemIdx].transitions[subIdx + st2idx * _items[itemIdx].statics.size()];
	const MGMSubItem &sub2 = _items[itemIdx].transitions[st1idx + subOffset * _items[itemIdx].statics.size()];

	if (subIdx != st2idx && !sub1.movement)
		return nullptr;

	if (st1idx != subOffset && !sub2.movement)
		return nullptr;

	int n1x = mkQueue->x1 - mkQueue->x2 - sub1.x - sub2.x;
	int n1y = mkQueue->y1 - mkQueue->y2 - sub1.y - sub2.y;

	const Common::Point point1 = mov->getMovementSize(0, -1);

	int n2x = point1.x;
	int n2y = point1.y;
	int mult;
	int len = -1;

	if (mkQueue->flags & 0x40) {
		mult = mkQueue->cycleCount;
		len = -1;
		n2x *= mult;
		n2y *= mult;
	} else {
		point = getNumCycles(mov, n1x, n1y, &mult, &len, 1);
		n2x = point.x;
		n2y = point.y;
	}

	if (!(mkQueue->flags & 2)) {
		len = -1;
		n2x = mult * point1.x;
		n1x = mult * point1.x;
		mkQueue->x1 = mkQueue->x2 + mult * point1.x + sub1.x + sub2.x;
	}

	if (!(mkQueue->flags & 4)) {
		n2y = mult * point1.y;
		n1y = mult * point1.y;
		len = -1;
		mkQueue->y1 = mkQueue->y2 + mult * point1.y + sub1.y + sub2.y;
	}

	int px = 0;
	int py = 0;

	if (sub1.movement) {
		px = getFramesCount(itemIdx, subIdx, st2idx, 1);
		py = getFramesCount(itemIdx, subIdx, st2idx, 2);
	}

	if (mult > 1) {
		px += (mult - 1) * mov->countPhasesWithFlag(-1, 1);
		py += (mult - 1) * mov->countPhasesWithFlag(-1, 2);
	}

	if (mult > 0) {
		px += mov->countPhasesWithFlag(len, 1);
		py += mov->countPhasesWithFlag(len, 2);
	}

	if (sub2.movement) {
		px += getFramesCount(itemIdx, st1idx, subOffset, 1);
		py += getFramesCount(itemIdx, st1idx, subOffset, 2);
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

	MessageQueue *mq = new MessageQueue(g_nmi->_globalMessageQueueList->compact());
	ExCommand2 *ex2;

	for (int i = subIdx; i != st2idx;) {
		const MGMSubItem &s = _items[itemIdx].transitions[i + st2idx * _items[itemIdx].statics.size()];

		ex2 = createCommand(s.movement, mkQueue->ani->_id, x1, y1, x2, y2, -1);
		ex2->_parId = mq->_id;
		ex2->_param = mkQueue->ani->_copyId;

		mq->addCommandToEnd(ex2);

		i = s.staticsIndex;
	}

	for (int i = 0; i < mult; ++i) {
		int plen;

		if (i == mult - 1)
			plen = len;
		else
			plen = -1;

		ex2 = createCommand(mov, mkQueue->ani->_id, x1, y1, x2, y2, plen);
		ex2->_parId = mq->_id;
		ex2->_param = mkQueue->ani->_copyId;

		mq->addCommandToEnd(ex2);
	}

	for (int j = st1idx; j != subOffset;) {
		const MGMSubItem &s = _items[itemIdx].transitions[j + subOffset * _items[itemIdx].statics.size()];

		ex2 = createCommand(s.movement, mkQueue->ani->_id, x1, y1, x2, y2, -1);
		ex2->_parId = mq->_id;
		ex2->_param = mkQueue->ani->_copyId;

		mq->addCommandToEnd(ex2);

		j = s.staticsIndex;
	}

	Command *ex = new Command(mkQueue->ani->_id, 5, -1, mkQueue->x1, mkQueue->y1, 0, 1, 0, 0, 0);

	ex->_z = mkQueue->destZ;
	ex->_param = mkQueue->ani->_copyId;
	ex->_wait = 0;
	ex->_excFlags |= 3;

	mq->addCommandToEnd(ex);

	debugC(3, kDebugPathfinding, "AniHandler::genMovement. (2) movements sz: %d visitedMovements sz: %d", _items[itemIdx].movements.size(), _items[itemIdx].visitedMovements.size());

	return mq;
}

int AniHandler::getFramesCount(int idx, int subIdx, int endIdx, int flag) {
	int res = 0;

	if (endIdx < 0)
		return 0;

	while (subIdx != endIdx) {
		if (subIdx < 0)
			break;

		res += _items[idx].transitions[subIdx + endIdx * _items[idx].statics.size()].movement->countPhasesWithFlag(0xffffffff, flag);

		subIdx = _items[idx].transitions[subIdx + endIdx * _items[idx].statics.size()].staticsIndex;
	}

	return res;
}
void AniHandler::putObjectToStatic(AniObject *ani, int staticsId) {
	debugC(4, kDebugPathfinding, "AniHandler::putObjectToStatic(*%d, %d)", ani->_id, staticsId);

	if (getIndex(ani->_id) == -1)
		return;

	if (ani->_movement) {
		ani->setQueue(nullptr);
		ani->_movement->setLast();
		ani->_statics = ani->_movement->_nextStatic;

		int x = ani->_movement->_x;
		int y = ani->_movement->_y;

		ani->_movement = nullptr;

		ani->setPos(x, y);
	}

	if (ani->_statics) {
		const Common::Point point = getTransitionSize(ani->_id, ani->_statics->_staticsId, staticsId);

		ani->setPos(ani->_x + point.x, ani->_y + point.y);

		ani->_statics = ani->getStatic(staticsId);
	}
}

Common::Point AniHandler::getTransitionSize(int objectId, int staticsId1, int staticsId2) {
	debugC(4, kDebugPathfinding, "AniHandler::getTransitionSize(%d, %d, %d)", objectId, staticsId1, staticsId2);

	int idx = getIndex(objectId);

	if (idx == -1) {
		return Common::Point(-1, -1);
	}

	int st1idx = getStaticIndex(idx, staticsId1);
	int st2idx = getStaticIndex(idx, staticsId2);

	if (st1idx == st2idx) {
		return Common::Point(0, 0);
	}

	int subidx = st1idx + st2idx * _items[idx].statics.size();

	if (!_items[idx].transitions[subidx].movement) {
		clearVisitsList(idx);
		seekWay(idx, st1idx, st2idx, false, true);

		if (!_items[idx].transitions[subidx].movement) {
			clearVisitsList(idx);
			seekWay(idx, st1idx, st2idx, true, false);
		}
	}

	const MGMSubItem &sub = _items[idx].transitions[subidx];

	if (!sub.movement) {
		return Common::Point(0, 0);
	}

	return Common::Point(sub.x, sub.y);
}

int AniHandler::getStaticIndex(int idx, int16 id) {
	if (!_items[idx].statics.size())
		return -1;

	for (uint i = 0; i < _items[idx].statics.size(); i++) {
		if (_items[idx].statics[i]->_staticsId == id)
			return i;
	}

	return -1;
}

int AniHandler::getStaticIndex(int idx, Statics *st) {
	if (!_items[idx].statics.size())
		return -1;

	for (uint i = 0; i < _items[idx].statics.size(); i++) {
		if (_items[idx].statics[i] == st)
			return i;
	}

	return -1;
}

void AniHandler::clearVisitsList(int idx) {
	debugC(2, kDebugPathfinding, "AniHandler::clearVisitsList(%d)", idx);

	for (uint i = 0; i < _items[idx].visitedMovements.size(); i++)
		_items[idx].visitedMovements[i] = 0;

	debugC(3, kDebugPathfinding, "AniHandler::clearVisitsList. movements sz: %d visitedMovements sz: %d", _items[idx].movements.size(), _items[idx].visitedMovements.size());
}

int AniHandler::seekWay(int idx, int st1idx, int st2idx, bool allowReverse, bool linkedOnly) {
	MGMItem &item = _items[idx];
	int subIdx = st1idx + st2idx * item.statics.size();

	debugC(2, kDebugPathfinding, "AniHandler::seekWay(%d, %d, %d, %d, %d)", idx, st1idx, st2idx, allowReverse, linkedOnly);

	if (st1idx == st2idx) {
		item.transitions[subIdx].reset();
		return 0;
	}

	if (item.transitions[subIdx].movement)
		return item.transitions[subIdx].movementCount;

	Common::Point point;

	debugC(3, kDebugPathfinding, "AniHandler::seekWay. movements sz: %d visitedMovements sz: %d", item.movements.size(), item.visitedMovements.size());

	for (uint i = 0; i < item.movements.size(); i++) {
		Movement *mov = item.movements[i];

		if (mov->_prevStatic == item.statics[st1idx]) {
			if (item.visitedMovements[i] || (linkedOnly && !mov->_link))
				continue;

			item.visitedMovements[i] = 1;

			int stidx = getStaticIndex(idx, mov->_nextStatic);
			int recalc = seekWay(idx, stidx, st2idx, allowReverse, linkedOnly);
			int sz = mov->_currMovement ? mov->_currMovement->_phases.size() : mov->_phases.size();
			debugC(1, kDebugPathfinding, "AniHandler::seekWay, want idx: %d, off: %d (%d + %d), sz: %d", idx, stidx + st2idx * _items[idx].statics.size(), stidx, st2idx, item.transitions.size());

			int newsz = sz + item.transitions[stidx + st2idx * _items[idx].statics.size()].frameCount;

			if (recalc < 0)
				continue;

			if (!item.transitions[subIdx].movement || item.transitions[subIdx].movementCount > recalc + 1 ||
				(item.transitions[subIdx].movementCount == recalc + 1 && item.transitions[subIdx].frameCount > newsz)) {
				item.transitions[subIdx].movement = mov;
				item.transitions[subIdx].staticsIndex = stidx;
				item.transitions[subIdx].movementCount = recalc + 1;
				item.transitions[subIdx].frameCount = newsz;

				point = mov->getMovementSize(0, -1);

				item.transitions[subIdx].x = item.transitions[stidx + st2idx * _items[idx].statics.size()].x + point.x;
				item.transitions[subIdx].y = item.transitions[stidx + st2idx * _items[idx].statics.size()].y + point.y;
			}
		} else if (allowReverse) {
			if (mov->_nextStatic != item.statics[st1idx])
				continue;

			if (item.visitedMovements[i] || (linkedOnly && !mov->_link))
				continue;

			item.visitedMovements[i] = 1;

			int stidx = getStaticIndex(idx, mov->_prevStatic);
			int recalc = seekWay(idx, stidx, st2idx, allowReverse, linkedOnly);

			if (recalc < 0)
				continue;

			if (!item.transitions[subIdx].movement || item.transitions[subIdx].movementCount > recalc + 1) {
				item.transitions[subIdx].movement = mov;
				item.transitions[subIdx].staticsIndex = stidx;
				item.transitions[subIdx].movementCount = recalc + 1;

				int sz = mov->_currMovement ? mov->_currMovement->_phases.size() : mov->_phases.size();

				item.transitions[subIdx].frameCount = sz + item.transitions[stidx + st2idx * _items[idx].statics.size()].frameCount;

				point = mov->getMovementSize(0, -1);

				item.transitions[subIdx].x = item.transitions[stidx + st2idx * _items[idx].statics.size()].x - point.x;
				item.transitions[subIdx].y = item.transitions[stidx + st2idx * _items[idx].statics.size()].y - point.y;
			}
		}
	}

	if (item.transitions[subIdx].movement)
		return item.transitions[subIdx].movementCount;

	return -1;
}

int AniHandler::getNumMovements(int objectId, int idx1, int idx2) {
	debugC(4, kDebugPathfinding, "AniHandler::getNumMovements(%d, %d, %d)", objectId, idx1, idx2);

	int idx = getIndex(objectId);

	if (idx != -1) {
		int from = getStaticIndex(idx, idx1);
		int to = getStaticIndex(idx, idx2);

		debugC(1, kDebugPathfinding, "WWW 6, want idx: %d, off: %d", idx, from + to * _items[idx].statics.size());
		const MGMSubItem &sub = _items[idx].transitions[from + to * _items[idx].statics.size()];

		if (sub.movement) {
			idx = sub.movementCount;
		} else {
			clearVisitsList(idx);
			idx = seekWay(idx, from, to, 0, 1);
		}
	}

	return idx;
}

Common::Point AniHandler::getNumCycles(Movement *mov, int x, int y, int *mult, int *len, int flag) {
	Common::Point point = mov->getMovementSize(0, -1);
	int p1x = point.x;
	int p1y = point.y;

	int newmult = 0;

	if (abs(p1y) > abs(p1x)) {
		if (mov->getMovementSize(0, -1).y)
			newmult = (int)((double)y / mov->getMovementSize(0, -1).y);
	} else if (mov->getMovementSize(0, -1).x) {
		newmult = (int)((double)x / mov->getMovementSize(0, -1).x);
	}

	if (newmult < 0)
		newmult = 0;

	*mult = newmult;

	int phase = 1;
	int sz;

	if (flag) {
		if (abs(p1y) > abs(p1x)) {
			while (abs(p1y * newmult + mov->getMovementSize(0, phase).y) < abs(y)) {
				sz = mov->_currMovement ? mov->_currMovement->_phases.size() : mov->_phases.size();

				if (phase > sz)
					break;

				phase++;
			}
		} else {
			while (abs(p1x * newmult + mov->getMovementSize(0, phase).x) < abs(x)) {
				sz = mov->_currMovement ? mov->_currMovement->_phases.size() : mov->_phases.size();

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

	if (!*len)
		*len = -1;

	if (*len > 0) {
		++*mult;

		point = mov->getMovementSize(0, *len);
		p2x = point.x;
		p2y = point.y;

		if (abs(p1y) > abs(p1x))
			p2x = p1x;
		else
			p2y = p1y;
	}

	return Common::Point(p2x + p1x * newmult, p2y + p1y * newmult);
}

ExCommand2 *AniHandler::createCommand(Movement *mov, int objId, int x1, int y1, Common::Point &x2, Common::Point &y2, int len) {
	debugC(2, kDebugPathfinding, "AniHandler::createCommand(mov, %d, %d, %d, [%d, %d], [%d, %d], %d)", objId, x1, y1, x2.x, x2.y, y2.x, y2.y, len);

	uint cnt;

	if (mov->_currMovement)
		cnt = mov->_currMovement->_phases.size();
	else
		cnt = mov->_phases.size();

	if (len > 0 && cnt > (uint)len)
		cnt = len;

	PointList points(cnt);

	for (uint i = 0; i < cnt; i++) {
		int flags = mov->getPhase(i)->getDynFlags();

		if (flags & 1) {
			points[i].x = x1 + x2.x;

			y2.x -= x2.x;

			if (!y2.x)
				x2.x = 0;
		}

		if (flags & 2) {
			points[i].y = y1 + x2.y;

			y2.y -= x2.y;

			if (!y2.y)
				x2.y = 0;
		}
	}

	ExCommand2 *ex = new ExCommand2(20, objId, points);
	ex->_excFlags = 2;
	ex->_messageNum = mov->_id;
	ex->_z = len;
	ex->_wait = 1;
	ex->_param = -1;

	return ex;
}

} // End of namespace NGI
