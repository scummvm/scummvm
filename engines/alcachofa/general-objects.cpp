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

#include "objects.h"
#include "rooms.h"
#include "stream-helper.h"

#include "common/system.h"

using namespace Common;

namespace Alcachofa {

ObjectBase::ObjectBase(Room *room, const char *name)
	: _room(room)
	, _name(name)
	, _isEnabled(false) {
	assert(room != nullptr);
}

ObjectBase::ObjectBase(Room *room, ReadStream &stream)
	: _room(room) {
	assert(room != nullptr);
	_name = readVarString(stream);
	_isEnabled = readBool(stream);
}

void ObjectBase::toggle(bool isEnabled) {
	_isEnabled = isEnabled;
}

void ObjectBase::render() {
}

void ObjectBase::update() {
}

void ObjectBase::loadResources() {
}

void ObjectBase::freeResources() {
}

void ObjectBase::serializeSave(Serializer &serializer) {
	serializer.syncAsByte(_isEnabled);
}

Graphic *ObjectBase::graphic() {
	return nullptr;
}

Shape *ObjectBase::shape() {
	return nullptr;
}

PointObject::PointObject(Room *room, ReadStream &stream)
	: ObjectBase(room, stream) {
	_pos = Shape(stream).firstPoint();
}

GraphicObject::GraphicObject(Room *room, ReadStream &stream)
	: ObjectBase(room, stream)
	, _graphic(stream)
	, _type((GraphicObjectType)stream.readSint32LE())
	, _posterizeAlpha(100 - stream.readSint32LE()) {
	_graphic.start(true);
}

GraphicObject::GraphicObject(Room *room, const char *name)
	: ObjectBase(room, name)
	, _type(GraphicObjectType::Type0)
	, _posterizeAlpha(0) {
}

void GraphicObject::serializeSave(Serializer &serializer) {
	ObjectBase::serializeSave(serializer);
	_graphic.serializeSave(serializer);
}

Graphic *GraphicObject::graphic() {
	return &_graphic;
}

ShiftingGraphicObject::ShiftingGraphicObject(Room *room, ReadStream &stream)
	: GraphicObject(room, stream) {
	_pos = Shape(stream).firstPoint();
	_size = Shape(stream).firstPoint();
	_texShift.setX(stream.readSint32LE() / 256.0f);
	_texShift.setY(stream.readSint32LE() / 256.0f);
	_startTime = g_system->getMillis();
}

ShapeObject::ShapeObject(Room *room, ReadStream &stream)
	: ObjectBase(room, stream)
	, _shape(stream)
	, _cursorType((CursorType)stream.readSint32LE()) {
}

void ShapeObject::serializeSave(Serializer &serializer) {
	serializer.syncAsSByte(_order);
}

Shape *ShapeObject::shape() {
	return &_shape;
}

CursorType ShapeObject::cursorType() const {
	return _cursorType;
}

PhysicalObject::PhysicalObject(Room *room, ReadStream &stream)
	: ShapeObject(room, stream) {
	_order = stream.readSByte();
}

}
