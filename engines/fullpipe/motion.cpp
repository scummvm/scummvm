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

#include "common/file.h"
#include "common/array.h"
#include "common/list.h"

#include "fullpipe/objects.h"
#include "fullpipe/statics.h"
#include "fullpipe/gameloader.h"
#include "fullpipe/motion.h"
#include "fullpipe/messages.h"

namespace Fullpipe {

bool MotionController::load(MfcArchive &file) {
	// Is originally empty	file.readClass();

	debug(5, "MotionController::load()");

	return true;
}

void MotionController::enableLinks(const char *linkName, bool enable) {
	warning("STUB: MotionController::enableLinks()");
}

MovGraphLink *MotionController::getLinkByName(const char *name) {
	if (_objtype == kObjTypeMctlCompound) {
		MctlCompound *obj = (MctlCompound *)this;

		for (uint i = 0;  i < obj->getMotionControllerCount(); i++) {
			MotionController *con = obj->getMotionController(i);

			if (con->_objtype == kObjTypeMovGraph) {
				MovGraph *gr = (MovGraph *)con;

				for (ObList::iterator l = gr->_links.begin(); l != gr->_links.end(); ++l) {
					assert(((CObject *)*l)->_objtype == kObjTypeMovGraphLink);

					MovGraphLink *lnk = (MovGraphLink *)*l;

					if (!strcmp(lnk->_name, name))
						return lnk;
				}
			}
		}
	}

	if (_objtype == kObjTypeMovGraph) {
		MovGraph *gr = (MovGraph *)this;

		for (ObList::iterator l = gr->_links.begin(); l != gr->_links.end(); ++l) {
			assert(((CObject *)*l)->_objtype == kObjTypeMovGraphLink);

			MovGraphLink *lnk = (MovGraphLink *)*l;

			if (!strcmp(lnk->_name, name))
				return lnk;
		}
	}

	return 0;
}

bool MctlCompound::load(MfcArchive &file) {
	debug(5, "MctlCompound::load()");

	int count = file.readUint32LE();

	debug(6, "MctlCompound::count = %d", count);

	for (int i = 0; i < count; i++) {
		debug(6, "CompoundArray[%d]", i);
		MctlCompoundArrayItem *obj = new MctlCompoundArrayItem();

		obj->_motionControllerObj = (MotionController *)file.readClass();

		int count1 = file.readUint32LE();

		debug(6, "ConnectionPoint::count: %d", count1);
		for (int j = 0; j < count1; j++) {
			debug(6, "ConnectionPoint[%d]", j);
			MctlConnectionPoint *obj1 = (MctlConnectionPoint *)file.readClass();

			obj->_connectionPoints.push_back(obj1);
		}

		obj->_field_20 = file.readUint32LE();
		obj->_field_24 = file.readUint32LE();

		debug(6, "graphReact");
		obj->_movGraphReactObj = (MovGraphReact *)file.readClass();

		_motionControllers.push_back(obj);
	}

	return true;
}

void MctlCompound::addObject(StaticANIObject *obj) {
	for (uint i = 0; i < _motionControllers.size(); i++)
		_motionControllers[i]->_motionControllerObj->addObject(obj);
}

int MctlCompound::removeObject(StaticANIObject *obj) {
	for (uint i = 0; i < _motionControllers.size(); i++)
		_motionControllers[i]->_motionControllerObj->removeObject(obj);

	return 1;
}

void MctlCompound::initMovGraph2() {
	if (_objtype != kObjTypeMctlCompound)
		return;

	for (uint i = 0; i < _motionControllers.size(); i++) {
		if (_motionControllers[i]->_motionControllerObj->_objtype != kObjTypeMovGraph)
			continue;

		MovGraph *gr = (MovGraph *)_motionControllers[i]->_motionControllerObj;

		MovGraph2 *newgr = new MovGraph2();

		newgr->_links = gr->_links;
		newgr->_nodes = gr->_nodes;

		gr->_links.clear();
		gr->_nodes.clear();

		delete gr;

		_motionControllers[i]->_motionControllerObj = newgr;
	}
}

void MctlCompound::freeItems() {
	for (uint i = 0; i < _motionControllers.size(); i++)
		_motionControllers[i]->_motionControllerObj->freeItems();
}

MessageQueue *MctlCompound::method34(StaticANIObject *ani, int sourceX, int sourceY, int fuzzyMatch, int staticsId) {
	int idx = -1;
	int sourceIdx = -1;

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

	if (idx == sourceIdx)
		return _motionControllers[idx]->_motionControllerObj->method34(ani, sourceX, sourceY, fuzzyMatch, staticsId);

	double dist;
	MctlConnectionPoint *cp = findClosestConnectionPoint(ani->_ox, ani->_oy, idx, sourceX, sourceY, sourceIdx, &dist);

	if (!cp)
		return 0;

	MessageQueue *mq = _motionControllers[idx]->_motionControllerObj->doWalkTo(ani, cp->_connectionX, cp->_connectionY, 1, cp->_field_14);

	if (!mq)
		return 0;

	for (uint i = 0; i < cp->_messageQueueObj->getCount(); i++) {
		ExCommand *ex = new ExCommand(cp->_messageQueueObj->getExCommandByIndex(i));

		ex->_excFlags |= 2;

		mq->addExCommandToEnd(ex);
	}

	ExCommand *ex = new ExCommand(ani->_id, 51, 0, sourceX, sourceY, 0, 1, 0, 0, 0);

	ex->_excFlags |= 2;
	ex->_field_20 = fuzzyMatch;
	ex->_keyCode = ani->_okeyCode;

	mq->addExCommandToEnd(ex);

	if (!mq->chain(ani)) {
		delete mq;
		return 0;
	}

	return mq;
}

MessageQueue *MctlCompound::doWalkTo(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	int match1 = -1;
	int match2 = -1;

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
		return _motionControllers[match1]->_motionControllerObj->doWalkTo(subj, xpos, ypos, fuzzyMatch, staticsId);

	double dist;
	MctlConnectionPoint *closestP = findClosestConnectionPoint(subj->_ox, subj->_oy, match1, xpos, ypos, match2, &dist);

	if (!closestP)
		return 0;

	MessageQueue *mq = _motionControllers[match1]->_motionControllerObj->doWalkTo(subj, closestP->_connectionX, closestP->_connectionY, 1, closestP->_field_14);

	ExCommand *ex;

	if (mq) {
		for (uint i = 0; i < closestP->_messageQueueObj->getCount(); i++) {
			ex = closestP->_messageQueueObj->getExCommandByIndex(i)->createClone();
			ex->_excFlags |= 2;
			mq->addExCommandToEnd(ex);
		}

		ex = new ExCommand(subj->_id, 51, 0, xpos, ypos, 0, 1, 0, 0, 0);

		ex->_field_20 = fuzzyMatch;
		ex->_keyCode = subj->_okeyCode;
		ex->_excFlags |= 2;

		mq->addExCommandToEnd(ex);
	}

	return mq;
}

MctlCompoundArrayItem::~MctlCompoundArrayItem() {
	delete _movGraphReactObj;
	delete _motionControllerObj;
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
	freeItems();
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

void MctlLadder::addObject(StaticANIObject *obj) {
	if (findObjectPos(obj) < 0) {
		MctlLadderMovement *movement = new MctlLadderMovement;

		if (initMovement(obj, movement)) {
			_mgm.addItem(obj->_id);
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
	GameVar *v = g_fp->getGameLoaderGameVar()->getSubVarByName(ani->getName());

	if (!v)
		return false;

	v = v->getSubVarByName("Test_Ladder");

	if (!v)
		return false;

	movement->staticIdsSize = 6;
	movement->movVars = new MctlLadderMovementVars;
	movement->staticIds = new int[movement->staticIdsSize];

	v = v->getSubVarByName("Up");

	if (!v)
		return false;

	movement->movVars->varUpStart = v->getSubVarAsInt("Start");
	movement->movVars->varUpGo = v->getSubVarAsInt("Go");
	movement->movVars->varUpStop = v->getSubVarAsInt("Stop");

	movement->staticIds[0] = ani->getMovementById(movement->movVars->varUpStart)->_staticsObj1->_staticsId;
	movement->staticIds[2] = ani->getMovementById(movement->movVars->varUpGo)->_staticsObj1->_staticsId;

	v = v->getSubVarByName("Down");

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

void MctlLadder::freeItems() {
	_mgm.clear();

	for (uint i = 0; i < _ladmovements.size(); i++) {
		delete _ladmovements[i]->movVars;
		delete[] _ladmovements[i]->staticIds;
	}

	_ladmovements.clear();
}

MessageQueue *MctlLadder::method34(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	MessageQueue *mq = doWalkTo(subj, xpos, ypos, fuzzyMatch, staticsId);

	if (mq) {
		if (mq->chain(subj))
			return mq;
	}

	return 0;
}

MessageQueue *MctlLadder::doWalkTo(StaticANIObject *ani, int xpos, int ypos, int fuzzyMatch, int staticsId) {
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

	MGMInfo mgminfo;
	PicAniInfo picinfo;
	MessageQueue *mq;
	ExCommand *ex;
	Common::Point point;

	if (ani->_movement) {
		ani->getPicAniInfo(&picinfo);

		int ox = ani->_ox;
		int oy = ani->_oy;

		ani->_movement->calcSomeXY(point, 1, ani->_someDynamicPhaseIndex);
		ani->_statics = ani->_movement->_staticsObj2;
		ani->_movement = 0;
		ani->setOXY(point.x + ox, point.y + oy);

		mq = doWalkTo(ani, normx, normy, fuzzyMatch, staticsId);

		ani->setPicAniInfo(&picinfo);

		return mq;
	}

	if (ani->_statics->_staticsId == _ladmovements[pos]->staticIds[0]) {
		mgminfo.ani = ani;

		if (staticsId)
			mgminfo.staticsId2 = staticsId;
		else
			mgminfo.staticsId2 = _ladmovements[pos]->staticIds[direction];

		mgminfo.x1 = normx;
		mgminfo.y1 = normy;
		mgminfo.field_1C = _ladder_field_14;
		mgminfo.flags = 14;
		mgminfo.movementId = direction ? _ladmovements[pos]->movVars->varDownGo : _ladmovements[pos]->movVars->varUpGo;

		return _mgm.genMovement(&mgminfo);
	}

	if (ani->_statics->_staticsId == _ladmovements[pos]->staticIds[2]) {
		if (!direction) {
			mgminfo.ani = ani;

			if (staticsId)
				mgminfo.staticsId2 = staticsId;
			else
				mgminfo.staticsId2 = _ladmovements[pos]->staticIds[0];

			mgminfo.x1 = normx;
			mgminfo.y1 = normy;
			mgminfo.field_1C = _ladder_field_14;
			mgminfo.flags = 14;
			mgminfo.movementId = _ladmovements[pos]->movVars->varUpGo;

			return _mgm.genMovement(&mgminfo);
		}

		int ox = ani->_ox;
		int oy = ani->_oy;

		ani->getMovementById(_ladmovements[pos]->movVars->varUpStop)->calcSomeXY(point, 0, -1);

		mgminfo.ani = ani;

		if (staticsId)
			mgminfo.staticsId2 = staticsId;
		else
			mgminfo.staticsId2 = _ladmovements[pos]->staticIds[1];

		mgminfo.field_1C = _ladder_field_14;
		mgminfo.x1 = normx;
		mgminfo.y1 = normy;
		mgminfo.y2 = point.y + oy;
		mgminfo.x2 = point.x + ox;
		mgminfo.flags = 63;
		mgminfo.staticsId1 = _ladmovements[pos]->staticIds[0];
		mgminfo.movementId = _ladmovements[pos]->movVars->varDownGo;

		mq = _mgm.genMovement(&mgminfo);

		ex = new ExCommand(ani->_id, 1, _ladmovements[pos]->movVars->varUpStop, 0, 0, 0, 1, 0, 0, 0);
		ex->_keyCode = ani->_okeyCode;
		ex->_excFlags |= 2;

		mq->insertExCommandAt(0, ex);

		return mq;
	}

	if (ani->_statics->_staticsId != _ladmovements[pos]->staticIds[3]) {
		mq = _mgm.genMQ(ani, _ladmovements[pos]->staticIds[0], 0, 0, 0);

		if (!mq)
			return 0;

		int nx = ani->_ox;
		int ny = ani->_oy;

		_mgm.getPoint(&point, ani->_id, ani->_statics->_staticsId, _ladmovements[pos]->staticIds[0]);

		nx += point.x;
		ny += point.y;

		ani->getPicAniInfo(&picinfo);

		ani->_statics = ani->getStaticsById(_ladmovements[pos]->staticIds[0]);
		ani->_movement = 0;
		ani->setOXY(nx, ny);

		MessageQueue *newmq = doWalkTo(ani, normx, normy, fuzzyMatch, staticsId);

		mq->transferExCommands(newmq);

		delete newmq;

		ani->setPicAniInfo(&picinfo);

		return mq;
	}

	if (!direction) {
		int nx = ani->_ox;
		int ny = ani->_oy;

		ani->getMovementById(_ladmovements[pos]->movVars->varDownStop)->calcSomeXY(point, 0, -1);

		nx += point.x;
		ny += point.y;

		mgminfo.ani = ani;
		if (staticsId)
			mgminfo.staticsId2 = staticsId;
		else
			mgminfo.staticsId2 = _ladmovements[pos]->staticIds[0];

		mgminfo.field_1C = _ladder_field_14;
		mgminfo.x1 = normx;
		mgminfo.y1 = normy;
		mgminfo.y2 = ny;
		mgminfo.x2 = nx;
		mgminfo.flags = 63;
		mgminfo.staticsId1 = _ladmovements[pos]->staticIds[1];
		mgminfo.movementId = _ladmovements[pos]->movVars->varUpGo;

		mq = _mgm.genMovement(&mgminfo);

		ex = new ExCommand(ani->_id, 1, _ladmovements[pos]->movVars->varDownStop, 0, 0, 0, 1, 0, 0, 0);
		ex->_keyCode = ani->_okeyCode;
		ex->_excFlags |= 2;

		mq->insertExCommandAt(0, ex);

		return mq;
	}


	mgminfo.ani = ani;

	if (staticsId)
		mgminfo.staticsId2 = staticsId;
	else
		mgminfo.staticsId2 = _ladmovements[pos]->staticIds[1];

	mgminfo.x1 = normx;
	mgminfo.y1 = normy;
	mgminfo.field_1C = _ladder_field_14;
	mgminfo.flags = 14;
	mgminfo.movementId = _ladmovements[pos]->movVars->varDownGo;

   return _mgm.genMovement(&mgminfo);
}

MessageQueue *MctlLadder::controllerWalkTo(StaticANIObject *ani, int off) {
	return doWalkTo(ani, _ladderX + off * _width, _ladderY + off * _height, 1, 0);
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
			MovGraph *gr = (MovGraph *)_motionControllers[i]->_motionControllerObj;

			for (ObList::iterator n = gr->_nodes.begin(); n != gr->_nodes.end(); ++n) {
				MovGraphNode *node = (MovGraphNode *)*n;

				if (node->_x == from)
					node->_x = to;
			}

			gr->calcNodeDistancesAndAngles();
		}
	}
}

MctlConnectionPoint::MctlConnectionPoint() {
	_connectionX = 0;
	_connectionY = 0;
	_field_C = 0;
	_field_10 = 0;
	_field_14 = 0;
	_field_16 = 0;
	_messageQueueObj = 0;
	_motionControllerObj = 0;
}

MctlConnectionPoint::~MctlConnectionPoint() {
	delete _messageQueueObj;
}

MovInfo1::MovInfo1(MovInfo1 *src) {
	index = src->index;
	pt1 = src->pt1;
	pt2 = src->pt2;
	distance1 = src->distance1;
	distance2 = src->distance2;
	subIndex = src->subIndex;
	item1Index = src->item1Index;
	items = src->items;
	itemsCount = src->itemsCount;
	flags = src->flags;
}

void MovInfo1::clear() {
	index = 0;
	pt1.x = pt1.y = 0;
	pt2.x = pt2.y = 0;
	distance1 = 0;
	distance2 = 0;
	subIndex = 0;
	item1Index = 0;
	items.clear();
	itemsCount = 0;
	flags = 0;
}

bool MctlCompoundArray::load(MfcArchive &file) {
	debug(5, "MctlCompoundArray::load()");

	int count = file.readUint32LE();

	debug(0, "MctlCompoundArray::count = %d", count);

	assert(0);

	return true;
}

MovGraphItem::MovGraphItem() {
	ani = 0;
	field_4 = 0;
	movitems = 0;
	count = 0;
	field_30 = 0;
	field_34 = 0;
	field_38 = 0;
	field_3C = 0;
}

void MovGraphItem::free() {
	for (uint i = 0; i < movitems->size(); i++) {
		(*movitems)[i]->movarr->_movSteps.clear();
		delete (*movitems)[i]->movarr;
	}

	delete movitems;

	movitems = 0;
}

int MovGraph_messageHandler(ExCommand *cmd);

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
	insertMessageHandler(MovGraph_messageHandler, getMessageHandlersCount() - 1, 129);

	_objtype = kObjTypeMovGraph;
}

MovGraph::~MovGraph() {
	warning("STUB: MovGraph::~MovGraph()");
}

bool MovGraph::load(MfcArchive &file) {
	debug(5, "MovGraph::load()");

	_links.load(file);
	_nodes.load(file);

	return true;
}

void MovGraph::addObject(StaticANIObject *obj) {
	_mgm.clear();
	_mgm.addItem(obj->_id);

	for (uint i = 0; i < _items.size(); i++)
		if (_items[i]->ani == obj)
			return;

	MovGraphItem *item = new MovGraphItem();

	item->ani = obj;

	_items.push_back(item);

	_mgm.addItem(obj->_id); // FIXME: Is it really needed?
}

int MovGraph::removeObject(StaticANIObject *obj) {
	warning("STUB: MovGraph::removeObject()");

	return 0;
}

void MovGraph::freeItems() {
	for (uint i = 0; i < _items.size(); i++) {
		_items[i]->free();

		_items[i]->movarr._movSteps.clear();
	}

	_items.clear();
}

Common::Array<MovItem *> *MovGraph::method28(StaticANIObject *ani, int x, int y, int flag1, int *rescount) {
	*rescount = 0;

	if (_items.size() <= 0)
		return 0;

	uint idx = 0;

	while (_items[idx]->ani != ani) {
		idx++;

		if (idx >= _items.size())
			return 0;
	}
	_items[idx]->free();

	calcNodeDistancesAndAngles();

	_items[idx]->movarr._movSteps.clear();

	Common::Point point;

	point.x = ani->_ox;
	point.y = ani->_oy;

	if (!calcChunk(idx, ani->_ox, ani->_oy, &_items[idx]->movarr, 0))
		findClosestLink(idx, &point, &_items[idx]->movarr);

	_items[idx]->count = 0;

	delete _items[idx]->movitems;
	_items[idx]->movitems = 0;

	int arrSize;
	Common::Array<MovArr *> *movarr = genMovArr(x, y, &arrSize, flag1, 0);

	if (movarr) {
		for (int i = 0; i < arrSize; i++) {
			int sz;
			Common::Array<MovItem *> *movitems = calcMovItems(&_items[idx]->movarr, (*movarr)[i], &sz);

			if (sz > 0) {
				for (int j = 0; j < sz; j++)
					_items[idx]->movitems->push_back(movitems[j]);

				delete movitems;
			}
		}

		delete movarr;
	}

	if (_items[idx]->count) {
		*rescount = _items[idx]->count;

		return _items[idx]->movitems;
	}

	return 0;
}

bool MovGraph::method2C(StaticANIObject *obj, int x, int y) {
	obj->setOXY(x, y);
	return method3C(obj, 1);
}

MessageQueue *MovGraph::method34(StaticANIObject *ani, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	if (!ani) {
		if (!_items.size())
			return 0;

		ani = _items[0]->ani;
	}

	if (ABS(ani->_ox - xpos) < 50 && ABS(ani->_oy - ypos) < 50)
		return 0;

	if (!ani->isIdle())
		return 0;

	if (ani->_flags & 0x100)
		return 0;

	int count;
	Common::Array<MovItem *> *movitems = method28(ani, xpos, ypos, fuzzyMatch, &count);

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
			(ex->_field_14 >= 1 && ex->_field_14 <= ani->_movement->_currDynamicPhaseIndex)) {
			mq = new MessageQueue(g_fp->_globalMessageQueueList->compact());

			ex = new ExCommand(ani->_id, 21, 0, 0, 0, 0, 1, 0, 0, 0);
			ex->_keyCode = ani->_okeyCode;
			ex->_field_3C = 1;
			ex->_field_24 = 0;
			mq->addExCommandToEnd(ex);

			ex = new ExCommand(ani->_id, 51, 0, xpos, ypos, 0, 1, 0, 0, 0);
			ex->_keyCode = ani->_okeyCode;
			ex->_field_3C = 1;
			ex->_field_24 = 0;
			ex->_field_20 = fuzzyMatch;
			mq->addExCommandToEnd(ex);

			if (mq->chain(0))
				return mq;

			delete mq;

			return 0;
		}

		int count2;

		ani->setSomeDynamicPhaseIndex(ex->_field_14);
		method28(ani, xpos, ypos, fuzzyMatch, &count2);

		int idx = getItemIndexByStaticAni(ani);
		count = _items[idx]->count;
		movitems = _items[idx]->movitems;
	}

