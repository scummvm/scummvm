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

#include "alcachofa/objects.h"
#include "alcachofa/rooms.h"
#include "alcachofa/script.h"
#include "alcachofa/global-ui.h"
#include "alcachofa/alcachofa.h"

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

void Item::draw() {
	if (!isEnabled())
		return;
	Item *heldItem = g_engine->player().heldItem();
	if (heldItem == nullptr || !heldItem->name().equalsIgnoreCase(name()))
		GraphicObject::draw();
}

void Item::trigger() {
	auto &player = g_engine->player();
	auto &heldItem = player.heldItem();
	if (g_engine->input().wasMouseRightReleased()) {
		if (heldItem == nullptr)
			player.triggerObject(this, "MIRAR");
		else
			heldItem = nullptr;
	} else if (heldItem == nullptr)
		heldItem = this;
	else if (g_engine->script().hasProcedure(name(), heldItem->name()) ||
		!g_engine->script().hasProcedure(heldItem->name(), name()))
		player.triggerObject(this, heldItem->name().c_str());
	else
		player.triggerObject(heldItem, name().c_str());
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
	ObjectBase *related = room()->getObjectByName(_relatedObject.c_str());
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
	switch (_interactionDirection) {
	case Direction::Up:
		return CursorType::LeaveUp;
	case Direction::Right:
		return CursorType::LeaveRight;
	case Direction::Down:
		return CursorType::LeaveDown;
	case Direction::Left:
		return CursorType::LeaveLeft;
	default:
		assert(false && "Invalid door character direction");
		return fromObject;
	}
}

void Door::onClick() {
	if (g_engine->getMillis() - _lastClickTime < 500 && g_engine->player().activeCharacter()->clearTargetIf(this))
		trigger(nullptr);
	else {
		InteractableObject::onClick();
		_lastClickTime = g_engine->getMillis();
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
		animateGraphic->topLeft() = Point(0, 0);
		animateGraphic->update();
	} else if (_isTalking)
		updateTalkingAnimation();
	else if (g_engine->world().somebodyUsing(this)) {
		Graphic *talkGraphic = graphicOf(_curTalkingObject, &_graphicTalking);
		talkGraphic->start(true);
		talkGraphic->pause();
		talkGraphic->update();
	} else
		_graphicNormal.update();
}

void Character::updateTalkingAnimation() {
	Graphic *talkGraphic = graphicOf(_curTalkingObject, &_graphicTalking);
	if (!_isTalking) {
		talkGraphic->reset();
		return;
	}
	if (talkGraphic == &_graphicTalking && !_isSpeaking)
		talkGraphic->reset();
	talkGraphic->update();
}

void Character::draw() {
	if (!isEnabled())
		return;
	Graphic *activeGraphic = graphic();
	assert(activeGraphic != nullptr);
	if (activeGraphic->hasAnimation())
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

void Character::syncGame(Serializer &serializer) {
	ShapeObject::syncGame(serializer);
	serializer.syncAsByte(_isTalking);
	serializer.syncAsSint32LE(_curDialogId);
	_graphicNormal.syncGame(serializer);
	_graphicTalking.syncGame(serializer);
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
			object = room()->getObjectByName(name.c_str());
			if (object == nullptr)
				object = room()->world().getObjectByName(name.c_str());
			if (object == nullptr)
				g_engine->game().unknownSerializedObject(
					name.c_str(), this->name().c_str(), room()->name().c_str());
		}
	}
}

Graphic *Character::graphic() {
	Graphic *activeGraphic = graphicOf(_curAnimateObject);
	if (activeGraphic == nullptr && (_isTalking || g_engine->world().somebodyUsing(this)))
		activeGraphic = graphicOf(_curTalkingObject, &_graphicTalking);
	if (activeGraphic == nullptr)
		activeGraphic = &_graphicNormal;
	return activeGraphic;
}

void Character::onClick() {
	ITriggerableObject::onClick();
	onHoverUpdate();
}

void Character::trigger(const char *action) {
	g_engine->player().activeCharacter()->stopWalking(_interactionDirection);
	if (g_engine->game().shouldCharacterTrigger(this, action))
		g_engine->player().triggerObject(this, action);
}

struct SayTextTask final : public Task {
	SayTextTask(Process &process, Character *character, int32 dialogId)
		: Task(process)
		, _character(character)
		, _dialogId(dialogId) {}

	SayTextTask(Process &process, Serializer &s)
		: Task(process) {
		syncGame(s);
	}

