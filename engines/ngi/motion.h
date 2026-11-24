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

#ifndef NGI_MOTION_H
#define NGI_MOTION_H

#include "ngi/anihandler.h"

namespace NGI {

class MctlConnectionPoint;
class MovGraphLink;
class MessageQueue;
struct GrPathDesc;
struct PathDesc;

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
	virtual void attachObject(AniObject *obj) {}
	virtual int detachObject(AniObject *obj) { return 0; }
	virtual void detachAllObjects() {}
	virtual Common::Array<PathDesc *> *getPaths(AniObject *ani, int x, int y, int flag1, int *rescount) { return 0; }
	virtual bool setPosImmediate(AniObject *obj, int x, int y) { return false; }
	virtual int method30() { return 0; }
	virtual MessageQueue *startMove(AniObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) { return 0; }
	virtual void setSelFunc(GrPathDesc *(*_callback1)(AniObject *ani, Common::Array<PathDesc *> *items, signed int counter)) {}
	virtual bool resetPosition(AniObject *ani, int flag) { return 0; }
	virtual int checkCurrentPath() { return 0; }
	virtual bool canDropInventory(AniObject *ani, int x, int y) { return false; }
	virtual bool checkPoint(int x, int y) { return false; }
	virtual int getStopStatics() { return -1; }
	virtual MessageQueue *makeQueue(AniObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) { return 0; }

	void lockLink(const char *linkName, bool enable);
	MovGraphLink *getGraphLink(const char *name);
};

class MovGraphReact : public CObject {
public:
	PointList _points;

public:
	virtual void setAnchorPoints(int x1, int y1, int x2, int y2) {}
	virtual void create() {}
	virtual bool hitTest(int x, int y);
};

class MctlItem : public CObject {
public:
	Common::ScopedPtr<MotionController> _motionControllerObj;
	Common::ScopedPtr<MovGraphReact> _movGraphReactObj;
	Common::Array<MctlConnectionPoint *> _connectionPoints;
	int _ofsX;
	int _ofsY;
	int _ofsZ;

public:
	MctlItem() : _ofsX(0), _ofsY(0), _ofsZ(0) {}
	~MctlItem() override;
};

class MctlCompound : public MotionController {
public:
	/** list items are owned */
	Common::Array<MctlItem *> _motionControllers;

	MctlCompound() { _objtype = kObjTypeMctlCompound; }
	~MctlCompound() override;

	bool load(MfcArchive &file) override;
	bool checkPoint(int x, int y) override;

