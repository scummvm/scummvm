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

#ifndef FULLPIPE_MOTION_H
#define FULLPIPE_MOTION_H

namespace Fullpipe {

int startWalkTo(int objId, int objKey, int x, int y, int a5);
int doSomeAnimation(int objId, int objKey, int a3);
int doSomeAnimation2(int objId, int objKey);

class MotionController : public CObject {
public:
	int _field_4;
	bool _isEnabled;

public:
	MotionController() : _isEnabled(true), _field_4(0) {}
	virtual ~MotionController() {}
	virtual bool load(MfcArchive &file);
	virtual void methodC() {}
	virtual void method10() {}
	virtual void clearEnabled() { _isEnabled = false; }
	virtual void setEnabled() { _isEnabled = true; }
	virtual void addObject(StaticANIObject *obj) {}
	virtual int removeObject(StaticANIObject *obj) { return 0; }
	virtual void freeItems() {}
	virtual int method28() { return 0; }
	virtual int method2C() { return 0; }
	virtual int method30() { return 0; }
	virtual MessageQueue *method34(StaticANIObject *subj, int xpos, int ypos, int flag, int staticsId) { return 0; }
	virtual int changeCallback() { return 0; }
	virtual int method3C() { return 0; }
	virtual int method40() { return 0; }
	virtual int method44() { return 0; }
	virtual int method48() { return -1; }
	virtual MessageQueue *method4C(StaticANIObject *subj, int xpos, int ypos, int flag, int staticsId) { return 0; }
};

class MovGraphReact : public CObject {
	// Empty
};

typedef Common::Array<CObject> MctlConnectionPointsArray;

class MctlCompoundArrayItem : public CObject {
	friend class MctlCompound;

  protected:
	MotionController *_motionControllerObj;
	MovGraphReact *_movGraphReactObj;
	MctlConnectionPointsArray _connectionPoints;
	int _field_20;
	int _field_24;
	int _field_28;

 public:
	MctlCompoundArrayItem() : _movGraphReactObj(0) {}
};

class MctlCompoundArray : public Common::Array<MctlCompoundArrayItem *>, public CObject {
 public:
	virtual bool load(MfcArchive &file);
};

class MctlCompound : public MotionController {
	MctlCompoundArray _motionControllers;

 public:
	MctlCompound() { _objtype = kObjTypeMctlCompound; }

	virtual bool load(MfcArchive &file);

	virtual void addObject(StaticANIObject *obj);
	virtual int removeObject(StaticANIObject *obj);
	virtual void freeItems();
	virtual MessageQueue *method34(StaticANIObject *subj, int xpos, int ypos, int flag, int staticsId);
	virtual MessageQueue *method4C(StaticANIObject *subj, int xpos, int ypos, int flag, int staticsId);

	void initMovGraph2();
};

class MGM : public CObject {
public:
	int _items;
	int _count;

public:
	MGM() : _items(0), _count(0) {}
};

class MovGraphNode : public CObject {
 public:
	int _x;
	int _y;
	int _distance;
	int16 _field_10;
	int _field_14;

  public:
	MovGraphNode() : _x(0), _y(0), _distance(0), _field_10(0), _field_14(0) {}
	virtual bool load(MfcArchive &file);
};

class ReactParallel : public MovGraphReact {
	//CRgn _rgn;
	int _x1;
	int _y1;
	int _x2;
	int _y2;
	int _dx;
	int _dy;
	Common::Point **_points;

  public:
	ReactParallel();
	virtual bool load(MfcArchive &file);
	void createRegion();
};

class ReactPolygonal : public MovGraphReact {
	//CRgn _rgn;
	int _field_C;
	int _field_10;
	int _pointCount;
	Common::Point **_points;

  public:
	ReactPolygonal();
	virtual bool load(MfcArchive &file);
	void createRegion();
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
	virtual bool load(MfcArchive &file);
};

class MovGraph : public MotionController {
 public:
	ObList _nodes;
	ObList _links;
	int _field_44;
	int _items;
	int _itemsCount;
	int (*_callback1)(int, int, int);
	MGM _mgm;

 public:
	MovGraph();
	virtual bool load(MfcArchive &file);

	virtual void addObject(StaticANIObject *obj);
	virtual int removeObject(StaticANIObject *obj);
	virtual void freeItems();
	virtual int method28();
	virtual int method2C();
	virtual MessageQueue *method34(StaticANIObject *subj, int xpos, int ypos, int flag, int staticsId);
	virtual int changeCallback();
	virtual int method3C();
	virtual int method44();
	virtual MessageQueue *method4C(StaticANIObject *subj, int xpos, int ypos, int flag, int staticsId);
	virtual int method50();

	double calcDistance(Common::Point *point, MovGraphLink *link, int flag);
	MovGraphNode *calcOffset(int ox, int oy);
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

struct MovGraph2Item {
	int _objectId;
	StaticANIObject *_obj;
	MovGraph2ItemSub _subItems[4];
};

class MovGraph2 : public MovGraph {
public:
	Common::Array<MovGraph2Item *> _items;

public:
	virtual void addObject(StaticANIObject *obj);
	virtual int removeObject(StaticANIObject *obj);
	virtual void freeItems();
	virtual MessageQueue *method34(StaticANIObject *subj, int xpos, int ypos, int flag, int staticsId);
	virtual MessageQueue *method4C(StaticANIObject *subj, int xpos, int ypos, int flag, int staticsId);

	int getItemIndexByGameObjectId(int objectId);
	bool initDirections(StaticANIObject *obj, MovGraph2Item *item);
};

class MctlConnectionPoint : public CObject {
public:
	int _connectionX;
	int _connectionY;
	int _field_C;
	int _field_10;
	int16 _field_14;
	int16 _field_16;
	int _messageQueueObj;
	int _motionControllerObj;
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_MOTION_H */
