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

#include "titanic/core/game_object_desc_item.h"

namespace Titanic {

CGameObjectDescItem::CGameObjectDescItem(): CTreeItem() {
}

void CGameObjectDescItem::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	_clipList.save(file, indent);
	file->writeQuotedLine(_name, indent);
	file->writeQuotedLine(_string2, indent);
	_list1.save(file, indent);
	_list2.save(file, indent);

	CTreeItem::save(file, indent);
}

void CGameObjectDescItem::load(SimpleFile *file) {
	int val = file->readNumber();

	if (val != 1) {
		if (val)
			_clipList.load(file);

		_name = file->readString();
		_string2 = file->readString();
		_list1.load(file);
		_list1.load(file);
	}

	CTreeItem::load(file);
}

} // End of namespace Titanic
