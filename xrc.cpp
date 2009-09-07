/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "engines/stark/xrc.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/util.h"

namespace Stark {

XRCFile::XRCFile(Common::SeekableReadStream *stream){
	debug(1, "Opening XRC Archive");
	_xrcRoot = new xrcEntry[0];
	readEntry(stream, _xrcRoot);

	delete stream;
}

XRCFile::~XRCFile(){
	delete _xrcRoot;
}

void XRCFile::readEntry(Common::SeekableReadStream *stream, xrcEntry *parent){
	debug(2, " Entry");
	parent->dataType = stream->readByte();
	parent->unknown1 = stream->readByte();
	parent->unknown2 = stream->readUint16LE();
	parent->nameLength = stream->readUint16LE();
	parent->name = Common::String();
	for (int i = 0; i < parent->nameLength; i++)
		parent->name += stream->readByte();
	parent->dataLength = stream->readUint32LE();
	if (parent->dataLength == 0)
		parent->data = 0;
	else {
		parent->data = new byte[parent->dataLength];
		stream->read(parent->data, parent->dataLength);
	}
	parent->numChildren = stream->readUint16LE();
	parent->unknown3 = stream->readUint16LE();

	debug(2, "   Type %d, Name: %s, %d bytes", parent->dataType, parent->name.c_str(), parent->dataLength);
	debug(3, "   Raw: %X, %X, %X, %s, %d bytes, %X, %X", parent->dataType, parent->unknown1,
		parent->unknown2, parent->name.c_str(), parent->dataLength, parent->numChildren, parent->unknown3);

	if (parent->numChildren > 0) {
		debug(2, "   Children: %d", parent->numChildren);
		parent->children = new XRCFile::xrcEntry[parent->numChildren];
		for (int i = 0; i < parent->numChildren; i++)
			readEntry(stream, &parent->children[i]);
	} else {
		parent->children = 0;
	}

}

} // End of namespace Stark
