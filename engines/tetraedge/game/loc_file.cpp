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

#include "common/file.h"
#include "common/textconsole.h"
#include "common/formats/xmlparser.h"

#include "tetraedge/game/loc_file.h"
#include "tetraedge/te/te_name_val_xml_parser.h"

namespace Tetraedge {

LocFile::LocFile() {
}

void LocFile::load(const TetraedgeFSNode &fsnode) {
	TeNameValXmlParser parser;
	const Common::String xmlHeader("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	Common::ScopedPtr<Common::SeekableReadStream> locFile(fsnode.createReadStream());
	const Common::String path = fsnode.getName();
	if (!locFile)
		error("LocFile::load: failed to open %s.", path.c_str());

	int64 fileLen = locFile->size();
	char *buf = new char[fileLen + 1];
	buf[fileLen] = '\0';
	locFile->read(buf, fileLen);
	const Common::String xmlContents = xmlHeader + buf;
	delete [] buf;
	locFile.reset();
	if (!parser.loadBuffer((const byte *)xmlContents.c_str(), xmlContents.size()))
		error("LocFile::load: failed to load %s.", path.c_str());

	if (!parser.parse())
		error("LocFile::load: failed to parse %s.", path.c_str());

	_map = parser.getMap();
}

const Common::String *LocFile::value(const Common::String &key) const {
	return text(key);
}

} // end namespace Tetraedge
