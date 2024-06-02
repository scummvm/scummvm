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
#include "scheduler.h"
#include "stream-helper.h"
#include "alcachofa.h"

#include "common/system.h"

using namespace Common;

namespace Alcachofa {

const char *ObjectBase::typeName() const { return "ObjectBase"; }

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

void ObjectBase::draw() {
}

void ObjectBase::drawDebug() {
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

const char *PointObject::typeName() const { return "PointObject"; }

PointObject::PointObject(Room *room, ReadStream &stream)
	: ObjectBase(room, stream) {
	_pos = Shape(stream).firstPoint();
}

const char *GraphicObject::typeName() const { return "GraphicObject"; }

GraphicObject::GraphicObject(Room *room, ReadStream &stream)
	: ObjectBase(room, stream)
	, _graphic(stream)
	, _type((GraphicObjectType)stream.readSint32LE())
	, _posterizeAlpha(100 - stream.readSint32LE()) {
	_graphic.start(true);
}

GraphicObject::GraphicObject(Room *room, const char *name)
	: ObjectBase(room, name)
	, _type(GraphicObjectType::Normal)
	, _posterizeAlpha(0) {
}

void GraphicObject::draw() {
	if (!isEnabled())
		return;
	const BlendMode blendMode = _type == GraphicObjectType::Effect
		? BlendMode::Alpha
		: BlendMode::AdditiveAlpha;
	const bool is3D = room() != &g_engine->world().inventory();
	_graphic.update();
	g_engine->drawQueue().add<AnimationDrawRequest>(_graphic, is3D, blendMode);
}

void GraphicObject::loadResources() {
	_graphic.loadResources();
}

void GraphicObject::freeResources() {
	_graphic.freeResources();
}

void GraphicObject::serializeSave(Serializer &serializer) {
	ObjectBase::serializeSave(serializer);
	_graphic.serializeSave(serializer);
}

Graphic *GraphicObject::graphic() {
	return &_graphic;
}

struct AnimateTask : public Task {
	AnimateTask(Process &process, GraphicObject *object)
		: Task(process)
		, _object(object) {
		assert(_object != nullptr);
		_graphic = object->graphic();
		assert(_graphic != nullptr);
		_duration = _graphic->animation().totalDuration();
	}

	virtual TaskReturn run() override {
		TASK_BEGIN;
		_object->toggle(true);
		_graphic->start(false);
		TASK_WAIT(delay(_duration));
		_object->toggle(false);
		TASK_END;
	}

	virtual void debugPrint() override {
		g_engine->getDebugger()->debugPrintf("Animate \"%s\" for %ums", _object->name().c_str(), _duration);
	}

private:
	GraphicObject *_object;
	Graphic *_graphic;
	uint32 _duration;
};

Task *GraphicObject::animate(Process &process) {
	return new AnimateTask(process, this);
}

const char *SpecialEffectObject::typeName() const { return "SpecialEffectObject"; }

SpecialEffectObject::SpecialEffectObject(Room *room, ReadStream &stream)
	: GraphicObject(room, stream) {
	_topLeft = Shape(stream).firstPoint();
	_bottomRight = Shape(stream).firstPoint();
	_texShift.setX(stream.readSint32LE());
	_texShift.setY(stream.readSint32LE());
	_texShift *= kShiftSpeed;
}

void SpecialEffectObject::draw() {
	if (!isEnabled()) // TODO: Add high quality check
		return;
	const auto texOffset = g_system->getMillis() * 0.001f * _texShift;
	const BlendMode blendMode = _type == GraphicObjectType::Effect
		? BlendMode::Additive
		: BlendMode::AdditiveAlpha;
	Point topLeft = _topLeft, bottomRight = _bottomRight;
	if (topLeft.x == bottomRight.x || topLeft.y == bottomRight.y) {
		topLeft = _graphic.center();
		bottomRight = topLeft + _graphic.animation().imageSize(0);
	}

	_graphic.update();
	g_engine->drawQueue().add<SpecialEffectDrawRequest>(_graphic, topLeft, bottomRight, texOffset, blendMode);
}

const char *ShapeObject::typeName() const { return "ShapeObject"; }

ShapeObject::ShapeObject(Room *room, ReadStream &stream)
	: ObjectBase(room, stream)
	, _shape(stream)
	, _cursorType((CursorType)stream.readSint32LE()) {
}

void ShapeObject::update() {
	if (isEnabled())
		updateSelection();
	else {
		_isSelected = false;
		_wasSelected = false;
	}
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

void ShapeObject::onHoverStart() {
	onHoverUpdate();
}

void ShapeObject::onHoverEnd() {
}

void ShapeObject::onHoverUpdate() {
	g_engine->drawQueue().add<TextDrawRequest>(
		g_engine->world().generalFont(),
		g_engine->world().getLocalizedName(name()),
		g_engine->input().mousePos2D() - Point(0, 35),
		-1, true, kWhite, 0);
}

void ShapeObject::onClick() {
	onHoverUpdate();
}

void ShapeObject::markSelected() {
	_isSelected = true;
}

void ShapeObject::updateSelection() {
	if (_isSelected) {
		_isSelected = false;
		if (_wasSelected) {
			if (g_engine->input().wasAnyMouseReleased() && g_engine->player().selectedObject() == this)
				onClick();
			else
				onHoverUpdate();
		}
		else {
			_wasSelected = true;
			onHoverStart();
		}
	}
	else if (_wasSelected) {
		_wasSelected = false;
		onHoverEnd();
	}
}

const char *PhysicalObject::typeName() const { return "PhysicalObject"; }

PhysicalObject::PhysicalObject(Room *room, ReadStream &stream)
	: ShapeObject(room, stream) {
	_order = stream.readSByte();
}

}
