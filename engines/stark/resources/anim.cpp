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

#include "common/debug.h"

#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/direction.h"
#include "engines/stark/resources/image.h"
#include "engines/stark/xrcreader.h"

namespace Stark {

Resource *Anim::construct(Resource *parent, byte subType, uint16 index, const Common::String &name) {
	switch (subType) {
	case kAnimSub1:
		return new AnimSub1(parent, subType, index, name);
	case kAnimSub2:
		return new AnimSub2(parent, subType, index, name);
	case kAnimSub3:
		return new AnimSub3(parent, subType, index, name);
	case kAnimSub4:
		return new AnimSub4(parent, subType, index, name);
	default:
		error("Unknown anim subtype %d", subType);
	}
}

Anim::~Anim() {
}

Anim::Anim(Resource *parent, byte subType, uint16 index, const Common::String &name) :
				Resource(parent, subType, index, name),
				_field_30(0),
				_currentFrame(0),
				_numFrames(0),
				_refCount(0) {
	_type = TYPE;
}

void Anim::readData(XRCReadStream *stream) {
	_field_30 = stream->readUint32LE();
	_numFrames = stream->readUint32LE();
}

void Anim::selectFrame(uint32 frameIndex) {
}

Visual *Anim::getVisual() {
	return nullptr;
}

void Anim::reference(Item *item) {
	_refCount++;
}
void Anim::dereference(Item *item) {
	_refCount--;
}
bool Anim::isReferenced() {
	return _refCount > 0;
}

void Anim::printData() {
	debug("field_30: %d", _field_30);
	debug("numFrames: %d", _numFrames);
}

AnimSub1::~AnimSub1() {
}

AnimSub1::AnimSub1(Resource *parent, byte subType, uint16 index, const Common::String &name) :
				Anim(parent, subType, index, name),
				_field_3C(0),
				_currentDirection(0),
				_currentFrameImage(nullptr) {
}

void AnimSub1::readData(XRCReadStream *stream) {
	Anim::readData(stream);

	_field_3C = stream->readFloat();
}

void AnimSub1::onAllLoaded() {
	Anim::onAllLoaded();

	_directions = listChildren<Direction>();
}

void AnimSub1::selectFrame(uint32 frameIndex) {
	if (frameIndex > _numFrames) {
		error("Error setting frame %d for anim '%s'", frameIndex, getName().c_str());
	}

	_currentFrame = frameIndex;
}

Visual *AnimSub1::getVisual() {
	Direction *direction = _directions[_currentDirection];
	_currentFrameImage = direction->findChildWithIndex<Image>(_currentFrame);
	return _currentFrameImage->getVisual();
}

void AnimSub1::printData() {
	Anim::printData();

	debug("field_3C: %f", _field_3C);
}

AnimSub2::~AnimSub2() {
}

AnimSub2::AnimSub2(Resource *parent, byte subType, uint16 index, const Common::String &name) :
				Anim(parent, subType, index, name) {
}

AnimSub3::~AnimSub3() {
}

AnimSub3::AnimSub3(Resource *parent, byte subType, uint16 index, const Common::String &name) :
				Anim(parent, subType, index, name),
				_width(0),
				_height(0),
				_field_4C(-1),
				_field_50(0),
				_field_7C(0) {
}

void AnimSub3::readData(XRCReadStream *stream) {
	Anim::readData(stream);
	_smackerFile = stream->readString();
	_width = stream->readUint32LE();
	_height = stream->readUint32LE();

	_positions.clear();
	_sizes.clear();

	uint32 size = stream->readUint32LE();
	for (uint i = 0; i < size; i++) {
		_positions.push_back(stream->readPoint());
		_sizes.push_back(stream->readRect());
	}

	_field_7C = stream->readUint32LE();
	_field_4C = stream->readSint32LE();

	if (stream->isDataLeft()) {
		_field_50 = stream->readUint32LE();
	}
}

void AnimSub3::printData() {
	Anim::printData();

	debug("smackerFile: %s", _smackerFile.c_str());
	debug("size: x %d, y %d", _width, _height);

	Common::String description;
	for (uint32 i = 0; i < _positions.size(); i++) {
		description += Common::String::format("(x %d, y %d) ", _positions[i].x, _positions[i].y);
	}
	debug("positions: %s", description.c_str());

	description.clear();
	for (uint32 i = 0; i < _sizes.size(); i++) {
		description += Common::String::format("(l %d, t %d, r %d, b %d) ",
				_sizes[i].left, _sizes[i].top, _sizes[i].right, _sizes[i].bottom);
	}
	debug("sizes: %s", description.c_str());

	debug("field_4C: %d", _field_4C);
	debug("field_50: %d", _field_50);
	debug("field_7C: %d", _field_7C);
}

AnimSub4::~AnimSub4() {
}

AnimSub4::AnimSub4(Resource *parent, byte subType, uint16 index, const Common::String &name) :
				Anim(parent, subType, index, name) {
}

} // End of namespace Stark
