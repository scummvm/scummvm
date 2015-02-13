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

#include "engines/stark/resources/textureset.h"

#include "engines/stark/formats/tm.h"
#include "engines/stark/formats/xrc.h"
#include "engines/stark/gfx/texture.h"
#include "engines/stark/services/archiveloader.h"
#include "engines/stark/services/services.h"

namespace Stark {
namespace Resources {

TextureSet::~TextureSet() {
	delete _textureSet;
}

TextureSet::TextureSet(Object *parent, byte subType, uint16 index, const Common::String &name) :
				Object(parent, subType, index, name),
				_textureSet(nullptr) {
	_type = TYPE;
}

Gfx::TextureSet *TextureSet::getTexture() {
	return _textureSet;
}

void TextureSet::readData(Formats::XRCReadStream *stream) {
	_filename = stream->readString();
	_archiveName = stream->getArchiveName();
}

void TextureSet::onPostRead() {
	// Get the required services
	ArchiveLoader *archiveLoader = StarkServices::instance().archiveLoader;
	Gfx::Driver *gfxDriver = StarkServices::instance().gfx;

	Common::ReadStream *stream = archiveLoader->getFile(_filename, _archiveName);

	Formats::TextureSetReader *reader = new Formats::TextureSetReader(gfxDriver);

	_textureSet = reader->read(stream);

	delete reader;
	delete stream;
}

void TextureSet::printData() {
	debug("filename: %s", _filename.c_str());
}

} // End of namespace Resources
} // End of namespace Stark
