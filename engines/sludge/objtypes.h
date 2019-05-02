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
#ifndef SLUDGE_OBJTYPES_H
#define SLUDGE_OBJTYPES_H

namespace Sludge {

class SludgeEngine;

struct Combination {
	int withObj, funcNum;
};

struct ObjectType {
	Common::String screenName;
	int objectNum;
	byte r, g, b;
	int numCom;
	int speechGap, walkSpeed, wrapSpeech, spinSpeed;
	uint16 flags;
	Combination *allCombis;
};

typedef Common::List<ObjectType *> ObjectTypeList;

class ObjectManager {
public:
	ObjectManager(SludgeEngine *vm) : _vm(vm) { init(); }
	~ObjectManager();

	bool init();
	void kill();

	ObjectType *findObjectType(int i);
	ObjectType *loadObjectType(int i);
	int getCombinationFunction(int a, int b);
	void removeObjectType(ObjectType *oT);
	void saveObjectRef(ObjectType *r, Common::WriteStream *stream);
	ObjectType *loadObjectRef(Common::SeekableReadStream *stream);

private:
	ObjectTypeList _allObjectTypes;
	SludgeEngine *_vm;
};

} // End of namespace Sludge

#endif
