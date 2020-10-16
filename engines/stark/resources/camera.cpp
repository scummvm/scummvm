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

#include "engines/stark/debug.h"
#include "engines/stark/formats/xrc.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/scene.h"
#include "engines/stark/services/services.h"

namespace Stark {
namespace Resources {

Camera::~Camera() {
}

Camera::Camera(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_f1(0),
		_fov(45),
		_nearClipPlane(100.0),
		_farClipPlane(64000.0) {
	_type = TYPE;
}

void Camera::setClipPlanes(float near, float far) {
	_nearClipPlane = near;
	_farClipPlane = far;
}

void Camera::readData(Formats::XRCReadStream *stream) {
	_position = stream->readVector3();
	_lookDirection = stream->readVector3();
	_f1 = stream->readFloatLE();
	_fov = stream->readFloatLE();
	_viewSize = stream->readRect();
	_v4 = stream->readVector3();
}

void Camera::onAllLoaded() {
	Object::onAllLoaded();

	// Compute scroll coordinates bounds
	Common::Point maxScroll;
	maxScroll.x = _viewSize.width() - 640;
	maxScroll.y = _viewSize.height() - 365;

	Location *location = findParent<Location>();
	location->initScroll(maxScroll);
}

void Camera::onEnterLocation() {
	Object::onEnterLocation();

	// Setup the camera
	StarkScene->initCamera(_position, _lookDirection, _fov, _viewSize, _nearClipPlane, _farClipPlane);

	// Scroll the camera to its initial position
	Location *location = findParent<Location>();
	location->setScrollPosition(location->getScrollPosition());
}

Math::Angle Camera::getHorizontalAngle() const {
	Math::Angle lookDirectionAngle = Math::Vector3d::angle(_lookDirection, Math::Vector3d(1.0, 0.0, 0.0));
	Math::Vector3d cross = Math::Vector3d::crossProduct(_lookDirection, Math::Vector3d(1.0, 0.0, 0.0));
	if (cross.z() < 0) {
		return -lookDirectionAngle;
	} else {
		return lookDirectionAngle;
	}
}

void Camera::printData() {
	Common::StreamDebug debug = streamDbg();
	debug << "position: " << _position << "\n";
	debug << "lookDirection: " << _lookDirection << "\n";
	debug << "f1: " << _f1 << "\n";
	debug << "fov: " << _fov << "\n";
	debug << "viewSize:" << _viewSize.left << _viewSize.top << _viewSize.right << _viewSize.bottom << "\n";
	debug << "v4: " << _v4 << "\n";
}

} // End of namespace Resources
} // End of namespace Stark
