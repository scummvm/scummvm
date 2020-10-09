/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/hugo/resource_archive.h"
#include "glk/hugo/hugo.h"
#include "common/algorithm.h"
#include "common/memstream.h"

namespace Glk {
namespace Hugo {

bool ResourceArchive::splitName(const Common::String &name,
		Common::String &filename, Common::String &resName) {
	int commaIndex = name.findFirstOf(',');
	if (commaIndex == Common::String::npos)
		return false;

	filename = Common::String(name.c_str(), commaIndex);
	resName = Common::String(name.c_str() + commaIndex + 1);

	if (resName.hasSuffixIgnoreCase(".jpg"))
		resName = Common::String(resName.c_str(), resName.size() - 4);
	else if (resName.hasSuffixIgnoreCase(".jpeg"))
		resName = Common::String(resName.c_str(), resName.size() - 5);
	else if (resName.contains("."))
		return false;

	return true;
}


bool ResourceArchive::hasFile(const Common::String &name) const {
	Common::String filename, resName;

	if (!splitName(name, filename, resName))
		return false;
	size_t resLength = g_vm->FindResource(filename.c_str(), resName.c_str());
	g_vm->hugo_fclose(g_vm->resource_file);

	return resLength != 0;
}

const Common::ArchiveMemberPtr ResourceArchive::getMember(const Common::String &name) const {
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *ResourceArchive::createReadStreamForMember(const Common::String &name) const {
	Common::String filename, resName;

	// Split up the file and resource entry; return if it's not one
	if (!splitName(name, filename, resName))
		return nullptr;

	// Try and get the entry details from the given file
	size_t resLength = g_vm->FindResource(filename.c_str(), resName.c_str());

	if (!resLength) {
		g_vm->hugo_fclose(g_vm->resource_file);
		return nullptr;
	}

	// Otherwise, load the specified resource
	byte *buffer = (byte *)malloc(resLength);
	g_vm->glk_get_buffer_stream(g_vm->resource_file, (char *)buffer, resLength);
	g_vm->hugo_fclose(g_vm->resource_file);

	// Return a stream to allow access to the data
	return new Common::MemoryReadStream(buffer, resLength, DisposeAfterUse::YES);
}

} // End of namespace Hugo
} // End of namespace Glk
