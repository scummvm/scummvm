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

#include "ngi/ngi.h"

#include "ngi/utils.h"
#include "ngi/statics.h"
#include "ngi/gameloader.h"
#include "ngi/motion.h"

namespace NGI {

bool MotionController::load(MfcArchive &file) {
	// Is originally empty	file.readClass();

	debugC(5, kDebugLoading, "MotionController::load()");

	return true;
}

void MotionController::enableLinks(const char *linkName, bool enable) {
	if (_objtype != kObjTypeMctlCompound)
		return;

	MctlCompound *obj = static_cast<MctlCompound *>(this);

	for (uint i = 0;  i < obj->getMotionControllerCount(); i++) {
		MotionController *con = obj->getMotionController(i);

		if (con->_objtype == kObjTypeMovGraph) {
			MovGraph *gr = static_cast<MovGraph *>(con);

			for (MovGraph::LinkList::iterator l = gr->_links.begin(); l != gr->_links.end(); ++l) {
				assert((*l)->_objtype == kObjTypeMovGraphLink);

				MovGraphLink *lnk = static_cast<MovGraphLink *>(*l);

				if (lnk->_name == linkName) {
					if (enable)
						lnk->_flags |= 0x20000000;
					else
						lnk->_flags &= 0xDFFFFFFF;
				}
			}
		}
	}
}

MovGraphLink *MotionController::getLinkByName(const char *name) {
	debugC(4, kDebugPathfinding, "MotionController::getLinkByName(%s)", name);

	if (_objtype == kObjTypeMctlCompound) {
		MctlCompound *obj = static_cast<MctlCompound *>(this);

		for (uint i = 0;  i < obj->getMotionControllerCount(); i++) {
			MotionController *con = obj->getMotionController(i);

			if (con->_objtype == kObjTypeMovGraph) {
				MovGraph *gr = static_cast<MovGraph *>(con);

				for (MovGraph::LinkList::iterator l = gr->_links.begin(); l != gr->_links.end(); ++l) {
					assert((*l)->_objtype == kObjTypeMovGraphLink);

					MovGraphLink *lnk = static_cast<MovGraphLink *>(*l);

					if (lnk->_name == name)
						return lnk;
				}
			}
		}
	}

	if (_objtype == kObjTypeMovGraph) {
		MovGraph *gr = static_cast<MovGraph *>(this);

		for (MovGraph::LinkList::iterator l = gr->_links.begin(); l != gr->_links.end(); ++l) {
			assert((*l)->_objtype == kObjTypeMovGraphLink);

			MovGraphLink *lnk = static_cast<MovGraphLink *>(*l);

			if (lnk->_name == name)
				return lnk;
		}
	}

	return 0;
}

MctlCompound::~MctlCompound() {
	Common::for_each(_motionControllers.begin(), _motionControllers.end(), Common::DefaultDeleter<MctlItem>());
}

bool MctlCompound::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "MctlCompound::load()");

	int count = file.readUint32LE();

	debugC(6, kDebugLoading, "MctlCompound::count = %d", count);

	for (int i = 0; i < count; i++) {
		debugC(6, kDebugLoading, "CompoundArray[%d]", i);
		MctlItem *obj = new MctlItem();

		obj->_motionControllerObj.reset(file.readClass<MotionController>());

		int count1 = file.readUint32LE();

		debugC(6, kDebugLoading, "ConnectionPoint::count: %d", count1);
		for (int j = 0; j < count1; j++) {
			debugC(6, kDebugLoading, "ConnectionPoint[%d]", j);
			MctlConnectionPoint *obj1 = file.readClass<MctlConnectionPoint>();

			obj->_connectionPoints.push_back(obj1);
		}

		obj->_field_20 = file.readUint32LE();
		obj->_field_24 = file.readUint32LE();

		debugC(6, kDebugLoading, "graphReact");
		obj->_movGraphReactObj.reset(file.readClass<MovGraphReact>());

		_motionControllers.push_back(obj);
	}

	return true;
}

void MctlCompound::attachObject(StaticANIObject *obj) {
	debugC(4, kDebugPathfinding, "MctlCompound::attachObject(*%d)", obj->_id);

	for (uint i = 0; i < _motionControllers.size(); i++)
		_motionControllers[i]->_motionControllerObj->attachObject(obj);
}

int MctlCompound::detachObject(StaticANIObject *obj) {
	debugC(4, kDebugPathfinding, "MctlCompound::detachObject(*%d)", obj->_id);

	for (uint i = 0; i < _motionControllers.size(); i++)
		_motionControllers[i]->_motionControllerObj->detachObject(obj);

	return 1;
}

void MctlCompound::initMctlGraph() {
	if (_objtype != kObjTypeMctlCompound)
		return;

	debugC(4, kDebugPathfinding, "MctlCompound::initMctlGraph()");

	for (uint i = 0; i < _motionControllers.size(); i++) {
		if (_motionControllers[i]->_motionControllerObj->_objtype != kObjTypeMovGraph)
			continue;

		MovGraph *gr = static_cast<MovGraph *>(_motionControllers[i]->_motionControllerObj.get());

		MctlGraph *newgr = new MctlGraph();

		newgr->_links = gr->_links;
		gr->_links.clear();
		newgr->_nodes = gr->_nodes;
		gr->_nodes.clear();

		_motionControllers[i]->_motionControllerObj.reset(newgr);
	}
}

void MctlCompound::detachAllObjects() {
	debugC(4, kDebugPathfinding, "MctlCompound::detachAllObjects()");

	for (uint i = 0; i < _motionControllers.size(); i++)
		_motionControllers[i]->_motionControllerObj->detachAllObjects();
}

MessageQueue *MctlCompound::startMove(StaticANIObject *ani, int sourceX, int sourceY, int fuzzyMatch, int staticsId) {
	int idx = -1;
	int sourceIdx = -1;

	debugC(4, kDebugPathfinding, "MctlCompound::startMove(*%d, %d, %d, %d, %d)", (ani ? ani->_id : -1), sourceX, sourceY, fuzzyMatch, staticsId);

	if (!ani)
		return 0;

	for (uint i = 0; i < _motionControllers.size(); i++) {
		if (_motionControllers[i]->_movGraphReactObj) {
			if (_motionControllers[i]->_movGraphReactObj->pointInRegion(ani->_ox, ani->_oy)) {
				idx = i;
				break;
			}
		}
	}

	for (uint i = 0; i < _motionControllers.size(); i++) {
		if (_motionControllers[i]->_movGraphReactObj) {
			if (_motionControllers[i]->_movGraphReactObj->pointInRegion(sourceX, sourceY)) {
				sourceIdx = i;
				break;
			}
		}
	}

	if (idx == -1)
		return 0;

	if (sourceIdx == -1)
		return 0;

	debugC(1, kDebugPathfinding, "WWW 2");
	if (idx == sourceIdx)
		return _motionControllers[idx]->_motionControllerObj->startMove(ani, sourceX, sourceY, fuzzyMatch, staticsId);

	double dist;
	MctlConnectionPoint *cp = findClosestConnectionPoint(ani->_ox, ani->_oy, idx, sourceX, sourceY, sourceIdx, &dist);

	if (!cp)
		return 0;

	MessageQueue *mq = _motionControllers[idx]->_motionControllerObj->makeQueue(ani, cp->_connectionX, cp->_connectionY, 1, cp->_mctlmirror);

	if (!mq)
		return 0;

	for (uint i = 0; i < cp->_messageQueueObj->getCount(); i++) {
		ExCommand *ex = new ExCommand(cp->_messageQueueObj->getExCommandByIndex(i));

		ex->_excFlags |= 2;

		mq->addExCommandToEnd(ex);
	}

	ExCommand *ex = new ExCommand(ani->_id, 51, 0, sourceX, sourceY, 0, 1, 0, 0, 0);

	ex->_excFlags |= 2;
	ex->_invId = fuzzyMatch;
	ex->_param = ani->_odelay;

	mq->addExCommandToEnd(ex);

	if (!mq->chain(ani)) {
		delete mq;
		return 0;
	}

	return mq;
}

MessageQueue *MctlCompound::makeQueue(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	int match1 = -1;
	int match2 = -1;

	debugC(1, kDebugPathfinding, "MctlCompound::makeQueue(*%d, %d, %d, %d, %d)", (subj ? subj->_id : -1), xpos, ypos, fuzzyMatch, staticsId);

	if (!subj)
		return 0;

	for (uint i = 0; i < _motionControllers.size(); i++) {
		if (_motionControllers[i]->_movGraphReactObj) {
			if (_motionControllers[i]->_movGraphReactObj->pointInRegion(subj->_ox, subj->_oy)) {
				match1 = i;
				break;
			}
		}
	}

	if (match1 == -1)
		return 0;

	for (uint i = 0; i < _motionControllers.size(); i++) {
		if (_motionControllers[i]->_movGraphReactObj) {
			if (_motionControllers[i]->_movGraphReactObj->pointInRegion(xpos, ypos)) {
				match2 = i;
				break;
			}
		}
	}

	if (match2 == -1)
		return 0;

	if (match1 == match2)
		return _motionControllers[match1]->_motionControllerObj->makeQueue(subj, xpos, ypos, fuzzyMatch, staticsId);

	double dist;
	MctlConnectionPoint *closestP = findClosestConnectionPoint(subj->_ox, subj->_oy, match1, xpos, ypos, match2, &dist);

	if (!closestP)
		return 0;

	MessageQueue *mq = _motionControllers[match1]->_motionControllerObj->makeQueue(subj, closestP->_connectionX, closestP->_connectionY, 1, closestP->_mctlmirror);

	ExCommand *ex;

	if (mq) {
		for (uint i = 0; i < closestP->_messageQueueObj->getCount(); i++) {
			ex = closestP->_messageQueueObj->getExCommandByIndex(i)->createClone();
			ex->_excFlags |= 2;
			mq->addExCommandToEnd(ex);
		}

		ex = new ExCommand(subj->_id, 51, 0, xpos, ypos, 0, 1, 0, 0, 0);

		ex->_invId = fuzzyMatch;
		ex->_param = subj->_odelay;
		ex->_excFlags |= 2;

		mq->addExCommandToEnd(ex);
	}

	return mq;
}

MctlLadder::MctlLadder() {
	_width = 0;
	_ladderX = 0;
	_height = 0;
	_ladderY = 0;
	_ladder_field_14 = 0;

	_ladder_field_20 = 0;
	_ladder_field_24 = 0;
}

MctlLadder::~MctlLadder() {
	detachAllObjects();
}

int MctlLadder::collisionDetection(StaticANIObject *man) {
	if (findObjectPos(man) < 0)
		return 0;

	double delta;

	if ((double)(man->_oy - _ladderY) / (double)_height < 0.0)
		delta = -0.5;
	else
		delta = 0.5;

	int res = (int)((double)(man->_oy - _ladderY) / (double)_height + delta);

	if (res < 0)
		return 0;

	return res;
}

void MctlLadder::attachObject(StaticANIObject *obj) {
	debugC(4, kDebugPathfinding, "MctlLadder::attachObject(*%d)", obj->_id);

	if (findObjectPos(obj) < 0) {
		MctlLadderMovement *movement = new MctlLadderMovement;

		if (initMovement(obj, movement)) {
			_aniHandler.attachObject(obj->_id);
			_ladmovements.push_back(movement);
		} else {
			delete movement;
		}
	}
}

int MctlLadder::findObjectPos(StaticANIObject *obj) {
	for (uint i = 0; i < _ladmovements.size(); i++)
		if (_ladmovements[i]->objId == obj->_id)
			return i;

	return -1;
}

