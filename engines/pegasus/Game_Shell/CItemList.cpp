/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios
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

#include "common/error.h"
#include "common/stream.h"

#include "engines/pegasus/Game_Shell/CItem.h"
#include "engines/pegasus/Game_Shell/CItemList.h"

namespace Pegasus {

// TODO: Don't use global construction!
CItemList gAllItems;

CItemList::CItemList() {
}

CItemList::~CItemList() {
}

Common::Error CItemList::WriteToStream(Common::WriteStream *stream) {
	stream->writeUint32BE(size());

	for (CItemIterator it = begin(); it != end(); it++) {
		stream->writeUint16BE((*it)->GetObjectID());
		(*it)->WriteToStream(stream);
	}

	if (stream->err())
		return Common::kWritingFailed;
	
	return Common::kNoError;
}

Common::Error CItemList::ReadFromStream(Common::ReadStream *stream) {
	uint32 itemCount = stream->readUint32BE();

	for (uint32 i = 0; i < itemCount; i++) {
		tItemID itemID = stream->readUint16BE();
		gAllItems.FindItemByID(itemID)->ReadFromStream(stream);
	}

	if (stream->err())
		return Common::kReadingFailed;
	
	return Common::kNoError;
}

CItem *CItemList::FindItemByID(const tItemID id) {
	for (CItemIterator it = begin(); it != end(); it++)
		if ((*it)->GetObjectID() == id)
			return *it;

	return 0;
}

} // End of namespace Pegasus
