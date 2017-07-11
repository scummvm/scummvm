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

struct combination {
	int withObj, funcNum;
};

struct objectType {
	Common::String screenName;
	int objectNum;
	objectType *next;
	byte r, g, b;
	int numCom;
	int speechGap, walkSpeed, wrapSpeech, spinSpeed;
	uint16 flags;
	combination *allCombis;
};

bool initObjectTypes();
objectType *findObjectType(int i);
objectType *loadObjectType(int i);
int getCombinationFunction(int a, int b);
void removeObjectType(objectType *oT);
void saveObjectRef(objectType *r, Common::WriteStream *stream);
objectType *loadObjectRef(Common::SeekableReadStream *stream);

} // End of namespace Sludge

#endif
