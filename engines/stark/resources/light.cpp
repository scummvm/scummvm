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

#include "engines/stark/resources/light.h"

#include "engines/stark/formats/xrc.h"
#include "engines/stark/gfx/renderentry.h"
#include "engines/stark/services/stateprovider.h"

namespace Stark {
namespace Resources {

Light::~Light() {
	delete _lightEntry;
}

Light::Light(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_outerConeAngle(0),
		_innerConeAngle(0),
		_falloffNear(100.0),
		_falloffFar(500.0),
		_lightEntry(nullptr),
		_multiplier(1.0) {
	_type = TYPE;
}

void Light::readData(Formats::XRCReadStream *stream) {
	_color = stream->readVector3();
	_position = stream->readVector3();
	_direction = stream->readVector3();
	_outerConeAngle = stream->readFloatLE();
	_innerConeAngle = stream->readFloatLE();

	if (stream->isDataLeft()) {
		_falloffNear = stream->readFloatLE();
		_falloffFar = stream->readFloatLE();
	}
}

void Light::onPostRead() {
	Object::onPostRead();

	_lightEntry = new Gfx::LightEntry();
	_lightEntry->type = (Gfx::LightEntry::Type) _subType;
	_lightEntry->direction = _direction;
	_lightEntry->innerConeAngle = _innerConeAngle / 2.0;
	_lightEntry->outerConeAngle = _outerConeAngle / 2.0;
	_lightEntry->falloffNear = _falloffNear;
	_lightEntry->falloffFar = _falloffFar;

	// Negative lights add darkness
	_multiplier = _name.hasPrefix("x_neg") ? -1.0 : 1.0;
}

void Light::saveLoad(ResourceSerializer *serializer) {
	Object::saveLoad(serializer);

	serializer->syncAsVector3d(_color);
	serializer->syncAsVector3d(_position);
}

void Light::setColor(int32 red, int32 green, int32 blue) {
	_color.x() = (float) red / 255.0f;
	_color.y() = (float) green / 255.0f;
	_color.z() = (float) blue / 255.0f;
}

void Light::setPosition(const Math::Vector3d &position) {
	_position = position;
}

Gfx::LightEntry *Light::getLightEntry() {
	_lightEntry->color = _multiplier * _color;
	_lightEntry->position = _position;

	return _lightEntry;
}

void Light::printData() {
	Common::StreamDebug debug = streamDbg();
	debug << "color: " << _color << "\n";
	debug << "position: " << _position << "\n";
	debug << "direction: " << _direction << "\n";
	debug << "innerConeAngle: " << _innerConeAngle << "\n";
	debug << "outerConeAngle: " << _outerConeAngle << "\n";
	debug << "falloffNear: " << _falloffNear << "\n";
	debug << "falloffFar: " << _falloffFar << "\n";
}

} // End of namespace Resources
} // End of namespace Stark
