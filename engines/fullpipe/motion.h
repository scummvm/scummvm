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

class CMctlCompoundArrayItem : public CObject {
  int _motionControllerObj;
  int _movGraphReactObj;
  CMctlConnectionPointsArray _connectionPoints;
  int _field_20;
  int _field_24;
  int _field_28;

 public:
	CMctlCompoundArrayItem() : _movGraphReactObj(0), _motionControllerObj(0) {}
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

class CMovGraphLink : public CObject {
  int movGraphNode1;
  int movGraphNode2;
  int dwordArray1;
  int field_10;
  int field_14;
  int field_18;
  int field_1C;
  int dwordArray2;
  int field_24;
  int field_28;
  int field_2C;
  int field_30;
  int flags;
  int field_38;
  int field_3C;
  double distance;
  double angle;
  int movGraphReact;
  int name;
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

} // End of namespace Fullpipe

#endif /* FULLPIPE_MOTION_H */
