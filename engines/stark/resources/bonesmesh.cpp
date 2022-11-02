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

#include "engines/stark/resources/bonesmesh.h"

#include "engines/stark/model/animhandler.h"
#include "engines/stark/model/model.h"
#include "engines/stark/services/archiveloader.h"
#include "engines/stark/services/services.h"
#include "engines/stark/formats/xrc.h"

namespace Stark {
namespace Resources {

BonesMesh::~BonesMesh() {
	delete _model;
}

BonesMesh::BonesMesh(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_model(nullptr) {
	_type = TYPE;
}

void BonesMesh::readData(Formats::XRCReadStream *stream) {
	_filename = stream->readString();
	_archiveName = stream->getArchiveName();
}

void BonesMesh::onPostRead() {
	ArchiveReadStream *stream = StarkArchiveLoader->getFile(_filename, _archiveName);

	_model = new Model();
	_model->readFromStream(stream);

	delete stream;
}

Model *BonesMesh::getModel() {
	return _model;
}

void BonesMesh::printData() {
	debug("filename: %s", _filename.c_str());
}

} // End of namespace Resources
} // End of namespace Stark
