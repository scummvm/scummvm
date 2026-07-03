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

#ifndef MEDIASTATION_MINIGAMES_STALKINGZAZU_H
#define MEDIASTATION_MINIGAMES_STALKINGZAZU_H

#include "mediastation/statemachine.h"
#include "mediastation/actors/stage.h"

namespace MediaStation {

class StalkingZazuActor;
namespace StalkingZazuMinigame {

class Direction {
public:
	static const uint TOTAL_DIRECTIONS = 8;
	enum Value {
		kNone = 0,
		kEast = 1,
		kNorthEast = 2,
		kNorth = 3,
		kNorthWest = 4,
		kWest = 5,
		kSouthWest = 6,
		kSouth = 7,
		kSouthEast = 8
	};

	Direction() {}
	Direction(Value value) : _value(value) {}
	Direction(uint directionToken);

	// Conversion operator so a Direction stands in for its enum value directly.
	operator Value() const { return _value; }

	Direction counterClockwise() const { return rotate(1); }
	Direction opposite() const { return rotate(TOTAL_DIRECTIONS / 2); }

private:
	// Rotates counter-clockwise by the given number of eighth-turns,
	Direction rotate(uint steps) const;
	Value _value = kNone;
};

class CellCoord : public Common::Point {
public:
	CellCoord() : Common::Point() {}
	CellCoord(int16 column, int16 row) : Common::Point(column, row) {}

	CellCoord getAdjacentCell(const Direction &direction) const;
};

class StalkClips : public Common::Array<uint> {
public:
	static const uint SIZE = 18;
	uint indexOf(Direction direction, bool isCrouched);
};

// This is basically a uint16 matrix with a lot of additional methods attached.
// The ScummVM Matrix class was not used because it is float-backed and so not really
// a great choice for an obstacle field. This is really a translation layer a logical
// "stalking field" space that defines whether a given region of graphical space contains
// an obstacle or not.
class StalkingField {
public:
	StalkingField() = default;
	StalkingField(int rowCount, int columnCount, const Common::Point &graphicalSize);

	uint16 &at(int row, int column);
	const uint16 &at(int row, int column) const;
	uint16 &at(const CellCoord &coord);
	const uint16 &at(const CellCoord &coord) const;
	bool isEmpty(const CellCoord &coord) { return at(coord) == 0; }

	bool isInBounds(int row, int column) const;
	bool isInBounds(const CellCoord &coord) const;

	int rowCount() const { return _rows; }
	int columnCount() const { return _columns; }

	void clear();
	Common::Point bottomCenterPositionInFieldCoordinates();
	CellCoord cellAtPoint(const Common::Point &point);
	void placeObstacles(const Common::Array<uint> &obstacleActorIds);
	Common::Point cellSize();
	Common::Point centerOfCell(const CellCoord &coord);

private:
	bool addObstacleToRow(uint16 obstacleActorId, uint16 row);
	Common::Array<CellCoord> calcAvailableCellsOnRow(uint row);
	Common::Array<uint16> calcPartitionBoundaries(uint end, uint brk); // calcBreakPoints

	bool obstacleCanBePlaced(const CellCoord &coord); // cellIsValidObstacle
	bool obstacleCanBePlaced(CellCoord cellCoord, Direction direction, Common::Array<CellCoord> &cellCoordVector);

	int _columns = 0;
	int _rows = 0;
	Common::Array<uint16> _values;
	Common::Point _graphicalSize;
};

enum GameState {
	kStalkingZazuStateStart = 0,
	kStalkingZazuStateReset = 1,
	kStalkingZazuStateCrouched = 2,
	kStalkingZazuStateCanMove = 3,
	kStalkingZazuStateProcessMove = 4,
	kStalkingZazuStateDefeat = 5,
	kStalkingZazuStateSuccess = 6
};

enum GameStateEvent {
	kStalkingZazuResetStateEvent = 0,
	kStalkingZazuCrouchStateEvent = 1,
	kStalkingZazuUncrouchStateEvent = 2,
	kStalkingZazuMoveStateEvent = 3,
	kStalkingZazuLoseStateEvent = 4,
	kStalkingZazuWinStateEvent = 5
};

struct GameStateMachineEvent : public StateMachineEvent<GameState> {
public:
	uint obstaclesToShow = 0;
};

class GameStateMachine : public StateMachine<GameState, GameStateEvent> {
public:
	GameStateMachine(StalkingZazuActor *actor) : _actor(actor) { _currentState = kStalkingZazuStateStart; }

protected:
	virtual void executeNextState(GameStateEvent eventType) override;
	StalkingZazuActor *_actor = nullptr;
};

enum TimerState {
	kStalkingZazuExpiredTimerState = 0,
	kStalkingZazuQueuedTimerState = 1,
	kStalkingZazuExpiredInactiveTimerState = 2,
	kStalkingZazuExpiredActiveTimerState = 3,
	kStalkingZazuQueuedInactiveTimerState = 4
};

enum TimerEvent {
	kStalkingZazuActivateTimerEvent = 0,
	kStalkingZazuInactivateTimerEvent = 1,
	kStalkingZazuExpireTimerEvent = 2,
	kStalkingZazuQueueTimerEvent = 3
};

class TimerStateMachine : public StateMachine<TimerState, TimerEvent> {
public:
	TimerStateMachine(StalkingZazuActor *actor) : _actor(actor) { _currentState = kStalkingZazuExpiredTimerState; }

protected:
	virtual void executeNextState(TimerEvent eventType) override;
	StalkingZazuActor *_actor = nullptr;
};

}; // End of namespace StalkingZazuMinigame

class StalkingZazuActor : public SpatialEntity {
friend class StalkingZazuMinigame::GameStateMachine;
friend class StalkingZazuMinigame::TimerStateMachine;

public:
	StalkingZazuActor() : SpatialEntity(kActorTypeStalkingZazu), _gameMachine(this), _timerMachine(this) {};

