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

#ifndef DIRECTOR_LINGO_XLIBS_SPACEMGR_H
#define DIRECTOR_LINGO_XLIBS_SPACEMGR_H

namespace Director {

namespace SpaceMgr {

struct LLink {
	Common::String payload;
};

struct View {
	Common::String payload;
	Common::HashMap<Common::String, LLink> llinks;
};

struct Node {
	Common::HashMap<Common::String, View> views;
};

struct Space {
	Common::HashMap<Common::String, Node> nodes;
};

struct SpaceCollection {
	Common::HashMap<Common::String, Space> spaces;
};

extern const char *const xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_dispose(int nargs);
void m_lastError(int nargs);
void m_memUsed(int nargs);
void m_listData(int nargs);
void m_sortAll(int nargs);
void m_checkForDups(int nargs);
void m_parseText(int nargs);
void m_getCurData(int nargs);
void m_setCurData(int nargs);
void m_addSpaceCollection(int nargs);
void m_removeSpaceCollection(int nargs);
void m_setCurSpaceCollection(int nargs);
void m_getCurSpaceCollection(int nargs);
void m_getSpaceCollection(int nargs);
void m_addSpace(int nargs);
void m_removeSpace(int nargs);
void m_setCurSpace(int nargs);
void m_getCurSpace(int nargs);
void m_getSpace(int nargs);
void m_addNode(int nargs);
void m_removeNode(int nargs);
void m_setCurNode(int nargs);
void m_getCurNode(int nargs);
void m_getNode(int nargs);
void m_addView(int nargs);
void m_removeView(int nargs);
void m_setCurView(int nargs);
void m_getCurView(int nargs);
void m_getView(int nargs);
void m_addLocalLink(int nargs);
void m_removeLocalLink(int nargs);
void m_removeLocalLinks(int nargs);
void m_getLocalLink(int nargs);
void m_getLocalLinks(int nargs);
void m_addGlobalLink(int nargs);
void m_removeGlobalLink(int nargs);
void m_getGlobalLink(int nargs);
void m_getGlobalLinks(int nargs);

} // End of namespace SpaceMgr

class SpaceMgrXObject : public Object<SpaceMgrXObject> {
public:
	SpaceMgrXObject(ObjectType objType);

	Common::String _curSpaceCollection;
	Common::String _curSpace;
	Common::String _curNode;
	Common::String _curView;
	bool _checkForDups = false;

	Common::HashMap<Common::String, SpaceMgr::SpaceCollection> _spaceCollections;
};
} // End of namespace Director

#endif
