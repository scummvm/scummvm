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

#ifndef NGI_MOTION_H
#define NGI_MOTION_H

#include "ngi/anihandler.h"

namespace NGI {

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
	~MotionController() override {}
	bool load(MfcArchive &file) override;
	virtual void methodC() {}
	virtual void method10() {}
	virtual void deactivate() { _isEnabled = false; }
	virtual void activate() { _isEnabled = true; }
	virtual void attachObject(StaticANIObject *obj) {}
	virtual int detachObject(StaticANIObject *obj) { return 0; }
	virtual void detachAllObjects() {}
	virtual Common::Array<MovItem *> *getPaths(StaticANIObject *ani, int x, int y, int flag1, int *rescount) { return 0; }
	virtual bool setPosImmediate(StaticANIObject *obj, int x, int y) { return false; }
	virtual int method30() { return 0; }
	virtual MessageQueue *startMove(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) { return 0; }
	virtual void setSelFunc(MovArr *(*_callback1)(StaticANIObject *ani, Common::Array<MovItem *> *items, signed int counter)) {}
	virtual bool resetPosition(StaticANIObject *ani, int flag) { return 0; }
	virtual int method40() { return 0; }
	virtual bool canDropInventory(StaticANIObject *ani, int x, int y) { return false; }
	virtual int method48() { return -1; }
	virtual MessageQueue *makeQueue(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) { return 0; }

	void enableLinks(const char *linkName, bool enable);
	MovGraphLink *getLinkByName(const char *name);
};

class MovGraphReact : public CObject {
public:
	PointList _points;

public:
	virtual void setCenter(int x1, int y1, int x2, int y2) {}
	virtual void createRegion() {}
	virtual bool pointInRegion(int x, int y);
};

class MctlItem : public CObject {
public:
	Common::ScopedPtr<MotionController> _motionControllerObj;
	Common::ScopedPtr<MovGraphReact> _movGraphReactObj;
	Common::Array<MctlConnectionPoint *> _connectionPoints;
	int _field_20;
	int _field_24;
	int _field_28;

public:
	MctlItem() : _field_20(0), _field_24(0), _field_28(0) {}
	~MctlItem() override;
};

class MctlCompound : public MotionController {
public:
	/** list items are owned */
	Common::Array<MctlItem *> _motionControllers;

	MctlCompound() { _objtype = kObjTypeMctlCompound; }
	~MctlCompound() override;

	bool load(MfcArchive &file) override;