	TaskReturn run() override {
		bool isSoundStillPlaying;

		TASK_BEGIN;
		_character->_isTalking = true;
		graphicOf(_character->_curTalkingObject, &_character->_graphicTalking)->start(true);
		while (true) {
			g_engine->player().addLastDialogCharacter(_character);

			if (_soundHandle == SoundHandle {}) {
				bool hasMortadeloVoice = g_engine->game().hasMortadeloVoice(_character);
				_soundHandle = g_engine->sounds().playVoice(
					String::format(hasMortadeloVoice ? "M%04d" : "%04d", _dialogId),
					0);
			}
			isSoundStillPlaying = g_engine->sounds().isAlive(_soundHandle);
			g_engine->sounds().setAppropriateVolume(_soundHandle, process().character(), _character);
			if (!isSoundStillPlaying || g_engine->input().wasAnyMouseReleased())
				_character->_isTalking = false;

			if (g_engine->config().subtitles() &&
				process().isActiveForPlayer()) {
				g_engine->drawQueue().add<TextDrawRequest>(
					g_engine->globalUI().dialogFont(),
					g_engine->world().getDialogLine(_dialogId),
					Point(g_system->getWidth() / 2, g_system->getHeight() - 200),
					-1, true, kWhite, -kForegroundOrderCount);
			}

			if (!_character->_isTalking) {
				g_engine->sounds().fadeOut(_soundHandle, 100);
				TASK_WAIT(1, delay(200));
				TASK_RETURN(0);
			}

			_character->isSpeaking() = !isSoundStillPlaying ||
				g_engine->sounds().isNoisy(_soundHandle, 80.0f, 150.0f);
			TASK_YIELD(2);
		}
		TASK_END;
	}

	void debugPrint() override {
		g_engine->console().debugPrintf("SayText %s, %d\n", _character->name().c_str(), _dialogId);
	}

	void syncGame(Serializer &s) override {
		Task::syncGame(s);
		syncObjectAsString(s, _character);
		s.syncAsSint32LE(_dialogId);
	}

	const char *taskName() const override;

private:
	Character *_character = nullptr;
	int32 _dialogId = 0;
	SoundHandle _soundHandle = {};
};
DECLARE_TASK(SayTextTask)

Task *Character::sayText(Process &process, int32 dialogId) {
	return new SayTextTask(process, this, dialogId);
}

void Character::resetTalking() {
	_isTalking = false;
	_curDialogId = -1;
	_curTalkingObject = nullptr;
}

void Character::talkUsing(ObjectBase *talkObject) {
	_curTalkingObject = talkObject;
	if (talkObject == nullptr)
		return;
	auto graphic = talkObject->graphic();
	if (graphic == nullptr)
		error("Talk object %s does not have a graphic", talkObject->name().c_str());
	graphic->start(true);
	if (room() == g_engine->player().currentRoom())
		graphic->update();
}

struct AnimateCharacterTask final : public Task {
	AnimateCharacterTask(Process &process, Character *character, ObjectBase *animateObject)
		: Task(process)
		, _character(character)
		, _animateObject(animateObject)
		, _graphic(animateObject->graphic()) {
		scumm_assert(_graphic != nullptr);
	}

	AnimateCharacterTask(Process &process, Serializer &s)
		: Task(process) {
		syncGame(s);
	}

	TaskReturn run() override {
		TASK_BEGIN;
		while (_character->_curAnimateObject != nullptr)
			TASK_YIELD(1);

		_character->_curAnimateObject = _animateObject;
		_graphic->start(false);
		if (_character->room() == g_engine->player().currentRoom())
			_graphic->update();
		do {
			TASK_YIELD(2);
			if (process().isActiveForPlayer() && g_engine->input().wasAnyMouseReleased())
				_graphic->pause();
		} while (!_graphic->isPaused());

		_character->_curAnimateObject = nullptr;
		_character->_curTalkingObject = nullptr;
		TASK_END;
	}

	void debugPrint() override {
		g_engine->console().debugPrintf("AnimateCharacter %s, %s\n", _character->name().c_str(), _animateObject->name().c_str());
	}

	void syncGame(Serializer &s) override {
		Task::syncGame(s);
		syncObjectAsString(s, _character);
		syncObjectAsString(s, _animateObject);
		_graphic = _animateObject->graphic();
		scumm_assert(_graphic != nullptr);
	}

	const char *taskName() const override;

private:
	Character *_character = nullptr;
	ObjectBase *_animateObject = nullptr;
	Graphic *_graphic = nullptr;
};
DECLARE_TASK(AnimateCharacterTask)

