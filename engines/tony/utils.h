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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TONY_UTILS_H
#define TONY_UTILS_H

#include "common/scummsys.h"
#include "common/file.h"
#include "common/str.h"

namespace Tony {

class RMResUpdate {
	struct ResUpdInfo {
		uint32 dwRes;
		uint32 offset;
		uint32 size;
		uint32 cmpSize;
	};

	uint32 _numUpd;
	ResUpdInfo *_infos;
	Common::File _hFile;

public:
	RMResUpdate();
	~RMResUpdate();

	void Init(const Common::String &fileName);
	const byte *QueryResource(uint32 dwRes);
};

} // End of namespace Tony

#endif /* TONY_H */