	return method50(ani, _callback1(ani, movitems, count), staticsId);
}

void MovGraph::changeCallback(MovArr *(*callback1)(StaticANIObject *ani, Common::Array<MovItem *> *items, signed int counter)) {
	_callback1 = callback1;
}

bool MovGraph::method3C(StaticANIObject *ani, int flag) {
	int idx = getItemIndexByStaticAni(ani);

	if (idx == -1)
		return false;

	Common::Point point;
	MovArr movarr;

	point.x = ani->_ox;
	point.y = ani->_oy;

	findClosestLink(idx, &point, &movarr);
	ani->setOXY(point.x, point.y);

	if (flag) {
		Statics *st;

		if (ani->_statics) {
			int t = _mgm.refreshOffsets(ani->_id, ani->_statics->_staticsId, movarr._link->_dwordArray2[_field_44]);
			if (t > _mgm.refreshOffsets(ani->_id, ani->_statics->_staticsId, movarr._link->_dwordArray2[_field_44 + 1]))
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

bool MovGraph::method44(StaticANIObject *ani, int x, int y) {
	int idx = getItemIndexByStaticAni(ani);
	MovArr m;

	if (idx != -1) {
		if (x != -1 || y != -1) {
			int counter;

			Common::Array<MovItem *> *movitem = method28(ani, x, y, 0, &counter);

			if (movitem) {
				MovArr *movarr = _callback1(ani, movitem, counter);
				int cnt = movarr->_movStepCount;

				if (cnt > 0) {
					if (movarr->_movSteps[cnt - 1]->link->_flags & 0x4000000)
						return true;
				}
			}
		} else if (calcChunk(idx, ani->_ox, ani->_oy, &m, 0) && m._link && (m._link->_flags & 0x4000000)) {
			return true;
		}
	}

	return false;
}

MessageQueue *MovGraph::doWalkTo(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	PicAniInfo picAniInfo;
	int ss;

	Common::Array<MovItem *> *movitem = method28(subj, xpos, ypos, fuzzyMatch, &ss);

	subj->getPicAniInfo(&picAniInfo);

	if (movitem) {
		MovArr *goal = _callback1(subj, movitem, ss);
		int idx = getItemIndexByStaticAni(subj);

		for (int i = 0; i < _items[idx]->count; i++) {
			if ((*_items[idx]->movitems)[i]->movarr == goal) {
				if (subj->_movement) {
					Common::Point point;

					subj->calcStepLen(&point);

					MessageQueue *mq = sub1(subj, subj->_ox - point.x, subj->_oy - point.y, subj->_movement->_staticsObj1->_staticsId, xpos, ypos, 0, fuzzyMatch);

					if (!mq || !mq->getExCommandByIndex(0))
						return 0;
					
					ExCommand *ex = mq->getExCommandByIndex(0);

					if ((ex->_messageKind != 1 && ex->_messageKind != 20) || 
						ex->_messageNum != subj->_movement->_id || 
						(ex->_field_14 >= 1 && ex->_field_14 <= subj->_movement->_currDynamicPhaseIndex))
						subj->playIdle();
				}
			}
		}
	}

	movitem = method28(subj, xpos, ypos, fuzzyMatch, &ss);
	if (movitem) {
		MovArr *goal = _callback1(subj, movitem, ss);
		int idx = getItemIndexByStaticAni(subj);

		if (_items[idx]->count > 0) {
			int arridx = 0;

			while ((*_items[idx]->movitems)[arridx]->movarr != goal) {
				arridx++;

				if (arridx >= _items[idx]->count) {
					subj->setPicAniInfo(&picAniInfo);
					return 0;
				}
			}

			_items[idx]->movarr._movSteps.clear();
			_items[idx]->movarr = *(*_items[idx]->movitems)[arridx]->movarr;
			_items[idx]->movarr._movSteps = (*_items[idx]->movitems)[arridx]->movarr->_movSteps;
			_items[idx]->movarr._afield_8 = -1;
			_items[idx]->movarr._link = 0;

			MessageQueue *mq = fillMGMinfo(_items[idx]->ani, &_items[idx]->movarr, staticsId);
			if (mq) {
				ExCommand *ex = new ExCommand();
				ex->_messageKind = 17;
				ex->_messageNum = 54;
				ex->_parentId = subj->_id;
				ex->_field_3C = 1;
				mq->addExCommandToEnd(ex);
			}
			subj->setPicAniInfo(&picAniInfo);

			return mq;
		}
	}

	subj->setPicAniInfo(&picAniInfo);

	return 0;
}

MessageQueue *MovGraph::sub1(StaticANIObject *ani, int x, int y, int stid, int x1, int y1, int stid2, int flag1) {
	PicAniInfo picinfo;

	ani->getPicAniInfo(&picinfo);

	ani->_statics = ani->getStaticsById(stid);
	ani->_movement = 0;
	ani->setOXY(x, y);

	int rescount;

	Common::Array<MovItem *> *movitems = method28(ani, x1, y1, flag1, &rescount);

	if (!movitems) {
		ani->setPicAniInfo(&picinfo);

		return 0;
	}

	MessageQueue *res = 0;

	MovArr *goal = _callback1(ani, movitems, rescount);
	int idx = getItemIndexByStaticAni(ani);

	MovGraphItem *movgitem = _items[idx];
	int cnt = movgitem->count;

	for (int nidx = 0; nidx < cnt; nidx++) {
		if ((*movgitem->movitems)[nidx]->movarr == goal) {
			movgitem->movarr._movSteps.clear();
			_items[idx]->movarr = *(*movgitem->movitems)[nidx]->movarr;
			_items[idx]->movarr._movSteps = (*movgitem->movitems)[nidx]->movarr->_movSteps;
			_items[idx]->movarr._afield_8 = -1;
			_items[idx]->movarr._link = 0;

			res = fillMGMinfo(_items[idx]->ani, &_items[idx]->movarr, stid2);

			break;
		}
	}

	ani->setPicAniInfo(&picinfo);

	return res;
}

MessageQueue *MovGraph::fillMGMinfo(StaticANIObject *ani, MovArr *movarr, int staticsId) {
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
			nd = st->link->_movGraphNode1->_distance;
		} else {
			if (st->sfield_0) {
				nx = st->link->_movGraphNode1->_x;
				ny = st->link->_movGraphNode1->_y;
				nd = st->link->_movGraphNode1->_distance;
			} else {
				nx = st->link->_movGraphNode2->_x;
				ny = st->link->_movGraphNode2->_y;
				nd = st->link->_movGraphNode2->_distance;
			}
		}

		MGMInfo mgminfo;

		memset(&mgminfo, 0, sizeof(mgminfo));
		mgminfo.ani = ani;
		mgminfo.staticsId2 = id2;
		mgminfo.staticsId1 = id1;
		mgminfo.x1 = nx;
		mgminfo.x2 = ox;
		mgminfo.y2 = oy;
		mgminfo.y1 = ny;
		mgminfo.field_1C = nd;
		mgminfo.movementId = st->link->_dwordArray1[_field_44 + st->sfield_0];

		mgminfo.flags = 0xe;
		if (mq)
			mgminfo.flags |= 0x31;

		MessageQueue *newmq = _mgm.genMovement(&mgminfo);

		if (mq) {
			if (newmq) {
				mq->transferExCommands(newmq);

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
	if (_items.size() == 0)
		return 0;

	uint idx;
	int movidx;
	bool done = false;

	for (idx = 0; idx <= _items.size() && !done; idx++) {
		if (idx == _items.size())
			return 0;

		if (_items[idx]->ani == ani) {
			if (!_items[idx]->movitems)
				return 0;

			if (_items[idx]->count < 1)
				return 0;

			for (movidx = 0; movidx < _items[idx]->count; movidx++) {
				if ((*_items[idx]->movitems)[movidx]->movarr == movarr) {
					done = true;

					break;
				}
			}
		}
	}

	_items[idx]->movarr._movSteps.clear();
	_items[idx]->movarr = *(*_items[idx]->movitems)[movidx]->movarr;
	_items[idx]->movarr._movSteps = (*_items[idx]->movitems)[movidx]->movarr->_movSteps;
	_items[idx]->movarr._afield_8 = -1;
	_items[idx]->movarr._link = 0;

	MessageQueue *mq = fillMGMinfo(_items[idx]->ani, &_items[idx]->movarr, 0);

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

double MovGraph::calcDistance(Common::Point *point, MovGraphLink *link, int fuzzyMatch) {
	int n1x = link->_movGraphNode1->_x;
	int n1y = link->_movGraphNode1->_y;
	int n2x = link->_movGraphNode2->_x;
	int n2y = link->_movGraphNode2->_y;
	double dist1x = (double)(point->x - n1x);
	double dist1y = (double)(n1y - point->y);
	double dist2x = (double)(n2x - n1x);
	double dist2y = (double)(n2y - n1y);
	double dist1 = sqrt(dist1y * dist1y + dist1x * dist1x);
	double dist2 = ((double)(n1y - n2y) * dist1y + dist2x * dist1x) / link->_distance / dist1;
	double distm = dist2 * dist1;
	double res = sqrt(1.0 - dist2 * dist2) * dist1;

	if (dist2 <= 0.0 || distm >= link->_distance) {
		if (fuzzyMatch) {
			if (dist2 > 0.0) {
				if (distm >= link->_distance) {
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
		point->x = (int)(n1x + (dist2x * distm / link->_distance));
		point->y = (int)(n1y + (dist2y * distm / link->_distance));
	}

	return res;
}

void MovGraph::calcNodeDistancesAndAngles() {
	for (ObList::iterator i = _links.begin(); i != _links.end(); ++i) {
		assert(((CObject *)*i)->_objtype == kObjTypeMovGraphLink);

		MovGraphLink *lnk = (MovGraphLink *)*i;

		lnk->_flags &= 0x7FFFFFFF;

		lnk->calcNodeDistanceAndAngle();
	}
}

bool MovGraph::findClosestLink(int unusedArg, Common::Point *p, MovArr *movarr) {
	MovGraphLink *link = 0;
	double mindist = 1.0e20;
	int resx = 0, resy = 0;

	for (ObList::iterator i = _links.begin(); i != _links.end(); ++i) {
		MovGraphLink *lnk = (MovGraphLink *)*i;

		if ((lnk->_flags & 0x10000000) && !(lnk->_flags & 0x20000000) ) {
			double dx1 = lnk->_movGraphNode1->_x - p->x;
			double dy1 = lnk->_movGraphNode1->_y - p->y;
			double dx2 = lnk->_movGraphNode2->_x - p->x;
			double dy2 = lnk->_movGraphNode2->_y - p->y;
			double dx3 = lnk->_movGraphNode2->_x - lnk->_movGraphNode1->_x;
			double dy3 = lnk->_movGraphNode2->_y - lnk->_movGraphNode1->_y;
			double sq1 = sqrt(dy1 * dy1 + dx1 * dx1);
			double sdist = (dy3 * dy1 + dx3 * dx1) / lnk->_distance / sq1;
			double ldist = sdist * sq1;
			double dist = sqrt(1.0 - sdist * sdist) * sq1;

			if (ldist < 0.0) {
				ldist = 0.0;
				dist = sqrt(dx1 * dx1 + dy1 * dy1);
			}

			if (ldist > lnk->_distance) {
				ldist = lnk->_distance;
				dist = sqrt(dx2 * dx2 + dy2 * dy2);
			}

			if (ldist >= 0.0 && ldist <= lnk->_distance && dist < mindist) {
				resx = lnk->_movGraphNode1->_x + (int)(dx3 * ldist / lnk->_distance);
				resy = lnk->_movGraphNode1->_y + (int)(dy3 * ldist / lnk->_distance);

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

int MovGraph::getItemIndexByStaticAni(StaticANIObject *ani) {
	for (uint i = 0; i < _items.size(); i++)
		if (_items[i]->ani == ani)
			return i;

	return -1;
}

Common::Array<MovArr *> *MovGraph::genMovArr(int x, int y, int *arrSize, int flag1, int flag2) {
	if (!_links.size()) {
		*arrSize = 0;

		return 0;
	}

	Common::Array<MovArr *> *arr = new Common::Array<MovArr *>;
	MovArr *movarr;

	for (ObList::iterator i = _links.begin(); i != _links.end(); ++i) {
		MovGraphLink *lnk = (MovGraphLink *)*i;

		if (flag1) {
			Common::Point point(x, y);
			double dist = calcDistance(&point, lnk, 0);

			if (dist >= 0.0 && dist < 2.0) {
				movarr = new MovArr;

				movarr->_link = lnk;
				movarr->_dist = ((double)(lnk->_movGraphNode1->_y - lnk->_movGraphNode2->_y) * (double)(lnk->_movGraphNode1->_y - point.y) + 
								 (double)(lnk->_movGraphNode2->_x - lnk->_movGraphNode1->_x) * (double)(point.x - lnk->_movGraphNode1->_x)) / 
					lnk->_distance / lnk->_distance;
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
							movarr->_point.x = lnk->_movGraphNode1->_x;
							movarr->_point.y = lnk->_movGraphNode1->_y;
							arr->push_back(movarr);

							movarr = new MovArr;
							movarr->_link = lnk;
							movarr->_dist = 1.0;
							movarr->_point.x = lnk->_movGraphNode1->_x;
							movarr->_point.y = lnk->_movGraphNode1->_y;
							arr->push_back(movarr);
						}
					} else {
						movarr = new MovArr;
						movarr->_link = lnk;
						movarr->_dist = ((double)(lnk->_movGraphNode1->_y - lnk->_movGraphNode2->_y) * (double)(lnk->_movGraphNode1->_y - y) + 
										 (double)(lnk->_movGraphNode2->_x - lnk->_movGraphNode1->_x) * (double)(x - lnk->_movGraphNode1->_x)) / 
							lnk->_distance / lnk->_distance;
						movarr->_point.x = x;
						movarr->_point.y = y;

						calcDistance(&movarr->_point, lnk, 0);

						arr->push_back(movarr);
					}
				}
			}
		}
	}

	*arrSize = arr->size();

	return arr;
}

void MovGraph::shuffleTree(MovGraphLink *lnk, MovGraphLink *lnk2, Common::Array<MovGraphLink *> &tempObList1, Common::Array<MovGraphLink *> &tempObList2) {
	if (lnk == lnk2) {
		for (uint i = 0; i < tempObList1.size(); i++)
			tempObList2.push_back(tempObList1[i]);

		tempObList2.push_back(lnk);
	} else {
		lnk->_flags |= 0x80000000;

		tempObList1.push_back(lnk);

		for (ObList::iterator i = _links.begin(); i != _links.end(); ++i) {
			MovGraphLink *l = (MovGraphLink *)*i;

			if (l->_movGraphNode1 != lnk->_movGraphNode1) {
				if (l->_movGraphNode2 != lnk->_movGraphNode1) {
					if (l->_movGraphNode1 != lnk->_movGraphNode2 && l->_movGraphNode2 != lnk->_movGraphNode2)
						continue;
				}
			}

			if (!(l->_flags & 0xA0000000))
				shuffleTree(l, lnk2, tempObList1, tempObList2);
		}

		lnk->_flags &= 0x7FFFFFFF;
	}
}

Common::Array<MovItem *> *MovGraph::calcMovItems(MovArr *movarr1, MovArr *movarr2, int *listCount) {
	Common::Array<MovGraphLink *> tempObList1;
	Common::Array<MovGraphLink *> tempObList2;

	shuffleTree(movarr1->_link, movarr2->_link, tempObList1, tempObList2);

	*listCount = 0;

	if (!tempObList2.size())
		return 0;

	*listCount = tempObList2.size();

	Common::Array<MovItem *> *res = new Common::Array<MovItem *>;

	for (int i = 0; i < *listCount; i++) {
		MovItem *r = new MovItem;

		genMovItem(r, tempObList2[i], movarr1, movarr2);

		res->push_back(r);

		delete tempObList2[i];
	}

	movarr2->_link = movarr1->_link;

	return res;
}

void MovGraph::genMovItem(MovItem *movitem, MovGraphLink *grlink, MovArr *movarr1, MovArr *movarr2) {
	warning("STUB: MovGraph::genMovItem()");
}

bool MovGraph::calcChunk(int idx, int x, int y, MovArr *arr, int a6) {
	int staticsId;

	if (_items[idx]->ani->_statics) {
		staticsId = _items[idx]->ani->_statics->_staticsId;
	} else {
		if (!_items[idx]->ani->_movement->_staticsObj2)
			return 0;

		staticsId = _items[idx]->ani->_movement->_staticsObj2->_staticsId;
	}

	int arrSize;

	Common::Array<MovArr *> *movarr = genMovArr(x, y, &arrSize, 0, 1);

	if (!movarr)
		return findClosestLink(idx, 0, arr);

	bool res = false;

	int idxmin = -1;
	int offmin = 100;

	for (int i = 0; i < arrSize; i++) {
		int off = _mgm.refreshOffsets(_items[idx]->ani->_id, staticsId, (*movarr)[i]->_link->_dwordArray2[_field_44]);

		if (off < offmin) {
			offmin = off;
			idxmin = i;
		}

		off = _mgm.refreshOffsets(_items[idx]->ani->_id, staticsId, (*movarr)[i]->_link->_dwordArray2[_field_44 + 1]);
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
	if (step1->link->_movGraphNode1 == step2->link->_movGraphNode2) {
		step1->sfield_0 = 1;
		step2->sfield_0 = 1;

		return;
	}

	if (step1->link->_movGraphNode1 == step2->link->_movGraphNode1) {
		step1->sfield_0 = 1;
		step2->sfield_0 = 0;
	} else {
		step1->sfield_0 = 0;

		if (step1->link->_movGraphNode2 != step2->link->_movGraphNode1) {
			step2->sfield_0 = 1;
		} else {
			step2->sfield_0 = 0;
		}
	}
}

int MovGraph2::getItemIndexByGameObjectId(int objectId) {
	for (uint i = 0; i < _items2.size(); i++)
		if (_items2[i]->_objectId == objectId)
			return i;

	return -1;
}

int MovGraph2::getItemSubIndexByStaticsId(int idx, int staticsId) {
	for (int i = 0; i < 4; i++)
		if (_items2[idx]->_subItems[i]._staticsId1 == staticsId || _items2[idx]->_subItems[i]._staticsId2 == staticsId)
			return i;

	return -1;
}

int MovGraph2::getItemSubIndexByMovementId(int idx, int movId) {
	for (int i = 0; i < 4; i++)
		if (_items2[idx]->_subItems[i]._walk[0]._movementId == movId || _items2[idx]->_subItems[i]._turn[0]._movementId == movId ||
			_items2[idx]->_subItems[i]._turnS[0]._movementId == movId)
			return i;

	return -1;
}

int MovGraph2::getItemSubIndexByMGM(int index, StaticANIObject *ani) {
	if (findNode(ani->_ox, ani->_oy, 0) || findLink1(ani->_ox, ani->_oy, -1, 0) || findLink2(ani->_ox, ani->_oy)) {
		int minidx = -1;
		int min = 0;

		for (int i = 0; i < 4; i++) {
			int tmp = _mgm.refreshOffsets(ani->_id, ani->_statics->_staticsId, _items2[index]->_subItems[i]._staticsId1);

			if (tmp >= 0 && (minidx == -1 || tmp < min)) {
				minidx = i;
				min = tmp;
			}
		}

		return minidx;
	}

	return -1;
}

bool MovGraph2::initDirections(StaticANIObject *obj, MovGraph2Item *item) {
	item->_obj = obj;
	item->_objectId = obj->_id;

	GameVar *var = g_fp->getGameLoaderGameVar()->getSubVarByName(obj->_objectName);
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
			}

			item->_subItems[dir]._walk[act]._movementId = idx;

			Movement *mov = obj->getMovementById(idx);

			item->_subItems[dir]._walk[act]._mov = mov;
			if (mov) {
				mov->calcSomeXY(point, 0, -1);
				item->_subItems[dir]._walk[act]._mx = point.x;
				item->_subItems[dir]._walk[act]._my = point.y;
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
			}

			item->_subItems[dir]._turn[act]._movementId = idx;

			Movement *mov = obj->getMovementById(idx);

			item->_subItems[dir]._turn[act]._mov = mov;
			if (mov) {
				mov->calcSomeXY(point, 0, -1);
				item->_subItems[dir]._turn[act]._mx = point.x;
				item->_subItems[dir]._turn[act]._my = point.y;
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
			}

			item->_subItems[dir]._turnS[act]._movementId = idx;

			Movement *mov = obj->getMovementById(idx);

			item->_subItems[dir]._turnS[act]._mov = mov;
			if (mov) {
				mov->calcSomeXY(point, 0, -1);
				item->_subItems[dir]._turnS[act]._mx = point.x;
				item->_subItems[dir]._turnS[act]._my = point.y;
			}
		}

		item->_subItems[dir]._staticsId1 = item->_subItems[dir]._walk[0]._mov->_staticsObj1->_staticsId;
		item->_subItems[dir]._staticsId2 = item->_subItems[dir]._walk[0]._mov->_staticsObj2->_staticsId;

	}
	return true;
}

void MovGraph2::addObject(StaticANIObject *obj) {
	MovGraph::addObject(obj);

	int id = getItemIndexByGameObjectId(obj->_id);

	if (id >= 0) {
		_items2[id]->_obj = obj;
	} else {
		MovGraph2Item *item = new MovGraph2Item;

		if (initDirections(obj, item)) {
			_items2.push_back(item);
		} else {
			delete item;
		}
	}
}

void MovGraph2::buildMovInfo1SubItems(MovInfo1 *movinfo, Common::Array<MovGraphLink *> *linkList, LinkInfo *lnkSrc, LinkInfo *lnkDst) {
	MovInfo1Sub *elem;
	Common::Point point;
	Common::Rect rect;

	int subIndex = movinfo->subIndex;

	movinfo->items.clear();

	elem = new MovInfo1Sub;
	elem->subIndex = subIndex;
	elem->x = movinfo->pt1.x;
	elem->y = movinfo->pt1.y;
	elem->distance = -1;

	movinfo->items.push_back(elem);

	int prevSubIndex = movinfo->subIndex;

	for (uint i = 0; i < linkList->size(); i++) {
		int idx1;

		if (linkList->size() <= 1) {
			if (linkList->size() == 1)
				idx1 = getShortSide((*linkList)[0], movinfo->pt2.x - movinfo->pt1.x, movinfo->pt2.y - movinfo->pt1.y);
			else
				idx1 = getShortSide(0, movinfo->pt2.x - movinfo->pt1.x, movinfo->pt2.y - movinfo->pt1.y);

			point.y = -1;
			rect.bottom = -1;
			rect.right = -1;
			rect.top = -1;
			rect.left = -1;
		} else {
			idx1 = findLink(linkList, i, &rect, &point);
		}

		if (idx1 != prevSubIndex) {
			prevSubIndex = idx1;
			subIndex = idx1;

			elem = new MovInfo1Sub;
			elem->subIndex = subIndex;
			elem->x = rect.left;
			elem->y = rect.top;
			elem->distance = -1;

			movinfo->items.push_back(elem);
		}

		if (i != linkList->size() - 1) {
			while (1) {
				i++;
				if (findLink(linkList, i, &rect, 0) != prevSubIndex) {
					i--;
					findLink(linkList, i, &rect, &point);

					break;
				}

				if (i == linkList->size() - 1)
					break;
			}
		}

		if (movinfo->items.back()->subIndex != 10) {
			subIndex = prevSubIndex;

			elem = new MovInfo1Sub;
			elem->subIndex = 10;
			elem->x = -1;
			elem->y = -1;
			elem->distance = -1;

			movinfo->items.push_back(elem);

			if (i == linkList->size()) {
				elem = new MovInfo1Sub;
				elem->subIndex = prevSubIndex;
				elem->x = movinfo->pt2.x;
				elem->y = movinfo->pt2.y;
				elem->distance = movinfo->distance2;

				movinfo->items.push_back(elem);
			} else {
				elem = new MovInfo1Sub;
				elem->subIndex = prevSubIndex;
				elem->x = rect.right;
				elem->y = rect.bottom;
				elem->distance = point.y;

				movinfo->items.push_back(elem);
			}
		}
	}

	if (subIndex != movinfo->item1Index) {
		elem = new MovInfo1Sub;
		elem->subIndex = movinfo->item1Index;
		elem->x = movinfo->pt2.x;
		elem->y = movinfo->pt2.y;
		elem->distance = movinfo->distance2;

		movinfo->items.push_back(elem);
	}

	movinfo->itemsCount = movinfo->items.size();
}

MessageQueue *MovGraph2::buildMovInfo1MessageQueue(MovInfo1 *movInfo) {
	MovInfo1 movinfo(movInfo);

	int curX = movInfo->pt1.x;
	int curY = movInfo->pt1.y;
	int curDistance = movInfo->distance1;

	MessageQueue *mq = new MessageQueue(g_fp->_globalMessageQueueList->compact());

	for (int i = 0; i < movInfo->itemsCount - 1; i++) {
		if (movInfo->items[i + 1]->subIndex != 10) {
			MG2I *mg2i;

			if (i >= movInfo->itemsCount - 2 || movInfo->items[i + 2]->subIndex != 10) {
				movinfo.flags = 0;
				mg2i = &_items2[movInfo->index]->_subItems[movInfo->items[i]->subIndex]._turnS[movInfo->items[i + 1]->subIndex];
			} else {
				movinfo.flags = 2;
				mg2i = &_items2[movInfo->index]->_subItems[movInfo->items[i]->subIndex]._turn[movInfo->items[i + 1]->subIndex];
			}
			if (i < movInfo->itemsCount - 2
				|| (movInfo->items[i]->x == movInfo->items[i + 1]->x
					&& movInfo->items[i]->y == movInfo->items[i + 1]->y)
				 || movInfo->items[i]->x == -1
				 || movInfo->items[i]->y == -1
				 || movInfo->items[i + 1]->x == -1
				 || movInfo->items[i + 1]->y == -1) {

				ExCommand *ex = new ExCommand(_items2[movInfo->index]->_objectId, 1, mg2i->_movementId, 0, 0, 0, 1, 0, 0, 0);

				ex->_excFlags |= 2;
				ex->_keyCode = _items2[movInfo->index]->_obj->_okeyCode;
				ex->_field_24 = 1;
				ex->_field_14 = -1;
				mq->addExCommandToEnd(ex);

				curX += mg2i->_mx;
				curY += mg2i->_my;
			} else {
				MGMInfo mgminfo;

				memset(&mgminfo, 0, sizeof(mgminfo));

				mgminfo.ani = _items2[movInfo->index]->_obj;
				mgminfo.staticsId2 = mg2i->_mov->_staticsObj2->_staticsId;
				mgminfo.x1 = movInfo->items[i + 1]->x;
				mgminfo.y1 = movInfo->items[i + 1]->y;
				mgminfo.field_1C = movInfo->items[i + 1]->distance;
				mgminfo.staticsId1 = mg2i->_mov->_staticsObj1->_staticsId;

				mgminfo.x2 = movInfo->items[i]->x;
				mgminfo.y2 = movInfo->items[i]->y;
				mgminfo.field_10 = 1;
				mgminfo.flags = 0x7f;
				mgminfo.movementId = mg2i->_movementId;

				MessageQueue *mq2 = _mgm.genMovement(&mgminfo);
				mq->transferExCommands(mq2);

				delete mq2;

				curX = movInfo->items[i + 1]->x;
				curY = movInfo->items[i + 1]->y;
			}
		} else {
			movinfo.item1Index = movInfo->items[i]->subIndex;
			movinfo.subIndex = movinfo.item1Index;
			movinfo.pt1.y = curY;
			movinfo.pt1.x = curX;

			movinfo.distance1 = curDistance;
			movinfo.pt2.x = movInfo->items[i + 2]->x;
			movinfo.pt2.y = movInfo->items[i + 2]->y;
			movinfo.distance2 = movInfo->items[i + 2]->distance;

			if (i >= movInfo->itemsCount - 4
				 || movInfo->items[i + 2]->subIndex == 10
				 || movInfo->items[i + 3]->subIndex == 10
				 || movInfo->items[i + 2]->subIndex == movInfo->items[i + 3]->subIndex
				 || movInfo->items[i + 4]->subIndex != 10) {
				if (i >= movInfo->itemsCount - 3
					 || movInfo->items[i + 2]->subIndex == 10
					 || movInfo->items[i + 3]->subIndex == 10
					 || movInfo->items[i + 2]->subIndex == movInfo->items[i + 3]->subIndex) {
					movinfo.flags &= 3;
				} else {
					MG2I *m = &_items2[movInfo->index]->_subItems[movInfo->items[i + 2]->subIndex]._turnS[movInfo->items[i + 3]->subIndex];
					movinfo.pt2.x -= m->_mx;
					movinfo.pt2.y -= m->_my;
					movinfo.flags &= 3;
				}
			} else {
				MG2I *m = &_items2[movInfo->index]->_subItems[movInfo->items[i + 2]->subIndex]._turn[movInfo->items[i + 3]->subIndex];

				if (movinfo.item1Index && movinfo.item1Index != 1) {
					movinfo.pt2.y -= m->_my;
					movinfo.flags = (movinfo.flags & 2) | 1;
				} else {
					movinfo.pt2.x -= m->_mx;
					movinfo.flags = (movinfo.flags & 2) | 1;
				}
			}
			i++; // intentional

			MessageQueue *mq2 = genMovement(&movinfo);

			if (!mq2) {
				delete mq;
				return 0;
			}

			mq->transferExCommands(mq2);

			delete mq2;

			curX = movinfo.pt2.x;
			curY = movinfo.pt2.y;
			curDistance = movinfo.distance2;
		}
	}

	movInfo->pt2.x = movinfo.pt2.x;
	movInfo->pt2.y = movinfo.pt2.y;

	return mq;
}

int MovGraph2::removeObject(StaticANIObject *obj) {
	warning("STUB: MovGraph2::removeObject()");

	return 0;
}

void MovGraph2::freeItems() {
	for (uint i = 0; i < _items2.size(); i++)
		delete _items2[i];

	_items2.clear();
}

MessageQueue *MovGraph2::method34(StaticANIObject *ani, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	if (!ani->isIdle())
		return 0;

	if (ani->_flags & 0x100)
		return 0;

	MessageQueue *mq = doWalkTo(ani, xpos, ypos, fuzzyMatch, staticsId);

	if (!mq)
		return 0;

	if (ani->_movement) {
		if (mq->getCount() <= 1 || mq->getExCommandByIndex(0)->_messageKind != 22) {
			PicAniInfo picAniInfo;

			ani->getPicAniInfo(&picAniInfo);
			ani->updateStepPos();
			MessageQueue *mq1 = doWalkTo(ani, xpos, ypos, fuzzyMatch, staticsId);

			ani->setPicAniInfo(&picAniInfo);

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

MessageQueue *MovGraph2::doWalkTo(StaticANIObject *obj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	LinkInfo linkInfoDest;
	LinkInfo linkInfoSource;
	MovInfo1 movInfo1;
	PicAniInfo picAniInfo;
	Common::Point point;

	debug(0, "MovGraph2::doWalkTo(%d, %d, %d, %d, %d)", obj->_id, xpos, ypos, fuzzyMatch, staticsId);

	int idx = getItemIndexByGameObjectId(obj->_id);

	if (idx < 0)
		return 0;

	linkInfoSource.link = 0;
	linkInfoSource.node = 0;

	linkInfoDest.link = 0;
	linkInfoDest.node = 0;

	point.x = 0;

	obj->getPicAniInfo(&picAniInfo);

	int idxsub;

	if (obj->_movement)
		idxsub = getItemSubIndexByMovementId(idx, obj->_movement->_id);
	else
		idxsub = getItemSubIndexByStaticsId(idx, obj->_statics->_staticsId);

	bool subMgm = false;

	if (idxsub == -1) {
		idxsub = getItemSubIndexByMGM(idx, obj);
		subMgm = true;

		if (idxsub == -1)
			return 0;
	}

	if (obj->_movement) {
		int newx, newy;

		if (subMgm) {
			obj->_messageQueueId = 0;
			obj->changeStatics2(_items2[idx]->_subItems[idxsub]._staticsId1);
			newx = obj->_ox;
			newy = obj->_oy;
		} else {
			obj->_movement->calcSomeXY(point, 0, picAniInfo.dynamicPhaseIndex);
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
		g_fp->_globalMessageQueueList->compact();

		MessageQueue *mq = new MessageQueue();

		if (staticsId && obj->_statics->_staticsId != staticsId) {
			int idxwalk = getItemSubIndexByStaticsId(idx, staticsId);
			if (idxwalk == -1) {
				obj->setPicAniInfo(&picAniInfo);

				delete mq;

				return 0;
			}

			ExCommand *ex = new ExCommand(picAniInfo.objectId, 1, _items2[idx]->_subItems[idxsub]._walk[idxwalk]._movementId, 0, 0, 0, 1, 0, 0, 0);

			ex->_field_24 = 1;
			ex->_keyCode = picAniInfo.field_8;
			ex->_excFlags |= 2;

			mq->addExCommandToEnd(ex);
		} else {
			ExCommand *ex = new ExCommand(picAniInfo.objectId, 22, obj->_statics->_staticsId, 0, 0, 0, 1, 0, 0, 0);

			ex->_keyCode = picAniInfo.field_8;
			ex->_excFlags |= 3;
			mq->addExCommandToEnd(ex);

			ex = new ExCommand(picAniInfo.objectId, 5, -1, obj->_ox, obj->_oy, 0, 1, 0, 0, 0);

			ex->_field_14 = -1;
			ex->_keyCode = picAniInfo.field_8;
			ex->_excFlags |= 3;
			mq->addExCommandToEnd(ex);
		}

		obj->setPicAniInfo(&picAniInfo);

		return mq;
	}

	linkInfoSource.node = findNode(obj->_ox, obj->_oy, 0);

	if (!linkInfoSource.node) {
		linkInfoSource.link = findLink1(obj->_ox, obj->_oy, idxsub, 0);

		if (!linkInfoSource.link) {
			linkInfoSource.link = findLink2(obj->_ox, obj->_oy);

			if (!linkInfoSource.link) {
				obj->setPicAniInfo(&picAniInfo);

				return 0;
			}
		}
	}

	linkInfoDest.node = findNode(xpos, ypos, fuzzyMatch);

	if (!linkInfoDest.node) {
		linkInfoDest.link = findLink1(xpos, ypos, idxsub, fuzzyMatch);

		if (!linkInfoDest.link) {
			obj->setPicAniInfo(&picAniInfo);

			return 0;
		}
	}

	Common::Array<MovGraphLink *> tempLinkList;
	double minPath = findMinPath(&linkInfoSource, &linkInfoDest, &tempLinkList);

	debug(0, "MovGraph2::doWalkTo(): path: %g  parts: %d", minPath, tempLinkList.size());

	if (minPath < 0.0 || ((linkInfoSource.node != linkInfoDest.node || !linkInfoSource.node) && !tempLinkList.size()))
		return 0;

	movInfo1.clear();

	movInfo1.subIndex = idxsub;
	movInfo1.pt1.x = obj->_ox;
	movInfo1.pt1.y = obj->_oy;

	int dx1 = obj->_ox;
	int dy1 = obj->_oy;
	int dx2, dy2;

	if (linkInfoSource.node)
		movInfo1.distance1 = linkInfoSource.node->_distance;
	else
		movInfo1.distance1 = linkInfoSource.link->_movGraphNode1->_distance;

	if (linkInfoDest.node) {
		dx2 = linkInfoDest.node->_x;
		dy2 = linkInfoDest.node->_y;

		movInfo1.pt2.x = linkInfoDest.node->_x;
		movInfo1.pt2.y = linkInfoDest.node->_y;

		movInfo1.distance2 = linkInfoDest.node->_distance;
	} else {
		movInfo1.pt2.x = xpos;
		movInfo1.pt2.y = ypos;

		MovGraphNode *nod = linkInfoDest.link->_movGraphNode1;
		double dst1 = sqrt((double)((ypos - nod->_y) * (ypos - nod->_y) + (xpos - nod->_x) * (xpos - nod->_x)));
		int dst = linkInfoDest.link->_movGraphNode2->_distance - nod->_distance;

		movInfo1.distance2 = (int)(nod->_distance + (dst1 * (double)dst / linkInfoDest.link->_distance));

		calcDistance(&movInfo1.pt2, linkInfoDest.link, 1);

		dx1 = movInfo1.pt1.x;
		dy1 = movInfo1.pt1.y;
		dx2 = movInfo1.pt2.x;
		dy2 = movInfo1.pt2.y;
	}

	if (staticsId) {
		movInfo1.item1Index = getItemSubIndexByStaticsId(idx, staticsId);
	} else if (tempLinkList.size() <= 1) {
		if (tempLinkList.size() == 1)
			movInfo1.item1Index = getShortSide(tempLinkList[0], dx2 - dx1, dy2 - dy1);
		else
			movInfo1.item1Index = getShortSide(0, dx2 - dx1, dy2 - dy1);
	} else {
		movInfo1.item1Index = findLink(&tempLinkList, tempLinkList.size() - 1, 0, 0);
	}

	movInfo1.flags = fuzzyMatch != 0;

	if (_items2[idx]->_subItems[idxsub]._staticsId1 != obj->_statics->_staticsId)
		movInfo1.flags |= 2;

	buildMovInfo1SubItems(&movInfo1, &tempLinkList, &linkInfoSource, &linkInfoDest);

	MessageQueue *mq = buildMovInfo1MessageQueue(&movInfo1);

	linkInfoDest.node = findNode(movInfo1.pt2.x, movInfo1.pt2.y, fuzzyMatch);

	if (!linkInfoDest.node)
		linkInfoDest.link = findLink1(movInfo1.pt2.x, movInfo1.pt2.y, movInfo1.item1Index, fuzzyMatch);

	if (fuzzyMatch || linkInfoDest.link || linkInfoDest.node) {
		if (mq && mq->getCount() > 0 && picAniInfo.movementId) {
			ExCommand *ex = mq->getExCommandByIndex(0);

			if (ex && (ex->_messageKind == 1 || ex->_messageKind == 20)
					&& picAniInfo.movementId == ex->_messageNum
					&& picAniInfo.someDynamicPhaseIndex == ex->_field_14) {
				mq->deleteExCommandByIndex(0, 1);
			} else {
				ex = new ExCommand(picAniInfo.objectId, 5, ex->_messageNum, obj->_ox, obj->_oy, 0, 1, 0, 0, 0);
				ex->_field_14 = -1;
				ex->_keyCode = picAniInfo.field_8;
				ex->_excFlags |= 2;
				mq->addExCommand(ex);

				ex = new ExCommand(picAniInfo.objectId, 22, _items2[idx]->_subItems[idxsub]._staticsId1, 0, 0, 0, 1, 0, 0, 0);

				ex->_keyCode = picAniInfo.field_8;
				ex->_excFlags |= 3;
				mq->addExCommand(ex);
			}
		}
	} else {
		if (mq)
			delete mq;
		mq = 0;
	}

	obj->setPicAniInfo(&picAniInfo);

	return mq;
}

MovGraphNode *MovGraph2::findNode(int x, int y, int fuzzyMatch) {
	for (ObList::iterator i = _nodes.begin(); i != _nodes.end(); ++i) {
		assert(((CObject *)*i)->_objtype == kObjTypeMovGraphNode);

		MovGraphNode *node = (MovGraphNode *)*i;

		if (fuzzyMatch) {
			if (abs(node->_x - x) < 15 && abs(node->_y - y) < 15)
				return node;
		} else {
			if (node->_x == x && node->_y == y)
				return node;
		}
	}

	return 0;
}

int MovGraph2::getShortSide(MovGraphLink *lnk, int x, int y) {
	bool cond;

	if (lnk)
		cond = abs(lnk->_movGraphNode2->_x - lnk->_movGraphNode1->_x) > abs(lnk->_movGraphNode2->_y - lnk->_movGraphNode1->_y);
	else
		cond = abs(x) > abs(y);

	if (cond)
		return x <= 0;
	else
		return ((y > 0) + 2);
}

int MovGraph2::findLink(Common::Array<MovGraphLink *> *linkList, int idx, Common::Rect *rect, Common::Point *point) {
	MovGraphNode *node1 = (*linkList)[idx]->_movGraphNode1;
	MovGraphNode *node2 = (*linkList)[idx]->_movGraphNode2;
	MovGraphNode *node3 = node1;

	if (idx != 0) {
		MovGraphLink *lnk = (*linkList)[idx - 1];

		if (lnk->_movGraphNode2 != node1) {
			if (lnk->_movGraphNode1 != node1) {
				if (lnk->_movGraphNode2 == node2 || lnk->_movGraphNode1 == node2) {
					node3 = node2;
					node2 = node1;
				}
				goto LABEL_7;
			}
		}
		node3 = node1;
	} else if (idx != (int)(linkList->size() - 1)) {
		MovGraphLink *lnk = (*linkList)[idx + 1];

		if (lnk->_movGraphNode2 == node1 || lnk->_movGraphNode1 == node1) {
			node3 = node2;
			node2 = node1;
		} else if (lnk->_movGraphNode2 == node2 || lnk->_movGraphNode1 == node2) {
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
		point->x = node3->_distance;
		point->y = node2->_distance;
	}

	if (abs(node3->_x - node2->_x) <= abs(node3->_y - node2->_y))
		return (node3->_y < node2->_x) + 2;
	else
		return node3->_x >= node2->_x;
}

MessageQueue *MovGraph2::genMovement(MovInfo1 *info) {
	int mx1 = 0;
	int my1 = 0;

	if (!(info->flags & 2)) {
		mx1 = _items2[info->index]->_subItems[info->subIndex]._walk[0]._mx;
		my1 = _items2[info->index]->_subItems[info->subIndex]._walk[0]._my;
	}

	int mx2 = 0;
	int my2 = 0;

	if (!(info->flags & 4)) {
		mx2 = _items2[info->index]->_subItems[info->subIndex]._walk[2]._mx;
		my2 = _items2[info->index]->_subItems[info->subIndex]._walk[2]._my;
	}

	Common::Point point;

	int y = info->pt2.y - info->pt1.y - my2 - my1;
	int x = info->pt2.x - info->pt1.x - mx2 - mx1;
	int a2 = 0;
	int mgmLen;

	_mgm.calcLength(&point, _items2[info->index]->_subItems[info->subIndex]._walk[1]._mov, x, y, &mgmLen, &a2, info->flags & 1);

	int x1 = point.x;
	int y1 = point.y;

	if (!(info->flags & 1)) {
		if (info->subIndex == 1 || info->subIndex == 0) {
			a2 = -1;
			x1 = mgmLen * _items2[info->index]->_subItems[info->subIndex]._walk[1]._mx;
			x = x1;
			info->pt2.x = x1 + info->pt1.x + mx1 + mx2;
		}
	}

	if (!(info->flags & 1)) {
		if (info->subIndex == 2 || info->subIndex == 3) {
			a2 = -1;
			y1 = mgmLen * _items2[info->index]->_subItems[info->subIndex]._walk[1]._my;
			y = y1;
			info->pt2.y = y1 + info->pt1.y + my1 + my2;
		}
	}

	int cntX = 0;
	int cntY = 0;

	if (!(info->flags & 2)) {
		cntX = _items2[info->index]->_subItems[info->subIndex]._walk[0]._mov->countPhasesWithFlag(-1, 1);
		cntY = _items2[info->index]->_subItems[info->subIndex]._walk[0]._mov->countPhasesWithFlag(-1, 2);
	}

	if (mgmLen > 1) {
		cntX += (mgmLen - 1) * _items2[info->index]->_subItems[info->subIndex]._walk[1]._mov->countPhasesWithFlag(-1, 1);
		cntY += (mgmLen - 1) * _items2[info->index]->_subItems[info->subIndex]._walk[1]._mov->countPhasesWithFlag(-1, 2);
	}

	if (mgmLen > 0) {
		cntX += _items2[info->index]->_subItems[info->subIndex]._walk[1]._mov->countPhasesWithFlag(a2, 1);
		cntY += _items2[info->index]->_subItems[info->subIndex]._walk[1]._mov->countPhasesWithFlag(a2, 2);
	}

	if (!(info->flags & 4)) {
		cntX += _items2[info->index]->_subItems[info->subIndex]._walk[2]._mov->countPhasesWithFlag(-1, 1);
		cntY += _items2[info->index]->_subItems[info->subIndex]._walk[2]._mov->countPhasesWithFlag(-1, 2);
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

	MessageQueue *mq = new MessageQueue(g_fp->_globalMessageQueueList->compact());
	ExCommand *ex;

	if (info->flags & 2) {
		ex = new ExCommand(
							_items2[info->index]->_objectId,
							5,
							_items2[info->index]->_subItems[info->subIndex]._walk[1]._movementId,
							info->pt1.x,
							info->pt1.y,
							0,
							1,
							0,
							0,
							0);

		ex->_field_14 = info->distance1;

		ex->_keyCode = _items2[info->index]->_obj->_okeyCode;
		ex->_field_24 = 1;
		ex->_excFlags |= 2;
	} else {
		ex = new ExCommand(
							 _items2[info->index]->_objectId,
							 5,
							 _items2[info->index]->_subItems[info->subIndex]._walk[0]._movementId,
							 info->pt1.x,
							 info->pt1.y,
							 0,
							 1,
							 0,
							 0,
							 0);

		ex->_field_14 = info->distance1;

		ex->_keyCode = _items2[info->index]->_obj->_okeyCode;
		ex->_field_24 = 1;
		ex->_excFlags |= 2;
		mq->addExCommandToEnd(ex);

		ex = _mgm.buildExCommand2(
								  _items2[info->index]->_subItems[info->subIndex]._walk[0]._mov,
								  _items2[info->index]->_objectId,
								  x1,
								  y1,
								  &x2,
								  &y2,
								  -1);
		ex->_parId = mq->_id;
		ex->_keyCode = _items2[info->index]->_obj->_okeyCode;
	}

	mq->addExCommandToEnd(ex);

	for (int i = 0; i < mgmLen; ++i) {
		int par;

		if (i == mgmLen - 1)
			par = a2;
		else
			par = -1;

		ex = _mgm.buildExCommand2(
								  _items2[info->index]->_subItems[info->subIndex]._walk[1]._mov,
								  _items2[info->index]->_objectId,
								  x1,
								  y1,
								  &x2,
								  &y2,
								  par);
		ex->_parId = mq->_id;
		ex->_keyCode = _items2[info->index]->_obj->_okeyCode;
		mq->addExCommandToEnd(ex);
	}

	if (!(info->flags & 4)) {
		ex = _mgm.buildExCommand2(
								  _items2[info->index]->_subItems[info->subIndex]._walk[2]._mov,
								  _items2[info->index]->_objectId,
								  x1,
								  y1,
								  &x2,
								  &y2,
								  -1);
		ex->_parId = mq->_id;
		ex->_keyCode = _items2[info->index]->_obj->_okeyCode;

		mq->addExCommandToEnd(ex);
	}

    ex = new ExCommand(_items2[info->index]->_objectId, 5, -1, info->pt2.x, info->pt2.y, 0, 1, 0, 0, 0);
	ex->_field_14 = info->distance2;

	ex->_keyCode = _items2[info->index]->_obj->_okeyCode;
	ex->_field_24 = 0;
	ex->_excFlags |= 2;

	mq->addExCommandToEnd(ex);

	return mq;
}

MovGraphLink *MovGraph2::findLink1(int x, int y, int idx, int fuzzyMatch) {
	Common::Point point;
	MovGraphLink *res = 0;

	for (ObList::iterator i = _links.begin(); i != _links.end(); ++i) {
		assert(((CObject *)*i)->_objtype == kObjTypeMovGraphLink);

		MovGraphLink *lnk = (MovGraphLink *)*i;

		if (fuzzyMatch) {
			point.x = x;
			point.y = y;
			double dst = calcDistance(&point, lnk, 0);

			if (dst >= 0.0 && dst < 2.0)
				return lnk;
		} else if (!(lnk->_flags & 0x20000000)) {
			if (lnk->_movGraphReact->pointInRegion(x, y)) {
				if (abs(lnk->_movGraphNode1->_x - lnk->_movGraphNode2->_x) <= abs(lnk->_movGraphNode1->_y - lnk->_movGraphNode2->_y)) {
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

MovGraphLink *MovGraph2::findLink2(int x, int y) {
	double mindist = 1.0e20;
	MovGraphLink *res = 0;

	for (ObList::iterator i = _links.begin(); i != _links.end(); ++i) {
		assert(((CObject *)*i)->_objtype == kObjTypeMovGraphLink);

		MovGraphLink *lnk = (MovGraphLink *)*i;

		if (!(lnk->_flags & 0x20000000)) {
			double n1x = lnk->_movGraphNode1->_x;
			double n1y = lnk->_movGraphNode1->_y;
			double n2x = lnk->_movGraphNode2->_x;
			double n2y = lnk->_movGraphNode2->_y;
			double n1dx = n1x - x;
			double n1dy = n1y - y;
			double dst1 = sqrt(n1dy * n1dy + n1dx * n1dx);
			double coeff1 = ((n1y - n2y) * n1dy + (n2x - n1x) * n1dx) / lnk->_distance / dst1;
			double dst3 = coeff1 * dst1;
			double dst2 = sqrt(1.0 - coeff1 * coeff1) * dst1;

			if (coeff1 * dst1 < 0.0) {
				dst3 = 0.0;
				dst2 = sqrt(n1dy * n1dy + n1dx * n1dx);
			}
			if (dst3 > lnk->_distance) {
				dst3 = lnk->_distance;
				dst2 = sqrt((n2x - x) * (n2x - x) + (n2y - y) * (n2y - y));
			}
			if (dst3 >= 0.0 && dst3 <= lnk->_distance && dst2 < mindist) {
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

double MovGraph2::findMinPath(LinkInfo *linkInfoSource, LinkInfo *linkInfoDest, Common::Array<MovGraphLink *> *listObj) {
	LinkInfo linkInfoWorkSource;

	if (linkInfoSource->link != linkInfoDest->link || linkInfoSource->node != linkInfoDest->node) {
		double minDistance = -1.0;

		if (linkInfoSource->node) {
			for (ObList::iterator i = _links.begin(); i != _links.end(); ++i) {
				MovGraphLink *lnk = (MovGraphLink *)*i;

				if ((lnk->_movGraphNode1 == linkInfoSource->node || lnk->_movGraphNode2 == linkInfoSource->node) && !(lnk->_flags & 0xA0000000)) {
					linkInfoWorkSource.node = 0;
					linkInfoWorkSource.link = lnk;

					Common::Array<MovGraphLink *> tmpList;

					lnk->_flags |= 0x80000000;

					double newDistance = findMinPath(&linkInfoWorkSource, linkInfoDest, &tmpList);

					if (newDistance >= 0.0 && (minDistance < 0.0 || newDistance + lnk->_distance < minDistance)) {
						listObj->clear();
						listObj->push_back(tmpList);

						minDistance = newDistance + lnk->_distance;
					}

					lnk->_flags &= 0x7FFFFFFF;
				}
			}
		} else if (linkInfoSource->link) {
			linkInfoWorkSource.node = linkInfoSource->link->_movGraphNode1;
			linkInfoWorkSource.link = 0;

			Common::Array<MovGraphLink *> tmpList;

			double newDistance = findMinPath(&linkInfoWorkSource, linkInfoDest, &tmpList);

			if (newDistance >= 0.0) {
				listObj->clear();

				listObj->push_back(linkInfoSource->link);
				listObj->push_back(tmpList);

				minDistance = newDistance;
			}

			linkInfoWorkSource.link = 0;
			linkInfoWorkSource.node = linkInfoSource->link->_movGraphNode2;

			tmpList.clear();

			newDistance = findMinPath(&linkInfoWorkSource, linkInfoDest, &tmpList);

			if (newDistance >= 0 && (minDistance < 0.0 || newDistance < minDistance)) {
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

	for (ObList::iterator i = _nodes.begin(); i != _nodes.end(); ++i) {
		assert(((CObject *)*i)->_objtype == kObjTypeMovGraphNode);

		MovGraphNode *node = (MovGraphNode *)*i;

		double dist = sqrt((double)((node->_x - oy) * (node->_x - oy) + (node->_x - ox) * (node->_x - ox)));
		if (dist < mindist) {
			mindist = dist;
			res = node;
		}
	}

	return res;
}

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

	for (uint i = 0; i < obj->_staticsList.size(); i++)
		_items[idx]->statics.push_back((Statics *)obj->_staticsList[i]);

	for (uint i = 0; i < obj->_movements.size(); i++)
		_items[idx]->movements1.push_back((Movement *)obj->_movements[i]);

	_items[idx]->subItems.clear();
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

MovGraphLink::MovGraphLink() {
	_distance = 0;
	_angle = 0;
	_flags = 0x10000000;
	_movGraphNode2 = 0;
	_movGraphNode1 = 0;
	_field_3C = 0;
	_field_38 = 0;
	_movGraphReact = 0;
	_name = 0;

	_objtype = kObjTypeMovGraphLink;
}

MovGraphLink::~MovGraphLink() {
	delete _movGraphReact;

	_dwordArray1.clear();
	_dwordArray2.clear();
}


bool MovGraphLink::load(MfcArchive &file) {
	debug(5, "MovGraphLink::load()");

	_dwordArray1.load(file);
	_dwordArray2.load(file);

	_flags = file.readUint32LE();

	debug(8, "GraphNode1");
	_movGraphNode1 = (MovGraphNode *)file.readClass();
	debug(8, "GraphNode2");
	_movGraphNode2 = (MovGraphNode *)file.readClass();

	_distance = file.readDouble();
	_angle = file.readDouble();

	debug(8, "distance: %g, angle: %g", _distance, _angle);

	_movGraphReact = (MovGraphReact *)file.readClass();
	_name = file.readPascalString();

	return true;
}

void MovGraphLink::calcNodeDistanceAndAngle() {
	if (_movGraphNode1) {
		double dx = _movGraphNode2->_x - _movGraphNode1->_x;
		double dy = _movGraphNode2->_y - _movGraphNode1->_y;

		_distance = sqrt(dy * dy + dx * dx);
		_angle = atan2(dx, dy);
	}
}

bool MovGraphNode::load(MfcArchive &file) {
	debug(5, "MovGraphNode::load()");

	_field_14 = file.readUint32LE();
	_x = file.readUint32LE();
	_y = file.readUint32LE();
	_distance = file.readUint32LE();

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
	debug(5, "ReactParallel::load()");

	_x1 = file.readUint32LE();
	_y1 = file.readUint32LE();
	_x2 = file.readUint32LE();
	_y2 = file.readUint32LE();
	_dx = file.readUint32LE();
	_dy = file.readUint32LE();

	createRegion();

	return true;
}

void ReactParallel::createRegion() {
	_points = (Common::Point **)malloc(sizeof(Common::Point *) * 4);

	for (int i = 0; i < 4; i++)
		_points[i] = new Common::Point;

	double at = atan2((double)(_x1 - _x2), (double)(_y1 - _y2)) + 1.570796; // pi/2
	double sn = sin(at);
	double cs = cos(at);

	_points[0]->x = (int16)(_x1 - _dx * cs);
	_points[0]->y = (int16)(_y1 - _dx * sn);

	_points[1]->x = (int16)(_x2 - _dx * cs);
	_points[1]->y = (int16)(_y2 - _dx * sn);

	_points[2]->x = (int16)(_x2 + _dy * cs);
	_points[2]->y = (int16)(_y2 + _dy * sn);

	_points[3]->x = (int16)(_x1 + _dy * cs);
	_points[3]->y = (int16)(_y1 + _dy * sn);

	_pointCount = 4;
	// GdiObject::Attach(_rgn, CreatePolygonRgn(_points, 4, 2);
}

void ReactParallel::setCenter(int x1, int y1, int x2, int y2) {
	_x1 = x1;
	_y1 = y1;
	_x2 = x2;
	_y2 = y2;
}

ReactPolygonal::ReactPolygonal() {
	_centerX = 0;
	_centerY = 0;
	_bbox = 0;
}

ReactPolygonal::~ReactPolygonal() {
	delete _bbox;
}

bool ReactPolygonal::load(MfcArchive &file) {
	debug(5, "ReactPolygonal::load()");

	_centerX = file.readUint32LE();
	_centerY = file.readUint32LE();
	_pointCount = file.readUint32LE();

	if (_pointCount > 0) {
		_points = (Common::Point **)malloc(sizeof(Common::Point *) * _pointCount);

		for (int i = 0; i < _pointCount; i++) {
			_points[i] = new Common::Point;

			_points[i]->x = file.readUint32LE();
			_points[i]->y = file.readUint32LE();
		}

	}

	createRegion();

	return true;
}

void ReactPolygonal::createRegion() {
	if (_points) {

		// GdiObject::Attach(_rgn, CreatePolygonRgn(_points, _pointCount, 2);
	}
}

void ReactPolygonal::setCenter(int x1, int y1, int x2, int y2) {
	int cX = (x2 + x1) / 2;
	int cY = (y2 + y1) / 2;

	if (_points) {
		for (int i = 0; i < _pointCount; i++) {
			_points[i]->x += cX - _centerX;
			_points[i]->y += cY - _centerY;
		}
	}

	_centerX = cX;
	_centerY = cY;
}

void ReactPolygonal::getBBox(Common::Rect *rect) {
	if (!_pointCount)
		return;

	if (_bbox) {
		*rect = *_bbox;
		return;
	}

	rect->left = _points[0]->x;
	rect->top = _points[0]->y;
	rect->right = _points[0]->x;
	rect->bottom = _points[0]->y;

	for (int i = 1; i < _pointCount; i++) {
		if (rect->left > _points[i]->x)
			rect->left = _points[i]->x;

		if (rect->top < _points[i]->y)
			rect->top = _points[i]->y;

		if (rect->right < _points[i]->x)
			rect->right = _points[i]->x;

		if (rect->bottom > _points[i]->y)
			rect->bottom = _points[i]->y;
	}

	_bbox = new Common::Rect;
	*_bbox = *rect;
}


bool MovGraphReact::pointInRegion(int x, int y) {
	if (_pointCount < 3) {
		return false;
	}

	int counter = 0;
	double xinters;
	Common::Point p, p1, p2;

	p.x = x;
	p.y = y;

	p1.x = _points[0]->x;
	p1.y = _points[0]->y;

	for (int i = 1; i <= _pointCount; i++) {
		p2.x = _points[i % _pointCount]->x;
		p2.y = _points[i % _pointCount]->y;

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
		return (mc->method34(g_fp->_currentScene->getStaticANIObject1ById(objId, objKey), x, y, fuzzyMatch, 0) != 0);

	return 0;
}

bool doSomeAnimation(int objId, int objKey, int a3) {
	StaticANIObject *ani = g_fp->_currentScene->getStaticANIObject1ById(objId, objKey);
	MctlCompound *cmp = getCurrSceneSc2MotionController();

	if (ani && cmp)
		return cmp->method3C(ani, a3);

	return false;
}

bool doSomeAnimation2(int objId, int objKey) {
	return doSomeAnimation(objId, objKey, 0);
}

} // End of namespace Fullpipe
