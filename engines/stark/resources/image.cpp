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

#include "engines/stark/resources/image.h"

#include "common/debug.h"

#include "engines/stark/archiveloader.h"
#include "engines/stark/stark.h"
#include "engines/stark/xmg.h"
#include "engines/stark/xrcreader.h"

namespace Stark {

Resource *Image::construct(Resource *parent, byte subType, uint16 index, const Common::String &name) {
	switch (subType) {
	case kImageSub2:
	case kImageSub3:
		return new ImageSub23(parent, subType, index, name);
	case kImageSub4:
		return new UnimplementedResource(parent, TYPE, subType, index, name);
	default:
		error("Unknown anim subtype %d", subType);
	}
}

Image::~Image() {
}

Image::Image(Resource *parent, byte subType, uint16 index, const Common::String &name) :
				Resource(parent, subType, index, name),
				_transparent(false),
				_transparency(0),
				_field_44_ADF(0),
				_field_48_ADF(30),
				_visual(nullptr) {
	_type = TYPE;
}

void Image::readData(XRCReadStream *stream) {
	_filename = stream->readString();
	_hotspot = stream->readPoint();
	_transparent = stream->readBool();
	_transparency = stream->readUint32LE();

	uint32 polygonCount = stream->readUint32LE();
	for (uint32 i = 0; i < polygonCount; i++) {
		Polygon polygon;

		uint32 pointCount = stream->readUint32LE();
		for (uint32 j = 0; j < pointCount; j++) {
			polygon.push_back(stream->readPoint());
		}

		_polygons.push_back(polygon);
	}

	_archiveName = stream->getArchiveName();
}

Visual *Image::getVisual() {
	return nullptr;
}

void Image::printData() {
	debug("filename: %s", _filename.c_str());
	debug("hotspot: x %d, y %d", _hotspot.x, _hotspot.y);
	debug("transparent: %d", _transparent);
	debug("transparency: %d", _transparency);
	debug("field_44: %d", _field_44_ADF);
	debug("field_48: %d", _field_48_ADF);

	for (uint32 i = 0; i < _polygons.size(); i++) {
		Common::String description;
		for (uint32 j = 0; j < _polygons[i].size(); j++) {
			description += Common::String::format("(x %d, y %d) ", _polygons[i][j].x, _polygons[i][j].y);
		}
		debug("polygon %d: %s", i, description.c_str());
	}
}

ImageSub23::~ImageSub23() {
}

ImageSub23::ImageSub23(Resource *parent, byte subType, uint16 index, const Common::String &name) :
				Image(parent, subType, index, name),
				_noName(false) {
}

void ImageSub23::readData(XRCReadStream *stream) {
	Image::readData(stream);

	if (stream->isDataLeft()) {
		_field_44_ADF = stream->readUint32LE();
		_field_44_ADF /= 33;
	}

	if (stream->isDataLeft()) {
		_field_48_ADF = stream->readUint32LE();
	}

	_noName = _filename == "noname" || _filename == "noname.xmg";
}

void ImageSub23::onPostRead() {
	initVisual();
}

Visual *ImageSub23::getVisual() {
	initVisual();
	return _visual;
}

void ImageSub23::initVisual() {
	if (_visual) {
		return; // The visual is already there
	}

	if (_noName) {
		return; // No file to load
	}

	// Get the archive loader service
	ArchiveLoader *archiveLoader = StarkServices::instance().archiveLoader;

	Common::ReadStream *stream = archiveLoader->getFile(_filename, _archiveName);

	_visual = VisualImageXMG::load(stream);

	delete stream;
}

void ImageSub23::printData() {
	Image::printData();
}

} // End of namespace Stark
