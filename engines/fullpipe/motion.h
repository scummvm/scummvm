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
	MotionController() : _isEnabled(true) {}
	virtual bool load(MfcArchive &file);

	void setEnabled() { _isEnabled = true; }
	void clearEnabled() { _isEnabled = false; }

	virtual void addObject(StaticANIObject *obj) {}
	virtual void freeItems() {}
};

class MctlCompoundArray : public Common::Array<CObject>, public CObject {
 public:
	virtual bool load(MfcArchive &file);
};

class MctlConnectionPointsArray : public Common::Array<CObject>, public CObject {
 public:
	virtual bool load(MfcArchive &file);
};

class MctlCompound : public MotionController {
	MctlCompoundArray _motionControllers;

 public:
	virtual bool load(MfcArchive &file);

	virtual void addObject(StaticANIObject *obj);
	void initMovGraph2();

	MessageQueue *method34(StaticANIObject *subj, int xpos, int ypos, int flag, int staticsId);
	MessageQueue *method4C(StaticANIObject *subj, int xpos, int ypos, int flag, int staticsId);
};

class Unk2 : public CObject {
	int _items;
	int _count;

 public:
	Unk2() : _items(0), _count(0) {}
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

class MovGraphReact : public CObject {
	// Empty
};

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
	Unk2 _unk2;

 public:
	MovGraph();
	virtual bool load(MfcArchive &file);

	virtual void addObject(StaticANIObject *obj);

	double calcDistance(Common::Point *point, MovGraphLink *link, int flag);
	MovGraphNode *calcOffset(int ox, int oy);
};

class MctlConnectionPoint : public CObject {
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
