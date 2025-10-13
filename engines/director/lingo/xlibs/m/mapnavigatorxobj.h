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

#ifndef DIRECTOR_LINGO_XLIBS_MAPNAVIGATORXOBJ_H
#define DIRECTOR_LINGO_XLIBS_MAPNAVIGATORXOBJ_H

namespace Director {

class MapNavigatorXObject : public Object<MapNavigatorXObject> {
public:
	MapNavigatorXObject(ObjectType objType);
};

namespace MapNavigatorXObj {

extern const char *const xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_dispose(int nargs);
void m_getFirstNode(int nargs);
void m_resetHidden(int nargs);
void m_getNodeName(int nargs);
void m_getNodeIndex(int nargs);
void m_getBackgroundPicture(int nargs);
void m_getHotSpotCount(int nargs);
void m_setHidden(int nargs);
void m_getHidden(int nargs);
void m_pointInside(int nargs);
void m_getHotSpotRect(int nargs);
void m_getHotSpotCursor(int nargs);
void m_getEvaluationFcn(int nargs);
void m_getDestinationNode(int nargs);
void m_getInstructionCount(int nargs);
void m_getInstructionType(int nargs);
void m_getInstruction(int nargs);

} // End of namespace MapNavigatorXObj

} // End of namespace Director

#endif
