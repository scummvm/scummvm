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

#ifndef M4_MEM_RES_H
#define M4_MEM_RES_H

#include "common/str.h"
#include "m4/mem/memman.h"
#include "m4/fileio/sys_file.h"

namespace M4 {

#define	MAX_RESOURCES	(1 << 7)

class Resources {
	struct Entry {
		Common::String name;
		MemHandle RHandle = nullptr;
		int32 BufferSize = 0;
		int32 Size = 0;
		byte Flags = 0;
	};
private:
	Entry _resources[MAX_RESOURCES];
	SysFile *_fp = nullptr;
	bool _useLowercase = false;

	Entry *findAndSetResEntry(const Common::String &resourceName);
	int hash(const Common::String &sym) const;
	int32 get_file(const Common::String &name);
	bool do_file(MemHandle buffer);

public:
	~Resources();
	void setUseLowercase(bool flag) {
		_useLowercase = flag;
	}

	MemHandle rget(const Common::String &resourceName, int32 *resourceSize);
	void rtoss(const Common::String &resourceName);

	/**
	 * Dumps a list of any active resources
	 */
	void dumpResources();
};

MemHandle rget(const Common::String &resourceName, int32 *ResourceSize);
void rtoss(const Common::String &resourceName);

} // namespace M4

#endif