	void attachObject(StaticANIObject *obj) override;
	int detachObject(StaticANIObject *obj) override;
	void detachAllObjects() override;
	MessageQueue *startMove(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;
	MessageQueue *makeQueue(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;

	void initMctlGraph();
	MctlConnectionPoint *findClosestConnectionPoint(int ox, int oy, int destIndex, int connectionX, int connectionY, int sourceIndex, double *minDistancePtr);
	void replaceNodeX(int from, int to);

	uint getMotionControllerCount() { return _motionControllers.size(); }
	MotionController *getMotionController(int num) { return _motionControllers[num]->_motionControllerObj.get(); }
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
	AniHandler _aniHandler;

public:
	MctlLadder();
	~MctlLadder() override;
	int collisionDetection(StaticANIObject *man);

	void attachObject(StaticANIObject *obj) override;
	int detachObject(StaticANIObject *obj) override { return 1; }
	void detachAllObjects() override;
	MessageQueue *startMove(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;
	MessageQueue *makeQueue(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;

	MessageQueue *controllerWalkTo(StaticANIObject *ani, int off);

private:
	int findObjectPos(StaticANIObject *obj);
	bool initMovement(StaticANIObject *ani, MctlLadderMovement *movement);
};

class MovGraphNode : public CObject {
public:
	int _x;
	int _y;
	int _z;
	int16 _field_10;
	int _field_14;

public:
	MovGraphNode() : _x(0), _y(0), _z(0), _field_10(0), _field_14(0) { _objtype = kObjTypeMovGraphNode; }
	bool load(MfcArchive &file) override;
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
	bool load(MfcArchive &file) override;

	void setCenter(int x1, int y1, int x2, int y2) override;
	void createRegion() override;
};

class ReactPolygonal : public MovGraphReact {
	Common::Rect _bbox;
	int _centerX;
	int _centerY;

public:
	ReactPolygonal();

	bool load(MfcArchive &file) override;

	void setCenter(int x1, int y1, int x2, int y2) override;
	void createRegion() override;

	Common::Rect getBBox();
};

class MovGraphLink : public CObject {
 public:
	MovGraphNode *_graphSrc;
	MovGraphNode *_graphDst;
	DWordArray _dwordArray1;
	DWordArray _dwordArray2;
	uint32 _flags;
	int _field_38;
	int _field_3C;
	double _length;
	double _angle;
	MovGraphReact *_movGraphReact;
	Common::String _name;

  public:
	MovGraphLink();
	~MovGraphLink() override;

	bool load(MfcArchive &file) override;

	void recalcLength();
};
typedef Common::Array<MovGraphLink *> MovGraphLinkList;

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
	Common::Array<MovItem *> *mi_movitems;
	int count;
	int field_30;
	int field_34;
	int field_38;
	int field_3C;

	MovGraphItem();
	void free();
};

class MovGraph : public MotionController {
friend class MctlCompound;
friend class MctlGraph;
friend class MotionController;
private:
	typedef ObList<MovGraphNode> NodeList;
	typedef ObList<MovGraphLink> LinkList;
	NodeList _nodes;
	LinkList _links;
	int _field_44;
	Common::Array<MovGraphItem> _items;
	MovArr *(*_callback1)(StaticANIObject *ani, Common::Array<MovItem *> *items, signed int counter);
	AniHandler _aniHandler;

public:
	MovGraph();
	~MovGraph() override;

	static int messageHandler(ExCommand *cmd);

	bool load(MfcArchive &file) override;

	void attachObject(StaticANIObject *obj) override;
	int detachObject(StaticANIObject *obj) override;
	void detachAllObjects() override;
	Common::Array<MovItem *> *getPaths(StaticANIObject *ani, int x, int y, int flag1, int *rescount) override;
	bool setPosImmediate(StaticANIObject *obj, int x, int y) override;
	MessageQueue *startMove(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;
	void setSelFunc(MovArr *(*_callback1)(StaticANIObject *ani, Common::Array<MovItem *> *items, signed int counter)) override;
	bool resetPosition(StaticANIObject *ani, int flag) override;
	bool canDropInventory(StaticANIObject *ani, int x, int y) override;
	MessageQueue *makeQueue(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;
	virtual MessageQueue *method50(StaticANIObject *ani, MovArr *movarr, int staticsId);

	double putToLink(Common::Point *point, MovGraphLink *link, int fuzzyMatch);
	void recalcLinkParams();
	bool getNearestPoint(int unusedArg, Common::Point *p, MovArr *movarr);
	MovGraphNode *calcOffset(int ox, int oy);
	int getObjectIndex(StaticANIObject *ani);
	Common::Array<MovArr *> *getHitPoints(int x, int y, int *arrSize, int flag1, int flag2);
	void findAllPaths(MovGraphLink *lnk, MovGraphLink *lnk2, MovGraphLinkList &tempObList1, MovGraphLinkList &tempObList2);
	Common::Array<MovItem *> *getPaths(MovArr *movarr1, MovArr *movarr2, int *listCount);
	void genMovItem(MovItem *movitem, MovGraphLink *grlink, MovArr *movarr1, MovArr *movarr2);
	bool getHitPoint(int idx, int x, int y, MovArr *arr, int a6);
	MessageQueue *sub1(StaticANIObject *ani, int x, int y, int a5, int x1, int y1, int a8, int a9);
	MessageQueue *makeWholeQueue(StaticANIObject *ani, MovArr *movarr, int staticsId);
	void setEnds(MovStep *step1, MovStep *step2);
};

class Movement;

struct MG2I {
	int _movementId;
	Movement *_mov;
	int _mx;
	int _my;
};

struct MctlAniSub {
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

struct MctlMQSub {
	int subIndex;
	int x;
	int y;
	int distance;
};

struct MctlMQ {
	int index;
	Common::Point pt1;
	Common::Point pt2;
	int distance1;
	int distance2;
	int subIndex;
	int item1Index;
	Common::Array<MctlMQSub> items;
	int flags;

	MctlMQ() { clear(); }
	void clear();
};

struct MctlAni { // 744
	int _objectId;
	StaticANIObject *_obj;
	MctlAniSub _subItems[4];  // 184
};

class MctlGraph : public MovGraph {
public:
	Common::Array<MctlAni> _items2;

public:
	void attachObject(StaticANIObject *obj) override;
	int detachObject(StaticANIObject *obj) override;
	void detachAllObjects() override;
	MessageQueue *startMove(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;
	MessageQueue *makeQueue(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;

	int getObjIndex(int objectId);
	int getDirByStatics(int index, int staticsId);
	int getDirByMovement(int index, int movId);
	int getDirByPoint(int idx, StaticANIObject *ani);

	int getDirBySize(MovGraphLink *lnk, int x, int y);
	int getLinkDir(MovGraphLinkList *linkList, int idx, Common::Rect *a3, Common::Point *a4);

	bool fillData(StaticANIObject *obj, MctlAni &item);
	void generateList(MctlMQ &movinfo, MovGraphLinkList *linkList, LinkInfo *lnkSrc, LinkInfo *lnkDst);
	MessageQueue *makeWholeQueue(MctlMQ &mctlMQ);

	MovGraphNode *getHitNode(int x, int y, int strictMatch);
	MovGraphLink *getHitLink(int x, int y, int idx, int fuzzyMatch);
	MovGraphLink *getNearestLink(int x, int y);
	double iterate(LinkInfo *linkInfoSource, LinkInfo *linkInfoDest, MovGraphLinkList *listObj);

	MessageQueue *makeLineQueue(MctlMQ *movinfo);
};

class MctlConnectionPoint : public CObject {
public:
	int _connectionX;
	int _connectionY;
	int _mctlflags;
	int _mctlstatic;
	int16 _mctlmirror;
	Common::ScopedPtr<MessageQueue> _messageQueueObj;
	int _motionControllerObj;

	MctlConnectionPoint();
};

} // End of namespace NGI

#endif /* NGI_MOTION_H */
