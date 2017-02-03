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

#include "titanic/core/named_item.h"
#include "titanic/core/node_item.h"
#include "titanic/core/room_item.h"
#include "titanic/core/view_item.h"

namespace Titanic {

EMPTY_MESSAGE_MAP(CNamedItem, CTreeItem);

CString CNamedItem::dumpItem(int indent) const {
	CString result = CTreeItem::dumpItem(indent);
	result += " " + _name;

	return result;
}

void CNamedItem::save(SimpleFile *file, int indent) {
	file->writeNumberLine(0, indent);
	file->writeQuotedLine(_name, indent);

	CTreeItem::save(file, indent);
}

void CNamedItem::load(SimpleFile *file) {
	int val = file->readNumber();
	if (!val)
		_name = file->readString();

	CTreeItem::load(file);
}

bool CNamedItem::isEquals(const CString &name, bool startsWith) const {
	if (startsWith) {
		return getName().left(name.size()).compareToIgnoreCase(name) == 0;
	} else {
		return getName().compareToIgnoreCase(name) == 0;
	}
}

CViewItem *CNamedItem::findView() const {
	for (CTreeItem *parent = getParent(); parent; parent = parent->getParent()) {
		CViewItem *view = dynamic_cast<CViewItem *>(parent);
		if (view)
			return view;
	}

	error("Couldn't find parent view");
}

CNodeItem *CNamedItem::findNode() const {
	for (CTreeItem *parent = getParent(); parent; parent = parent->getParent()) {
		CNodeItem *node = dynamic_cast<CNodeItem *>(parent);
		if (node)
			return node;
	}

	error("Couldn't find parent node");
}

CRoomItem *CNamedItem::findRoom() const {
	for (CTreeItem *parent = getParent(); parent; parent = parent->getParent()) {
		CRoomItem *room = dynamic_cast<CRoomItem *>(parent);
		if (room)
			return room;
	}

	error("Couldn't find parent node");
}

} // End of namespace Titanic