bool MctlLadder::initMovement(StaticANIObject *ani, MctlLadderMovement *movement) {
	debugC(4, kDebugPathfinding, "MctlLadder::initMovement(*%d, ...)", ani->_id);

	GameVar *v = g_nmi->getGameLoaderGameVar()->getSubVarByName(ani->getName());

	if (!v)
		return false;

	GameVar *l = v->getSubVarByName("Test_Ladder");

	if (!l)
		return false;

	movement->staticIdsSize = 6;
	movement->movVars = new MctlLadderMovementVars;
	movement->staticIds = new int[movement->staticIdsSize];

	v = l->getSubVarByName("Up");

	if (!v)
		return false;

	movement->movVars->varUpStart = v->getSubVarAsInt("Start");
	movement->movVars->varUpGo = v->getSubVarAsInt("Go");
	movement->movVars->varUpStop = v->getSubVarAsInt("Stop");

	movement->staticIds[0] = ani->getMovementById(movement->movVars->varUpStart)->_staticsObj1->_staticsId;
	movement->staticIds[2] = ani->getMovementById(movement->movVars->varUpGo)->_staticsObj1->_staticsId;

	v = l->getSubVarByName("Down");

	if (!v)
		return false;

	movement->movVars->varDownStart = v->getSubVarAsInt("Start");
	movement->movVars->varDownGo = v->getSubVarAsInt("Go");
	movement->movVars->varDownStop = v->getSubVarAsInt("Stop");

	movement->staticIds[1] = ani->getMovementById(movement->movVars->varDownStart)->_staticsObj1->_staticsId;
	movement->staticIds[3] = ani->getMovementById(movement->movVars->varDownGo)->_staticsObj1->_staticsId;

	movement->objId = ani->_id;

	return true;
}

void MctlLadder::detachAllObjects() {
	debugC(4, kDebugPathfinding, "MctlLadder::detachAllObjects()");

	_aniHandler.detachAllObjects();

	for (uint i = 0; i < _ladmovements.size(); i++) {
		delete _ladmovements[i]->movVars;
		delete[] _ladmovements[i]->staticIds;
	}

	_ladmovements.clear();
}

MessageQueue *MctlLadder::startMove(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	debugC(4, kDebugPathfinding, "MctlLadder::startMove(*%d, %d, %d, %d, %d)", (subj ? subj->_id : -1), xpos, ypos, fuzzyMatch, staticsId);

	MessageQueue *mq = makeQueue(subj, xpos, ypos, fuzzyMatch, staticsId);

	if (mq) {
		if (mq->chain(subj))
			return mq;
	}

	return 0;
}

MessageQueue *MctlLadder::makeQueue(StaticANIObject *ani, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	debugC(1, kDebugPathfinding, "MctlLadder::makeQueue(*%d, %d, %d, %d, %d)", (ani ? ani->_id : -1), xpos, ypos, fuzzyMatch, staticsId);

	int pos = findObjectPos(ani);

	if (pos < 0)
		return 0;

	double dh = _height;
	double corr = (double)(ani->_oy - _ladderY) / dh;
	int dl = (int)(corr + (corr < 0.0 ? -0.5 : 0.5));

	corr = (double)(ypos - _ladderY) / dh;
	int dl2 = (int)(corr + (corr < 0.0 ? -0.5 : 0.5));

	int normx = _ladderX + dl2 * _width;
	int normy = _ladderY + dl2 * _height;

	if (dl == dl2 || dl2 < 0)
		return 0;

	int direction = (normy - ani->_oy) < 0 ? 0 : 1;

	MakeQueueStruct mkQueue;
	PicAniInfo picinfo;
	MessageQueue *mq;
	ExCommand *ex;
	Common::Point point;

	if (ani->_movement) {
		ani->getPicAniInfo(picinfo);

		int ox = ani->_ox;
		int oy = ani->_oy;

		point = ani->_movement->calcSomeXY(1, ani->_someDynamicPhaseIndex);
		ani->_statics = ani->_movement->_staticsObj2;
		ani->_movement = 0;
		ani->setOXY(point.x + ox, point.y + oy);

		mq = makeQueue(ani, normx, normy, fuzzyMatch, staticsId);

		ani->setPicAniInfo(picinfo);

		return mq;
	}

	if (ani->_statics->_staticsId == _ladmovements[pos]->staticIds[0]) {
		mkQueue.ani = ani;

		if (staticsId)
			mkQueue.staticsId2 = staticsId;
		else
			mkQueue.staticsId2 = _ladmovements[pos]->staticIds[direction];

		mkQueue.x1 = normx;
		mkQueue.y1 = normy;
		mkQueue.field_1C = _ladder_field_14;
		mkQueue.flags = 14;
		mkQueue.movementId = direction ? _ladmovements[pos]->movVars->varDownGo : _ladmovements[pos]->movVars->varUpGo;

		return _aniHandler.makeRunQueue(&mkQueue);
	}

	if (ani->_statics->_staticsId == _ladmovements[pos]->staticIds[2]) {
		if (!direction) {
			mkQueue.ani = ani;

			if (staticsId)
				mkQueue.staticsId2 = staticsId;
			else
				mkQueue.staticsId2 = _ladmovements[pos]->staticIds[0];

			mkQueue.x1 = normx;
			mkQueue.y1 = normy;
			mkQueue.field_1C = _ladder_field_14;
			mkQueue.flags = 14;
			mkQueue.movementId = _ladmovements[pos]->movVars->varUpGo;

			return _aniHandler.makeRunQueue(&mkQueue);
		}

		int ox = ani->_ox;
		int oy = ani->_oy;

		point = ani->getMovementById(_ladmovements[pos]->movVars->varUpStop)->calcSomeXY(0, -1);

		mkQueue.ani = ani;

		if (staticsId)
			mkQueue.staticsId2 = staticsId;
		else
			mkQueue.staticsId2 = _ladmovements[pos]->staticIds[1];

		mkQueue.field_1C = _ladder_field_14;
		mkQueue.x1 = normx;
		mkQueue.y1 = normy;
		mkQueue.y2 = point.y + oy;
		mkQueue.x2 = point.x + ox;
		mkQueue.flags = 63;
		mkQueue.staticsId1 = _ladmovements[pos]->staticIds[0];
		mkQueue.movementId = _ladmovements[pos]->movVars->varDownGo;

		mq = _aniHandler.makeRunQueue(&mkQueue);

		ex = new ExCommand(ani->_id, 1, _ladmovements[pos]->movVars->varUpStop, 0, 0, 0, 1, 0, 0, 0);
		ex->_param = ani->_odelay;
		ex->_excFlags |= 2;

		mq->insertExCommandAt(0, ex);

		return mq;
	}

	if (ani->_statics->_staticsId != _ladmovements[pos]->staticIds[3]) {
		mq = _aniHandler.makeQueue(ani, _ladmovements[pos]->staticIds[0], 0, 0, 0);

		if (!mq)
			return 0;

		int nx = ani->_ox;
		int ny = ani->_oy;

		point = _aniHandler.getTransitionSize(ani->_id, ani->_statics->_staticsId, _ladmovements[pos]->staticIds[0]);

		nx += point.x;
		ny += point.y;

		ani->getPicAniInfo(picinfo);

		ani->_statics = ani->getStaticsById(_ladmovements[pos]->staticIds[0]);
		ani->_movement = 0;
		ani->setOXY(nx, ny);

		MessageQueue *newmq = makeQueue(ani, normx, normy, fuzzyMatch, staticsId);

		mq->mergeQueue(newmq);

		delete newmq;

		ani->setPicAniInfo(picinfo);

		return mq;
	}

	if (!direction) {
		int nx = ani->_ox;
		int ny = ani->_oy;

		point = ani->getMovementById(_ladmovements[pos]->movVars->varDownStop)->calcSomeXY(0, -1);

		nx += point.x;
		ny += point.y;

		mkQueue.ani = ani;
		if (staticsId)
			mkQueue.staticsId2 = staticsId;
		else
			mkQueue.staticsId2 = _ladmovements[pos]->staticIds[0];

		mkQueue.field_1C = _ladder_field_14;
		mkQueue.x1 = normx;
		mkQueue.y1 = normy;
		mkQueue.y2 = ny;
		mkQueue.x2 = nx;
		mkQueue.flags = 63;
		mkQueue.staticsId1 = _ladmovements[pos]->staticIds[1];
		mkQueue.movementId = _ladmovements[pos]->movVars->varUpGo;

		mq = _aniHandler.makeRunQueue(&mkQueue);

		ex = new ExCommand(ani->_id, 1, _ladmovements[pos]->movVars->varDownStop, 0, 0, 0, 1, 0, 0, 0);
		ex->_param = ani->_odelay;
		ex->_excFlags |= 2;

		mq->insertExCommandAt(0, ex);

		return mq;
	}


	mkQueue.ani = ani;

	if (staticsId)
		mkQueue.staticsId2 = staticsId;
	else
		mkQueue.staticsId2 = _ladmovements[pos]->staticIds[1];

	mkQueue.x1 = normx;
	mkQueue.y1 = normy;
	mkQueue.field_1C = _ladder_field_14;
	mkQueue.flags = 14;
	mkQueue.movementId = _ladmovements[pos]->movVars->varDownGo;

	return _aniHandler.makeRunQueue(&mkQueue);
}

MessageQueue *MctlLadder::controllerWalkTo(StaticANIObject *ani, int off) {
	return makeQueue(ani, _ladderX + off * _width, _ladderY + off * _height, 1, 0);
}

MctlConnectionPoint *MctlCompound::findClosestConnectionPoint(int ox, int oy, int destIndex, int connectionX, int connectionY, int sourceIdx, double *minDistancePtr) {
	if (destIndex == sourceIdx) {
		*minDistancePtr = sqrt((double)((oy - connectionY) * (oy - connectionY) + (ox - connectionX) * (ox - connectionX)));

		return 0;
	}

	double currDistance = 0.0;
	double minDistance = 1.0e10;
	MctlConnectionPoint *minConnectionPoint = 0;

	for (uint i = 0; i < _motionControllers[sourceIdx]->_connectionPoints.size(); i++) {
		for (uint j = 0; j < _motionControllers.size(); j++) {
			if (_motionControllers[j]->_movGraphReactObj) {
				MctlConnectionPoint *pt = _motionControllers[sourceIdx]->_connectionPoints[i];

				if (_motionControllers[j]->_movGraphReactObj->pointInRegion(pt->_connectionX, pt->_connectionY)) {
					MctlConnectionPoint *npt = findClosestConnectionPoint(ox, oy, destIndex, pt->_connectionX, pt->_connectionY, j, &currDistance);

					if (currDistance < minDistance) {
						minDistance = currDistance;

						if (npt)
							minConnectionPoint = npt;
						else
							minConnectionPoint = pt;
					}
				}
			}
		}
	}

	*minDistancePtr = minDistance;

	return minConnectionPoint;
}

void MctlCompound::replaceNodeX(int from, int to) {
	for (uint i = 0; i < _motionControllers.size(); i++) {
		if (_motionControllers[i]->_motionControllerObj->_objtype == kObjTypeMovGraph) {
			MovGraph *gr = static_cast<MovGraph *>(_motionControllers[i]->_motionControllerObj.get());

			for (MovGraph::NodeList::iterator n = gr->_nodes.begin(); n != gr->_nodes.end(); ++n) {
				MovGraphNode *node = static_cast<MovGraphNode *>(*n);

				if (node->_x == from)
					node->_x = to;
			}

			gr->recalcLinkParams();
		}
	}
}

MctlConnectionPoint::MctlConnectionPoint() {
	_connectionX = 0;
	_connectionY = 0;
	_mctlflags = 0;
	_mctlstatic = 0;
	_mctlmirror = 0;
}

void MctlMQ::clear() {
	index = 0;
	pt1.x = pt1.y = 0;
	pt2.x = pt2.y = 0;
	distance1 = 0;
	distance2 = 0;
	subIndex = 0;
	item1Index = 0;
	items.clear();
	flags = 0;
}

MctlItem::~MctlItem() {
	Common::for_each(_connectionPoints.begin(), _connectionPoints.end(), Common::DefaultDeleter<MctlConnectionPoint>());
}

MovGraphItem::MovGraphItem() {
	ani = 0;
	field_4 = 0;
	mi_movitems = 0;
	count = 0;
	field_30 = 0;
	field_34 = 0;
	field_38 = 0;
	field_3C = 0;
}

void MovGraphItem::free() {
	if (!mi_movitems)
		return;

	for (uint i = 0; i < mi_movitems->size(); i++) {
		(*mi_movitems)[i]->movarr->_movSteps.clear();
		delete (*mi_movitems)[i]->movarr;
	}

	delete mi_movitems;

	mi_movitems = 0;
}

MovArr *movGraphCallback(StaticANIObject *ani, Common::Array<MovItem *> *items, signed int counter) {
	int residx = 0;
	int itemidx = 0;

	while (counter > 1) {
		if ((*items)[itemidx]->_mfield_4 > (*items)[itemidx + 1]->_mfield_4)
			residx = itemidx;

		counter--;
		itemidx++;
	}

	return (*items)[residx]->movarr;
}

MovGraph::MovGraph() {
	_callback1 = movGraphCallback;
	_field_44 = 0;
	insertMessageHandler(MovGraph::messageHandler, getMessageHandlersCount() - 1, 129);

	_objtype = kObjTypeMovGraph;
}

