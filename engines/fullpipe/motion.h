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

#ifndef FULLPIPE_MOTION_H
#define FULLPIPE_MOTION_H

#include "fullpipe/mgm.h"

namespace Fullpipe {

class MctlConnectionPoint;
class MovGraphLink;
class MessageQueue;
struct MovArr;
struct MovItem;

int startWalkTo(int objId, int objKey, int x, int y, int a5);
bool doSomeAnimation(int objId, int objKey, int a3);
bool doSomeAnimation2(int objId, int objKey);

class MotionController : public CObject {
public:
	int _field_4;
	bool _isEnabled;

public:
	MotionController() : _isEnabled(true), _field_4(0) {}
	virtual ~MotionController() {}
	virtual bool load(MfcArchive &file) override;
	virtual void methodC() {}
	virtual void method10() {}
	virtual void clearEnabled() { _isEnabled = false; }
	virtual void setEnabled() { _isEnabled = true; }
	virtual void addObject(StaticANIObject *obj) {}
	virtual int removeObject(StaticANIObject *obj) { return 0; }
	virtual void freeItems() {}
	virtual Common::Array<MovItem *> *method28(StaticANIObject *ani, int x, int y, int flag1, int *rescount) { return 0; }
	virtual bool method2C(StaticANIObject *obj, int x, int y) { return false; }
	virtual int method30() { return 0; }
	virtual MessageQueue *method34(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) { return 0; }
	virtual void changeCallback(MovArr *(*_callback1)(StaticANIObject *ani, Common::Array<MovItem *> *items, signed int counter)) {}
	virtual bool method3C(StaticANIObject *ani, int flag) { return 0; }
	virtual int method40() { return 0; }
	virtual bool method44(StaticANIObject *ani, int x, int y) { return false; }
	virtual int method48() { return -1; }
	virtual MessageQueue *doWalkTo(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) { return 0; }

	void enableLinks(const char *linkName, bool enable);
	MovGraphLink *getLinkByName(const char *name);
};

class MovGraphReact : public CObject {
public:
	int _pointCount;
	Common::Point **_points;

public:
	MovGraphReact() : _pointCount(0), _points(0) {}
	~MovGraphReact() { free(_points); }

	virtual void setCenter(int x1, int y1, int x2, int y2) {}
	virtual void createRegion() {}
	virtual bool pointInRegion(int x, int y);
};

class MctlCompoundArrayItem : public CObject {
public:
	MotionController *_motionControllerObj;
	MovGraphReact *_movGraphReactObj;
	Common::Array<MctlConnectionPoint *> _connectionPoints;
	int _field_20;
	int _field_24;
	int _field_28;

public:
	MctlCompoundArrayItem() : _movGraphReactObj(0), _motionControllerObj(0), _field_20(0), _field_24(0), _field_28(0) {}
	~MctlCompoundArrayItem();
};

class MctlCompoundArray : public Common::Array<MctlCompoundArrayItem *>, public CObject {
 public:
	virtual bool load(MfcArchive &file) override;
};

class MctlCompound : public MotionController {
public:
	MctlCompoundArray _motionControllers;

	MctlCompound() { _objtype = kObjTypeMctlCompound; }

	virtual bool load(MfcArchive &file) override;

