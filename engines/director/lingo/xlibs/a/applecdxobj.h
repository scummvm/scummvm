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

#ifndef DIRECTOR_LINGO_XLIBS_APPLECDXOBJ_H
#define DIRECTOR_LINGO_XLIBS_APPLECDXOBJ_H

#include "common/ptr.h"

namespace Common {
	class CueSheet;
}

namespace Director {

class AppleCDXObject : public Object<AppleCDXObject> {
public:
	AppleCDXObject(ObjectType objType);
	int _inpoint;
	int _outpoint;
	// Instead of immediately returning values, methods which return
	// a value store it internally and return it via a subsequent
	// call to mGetValue.
	int _returnValue;
	Common::SharedPtr<Common::CueSheet> _cue;
};

namespace AppleCDXObj {

extern const char *const xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_dispose(int nargs);
void m_still(int nargs);
void m_service(int nargs);
void m_readStatus(int nargs);
void m_getValue(int nargs);
void m_setInPoint(int nargs);
void m_setOutPoint(int nargs);
void m_playCue(int nargs);
void m_playSegment(int nargs);
void m_readPos(int nargs);
void m_getFirstTrack(int nargs);
void m_getLastTrack(int nargs);
void m_getFirstFrame(int nargs);
void m_getLastFrame(int nargs);
void m_eject(int nargs);

} // End of namespace AppleCDXObj

} // End of namespace Director

#endif