MovGraph::~MovGraph() {
	for (LinkList::iterator i = _links.begin(); i != _links.end(); ++i)
		delete *i;

	for (NodeList::iterator i = _nodes.begin(); i != _nodes.end(); ++i)
		delete *i;

	detachAllObjects();
}

bool MovGraph::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "MovGraph::load()");

	_links.load(file);
	_nodes.load(file);

	return true;
}

void MovGraph::attachObject(StaticANIObject *obj) {
	debugC(4, kDebugPathfinding, "MovGraph::attachObject(*%d)", obj->_id);

	_aniHandler.detachAllObjects();
	_aniHandler.attachObject(obj->_id);

	for (uint i = 0; i < _items.size(); i++)
		if (_items[i].ani == obj)
			return;

	_items.push_back(MovGraphItem());
	MovGraphItem &item = _items.back();
	item.ani = obj;

	_aniHandler.attachObject(obj->_id); // FIXME: Is it really needed?
}

int MovGraph::detachObject(StaticANIObject *obj) {
	warning("STUB: MovGraph::detachObject()");

	return 0;
}

void MovGraph::detachAllObjects() {
	debugC(4, kDebugPathfinding, "MovGraph::detachAllObjects()");

	for (uint i = 0; i < _items.size(); i++) {
		_items[i].free();

		_items[i].movarr._movSteps.clear();
	}

	_items.clear();
}

Common::Array<MovItem *> *MovGraph::getPaths(StaticANIObject *ani, int x, int y, int flag1, int *rescount) {
	debugC(4, kDebugPathfinding, "MovGraph::getPaths(*%d, %d, %d, %d, &rescount)", (ani ? ani->_id : -1), x, y, flag1);

	*rescount = 0;

	if (_items.size() <= 0)
		return 0;

	uint idx = 0;

	while (_items[idx].ani != ani) {
		idx++;

		if (idx >= _items.size())
			return 0;
	}
	_items[idx].free();

	recalcLinkParams();

	_items[idx].movarr._movSteps.clear();

	Common::Point point;

	point.x = ani->_ox;
	point.y = ani->_oy;

	if (!getHitPoint(idx, ani->_ox, ani->_oy, &_items[idx].movarr, 0))
		getNearestPoint(idx, &point, &_items[idx].movarr);

	_items[idx].count = 0;

	delete _items[idx].mi_movitems;
	_items[idx].mi_movitems = 0;

	int arrSize;
	Common::Array<MovArr *> *movarr = getHitPoints(x, y, &arrSize, flag1, 0);

	if (movarr) {
		for (int i = 0; i < arrSize; i++) {
			int sz;
			Common::Array<MovItem *> *movitems = getPaths(&_items[idx].movarr, (*movarr)[i], &sz);

			if (sz > 0) {
				_items[idx].mi_movitems = new Common::Array<MovItem *>;
				for (int j = 0; j < sz; j++)
					_items[idx].mi_movitems->push_back(movitems[j]);
			}

			delete movitems;
		}

		delete movarr;
	}

	if (_items[idx].count) {
		*rescount = _items[idx].count;

		return _items[idx].mi_movitems;
	}

	return 0;
}

bool MovGraph::setPosImmediate(StaticANIObject *obj, int x, int y) {
	obj->setOXY(x, y);
	return resetPosition(obj, 1);
}

MessageQueue *MovGraph::startMove(StaticANIObject *ani, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	debugC(4, kDebugPathfinding, "MovGraph::startMove(*%d, %d, %d, %d, %d)", (ani ? ani->_id : -1), xpos, ypos, fuzzyMatch, staticsId);

	if (!ani) {
		if (!_items.size())
			return 0;

		ani = _items[0].ani;
	}

	if (ABS(ani->_ox - xpos) < 50 && ABS(ani->_oy - ypos) < 50)
		return 0;

	if (!ani->isIdle())
		return 0;

	if (ani->_flags & 0x100)
		return 0;

	int count;
	Common::Array<MovItem *> *movitems = getPaths(ani, xpos, ypos, fuzzyMatch, &count);

	if (!movitems)
		return 0;

	if (ani->_movement) {
		Common::Point point;

		ani->calcStepLen(&point);

		MessageQueue *mq = sub1(ani, ani->_ox - point.x, ani->_oy - point.y, ani->_movement->_staticsObj1->_staticsId, xpos, ypos, 0, fuzzyMatch);

		if (!mq || !mq->getExCommandByIndex(0))
			return 0;

		ExCommand *ex = mq->getExCommandByIndex(0);

		if ((ex->_messageKind != 1 && ex->_messageKind != 20) || ex->_messageNum != ani->_movement->_id ||
			(ex->_z >= 1 && ex->_z <= ani->_movement->_currDynamicPhaseIndex)) {
			mq = new MessageQueue(g_nmi->_globalMessageQueueList->compact());

			ex = new ExCommand(ani->_id, 21, 0, 0, 0, 0, 1, 0, 0, 0);
			ex->_param = ani->_odelay;
			ex->_field_3C = 1;
			ex->_field_24 = 0;
			mq->addExCommandToEnd(ex);

			ex = new ExCommand(ani->_id, 51, 0, xpos, ypos, 0, 1, 0, 0, 0);
			ex->_param = ani->_odelay;
			ex->_field_3C = 1;
			ex->_field_24 = 0;
			ex->_invId = fuzzyMatch;
			mq->addExCommandToEnd(ex);

			if (mq->chain(0))
				return mq;

			delete mq;

			return 0;
		}

		int count2;

		ani->setSomeDynamicPhaseIndex(ex->_z);
		getPaths(ani, xpos, ypos, fuzzyMatch, &count2);

		int idx = getObjectIndex(ani);
		count = _items[idx].count;
		movitems = _items[idx].mi_movitems;
	}

	return method50(ani, _callback1(ani, movitems, count), staticsId);
}

void MovGraph::setSelFunc(MovArr *(*callback1)(StaticANIObject *ani, Common::Array<MovItem *> *items, signed int counter)) {
	_callback1 = callback1;
}

bool MovGraph::resetPosition(StaticANIObject *ani, int flag) {
	debugC(4, kDebugPathfinding, "MovGraph::resetPosition(*%d, %d)", (ani ? ani->_id : -1), flag);

	int idx = getObjectIndex(ani);

	if (idx == -1)
		return false;

	Common::Point point;
	MovArr movarr;

	point.x = ani->_ox;
	point.y = ani->_oy;

	getNearestPoint(idx, &point, &movarr);
	ani->setOXY(point.x, point.y);

	if (flag) {
		Statics *st;

		if (ani->_statics) {
			int t = _aniHandler.getNumMovements(ani->_id, ani->_statics->_staticsId, movarr._link->_dwordArray2[_field_44]);
			if (t > _aniHandler.getNumMovements(ani->_id, ani->_statics->_staticsId, movarr._link->_dwordArray2[_field_44 + 1]))
				st = ani->getStaticsById(movarr._link->_dwordArray2[_field_44 + 1]);
			else
				st = ani->getStaticsById(movarr._link->_dwordArray2[_field_44]);
		} else {
			ani->stopAnim_maybe();
			st = ani->getStaticsById(movarr._link->_dwordArray2[_field_44]);
		}

		ani->_statics = st;
	}

	return true;
}

bool MovGraph::canDropInventory(StaticANIObject *ani, int x, int y) {
	int idx = getObjectIndex(ani);
	MovArr m;

	if (idx != -1) {
		if (x != -1 || y != -1) {
			int counter;

			Common::Array<MovItem *> *movitem = getPaths(ani, x, y, 0, &counter);

			if (movitem) {
				MovArr *movarr = _callback1(ani, movitem, counter);
				int cnt = movarr->_movStepCount;

				if (cnt > 0) {
					if (movarr->_movSteps[cnt - 1]->link->_flags & 0x4000000)
						return true;
				}
			}
		} else if (getHitPoint(idx, ani->_ox, ani->_oy, &m, 0) && m._link && (m._link->_flags & 0x4000000)) {
			return true;
		}
	}

	return false;
}

MessageQueue *MovGraph::makeQueue(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	debugC(1, kDebugPathfinding, "MovGraph::makeQueue(*%d, %d, %d, %d, %d)", (subj ? subj->_id : -1), xpos, ypos, fuzzyMatch, staticsId);

	PicAniInfo picAniInfo;
	int ss;

	Common::Array<MovItem *> *movitem = getPaths(subj, xpos, ypos, fuzzyMatch, &ss);

	subj->getPicAniInfo(picAniInfo);

	if (movitem) {
		MovArr *goal = _callback1(subj, movitem, ss);
		int idx = getObjectIndex(subj);

		for (int i = 0; i < _items[idx].count; i++) {
			if ((*_items[idx].mi_movitems)[i]->movarr == goal) {
				if (subj->_movement) {
					Common::Point point;

					subj->calcStepLen(&point);

					MessageQueue *mq = sub1(subj, subj->_ox - point.x, subj->_oy - point.y, subj->_movement->_staticsObj1->_staticsId, xpos, ypos, 0, fuzzyMatch);

					if (!mq || !mq->getExCommandByIndex(0))
						return 0;

					ExCommand *ex = mq->getExCommandByIndex(0);

					if ((ex->_messageKind != 1 && ex->_messageKind != 20) ||
						ex->_messageNum != subj->_movement->_id ||
						(ex->_z >= 1 && ex->_z <= subj->_movement->_currDynamicPhaseIndex))
						subj->playIdle();
				}
			}
		}
	}

	movitem = getPaths(subj, xpos, ypos, fuzzyMatch, &ss);
	if (movitem) {
		MovArr *goal = _callback1(subj, movitem, ss);
		int idx = getObjectIndex(subj);

		if (_items[idx].count > 0) {
			int arridx = 0;

			while ((*_items[idx].mi_movitems)[arridx]->movarr != goal) {
				arridx++;

				if (arridx >= _items[idx].count) {
					subj->setPicAniInfo(picAniInfo);
					return 0;
				}
			}

			_items[idx].movarr._movSteps.clear();
			_items[idx].movarr = *(*_items[idx].mi_movitems)[arridx]->movarr;
			_items[idx].movarr._movSteps = (*_items[idx].mi_movitems)[arridx]->movarr->_movSteps;
			_items[idx].movarr._afield_8 = -1;
			_items[idx].movarr._link = 0;

			MessageQueue *mq = makeWholeQueue(_items[idx].ani, &_items[idx].movarr, staticsId);
			if (mq) {
				ExCommand *ex = new ExCommand();
				ex->_messageKind = 17;
				ex->_messageNum = 54;
				ex->_parentId = subj->_id;
				ex->_field_3C = 1;
				mq->addExCommandToEnd(ex);
			}
			subj->setPicAniInfo(picAniInfo);

			return mq;
		}
	}

	subj->setPicAniInfo(picAniInfo);

	return 0;
}

MessageQueue *MovGraph::sub1(StaticANIObject *ani, int x, int y, int stid, int x1, int y1, int stid2, int flag1) {
	debugC(4, kDebugPathfinding, "MovGraph::sub1(*%d, %d, %d, %d, %d, %d, %d, %d)", (ani ? ani->_id : -1), x, y, stid, x1, y1, stid2, flag1);

	PicAniInfo picinfo;

	ani->getPicAniInfo(picinfo);

	ani->_statics = ani->getStaticsById(stid);
	ani->_movement = 0;
	ani->setOXY(x, y);

	int rescount;

	Common::Array<MovItem *> *movitems = getPaths(ani, x1, y1, flag1, &rescount);

	if (!movitems) {
		ani->setPicAniInfo(picinfo);

		return 0;
	}

	MessageQueue *res = 0;

	MovArr *goal = _callback1(ani, movitems, rescount);
	int idx = getObjectIndex(ani);

	MovGraphItem &movgitem = _items[idx];
	int cnt = movgitem.count;

	for (int nidx = 0; nidx < cnt; nidx++) {
		if ((*movgitem.mi_movitems)[nidx]->movarr == goal) {
			movgitem.movarr._movSteps.clear();
			_items[idx].movarr = *(*movgitem.mi_movitems)[nidx]->movarr;
			_items[idx].movarr._movSteps = (*movgitem.mi_movitems)[nidx]->movarr->_movSteps;
			_items[idx].movarr._afield_8 = -1;
			_items[idx].movarr._link = 0;

			res = makeWholeQueue(_items[idx].ani, &_items[idx].movarr, stid2);

			break;
		}
	}

	ani->setPicAniInfo(picinfo);

	return res;
}