	virtual void addObject(StaticANIObject *obj) override;
	virtual int removeObject(StaticANIObject *obj) override;
	virtual void freeItems() override;
	virtual MessageQueue *method34(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;
	virtual MessageQueue *doWalkTo(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;

	void initMovGraph2();
	MctlConnectionPoint *findClosestConnectionPoint(int ox, int oy, int destIndex, int connectionX, int connectionY, int sourceIndex, double *minDistancePtr);
	void replaceNodeX(int from, int to);

	uint getMotionControllerCount() { return _motionControllers.size(); }
	MotionController *getMotionController(int num) { return _motionControllers[num]->_motionControllerObj; }
};

struct MctlLadderMovementVars {
	int varUpGo;
	int varDownGo;
	int varUpStop;
	int varDownStop;
	int varUpStart;
	int varDownStart;
};

struct MctlLadderMovement {
	int objId;
	int staticIdsSize;
	MctlLadderMovementVars *movVars;
	int *staticIds;
};

class MctlLadder : public MotionController {
public:
	int _ladderX;
	int _ladderY;
	int _ladder_field_14;
	int _width;
	int _height;
	int _ladder_field_20;
	int _ladder_field_24;
	Common::Array<MctlLadderMovement *> _ladmovements;
	MGM _mgm;

public:
	MctlLadder();
	virtual ~MctlLadder();
	int collisionDetection(StaticANIObject *man);

	virtual void addObject(StaticANIObject *obj) override;
	virtual int removeObject(StaticANIObject *obj) override { return 1; }
	virtual void freeItems() override;
	virtual MessageQueue *method34(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;
	virtual MessageQueue *doWalkTo(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;

	MessageQueue *controllerWalkTo(StaticANIObject *ani, int off);

private:
	int findObjectPos(StaticANIObject *obj);
	bool initMovement(StaticANIObject *ani, MctlLadderMovement *movement);
};

class MovGraphNode : public CObject {
public:
	int _x;
	int _y;
	int _distance;
	int16 _field_10;
	int _field_14;

public:
	MovGraphNode() : _x(0), _y(0), _distance(0), _field_10(0), _field_14(0) { _objtype = kObjTypeMovGraphNode; }
	virtual bool load(MfcArchive &file) override;
};

class ReactParallel : public MovGraphReact {
	//CRgn _rgn;
	int _x1;
	int _y1;
	int _x2;
	int _y2;
	int _dx;
	int _dy;

public:
	ReactParallel();
	virtual bool load(MfcArchive &file) override;

	virtual void setCenter(int x1, int y1, int x2, int y2) override;
	virtual void createRegion() override;
};

class ReactPolygonal : public MovGraphReact {
	Common::Rect *_bbox;
	int _centerX;
	int _centerY;

public:
	ReactPolygonal();
	~ReactPolygonal();

	virtual bool load(MfcArchive &file) override;

	virtual void setCenter(int x1, int y1, int x2, int y2) override;
	virtual void createRegion() override;

	void getBBox(Common::Rect *rect);
};

class MovGraphLink : public CObject {
 public:
	MovGraphNode *_movGraphNode1;
	MovGraphNode *_movGraphNode2;
	DWordArray _dwordArray1;
	DWordArray _dwordArray2;
	int _flags;
	int _field_38;
	int _field_3C;
	double _distance;
	double _angle;
	MovGraphReact *_movGraphReact;
	char *_name;

  public:
	MovGraphLink();
	virtual ~MovGraphLink();

	virtual bool load(MfcArchive &file) override;

	void calcNodeDistanceAndAngle();
};

struct MovStep {
	int sfield_0;
	MovGraphLink *link;
};

struct MovArr {
	Common::Array<MovStep *> _movSteps;
	int _movStepCount;
	int _afield_8;
	MovGraphLink *_link;
	double _dist;
	Common::Point _point;
};

struct MovItem {
	MovArr *movarr;
	int _mfield_4;
	int _mfield_8;
	int _mfield_C;
};

struct MovGraphItem {
	StaticANIObject *ani;
	int field_4;
	MovArr movarr;
	Common::Array<MovItem *> *movitems;
	int count;
	int field_30;
	int field_34;
	int field_38;
	int field_3C;

	MovGraphItem();
	void free();
};

class MovGraph : public MotionController {
public:
	ObList _nodes;
	ObList _links;
	int _field_44;
	Common::Array<MovGraphItem *> _items;
	MovArr *(*_callback1)(StaticANIObject *ani, Common::Array<MovItem *> *items, signed int counter);
	MGM _mgm;

public:
	MovGraph();
	virtual ~MovGraph();

	virtual bool load(MfcArchive &file) override;

	virtual void addObject(StaticANIObject *obj) override;
	virtual int removeObject(StaticANIObject *obj) override;
	virtual void freeItems() override;
	virtual Common::Array<MovItem *> *method28(StaticANIObject *ani, int x, int y, int flag1, int *rescount) override;
	virtual bool method2C(StaticANIObject *obj, int x, int y) override;
	virtual MessageQueue *method34(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;
	virtual void changeCallback(MovArr *(*_callback1)(StaticANIObject *ani, Common::Array<MovItem *> *items, signed int counter)) override;
	virtual bool method3C(StaticANIObject *ani, int flag) override;
	virtual bool method44(StaticANIObject *ani, int x, int y) override;
	virtual MessageQueue *doWalkTo(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;
	virtual MessageQueue *method50(StaticANIObject *ani, MovArr *movarr, int staticsId);

	double calcDistance(Common::Point *point, MovGraphLink *link, int fuzzyMatch);
	void calcNodeDistancesAndAngles();
	bool findClosestLink(int unusedArg, Common::Point *p, MovArr *movarr);
	MovGraphNode *calcOffset(int ox, int oy);
	int getItemIndexByStaticAni(StaticANIObject *ani);
	Common::Array<MovArr *> *genMovArr(int x, int y, int *arrSize, int flag1, int flag2);
	void findAllPaths(MovGraphLink *lnk, MovGraphLink *lnk2, Common::Array<MovGraphLink *> &tempObList1, Common::Array<MovGraphLink *> &tempObList2);
	Common::Array<MovItem *> *calcMovItems(MovArr *movarr1, MovArr *movarr2, int *listCount);
	void genMovItem(MovItem *movitem, MovGraphLink *grlink, MovArr *movarr1, MovArr *movarr2);
	bool calcChunk(int idx, int x, int y, MovArr *arr, int a6);
	MessageQueue *sub1(StaticANIObject *ani, int x, int y, int a5, int x1, int y1, int a8, int a9);
	MessageQueue *fillMGMinfo(StaticANIObject *ani, MovArr *movarr, int staticsId);
	void setEnds(MovStep *step1, MovStep *step2);
};

class Movement;

struct MG2I {
	int _movementId;
	Movement *_mov;
	int _mx;
	int _my;
};

struct MovGraph2ItemSub {
	int _staticsId2;
	int _staticsId1;
	MG2I _walk[3];
	MG2I _turn[4];
	MG2I _turnS[4];
};

struct LinkInfo {
	MovGraphLink *link;
	MovGraphNode *node;
};

struct MovInfo1Sub {
	int subIndex;
	int x;
	int y;
	int distance;
};

struct MovInfo1 {
	int index;
	Common::Point pt1;
	Common::Point pt2;
	int distance1;
	int distance2;
	int subIndex;
	int item1Index;
	Common::Array<MovInfo1Sub *> items;
	int itemsCount;
	int flags;

	MovInfo1() { clear(); }
	MovInfo1(MovInfo1 *src);
	void clear();
};

struct MovGraph2Item { // 744
	int _objectId;
	StaticANIObject *_obj;
	MovGraph2ItemSub _subItems[4];  // 184
};

class MovGraph2 : public MovGraph {
public:
	Common::Array<MovGraph2Item *> _items2;

public:
	virtual void addObject(StaticANIObject *obj) override;
	virtual int removeObject(StaticANIObject *obj) override;
	virtual void freeItems() override;
	virtual MessageQueue *method34(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;
	virtual MessageQueue *doWalkTo(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;

	int getItemIndexByGameObjectId(int objectId);
	int getItemSubIndexByStaticsId(int index, int staticsId);
	int getItemSubIndexByMovementId(int index, int movId);
	int getItemSubIndexByMGM(int idx, StaticANIObject *ani);

	int getShortSide(MovGraphLink *lnk, int x, int y);
	int findLink(Common::Array<MovGraphLink *> *linkList, int idx, Common::Rect *a3, Common::Point *a4);

	bool initDirections(StaticANIObject *obj, MovGraph2Item *item);
	void buildMovInfo1SubItems(MovInfo1 *movinfo, Common::Array<MovGraphLink *> *linkList, LinkInfo *lnkSrc, LinkInfo *lnkDst);
	MessageQueue *buildMovInfo1MessageQueue(MovInfo1 *movInfo);

	MovGraphNode *findNode(int x, int y, int fuzzyMatch);
	MovGraphLink *findLink1(int x, int y, int idx, int fuzzyMatch);
	MovGraphLink *findLink2(int x, int y);
	double findMinPath(LinkInfo *linkInfoSource, LinkInfo *linkInfoDest, Common::Array<MovGraphLink *> *listObj);

	MessageQueue *genMovement(MovInfo1 *movinfo);
};

class MctlConnectionPoint : public CObject {
public:
	int _connectionX;
	int _connectionY;
	int _field_C;
	int _field_10;
	int16 _field_14;
	int16 _field_16;
	MessageQueue *_messageQueueObj;
	int _motionControllerObj;

	MctlConnectionPoint();
	~MctlConnectionPoint();
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_MOTION_H */
