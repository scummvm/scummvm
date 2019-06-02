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

#include "engines/stark/debug.h"

#include "engines/stark/formats/dds.h"
#include "engines/stark/formats/tm.h"
#include "engines/stark/formats/xrc.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"

#include "engines/stark/services/archiveloader.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/settings.h"

#include "common/file.h"
#include "common/unzip.h"
#include "image/png.h"

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
	if (StarkSettings->isAssetsModEnabled()) {
		_textureSet = readOverrideDdsArchive();
	}

	if (!_textureSet) {
		ArchiveReadStream *stream = StarkArchiveLoader->getFile(_filename, _archiveName);

		_textureSet = Formats::TextureSetReader::read(stream);

		delete stream;
	}
}

static Common::String stripExtension(const Common::String &filename) {
	if (filename.hasSuffixIgnoreCase(".bmp")) {
		return Common::String(filename.c_str(), filename.size() - 4);
	}
	return filename;
}

void TextureSet::extractArchive() {
	ArchiveReadStream *stream = StarkArchiveLoader->getFile(_filename, _archiveName);
	Formats::BiffArchive *archive = Formats::TextureSetReader::readArchive(stream);

	Common::Array<Formats::Texture *> textures = archive->listObjectsRecursive<Formats::Texture>();
	for (uint i = 0; i < textures.size(); i++) {
		Common::String filename = Common::String::format(
		            "dump/%s/%s.png",
		            _filename.c_str(),
		            stripExtension(textures[i]->getName()).c_str());

		if (Common::File::exists(filename)) {
			continue;
		}

		Common::DumpFile out;
		if (!out.open(filename, true)) {
			warning("Unable to open file '%s' for writing", filename.c_str());
			return;
		}

		Graphics::Surface *surface = textures[i]->getSurface();

		Image::writePNG(out, *surface);

		out.close();

		surface->free();
		delete surface;
	}

	delete archive;
	delete stream;
}

Gfx::TextureSet *TextureSet::readOverrideDdsArchive() {
	Common::String archiveName = _filename + ".zip";

	debugC(kDebugModding, "Attempting to load %s", archiveName.c_str());

	Common::Archive *archive = Common::makeZipArchive(archiveName);
	if (!archive) {
		return nullptr;
	}

	Common::ArchiveMemberList files;
	archive->listMatchingMembers(files, "*.dds");
	if (files.empty()) {
		warning("No DDS files found in archive %s", archiveName.c_str());
		delete archive;
		return nullptr;
	}

	uint loadedCount = 0;
	Gfx::TextureSet *textureSet = new Gfx::TextureSet();

	for (Common::ArchiveMemberList::const_iterator it = files.begin(); it != files.end(); it++) {
		const Common::String &name = (*it)->getName();

		Common::SeekableReadStream *ddsStream = (*it)->createReadStream();
		if (!ddsStream) {
			warning("Unable to open %s for reading in %s", (*it)->getName().c_str(), archiveName.c_str());
			continue;
		}

		Formats::DDS dds;
		if (!dds.load(*ddsStream, name + " in " + archiveName)) {
			delete ddsStream;
			continue;
		}

		const Formats::DDS::MipMaps &mipmaps = dds.getMipMaps();
		if (mipmaps.empty()) {
			warning("No mipmaps in %s", name.c_str());
			delete ddsStream;
			continue;
		}

		Gfx::Texture *texture = StarkGfx->createTexture();
		texture->setLevelCount(mipmaps.size());
		for (uint i = 0; i < mipmaps.size(); i++) {
			texture->addLevel(i, &mipmaps[i]);
		}

		// Remove the .dds extension, add .bmp to match the names
		// used by the models.
		Common::String textureName = Common::String(name.c_str(), name.size() - 4);

		textureSet->addTexture(textureName + ".bmp", texture);

		delete ddsStream;

		loadedCount++;
	}

	debugC(kDebugModding, "Loaded %d textures from %s", loadedCount, archiveName.c_str());

	delete archive;

	return textureSet;
}

void TextureSet::printData() {
	debug("filename: %s", _filename.c_str());
}

} // End of namespace Resources
} // End of namespace Stark
