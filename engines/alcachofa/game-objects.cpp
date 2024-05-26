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

ITriggerableObject::ITriggerableObject(ReadStream &stream)
	: _interactionPoint(Shape(stream).firstPoint())
	, _interactionDirection((Direction)stream.readSint32LE()) {}

InteractableObject::InteractableObject(Room *room, ReadStream &stream)
	: PhysicalObject(room, stream)
	, ITriggerableObject(stream)
	, _relatedObject(readVarString(stream)) {
	_relatedObject.toUppercase();
}

void InteractableObject::drawDebug() {
	auto renderer = dynamic_cast<IDebugRenderer *>(&g_engine->renderer());
	if (!g_engine->console().showInteractables() || renderer == nullptr || !isEnabled())
		return;

	renderer->debugShape(*shape());
}

void InteractableObject::trigger(const char *action) {
	warning("stub: Trigger object %s with %s", name().c_str(), action == nullptr ? "<null>" : action);
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
	, ITriggerableObject(stream)
	, _graphicNormal(stream)
	, _graphicTalking(stream) {
	_graphicNormal.start(true);
	_graphicNormal.frameI() = _graphicTalking.frameI() = 0;
	_order = _graphicNormal.order();
}

static Graphic *graphicOf(ObjectBase *object, Graphic *fallback = nullptr) {
	auto objectGraphic = object == nullptr ? nullptr : object->graphic();
	return objectGraphic == nullptr ? fallback : objectGraphic;
}

void Character::update() {
	if (!isEnabled())
		return;
	updateSelection();

	Graphic *animateGraphic = graphicOf(_curAnimateObject);
	if (animateGraphic != nullptr) {
		animateGraphic->center() = Point(0, 0);
		animateGraphic->update();
	}
	else if (_isTalking)
		updateTalkingAnimation();
	else if (g_engine->world().somebodyUsing(this)) {
		Graphic *talkGraphic = graphicOf(_curTalkingObject, &_graphicTalking);
		talkGraphic->start(true);
		talkGraphic->pause();
		talkGraphic->update();
	}
	else
		_graphicNormal.update();
}

void Character::updateTalkingAnimation() {
	Graphic *talkGraphic = graphicOf(_curTalkingObject, &_graphicTalking);
	if (!_isTalking) {
		talkGraphic->reset();
		return;
	}
	// TODO: Add lip-sync(?) animation behavior
	talkGraphic->update();
}

void Character::draw() {
	if (!isEnabled())
		return;
	Graphic *activeGraphic = graphic();
	assert(activeGraphic != nullptr);
	g_engine->drawQueue().add<AnimationDrawRequest>(*activeGraphic, true, BlendMode::AdditiveAlpha, _lodBias);
}

void Character::drawDebug() {
	auto renderer = dynamic_cast<IDebugRenderer *>(&g_engine->renderer());
	if (!g_engine->console().showCharacters() || renderer == nullptr || !isEnabled())
		return;

	renderer->debugShape(*shape());
}

void Character::loadResources() {
	_graphicNormal.loadResources();
	_graphicTalking.loadResources();
}

void Character::freeResources() {
	_graphicNormal.freeResources();
	_graphicTalking.freeResources();
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

Graphic *Character::graphic() {
	Graphic *activeGraphic = graphicOf(_curAnimateObject);
	if (activeGraphic == nullptr && (_isTalking || g_engine->world().somebodyUsing(this)))
		activeGraphic = graphicOf(_curTalkingObject, &_graphicTalking);
	if (activeGraphic == nullptr)
		activeGraphic = &_graphicNormal;
	return activeGraphic;
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

void Character::trigger(const char *action) {
	warning("stub: Trigger character %s with %s", name().c_str(), action == nullptr ? "<null>" : action);
}

WalkingCharacter::WalkingCharacter(Room *room, ReadStream &stream)
	: Character(room, stream) {
	for (int32 i = 0; i < kDirectionCount; i++) {
		auto fileName = readVarString(stream);
		_walkingAnimations[i].reset(new Animation(Common::move(fileName)));
	}
	for (int32 i = 0; i < kDirectionCount; i++) {
		auto fileName = readVarString(stream);
		_talkingAnimations[i].reset(new Animation(Common::move(fileName)));
	}
}

void WalkingCharacter::update() {
	Character::update();
	if (!isEnabled())
		return;
	updateWalking();

	auto activeFloor = room()->activeFloor();
	if (activeFloor != nullptr) {
		if (activeFloor->polygonContaining(_sourcePos) < 0)
			_sourcePos = _currentPos = activeFloor->getClosestPoint(_sourcePos);
		if (activeFloor->polygonContaining(_currentPos) < 0)
			_currentPos = activeFloor->getClosestPoint(_currentPos);
	}

	if (!_isWalking) {
		_graphicTalking.setAnimation(talkingAnimation());
		updateTalkingAnimation();
		_currentPos = _sourcePos;
	}

	_graphicNormal.center() = _graphicTalking.center() = _currentPos;
	auto animateGraphic = graphicOf(_curAnimateObject);
	auto talkingGraphic = graphicOf(_curTalkingObject);
	if (animateGraphic != nullptr)
		animateGraphic->center() = _currentPos;
	if (talkingGraphic != nullptr)
		talkingGraphic->center() = _currentPos;
	if (room() != &g_engine->world().globalRoom()) {
		float depth = room()->depthAt(_currentPos);
		int8 order = room()->orderAt(_currentPos);
		_graphicNormal.order() = _graphicTalking.order() = order;
		_graphicNormal.depthScale() = _graphicTalking.depthScale() = depth;
		if (animateGraphic != nullptr) {
			animateGraphic->order() = order;
			animateGraphic->depthScale() = depth;
		}
		if (talkingGraphic != nullptr) {
			talkingGraphic->order() = order;
			talkingGraphic->depthScale() = depth;
		}
	}

	_interactionPoint = _currentPos;
	_interactionDirection = Direction::Right;
	if (this != g_engine->world().activeCharacter()) {
		int16 interactionOffset = (int16)(150 * _graphicNormal.depthScale());
		_interactionPoint.x -= interactionOffset;
		if (activeFloor != nullptr && activeFloor->polygonContaining(_interactionPoint) < 0) {
			_interactionPoint.x = _currentPos.x + interactionOffset;
			_interactionDirection = Direction::Left;
		}
	}
}

static Direction getDirection(const Point &from, const Point &to) {
	Point delta = from - to;
	if (from.x == to.x)
		return from.y < to.y ? Direction::Up : Direction::Down;
	else if (from.x < to.x) {
		int slope = 1000 * delta.y / -delta.x;
		return slope > 1000 ? Direction::Up
			: slope < -1000 ? Direction::Down
			: Direction::Right;
	}
	else { // from.x > to.x
		int slope = 1000 * delta.y / delta.x;
		return slope > 1000 ? Direction::Up
			: slope < -1000 ? Direction::Down
			: Direction::Left;
	}
}

void WalkingCharacter::updateWalking() {
	if (!_isWalking)
		return;
	static constexpr float kHigherStepSizeThreshold = 0x4CCC / 65535.0f;
	static constexpr float kMinStepSizeFactor = 0x3333 / 65535.0f;
	_stepSizeFactor = _graphicNormal.depthScale();
	if (_stepSizeFactor < kHigherStepSizeThreshold)
		_stepSizeFactor = _stepSizeFactor / 3.0f + kMinStepSizeFactor;

	Point targetPos = _pathPoints.top();
	if (_sourcePos == targetPos) {
		_currentPos = targetPos;
		_pathPoints.pop();
	}
	else {
		updateWalkingAnimation();
		const int32 distanceToTarget = (int32)(sqrtf(_sourcePos.sqrDist(targetPos)));
		if (_walkedDistance < distanceToTarget) {
			// separated because having only 16 bits and multiplications seems dangerous
			_currentPos.x = _sourcePos.x + _walkedDistance * (targetPos.x - _sourcePos.x) / distanceToTarget;
			_currentPos.y = _sourcePos.y + _walkedDistance * (targetPos.y - _sourcePos.y) / distanceToTarget;
		}
		else {
			_sourcePos = _currentPos = targetPos;
			_pathPoints.pop();
			_walkedDistance = 1;
			_lastWalkAnimFrame = 0;
		}
	}
	
	if (_pathPoints.empty()) {
		_isWalking = false;
		_currentPos = _sourcePos = targetPos;
		if (_endWalkingDirection != Direction::Invalid)
			_direction = _endWalkingDirection;
		onArrived();
	}
	_graphicNormal.center() = _currentPos;
}

void WalkingCharacter::updateWalkingAnimation()
{
	_direction = getDirection(_sourcePos, _pathPoints.top());
	auto animation = walkingAnimation();
	_graphicNormal.setAnimation(animation);

	// this is very confusing. Let's see what it does
	const int32 halfFrameCount = (int32)animation->frameCount() / 2;
	int32 expectedFrame = (int32)(g_system->getMillis() - _graphicNormal.lastTime()) * 12 / 1000;
	const bool isUnexpectedFrame = expectedFrame != _lastWalkAnimFrame;
	int32 stepFrameFrom, stepFrameTo;
	if (expectedFrame < halfFrameCount - 1) {
		_lastWalkAnimFrame = expectedFrame;
		stepFrameFrom = 2 * expectedFrame - 2;
		stepFrameTo = 2 * expectedFrame;
	}
	else {
		const int32 frameThreshold = _lastWalkAnimFrame <= halfFrameCount - 1
			? _lastWalkAnimFrame
			: (_lastWalkAnimFrame - halfFrameCount + 1) % (halfFrameCount - 2) + 1;
		_lastWalkAnimFrame = expectedFrame;
		expectedFrame = (expectedFrame - halfFrameCount + 1) % (halfFrameCount - 2) + 1;
		if (expectedFrame >= frameThreshold) {
			stepFrameFrom = 2 * expectedFrame - 2;
			stepFrameTo = 2 * expectedFrame;
		}
		else {
			stepFrameFrom = 2 * halfFrameCount - 4;
			stepFrameTo = 2 * halfFrameCount - 2;
		}
	}
	if (isUnexpectedFrame) {
		const uint stepSize = (uint)sqrtf(animation->frameCenter(stepFrameFrom).sqrDist(animation->frameCenter(stepFrameTo)));
		_walkedDistance += (int32)(stepSize * _stepSizeFactor);
	}
	_graphicNormal.frameI() = 2 * expectedFrame; // especially this: wtf?
}

void WalkingCharacter::onArrived() {
}

void WalkingCharacter::stopWalkingAndTurn(Direction direction) {
	_isWalking = false;
	_direction = direction;
}

void WalkingCharacter::walkTo(
	const Point &target, Direction endDirection,
	ITriggerableObject *activateObject, const char *activateAction) {
	// all the activation parameters are only relevant for MainCharacter

	if (_isWalking)
		_sourcePos = _currentPos;
	else {
		_lastWalkAnimFrame = 0;
		int32 prevWalkFrame = _graphicNormal.frameI();
		_graphicNormal.reset();
		_graphicNormal.frameI() = prevWalkFrame;
	}

	_pathPoints.clear();
	auto floor = room()->activeFloor();
	if (floor != nullptr)
		floor->findPath(_sourcePos, target, _pathPoints);
	if (_pathPoints.empty()) {
		_isWalking = false;
		onArrived();
		return;
	}

	_isWalking = true;
	_endWalkingDirection = endDirection;
	_walkedDistance = 0;
	updateWalking();
}

void WalkingCharacter::setPosition(const Point &target) {
	_isWalking = false;
	_sourcePos = _currentPos = target;
}

void WalkingCharacter::draw() {
	if (!isEnabled())
		return;

	Graphic *currentGraphic = graphicOf(_curAnimateObject);
	if (currentGraphic == nullptr && _isWalking)
		currentGraphic = &_graphicNormal;
	if (currentGraphic == nullptr && g_engine->world().somebodyUsing(this)) {
		currentGraphic = graphicOf(_curTalkingObject, &_graphicTalking);
		currentGraphic->start(true);
		currentGraphic->pause();
	}
	if (currentGraphic == nullptr) {
		// TODO: draw dialog line
		currentGraphic = graphicOf(_curTalkingObject, &_graphicTalking);
	}

	assert(currentGraphic != nullptr);
	g_engine->drawQueue().add<AnimationDrawRequest>(*currentGraphic, true, BlendMode::AdditiveAlpha);
}

void WalkingCharacter::drawDebug() {
	Character::drawDebug();
	auto renderer = dynamic_cast<IDebugRenderer *>(&g_engine->renderer());
	if (!g_engine->console().showCharacters() || renderer == nullptr || !isEnabled() || _pathPoints.empty())
		return;

	Array<Vector2d> points2D(_pathPoints.size() + 1);
	_pathPoints.push(_sourcePos);
	for (uint i = 0; i < _pathPoints.size(); i++) {
		auto v = g_engine->camera().transform3Dto2D({ (float)_pathPoints[i].x, (float)_pathPoints[i].y, kBaseScale });
		points2D[i] = { v.x(), v.y() };
	}
	_pathPoints.pop();
	renderer->debugPolyline({ points2D.data(), points2D.size() }, kWhite);
}

void WalkingCharacter::loadResources() {
	Character::loadResources();
	for (int i = 0; i < kDirectionCount; i++) {
		_walkingAnimations[i]->load();
		_talkingAnimations[i]->load();
	}
}

void WalkingCharacter::freeResources() {
	Character::freeResources();
	for (int i = 0; i < kDirectionCount; i++) {
		_walkingAnimations[i]->freeImages();
		_talkingAnimations[i]->freeImages();
	}
}

void WalkingCharacter::serializeSave(Serializer &serializer) {
	Character::serializeSave(serializer);
	serializer.syncAsSint32LE(_lastWalkAnimFrame);
	serializer.syncAsSint32LE(_walkedDistance);
	syncPoint(serializer, _sourcePos);
	syncPoint(serializer, _currentPos);
	serializer.syncAsByte(_isWalking);
	syncStack(serializer, _pathPoints, syncPoint);
	syncEnum(serializer, _direction);
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

void MainCharacter::update() {
	if (_relatedProcessCounter == 0)
		_currentlyUsingObject = nullptr;
	WalkingCharacter::update();

	const int16 halfWidth = (int16)(60 * _graphicNormal.depthScale());
	const int16 height = (int16)(310 * _graphicNormal.depthScale());
	shape()->setAsRectangle(Rect(
		_currentPos.x - halfWidth, _currentPos.y - height,
		_currentPos.x + halfWidth, _currentPos.y));

	// TODO: Update character alpha tint
}

void MainCharacter::onArrived() {
	if (_activateObject == nullptr)
		return;

	ITriggerableObject *activateObject = _activateObject;
	const char *activateAction = _activateAction;
	_activateObject = nullptr;
	_activateAction = nullptr;

	stopWalkingAndTurn(activateObject->interactionDirection());
	if (g_engine->world().activeCharacter() == this)
		activateObject->trigger(activateAction);
}

void MainCharacter::walkTo(
	const Point &target, Direction endDirection,
	ITriggerableObject *activateObject, const char *activateAction) {
	_activateObject = activateObject;
	_activateAction = activateAction;

	// TODO: Add collision avoidance

	WalkingCharacter::walkTo(target, endDirection, activateObject, activateAction);
	if (this == g_engine->world().activeCharacter()) {
		// TODO: Add camera following character
	}
}

void MainCharacter::draw() {
	if (this == &g_engine->world().mortadelo()) {
		if (_currentPos.y <= g_engine->world().filemon()._currentPos.y) {
			g_engine->world().mortadelo().drawInner();
			g_engine->world().filemon().drawInner();
		}
		else {
			g_engine->world().filemon().drawInner();
			g_engine->world().mortadelo().drawInner();
		}
	}
}

void MainCharacter::drawInner() {
	if (room() != g_engine->world().currentRoom() || !isEnabled())
		return;
	Graphic *activeGraphic = graphicOf(_curAnimateObject);
	if (activeGraphic == nullptr && _isWalking) {
		activeGraphic = &_graphicNormal;
		_graphicNormal.premultiplyAlpha() = room()->characterAlphaPremultiplier();
	}
	if (activeGraphic == nullptr) {
		activeGraphic = graphicOf(_curTalkingObject, &_graphicTalking);
		_graphicTalking.premultiplyAlpha() = room()->characterAlphaPremultiplier();
	}

	assert(activeGraphic != nullptr);
	activeGraphic->color() = kWhite; // TODO: Add and use character color
	g_engine->drawQueue().add<AnimationDrawRequest>(*activeGraphic, true, BlendMode::AdditiveAlpha, _lodBias);

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