	virtual Common::String debugString() const override;
	virtual void readParameter(Chunk &chunk, ActorHeaderSectionType paramType) override;
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) override;
	virtual bool isVisible() const override;
	virtual void draw(DisplayContext &displayContext) override;
	virtual void loadIsComplete() override;

	virtual void onEvent(const ActorEvent &event) override;
	virtual void timerEvent(const MediaStation::TimerEvent &event) override;

private:
	StalkingZazuMinigame::GameStateMachine _gameMachine;
	void generateStalkingEvent(StalkingZazuMinigame::GameStateEvent event);
	StalkingZazuMinigame::TimerStateMachine _timerMachine;
	void generateTimerEvent(StalkingZazuMinigame::TimerEvent event);

	void calcWhichObstaclesToShow(uint16 obstaclesToShow);
	void calcObstaclePlacements();
	void placeObstaclesOnStage();
	void positionSimbaStartOnStage();

	bool simbaIsInSuccessPosition();
	void setSimbaCrouchClip(bool isSimbaCrouching);
	void setSimbaClipFromDirection(StalkingZazuMinigame::Direction direction);
	bool zazuSeesSimba();
	bool simbaIsBehindRock();

	void placeAndShowActor(uint obstacleActorId, const Common::Point &pos);
	void setClipOfMovie(uint spriteActorId, uint clipId);
	int zCoordOfPoint(const Common::Point &point);

	void processMove(Common::Point dest);
	bool getMoveInformation(const Common::Point &targetPos, Common::Point &nextPos, StalkingZazuMinigame::Direction &direction);
	StalkingZazuMinigame::Direction directionOfVector(const Common::Point &dest);
	Common::Point desiredPositionFromVectorInFieldCoordinates(const Common::Point &point);
	bool isClearAtPoint(const Common::Point &point);

	StalkingZazuMinigame::Direction directionZazuWillLook();
	void setZazuLook(bool isLooking);
	void setActive(bool isActive);

	void stalkingState_1_reset();
	void stalkingState_2_crouched();
	void stalkingState_3_canMove();
	void stalkingState_4_processMove();
	void stalkingState_5_defeat();
	void stalkingState_6_success();
	void timerState_0_expired();
	void timerState_1_queued();
	// The original has a few other timer states that are no-ops,
	// so they are excluded here.

	virtual bool interactsWithMouse() const override { return _isActive; }
	virtual uint16 findActorToAcceptMouseEvents(
		const Common::Point &point,
		uint16 eventMask,
		MouseActorState &state,
		bool clipMouseEvents) override;
	bool mousePositionIsInteresting(const Common::Point &pos);
	void mouseDownEvent(const MouseEvent &event) override;
	void mouseUpEvent(const MouseEvent &event) override;
	void mouseEnteredEvent(const MouseEvent &event) override;
	void mouseExitedEvent(const MouseEvent &event) override;
	bool _mouseIsDown = false;
	bool _mouseIsInside = false;

	StalkingZazuMinigame::StalkingField _stalkingField;
	StalkingZazuMinigame::StalkClips _stalkClips;
	Common::Point _targetPosInFieldCoordinates;
	Common::Point _simbaPosInFieldCoordinates;
	Common::Rect _simbaMovementBoundsInFieldCoordinates; // Offset 0x104 in original (FP68K calls obscure xrefs).
	StalkingZazuMinigame::Direction _currentDirection;

	uint _cursorResourceId = 0;
	uint _simbaActorId = 0;
	int _obstaclesToShow = 0;
	Common::Array<uint> _obstacleActorIds;

	bool _isActive = false;
	bool _simbaIsCrouching = false;
	bool _crouchClipIsSet = false;
	bool _acceptingMouseEvents = true;
	bool _zazuIsLooking = false;

	double _timerDurationInSeconds = 0.10;
	uint _scalingFactor = 10; // Offset 0xD8 in original (FP68K calls obscure xrefs).
	uint _successRadius = 20; // Offset 0xFE in original (FP68K calls obscure xrefs).

	// These are read in the original, but they don't seem to actually be populated
	// and sound effects don't seem to be enabled. However, they are included in case
	// they are used in some localization.
	bool _soundEffectsEnabled = false;
	uint _soundEffectId1 = 0;
	uint _soundEffectId2 = 0;
	void setAudio(bool isEnabled);
	void startSound(uint soundActorId);
	void stopSound(uint soundActorId);
};

} // End namespace MediaStation

#endif