Task *Character::animate(Process &process, ObjectBase *animateObject) {
	assert(animateObject != nullptr);
	return new AnimateCharacterTask(process, this, animateObject);
}

struct LerpLodBiasTask final : public Task {
	LerpLodBiasTask(Process &process, Character *character, float targetLodBias, uint32 durationMs)
		: Task(process)
		, _character(character)
		, _targetLodBias(targetLodBias)
		, _durationMs(durationMs) {}

	LerpLodBiasTask(Process &process, Serializer &s)
		: Task(process) {
		syncGame(s);
	}

	TaskReturn run() override {
		TASK_BEGIN;
		_startTime = g_engine->getMillis();
		_sourceLodBias = _character->lodBias();
		while (g_engine->getMillis() - _startTime < _durationMs) {
			_character->lodBias() = _sourceLodBias + (_targetLodBias - _sourceLodBias) *
				((g_engine->getMillis() - _startTime) / (float)_durationMs);
			TASK_YIELD(1);
		}
		_character->lodBias() = _targetLodBias;
		TASK_END;
	}

	void debugPrint() override {
		uint32 remaining = g_engine->getMillis() - _startTime <= _durationMs
			? _durationMs - (g_engine->getMillis() - _startTime)
			: 0;
		g_engine->console().debugPrintf("Lerp lod bias of %s to %f with %ums remaining\n",
			_character->name().c_str(), _targetLodBias, remaining);
	}

	void syncGame(Serializer &s) override {
		Task::syncGame(s);
		syncObjectAsString(s, _character);
		s.syncAsFloatLE(_sourceLodBias);
		s.syncAsFloatLE(_targetLodBias);
		s.syncAsUint32LE(_startTime);
		s.syncAsUint32LE(_durationMs);
	}

	const char *taskName() const override;

private:
	Character *_character = nullptr;
	float _sourceLodBias = 0, _targetLodBias = 0;
	uint32 _startTime = 0, _durationMs = 0;
};
DECLARE_TASK(LerpLodBiasTask)

Task *Character::lerpLodBias(Process &process, float targetLodBias, int32 durationMs) {
	return new LerpLodBiasTask(process, this, targetLodBias, durationMs);
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
			_sourcePos = _currentPos = activeFloor->closestPointTo(_sourcePos);
		if (activeFloor->polygonContaining(_currentPos) < 0)
			_currentPos = activeFloor->closestPointTo(_currentPos);
	}

	if (!_isWalking) {
		_graphicTalking.setAnimation(talkingAnimation());
		updateTalkingAnimation();
		_currentPos = _sourcePos;
	}

	_graphicNormal.topLeft() = _graphicTalking.topLeft() = _currentPos;
	auto animateGraphic = graphicOf(_curAnimateObject);
	auto talkingGraphic = graphicOf(_curTalkingObject);
	if (animateGraphic != nullptr)
		animateGraphic->topLeft() = _currentPos;
	if (talkingGraphic != nullptr)
		talkingGraphic->topLeft() = _currentPos;
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

