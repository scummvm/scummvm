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
#include "alcachofa.h"

using namespace Common;
using namespace Math;

namespace Alcachofa {

const char *Item::typeName() const { return "Item"; }

Item::Item(Room *room, ReadStream &stream)
	: GraphicObject(room, stream) {
	stream.readByte(); // unused and ignored byte
}

Item::Item(const Item &other)
	: GraphicObject(other.room(), other.name().c_str()) {
	_type = other._type;
	_posterizeAlpha = other._posterizeAlpha;
	_graphic.~Graphic();
	new (&_graphic) Graphic(other._graphic);
}

ITriggerableObject::ITriggerableObject(ReadStream &stream)
	: _interactionPoint(Shape(stream).firstPoint())
	, _interactionDirection((Direction)stream.readSint32LE()) {}

void ITriggerableObject::onClick() {
	auto heldItem = g_engine->player().heldItem();
	const char *action;
	if (heldItem == nullptr)
		action = g_engine->input().wasMouseLeftReleased() ? "MIRAR" : "PULSAR";
	else
		action = heldItem->name().c_str();
	g_engine->player().activeCharacter()->walkTo(_interactionPoint, Direction::Invalid, this, action);
}

const char *InteractableObject::typeName() const { return "InteractableObject"; }

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

void InteractableObject::onClick() {
	ITriggerableObject::onClick();
	onHoverUpdate();
}

void InteractableObject::trigger(const char *action) {
	g_engine->player().activeCharacter()->stopWalking();
	g_engine->player().triggerObject(this, action);
}

void InteractableObject::toggle(bool isEnabled) {
	ObjectBase::toggle(isEnabled);
	ObjectBase *related = room()->getObjectByName(_relatedObject);
	if (related != nullptr)
		related->toggle(isEnabled);
}

const char *Door::typeName() const { return "Door"; }

Door::Door(Room *room, ReadStream &stream)
	: InteractableObject(room, stream)
	, _targetRoom(readVarString(stream))
	, _targetObject(readVarString(stream))
	, _characterDirection((Direction)stream.readSint32LE()) {
	_targetRoom.replace(' ', '_');
}

CursorType Door::cursorType() const {
	CursorType fromObject = ShapeObject::cursorType();
	if (fromObject != CursorType::Point)
		return fromObject;
	switch (_characterDirection) {
	case Direction::Up: return CursorType::LeaveUp;
	case Direction::Right: return CursorType::LeaveRight;
	case Direction::Down: return CursorType::LeaveDown;
	case Direction::Left: return CursorType::LeaveLeft;
	default: assert(false && "Invalid door character direction"); return fromObject;
	}
}

void Door::onClick() {
	if (g_system->getMillis() - _lastClickTime < 500 && g_engine->player().activeCharacter()->clearTargetIf(this))
		trigger(nullptr);
	else {
		InteractableObject::onClick();
		_lastClickTime = g_system->getMillis();
	}
}

void Door::trigger(const char *_) {
	g_engine->player().triggerDoor(this);
}

const char *Character::typeName() const { return "Character"; }

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

void Character::onClick() {
	ITriggerableObject::onClick();
	onHoverUpdate();
}

void Character::trigger(const char *action) {
	g_engine->player().activeCharacter()->stopWalking(_interactionDirection);
	if (scumm_stricmp(action, "iSABANA") == 0 && // Original hack probably to fix some bug :)
		dynamic_cast<MainCharacter *>(this) != nullptr &&
		room()->name().equalsIgnoreCase("CASA_FREDDY_ARRIBA"))
		error("Not sure what *should* happen. How do we get here?");
	g_engine->player().triggerObject(this, action);
}

struct SayTextTask : public Task {
	SayTextTask(Process &process, Character *character, int32 dialogId)
		: Task(process)
		, _character(character)
		, _dialogId(dialogId) { }

	virtual TaskReturn run() override {
		TASK_BEGIN;
		_character->_isTalking = true;
		graphicOf(_character->_curTalkingObject, &_character->_graphicTalking)->start(true);
		while (true) {
			if (_soundId == kInvalidSoundID)
				_soundId = g_engine->sounds().playVoice(
					String::format(_character == &g_engine->world().mortadelo() ? "M%04d" : "%04d", _dialogId),
					0);
			g_engine->sounds().setAppropriateVolume(_soundId, process().character(), _character);
			if (!g_engine->sounds().isAlive(_soundId) || g_engine->input().wasAnyMouseReleased())
				_character->_isTalking = false;

			if (true && // TODO: Add game option for subtitles
				process().isActiveForPlayer()) {
				g_engine->drawQueue().add<TextDrawRequest>(
					g_engine->world().dialogFont(),
					g_engine->world().getDialogLine(_dialogId),
					Point(g_system->getWidth() / 2, g_system->getHeight() - 200),
					-1, true, kWhite, 0);
			}
			// TODO: Add lip sync for sayText

			if (!_character->_isTalking) {
				g_engine->sounds().fadeOut(_soundId, 100);
				TASK_WAIT(delay(200));
				TASK_RETURN(0);
			}
			TASK_YIELD;
		}
		TASK_END;
	}