MessageQueue *MovGraph::makeWholeQueue(StaticANIObject *ani, MovArr *movarr, int staticsId) {
	debugC(4, kDebugPathfinding, "MovGraph::makeWholeQueue(*%d, *, %d)", (ani ? ani->_id : -1), staticsId);

	if (!movarr->_movStepCount)
		return 0;

	MessageQueue *mq = 0;
	int ox = ani->_ox;
	int oy = ani->_oy;
	int id1 = 0;
	int id2;

	for (int i = 0; i < movarr->_movStepCount; i++) {
		while (i < movarr->_movStepCount - 1) {
			if (movarr->_movSteps[i    ]->link->_dwordArray1[movarr->_movSteps[i - 1]->sfield_0 + _field_44] !=
				movarr->_movSteps[i + 1]->link->_dwordArray1[movarr->_movSteps[i    ]->sfield_0 + _field_44])
				break;
			i++;
		}

		MovStep *st = movarr->_movSteps[i];

		ani->getMovementById(st->link->_dwordArray1[_field_44 + st->sfield_0]);

		if (i == movarr->_movStepCount - 1 && staticsId) {
			id2 = staticsId;
		} else {
			if (i < movarr->_movStepCount - 1)
				id2 = ani->getMovementById(movarr->_movSteps[i + 1]->link->_dwordArray1[_field_44 + st->sfield_0])->_staticsObj1->_staticsId;
			else
				id2 = st->link->_dwordArray2[_field_44 + st->sfield_0];
		}

		int nx, ny, nd;

		if (i == movarr->_movStepCount - 1) {
			nx = movarr->_point.x;
			ny = movarr->_point.y;
			nd = st->link->_graphSrc->_z;
		} else {
			if (st->sfield_0) {
				nx = st->link->_graphSrc->_x;
				ny = st->link->_graphSrc->_y;
				nd = st->link->_graphSrc->_z;
			} else {
				nx = st->link->_graphDst->_x;
				ny = st->link->_graphDst->_y;
				nd = st->link->_graphDst->_z;
			}
		}

		MakeQueueStruct mkQueue;

		mkQueue.reset();
		mkQueue.ani = ani;
		mkQueue.staticsId2 = id2;
		mkQueue.staticsId1 = id1;
		mkQueue.x1 = nx;
		mkQueue.x2 = ox;
		mkQueue.y2 = oy;
		mkQueue.y1 = ny;
		mkQueue.field_1C = nd;
		mkQueue.movementId = st->link->_dwordArray1[_field_44 + st->sfield_0];

		mkQueue.flags = 0xe;
		if (mq)
			mkQueue.flags |= 0x31;

		MessageQueue *newmq = _aniHandler.makeRunQueue(&mkQueue);

		if (mq) {
			if (newmq) {
				mq->mergeQueue(newmq);

				delete newmq;
			}
		} else {
			mq = newmq;
		}

		ox = nx;
		oy = ny;
		id1 = id2;
	}

	return mq;
}

MessageQueue *MovGraph::method50(StaticANIObject *ani, MovArr *movarr, int staticsId) {
	debugC(4, kDebugPathfinding, "MovGraph::method50(*%d, *, %d)", (ani ? ani->_id : -1), staticsId);

	if (_items.size() == 0)
		return 0;

	uint idx;
	int movidx = 0;
	bool done = false;

	for (idx = 0; idx <= _items.size() && !done; idx++) {
		if (idx == _items.size())
			return 0;

		if (_items[idx].ani == ani) {
			if (!_items[idx].mi_movitems)
				return 0;

			if (_items[idx].count < 1)
				return 0;

			for (movidx = 0; movidx < _items[idx].count; movidx++) {
				if ((*_items[idx].mi_movitems)[movidx]->movarr == movarr) {
					done = true;

					break;
				}
			}
		}
	}

	_items[idx].movarr._movSteps.clear();
	_items[idx].movarr = *(*_items[idx].mi_movitems)[movidx]->movarr;
	_items[idx].movarr._movSteps = (*_items[idx].mi_movitems)[movidx]->movarr->_movSteps;
	_items[idx].movarr._afield_8 = -1;
	_items[idx].movarr._link = 0;

	MessageQueue *mq = makeWholeQueue(_items[idx].ani, &_items[idx].movarr, 0);

	if (!mq)
		return 0;

	ExCommand *ex = new ExCommand();

	ex->_messageKind = 17;
	ex->_messageNum = 54;
	ex->_parentId = ani->_id;
	ex->_field_3C = 1;
	mq->addExCommandToEnd(ex);

	if (!mq->chain(ani)) {
		delete mq;

		return 0;
	}

	return mq;
}

double MovGraph::putToLink(Common::Point *point, MovGraphLink *link, int fuzzyMatch) {
	int n1x = link->_graphSrc->_x;
	int n1y = link->_graphSrc->_y;
	int n2x = link->_graphDst->_x;
	int n2y = link->_graphDst->_y;
	double dist1x = (double)(point->x - n1x);
	double dist1y = (double)(n1y - point->y);
	double dist2x = (double)(n2x - n1x);
	double dist2y = (double)(n1y - n2y);
	double dist1 = sqrt(dist1x * dist1x + dist1y * dist1y);
	double dist2 = (dist2y * dist1y + dist2x * dist1x) / link->_length / dist1;
	double distm = dist2 * dist1;
	double res = sqrt(1.0 - dist2 * dist2) * dist1;

	if (dist2 <= 0.0 || distm >= link->_length) {
		if (fuzzyMatch) {
			if (dist2 > 0.0) {
				if (distm >= link->_length) {
					point->x = n2x;
					point->y = n2y;
				}
			} else {
				point->x = n1x;
				point->y = n1y;
			}
		} else {
			return -1.0;
		}
	} else {
		point->x = n1x + (int)((double)(n2x - n1x) * distm / link->_length);
		point->y = n1y + (int)((double)(n2y - n1y) * distm / link->_length);
	}

	return res;
}

void MovGraph::recalcLinkParams() {
	debugC(4, kDebugPathfinding, "MovGraph::recalcLinkParams()");

	for (LinkList::iterator i = _links.begin(); i != _links.end(); ++i) {
		assert((*i)->_objtype == kObjTypeMovGraphLink);

		MovGraphLink *lnk = static_cast<MovGraphLink *>(*i);

		lnk->_flags &= 0x7FFFFFFF;

		lnk->recalcLength();
	}
}

bool MovGraph::getNearestPoint(int unusedArg, Common::Point *p, MovArr *movarr) {
	debugC(4, kDebugPathfinding, "MovGraph::getNearestPoint(...)");

	MovGraphLink *link = 0;
	double mindist = 1.0e20;
	int resx = 0, resy = 0;

	for (LinkList::iterator i = _links.begin(); i != _links.end(); ++i) {
		MovGraphLink *lnk = static_cast<MovGraphLink *>(*i);

		if ((lnk->_flags & 0x10000000) && !(lnk->_flags & 0x20000000) ) {
			double dx1 = lnk->_graphSrc->_x - p->x;
			double dy1 = lnk->_graphSrc->_y - p->y;
			double dx2 = lnk->_graphDst->_x - p->x;
			double dy2 = lnk->_graphDst->_y - p->y;
			double dx3 = lnk->_graphDst->_x - lnk->_graphSrc->_x;
			double dy3 = lnk->_graphDst->_y - lnk->_graphSrc->_y;
			double sq1 = sqrt(dy1 * dy1 + dx1 * dx1);
			double sdist = (dy3 * dy1 + dx3 * dx1) / lnk->_length / sq1;
			double ldist = sdist * sq1;
			double dist = sqrt(1.0 - sdist * sdist) * sq1;

			if (ldist < 0.0) {
				ldist = 0.0;
				dist = sqrt(dx1 * dx1 + dy1 * dy1);
			}

			if (ldist > lnk->_length) {
				ldist = lnk->_length;
				dist = sqrt(dx2 * dx2 + dy2 * dy2);
			}

			if (ldist >= 0.0 && ldist <= lnk->_length && dist < mindist) {
				resx = lnk->_graphSrc->_x + (int)(dx3 * ldist / lnk->_length);
				resy = lnk->_graphSrc->_y + (int)(dy3 * ldist / lnk->_length);

				mindist = dist;
				link = lnk;
			}
		}
	}

	if (mindist < 1.0e20) {
		if (movarr)
			movarr->_link = link;

		if (p) {
			p->x = resx;
			p->y = resy;
		}

		return true;
	}

	return false;
}

int MovGraph::getObjectIndex(StaticANIObject *ani) {
	for (uint i = 0; i < _items.size(); i++)
		if (_items[i].ani == ani)
			return i;

	return -1;
}

Common::Array<MovArr *> *MovGraph::getHitPoints(int x, int y, int *arrSize, int flag1, int flag2) {
	debugC(4, kDebugPathfinding, "MovGraph::getHitPoints(...)");

	if (!_links.size()) {
		*arrSize = 0;

		return 0;
	}

	Common::Array<MovArr *> *arr = new Common::Array<MovArr *>;
	MovArr *movarr;

	for (LinkList::iterator i = _links.begin(); i != _links.end(); ++i) {
		MovGraphLink *lnk = static_cast<MovGraphLink *>(*i);

		if (flag1) {
			Common::Point point(x, y);
			double dist = putToLink(&point, lnk, 0);

			if (dist >= 0.0 && dist < 2.0) {
				movarr = new MovArr;

				movarr->_link = lnk;
				movarr->_dist = ((double)(lnk->_graphSrc->_y - lnk->_graphDst->_y) * (double)(lnk->_graphSrc->_y - point.y) +
								 (double)(lnk->_graphDst->_x - lnk->_graphSrc->_x) * (double)(point.x - lnk->_graphSrc->_x)) /
					lnk->_length / lnk->_length;
				movarr->_point = point;

				arr->push_back(movarr);
			}
		} else {
			if (lnk->_movGraphReact) {
				if (lnk->_movGraphReact->pointInRegion(x, y)) {
					if (!(lnk->_flags & 0x10000000) || lnk->_flags & 0x20000000) {
						if (!flag2) {
							movarr = new MovArr;
							movarr->_link = lnk;
							movarr->_dist = 0.0;
							movarr->_point.x = lnk->_graphSrc->_x;
							movarr->_point.y = lnk->_graphSrc->_y;
							arr->push_back(movarr);

							movarr = new MovArr;
							movarr->_link = lnk;
							movarr->_dist = 1.0;
							movarr->_point.x = lnk->_graphSrc->_x;
							movarr->_point.y = lnk->_graphSrc->_y;
							arr->push_back(movarr);
						}
					} else {
						movarr = new MovArr;
						movarr->_link = lnk;
						movarr->_dist = ((double)(lnk->_graphSrc->_y - lnk->_graphDst->_y) * (double)(lnk->_graphSrc->_y - y) +
										 (double)(lnk->_graphDst->_x - lnk->_graphSrc->_x) * (double)(x - lnk->_graphSrc->_x)) /
							lnk->_length / lnk->_length;
						movarr->_point.x = x;
						movarr->_point.y = y;

						putToLink(&movarr->_point, lnk, 0);

						arr->push_back(movarr);
					}
				}
			}
		}
	}

	*arrSize = arr->size();

	return arr;
}

void MovGraph::findAllPaths(MovGraphLink *lnk, MovGraphLink *lnk2, MovGraphLinkList &tempObList1, MovGraphLinkList &allPaths) {
	debugC(4, kDebugPathfinding, "MovGraph::findAllPaths(...)");

	if (lnk == lnk2) {
		for (uint i = 0; i < tempObList1.size(); i++)
			allPaths.push_back(tempObList1[i]);

		allPaths.push_back(lnk);
	} else {
		lnk->_flags |= 0x80000000;

		tempObList1.push_back(lnk);

		for (LinkList::iterator i = _links.begin(); i != _links.end(); ++i) {
			MovGraphLink *l = static_cast<MovGraphLink *>(*i);

			if (l->_graphSrc != lnk->_graphSrc) {
				if (l->_graphDst != lnk->_graphSrc) {
					if (l->_graphSrc != lnk->_graphDst && l->_graphDst != lnk->_graphDst)
						continue;
				}
			}

			if (!(l->_flags & 0xA0000000))
				findAllPaths(l, lnk2, tempObList1, allPaths);
		}

		lnk->_flags &= 0x7FFFFFFF;
	}
}

