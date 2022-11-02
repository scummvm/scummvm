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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "trecision/struct.h"

namespace Trecision {

void SRoom::loadRoom(Common::SeekableReadStreamEndian *stream) {
	stream->read(&_baseName, 4);
	_flag = stream->readByte();
	stream->readByte(); // Padding
	_bkgAnim = stream->readUint16();
	for (int j = 0; j < MAXOBJINROOM; ++j)
		_object[j] = stream->readUint16();
	for (int j = 0; j < MAXSOUNDSINROOM; ++j)
		_sounds[j] = stream->readUint16();
	for (int j = 0; j < MAXACTIONINROOM; ++j)
		_actions[j] = stream->readUint16();
}

void SRoom::syncGameStream(Common::Serializer &ser) {
	ser.syncBytes((byte *)_baseName, 4);
	for (int i = 0; i < MAXACTIONINROOM; i++)
		ser.syncAsUint16LE(_actions[i]);
	ser.syncAsByte(_flag);
	ser.syncAsUint16LE(_bkgAnim);
}

/********************************************************************/

void SObject::readRect(Common::SeekableReadStream *stream) {
	_rect.left = stream->readUint16LE();
	_rect.top = stream->readUint16LE();
	_rect.setWidth(stream->readUint16LE());
	_rect.setHeight(stream->readUint16LE());
}

void SObject::syncGameStream(Common::Serializer &ser) {
	ser.syncAsUint16LE(_area.left);
	ser.syncAsUint16LE(_area.top);
	ser.syncAsUint16LE(_area.right);
	ser.syncAsUint16LE(_area.bottom);
	ser.syncAsUint16LE(_name);
	ser.syncAsUint16LE(_examine);
	ser.syncAsUint16LE(_action);
	ser.syncAsUint16LE(_anim);
	ser.syncAsByte(_mode);
	ser.syncAsByte(_flag);
	ser.syncAsByte(_goRoom);
	ser.syncAsByte(_nbox);
	ser.syncAsByte(_ninv);
	ser.syncAsSByte(_position);
}

void SObject::loadObj(Common::SeekableReadStreamEndian *stream) {
	uint16 w = stream->readUint16();
	uint16 h = stream->readUint16();
	_rect.left = stream->readUint16();
	_rect.top = stream->readUint16();
	_rect.setWidth(w);
	_rect.setHeight(h);

	_area.left = stream->readUint16();
	_area.top = stream->readUint16();
	_area.right = stream->readUint16();
	_area.bottom = stream->readUint16();

	_position = stream->readSByte();
	stream->readByte(); // Padding
	_name = stream->readUint16();
	_examine = stream->readUint16();
	_action = stream->readUint16();
	_goRoom = stream->readByte();
	_nbox = stream->readByte();
	_ninv = stream->readByte();
	_mode = stream->readByte();
	_flag = stream->readByte();
	stream->readByte(); // Padding
	_anim = stream->readUint16();
}

/********************************************************************/

void SInvObject::syncGameStream(Common::Serializer &ser) {
	ser.syncAsUint16LE(_name);
	ser.syncAsUint16LE(_examine);
	ser.syncAsUint16LE(_action);
	ser.syncAsUint16LE(_anim);
	ser.syncAsByte(_flag);
}

void SInvObject::loadObj(Common::SeekableReadStreamEndian *stream) {
	_name = stream->readUint16();
	_examine = stream->readUint16();
	_action = stream->readUint16();
	_flag = stream->readByte();
	stream->readByte(); // Padding
	_anim = stream->readUint16();
}

/********************************************************************/

void STexture::clear() {
	_dx = _dy = _angle = 0;
	_texture = nullptr;
	_active = false;
}

void STexture::set(int16 x, int16 y, uint8 *buffer) {
	_dx = x;
	_dy = y;
	_angle = 0;

	_active = true;
	_texture = buffer;
}

/********************************************************************/

void SVertex::clear() {
	_x = _y = _z = 0.0f;
	_nx = _ny = _nz = 0.0f;
}

/********************************************************************/

void SLight::clear() {
	_x = _y = _z = 0.0f;
	_dx = _dy = _dz = 0.0f;
	_inr = _outr = 0.0f;
	_hotspot = 0;
	_fallOff = 0;
	_inten = 0;
	_position = 0;
}

/********************************************************************/

void SCamera::clear() {
	_ex = _ey = _ez = 0.0f;
	_fovX = _fovY = 0.0f;
	for (uint8 i = 0; i < 3; ++i)
		_e1[i] = _e2[i] = _e3[i] = 0.0f;
}

} // namespace Trecision
