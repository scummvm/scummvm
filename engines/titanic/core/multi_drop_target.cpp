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

#include "titanic/core/multi_drop_target.h"
#include "titanic/support/string_parser.h"
#include "titanic/carry/carry.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CMultiDropTarget, CDropTarget)
	ON_MESSAGE(DropObjectMsg)
END_MESSAGE_MAP()

void CMultiDropTarget::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_dropFrames, indent);
	file->writeQuotedLine(_dropNames, indent);

	CDropTarget::save(file, indent);
}

void CMultiDropTarget::load(SimpleFile *file) {
	file->readNumber();
	_dropFrames = file->readString();
	_dropNames = file->readString();

	CDropTarget::load(file);
}

bool CMultiDropTarget::DropObjectMsg(CDropObjectMsg *msg) {
	CStringParser parser1(_dropFrames);
	CStringParser parser2(_dropNames);
	CString seperatorChars = ",";

	// WORKAROUND: The original didn't break out of loop if a drop target
	// succeeded, nor did it return the item to the inventory if incorrect
	while (parser2.parse(_itemMatchName, seperatorChars)) {
		_dropFrame = parser1.readInt();
		if (CDropTarget::DropObjectMsg(msg))
			return true;

		parser1.skipSeperators(seperatorChars);
		parser2.skipSeperators(seperatorChars);
	}

	msg->_item->petAddToInventory();
	return true;
}

} // End of namespace Titanic