// Returns a list of possible paths two points in graph space
Common::Array<MovItem *> *MovGraph::getPaths(MovArr *currPos, MovArr *destPos, int *pathCount) {
	debugC(4, kDebugPathfinding, "MovGraph::getPaths(...)");

	MovGraphLinkList tempObList1;
	MovGraphLinkList allPaths;

	// Get all paths between two edges of the graph
	findAllPaths(currPos->_link, destPos->_link, tempObList1, allPaths);

	*pathCount = 0;

	if (!allPaths.size())
		return 0;

	*pathCount = allPaths.size();

	Common::Array<MovItem *> *res = new Common::Array<MovItem *>;

	for (int i = 0; i < *pathCount; i++) {
		MovItem *r = new MovItem;

		genMovItem(r, allPaths[i], currPos, destPos);

		res->push_back(r);

		delete allPaths[i];
	}

	// Start the resulting path from current position
	destPos->_link = currPos->_link;

	return res;
}

void MovGraph::genMovItem(MovItem *movitem, MovGraphLink *grlink, MovArr *movarr1, MovArr *movarr2) {
	warning("STUB: MovGraph::genMovItem()");
}

bool MovGraph::getHitPoint(int idx, int x, int y, MovArr *arr, int a6) {
	int staticsId;

	if (_items[idx].ani->_statics) {
		staticsId = _items[idx].ani->_statics->_staticsId;
	} else {
		if (!_items[idx].ani->_movement->_staticsObj2)
			return 0;

		staticsId = _items[idx].ani->_movement->_staticsObj2->_staticsId;
	}

	int arrSize;

	Common::Array<MovArr *> *movarr = getHitPoints(x, y, &arrSize, 0, 1);

	if (!movarr)
		return getNearestPoint(idx, 0, arr);

	bool res = false;

	int idxmin = -1;
	int offmin = 100;

	for (int i = 0; i < arrSize; i++) {
		int off = _aniHandler.getNumMovements(_items[idx].ani->_id, staticsId, (*movarr)[i]->_link->_dwordArray2[_field_44]);

		if (off < offmin) {
			offmin = off;
			idxmin = i;
		}

		off = _aniHandler.getNumMovements(_items[idx].ani->_id, staticsId, (*movarr)[i]->_link->_dwordArray2[_field_44 + 1]);
		if (off < offmin) {
			offmin = off;
			idxmin = i;
		}
	}

	if (idxmin != -1) {
		*arr = *(*movarr)[idxmin];

		res = true;
	}

	delete movarr;

	return res;
}

void MovGraph::setEnds(MovStep *step1, MovStep *step2) {
	debugC(4, kDebugPathfinding, "MovGraph::setEnds(...)");

	if (step1->link->_graphSrc == step2->link->_graphDst) {
		step1->sfield_0 = 1;
		step2->sfield_0 = 1;

		return;
	}

	if (step1->link->_graphSrc == step2->link->_graphSrc) {
		step1->sfield_0 = 1;
		step2->sfield_0 = 0;
	} else {
		step1->sfield_0 = 0;

		if (step1->link->_graphDst != step2->link->_graphSrc) {
			step2->sfield_0 = 1;
		} else {
			step2->sfield_0 = 0;
		}
	}
}

int MctlGraph::getObjIndex(int objectId) {
	for (uint i = 0; i < _items2.size(); i++)
		if (_items2[i]._objectId == objectId)
			return i;

	return -1;
}

int MctlGraph::getDirByStatics(int idx, int staticsId) {
	for (int i = 0; i < 4; i++)
		if (_items2[idx]._subItems[i]._staticsId1 == staticsId || _items2[idx]._subItems[i]._staticsId2 == staticsId)
			return i;

	return -1;
}

int MctlGraph::getDirByMovement(int idx, int movId) {
	for (int i = 0; i < 4; i++)
		if (_items2[idx]._subItems[i]._walk[0]._movementId == movId
		 || _items2[idx]._subItems[i]._walk[1]._movementId == movId
		 || _items2[idx]._subItems[i]._walk[2]._movementId == movId)
			return i;

	return -1;
}

int MctlGraph::getDirByPoint(int index, StaticANIObject *ani) {
	if (getHitNode(ani->_ox, ani->_oy, 0) || getHitLink(ani->_ox, ani->_oy, -1, 0) || getNearestLink(ani->_ox, ani->_oy)) {
		int minidx = -1;
		int min = 0;

		for (int i = 0; i < 4; i++) {
			debugC(1, kDebugPathfinding, "WWW 5");
			int tmp = _aniHandler.getNumMovements(ani->_id, ani->_statics->_staticsId, _items2[index]._subItems[i]._staticsId1);

			if (tmp >= 0 && (minidx == -1 || tmp < min)) {
				minidx = i;
				min = tmp;
			}
		}

		return minidx;
	}

	return -1;
}

bool MctlGraph::fillData(StaticANIObject *obj, MctlAni &item) {
	debugC(4, kDebugPathfinding, "MovGraph::fillData(%d, ...)", obj->_id);

	item._obj = obj;
	item._objectId = obj->_id;

	GameVar *var = g_nmi->getGameLoaderGameVar()->getSubVarByName(obj->_objectName);
	if (!var)
		return false;

	var = var->getSubVarByName("Test_walk");

	if (!var)
		return false;

	GameVar *varD = 0;
	Common::Point point;

	for (int dir = 0; dir < 4; dir++) {
		switch (dir) {
		case 0:
			varD = var->getSubVarByName("Right");
			break;
		case 1:
			varD = var->getSubVarByName("Left");
			break;
		case 2:
			varD = var->getSubVarByName("Up");
			break;
		case 3:
			varD = var->getSubVarByName("Down");
			break;
		default:
			break;
		}

		if (!varD)
			return false;

		for (int act = 0; act < 3; act++) {
			int idx = 0;

			switch(act) {
			case 0:
				idx = varD->getSubVarAsInt("Start");
				break;
			case 1:
				idx = varD->getSubVarAsInt("Go");
				break;
			case 2:
				idx = varD->getSubVarAsInt("Stop");
				break;
			default:
				break;
			}

			item._subItems[dir]._walk[act]._movementId = idx;

			Movement *mov = obj->getMovementById(idx);

			item._subItems[dir]._walk[act]._mov = mov;
			if (mov) {
				point = mov->calcSomeXY(0, -1);
				item._subItems[dir]._walk[act]._mx = point.x;
				item._subItems[dir]._walk[act]._my = point.y;
			}
		}

		for (int act = 0; act < 4; act++) {
			int idx = 0;

			switch(act) {
			case 0:
				idx = varD->getSubVarAsInt("TurnR");
				break;
			case 1:
				idx = varD->getSubVarAsInt("TurnL");
				break;
			case 2:
				idx = varD->getSubVarAsInt("TurnU");
				break;
			case 3:
				idx = varD->getSubVarAsInt("TurnD");
				break;
			default:
				break;
			}

			item._subItems[dir]._turn[act]._movementId = idx;

			Movement *mov = obj->getMovementById(idx);

			item._subItems[dir]._turn[act]._mov = mov;
			if (mov) {
				point = mov->calcSomeXY(0, -1);
				item._subItems[dir]._turn[act]._mx = point.x;
				item._subItems[dir]._turn[act]._my = point.y;
			}
		}

		for (int act = 0; act < 4; act++) {
			int idx = 0;

			switch(act) {
			case 0:
				idx = varD->getSubVarAsInt("TurnSR");
				break;
			case 1:
				idx = varD->getSubVarAsInt("TurnSL");
				break;
			case 2:
				idx = varD->getSubVarAsInt("TurnSU");
				break;
			case 3:
				idx = varD->getSubVarAsInt("TurnSD");
				break;
			default:
				break;
			}

			item._subItems[dir]._turnS[act]._movementId = idx;

			Movement *mov = obj->getMovementById(idx);

			item._subItems[dir]._turnS[act]._mov = mov;
			if (mov) {
				point = mov->calcSomeXY(0, -1);
				item._subItems[dir]._turnS[act]._mx = point.x;
				item._subItems[dir]._turnS[act]._my = point.y;
			}
		}

		item._subItems[dir]._staticsId1 = item._subItems[dir]._walk[0]._mov->_staticsObj1->_staticsId;
		item._subItems[dir]._staticsId2 = item._subItems[dir]._walk[0]._mov->_staticsObj2->_staticsId;

	}
	return true;
}

void MctlGraph::attachObject(StaticANIObject *obj) {
	debugC(4, kDebugPathfinding, "MctlGraph::attachObject(*%d)", obj->_id);

	MovGraph::attachObject(obj);

	int id = getObjIndex(obj->_id);

	if (id >= 0) {
		_items2[id]._obj = obj;
	} else {
		// this is a little dumb due to no move semantics
		_items2.push_back(MctlAni());
		if (!fillData(obj, _items2.back())) {
			_items2.pop_back();
		}
	}
}

void MctlGraph::generateList(MctlMQ &movinfo, MovGraphLinkList *linkList, LinkInfo *lnkSrc, LinkInfo *lnkDst) {
	debugC(4, kDebugPathfinding, "MctlGraph::generateList(...)");

	MctlMQSub *elem;
	Common::Point point;
	Common::Rect rect;

	int subIndex = movinfo.subIndex;

	movinfo.items.clear();

	movinfo.items.push_back(MctlMQSub());
	elem = &movinfo.items.back();
	elem->subIndex = subIndex;
	elem->x = movinfo.pt1.x;
	elem->y = movinfo.pt1.y;
	elem->distance = -1;

	int prevSubIndex = movinfo.subIndex;

	for (uint i = 0; i < linkList->size(); i++) {
		int idx1;

		if (linkList->size() <= 1) {
			if (linkList->size() == 1)
				idx1 = getDirBySize((*linkList)[0], movinfo.pt2.x - movinfo.pt1.x, movinfo.pt2.y - movinfo.pt1.y);
			else
				idx1 = getDirBySize(0, movinfo.pt2.x - movinfo.pt1.x, movinfo.pt2.y - movinfo.pt1.y);

			point.y = -1;
			rect.bottom = -1;
			rect.right = -1;
			rect.top = -1;
			rect.left = -1;
		} else {
			idx1 = getLinkDir(linkList, i, &rect, &point);
		}

		if (idx1 != prevSubIndex) {
			prevSubIndex = idx1;
			subIndex = idx1;

			movinfo.items.push_back(MctlMQSub());
			elem = &movinfo.items.back();
			elem->subIndex = subIndex;
			elem->x = rect.left;
			elem->y = rect.top;
			elem->distance = -1;
		}

		if (i != linkList->size() - 1) {
			for (;;) {
				i++;
				if (getLinkDir(linkList, i, &rect, 0) != prevSubIndex) {
					i--;
					getLinkDir(linkList, i, &rect, &point);

					break;
				}

				if (i == linkList->size() - 1)
					break;
			}
		}

		if (movinfo.items.back().subIndex != 10) {
			subIndex = prevSubIndex;

			movinfo.items.push_back(MctlMQSub());
			elem = &movinfo.items.back();
			elem->subIndex = 10;
			elem->x = -1;
			elem->y = -1;
			elem->distance = -1;

			movinfo.items.push_back(MctlMQSub());
			elem = &movinfo.items.back();
			elem->subIndex = prevSubIndex;
			if (i == linkList->size() - 1) {
				elem->x = movinfo.pt2.x;
				elem->y = movinfo.pt2.y;
				elem->distance = movinfo.distance2;
			} else {
				elem->x = rect.right;
				elem->y = rect.bottom;
				elem->distance = point.y;
			}
		}
	}

	if (subIndex != movinfo.item1Index) {
		movinfo.items.push_back(MctlMQSub());
		elem = &movinfo.items.back();
		elem->subIndex = movinfo.item1Index;
		elem->x = movinfo.pt2.x;
		elem->y = movinfo.pt2.y;
		elem->distance = movinfo.distance2;
	}
}

