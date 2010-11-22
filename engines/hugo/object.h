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
 * $URL$
 * $Id$
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

#define MAXOBJECTS      128                         // Used in Update_images()
#define BOUND(X, Y)     ((_vm->getBoundaryOverlay()[Y * XBYTES + X / 8] & (0x80 >> X % 8)) != 0)  // Boundary bit set

namespace Hugo {

class ObjectHandler {
public:
	ObjectHandler(HugoEngine *vm);
	virtual ~ObjectHandler();

	object_t  *_objects;
	uint16    _numObj;

	virtual void moveObjects() = 0;
	virtual void updateImages() = 0;
	virtual void swapImages(int objNumb1, int objNumb2) = 0;

	bool isCarrying(uint16 wordIndex);
	bool findObjectSpace(object_t *obj, int16 *destx, int16 *desty);

	int   calcMaxScore();
	int16 findObject(uint16 x, uint16 y);
	void freeObjects();
	void loadObjectArr(Common::File &in);
	void freeObjectArr();
	void loadNumObj(Common::File &in);
	void lookObject(object_t *obj);
	void readObjectImages();
	void restoreAllSeq();
	void restoreObjects(Common::SeekableReadStream *in);
	void saveObjects(Common::WriteStream *out);
	void saveSeq(object_t *obj);
	void setCarriedScreen(int screenNum);
	void showTakeables();
	void useObject(int16 objId);

	static int y2comp(const void *a, const void *b);

	bool isCarried(int objIndex) {
		return _objects[objIndex].carriedFl;
	}

	void setCarry(int objIndex, bool val) {
		_objects[objIndex].carriedFl = val;
	}

	void setVelocity(int objIndex, int8 vx, int8 vy) {
		_objects[objIndex].vx = vx;
		_objects[objIndex].vy = vy;
	}

	void setPath(int objIndex, path_t pathType, int16 vxPath, int16 vyPath) {
		_objects[objIndex].pathType = pathType;
		_objects[objIndex].vxPath = vxPath;
		_objects[objIndex].vyPath = vyPath;
	}
protected:
	HugoEngine *_vm;
	uint16     _objCount;

	void restoreSeq(object_t *obj);
};

class ObjectHandler_v1d : public ObjectHandler {
public:
	ObjectHandler_v1d(HugoEngine *vm);
	virtual ~ObjectHandler_v1d();

	void moveObjects();
	void updateImages();
	void swapImages(int objNumb1, int objNumb2);
};

class ObjectHandler_v1w : public ObjectHandler {
public:
	ObjectHandler_v1w(HugoEngine *vm);
	~ObjectHandler_v1w();

	void moveObjects();
	void updateImages();
	void swapImages(int objNumb1, int objNumb2);
};

class ObjectHandler_v2d : public ObjectHandler_v1d {
public:
	ObjectHandler_v2d(HugoEngine *vm);
	virtual ~ObjectHandler_v2d();

	void moveObjects();
	void updateImages();
};

class ObjectHandler_v3d : public ObjectHandler_v2d {
public:
	ObjectHandler_v3d(HugoEngine *vm);
	~ObjectHandler_v3d();

	void moveObjects();
	void swapImages(int objNumb1, int objNumb2);
};

} // End of namespace Hugo
#endif //HUGO_OBJECT_H
