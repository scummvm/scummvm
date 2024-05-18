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

#include "graphics.h"
#include "stream-helper.h"

#include "common/system.h"

using namespace Common;

namespace Alcachofa {

Animation::Animation(String fileName, AnimationFolder folder)
	: _fileName(move(fileName))
	, _folder(folder) {
}

Graphic::Graphic() {
}

Graphic::Graphic(ReadStream &stream) {
	_center.x = stream.readSint16LE();
	_center.y = stream.readSint16LE();
	_scale = stream.readSint16LE();
	_order = stream.readSByte();
	auto animationName = readVarString(stream);
	_animation.reset(new Animation(std::move(animationName)));
}

void Graphic::start(bool isLooping) {
	_isPaused = false;
	_isLooping = isLooping;
	_lastTime = g_system->getMillis();
}

void Graphic::stop() {
	_isPaused = true;
	_isLooping = false;
	_lastTime = g_system->getMillis() - _lastTime;
}

void Graphic::serializeSave(Serializer &serializer) {
	syncPoint(serializer, _center);
	serializer.syncAsSint16LE(_scale);
	serializer.syncAsUint32LE(_lastTime);
	serializer.syncAsByte(_isPaused);
	serializer.syncAsByte(_isLooping);
	serializer.syncAsFloatLE(_camAcceleration);
}

}