MessageQueue *MctlGraph::makeWholeQueue(MctlMQ &mctlMQ) {
	debugC(4, kDebugPathfinding, "MctlGraph::makeWholeQueue(...)");

	MctlMQ movinfo(mctlMQ);

	int curX = mctlMQ.pt1.x;
	int curY = mctlMQ.pt1.y;
	int curDistance = mctlMQ.distance1;

	MessageQueue *mq = new MessageQueue(g_nmi->_globalMessageQueueList->compact());

	int numItems = mctlMQ.items.size();

	for (int i = 0; i < numItems - 1; i++) {
		if (mctlMQ.items[i + 1].subIndex != 10) {
			MG2I *mg2i;

			if (i >= numItems - 2 || mctlMQ.items[i + 2].subIndex != 10) {
				movinfo.flags = 0;
				mg2i = &_items2[mctlMQ.index]._subItems[mctlMQ.items[i].subIndex]._turnS[mctlMQ.items[i + 1].subIndex];
			} else {
				movinfo.flags = 2;
				mg2i = &_items2[mctlMQ.index]._subItems[mctlMQ.items[i].subIndex]._turn[mctlMQ.items[i + 1].subIndex];
			}
			if (i < numItems - 2
				|| (mctlMQ.items[i].x == mctlMQ.items[i + 1].x
					&& mctlMQ.items[i].y == mctlMQ.items[i + 1].y)
				 || mctlMQ.items[i].x == -1
				 || mctlMQ.items[i].y == -1
				 || mctlMQ.items[i + 1].x == -1
				 || mctlMQ.items[i + 1].y == -1) {

				ExCommand *ex = new ExCommand(_items2[mctlMQ.index]._objectId, 1, mg2i->_movementId, 0, 0, 0, 1, 0, 0, 0);

				ex->_excFlags |= 2;
				ex->_param = _items2[mctlMQ.index]._obj->_odelay;
				ex->_field_24 = 1;
				ex->_z = -1;
				mq->addExCommandToEnd(ex);

				curX += mg2i->_mx;
				curY += mg2i->_my;
			} else {
				MakeQueueStruct mkQueue;

				mkQueue.reset();

				mkQueue.ani = _items2[mctlMQ.index]._obj;
				mkQueue.staticsId2 = mg2i->_mov->_staticsObj2->_staticsId;
				mkQueue.x1 = mctlMQ.items[i + 1].x;
				mkQueue.y1 = mctlMQ.items[i + 1].y;
				mkQueue.field_1C = mctlMQ.items[i + 1].distance;
				mkQueue.staticsId1 = mg2i->_mov->_staticsObj1->_staticsId;

				mkQueue.x2 = mctlMQ.items[i].x;
				mkQueue.y2 = mctlMQ.items[i].y;
				mkQueue.field_10 = 1;
				mkQueue.flags = 0x7f;
				mkQueue.movementId = mg2i->_movementId;

				Common::ScopedPtr<MessageQueue> mq2(_aniHandler.makeRunQueue(&mkQueue));
				mq->mergeQueue(mq2.get());

				curX = mctlMQ.items[i + 1].x;
				curY = mctlMQ.items[i + 1].y;
			}
		} else {
			movinfo.item1Index = mctlMQ.items[i].subIndex;
			movinfo.subIndex = movinfo.item1Index;
			movinfo.pt1.y = curY;
			movinfo.pt1.x = curX;

			movinfo.distance1 = curDistance;
			movinfo.pt2.x = mctlMQ.items[i + 2].x;
			movinfo.pt2.y = mctlMQ.items[i + 2].y;
			movinfo.distance2 = mctlMQ.items[i + 2].distance;

			if (i < numItems - 4
				&& mctlMQ.items[i + 2].subIndex != 10
				&& mctlMQ.items[i + 3].subIndex != 10
				&& mctlMQ.items[i + 2].subIndex != mctlMQ.items[i + 3].subIndex
				&& mctlMQ.items[i + 4].subIndex == 10) {

				MG2I *m = &_items2[mctlMQ.index]._subItems[mctlMQ.items[i + 2].subIndex]._turn[mctlMQ.items[i + 3].subIndex];

				if (movinfo.item1Index && movinfo.item1Index != 1) {
					movinfo.pt2.y -= m->_my;
					movinfo.flags = (movinfo.flags & 2) | 1;
				} else {
					movinfo.pt2.x -= m->_mx;
					movinfo.flags = (movinfo.flags & 2) | 1;
				}

			} else if (i < numItems - 3
				&& mctlMQ.items[i + 2].subIndex != 10
				&& mctlMQ.items[i + 3].subIndex != 10
				&& mctlMQ.items[i + 2].subIndex != mctlMQ.items[i + 3].subIndex) {

				MG2I *m = &_items2[mctlMQ.index]._subItems[mctlMQ.items[i + 2].subIndex]._turnS[mctlMQ.items[i + 3].subIndex];
				movinfo.pt2.x -= m->_mx;
				movinfo.pt2.y -= m->_my;
				movinfo.flags = (movinfo.flags & 2) | (mctlMQ.flags & 1);

			} else {
				movinfo.flags = (movinfo.flags & 2) | (mctlMQ.flags & 1);
			}

			i++; // intentional

			MessageQueue *mq2 = makeLineQueue(&movinfo);

			if (!mq2) {
				delete mq;
				return 0;
			}

			mq->mergeQueue(mq2);

			delete mq2;

			curX = movinfo.pt2.x;
			curY = movinfo.pt2.y;
			curDistance = movinfo.distance2;
		}
	}

	mctlMQ.pt2.x = movinfo.pt2.x;
	mctlMQ.pt2.y = movinfo.pt2.y;

	return mq;
}

int MctlGraph::detachObject(StaticANIObject *obj) {
	warning("STUB: MctlGraph::detachObject()");

	return 0;
}

void MctlGraph::detachAllObjects() {
	debugC(4, kDebugPathfinding, "MctlGraph::detachAllObjects()");
	_items2.clear();
}

MessageQueue *MctlGraph::startMove(StaticANIObject *ani, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	debugC(4, kDebugPathfinding, "MctlGraph::startMove(*%d, %d, %d, %d, %d)", ani->_id, xpos, ypos, fuzzyMatch, staticsId);

	if (!ani->isIdle())
		return 0;

	if (ani->_flags & 0x100)
		return 0;

	debugC(1, kDebugPathfinding, "WWW 3");
	MessageQueue *mq = makeQueue(ani, xpos, ypos, fuzzyMatch, staticsId);

	if (!mq)
		return 0;

	if (ani->_movement) {
		if (mq->getCount() <= 1 || mq->getExCommandByIndex(0)->_messageKind != 22) {
			PicAniInfo picAniInfo;

			ani->getPicAniInfo(picAniInfo);
			ani->updateStepPos();
			MessageQueue *mq1 = makeQueue(ani, xpos, ypos, fuzzyMatch, staticsId);

			ani->setPicAniInfo(picAniInfo);

			if (mq1) {
				delete mq;

				mq = mq1;
			}
		} else {
			ani->_movement = 0;
		}
	}

	if (!mq->chain(ani)) {
		delete mq;

		return 0;
	}

	return mq;
}

MessageQueue *MctlGraph::makeQueue(StaticANIObject *obj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	LinkInfo linkInfoDest;
	LinkInfo linkInfoSource;
	MctlMQ mctlMQ1;
	PicAniInfo picAniInfo;
	Common::Point point;

	debugC(1, kDebugPathfinding, "MctlGraph::makeQueue(%d, %d, %d, %d, %d)", obj->_id, xpos, ypos, fuzzyMatch, staticsId);

	int idx = getObjIndex(obj->_id);

	if (idx < 0)
		return 0;

	linkInfoSource.link = 0;
	linkInfoSource.node = 0;

	linkInfoDest.link = 0;
	linkInfoDest.node = 0;

	point.x = 0;

	obj->getPicAniInfo(picAniInfo);

	int idxsub;

	if (obj->_movement)
		idxsub = getDirByMovement(idx, obj->_movement->_id);
	else
		idxsub = getDirByStatics(idx, obj->_statics->_staticsId);

	bool subMgm = false;

	if (idxsub == -1) {
		debugC(1, kDebugPathfinding, "WWW 4");
		idxsub = getDirByPoint(idx, obj);
		subMgm = true;

		if (idxsub == -1)
			return 0;
	}

	if (obj->_movement) {
		int newx, newy;

		if (subMgm) {
			obj->_messageQueueId = 0;
			obj->changeStatics2(_items2[idx]._subItems[idxsub]._staticsId1);
			newx = obj->_ox;
			newy = obj->_oy;
		} else {
			point = obj->_movement->calcSomeXY(0, picAniInfo.dynamicPhaseIndex);
			newx = obj->_movement->_ox - point.x;
			newy = obj->_movement->_oy - point.y;
			if (idxsub != 1 && idxsub) {
				if (idxsub == 2 || idxsub == 3) {
					newy = obj->_movement->_oy;
				}
			} else {
				newx = obj->_movement->_ox;
			}
		}

		obj->_movement = 0;
		obj->setOXY(newx, newy);
	}

	if (obj->_ox == xpos && obj->_oy == ypos) {
		MessageQueue *mq = new MessageQueue(g_nmi->_globalMessageQueueList->compact());

		if (staticsId && obj->_statics->_staticsId != staticsId) {
			int idxwalk = getDirByStatics(idx, staticsId);
			if (idxwalk == -1) {
				obj->setPicAniInfo(picAniInfo);

				delete mq;

				return 0;
			}

			ExCommand *ex = new ExCommand(picAniInfo.objectId, 1, _items2[idx]._subItems[idxsub]._turnS[idxwalk]._movementId, 0, 0, 0, 1, 0, 0, 0);

			ex->_field_24 = 1;
			ex->_param = picAniInfo.field_8;
			ex->_excFlags |= 2;

			mq->addExCommandToEnd(ex);
		} else {
			ExCommand *ex = new ExCommand(picAniInfo.objectId, 22, obj->_statics->_staticsId, 0, 0, 0, 1, 0, 0, 0);

			ex->_param = picAniInfo.field_8;
			ex->_excFlags |= 3;
			mq->addExCommandToEnd(ex);

			ex = new ExCommand(picAniInfo.objectId, 5, -1, obj->_ox, obj->_oy, 0, 1, 0, 0, 0);

			ex->_z = -1;
			ex->_param = picAniInfo.field_8;
			ex->_excFlags |= 3;
			mq->addExCommandToEnd(ex);
		}

		obj->setPicAniInfo(picAniInfo);

		return mq;
	}

	linkInfoSource.node = getHitNode(obj->_ox, obj->_oy, 0);

	if (!linkInfoSource.node) {
		linkInfoSource.link = getHitLink(obj->_ox, obj->_oy, idxsub, 0);

		if (!linkInfoSource.link) {
			linkInfoSource.link = getNearestLink(obj->_ox, obj->_oy);

			if (!linkInfoSource.link) {
				obj->setPicAniInfo(picAniInfo);

				return 0;
			}
		}
	}

	linkInfoDest.node = getHitNode(xpos, ypos, fuzzyMatch);

	if (!linkInfoDest.node) {
		linkInfoDest.link = getHitLink(xpos, ypos, idxsub, fuzzyMatch);

		if (!linkInfoDest.link) {
			obj->setPicAniInfo(picAniInfo);

			return 0;
		}
	}

	MovGraphLinkList tempLinkList;
	double minPath = iterate(&linkInfoSource, &linkInfoDest, &tempLinkList);

	debugC(0, kDebugPathfinding, "MctlGraph::makeQueue(): path: %g  parts: %d", minPath, tempLinkList.size());

	if (minPath < 0.0 || ((linkInfoSource.node != linkInfoDest.node || !linkInfoSource.node) && !tempLinkList.size()))
		return 0;

	mctlMQ1.clear();

	mctlMQ1.subIndex = idxsub;
	mctlMQ1.pt1.x = obj->_ox;
	mctlMQ1.pt1.y = obj->_oy;

	int dx1 = obj->_ox;
	int dy1 = obj->_oy;
	int dx2, dy2;

	if (linkInfoSource.node)
		mctlMQ1.distance1 = linkInfoSource.node->_z;
	else
		mctlMQ1.distance1 = linkInfoSource.link->_graphSrc->_z;

	if (linkInfoDest.node) {
		dx2 = linkInfoDest.node->_x;
		dy2 = linkInfoDest.node->_y;

		mctlMQ1.pt2.x = linkInfoDest.node->_x;
		mctlMQ1.pt2.y = linkInfoDest.node->_y;

		mctlMQ1.distance2 = linkInfoDest.node->_z;
	} else {
		mctlMQ1.pt2.x = xpos;
		mctlMQ1.pt2.y = ypos;

		MovGraphNode *nod = linkInfoDest.link->_graphSrc;
		double dst1 = sqrt((double)((ypos - nod->_y) * (ypos - nod->_y) + (xpos - nod->_x) * (xpos - nod->_x)));
		int dst = linkInfoDest.link->_graphDst->_z - nod->_z;

		mctlMQ1.distance2 = nod->_z + (int)(dst1 * (double)dst / linkInfoDest.link->_length);

		putToLink(&mctlMQ1.pt2, linkInfoDest.link, 1);

		dx1 = mctlMQ1.pt1.x;
		dy1 = mctlMQ1.pt1.y;
		dx2 = mctlMQ1.pt2.x;
		dy2 = mctlMQ1.pt2.y;
	}

	if (staticsId) {
		mctlMQ1.item1Index = getDirByStatics(idx, staticsId);
	} else if (tempLinkList.size() <= 1) {
		if (tempLinkList.size() == 1)
			mctlMQ1.item1Index = getDirBySize(tempLinkList[0], dx2 - dx1, dy2 - dy1);
		else
			mctlMQ1.item1Index = getDirBySize(0, dx2 - dx1, dy2 - dy1);
	} else {
		mctlMQ1.item1Index = getLinkDir(&tempLinkList, tempLinkList.size() - 1, 0, 0);
	}

	mctlMQ1.flags = fuzzyMatch != 0;

	if (_items2[idx]._subItems[idxsub]._staticsId1 != obj->_statics->_staticsId)
		mctlMQ1.flags |= 2;

	generateList(mctlMQ1, &tempLinkList, &linkInfoSource, &linkInfoDest);

	MessageQueue *mq = makeWholeQueue(mctlMQ1);

	linkInfoDest.node = getHitNode(mctlMQ1.pt2.x, mctlMQ1.pt2.y, fuzzyMatch);

	if (!linkInfoDest.node)
		linkInfoDest.link = getHitLink(mctlMQ1.pt2.x, mctlMQ1.pt2.y, mctlMQ1.item1Index, fuzzyMatch);

	if (fuzzyMatch || linkInfoDest.link || linkInfoDest.node) {
		if (mq && mq->getCount() > 0 && picAniInfo.movementId) {
			ExCommand *ex = mq->getExCommandByIndex(0);

			if (ex && (ex->_messageKind == 1 || ex->_messageKind == 20)
					&& picAniInfo.movementId == ex->_messageNum
					&& picAniInfo.someDynamicPhaseIndex == ex->_z) {
				mq->deleteExCommandByIndex(0, 1);
			} else {
				ex = new ExCommand(picAniInfo.objectId, 5, ex->_messageNum, obj->_ox, obj->_oy, 0, 1, 0, 0, 0);
				ex->_z = -1;
				ex->_param = picAniInfo.field_8;
				ex->_excFlags |= 2;
				mq->addExCommand(ex);

				ex = new ExCommand(picAniInfo.objectId, 22, _items2[idx]._subItems[idxsub]._staticsId1, 0, 0, 0, 1, 0, 0, 0);

				ex->_param = picAniInfo.field_8;
				ex->_excFlags |= 3;
				mq->addExCommand(ex);
			}
		}
	} else {
		delete mq;
		mq = nullptr;
	}

	obj->setPicAniInfo(picAniInfo);

	return mq;
}