static Direction getDirection(Point from, Point to) {
	Point delta = from - to;
	if (from.x == to.x)
		return from.y < to.y ? Direction::Down : Direction::Up;
	else if (from.x < to.x) {
		int slope = 1000 * delta.y / -delta.x;
		return slope > 1000 ? Direction::Up
			: slope < -1000 ? Direction::Down
			: Direction::Right;
	} else { // from.x > to.x
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
	} else {
		updateWalkingAnimation();
		const int32 distanceToTarget = (int32)(sqrtf(_sourcePos.sqrDist(targetPos)));
		if (_walkedDistance < distanceToTarget) {
			// separated because having only 16 bits and multiplications seems dangerous
			_currentPos.x = _sourcePos.x + _walkedDistance * (targetPos.x - _sourcePos.x) / distanceToTarget;
			_currentPos.y = _sourcePos.y + _walkedDistance * (targetPos.y - _sourcePos.y) / distanceToTarget;
		} else {
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
	_graphicNormal.topLeft() = _currentPos;
}

void WalkingCharacter::updateWalkingAnimation() {
	_direction = getDirection(_sourcePos, _pathPoints.top());
	auto animation = walkingAnimation();
	_graphicNormal.setAnimation(animation);

	// this is very confusing. Let's see what it does
	const int32 halfFrameCount = (int32)animation->frameCount() / 2;
	int32 expectedFrame = (int32)(g_engine->getMillis() - _graphicNormal.lastTime()) * 12 / 1000;
	const bool isUnexpectedFrame = expectedFrame != _lastWalkAnimFrame;
	int32 stepFrameFrom, stepFrameTo;
	if (expectedFrame < halfFrameCount - 1) {
		_lastWalkAnimFrame = expectedFrame;
		stepFrameFrom = 2 * expectedFrame - 2;
		stepFrameTo = 2 * expectedFrame;
	} else {
		const int32 frameThreshold = _lastWalkAnimFrame <= halfFrameCount - 1
			? _lastWalkAnimFrame
			: (_lastWalkAnimFrame - halfFrameCount + 1) % (halfFrameCount - 2) + 1;
		_lastWalkAnimFrame = expectedFrame;
		expectedFrame = (expectedFrame - halfFrameCount + 1) % (halfFrameCount - 2) + 1;
		if (expectedFrame >= frameThreshold) {
			stepFrameFrom = 2 * expectedFrame - 2;
			stepFrameTo = 2 * expectedFrame;
		} else {
			stepFrameFrom = 2 * (halfFrameCount - 2);
			stepFrameTo = 2 * halfFrameCount - 2;
		}
	}
	if (isUnexpectedFrame) {
		const float stepSize = sqrtf(animation->frameCenter(stepFrameFrom).sqrDist(animation->frameCenter(stepFrameTo)));
		_walkedDistance += (int32)(stepSize * _stepSizeFactor);
	}
	_graphicNormal.frameI() = 2 * expectedFrame; // especially this: wtf?
}

void WalkingCharacter::onArrived() {}

void WalkingCharacter::stopWalking(Direction direction) {
	// be careful, the original engine had two versions of this method
	// one without resetting _sourcePos
	_isWalking = false;
	_sourcePos = _currentPos;
	if (direction != Direction::Invalid)
		_direction = direction;
}

void WalkingCharacter::walkTo(
	Point target, Direction endDirection,
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

void WalkingCharacter::setPosition(Point target) {
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
		// The original game drew the current dialog line at this point,
		// but I do not know of a scenario where this would be necessary
		// As long as we cannot test this or have a bug report I rather not implement it

		currentGraphic = graphicOf(_curTalkingObject, &_graphicTalking);
	}

	assert(currentGraphic != nullptr);
	g_engine->drawQueue().add<AnimationDrawRequest>(*currentGraphic, true, BlendMode::AdditiveAlpha, _lodBias);
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

void WalkingCharacter::syncGame(Serializer &serializer) {
	Character::syncGame(serializer);
	serializer.syncAsSint32LE(_lastWalkAnimFrame);
	serializer.syncAsSint32LE(_walkedDistance);
	syncPoint(serializer, _sourcePos);
	syncPoint(serializer, _currentPos);
	serializer.syncAsByte(_isWalking);
	syncStack(serializer, _pathPoints, syncPoint);
	syncEnum(serializer, _direction);
}

struct ArriveTask : public Task {
	ArriveTask(Process &process, const WalkingCharacter *character)
		: Task(process)
		, _character(character) {}

	ArriveTask(Process &process, Serializer &s)
		: Task(process) {
		syncGame(s);
	}

	TaskReturn run() override {
		return _character->isWalking()
			? TaskReturn::yield()
			: TaskReturn::finish(1);
	}

	void debugPrint() override {
		g_engine->getDebugger()->debugPrintf("Wait for %s to arrive", _character->name().c_str());
	}

	void syncGame(Serializer &s) override {
		syncObjectAsString(s, _character);
	}

	const char *taskName() const override;
private:
	const WalkingCharacter *_character = nullptr;
};
DECLARE_TASK(ArriveTask)

Task *WalkingCharacter::waitForArrival(Process &process) {
	return new ArriveTask(process, this);
}

const char *MainCharacter::typeName() const { return "MainCharacter"; }

MainCharacter::MainCharacter(Room *room, ReadStream &stream)
	: WalkingCharacter(room, stream)
	, _semaphore(name().firstChar() == 'M' ? "mortadelo" : "filemon") {
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

	// These are set as members as FloorColor might want to change them
	_alphaPremultiplier = room()->characterAlphaPremultiplier();
	_color = { 255, 255, 255, (uint8)(room()->characterAlphaTint() * 255 / 100) };
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
	Point target_, Direction endDirection,
	ITriggerableObject *activateObject, const char *activateAction) {
	_activateObject = activateObject;
	_activateAction = activateAction;
	Point target = target_;

	Point evadeTarget = target;
	const PathFindingShape *activeFloor = room()->activeFloor();
	if (activeFloor != nullptr && activeFloor->findPath(_currentPos, target, _pathPoints))
		evadeTarget = _pathPoints[0];

	MainCharacter *otherCharacter = &g_engine->world().getOtherMainCharacterByKind(_kind);
	Point otherTarget = otherCharacter->_currentPos;
	if (otherCharacter->isWalking() && !otherCharacter->_pathPoints.empty())
		otherTarget = otherCharacter->_pathPoints[0];

	const float activeDepthScale = g_engine->player().activeCharacter()->_graphicNormal.depthScale();
	const float avoidanceDistSqr = pow(75 * activeDepthScale, 2);
	const bool willIBeBusy =
		_activateObject != nullptr &&
		strcmp(_activateAction, "MIRAR") != 0 &&
		otherCharacter->currentlyUsing() != dynamic_cast<ObjectBase *>(_activateObject);

	if (otherCharacter->room() == room() && evadeTarget.sqrDist(otherTarget) <= avoidanceDistSqr) {
		if (!otherCharacter->isBusy()) {
			if (activeFloor != nullptr && activeFloor->findEvadeTarget(evadeTarget, activeDepthScale, avoidanceDistSqr, evadeTarget))
				otherCharacter->WalkingCharacter::walkTo(evadeTarget);
		} else if (!willIBeBusy) {
			if (activeFloor != nullptr)
				activeFloor->findEvadeTarget(evadeTarget, activeDepthScale, avoidanceDistSqr, target);
		}
	}

	WalkingCharacter::walkTo(target, endDirection, activateObject, activateAction);
	if (this == g_engine->player().activeCharacter())
		g_engine->camera().setFollow(this);
}

void MainCharacter::draw() {
	if (this == &g_engine->world().mortadelo()) {
		if (_currentPos.y <= g_engine->world().filemon()._currentPos.y) {
			g_engine->world().mortadelo().drawInner();
			g_engine->world().filemon().drawInner();
		} else {
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
		_graphicNormal.premultiplyAlpha() = _alphaPremultiplier;
	}
	if (activeGraphic == nullptr) {
		activeGraphic = graphicOf(_curTalkingObject, &_graphicTalking);
		_graphicTalking.premultiplyAlpha() = _alphaPremultiplier;
	}

	assert(activeGraphic != nullptr);
	activeGraphic->color() = _color;
	g_engine->drawQueue().add<AnimationDrawRequest>(*activeGraphic, true, BlendMode::AdditiveAlpha, _lodBias);
}

void syncDialogMenuLine(Serializer &serializer, DialogMenuLine &line) {
	serializer.syncAsSint32LE(line._dialogId);
	serializer.syncAsSint32LE(line._yPosition);
	serializer.syncAsSint32LE(line._returnValue);
}

void MainCharacter::syncGame(Serializer &serializer) {
	String roomName = room()->name();
	serializer.syncString(roomName);
	if (serializer.isLoading()) {
		room() = room()->world().getRoomByName(roomName.c_str());
		if (room() == nullptr)
			// no good way to recover from this
			error("Invalid room name \"%s\" saved for \"%s\"", roomName.c_str(), name().c_str());
	}

	WalkingCharacter::syncGame(serializer);
	FakeSemaphore::sync(serializer, _semaphore);
	syncArray(serializer, _dialogLines, syncDialogMenuLine);
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
	if (g_engine->player().activeCharacter() == this)
		g_engine->player().heldItem() = nullptr;
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
	if (item == nullptr) {
		g_engine->game().unknownPickupItem(name.c_str());
		return;
	}
	item->toggle(true);
	if (g_engine->player().activeCharacter() == this) {
		if (putInHand)
			g_engine->player().heldItem() = item;
		g_engine->world().inventory().updateItemsByActiveCharacter();
	}
}

void MainCharacter::drop(const Common::String &name) {
	if (!name.empty()) {
		auto item = getItemByName(name);
		if (item == nullptr)
			g_engine->game().unknownDropItem(name.c_str());
		else
			item->toggle(false);
	}
	if (g_engine->player().activeCharacter() == this) {
		g_engine->player().heldItem() = nullptr;
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

struct DialogMenuTask : public Task {
	DialogMenuTask(Process &process, MainCharacter *character)
		: Task(process)
		, _input(g_engine->input())
		, _character(character) {}

	DialogMenuTask(Process &process, Serializer &s)
		: Task(process)
		, _input(g_engine->input()) {
		syncGame(s);
	}

	TaskReturn run() override {
		TASK_BEGIN;
		layoutLines();
		while (true) {
			TASK_YIELD(1);
			if (g_engine->player().activeCharacter() != _character)
				continue;
			g_engine->globalUI().updateChangingCharacter();
			g_engine->player().heldItem() = nullptr;
			g_engine->player().drawCursor();

			_clickedLineI = updateLines();
			if (_clickedLineI != UINT_MAX) {
				TASK_YIELD(2);
				TASK_WAIT(3, _character->sayText(process(), _character->_dialogLines[_clickedLineI]._dialogId));
				int32 returnValue = _character->_dialogLines[_clickedLineI]._returnValue;
				_character->_dialogLines.clear();
				TASK_RETURN(returnValue);
			}
		}
		TASK_END;
	}

	void debugPrint() override {
		g_engine->console().debugPrintf("DialogMenu for %s with %u lines\n",
			_character->name().c_str(), _character->_dialogLines.size());
	}

	void syncGame(Serializer &s) override {
		Task::syncGame(s);
		syncObjectAsString(s, _character);
		s.syncAsUint32LE(_clickedLineI);
	}

	const char *taskName() const override;

private:
	static constexpr int kTextXOffset = 5;
	static constexpr int kTextYOffset = 10;
	inline int maxTextWidth() const {
		return g_system->getWidth() - 2 * kTextXOffset;
	}

	void layoutLines() {
		auto &lines = _character->_dialogLines;
		for (auto &itLine : lines) {
			// we reuse the draw request to measure the actual height without using it to actually draw
			TextDrawRequest request(
				g_engine->globalUI().dialogFont(),
				g_engine->world().getDialogLine(itLine._dialogId),
				Point(kTextXOffset, 0), maxTextWidth(), false, kWhite, 2);
			itLine._yPosition = request.size().y; // briefly storing line height
		}

		lines.back()._yPosition = g_system->getHeight() - kTextYOffset - lines.back()._yPosition;
		for (uint i = lines.size() - 1; i > 0; i--)
			lines[i - 1]._yPosition = lines[i]._yPosition - kTextYOffset - lines[i - 1]._yPosition;
	}

	uint updateLines() {
		bool isSomethingHovered = false;
		for (uint i = _character->_dialogLines.size(); i > 0; i--) {
			auto &itLine = _character->_dialogLines[i - 1];
			bool isHovered = !isSomethingHovered && _input.mousePos2D().y >= itLine._yPosition - kTextYOffset;
			g_engine->drawQueue().add<TextDrawRequest>(
				g_engine->globalUI().dialogFont(),
				g_engine->world().getDialogLine(itLine._dialogId),
				Point(kTextXOffset, itLine._yPosition),
				maxTextWidth(), false, isHovered ? Color { 255, 255, 128, 255 } : kWhite, -kForegroundOrderCount + 2);
			isSomethingHovered = isSomethingHovered || isHovered;
			if (isHovered && _input.wasMouseLeftReleased())
				return i - 1;
		}
		return UINT_MAX;
	}

	Input &_input;
	MainCharacter *_character = nullptr;
	uint _clickedLineI = UINT_MAX;
};
DECLARE_TASK(DialogMenuTask)

void MainCharacter::addDialogLine(int32 dialogId) {
	assert(dialogId >= 0);
	DialogMenuLine line;
	line._dialogId = dialogId;
	_dialogLines.push_back(line);
}

void MainCharacter::setLastDialogReturnValue(int32 returnValue) {
	if (_dialogLines.empty())
		error("Tried to set return value of non-existent dialog line");
	_dialogLines.back()._returnValue = returnValue;
}

Task *MainCharacter::dialogMenu(Process &process) {
	if (_dialogLines.empty())
		error("Tried to open dialog menu without any lines set");
	return new DialogMenuTask(process, this);
}

void MainCharacter::resetUsingObjectAndDialogMenu() {
	_currentlyUsingObject = nullptr;
	_dialogLines.clear();
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

void FloorColor::update() {
	auto updateFor = [&] (MainCharacter &character) {
		if (character.room() == room()) {
			const auto result = _shape.colorAt(character.position());
			if (result.first)
				character.color() = { 255, 255, 255, result.second.a };
		}
	};
	updateFor(g_engine->world().mortadelo());
	updateFor(g_engine->world().filemon());
}

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
