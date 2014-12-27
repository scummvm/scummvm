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

#include "engines/stark/resources/camera.h"
#include "engines/stark/xrcreader.h"
#include "engines/stark/debug.h"

namespace Stark {

Camera::~Camera() {
}

Camera::Camera(Resource *parent, byte subType, uint16 index, const Common::String &name) :
		Resource(parent, subType, index, name),
		_fov(0),
		_f2(0) {
	_type = ResourceType::kCamera;
}

void Camera::readData(XRCReadStream *stream) {
	_position = stream->readVector3();
	_lookAt = stream->readVector3();
	_fov = stream->readFloat();
	_f2 = stream->readFloat();
	_v3 = stream->readVector4();
	_v4 = stream->readVector3();
}

void Camera::printData() {
	Common::Debug debug = streamDbg();
	debug << "position: " << _position << "\n";
	debug << "lookAt: " << _lookAt << "\n";
	debug << "fov: " << _fov << "\n";
	debug << "f1: " << _f2 << "\n";
	debug << "v3: " << _v3 << "\n";
	debug << "v4: " << _v4 << "\n";
}

} // End of namespace Stark