MovGraphNode *MctlGraph::getHitNode(int x, int y, int strictMatch) {
	for (NodeList::iterator i = _nodes.begin(); i != _nodes.end(); ++i) {
		assert((*i)->_objtype == kObjTypeMovGraphNode);

		MovGraphNode *node = *i;

		if (!strictMatch) {
			if (abs(node->_x - x) < 15 && abs(node->_y - y) < 15)
				return node;
		} else {
			if (node->_x == x && node->_y == y)
				return node;
		}
	}

	return 0;
}

int MctlGraph::getDirBySize(MovGraphLink *lnk, int x, int y) {
	bool cond;

	if (lnk)
		cond = abs(lnk->_graphDst->_x - lnk->_graphSrc->_x) > abs(lnk->_graphDst->_y - lnk->_graphSrc->_y);
	else
		cond = abs(x) > abs(y);

	if (cond)
		return x <= 0;
	else
		return ((y > 0) + 2);
}

int MctlGraph::getLinkDir(MovGraphLinkList *linkList, int idx, Common::Rect *rect, Common::Point *point) {
	debugC(4, kDebugPathfinding, "MctlGraph::getLinkDir(...)");

	MovGraphNode *node1 = (*linkList)[idx]->_graphSrc;
	MovGraphNode *node2 = (*linkList)[idx]->_graphDst;
	MovGraphNode *node3 = node1;

	if (idx != 0) {
		MovGraphLink *lnk = (*linkList)[idx - 1];

		if (lnk->_graphDst != node1) {
			if (lnk->_graphSrc != node1) {
				if (lnk->_graphDst == node2 || lnk->_graphSrc == node2) {
					node3 = node2;
					node2 = node1;
				}
				goto LABEL_7;
			}
		}
		node3 = node1;
	} else if (idx != (int)(linkList->size() - 1)) {
		MovGraphLink *lnk = (*linkList)[idx + 1];

		if (lnk->_graphDst == node1 || lnk->_graphSrc == node1) {
			node3 = node2;
			node2 = node1;
		} else if (lnk->_graphDst == node2 || lnk->_graphSrc == node2) {
			node3 = node1;
		}
	}

 LABEL_7:
	if (rect) {
		rect->left = node3->_x;
		rect->top = node3->_y;
		rect->right = node2->_x;
		rect->bottom = node2->_y;
	}
	if (point) {
		point->x = node3->_z;
		point->y = node2->_z;
	}

	if (abs(node3->_x - node2->_x) <= abs(node3->_y - node2->_y))
		return (node3->_y < node2->_y) + 2;
	else
		return node3->_x >= node2->_x;
}

MessageQueue *MctlGraph::makeLineQueue(MctlMQ *info) {
	debugC(4, kDebugPathfinding, "MctlGraph::makeLineQueue(...)");

	int mx1 = 0;
	int my1 = 0;

	if (!(info->flags & 2)) {
		mx1 = _items2[info->index]._subItems[info->subIndex]._walk[0]._mx;
		my1 = _items2[info->index]._subItems[info->subIndex]._walk[0]._my;
	}

	int mx2 = 0;
	int my2 = 0;

	if (!(info->flags & 4)) {
		mx2 = _items2[info->index]._subItems[info->subIndex]._walk[2]._mx;
		my2 = _items2[info->index]._subItems[info->subIndex]._walk[2]._my;
	}

	Common::Point point;

	int y = info->pt2.y - info->pt1.y - my2 - my1;
	int x = info->pt2.x - info->pt1.x - mx2 - mx1;
	int a2 = 0;
	int mgmLen;

	point = _aniHandler.getNumCycles(_items2[info->index]._subItems[info->subIndex]._walk[1]._mov, x, y, &mgmLen, &a2, info->flags & 1);

	int x1 = point.x;
	int y1 = point.y;

	if (!(info->flags & 1)) {
		if (info->subIndex == 1 || info->subIndex == 0) {
			a2 = -1;
			x1 = mgmLen * _items2[info->index]._subItems[info->subIndex]._walk[1]._mx;
			x = x1;
			info->pt2.x = x1 + info->pt1.x + mx1 + mx2;
		}
	}

	if (!(info->flags & 1)) {
		if (info->subIndex == 2 || info->subIndex == 3) {
			a2 = -1;
			y1 = mgmLen * _items2[info->index]._subItems[info->subIndex]._walk[1]._my;
			y = y1;
			info->pt2.y = y1 + info->pt1.y + my1 + my2;
		}
	}

	int cntX = 0;
	int cntY = 0;

	if (!(info->flags & 2)) {
		cntX = _items2[info->index]._subItems[info->subIndex]._walk[0]._mov->countPhasesWithFlag(-1, 1);
		cntY = _items2[info->index]._subItems[info->subIndex]._walk[0]._mov->countPhasesWithFlag(-1, 2);
	}

	if (mgmLen > 1) {
		cntX += (mgmLen - 1) * _items2[info->index]._subItems[info->subIndex]._walk[1]._mov->countPhasesWithFlag(-1, 1);
		cntY += (mgmLen - 1) * _items2[info->index]._subItems[info->subIndex]._walk[1]._mov->countPhasesWithFlag(-1, 2);
	}

	if (mgmLen > 0) {
		cntX += _items2[info->index]._subItems[info->subIndex]._walk[1]._mov->countPhasesWithFlag(a2, 1);
		cntY += _items2[info->index]._subItems[info->subIndex]._walk[1]._mov->countPhasesWithFlag(a2, 2);
	}

	if (!(info->flags & 4)) {
		cntX += _items2[info->index]._subItems[info->subIndex]._walk[2]._mov->countPhasesWithFlag(-1, 1);
		cntY += _items2[info->index]._subItems[info->subIndex]._walk[2]._mov->countPhasesWithFlag(-1, 2);
	}

	int dx1 = x - x1;
	int dy1 = y - y1;

	if (cntX)
		x1 = (int)((double)dx1 / (double)cntX);
	else
		x1 = 0;

	if (cntY)
		y1 = (int)((double)dy1 / (double)cntY);
	else
		y1 = 0;

	int v34 = dx1 - cntX * x1;
	int v35 = dy1 - cntY * y1;
	Common::Point x2;
	Common::Point y2(v34, v35);

	if (v34)
		x2.x = v34 / abs(v34);
	else
		x2.x = 0;

	if (v35)
		x2.y = v35 / abs(v35);
	else
		x2.y = 0;

	MessageQueue *mq = new MessageQueue(g_nmi->_globalMessageQueueList->compact());
	ExCommand *ex;

	if (info->flags & 2) {
		ex = new ExCommand(
							_items2[info->index]._objectId,
							5,
							_items2[info->index]._subItems[info->subIndex]._walk[1]._movementId,
							info->pt1.x,
							info->pt1.y,
							0,
							1,
							0,
							0,
							0);

		ex->_z = info->distance1;

		ex->_param = _items2[info->index]._obj->_odelay;
		ex->_field_24 = 1;
		ex->_excFlags |= 2;
	} else {
		ex = new ExCommand(
							 _items2[info->index]._objectId,
							 5,
							 _items2[info->index]._subItems[info->subIndex]._walk[0]._movementId,
							 info->pt1.x,
							 info->pt1.y,
							 0,
							 1,
							 0,
							 0,
							 0);

		ex->_z = info->distance1;

		ex->_param = _items2[info->index]._obj->_odelay;
		ex->_field_24 = 1;
		ex->_excFlags |= 2;
		mq->addExCommandToEnd(ex);

		ex = _aniHandler.createCommand(
								  _items2[info->index]._subItems[info->subIndex]._walk[0]._mov,
								  _items2[info->index]._objectId,
								  x1,
								  y1,
								  x2,
								  y2,
								  -1);
		ex->_parId = mq->_id;
		ex->_param = _items2[info->index]._obj->_odelay;
	}

	mq->addExCommandToEnd(ex);

	for (int i = 0; i < mgmLen; ++i) {
		int par;

		if (i == mgmLen - 1)
			par = a2;
		else
			par = -1;

		ex = _aniHandler.createCommand(
								  _items2[info->index]._subItems[info->subIndex]._walk[1]._mov,
								  _items2[info->index]._objectId,
								  x1,
								  y1,
								  x2,
								  y2,
								  par);
		ex->_parId = mq->_id;
		ex->_param = _items2[info->index]._obj->_odelay;
		mq->addExCommandToEnd(ex);
	}

	if (!(info->flags & 4)) {
		ex = _aniHandler.createCommand(
								  _items2[info->index]._subItems[info->subIndex]._walk[2]._mov,
								  _items2[info->index]._objectId,
								  x1,
								  y1,
								  x2,
								  y2,
								  -1);
		ex->_parId = mq->_id;
		ex->_param = _items2[info->index]._obj->_odelay;

		mq->addExCommandToEnd(ex);
	}

	ex = new ExCommand(_items2[info->index]._objectId, 5, -1, info->pt2.x, info->pt2.y, 0, 1, 0, 0, 0);
	ex->_z = info->distance2;

	ex->_param = _items2[info->index]._obj->_odelay;
	ex->_field_24 = 0;
	ex->_excFlags |= 2;

	mq->addExCommandToEnd(ex);

	return mq;
}

