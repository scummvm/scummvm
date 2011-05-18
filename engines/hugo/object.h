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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef HUGO_OBJECT_H
#define HUGO_OBJECT_H

#include "common/file.h"

namespace Hugo {

struct target_t {                                   // Secondary target for action
	uint16 nounIndex;                               // Secondary object
	uint16 verbIndex;                               // Action on secondary object
};

struct uses_t {                                     // Define uses of certain objects
	int16     objId;                                // Primary object
	uint16    dataIndex;                            // String if no secondary object matches
	target_t *targets;                              // List of secondary targets
};

class ObjectHandler {
public:
	ObjectHandler(HugoEngine *vm);
	virtual ~ObjectHandler();

	overlay_t _objBound;
	overlay_t _boundary;                            // Boundary overlay file
	overlay_t _overlay;                             // First overlay file
	overlay_t _ovlBase;                             // First overlay base file

	object_t  *_objects;
	uint16    _numObj;

	byte getBoundaryOverlay(uint16 index) const;
	byte getObjectBoundary(uint16 index) const;
	byte getBaseBoundary(uint16 index) const;
	byte getFirstOverlay(uint16 index) const;

	int  deltaX(const int x1, const int x2, const int vx, int y) const;
	int  deltaY(const int x1, const int x2, const int vy, const int y) const;
	void boundaryCollision(object_t *obj);
	void clearBoundary(const int x1, const int x2, const int y);
	void clearScreenBoundary(const int x1, const int x2, const int y);
	void storeBoundary(const int x1, const int x2, const int y);

	virtual void homeIn(const int objIndex1, const int objIndex2, const int8 objDx, const int8 objDy) = 0;
	virtual void moveObjects() = 0;
	virtual void updateImages() = 0;
	virtual void swapImages(int objIndex1, int objIndex2) = 0;

	bool isCarrying(uint16 wordIndex);
	bool findObjectSpace(object_t *obj, int16 *destx, int16 *desty);

	int   calcMaxScore();
	int16 findObject(uint16 x, uint16 y);
	void freeObjects();
	void loadObjectArr(Common::ReadStream &in);
	void loadObjectUses(Common::ReadStream &in);
	void loadNumObj(Common::ReadStream &in);
	void lookObject(object_t *obj);
	void readObjectImages();
	void readObject(Common::ReadStream &in, object_t &curObject);
	void readUse(Common::ReadStream &in, uses_t &curUse);
	void restoreAllSeq();
	void restoreObjects(Common::SeekableReadStream *in);
	void saveObjects(Common::WriteStream *out);
	void saveSeq(object_t *obj);
	void setCarriedScreen(int screenNum);
	void showTakeables();
	void useObject(int16 objId);

	static int y2comp(const void *a, const void *b);

	bool isCarried(int objIndex) const;
	void setCarry(int objIndex, bool val);
	void setVelocity(int objIndex, int8 vx, int8 vy);
	void setPath(int objIndex, path_t pathType, int16 vxPath, int16 vyPath);

protected:
	HugoEngine *_vm;

	static const int kEdge = 10;                    // Closest object can get to edge of screen
	static const int kEdge2 = kEdge * 2;            // Push object further back on edge collision
	static const int kMaxObjNumb = 128;             // Used in Update_images()

	uint16     _objCount;
	uses_t    *_uses;
	uint16     _usesSize;

	void restoreSeq(object_t *obj);

	inline bool checkBoundary(int16 x, int16 y);
	template <typename T>
	inline int sign(T a) { if ( a < 0) return -1; else return 1; }
};

class ObjectHandler_v1d : public ObjectHandler {
public:
	ObjectHandler_v1d(HugoEngine *vm);
	virtual ~ObjectHandler_v1d();

	virtual void homeIn(const int objIndex1, const int objIndex2, const int8 objDx, const int8 objDy);
	virtual void moveObjects();
	virtual void updateImages();
	virtual void swapImages(int objIndex1, int objIndex2);
};

class ObjectHandler_v2d : public ObjectHandler_v1d {
public:
	ObjectHandler_v2d(HugoEngine *vm);
	virtual ~ObjectHandler_v2d();

	virtual void moveObjects();
	virtual void updateImages();

	void homeIn(const int objIndex1, const int objIndex2, const int8 objDx, const int8 objDy);
};

class ObjectHandler_v3d : public ObjectHandler_v2d {
public:
	ObjectHandler_v3d(HugoEngine *vm);
	~ObjectHandler_v3d();

	virtual void moveObjects();
	virtual void swapImages(int objIndex1, int objIndex2);
};

class ObjectHandler_v1w : public ObjectHandler_v3d {
public:
	ObjectHandler_v1w(HugoEngine *vm);
	~ObjectHandler_v1w();

	void moveObjects();
	void updateImages();
	void swapImages(int objIndex1, int objIndex2);
};

} // End of namespace Hugo
#endif //HUGO_OBJECT_H
