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

#ifndef PEGASUS_GAMESHELL_CITEMLIST_H
#define PEGASUS_GAMESHELL_CITEMLIST_H

#include "common/list.h"

#include "pegasus/Game_Shell/Headers/Game_Shell_Types.h"

namespace Common {
	class ReadStream;
	class WriteStream;
}

namespace Pegasus {

class CItem;

class CItemList : public Common::List<CItem *> {
public:
	CItemList();
	virtual ~CItemList();
	
	virtual Common::Error WriteToStream(Common::WriteStream *stream);
	virtual Common::Error ReadFromStream(Common::ReadStream *stream);
	
	CItem *FindItemByID(const tItemID id);
};

typedef CItemList::iterator CItemIterator;

// TODO: Don't use global construction!
extern CItemList gAllItems;

} // End of namespace Pegasus

#endif