MovGraphLink *MctlGraph::getHitLink(int x, int y, int idx, int fuzzyMatch) {
	debugC(4, kDebugPathfinding, "MctlGraph::getHitLink(...)");

	Common::Point point;
	MovGraphLink *res = 0;

	for (LinkList::iterator i = _links.begin(); i != _links.end(); ++i) {
		assert((*i)->_objtype == kObjTypeMovGraphLink);

		MovGraphLink *lnk = static_cast<MovGraphLink *>(*i);

		if (fuzzyMatch) {
			point.x = x;
			point.y = y;
			double dst = putToLink(&point, lnk, 0);

			if (dst >= 0.0 && dst < 2.0)
				return lnk;
		} else if (!(lnk->_flags & 0x20000000)) {
			if (lnk->_movGraphReact->pointInRegion(x, y)) {
				if (abs(lnk->_graphSrc->_x - lnk->_graphDst->_x) <= abs(lnk->_graphSrc->_y - lnk->_graphDst->_y)) {
					if (idx == 2 || idx == 3)
						return lnk;
					res = lnk;
				} else {
					if (idx == 1 || !idx)
						return lnk;
					res = lnk;
				}
			}
		}
	}

	return res;
}

MovGraphLink *MctlGraph::getNearestLink(int x, int y) {
	debugC(4, kDebugPathfinding, "MctlGraph::getNearestLink(...)");

	double mindist = 1.0e20;
	MovGraphLink *res = 0;

	for (LinkList::iterator i = _links.begin(); i != _links.end(); ++i) {
		assert((*i)->_objtype == kObjTypeMovGraphLink);

		MovGraphLink *lnk = static_cast<MovGraphLink *>(*i);

		if (!(lnk->_flags & 0x20000000)) {
			double n1x = lnk->_graphSrc->_x;
			double n1y = lnk->_graphSrc->_y;
			double n2x = lnk->_graphDst->_x;
			double n2y = lnk->_graphDst->_y;
			double n1dx = x - n1x;
			double n1dy = n1y - y;
			double dst1 = sqrt(n1dy * n1dy + n1dx * n1dx);
			double coeff1 = ((n1y - n2y) * n1dy + (n2x - n1x) * n1dx) / lnk->_length / dst1;
			double dst3 = coeff1 * dst1;
			double dst2 = sqrt(1.0 - coeff1 * coeff1) * dst1;

			if (dst3 < 0.0) {
				dst3 = 0.0;
				dst2 = sqrt((n1x - x) * (n1x - x) + (n1y - y) * (n1y - y));
			}
			if (dst3 > lnk->_length) {
				dst3 = lnk->_length;
				dst2 = sqrt((n2x - x) * (n2x - x) + (n2y - y) * (n2y - y));
			}
			if (dst3 >= 0.0 && dst3 <= lnk->_length && dst2 < mindist) {
				mindist = dst2;
				res = lnk;
			}
		}
	}

	if (mindist < 1.0e20)
		return res;
	else
		return 0;
}

double MctlGraph::iterate(LinkInfo *linkInfoSource, LinkInfo *linkInfoDest, MovGraphLinkList *listObj) {
	debugC(4, kDebugPathfinding, "MctlGraph::iterate(...)");

	LinkInfo linkInfoWorkSource;

	if (linkInfoSource->link != linkInfoDest->link || linkInfoSource->node != linkInfoDest->node) {
		double minDistance = -1.0;

		if (linkInfoSource->node) {
			for (LinkList::iterator i = _links.begin(); i != _links.end(); ++i) {
				MovGraphLink *lnk = static_cast<MovGraphLink *>(*i);

				if ((lnk->_graphSrc == linkInfoSource->node || lnk->_graphDst == linkInfoSource->node) && !(lnk->_flags & 0xA0000000)) {
					linkInfoWorkSource.node = 0;
					linkInfoWorkSource.link = lnk;

					MovGraphLinkList tmpList;

					lnk->_flags |= 0x80000000;

					double newDistance = iterate(&linkInfoWorkSource, linkInfoDest, &tmpList);

					if (newDistance >= 0.0 && (minDistance < 0.0 || newDistance + lnk->_length < minDistance)) {
						listObj->clear();
						listObj->push_back(tmpList);

						minDistance = newDistance + lnk->_length;
					}

					lnk->_flags &= 0x7FFFFFFF;
				}
			}
		} else if (linkInfoSource->link) {
			linkInfoWorkSource.node = linkInfoSource->link->_graphSrc;
			linkInfoWorkSource.link = 0;

			MovGraphLinkList tmpList;

			double newDistance = iterate(&linkInfoWorkSource, linkInfoDest, &tmpList);

			if (newDistance >= 0.0) {
				listObj->clear();

				listObj->push_back(linkInfoSource->link);
				listObj->push_back(tmpList);

				minDistance = newDistance;
			}

			linkInfoWorkSource.link = 0;
			linkInfoWorkSource.node = linkInfoSource->link->_graphDst;

			tmpList.clear();

			newDistance = iterate(&linkInfoWorkSource, linkInfoDest, &tmpList);

			if (newDistance >= 0 && (minDistance < 0.0 || newDistance < minDistance)) {
				listObj->clear();

				listObj->push_back(linkInfoSource->link);
				listObj->push_back(tmpList);

				minDistance = newDistance;
			}
		}

		return minDistance;
	} else {
		if (linkInfoSource->link)
			listObj->push_back(linkInfoSource->link);

		return 0.0;
	}
}

MovGraphNode *MovGraph::calcOffset(int ox, int oy) {
	MovGraphNode *res = 0;
	double mindist = 1.0e10;

	for (NodeList::iterator i = _nodes.begin(); i != _nodes.end(); ++i) {
		assert((*i)->_objtype == kObjTypeMovGraphNode);

		MovGraphNode *node = static_cast<MovGraphNode *>(*i);

		double dist = sqrt((double)((node->_x - oy) * (node->_x - oy) + (node->_x - ox) * (node->_x - ox)));
		if (dist < mindist) {
			mindist = dist;
			res = node;
		}
	}

	return res;
}

MovGraphLink::MovGraphLink() {
	_length = 0;
	_angle = 0;
	_flags = 0x10000000;
	_graphDst = 0;
	_graphSrc = 0;
	_field_3C = 0;
	_field_38 = 0;
	_movGraphReact = 0;

	_objtype = kObjTypeMovGraphLink;
}

MovGraphLink::~MovGraphLink() {
	delete _movGraphReact;

	_dwordArray1.clear();
	_dwordArray2.clear();
}


bool MovGraphLink::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "MovGraphLink::load()");

	_dwordArray1.load(file);
	_dwordArray2.load(file);

	_flags = file.readUint32LE();

	debugC(8, kDebugLoading, "GraphNode1");
	_graphSrc = file.readClass<MovGraphNode>();
	debugC(8, kDebugLoading, "GraphNode2");
	_graphDst = file.readClass<MovGraphNode>();

	_length = file.readDoubleLE();
	_angle = file.readDoubleLE();

	debugC(8, kDebugLoading, "length: %g, angle: %g", _length, _angle);

	_movGraphReact = file.readClass<MovGraphReact>();
	_name = file.readPascalString();

	return true;
}

void MovGraphLink::recalcLength() {
	if (_graphSrc) {
		double dx = _graphDst->_x - _graphSrc->_x;
		double dy = _graphDst->_y - _graphSrc->_y;

		_length = sqrt(dy * dy + dx * dx);
		_angle = atan2(dy, dx);
	}
}

bool MovGraphNode::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "MovGraphNode::load()");

	_field_14 = file.readUint32LE();
	_x = file.readSint32LE();
	_y = file.readSint32LE();
	_z = file.readSint32LE();

	return true;
}

ReactParallel::ReactParallel() {
	_x1 = 0;
	_x2 = 0;
	_dy = 0;
	_dx = 0;
	_y1 = 0;
	_y2 = 0;
}

bool ReactParallel::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "ReactParallel::load()");

	_x1 = file.readSint32LE();
	_y1 = file.readSint32LE();
	_x2 = file.readSint32LE();
	_y2 = file.readSint32LE();
	_dx = file.readSint32LE();
	_dy = file.readSint32LE();

	createRegion();

	return true;
}

void ReactParallel::createRegion() {
	_points.resize(4);

	double at = atan2((double)(_y1 - _y2), (double)(_x1 - _x2)) + 1.570796; // pi/2
	double sn = sin(at);
	double cs = cos(at);

	_points[0].x = _x1 - _dx * cs;
	_points[0].y = _y1 - _dx * sn;

	_points[1].x = _x2 - _dx * cs;
	_points[1].y = _y2 - _dx * sn;

	_points[2].x = _x2 + _dy * cs;
	_points[2].y = _y2 + _dy * sn;

	_points[3].x = _x1 + _dy * cs;
	_points[3].y = _y1 + _dy * sn;

	// GdiObject::Attach(_rgn, CreatePolygonRgn(_points, 4, 2);
}

void ReactParallel::setCenter(int x1, int y1, int x2, int y2) {
	_x1 = x1;
	_y1 = y1;
	_x2 = x2;
	_y2 = y2;
}

ReactPolygonal::ReactPolygonal() {
	// hack for using isValid to avoid creating another state variable for
	// getBBox
	_bbox.right = -1;

	_centerX = 0;
	_centerY = 0;
}

bool ReactPolygonal::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "ReactPolygonal::load()");

	_centerX = file.readSint32LE();
	_centerY = file.readSint32LE();
	_points.resize(file.readUint32LE());

	for (uint i = 0; i < _points.size(); ++i) {
		_points[i].x = file.readUint32LE();
		_points[i].y = file.readUint32LE();
	}

	createRegion();

	return true;
}

void ReactPolygonal::createRegion() {
	if (_points.size()) {

		// GdiObject::Attach(_rgn, CreatePolygonRgn(_points, _pointCount, 2);
	}
}

void ReactPolygonal::setCenter(int x1, int y1, int x2, int y2) {
	int cX = (x2 + x1) / 2;
	int cY = (y2 + y1) / 2;

	for (uint i = 0; i < _points.size(); ++i) {
		_points[i].x += cX - _centerX;
		_points[i].y += cY - _centerY;
	}

	_centerX = cX;
	_centerY = cY;
}

Common::Rect ReactPolygonal::getBBox() {
	if (!_points.size())
		return Common::Rect();

	if (!_bbox.isValidRect()) {
		_bbox.left = _points[0].x;
		_bbox.top = _points[0].y;
		_bbox.right = _points[0].x;
		_bbox.bottom = _points[0].y;

		for (uint i = 1; i < _points.size(); ++i) {
			if (_bbox.left > _points[i].x)
				_bbox.left = _points[i].x;

			if (_bbox.top > _points[i].y)
				_bbox.top = _points[i].y;

			if (_bbox.right < _points[i].x)
				_bbox.right = _points[i].x;

			if (_bbox.bottom < _points[i].y)
				_bbox.bottom = _points[i].y;
		}
	}

	return _bbox;
}


bool MovGraphReact::pointInRegion(int x, int y) {
	if (_points.size() < 3) {
		return false;
	}

	int counter = 0;
	double xinters;
	Common::Point p, p1, p2;

	p.x = x;
	p.y = y;

	p1.x = _points[0].x;
	p1.y = _points[0].y;

	for (uint i = 1; i <= _points.size(); i++) {
		p2.x = _points[i % _points.size()].x;
		p2.y = _points[i % _points.size()].y;

		if (p.y > MIN(p1.y, p2.y)) {
			if (p.y <= MAX(p1.y, p2.y)) {
				if (p.x <= MAX(p1.x, p2.x)) {
					if (p1.y != p2.y) {
						xinters = (p.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;
						if (p1.x == p2.x || p.x <= xinters) {
							counter++;
						}
					}
				}
			}
		}
		p1 = p2;
	}

	if (counter % 2 == 0) {
		return false;
	} else {
		return true;
	}
}

int startWalkTo(int objId, int objKey, int x, int y, int fuzzyMatch) {
	MctlCompound *mc = getCurrSceneSc2MotionController();

	if (mc)
		return (mc->startMove(g_nmi->_currentScene->getStaticANIObject1ById(objId, objKey), x, y, fuzzyMatch, 0) != 0);

	return 0;
}

bool doSomeAnimation(int objId, int objKey, int a3) {
	StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObject1ById(objId, objKey);
	MctlCompound *cmp = getCurrSceneSc2MotionController();

	if (ani && cmp)
		return cmp->resetPosition(ani, a3);

	return false;
}

bool doSomeAnimation2(int objId, int objKey) {
	return doSomeAnimation(objId, objKey, 0);
}

} // End of namespace NGI
