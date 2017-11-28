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

#ifndef AGDS_RESOURCE_MANAGER_H
#define AGDS_RESOURCE_MANAGER_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/ptr.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

namespace AGDS {

class ResourceManager {
private:
	struct GrpFile
	{
		Common::String filename;

		GrpFile(const Common::String &fname): filename(fname) {
		}
	};
	typedef Common::SharedPtr<GrpFile> GrpFilePtr;

	struct Resource
	{
		GrpFilePtr	grp;
		uint32		offset;
		uint32		size;
		Resource(const GrpFilePtr &g, uint32 o, uint32 s):
		grp(g), offset(o), size(s) {
		}
	};
	typedef Common::SharedPtr<Resource> ResourcePtr;

	typedef Common::HashMap<Common::String, ResourcePtr> ResourcesType;
	ResourcesType _resources;

	template<typename T>
	static void decrypt(T * data, unsigned size);

public:
	ResourceManager();
	~ResourceManager();

	bool addPath(const Common::String &grpFilename);
};


} // End of namespace AGDS

#endif /* AGDS_RESOURCE_MANAGER_H */