	virtual void debugPrint() override {
		g_engine->console().debugPrintf("SayText %s, %d\n", _character->name().c_str(), _dialogId);
	}

private:
	Character *_character;
	int32 _dialogId;
	SoundID _soundId = kInvalidSoundID;
};

Task *Character::sayText(Process &process, int32 dialogId) {
	return new SayTextTask(process, this, dialogId);
}

const char *WalkingCharacter::typeName() const { return "WalkingCharacter"; }

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
	if (this != g_engine->player().activeCharacter()) {
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
		return from.y < to.y ? Direction::Down : Direction::Up;
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

void WalkingCharacter::stopWalking(Direction direction) {
	// be careful, the original engine had two versions of this method
	// one without resetting _sourcePos
	_isWalking = false;
	_sourcePos = _currentPos;
	if (direction != Direction::Invalid)
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

struct ArriveTask : public Task {
	ArriveTask(Process &process, const WalkingCharacter &character)
		: Task(process)
		, _character(character) {}

	virtual TaskReturn run() override {
		return _character.isWalking()
			? TaskReturn::yield()
			: TaskReturn::finish(1);
	}

	virtual void debugPrint() override {
		g_engine->getDebugger()->debugPrintf("Wait for %s to arrive", _character.name().c_str());
	}
private:
	const WalkingCharacter &_character;
};

Task *WalkingCharacter::waitForArrival(Process &process) {
	return new ArriveTask(process, *this);
}

const char *MainCharacter::typeName() const { return "MainCharacter"; }

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

bool MainCharacter::isBusy() const {
	return !_semaphore.isReleased() || !g_engine->player().semaphore().isReleased();
}

void MainCharacter::update() {
	if (_semaphore.isReleased())
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

	stopWalking(activateObject->interactionDirection());
	if (g_engine->player().activeCharacter() == this)
		activateObject->trigger(activateAction);
}

void MainCharacter::walkTo(
	const Point &target, Direction endDirection,
	ITriggerableObject *activateObject, const char *activateAction) {
	_activateObject = activateObject;
	_activateAction = activateAction;

	// TODO: Add collision avoidance

	WalkingCharacter::walkTo(target, endDirection, activateObject, activateAction);
	if (this == g_engine->player().activeCharacter()) {
		g_engine->camera().setFollow(this);
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
	if (room() != g_engine->player().currentRoom() || !isEnabled())
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
	uint semaphoreCounter = _semaphore.counter();
	serializer.syncAsSint32LE(semaphoreCounter);
	_semaphore = FakeSemaphore(semaphoreCounter);
	syncArray(serializer, _dialogMenuLines, syncDialogMenuLine);
	syncObjectAsString(serializer, _currentlyUsingObject);

	for (auto *item : _items) {
		bool isEnabled = item->isEnabled();
		serializer.syncAsByte(isEnabled);
		item->toggle(isEnabled);
	}
}

void MainCharacter::clearInventory() {
	for (auto *item : _items)
		item->toggle(false);
	// TODO: Clear held item on clearInventory
	g_engine->world().inventory().updateItemsByActiveCharacter();
}

Item *MainCharacter::getItemByName(const String &name) const {
	for (auto *item : _items) {
		if (item->name() == name)
			return item;
	}
	return nullptr;
}

bool MainCharacter::hasItem(const String &name) const {
	auto item = getItemByName(name);
	return item == nullptr || item->isEnabled();
}

void MainCharacter::pickup(const String &name, bool putInHand) {
	auto item = getItemByName(name);
	if (item == nullptr)
		error("Tried to pickup unknown item: %s", name.c_str());
	item->toggle(true);
	if (g_engine->player().activeCharacter() == this) {
		// TODO: Put item in hand for pickup
		g_engine->world().inventory().updateItemsByActiveCharacter();
	}
}

void MainCharacter::drop(const Common::String &name) {
	auto item = getItemByName(name);
	if (item == nullptr)
		error("Tried to drop unknown item: %s", name.c_str());
	item->toggle(false);
	if (g_engine->player().activeCharacter() == this) {
		// TODO: Clear held item for drop
		g_engine->world().inventory().updateItemsByActiveCharacter();
	}
}

void MainCharacter::walkToMouse() {
	Point targetPos = g_engine->input().mousePos3D();
	if (room()->activeFloor() != nullptr) {
		// original would be overwriting the current path but this
		// can cause the character teleporting to the new target

		Stack<Point> tmpPath;
		room()->activeFloor()->findPath(_sourcePos, targetPos, tmpPath);
		if (!tmpPath.empty())
			targetPos = tmpPath[0];
	}

	const uint minDistance = (uint)(50 * _graphicNormal.depthScale());
	if (_sourcePos.sqrDist(targetPos) > minDistance * minDistance)
		walkTo(targetPos);
}

bool MainCharacter::clearTargetIf(const ITriggerableObject *target) {
	if (_activateObject == target) {
		_activateObject = nullptr;
		return true;
	}
	return false;
}

const char *Background::typeName() const { return "Background"; }

Background::Background(Room *room, const String &animationFileName, int16 scale)
	: GraphicObject(room, "BACKGROUND") {
	toggle(true);
	_graphic.setAnimation(animationFileName, AnimationFolder::Backgrounds);
	_graphic.scale() = scale;
	_graphic.order() = 59;
}

const char *FloorColor::typeName() const { return "FloorColor"; }

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
