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

class CMotionController : public CObject {
	int _field_4;
	int _isEnabled;

 public:
	CMotionController() : _isEnabled(1) {}
	virtual bool load(MfcArchive &file);
};

class CMctlCompoundArray : public Common::Array<CObject>, public CObject {
 public:
	virtual bool load(MfcArchive &file);
};

class CMctlConnectionPointsArray : public Common::Array<CObject>, public CObject {
 public:
	virtual bool load(MfcArchive &file);
};

class CMctlCompound : public CMotionController {
	CMctlCompoundArray _motionControllers;

 public:
	virtual bool load(MfcArchive &file);
};

class Unk2 : public CObject {
	int _items;
	int _count;

 public:
	Unk2() : _items(0), _count(0) {}
};

class CMovGraphNode : public CObject {
	int _x;
	int _y;
	int _distance;
	int16 _field_10;
	int16 _field_12;
	int _field_14;

  public:
	CMovGraphNode() : _x(0), _y(0), _distance(0), _field_10(0), _field_14(0) {}
	virtual bool load(MfcArchive &file);
};

class CMovGraphReact : public CObject {
	// Empty
};

class CMctlCompoundArrayItem : public CMotionController {
	friend class CMctlCompound;

  protected:
	CMovGraphReact *_movGraphReactObj;
	CMctlConnectionPointsArray _connectionPoints;
	int _field_20;
	int _field_24;
	int _field_28;

 public:
	CMctlCompoundArrayItem() : _movGraphReactObj(0) {}
};

class CReactParallel : public CMovGraphReact {
	//CRgn _rgn;
	int _x1;
	int _y1;
	int _x2;
	int _y2;
	int _dx;
	int _dy;
	Common::Point **_points;

  public:
	CReactParallel();
	virtual bool load(MfcArchive &file);
	void createRegion();
};

class CReactPolygonal : public CMovGraphReact {
	//CRgn _rgn;
	int _field_C;
	int _field_10;
	int _pointCount;
	Common::Point **_points;

  public:
	CReactPolygonal();
	virtual bool load(MfcArchive &file);
	void createRegion();
};

class CMovGraphLink : public CObject {
	CMovGraphNode *_movGraphNode1;
	CMovGraphNode *_movGraphNode2;
	CDWordArray _dwordArray1;
	CDWordArray _dwordArray2;
	int _flags;
	int _field_38;
	int _field_3C;
	double _distance;
	double _angle;
	CMovGraphReact *_movGraphReact;
	char *_name;

  public:
	CMovGraphLink();
	virtual bool load(MfcArchive &file);
};

class CMovGraph : public CMotionController {
	CObList _nodes;
	CObList _links;
	int _field_44;
	int _items;
	int _itemsCount;
	int (*_callback1)(int, int, int);
	Unk2 _unk2;

 public:
	CMovGraph();
	virtual bool load(MfcArchive &file);
};

class CMctlConnectionPoint : public CObject {
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