	void attachObject(AniObject *obj) override;
	int detachObject(AniObject *obj) override;
	void detachAllObjects() override;
	MessageQueue *startMove(AniObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;
	MessageQueue *makeQueue(AniObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;

	void initMctlGraph();
	MctlConnectionPoint *getNextPoint(int ox, int oy, int destIndex, int connectionX, int connectionY, int sourceIndex, double *minDistancePtr);
	void replaceNodeX(int from, int to);

	uint getNumChildren() { return _motionControllers.size(); }
	MotionController *getChild(int num) { return _motionControllers[num]->_motionControllerObj.get(); }
};

struct MctlLadderMovementVars {
	int upGoId;
	int downGoId;
	int upStopId;
	int downStopId;
	int upStartId;
	int downStartId;
};

struct LmctlAni {
	int aniId;
	int staticIdCount;
	MctlLadderMovementVars *movementIds;
	int *staticIds;
};

class MctlLadder : public MotionController {
public:
	int _x;
	int _y;
	int _z;
	int _stepX;
	int _stepY;
	int _entryOffsetX;
	int _entryOffsetY;
	Common::Array<LmctlAni *> _ladmovements;
	AniHandler _aniHandler;

public:
	MctlLadder();
	~MctlLadder() override;
	int getCurrentStep(AniObject *man);

	void attachObject(AniObject *obj) override;
	int detachObject(AniObject *obj) override { return 1; }
	void detachAllObjects() override;
	MessageQueue *startMove(AniObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;
	MessageQueue *makeQueue(AniObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;

	MessageQueue *makeQueue(AniObject *ani, int off);

private:
	int findIndex(AniObject *obj);
	bool fillData(AniObject *ani, LmctlAni *movement);
};

class MovGraphNode : public CObject {
public:
	int _id = 0;
	int _x;
	int _y;
	int _z;
	int16 _field_10;
	int _flags;

public:
	MovGraphNode() : _x(0), _y(0), _z(0), _field_10(0), _flags(0) { _objtype = kObjTypeMovGraphNode; }
	bool load(MfcArchive &file) override;
};

class ReactParallel : public MovGraphReact {
	//CRgn _rgn;
	int _x1;
	int _y1;
	int _x2;
	int _y2;
	int _left;
	int _right;

public:
	ReactParallel(int left = 0, int right = 0);
	bool load(MfcArchive &file) override;

	void setAnchorPoints(int x1, int y1, int x2, int y2) override;
	void create() override;
};

class ReactPolygonal : public MovGraphReact {
	Common::Rect _bbox;
	int _centerX;
	int _centerY;

public:
	ReactPolygonal(int centerX = 0, int centerY = 0);

	bool load(MfcArchive &file) override;

	void setAnchorPoints(int x1, int y1, int x2, int y2) override;
	void create() override;

	Common::Rect getBBox();
};

class MovGraphLink : public CObject {
 public:
	MovGraphNode *_graphSrc;
	MovGraphNode *_graphDst;
	DWordArray _movementIds;
	DWordArray _staticsIds;
	// XML stores the facing separately from the movement or statics ID.
	Common::Array<bool> _movementMirrors;
	Common::Array<bool> _staticsMirrors;
	uint32 _flags;
	int _field_38;
	int _field_3C;
	double _length;
	double _angle;
	MovGraphReact *_movGraphReact;
	Common::String _title;

  public:
	MovGraphLink();
	~MovGraphLink() override;

	bool load(MfcArchive &file) override;

	void recalcLength();
};
typedef Common::Array<MovGraphLink *> MovGraphLinkList;

struct MovStep {
	int reverse;
	MovGraphLink *link;
};

struct GrPathDesc {
	Common::Array<MovStep *> _steps;
	int _stepCount;
	int _afield_8;
	MovGraphLink *_link;
	double _linkFraction;
	Common::Point _position;
};

struct PathDesc {
	GrPathDesc *path;
	int _length;
	int _mfield_8;
	int _mfield_C;
};

struct NavDatum {
	AniObject *ani;
	int field_4;
	GrPathDesc currentPath;
	Common::Array<PathDesc *> *candidatePaths;
	int candidatePathCount;
	int field_30;
	int field_34;
	int field_38;
	int field_3C;

	NavDatum();
	void clearPathList();
};

class MovGraph : public MotionController {
friend class MctlCompound;
friend class MctlGraph;
friend class MotionController;
public:
	typedef ObList<MovGraphNode> NodeList;
	typedef ObList<MovGraphLink> LinkList;
	NodeList _nodes;
	LinkList _links;

private:
	int _linkItemOffset;
	Common::Array<NavDatum> _items;
	GrPathDesc *(*_selFunc)(AniObject *ani, Common::Array<PathDesc *> *items, signed int counter);
	AniHandler _aniHandler;

public:
	MovGraph();
	~MovGraph() override;

	static int messageHandler(Command *cmd);

	bool load(MfcArchive &file) override;

	void attachObject(AniObject *obj) override;
	int detachObject(AniObject *obj) override;
	void detachAllObjects() override;
	Common::Array<PathDesc *> *getPaths(AniObject *ani, int x, int y, int flag1, int *rescount) override;
	bool setPosImmediate(AniObject *obj, int x, int y) override;
	MessageQueue *startMove(AniObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;
	void setSelFunc(GrPathDesc *(*_selFunc)(AniObject *ani, Common::Array<PathDesc *> *items, signed int counter)) override;
	bool resetPosition(AniObject *ani, int flag) override;
	bool canDropInventory(AniObject *ani, int x, int y) override;
	MessageQueue *makeQueue(AniObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;
	virtual MessageQueue *method50(AniObject *ani, GrPathDesc *movarr, int staticsId);

	double putToLink(Common::Point *point, MovGraphLink *link, int fuzzyMatch);
	void recalcLinkParams();
	bool getNearestPoint(int unusedArg, Common::Point *p, GrPathDesc *movarr);
	MovGraphNode *findNearestNode(int ox, int oy);
	MovGraphNode *getNodeById(int id) const;
	int getObjectIndex(AniObject *ani);
	Common::Array<GrPathDesc *> *getHitPoints(int x, int y, int *arrSize, int flag1, int flag2);
	void findAllPaths(MovGraphLink *lnk, MovGraphLink *lnk2, MovGraphLinkList &tempObList1, MovGraphLinkList &tempObList2);
	Common::Array<PathDesc *> *getPaths(GrPathDesc *movarr1, GrPathDesc *movarr2, int *listCount);
	void genMovItem(PathDesc *movitem, MovGraphLink *grlink, GrPathDesc *movarr1, GrPathDesc *movarr2);
	bool getHitPoint(int idx, int x, int y, GrPathDesc *arr, int a6);
	MessageQueue *sub1(AniObject *ani, int x, int y, int a5, int x1, int y1, int a8, int a9);
	MessageQueue *makeWholeQueue(AniObject *ani, GrPathDesc *movarr, int staticsId);
	void setEnds(MovStep *step1, MovStep *step2);
};

class Movement;

struct MG2I {
	int _movementId;
	Movement *_movement;
	int _deltaX;
	int _deltaY;
};

struct MctlAniSub {
	int _nextStaticId;
	int _prevStaticId;
	MG2I _walk[3];
	MG2I _turn[4];
	MG2I _turnS[4];
};

struct LinkOrNode {
	MovGraphLink *link;
	MovGraphNode *node;
};

struct GmctlPathItem {
	int direction;
	int x;
	int y;
	int z;
};

struct GmctlMqStruct {
	int objectIndex;
	Common::Point sourcePos;
	Common::Point destPos;
	int sourceZ;
	int destZ;
	int sourceDir;
	int destDir;
	Common::Array<GmctlPathItem> items;
	int flags;

	GmctlMqStruct() { clear(); }
	void clear();
};

struct GmctlAni { // 744
	int _objectId;
	AniObject *_ani;
	MctlAniSub _directions[4];  // 184
};

class MctlGraph : public MovGraph {
public:
	Common::Array<GmctlAni> _aniData;

public:
	bool checkPoint(int x, int y) override;
	void attachObject(AniObject *obj) override;
	int detachObject(AniObject *obj) override;
	void detachAllObjects() override;
	MessageQueue *startMove(AniObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;
	MessageQueue *makeQueue(AniObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) override;

	int getObjIndex(int objectId);
	int getDirByStatics(int index, int staticsId);
	int getDirByMovement(int index, int movId);
	int getDirByPoint(int idx, AniObject *ani);

	int getDirBySize(MovGraphLink *lnk, int x, int y);
	int getLinkDir(MovGraphLinkList *linkList, int idx, Common::Rect *a3, Common::Point *a4);

	bool fillData(AniObject *obj, GmctlAni &item);
	void generateList(GmctlMqStruct &movinfo, MovGraphLinkList *linkList, LinkOrNode *lnkSrc, LinkOrNode *lnkDst);
	MessageQueue *makeWholeQueue(GmctlMqStruct &mctlMQ);

	MovGraphNode *getHitNode(int x, int y, int strictMatch);
	MovGraphLink *getHitLink(int x, int y, int idx, int fuzzyMatch);
	MovGraphLink *getNearestLink(int x, int y);
	double iterate(LinkOrNode *linkInfoSource, LinkOrNode *linkInfoDest, MovGraphLinkList *listObj);

	MessageQueue *makeLineQueue(GmctlMqStruct *movinfo);
};

class MctlConnectionPoint : public CObject {
public:
	int _x;
	int _y;
	int _flags;
	int _static;
	int16 _mirror;
	Common::ScopedPtr<MessageQueue> _messageQueueObj;
	int _motionControllerObj;

	MctlConnectionPoint();
};

} // End of namespace NGI

#endif /* NGI_MOTION_H */
