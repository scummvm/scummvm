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
#include "alcachofa.h"

using namespace Common;
using namespace Math;

namespace Alcachofa {

Item::Item(Room *room, ReadStream &stream)
	: GraphicObject(room, stream) {
	stream.readByte(); // unused and ignored byte
}

InteractableObject::InteractableObject(Room *room, ReadStream &stream)
	: PhysicalObject(room, stream)
	, _interactionPoint(Shape(stream).firstPoint())
	, _cursorType((CursorType)stream.readSint32LE())
	, _relatedObject(readVarString(stream)) {
	_relatedObject.toUppercase();
}

void InteractableObject::drawDebug() {
	auto renderer = dynamic_cast<IDebugRenderer *>(&g_engine->renderer());
	if (!g_engine->console().showInteractables() || renderer == nullptr || !isEnabled())
		return;

	renderer->debugShape(*shape());
}

Door::Door(Room *room, ReadStream &stream)
	: InteractableObject(room, stream)
	, _targetRoom(readVarString(stream))
	, _targetObject(readVarString(stream))
	, _characterDirection((Direction)stream.readSint32LE()) {
	_targetRoom.replace(' ', '_');
}

Character::Character(Room *room, ReadStream &stream)
	: ShapeObject(room, stream)
	, _interactionPoint(Shape(stream).firstPoint())
	, _direction((Direction)stream.readSint32LE())
	, _graphicNormal(stream)
	, _graphicTalking(stream) {
	_graphicNormal.start(true);
	_order = _graphicNormal.order();
}

void Character::serializeSave(Serializer &serializer) {
	ShapeObject::serializeSave(serializer);
	serializer.syncAsByte(_isTalking);
	serializer.syncAsSint32LE(_curDialogId);
	_graphicNormal.serializeSave(serializer);
	_graphicTalking.serializeSave(serializer);
	syncObjectAsString(serializer, _curAnimateObject);
	syncObjectAsString(serializer, _curTalkingObject);
	serializer.syncAsFloatLE(_lodBias);
}

void Character::syncObjectAsString(Serializer &serializer, ObjectBase *&object) {
	String name;
	if (serializer.isSaving() && object != nullptr)
		name = object->name();

	serializer.syncString(name);

	if (serializer.isLoading()) {
		if (name.empty())
			object = nullptr;
		else {
			object = room()->getObjectByName(name);
			if (object == nullptr)
				object = room()->world().getObjectByName(name);
			if (object == nullptr)
				error("Invalid object name \"%s\" saved for \"%s\" in \"%s\"",
					name.c_str(), this->name().c_str(), room()->name().c_str());
		}
	}
}

WalkingCharacter::WalkingCharacter(Room *room, ReadStream &stream)
	: Character(room, stream) {
	for (int32 i = 0; i < kDirectionCount; i++) {
		auto fileName = readVarString(stream);
		_walkingAnimations[i].reset(new Animation(Common::move(fileName)));
	}
	for (int32 i = 0; i < kDirectionCount; i++) {
		auto fileName = readVarString(stream);
		_standingAnimations[i].reset(new Animation(Common::move(fileName)));
	}
}

void WalkingCharacter::serializeSave(Serializer &serializer) {
	Character::serializeSave(serializer);
	serializer.syncAsSint32LE(_lastWalkAnimFrame);
	serializer.syncAsSint32LE(_walkSpeed);
	syncPoint(serializer, _sourcePos);
	syncPoint(serializer, _targetPos);
	serializer.syncAsByte(_isWalking);
	syncArray(serializer, _pathPoints, syncPoint);
	syncEnum(serializer, _direction);
	_graphicWalking.serializeSave(serializer);
}

MainCharacter::MainCharacter(Room *room, ReadStream &stream)
	: WalkingCharacter(room, stream) {
	stream.readByte(); // unused byte
	_order = 100;

	_kind =
		name().equalsIgnoreCase("MORTADELO") ? MainCharacterKind::Mortadelo
		: name().equalsIgnoreCase("FILEMON") ? MainCharacterKind::Filemon
		: MainCharacterKind::None;
}

MainCharacter::~MainCharacter() {
	for (auto *item : _items)
		delete item;
}

void syncDialogMenuLine(Serializer &serializer, DialogMenuLine &line) {
	serializer.syncAsSint32LE(line._dialogId);
	serializer.syncAsSint32LE(line._yPosition);
	serializer.syncAsSint32LE(line._returnId);
}

void MainCharacter::serializeSave(Serializer &serializer) {
	String roomName = room()->name();
	serializer.syncString(roomName);
	if (serializer.isLoading()) {
		room() = room()->world().getRoomByName(roomName);
		if (room() == nullptr)
			error("Invalid room name \"%s\" saved for \"%s\"", roomName.c_str(), name().c_str());
	}

	Character::serializeSave(serializer);
	serializer.syncAsSint32LE(_relatedProcessCounter);
	syncArray(serializer, _dialogMenuLines, syncDialogMenuLine);
	syncObjectAsString(serializer, _currentlyUsingObject);

	for (auto *item : _items) {
		bool isEnabled = item->isEnabled();
		serializer.syncAsByte(isEnabled);
		item->toggle(isEnabled);
	}
}

Background::Background(Room *room, const String &animationFileName, int16 scale)
	: GraphicObject(room, "BACKGROUND") {
	toggle(true);
	_graphic.setAnimation(animationFileName, AnimationFolder::Backgrounds);
	_graphic.scale() = scale;
	_graphic.order() = 59;
}

FloorColor::FloorColor(Room *room, ReadStream &stream)
	: ObjectBase(room, stream)
	, _shape(stream) {}

void FloorColor::drawDebug() {
	auto renderer = dynamic_cast<IDebugRenderer *>(&g_engine->renderer());
	if (!g_engine->console().showFloorColor() || renderer == nullptr || !isEnabled())
		return;

	renderer->debugShape(*shape(), kDebugGreen);
}

Shape *FloorColor::shape() {
	return &_shape;
}

}
