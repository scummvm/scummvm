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
#include "ngi/gameloader.h"
#include "ngi/motion.h"

namespace NGI {

bool MotionController::load(MfcArchive &file) {
	// Is originally empty	file.readClass();

	debugC(5, kDebugLoading, "MotionController::load()");

	return true;
}

void MotionController::lockLink(const char *linkName, bool enable) {
	if (_objtype != kObjTypeMctlCompound)
		return;

	MctlCompound *obj = static_cast<MctlCompound *>(this);

	for (uint i = 0;  i < obj->getNumChildren(); i++) {
		MotionController *con = obj->getChild(i);

		if (con->_objtype == kObjTypeMovGraph) {
			MovGraph *gr = static_cast<MovGraph *>(con);

			for (MovGraph::LinkList::iterator l = gr->_links.begin(); l != gr->_links.end(); ++l) {
				assert((*l)->_objtype == kObjTypeMovGraphLink);

				MovGraphLink *lnk = static_cast<MovGraphLink *>(*l);

				if (lnk->_title == linkName) {
					if (enable)
						lnk->_flags |= 0x20000000;
					else
						lnk->_flags &= 0xDFFFFFFF;
				}
			}
		}
	}
}

MovGraphLink *MotionController::getGraphLink(const char *name) {
	debugC(4, kDebugPathfinding, "MotionController::getGraphLink(%s)", name);

	if (_objtype == kObjTypeMctlCompound) {
		MctlCompound *obj = static_cast<MctlCompound *>(this);

		for (uint i = 0;  i < obj->getNumChildren(); i++) {
			MotionController *con = obj->getChild(i);

			if (con->_objtype == kObjTypeMovGraph) {
				MovGraph *gr = static_cast<MovGraph *>(con);

				for (MovGraph::LinkList::iterator l = gr->_links.begin(); l != gr->_links.end(); ++l) {
					assert((*l)->_objtype == kObjTypeMovGraphLink);

					MovGraphLink *lnk = static_cast<MovGraphLink *>(*l);

					if (lnk->_title == name)
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

			if (lnk->_title == name)
				return lnk;
		}
	}

	return nullptr;
}

MctlCompound::~MctlCompound() {
	Common::for_each(_motionControllers.begin(), _motionControllers.end(), Common::DefaultDeleter<MctlItem>());
}

bool MctlCompound::checkPoint(int x, int y) {
	for (MctlItem *item : _motionControllers) {
		if (item->_movGraphReactObj && item->_movGraphReactObj->hitTest(x, y))
			return item->_motionControllerObj->checkPoint(x, y);
	}
	return false;
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

		obj->_ofsX = file.readUint32LE();
		obj->_ofsY = file.readUint32LE();

		debugC(6, kDebugLoading, "graphReact");
		obj->_movGraphReactObj.reset(file.readClass<MovGraphReact>());

		_motionControllers.push_back(obj);
	}

	return true;
}

void MctlCompound::attachObject(AniObject *obj) {
	debugC(4, kDebugPathfinding, "MctlCompound::attachObject(*%d)", obj->_id);

	for (uint i = 0; i < _motionControllers.size(); i++)
		_motionControllers[i]->_motionControllerObj->attachObject(obj);
}

int MctlCompound::detachObject(AniObject *obj) {
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

MessageQueue *MctlCompound::startMove(AniObject *ani, int sourceX, int sourceY, int fuzzyMatch, int staticsId) {
	int idx = -1;
	int sourceIdx = -1;

	debugC(4, kDebugPathfinding, "MctlCompound::startMove(*%d, %d, %d, %d, %d)", (ani ? ani->_id : -1), sourceX, sourceY, fuzzyMatch, staticsId);

	if (!ani)
		return nullptr;

	for (uint i = 0; i < _motionControllers.size(); i++) {
		if (_motionControllers[i]->_movGraphReactObj) {
			if (_motionControllers[i]->_movGraphReactObj->hitTest(ani->_x, ani->_y)) {
				idx = i;
				break;
			}
		}
	}

	for (uint i = 0; i < _motionControllers.size(); i++) {
		if (_motionControllers[i]->_movGraphReactObj) {
			if (_motionControllers[i]->_movGraphReactObj->hitTest(sourceX, sourceY)) {
				sourceIdx = i;
				break;
			}
		}
	}

	if (idx == -1)
		return nullptr;

	if (sourceIdx == -1)
		return nullptr;

	debugC(1, kDebugPathfinding, "WWW 2");
	if (idx == sourceIdx)
		return _motionControllers[idx]->_motionControllerObj->startMove(ani, sourceX, sourceY, fuzzyMatch, staticsId);

	double dist;
	MctlConnectionPoint *cp = getNextPoint(ani->_x, ani->_y, idx, sourceX, sourceY, sourceIdx, &dist);

	if (!cp)
		return nullptr;

	MessageQueue *mq = _motionControllers[idx]->_motionControllerObj->makeQueue(ani, cp->_x, cp->_y, 1, cp->_mirror);

	if (!mq)
		return nullptr;

	for (uint i = 0; i < cp->_messageQueueObj->getCount(); i++) {
		Command *ex = new Command(cp->_messageQueueObj->getCommandByIndex(i));

		ex->_excFlags |= 2;

		mq->addCommandToEnd(ex);
	}

	Command *ex = new Command(ani->_id, 51, 0, sourceX, sourceY, 0, 1, 0, 0, 0);

	ex->_excFlags |= 2;
	ex->_invId = fuzzyMatch;
	ex->_param = ani->_copyId;

	mq->addCommandToEnd(ex);

	if (!mq->run(ani)) {
		delete mq;
		return nullptr;
	}

	return mq;
}

MessageQueue *MctlCompound::makeQueue(AniObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	int match1 = -1;
	int match2 = -1;

	debugC(1, kDebugPathfinding, "MctlCompound::makeQueue(*%d, %d, %d, %d, %d)", (subj ? subj->_id : -1), xpos, ypos, fuzzyMatch, staticsId);

	if (!subj)
		return nullptr;

	for (uint i = 0; i < _motionControllers.size(); i++) {
		if (_motionControllers[i]->_movGraphReactObj) {
			if (_motionControllers[i]->_movGraphReactObj->hitTest(subj->_x, subj->_y)) {
				match1 = i;
				break;
			}
		}
	}

	if (match1 == -1)
		return nullptr;

	for (uint i = 0; i < _motionControllers.size(); i++) {
		if (_motionControllers[i]->_movGraphReactObj) {
			if (_motionControllers[i]->_movGraphReactObj->hitTest(xpos, ypos)) {
				match2 = i;
				break;
			}
		}
	}

	if (match2 == -1)
		return nullptr;

	if (match1 == match2)
		return _motionControllers[match1]->_motionControllerObj->makeQueue(subj, xpos, ypos, fuzzyMatch, staticsId);

	double dist;
	MctlConnectionPoint *closestP = getNextPoint(subj->_x, subj->_y, match1, xpos, ypos, match2, &dist);

	if (!closestP)
		return nullptr;

	MessageQueue *mq = _motionControllers[match1]->_motionControllerObj->makeQueue(subj, closestP->_x, closestP->_y, 1, closestP->_mirror);

	Command *ex;

	if (mq) {
		for (uint i = 0; i < closestP->_messageQueueObj->getCount(); i++) {
			ex = closestP->_messageQueueObj->getCommandByIndex(i)->createClone();
			ex->_excFlags |= 2;
			mq->addCommandToEnd(ex);
		}

		ex = new Command(subj->_id, 51, 0, xpos, ypos, 0, 1, 0, 0, 0);

		ex->_invId = fuzzyMatch;
		ex->_param = subj->_copyId;
		ex->_excFlags |= 2;

		mq->addCommandToEnd(ex);
	}

	return mq;
}

MctlLadder::MctlLadder() {
	_stepX = 0;
	_x = 0;
	_stepY = 0;
	_y = 0;
	_z = 0;

	_entryOffsetX = 0;
	_entryOffsetY = 0;
}

MctlLadder::~MctlLadder() {
	detachAllObjects();
}

int MctlLadder::getCurrentStep(AniObject *man) {
	if (findIndex(man) < 0)
		return 0;

	double delta;

	if ((double)(man->_y - _y) / (double)_stepY < 0.0)
		delta = -0.5;
	else
		delta = 0.5;

	int res = (int)((double)(man->_y - _y) / (double)_stepY + delta);

	if (res < 0)
		return 0;

	return res;
}

void MctlLadder::attachObject(AniObject *obj) {
	debugC(4, kDebugPathfinding, "MctlLadder::attachObject(*%d)", obj->_id);

	if (findIndex(obj) < 0) {
		LmctlAni *movement = new LmctlAni;

		if (fillData(obj, movement)) {
			_aniHandler.attachObject(obj->_id);
			_ladmovements.push_back(movement);
		} else {
			delete movement->movementIds;
			delete[] movement->staticIds;
			delete movement;
		}
	}
}

int MctlLadder::findIndex(AniObject *obj) {
	for (uint i = 0; i < _ladmovements.size(); i++)
		if (_ladmovements[i]->aniId == obj->_id)
			return i;

	return -1;
}

bool MctlLadder::fillData(AniObject *ani, LmctlAni *movement) {
	debugC(4, kDebugPathfinding, "MctlLadder::fillData(*%d, ...)", ani->_id);
	movement->movementIds = nullptr;
	movement->staticIds = nullptr;

	GameVar *v = g_nmi->getGameLoaderGameVar()->getSubVarByName(ani->getName());

	if (!v)
		return false;

	GameVar *l = v->getSubVarByName("Test_Ladder");

	if (!l)
		return false;

	movement->staticIdCount = 6;
	movement->movementIds = new MctlLadderMovementVars;
	movement->staticIds = new int[movement->staticIdCount];

	v = l->getSubVarByName("Up");

	if (!v)
		return false;

	movement->movementIds->upStartId = v->getSubVarAsInt("Start");
	movement->movementIds->upGoId = v->getSubVarAsInt("Go");
	movement->movementIds->upStopId = v->getSubVarAsInt("Stop");

	movement->staticIds[0] = ani->getMovement(movement->movementIds->upStartId)->_prevStatic->_staticsId;
	movement->staticIds[2] = ani->getMovement(movement->movementIds->upGoId)->_prevStatic->_staticsId;

	v = l->getSubVarByName("Down");

	if (!v)
		return false;

	movement->movementIds->downStartId = v->getSubVarAsInt("Start");
	movement->movementIds->downGoId = v->getSubVarAsInt("Go");
	movement->movementIds->downStopId = v->getSubVarAsInt("Stop");

	movement->staticIds[1] = ani->getMovement(movement->movementIds->downStartId)->_prevStatic->_staticsId;
	movement->staticIds[3] = ani->getMovement(movement->movementIds->downGoId)->_prevStatic->_staticsId;

	movement->aniId = ani->_id;

	return true;
}

void MctlLadder::detachAllObjects() {
	debugC(4, kDebugPathfinding, "MctlLadder::detachAllObjects()");

	_aniHandler.detachAllObjects();

	for (uint i = 0; i < _ladmovements.size(); i++) {
		delete _ladmovements[i]->movementIds;
		delete[] _ladmovements[i]->staticIds;
	}

	_ladmovements.clear();
}

MessageQueue *MctlLadder::startMove(AniObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	debugC(4, kDebugPathfinding, "MctlLadder::startMove(*%d, %d, %d, %d, %d)", (subj ? subj->_id : -1), xpos, ypos, fuzzyMatch, staticsId);

	MessageQueue *mq = makeQueue(subj, xpos, ypos, fuzzyMatch, staticsId);

	if (mq) {
		if (mq->run(subj))
			return mq;
	}

	return nullptr;
}

MessageQueue *MctlLadder::makeQueue(AniObject *ani, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	debugC(1, kDebugPathfinding, "MctlLadder::makeQueue(*%d, %d, %d, %d, %d)", (ani ? ani->_id : -1), xpos, ypos, fuzzyMatch, staticsId);

	int pos = findIndex(ani);

	if (pos < 0)
		return nullptr;

	double dh = _stepY;
	double corr = (double)(ani->_y - _y) / dh;
	int dl = (int)(corr + (corr < 0.0 ? -0.5 : 0.5));

	corr = (double)(ypos - _y) / dh;
	int dl2 = (int)(corr + (corr < 0.0 ? -0.5 : 0.5));

	int normx = _x + dl2 * _stepX;
	int normy = _y + dl2 * _stepY;

	if (dl == dl2 || dl2 < 0)
		return nullptr;

	int direction = (normy - ani->_y) < 0 ? 0 : 1;

	MakeQueueStruct mkQueue;
	PicAniInfo picinfo;
	MessageQueue *mq;
	Command *ex;
	Common::Point point;

	if (ani->_movement) {
		ani->getPicAniInfo(picinfo);

		int ox = ani->_x;
		int oy = ani->_y;

		point = ani->_movement->getMovementSize(1, ani->_targetFrame);
		ani->_statics = ani->_movement->_nextStatic;
		ani->_movement = nullptr;
		ani->setPos(point.x + ox, point.y + oy);

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
		mkQueue.destZ = _z;
		mkQueue.flags = 14;
		mkQueue.movementId = direction ? _ladmovements[pos]->movementIds->downGoId : _ladmovements[pos]->movementIds->upGoId;

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
			mkQueue.destZ = _z;
			mkQueue.flags = 14;
			mkQueue.movementId = _ladmovements[pos]->movementIds->upGoId;

			return _aniHandler.makeRunQueue(&mkQueue);
		}

		int ox = ani->_x;
		int oy = ani->_y;

		point = ani->getMovement(_ladmovements[pos]->movementIds->upStopId)->getMovementSize(0, -1);

		mkQueue.ani = ani;

		if (staticsId)
			mkQueue.staticsId2 = staticsId;
		else
			mkQueue.staticsId2 = _ladmovements[pos]->staticIds[1];

		mkQueue.destZ = _z;
		mkQueue.x1 = normx;
		mkQueue.y1 = normy;
		mkQueue.y2 = point.y + oy;
		mkQueue.x2 = point.x + ox;
		mkQueue.flags = 63;
		mkQueue.staticsId1 = _ladmovements[pos]->staticIds[0];
		mkQueue.movementId = _ladmovements[pos]->movementIds->downGoId;

		mq = _aniHandler.makeRunQueue(&mkQueue);

		ex = new Command(ani->_id, 1, _ladmovements[pos]->movementIds->upStopId, 0, 0, 0, 1, 0, 0, 0);
		ex->_param = ani->_copyId;
		ex->_excFlags |= 2;

		mq->addCommand(0, ex);

		return mq;
	}

	if (ani->_statics->_staticsId != _ladmovements[pos]->staticIds[3]) {
		mq = _aniHandler.makeQueue(ani, _ladmovements[pos]->staticIds[0], 0, nullptr, nullptr);

		if (!mq)
			return nullptr;

		int nx = ani->_x;
		int ny = ani->_y;

		point = _aniHandler.getTransitionSize(ani->_id, ani->_statics->_staticsId, _ladmovements[pos]->staticIds[0]);

		nx += point.x;
		ny += point.y;

		ani->getPicAniInfo(picinfo);

		ani->_statics = ani->getStatic(_ladmovements[pos]->staticIds[0]);
		ani->_movement = nullptr;
		ani->setPos(nx, ny);

		MessageQueue *newmq = makeQueue(ani, normx, normy, fuzzyMatch, staticsId);

		mq->mergeQueue(newmq);

		delete newmq;

		ani->setPicAniInfo(picinfo);

		return mq;
	}

	if (!direction) {
		int nx = ani->_x;
		int ny = ani->_y;

		point = ani->getMovement(_ladmovements[pos]->movementIds->downStopId)->getMovementSize(0, -1);

		nx += point.x;
		ny += point.y;

		mkQueue.ani = ani;
		if (staticsId)
			mkQueue.staticsId2 = staticsId;
		else
			mkQueue.staticsId2 = _ladmovements[pos]->staticIds[0];

		mkQueue.destZ = _z;
		mkQueue.x1 = normx;
		mkQueue.y1 = normy;
		mkQueue.y2 = ny;
		mkQueue.x2 = nx;
		mkQueue.flags = 63;
		mkQueue.staticsId1 = _ladmovements[pos]->staticIds[1];
		mkQueue.movementId = _ladmovements[pos]->movementIds->upGoId;

		mq = _aniHandler.makeRunQueue(&mkQueue);

		ex = new Command(ani->_id, 1, _ladmovements[pos]->movementIds->downStopId, 0, 0, 0, 1, 0, 0, 0);
		ex->_param = ani->_copyId;
		ex->_excFlags |= 2;

		mq->addCommand(0, ex);

		return mq;
	}


	mkQueue.ani = ani;

	if (staticsId)
		mkQueue.staticsId2 = staticsId;
	else
		mkQueue.staticsId2 = _ladmovements[pos]->staticIds[1];

	mkQueue.x1 = normx;
	mkQueue.y1 = normy;
	mkQueue.destZ = _z;
	mkQueue.flags = 14;
	mkQueue.movementId = _ladmovements[pos]->movementIds->downGoId;

	return _aniHandler.makeRunQueue(&mkQueue);
}

MessageQueue *MctlLadder::makeQueue(AniObject *ani, int off) {
	return makeQueue(ani, _x + off * _stepX, _y + off * _stepY, 1, 0);
}

MctlConnectionPoint *MctlCompound::getNextPoint(int ox, int oy, int destIndex, int connectionX, int connectionY, int sourceIdx, double *minDistancePtr) {
	if (destIndex == sourceIdx) {
		*minDistancePtr = sqrt((double)((oy - connectionY) * (oy - connectionY) + (ox - connectionX) * (ox - connectionX)));

		return nullptr;
	}

	double currDistance = 0.0;
	double minDistance = 1.0e10;
	MctlConnectionPoint *minConnectionPoint = nullptr;

	for (uint i = 0; i < _motionControllers[sourceIdx]->_connectionPoints.size(); i++) {
		for (uint j = 0; j < _motionControllers.size(); j++) {
			if (_motionControllers[j]->_movGraphReactObj) {
				MctlConnectionPoint *pt = _motionControllers[sourceIdx]->_connectionPoints[i];

				if (_motionControllers[j]->_movGraphReactObj->hitTest(pt->_x, pt->_y)) {
					MctlConnectionPoint *npt = getNextPoint(ox, oy, destIndex, pt->_x, pt->_y, j, &currDistance);

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
	_x = 0;
	_y = 0;
	_flags = 0;
	_static = 0;
	_mirror = 0;
}

void GmctlMqStruct::clear() {
	objectIndex = 0;
	sourcePos.x = sourcePos.y = 0;
	destPos.x = destPos.y = 0;
	sourceZ = 0;
	destZ = 0;
	sourceDir = 0;
	destDir = 0;
	items.clear();
	flags = 0;
}

MctlItem::~MctlItem() {
	Common::for_each(_connectionPoints.begin(), _connectionPoints.end(), Common::DefaultDeleter<MctlConnectionPoint>());
}

NavDatum::NavDatum() {
	ani = nullptr;
	field_4 = 0;
	candidatePaths = nullptr;
	candidatePathCount = 0;
	field_30 = 0;
	field_34 = 0;
	field_38 = 0;
	field_3C = 0;
}

void NavDatum::clearPathList() {
	if (!candidatePaths)
		return;

	for (uint i = 0; i < candidatePaths->size(); i++) {
		(*candidatePaths)[i]->path->_steps.clear();
		delete (*candidatePaths)[i]->path;
	}

	delete candidatePaths;

	candidatePaths = nullptr;
}

GrPathDesc *getPreferredPath(AniObject *ani, Common::Array<PathDesc *> *items, signed int counter) {
	int residx = 0;
	int itemidx = 0;

	while (counter > 1) {
		if ((*items)[itemidx]->_length > (*items)[itemidx + 1]->_length)
			residx = itemidx;

		counter--;
		itemidx++;
	}

	return (*items)[residx]->path;
}

MovGraph::MovGraph() {
	_selFunc = getPreferredPath;
	_linkItemOffset = 0;
	if (g_nmi->getPlatform() != Common::kPlatformAndroid)
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

MovGraphNode *MovGraph::getNodeById(int id) const {
	for (MovGraphNode *node : _nodes) {
		if (node->_id == id)
			return node;
	}
	return nullptr;
}

bool MovGraph::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "MovGraph::load()");

	_links.load(file);
	_nodes.load(file);

	return true;
}

void MovGraph::attachObject(AniObject *obj) {
	debugC(4, kDebugPathfinding, "MovGraph::attachObject(*%d)", obj->_id);

	_aniHandler.detachAllObjects();
	_aniHandler.attachObject(obj->_id);

	for (uint i = 0; i < _items.size(); i++)
		if (_items[i].ani == obj)
			return;

	_items.push_back(NavDatum());
	NavDatum &item = _items.back();
	item.ani = obj;

	_aniHandler.attachObject(obj->_id); // FIXME: Is it really needed?
}

int MovGraph::detachObject(AniObject *obj) {
	warning("STUB: MovGraph::detachObject()");

	return 0;
}

void MovGraph::detachAllObjects() {
	debugC(4, kDebugPathfinding, "MovGraph::detachAllObjects()");

	for (uint i = 0; i < _items.size(); i++) {
		_items[i].clearPathList();

		_items[i].currentPath._steps.clear();
	}

	_items.clear();
}

Common::Array<PathDesc *> *MovGraph::getPaths(AniObject *ani, int x, int y, int flag1, int *rescount) {
	debugC(4, kDebugPathfinding, "MovGraph::getPaths(*%d, %d, %d, %d, &rescount)", (ani ? ani->_id : -1), x, y, flag1);

	*rescount = 0;

	if (_items.size() <= 0)
		return nullptr;

	uint idx = 0;

	while (_items[idx].ani != ani) {
		idx++;

		if (idx >= _items.size())
			return nullptr;
	}
	_items[idx].clearPathList();

	recalcLinkParams();

	_items[idx].currentPath._steps.clear();

	Common::Point point;

	point.x = ani->_x;
	point.y = ani->_y;

	if (!getHitPoint(idx, ani->_x, ani->_y, &_items[idx].currentPath, 0))
		getNearestPoint(idx, &point, &_items[idx].currentPath);

	_items[idx].candidatePathCount = 0;

	delete _items[idx].candidatePaths;
	_items[idx].candidatePaths = nullptr;

	int arrSize;
	Common::Array<GrPathDesc *> *movarr = getHitPoints(x, y, &arrSize, flag1, 0);

	if (movarr) {
		for (int i = 0; i < arrSize; i++) {
			int sz;
			Common::Array<PathDesc *> *movitems = getPaths(&_items[idx].currentPath, (*movarr)[i], &sz);

			if (sz > 0) {
				_items[idx].candidatePaths = new Common::Array<PathDesc *>;
				for (int j = 0; j < sz; j++)
					_items[idx].candidatePaths->push_back(movitems[j]);
			}

			delete movitems;
		}

		delete movarr;
	}

	if (_items[idx].candidatePathCount) {
		*rescount = _items[idx].candidatePathCount;

		return _items[idx].candidatePaths;
	}

	return nullptr;
}

bool MovGraph::setPosImmediate(AniObject *obj, int x, int y) {
	obj->setPos(x, y);
	return resetPosition(obj, 1);
}

MessageQueue *MovGraph::startMove(AniObject *ani, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	debugC(4, kDebugPathfinding, "MovGraph::startMove(*%d, %d, %d, %d, %d)", (ani ? ani->_id : -1), xpos, ypos, fuzzyMatch, staticsId);

	if (!ani) {
		if (!_items.size())
			return nullptr;

		ani = _items[0].ani;
	}

	if (ABS(ani->_x - xpos) < 50 && ABS(ani->_y - ypos) < 50)
		return nullptr;

	if (!ani->isIdle())
		return nullptr;

	if (ani->_flags & 0x100)
		return nullptr;

	int count;
	Common::Array<PathDesc *> *movitems = getPaths(ani, xpos, ypos, fuzzyMatch, &count);

	if (!movitems)
		return nullptr;

	if (ani->_movement) {
		Common::Point point;

		ani->calcStepLen(&point);

		MessageQueue *mq = sub1(ani, ani->_x - point.x, ani->_y - point.y, ani->_movement->_prevStatic->_staticsId, xpos, ypos, 0, fuzzyMatch);

		if (!mq || !mq->getCommandByIndex(0))
			return nullptr;

		Command *ex = mq->getCommandByIndex(0);

		if ((ex->_cmd != 1 && ex->_cmd != 20) || ex->_messageNum != ani->_movement->_id ||
			(ex->_z >= 1 && ex->_z <= ani->_movement->_curFrame)) {
			mq = new MessageQueue(g_nmi->_globalMessageQueueList->compact());

			ex = new Command(ani->_id, 21, 0, 0, 0, 0, 1, 0, 0, 0);
			ex->_param = ani->_copyId;
			ex->_field_3C = 1;
			ex->_wait = 0;
			mq->addCommandToEnd(ex);

			ex = new Command(ani->_id, 51, 0, xpos, ypos, 0, 1, 0, 0, 0);
			ex->_param = ani->_copyId;
			ex->_field_3C = 1;
			ex->_wait = 0;
			ex->_invId = fuzzyMatch;
			mq->addCommandToEnd(ex);

			if (mq->run(nullptr))
				return mq;

			delete mq;

			return nullptr;
		}

		int count2;

		ani->setTargetFrame(ex->_z);
		getPaths(ani, xpos, ypos, fuzzyMatch, &count2);

		int idx = getObjectIndex(ani);
		count = _items[idx].candidatePathCount;
		movitems = _items[idx].candidatePaths;
	}

	return method50(ani, _selFunc(ani, movitems, count), staticsId);
}

void MovGraph::setSelFunc(GrPathDesc *(*callback1)(AniObject *ani, Common::Array<PathDesc *> *items, signed int counter)) {
	_selFunc = callback1;
}

bool MovGraph::resetPosition(AniObject *ani, int flag) {
	debugC(4, kDebugPathfinding, "MovGraph::resetPosition(*%d, %d)", (ani ? ani->_id : -1), flag);

	int idx = getObjectIndex(ani);

	if (idx == -1)
		return false;

	Common::Point point;
	GrPathDesc movarr;

	point.x = ani->_x;
	point.y = ani->_y;

	getNearestPoint(idx, &point, &movarr);
	ani->setPos(point.x, point.y);

	if (flag) {
		Statics *st;

		if (ani->_statics) {
			int t = _aniHandler.getNumMovements(ani->_id, ani->_statics->_staticsId, movarr._link->_staticsIds[_linkItemOffset]);
			if (t > _aniHandler.getNumMovements(ani->_id, ani->_statics->_staticsId, movarr._link->_staticsIds[_linkItemOffset + 1]))
				st = ani->getStatic(movarr._link->_staticsIds[_linkItemOffset + 1]);
			else
				st = ani->getStatic(movarr._link->_staticsIds[_linkItemOffset]);
		} else {
			ani->stopMovement();
			st = ani->getStatic(movarr._link->_staticsIds[_linkItemOffset]);
		}

		ani->_statics = st;
	}

	return true;
}

bool MovGraph::canDropInventory(AniObject *ani, int x, int y) {
	int idx = getObjectIndex(ani);
	GrPathDesc m;

	if (idx != -1) {
		if (x != -1 || y != -1) {
			int counter;

			Common::Array<PathDesc *> *movitem = getPaths(ani, x, y, 0, &counter);

			if (movitem) {
				GrPathDesc *movarr = _selFunc(ani, movitem, counter);
				int cnt = movarr->_stepCount;

				if (cnt > 0) {
					if (movarr->_steps[cnt - 1]->link->_flags & 0x4000000)
						return true;
				}
			}
		} else if (getHitPoint(idx, ani->_x, ani->_y, &m, 0) && m._link && (m._link->_flags & 0x4000000)) {
			return true;
		}
	}

	return false;
}

MessageQueue *MovGraph::makeQueue(AniObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	debugC(1, kDebugPathfinding, "MovGraph::makeQueue(*%d, %d, %d, %d, %d)", (subj ? subj->_id : -1), xpos, ypos, fuzzyMatch, staticsId);

	PicAniInfo picAniInfo;
	int ss;

	Common::Array<PathDesc *> *movitem = getPaths(subj, xpos, ypos, fuzzyMatch, &ss);

	subj->getPicAniInfo(picAniInfo);

	if (movitem) {
		GrPathDesc *goal = _selFunc(subj, movitem, ss);
		int idx = getObjectIndex(subj);

		for (int i = 0; i < _items[idx].candidatePathCount; i++) {
			if ((*_items[idx].candidatePaths)[i]->path == goal) {
				if (subj->_movement) {
					Common::Point point;

					subj->calcStepLen(&point);

					MessageQueue *mq = sub1(subj, subj->_x - point.x, subj->_y - point.y, subj->_movement->_prevStatic->_staticsId, xpos, ypos, 0, fuzzyMatch);

					if (!mq || !mq->getCommandByIndex(0))
						return nullptr;

					Command *ex = mq->getCommandByIndex(0);

					if ((ex->_cmd != 1 && ex->_cmd != 20) ||
						ex->_messageNum != subj->_movement->_id ||
						(ex->_z >= 1 && ex->_z <= subj->_movement->_curFrame))
						subj->playIdle();
				}
			}
		}
	}

	movitem = getPaths(subj, xpos, ypos, fuzzyMatch, &ss);
	if (movitem) {
		GrPathDesc *goal = _selFunc(subj, movitem, ss);
		int idx = getObjectIndex(subj);

		if (_items[idx].candidatePathCount > 0) {
			int arridx = 0;

			while ((*_items[idx].candidatePaths)[arridx]->path != goal) {
				arridx++;

				if (arridx >= _items[idx].candidatePathCount) {
					subj->setPicAniInfo(picAniInfo);
					return nullptr;
				}
			}

			_items[idx].currentPath._steps.clear();
			_items[idx].currentPath = *(*_items[idx].candidatePaths)[arridx]->path;
			_items[idx].currentPath._steps = (*_items[idx].candidatePaths)[arridx]->path->_steps;
			_items[idx].currentPath._afield_8 = -1;
			_items[idx].currentPath._link = nullptr;

			MessageQueue *mq = makeWholeQueue(_items[idx].ani, &_items[idx].currentPath, staticsId);
			if (mq) {
				Command *ex = new Command();
				ex->_cmd = 17;
				ex->_messageNum = 54;
				ex->_parentId = subj->_id;
				ex->_field_3C = 1;
				mq->addCommandToEnd(ex);
			}
			subj->setPicAniInfo(picAniInfo);

			return mq;
		}
	}

	subj->setPicAniInfo(picAniInfo);

	return nullptr;
}

MessageQueue *MovGraph::sub1(AniObject *ani, int x, int y, int stid, int x1, int y1, int stid2, int flag1) {
	debugC(4, kDebugPathfinding, "MovGraph::sub1(*%d, %d, %d, %d, %d, %d, %d, %d)", (ani ? ani->_id : -1), x, y, stid, x1, y1, stid2, flag1);

	PicAniInfo picinfo;

	ani->getPicAniInfo(picinfo);

	ani->_statics = ani->getStatic(stid);
	ani->_movement = nullptr;
	ani->setPos(x, y);

	int rescount;

	Common::Array<PathDesc *> *movitems = getPaths(ani, x1, y1, flag1, &rescount);

	if (!movitems) {
		ani->setPicAniInfo(picinfo);

		return nullptr;
	}

	MessageQueue *res = nullptr;

	GrPathDesc *goal = _selFunc(ani, movitems, rescount);
	int idx = getObjectIndex(ani);

	NavDatum &movgitem = _items[idx];
	int cnt = movgitem.candidatePathCount;

	for (int nidx = 0; nidx < cnt; nidx++) {
		if ((*movgitem.candidatePaths)[nidx]->path == goal) {
			movgitem.currentPath._steps.clear();
			_items[idx].currentPath = *(*movgitem.candidatePaths)[nidx]->path;
			_items[idx].currentPath._steps = (*movgitem.candidatePaths)[nidx]->path->_steps;
			_items[idx].currentPath._afield_8 = -1;
			_items[idx].currentPath._link = nullptr;

			res = makeWholeQueue(_items[idx].ani, &_items[idx].currentPath, stid2);

			break;
		}
	}

	ani->setPicAniInfo(picinfo);

	return res;
}

MessageQueue *MovGraph::makeWholeQueue(AniObject *ani, GrPathDesc *movarr, int staticsId) {
	debugC(4, kDebugPathfinding, "MovGraph::makeWholeQueue(*%d, *, %d)", (ani ? ani->_id : -1), staticsId);

	if (!movarr->_stepCount)
		return nullptr;

	MessageQueue *mq = nullptr;
	int ox = ani->_x;
	int oy = ani->_y;
	int id1 = 0;
	int id2;

	for (int i = 0; i < movarr->_stepCount; i++) {
		while (i < movarr->_stepCount - 1) {
			if (movarr->_steps[i    ]->link->_movementIds[movarr->_steps[i - 1]->reverse + _linkItemOffset] !=
				movarr->_steps[i + 1]->link->_movementIds[movarr->_steps[i    ]->reverse + _linkItemOffset])
				break;
			i++;
		}

		MovStep *st = movarr->_steps[i];

		ani->getMovement(st->link->_movementIds[_linkItemOffset + st->reverse]);

		if (i == movarr->_stepCount - 1 && staticsId) {
			id2 = staticsId;
		} else {
			if (i < movarr->_stepCount - 1)
				id2 = ani->getMovement(movarr->_steps[i + 1]->link->_movementIds[_linkItemOffset + st->reverse])->_prevStatic->_staticsId;
			else
				id2 = st->link->_staticsIds[_linkItemOffset + st->reverse];
		}

		int nx, ny, nd;

		if (i == movarr->_stepCount - 1) {
			nx = movarr->_position.x;
			ny = movarr->_position.y;
			nd = st->link->_graphSrc->_z;
		} else {
			if (st->reverse) {
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
		mkQueue.destZ = nd;
		mkQueue.movementId = st->link->_movementIds[_linkItemOffset + st->reverse];

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

MessageQueue *MovGraph::method50(AniObject *ani, GrPathDesc *movarr, int staticsId) {
	debugC(4, kDebugPathfinding, "MovGraph::method50(*%d, *, %d)", (ani ? ani->_id : -1), staticsId);

	if (_items.size() == 0)
		return nullptr;

	uint idx;
	int movidx = 0;
	bool done = false;

	for (idx = 0; idx <= _items.size() && !done; idx++) {
		if (idx == _items.size())
			return nullptr;

		if (_items[idx].ani == ani) {
			if (!_items[idx].candidatePaths)
				return nullptr;

			if (_items[idx].candidatePathCount < 1)
				return nullptr;

			for (movidx = 0; movidx < _items[idx].candidatePathCount; movidx++) {
				if ((*_items[idx].candidatePaths)[movidx]->path == movarr) {
					done = true;

					break;
				}
			}
		}
	}

	_items[idx].currentPath._steps.clear();
	_items[idx].currentPath = *(*_items[idx].candidatePaths)[movidx]->path;
	_items[idx].currentPath._steps = (*_items[idx].candidatePaths)[movidx]->path->_steps;
	_items[idx].currentPath._afield_8 = -1;
	_items[idx].currentPath._link = nullptr;

	MessageQueue *mq = makeWholeQueue(_items[idx].ani, &_items[idx].currentPath, 0);

	if (!mq)
		return nullptr;

	Command *ex = new Command();

	ex->_cmd = 17;
	ex->_messageNum = 54;
	ex->_parentId = ani->_id;
	ex->_field_3C = 1;
	mq->addCommandToEnd(ex);

	if (!mq->run(ani)) {
		delete mq;

		return nullptr;
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

bool MovGraph::getNearestPoint(int unusedArg, Common::Point *p, GrPathDesc *movarr) {
	debugC(4, kDebugPathfinding, "MovGraph::getNearestPoint(...)");

	MovGraphLink *link = nullptr;
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

int MovGraph::getObjectIndex(AniObject *ani) {
	for (uint i = 0; i < _items.size(); i++)
		if (_items[i].ani == ani)
			return i;

	return -1;
}

Common::Array<GrPathDesc *> *MovGraph::getHitPoints(int x, int y, int *arrSize, int flag1, int flag2) {
	debugC(4, kDebugPathfinding, "MovGraph::getHitPoints(...)");

	if (!_links.size()) {
		*arrSize = 0;

		return nullptr;
	}

	Common::Array<GrPathDesc *> *arr = new Common::Array<GrPathDesc *>;
	GrPathDesc *movarr;

	for (LinkList::iterator i = _links.begin(); i != _links.end(); ++i) {
		MovGraphLink *lnk = static_cast<MovGraphLink *>(*i);

		if (flag1) {
			Common::Point point(x, y);
			double dist = putToLink(&point, lnk, 0);

			if (dist >= 0.0 && dist < 2.0) {
				movarr = new GrPathDesc;

				movarr->_link = lnk;
				movarr->_linkFraction = ((double)(lnk->_graphSrc->_y - lnk->_graphDst->_y) * (double)(lnk->_graphSrc->_y - point.y) +
								 (double)(lnk->_graphDst->_x - lnk->_graphSrc->_x) * (double)(point.x - lnk->_graphSrc->_x)) /
					lnk->_length / lnk->_length;
				movarr->_position = point;

				arr->push_back(movarr);
			}
		} else {
			if (lnk->_movGraphReact) {
				if (lnk->_movGraphReact->hitTest(x, y)) {
					if (!(lnk->_flags & 0x10000000) || lnk->_flags & 0x20000000) {
						if (!flag2) {
							movarr = new GrPathDesc;
							movarr->_link = lnk;
							movarr->_linkFraction = 0.0;
							movarr->_position.x = lnk->_graphSrc->_x;
							movarr->_position.y = lnk->_graphSrc->_y;
							arr->push_back(movarr);

							movarr = new GrPathDesc;
							movarr->_link = lnk;
							movarr->_linkFraction = 1.0;
							movarr->_position.x = lnk->_graphSrc->_x;
							movarr->_position.y = lnk->_graphSrc->_y;
							arr->push_back(movarr);
						}
					} else {
						movarr = new GrPathDesc;
						movarr->_link = lnk;
						movarr->_linkFraction = ((double)(lnk->_graphSrc->_y - lnk->_graphDst->_y) * (double)(lnk->_graphSrc->_y - y) +
										 (double)(lnk->_graphDst->_x - lnk->_graphSrc->_x) * (double)(x - lnk->_graphSrc->_x)) /
							lnk->_length / lnk->_length;
						movarr->_position.x = x;
						movarr->_position.y = y;

						putToLink(&movarr->_position, lnk, 0);

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
Common::Array<PathDesc *> *MovGraph::getPaths(GrPathDesc *currPos, GrPathDesc *destPos, int *pathCount) {
	debugC(4, kDebugPathfinding, "MovGraph::getPaths(...)");

	MovGraphLinkList tempObList1;
	MovGraphLinkList allPaths;

	// Get all paths between two edges of the graph
	findAllPaths(currPos->_link, destPos->_link, tempObList1, allPaths);

	*pathCount = 0;

	if (!allPaths.size())
		return nullptr;

	*pathCount = allPaths.size();

	Common::Array<PathDesc *> *res = new Common::Array<PathDesc *>;

	for (int i = 0; i < *pathCount; i++) {
		PathDesc *r = new PathDesc;

		genMovItem(r, allPaths[i], currPos, destPos);

		res->push_back(r);

		delete allPaths[i];
	}

	// Start the resulting path from current position
	destPos->_link = currPos->_link;

	return res;
}

void MovGraph::genMovItem(PathDesc *movitem, MovGraphLink *grlink, GrPathDesc *movarr1, GrPathDesc *movarr2) {
	warning("STUB: MovGraph::genMovItem()");
}

bool MovGraph::getHitPoint(int idx, int x, int y, GrPathDesc *arr, int a6) {
	int staticsId;

	if (_items[idx].ani->_statics) {
		staticsId = _items[idx].ani->_statics->_staticsId;
	} else {
		if (!_items[idx].ani->_movement->_nextStatic)
			return 0;

		staticsId = _items[idx].ani->_movement->_nextStatic->_staticsId;
	}

	int arrSize;

	Common::Array<GrPathDesc *> *movarr = getHitPoints(x, y, &arrSize, 0, 1);

	if (!movarr)
		return getNearestPoint(idx, nullptr, arr);

	bool res = false;

	int idxmin = -1;
	int offmin = 100;

	for (int i = 0; i < arrSize; i++) {
		int off = _aniHandler.getNumMovements(_items[idx].ani->_id, staticsId, (*movarr)[i]->_link->_staticsIds[_linkItemOffset]);

		if (off < offmin) {
			offmin = off;
			idxmin = i;
		}

		off = _aniHandler.getNumMovements(_items[idx].ani->_id, staticsId, (*movarr)[i]->_link->_staticsIds[_linkItemOffset + 1]);
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
		step1->reverse = 1;
		step2->reverse = 1;

		return;
	}

	if (step1->link->_graphSrc == step2->link->_graphSrc) {
		step1->reverse = 1;
		step2->reverse = 0;
	} else {
		step1->reverse = 0;

		if (step1->link->_graphDst != step2->link->_graphSrc) {
			step2->reverse = 1;
		} else {
			step2->reverse = 0;
		}
	}
}

int MctlGraph::getObjIndex(int objectId) {
	for (uint i = 0; i < _aniData.size(); i++)
		if (_aniData[i]._objectId == objectId)
			return i;

	return -1;
}

int MctlGraph::getDirByStatics(int idx, int staticsId) {
	for (int i = 0; i < 4; i++)
		if (_aniData[idx]._directions[i]._prevStaticId == staticsId || _aniData[idx]._directions[i]._nextStaticId == staticsId)
			return i;

	return -1;
}

int MctlGraph::getDirByMovement(int idx, int movId) {
	for (int i = 0; i < 4; i++)
		if (_aniData[idx]._directions[i]._walk[0]._movementId == movId
		 || _aniData[idx]._directions[i]._walk[1]._movementId == movId
		 || _aniData[idx]._directions[i]._walk[2]._movementId == movId)
			return i;

	return -1;
}

int MctlGraph::getDirByPoint(int index, AniObject *ani) {
	if (getHitNode(ani->_x, ani->_y, 0) || getHitLink(ani->_x, ani->_y, -1, 0) || getNearestLink(ani->_x, ani->_y)) {
		int minidx = -1;
		int min = 0;

		for (int i = 0; i < 4; i++) {
			debugC(1, kDebugPathfinding, "WWW 5");
			int tmp = _aniHandler.getNumMovements(ani->_id, ani->_statics->_staticsId, _aniData[index]._directions[i]._prevStaticId);

			if (tmp >= 0 && (minidx == -1 || tmp < min)) {
				minidx = i;
				min = tmp;
			}
		}

		return minidx;
	}

	return -1;
}

bool MctlGraph::fillData(AniObject *obj, GmctlAni &item) {
	debugC(4, kDebugPathfinding, "MovGraph::fillData(%d, ...)", obj->_id);

	item._ani = obj;
	item._objectId = obj->_id;

	GameVar *var = g_nmi->getGameLoaderGameVar()->getSubVarByName(obj->_title);
	if (!var)
		return false;

	var = var->getSubVarByName("Test_walk");

	if (!var)
		return false;

	GameVar *varD = nullptr;
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

			item._directions[dir]._walk[act]._movementId = idx;

			Movement *mov = obj->getMovement(idx);

			item._directions[dir]._walk[act]._movement = mov;
			if (mov) {
				point = mov->getMovementSize(0, -1);
				item._directions[dir]._walk[act]._deltaX = point.x;
				item._directions[dir]._walk[act]._deltaY = point.y;
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

			item._directions[dir]._turn[act]._movementId = idx;

			Movement *mov = obj->getMovement(idx);

			item._directions[dir]._turn[act]._movement = mov;
			if (mov) {
				point = mov->getMovementSize(0, -1);
				item._directions[dir]._turn[act]._deltaX = point.x;
				item._directions[dir]._turn[act]._deltaY = point.y;
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

			item._directions[dir]._turnS[act]._movementId = idx;

			Movement *mov = obj->getMovement(idx);

			item._directions[dir]._turnS[act]._movement = mov;
			if (mov) {
				point = mov->getMovementSize(0, -1);
				item._directions[dir]._turnS[act]._deltaX = point.x;
				item._directions[dir]._turnS[act]._deltaY = point.y;
			}
		}

		item._directions[dir]._prevStaticId = item._directions[dir]._walk[0]._movement->_prevStatic->_staticsId;
		item._directions[dir]._nextStaticId = item._directions[dir]._walk[0]._movement->_nextStatic->_staticsId;

	}
	return true;
}

void MctlGraph::attachObject(AniObject *obj) {
	debugC(4, kDebugPathfinding, "MctlGraph::attachObject(*%d)", obj->_id);

	MovGraph::attachObject(obj);

	int id = getObjIndex(obj->_id);

	if (id >= 0) {
		_aniData[id]._ani = obj;
	} else {
		// this is a little dumb due to no move semantics
		_aniData.push_back(GmctlAni());
		if (!fillData(obj, _aniData.back())) {
			_aniData.pop_back();
		}
	}
}

void MctlGraph::generateList(GmctlMqStruct &movinfo, MovGraphLinkList *linkList, LinkOrNode *lnkSrc, LinkOrNode *lnkDst) {
	debugC(4, kDebugPathfinding, "MctlGraph::generateList(...)");

	GmctlPathItem *elem;
	Common::Point point;
	Common::Rect rect;

	int subIndex = movinfo.sourceDir;

	movinfo.items.clear();

	movinfo.items.push_back(GmctlPathItem());
	elem = &movinfo.items.back();
	elem->direction = subIndex;
	elem->x = movinfo.sourcePos.x;
	elem->y = movinfo.sourcePos.y;
	elem->z = -1;

	int prevSubIndex = movinfo.sourceDir;

	for (uint i = 0; i < linkList->size(); i++) {
		int idx1;

		if (linkList->size() <= 1) {
			if (linkList->size() == 1)
				idx1 = getDirBySize((*linkList)[0], movinfo.destPos.x - movinfo.sourcePos.x, movinfo.destPos.y - movinfo.sourcePos.y);
			else
				idx1 = getDirBySize(nullptr, movinfo.destPos.x - movinfo.sourcePos.x, movinfo.destPos.y - movinfo.sourcePos.y);

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

			movinfo.items.push_back(GmctlPathItem());
			elem = &movinfo.items.back();
			elem->direction = subIndex;
			elem->x = rect.left;
			elem->y = rect.top;
			elem->z = -1;
		}

		if (i != linkList->size() - 1) {
			for (;;) {
				i++;
				if (getLinkDir(linkList, i, &rect, nullptr) != prevSubIndex) {
					i--;
					getLinkDir(linkList, i, &rect, &point);

					break;
				}

				if (i == linkList->size() - 1)
					break;
			}
		}

		if (movinfo.items.back().direction != 10) {
			subIndex = prevSubIndex;

			movinfo.items.push_back(GmctlPathItem());
			elem = &movinfo.items.back();
			elem->direction = 10;
			elem->x = -1;
			elem->y = -1;
			elem->z = -1;

			movinfo.items.push_back(GmctlPathItem());
			elem = &movinfo.items.back();
			elem->direction = prevSubIndex;
			if (i == linkList->size() - 1) {
				elem->x = movinfo.destPos.x;
				elem->y = movinfo.destPos.y;
				elem->z = movinfo.destZ;
			} else {
				elem->x = rect.right;
				elem->y = rect.bottom;
				elem->z = point.y;
			}
		}
	}

	if (subIndex != movinfo.destDir) {
		movinfo.items.push_back(GmctlPathItem());
		elem = &movinfo.items.back();
		elem->direction = movinfo.destDir;
		elem->x = movinfo.destPos.x;
		elem->y = movinfo.destPos.y;
		elem->z = movinfo.destZ;
	}
}

MessageQueue *MctlGraph::makeWholeQueue(GmctlMqStruct &mctlMQ) {
	debugC(4, kDebugPathfinding, "MctlGraph::makeWholeQueue(...)");

	GmctlMqStruct movinfo(mctlMQ);

	int curX = mctlMQ.sourcePos.x;
	int curY = mctlMQ.sourcePos.y;
	int curDistance = mctlMQ.sourceZ;

	MessageQueue *mq = new MessageQueue(g_nmi->_globalMessageQueueList->compact());

	int numItems = mctlMQ.items.size();

	for (int i = 0; i < numItems - 1; i++) {
		if (mctlMQ.items[i + 1].direction != 10) {
			MG2I *mg2i;

			if (i >= numItems - 2 || mctlMQ.items[i + 2].direction != 10) {
				movinfo.flags = 0;
				mg2i = &_aniData[mctlMQ.objectIndex]._directions[mctlMQ.items[i].direction]._turnS[mctlMQ.items[i + 1].direction];
			} else {
				movinfo.flags = 2;
				mg2i = &_aniData[mctlMQ.objectIndex]._directions[mctlMQ.items[i].direction]._turn[mctlMQ.items[i + 1].direction];
			}
			if (i < numItems - 2
				|| (mctlMQ.items[i].x == mctlMQ.items[i + 1].x
					&& mctlMQ.items[i].y == mctlMQ.items[i + 1].y)
				 || mctlMQ.items[i].x == -1
				 || mctlMQ.items[i].y == -1
				 || mctlMQ.items[i + 1].x == -1
				 || mctlMQ.items[i + 1].y == -1) {

				Command *ex = new Command(_aniData[mctlMQ.objectIndex]._objectId, 1, mg2i->_movementId, 0, 0, 0, 1, 0, 0, 0);

				ex->_excFlags |= 2;
				ex->_param = _aniData[mctlMQ.objectIndex]._ani->_copyId;
				ex->_wait = 1;
				ex->_z = -1;
				mq->addCommandToEnd(ex);

				curX += mg2i->_deltaX;
				curY += mg2i->_deltaY;
			} else {
				MakeQueueStruct mkQueue;

				mkQueue.reset();

				mkQueue.ani = _aniData[mctlMQ.objectIndex]._ani;
				mkQueue.staticsId2 = mg2i->_movement->_nextStatic->_staticsId;
				mkQueue.x1 = mctlMQ.items[i + 1].x;
				mkQueue.y1 = mctlMQ.items[i + 1].y;
				mkQueue.destZ = mctlMQ.items[i + 1].z;
				mkQueue.staticsId1 = mg2i->_movement->_prevStatic->_staticsId;

				mkQueue.x2 = mctlMQ.items[i].x;
				mkQueue.y2 = mctlMQ.items[i].y;
				mkQueue.cycleCount = 1;
				mkQueue.flags = 0x7f;
				mkQueue.movementId = mg2i->_movementId;

				Common::ScopedPtr<MessageQueue> mq2(_aniHandler.makeRunQueue(&mkQueue));
				mq->mergeQueue(mq2.get());

				curX = mctlMQ.items[i + 1].x;
				curY = mctlMQ.items[i + 1].y;
			}
		} else {
			movinfo.destDir = mctlMQ.items[i].direction;
			movinfo.sourceDir = movinfo.destDir;
			movinfo.sourcePos.y = curY;
			movinfo.sourcePos.x = curX;

			movinfo.sourceZ = curDistance;
			movinfo.destPos.x = mctlMQ.items[i + 2].x;
			movinfo.destPos.y = mctlMQ.items[i + 2].y;
			movinfo.destZ = mctlMQ.items[i + 2].z;

			if (i < numItems - 4
				&& mctlMQ.items[i + 2].direction != 10
				&& mctlMQ.items[i + 3].direction != 10
				&& mctlMQ.items[i + 2].direction != mctlMQ.items[i + 3].direction
				&& mctlMQ.items[i + 4].direction == 10) {

				MG2I *m = &_aniData[mctlMQ.objectIndex]._directions[mctlMQ.items[i + 2].direction]._turn[mctlMQ.items[i + 3].direction];

				if (movinfo.destDir && movinfo.destDir != 1) {
					movinfo.destPos.y -= m->_deltaY;
					movinfo.flags = (movinfo.flags & 2) | 1;
				} else {
					movinfo.destPos.x -= m->_deltaX;
					movinfo.flags = (movinfo.flags & 2) | 1;
				}

			} else if (i < numItems - 3
				&& mctlMQ.items[i + 2].direction != 10
				&& mctlMQ.items[i + 3].direction != 10
				&& mctlMQ.items[i + 2].direction != mctlMQ.items[i + 3].direction) {

				MG2I *m = &_aniData[mctlMQ.objectIndex]._directions[mctlMQ.items[i + 2].direction]._turnS[mctlMQ.items[i + 3].direction];
				movinfo.destPos.x -= m->_deltaX;
				movinfo.destPos.y -= m->_deltaY;
				movinfo.flags = (movinfo.flags & 2) | (mctlMQ.flags & 1);

			} else {
				movinfo.flags = (movinfo.flags & 2) | (mctlMQ.flags & 1);
			}

			i++; // intentional

			MessageQueue *mq2 = makeLineQueue(&movinfo);

			if (!mq2) {
				delete mq;
				return nullptr;
			}

			mq->mergeQueue(mq2);

			delete mq2;

			curX = movinfo.destPos.x;
			curY = movinfo.destPos.y;
			curDistance = movinfo.destZ;
		}
	}

	mctlMQ.destPos.x = movinfo.destPos.x;
	mctlMQ.destPos.y = movinfo.destPos.y;

	return mq;
}

int MctlGraph::detachObject(AniObject *obj) {
	warning("STUB: MctlGraph::detachObject()");

	return 0;
}

void MctlGraph::detachAllObjects() {
	debugC(4, kDebugPathfinding, "MctlGraph::detachAllObjects()");
	_aniData.clear();
}

MessageQueue *MctlGraph::startMove(AniObject *ani, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	debugC(4, kDebugPathfinding, "MctlGraph::startMove(*%d, %d, %d, %d, %d)", ani->_id, xpos, ypos, fuzzyMatch, staticsId);

	if (!ani->isIdle())
		return nullptr;

	if (ani->_flags & 0x100)
		return nullptr;

	debugC(1, kDebugPathfinding, "WWW 3");
	MessageQueue *mq = makeQueue(ani, xpos, ypos, fuzzyMatch, staticsId);

	if (!mq)
		return nullptr;

	if (ani->_movement) {
		if (mq->getCount() <= 1 || mq->getCommandByIndex(0)->_cmd != 22) {
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
			ani->_movement = nullptr;
		}
	}

	if (!mq->run(ani)) {
		delete mq;

		return nullptr;
	}

	return mq;
}

MessageQueue *MctlGraph::makeQueue(AniObject *obj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	LinkOrNode linkInfoDest;
	LinkOrNode linkInfoSource;
	GmctlMqStruct mctlMQ1;
	PicAniInfo picAniInfo;
	Common::Point point;

	debugC(1, kDebugPathfinding, "MctlGraph::makeQueue(%d, %d, %d, %d, %d)", obj->_id, xpos, ypos, fuzzyMatch, staticsId);

	int idx = getObjIndex(obj->_id);

	if (idx < 0)
		return nullptr;

	linkInfoSource.link = nullptr;
	linkInfoSource.node = nullptr;

	linkInfoDest.link = nullptr;
	linkInfoDest.node = nullptr;

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
			return nullptr;
	}

	if (obj->_movement) {
		int newx, newy;

		if (subMgm) {
			obj->_messageQueueId = 0;
			obj->setStatic(_aniData[idx]._directions[idxsub]._prevStaticId);
			newx = obj->_x;
			newy = obj->_y;
		} else {
			point = obj->_movement->getMovementSize(0, picAniInfo.curFrame);
			newx = obj->_movement->_x - point.x;
			newy = obj->_movement->_y - point.y;
			if (idxsub != 1 && idxsub) {
				if (idxsub == 2 || idxsub == 3) {
					newy = obj->_movement->_y;
				}
			} else {
				newx = obj->_movement->_x;
			}
		}

		obj->_movement = nullptr;
		obj->setPos(newx, newy);
	}

	if (obj->_x == xpos && obj->_y == ypos) {
		MessageQueue *mq = new MessageQueue(g_nmi->_globalMessageQueueList->compact());

		if (staticsId && obj->_statics->_staticsId != staticsId) {
			int idxwalk = getDirByStatics(idx, staticsId);
			if (idxwalk == -1) {
				obj->setPicAniInfo(picAniInfo);

				delete mq;

				return nullptr;
			}

			Command *ex = new Command(picAniInfo.objectId, 1, _aniData[idx]._directions[idxsub]._turnS[idxwalk]._movementId, 0, 0, 0, 1, 0, 0, 0);

			ex->_wait = 1;
			ex->_param = picAniInfo.copyId;
			ex->_excFlags |= 2;

			mq->addCommandToEnd(ex);
		} else {
			Command *ex = new Command(picAniInfo.objectId, 22, obj->_statics->_staticsId, 0, 0, 0, 1, 0, 0, 0);

			ex->_param = picAniInfo.copyId;
			ex->_excFlags |= 3;
			mq->addCommandToEnd(ex);

			ex = new Command(picAniInfo.objectId, 5, -1, obj->_x, obj->_y, 0, 1, 0, 0, 0);

			ex->_z = -1;
			ex->_param = picAniInfo.copyId;
			ex->_excFlags |= 3;
			mq->addCommandToEnd(ex);
		}

		obj->setPicAniInfo(picAniInfo);

		return mq;
	}

	linkInfoSource.node = getHitNode(obj->_x, obj->_y, 0);

	if (!linkInfoSource.node) {
		linkInfoSource.link = getHitLink(obj->_x, obj->_y, idxsub, 0);

		if (!linkInfoSource.link) {
			linkInfoSource.link = getNearestLink(obj->_x, obj->_y);

			if (!linkInfoSource.link) {
				obj->setPicAniInfo(picAniInfo);

				return nullptr;
			}
		}
	}

	linkInfoDest.node = getHitNode(xpos, ypos, fuzzyMatch);

	if (!linkInfoDest.node) {
		linkInfoDest.link = getHitLink(xpos, ypos, idxsub, fuzzyMatch);

		if (!linkInfoDest.link) {
			obj->setPicAniInfo(picAniInfo);

			return nullptr;
		}
	}

	MovGraphLinkList tempLinkList;
	double minPath = iterate(&linkInfoSource, &linkInfoDest, &tempLinkList);

	debugC(0, kDebugPathfinding, "MctlGraph::makeQueue(): path: %g  parts: %d", minPath, tempLinkList.size());

	if (minPath < 0.0 || ((linkInfoSource.node != linkInfoDest.node || !linkInfoSource.node) && !tempLinkList.size()))
		return nullptr;

	mctlMQ1.clear();

	mctlMQ1.sourceDir = idxsub;
	mctlMQ1.sourcePos.x = obj->_x;
	mctlMQ1.sourcePos.y = obj->_y;

	int dx1 = obj->_x;
	int dy1 = obj->_y;
	int dx2, dy2;

	if (linkInfoSource.node)
		mctlMQ1.sourceZ = linkInfoSource.node->_z;
	else
		mctlMQ1.sourceZ = linkInfoSource.link->_graphSrc->_z;

	if (linkInfoDest.node) {
		dx2 = linkInfoDest.node->_x;
		dy2 = linkInfoDest.node->_y;

		mctlMQ1.destPos.x = linkInfoDest.node->_x;
		mctlMQ1.destPos.y = linkInfoDest.node->_y;

		mctlMQ1.destZ = linkInfoDest.node->_z;
	} else {
		mctlMQ1.destPos.x = xpos;
		mctlMQ1.destPos.y = ypos;

		MovGraphNode *nod = linkInfoDest.link->_graphSrc;
		double dst1 = sqrt((double)((ypos - nod->_y) * (ypos - nod->_y) + (xpos - nod->_x) * (xpos - nod->_x)));
		int dst = linkInfoDest.link->_graphDst->_z - nod->_z;

		mctlMQ1.destZ = nod->_z + (int)(dst1 * (double)dst / linkInfoDest.link->_length);

		putToLink(&mctlMQ1.destPos, linkInfoDest.link, 1);

		dx1 = mctlMQ1.sourcePos.x;
		dy1 = mctlMQ1.sourcePos.y;
		dx2 = mctlMQ1.destPos.x;
		dy2 = mctlMQ1.destPos.y;
	}

	if (staticsId) {
		mctlMQ1.destDir = getDirByStatics(idx, staticsId);
	} else if (tempLinkList.size() <= 1) {
		if (tempLinkList.size() == 1)
			mctlMQ1.destDir = getDirBySize(tempLinkList[0], dx2 - dx1, dy2 - dy1);
		else
			mctlMQ1.destDir = getDirBySize(nullptr, dx2 - dx1, dy2 - dy1);
	} else {
		mctlMQ1.destDir = getLinkDir(&tempLinkList, tempLinkList.size() - 1, nullptr, nullptr);
	}

	mctlMQ1.flags = fuzzyMatch != 0;

	if (_aniData[idx]._directions[idxsub]._prevStaticId != obj->_statics->_staticsId)
		mctlMQ1.flags |= 2;

	generateList(mctlMQ1, &tempLinkList, &linkInfoSource, &linkInfoDest);

	MessageQueue *mq = makeWholeQueue(mctlMQ1);

	linkInfoDest.node = getHitNode(mctlMQ1.destPos.x, mctlMQ1.destPos.y, fuzzyMatch);

	if (!linkInfoDest.node)
		linkInfoDest.link = getHitLink(mctlMQ1.destPos.x, mctlMQ1.destPos.y, mctlMQ1.destDir, fuzzyMatch);

	if (fuzzyMatch || linkInfoDest.link || linkInfoDest.node) {
		if (mq && mq->getCount() > 0 && picAniInfo.movementId) {
			Command *ex = mq->getCommandByIndex(0);

			if (ex && (ex->_cmd == 1 || ex->_cmd == 20)
					&& picAniInfo.movementId == ex->_messageNum
					&& picAniInfo.targetFrame == ex->_z) {
				mq->removeCommand(0, 1);
			} else {
				ex = new Command(picAniInfo.objectId, 5, ex->_messageNum, obj->_x, obj->_y, 0, 1, 0, 0, 0);
				ex->_z = -1;
				ex->_param = picAniInfo.copyId;
				ex->_excFlags |= 2;
				mq->addCommandToFront(ex);

				ex = new Command(picAniInfo.objectId, 22, _aniData[idx]._directions[idxsub]._prevStaticId, 0, 0, 0, 1, 0, 0, 0);

				ex->_param = picAniInfo.copyId;
				ex->_excFlags |= 3;
				mq->addCommandToFront(ex);
			}
		}
	} else {
		delete mq;
		mq = nullptr;
	}

	obj->setPicAniInfo(picAniInfo);

	return mq;
}

bool MctlGraph::checkPoint(int x, int y) {
	return getHitNode(x, y, 0) || getHitLink(x, y, -1, 0);
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

	return nullptr;
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

MessageQueue *MctlGraph::makeLineQueue(GmctlMqStruct *info) {
	debugC(4, kDebugPathfinding, "MctlGraph::makeLineQueue(...)");

	int mx1 = 0;
	int my1 = 0;

	if (!(info->flags & 2)) {
		mx1 = _aniData[info->objectIndex]._directions[info->sourceDir]._walk[0]._deltaX;
		my1 = _aniData[info->objectIndex]._directions[info->sourceDir]._walk[0]._deltaY;
	}

	int mx2 = 0;
	int my2 = 0;

	if (!(info->flags & 4)) {
		mx2 = _aniData[info->objectIndex]._directions[info->sourceDir]._walk[2]._deltaX;
		my2 = _aniData[info->objectIndex]._directions[info->sourceDir]._walk[2]._deltaY;
	}

	Common::Point point;

	int y = info->destPos.y - info->sourcePos.y - my2 - my1;
	int x = info->destPos.x - info->sourcePos.x - mx2 - mx1;
	int a2 = 0;
	int mgmLen;

	point = _aniHandler.getNumCycles(_aniData[info->objectIndex]._directions[info->sourceDir]._walk[1]._movement, x, y, &mgmLen, &a2, info->flags & 1);

	int x1 = point.x;
	int y1 = point.y;

	if (!(info->flags & 1)) {
		if (info->sourceDir == 1 || info->sourceDir == 0) {
			a2 = -1;
			x1 = mgmLen * _aniData[info->objectIndex]._directions[info->sourceDir]._walk[1]._deltaX;
			x = x1;
			info->destPos.x = x1 + info->sourcePos.x + mx1 + mx2;
		}
	}

	if (!(info->flags & 1)) {
		if (info->sourceDir == 2 || info->sourceDir == 3) {
			a2 = -1;
			y1 = mgmLen * _aniData[info->objectIndex]._directions[info->sourceDir]._walk[1]._deltaY;
			y = y1;
			info->destPos.y = y1 + info->sourcePos.y + my1 + my2;
		}
	}

	int cntX = 0;
	int cntY = 0;

	if (!(info->flags & 2)) {
		cntX = _aniData[info->objectIndex]._directions[info->sourceDir]._walk[0]._movement->countPhasesWithFlag(-1, 1);
		cntY = _aniData[info->objectIndex]._directions[info->sourceDir]._walk[0]._movement->countPhasesWithFlag(-1, 2);
	}

	if (mgmLen > 1) {
		cntX += (mgmLen - 1) * _aniData[info->objectIndex]._directions[info->sourceDir]._walk[1]._movement->countPhasesWithFlag(-1, 1);
		cntY += (mgmLen - 1) * _aniData[info->objectIndex]._directions[info->sourceDir]._walk[1]._movement->countPhasesWithFlag(-1, 2);
	}

	if (mgmLen > 0) {
		cntX += _aniData[info->objectIndex]._directions[info->sourceDir]._walk[1]._movement->countPhasesWithFlag(a2, 1);
		cntY += _aniData[info->objectIndex]._directions[info->sourceDir]._walk[1]._movement->countPhasesWithFlag(a2, 2);
	}

	if (!(info->flags & 4)) {
		cntX += _aniData[info->objectIndex]._directions[info->sourceDir]._walk[2]._movement->countPhasesWithFlag(-1, 1);
		cntY += _aniData[info->objectIndex]._directions[info->sourceDir]._walk[2]._movement->countPhasesWithFlag(-1, 2);
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
	Command *ex;

	if (info->flags & 2) {
		ex = new Command(
							_aniData[info->objectIndex]._objectId,
							5,
							_aniData[info->objectIndex]._directions[info->sourceDir]._walk[1]._movementId,
							info->sourcePos.x,
							info->sourcePos.y,
							0,
							1,
							0,
							0,
							0);

		ex->_z = info->sourceZ;

		ex->_param = _aniData[info->objectIndex]._ani->_copyId;
		ex->_wait = 1;
		ex->_excFlags |= 2;
	} else {
		ex = new Command(
							 _aniData[info->objectIndex]._objectId,
							 5,
							 _aniData[info->objectIndex]._directions[info->sourceDir]._walk[0]._movementId,
							 info->sourcePos.x,
							 info->sourcePos.y,
							 0,
							 1,
							 0,
							 0,
							 0);

		ex->_z = info->sourceZ;

		ex->_param = _aniData[info->objectIndex]._ani->_copyId;
		ex->_wait = 1;
		ex->_excFlags |= 2;
		mq->addCommandToEnd(ex);

		ex = _aniHandler.createCommand(
								  _aniData[info->objectIndex]._directions[info->sourceDir]._walk[0]._movement,
								  _aniData[info->objectIndex]._objectId,
								  x1,
								  y1,
								  x2,
								  y2,
								  -1);
		ex->_parId = mq->_id;
		ex->_param = _aniData[info->objectIndex]._ani->_copyId;
	}

	mq->addCommandToEnd(ex);

	for (int i = 0; i < mgmLen; ++i) {
		int par;

		if (i == mgmLen - 1)
			par = a2;
		else
			par = -1;

		ex = _aniHandler.createCommand(
								  _aniData[info->objectIndex]._directions[info->sourceDir]._walk[1]._movement,
								  _aniData[info->objectIndex]._objectId,
								  x1,
								  y1,
								  x2,
								  y2,
								  par);
		ex->_parId = mq->_id;
		ex->_param = _aniData[info->objectIndex]._ani->_copyId;
		mq->addCommandToEnd(ex);
	}

	if (!(info->flags & 4)) {
		ex = _aniHandler.createCommand(
								  _aniData[info->objectIndex]._directions[info->sourceDir]._walk[2]._movement,
								  _aniData[info->objectIndex]._objectId,
								  x1,
								  y1,
								  x2,
								  y2,
								  -1);
		ex->_parId = mq->_id;
		ex->_param = _aniData[info->objectIndex]._ani->_copyId;

		mq->addCommandToEnd(ex);
	}

	ex = new Command(_aniData[info->objectIndex]._objectId, 5, -1, info->destPos.x, info->destPos.y, 0, 1, 0, 0, 0);
	ex->_z = info->destZ;

	ex->_param = _aniData[info->objectIndex]._ani->_copyId;
	ex->_wait = 0;
	ex->_excFlags |= 2;

	mq->addCommandToEnd(ex);

	return mq;
}

MovGraphLink *MctlGraph::getHitLink(int x, int y, int idx, int fuzzyMatch) {
	debugC(4, kDebugPathfinding, "MctlGraph::getHitLink(...)");

	Common::Point point;
	MovGraphLink *res = nullptr;

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
			if (lnk->_movGraphReact->hitTest(x, y)) {
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
	MovGraphLink *res = nullptr;

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
		return nullptr;
}

double MctlGraph::iterate(LinkOrNode *linkInfoSource, LinkOrNode *linkInfoDest, MovGraphLinkList *listObj) {
	debugC(4, kDebugPathfinding, "MctlGraph::iterate(...)");

	LinkOrNode linkInfoWorkSource;

	if (linkInfoSource->link != linkInfoDest->link || linkInfoSource->node != linkInfoDest->node) {
		double minDistance = -1.0;

		if (linkInfoSource->node) {
			for (LinkList::iterator i = _links.begin(); i != _links.end(); ++i) {
				MovGraphLink *lnk = static_cast<MovGraphLink *>(*i);

				if ((lnk->_graphSrc == linkInfoSource->node || lnk->_graphDst == linkInfoSource->node) && !(lnk->_flags & 0xA0000000)) {
					linkInfoWorkSource.node = nullptr;
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
			linkInfoWorkSource.link = nullptr;

			MovGraphLinkList tmpList;

			double newDistance = iterate(&linkInfoWorkSource, linkInfoDest, &tmpList);

			if (newDistance >= 0.0) {
				listObj->clear();

				listObj->push_back(linkInfoSource->link);
				listObj->push_back(tmpList);

				minDistance = newDistance;
			}

			linkInfoWorkSource.link = nullptr;
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

MovGraphNode *MovGraph::findNearestNode(int ox, int oy) {
	MovGraphNode *res = nullptr;
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
	_graphDst = nullptr;
	_graphSrc = nullptr;
	_field_3C = 0;
	_field_38 = 0;
	_movGraphReact = nullptr;

	_objtype = kObjTypeMovGraphLink;
}

MovGraphLink::~MovGraphLink() {
	delete _movGraphReact;

	_movementIds.clear();
	_staticsIds.clear();
}


bool MovGraphLink::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "MovGraphLink::load()");

	_movementIds.load(file);
	_staticsIds.load(file);

	_flags = file.readUint32LE();

	debugC(8, kDebugLoading, "GraphNode1");
	_graphSrc = file.readClass<MovGraphNode>();
	debugC(8, kDebugLoading, "GraphNode2");
	_graphDst = file.readClass<MovGraphNode>();

	_length = file.readDoubleLE();
	_angle = file.readDoubleLE();

	debugC(8, kDebugLoading, "length: %g, angle: %g", _length, _angle);

	_movGraphReact = file.readClass<MovGraphReact>();
	_title = file.readPascalString();

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

	_flags = file.readUint32LE();
	_x = file.readSint32LE();
	_y = file.readSint32LE();
	_z = file.readSint32LE();

	return true;
}

ReactParallel::ReactParallel(int left, int right) {
	_x1 = 0;
	_x2 = 0;
	_right = right;
	_left = left;
	_y1 = 0;
	_y2 = 0;
}

bool ReactParallel::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "ReactParallel::load()");

	_x1 = file.readSint32LE();
	_y1 = file.readSint32LE();
	_x2 = file.readSint32LE();
	_y2 = file.readSint32LE();
	_left = file.readSint32LE();
	_right = file.readSint32LE();

	create();

	return true;
}

void ReactParallel::create() {
	_points.resize(4);

	const bool android = g_nmi->getPlatform() == Common::kPlatformAndroid;
	double angle = android ? atan2f(_y1 - _y2, _x1 - _x2) : atan2((double)(_y1 - _y2), (double)(_x1 - _x2));
	double at = angle + 1.570796; // pi/2
	double sn = sin(at);
	double cs = cos(at);
	double lx = _left * cs, ly = _left * sn;
	double rx = _right * cs, ry = _right * sn;
	if (android) {
		// The XML engine truncates the offsets before adding them to each endpoint.
		lx = (int)lx;
		ly = (int)ly;
		rx = (int)rx;
		ry = (int)ry;
	}

	_points[0] = Common::Point(_x1 - lx, _y1 - ly);
	_points[1] = Common::Point(_x2 - lx, _y2 - ly);
	_points[2] = Common::Point(_x2 + rx, _y2 + ry);
	_points[3] = Common::Point(_x1 + rx, _y1 + ry);

	// GdiObject::Attach(_rgn, CreatePolygonRgn(_points, 4, 2);
}

void ReactParallel::setAnchorPoints(int x1, int y1, int x2, int y2) {
	_x1 = x1;
	_y1 = y1;
	_x2 = x2;
	_y2 = y2;
}

ReactPolygonal::ReactPolygonal(int centerX, int centerY) {
	// hack for using isValid to avoid creating another state variable for
	// getBBox
	_bbox.right = -1;

	_centerX = centerX;
	_centerY = centerY;
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

	create();

	return true;
}

void ReactPolygonal::create() {
	if (_points.size()) {

		// GdiObject::Attach(_rgn, CreatePolygonRgn(_points, _pointCount, 2);
	}
}

void ReactPolygonal::setAnchorPoints(int x1, int y1, int x2, int y2) {
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


bool MovGraphReact::hitTest(int x, int y) {
	if (g_nmi->getPlatform() == Common::kPlatformAndroid) {
		if (_points.size() <= 1)
			return !_points.empty() && _points[0] == Common::Point(x, y);
		bool inside = false;
		Common::Point prev = _points.back();
		for (const Common::Point &cur : _points) {
			if ((cur.y < y) != (prev.y < y)) {
				const int64 cross = (int64)(prev.x - x) * (cur.y - prev.y) - (int64)(prev.y - y) * (cur.x - prev.x);
				if (cur.y < y ? cross > 0 : cross < 0)
					inside = !inside;
			}
			prev = cur;
		}
		return inside;
	}
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
		return (mc->startMove(g_nmi->_currentScene->findAniObjectById(objId, objKey), x, y, fuzzyMatch, 0) != nullptr);

	return 0;
}

bool doSomeAnimation(int objId, int objKey, int a3) {
	AniObject *ani = g_nmi->_currentScene->findAniObjectById(objId, objKey);
	MctlCompound *cmp = getCurrSceneSc2MotionController();

	if (ani && cmp)
		return cmp->resetPosition(ani, a3);

	return false;
}

bool doSomeAnimation2(int objId, int objKey) {
	return doSomeAnimation(objId, objKey, 0);
}

} // End of namespace NGI
