/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "trecision/struct.h"

namespace Trecision {

void SRoom::loadRoom(Common::File *file) {
	file->read(&_baseName, 4);
	_flag = file->readByte();
	file->readByte(); // Padding
	_bkgAnim = file->readUint16LE();
	for (int j = 0; j < MAXOBJINROOM; ++j)
		_object[j] = file->readUint16LE();
	for (int j = 0; j < MAXSOUNDSINROOM; ++j)
		_sounds[j] = file->readUint16LE();
	for (int j = 0; j < MAXACTIONINROOM; ++j)
		_actions[j] = file->readUint16LE();
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
	ser.syncAsUint16LE(_lim.left);
	ser.syncAsUint16LE(_lim.top);
	ser.syncAsUint16LE(_lim.right);
	ser.syncAsUint16LE(_lim.bottom);
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

void SObject::loadObj(Common::File *file) {
	uint16 w = file->readUint16LE();
	uint16 h = file->readUint16LE();
	_rect.left = file->readUint16LE();
	_rect.top = file->readUint16LE();
	_rect.setWidth(w);
	_rect.setHeight(h);

	_lim.left = file->readUint16LE();
	_lim.top = file->readUint16LE();
	_lim.right = file->readUint16LE();
	_lim.bottom = file->readUint16LE();

	_position = file->readSByte();
	file->readByte(); // Padding
	_name = file->readUint16LE();
	_examine = file->readUint16LE();
	_action = file->readUint16LE();
	_goRoom = file->readByte();
	_nbox = file->readByte();
	_ninv = file->readByte();
	_mode = file->readByte();
	_flag = file->readByte();
	file->readByte(); // Padding
	_anim = file->readUint16LE();
}

/********************************************************************/

void SInvObject::syncGameStream(Common::Serializer &ser) {
	ser.syncAsUint16LE(_name);
	ser.syncAsUint16LE(_examine);
	ser.syncAsUint16LE(_action);
	ser.syncAsUint16LE(_anim);
	ser.syncAsByte(_flag);
}

void SInvObject::loadObj(Common::File *file) {
	_name = file->readUint16LE();
	_examine = file->readUint16LE();
	_action = file->readUint16LE();
	_flag = file->readByte();
	file->readByte(); // Padding
	_anim = file->readUint16LE();
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
