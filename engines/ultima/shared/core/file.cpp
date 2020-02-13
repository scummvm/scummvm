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

#include "common/util.h"
#include "ultima/shared/core/file.h"
#include "ultima/shared/early/ultima_early.h"

namespace Ultima {
namespace Shared {

Common::String readStringFromStream(Common::SeekableReadStream *s) {
	Common::String result;
	char c;
	while ((c = s->readByte()) != '\0')
		result += c;

	return result;
}

/*------------------------------------------------------------------------*/


#define ERROR error("Could not open file - %s", name.c_str())

File::File(const Common::String &name) : Common::File() {
	if (!Common::File::open(name))
		ERROR;
}

bool File::open(const Common::String &name) {
	if (!Common::File::open(name))
		ERROR;
	return true;
}

bool File::open(const Common::String &name, Common::Archive &archive) {
	if (!Common::File::open(name, archive))
		ERROR;
	return true;
}

bool File::open(const Common::FSNode &node) {
	Common::String name = node.getName();
	if (!Common::File::open(node))
		ERROR;
	return true;
}

bool File::open(SeekableReadStream *stream, const Common::String &name) {
	if (!Common::File::open(stream, name))
		ERROR;
	return true;
}

Common::String File::readString() {
	Common::String result;
	char c;

	while (pos() < size() && (c = (char)readByte()) != '\0')
		result += c;

	return result;
}

} // End of namespace Shared
} // End of namespace Ultima
