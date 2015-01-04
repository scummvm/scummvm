/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/archiveloader.h"
#include "engines/stark/resources/textureset.h"
#include "engines/stark/stark.h"
#include "engines/stark/texture.h"
#include "engines/stark/xrcreader.h"

namespace Stark {

TextureSet::~TextureSet() {
	delete _texture;
}

TextureSet::TextureSet(Resource *parent, byte subType, uint16 index, const Common::String &name) :
				Resource(parent, subType, index, name),
				_texture(nullptr) {
	_type = TYPE;
}

Texture *TextureSet::getTexture() {
	return _texture;
}

void TextureSet::readData(XRCReadStream *stream) {
	_filename = stream->readString();
	_archiveName = stream->getArchiveName();
}

void TextureSet::onPostRead() {
	// Get the archive loader service
	ArchiveLoader *archiveLoader = StarkServices::instance().archiveLoader;

	Common::ReadStream *stream = archiveLoader->getFile(_filename, _archiveName);

	_texture = new Texture();
	_texture->createFromStream(stream);

	delete stream;
}

void TextureSet::printData() {
	debug("filename: %s", _filename.c_str());
}

} // End of namespace Stark
