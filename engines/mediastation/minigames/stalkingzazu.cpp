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

#include "math/utils.h"
#include "graphics/paletteman.h"

#include "mediastation/minigames/stalkingzazu.h"
#include "mediastation/actors/sprite.h"
#include "mediastation/actors/sound.h"
#include "mediastation/datafile.h"
#include "mediastation/debugchannels.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

namespace StalkingZazuMinigame {

Direction::Direction(uint directionToken) {
	switch(directionToken) {
	case 0x44c:
		_value = Direction::kWest;
		break;

	case 0x44d:
		_value = Direction::kSouthWest;
		break;

	case 0x44e:
		_value = Direction::kSouth;
		break;

	case 0x44f:
		_value = Direction::kSouthEast;
		break;

	case 0x450:
		_value = Direction::kEast;
		break;

	case 0x451:
		_value = Direction::kNorthEast;
		break;

	case 0x452:
		_value = Direction::kNorth;
		break;

	case 0x453:
		_value = Direction::kNorthWest;
		break;

	default:
		error("%s: Got bad direction token %d", __func__, directionToken);
	}
}

StalkingZazuMinigame::Direction StalkingZazuMinigame::Direction::rotate(uint steps) const {
	uint zeroBased = (_value - 1 + steps) % Direction::TOTAL_DIRECTIONS;
	return static_cast<Value>(zeroBased + 1);
}

uint StalkingZazuMinigame::StalkClips::indexOf(Direction direction, bool isSimbaCrouching) {
	uint index = static_cast<uint>(direction);
	if (isSimbaCrouching) {
		// The clips are read in such that, for one direction, the crouched clip is always
		// this far away from the uncrouched clip.
		index += 9;
	}
	return index;
}

CellCoord CellCoord::getAdjacentCell(const Direction &direction) const {
	// Compute the neighbour purely arithmetically. Whether the result lands on
	// the grid is the field's concern, so this must be validated before use.
	CellCoord adjacentCell(x, y);

	switch (direction) {
	case Direction::kEast:
		adjacentCell.x = adjacentCell.x + 1;
		break;

	case Direction::kNorthEast:
		adjacentCell.y = adjacentCell.y - 1;
		adjacentCell.x = adjacentCell.x + 1;
		break;

	case Direction::kNorth:
		adjacentCell.y = adjacentCell.y - 1;
		break;

	case Direction::kNorthWest:
		adjacentCell.y = adjacentCell.y - 1;
		adjacentCell.x = adjacentCell.x - 1;
		break;

	case Direction::kWest:
		adjacentCell.x = adjacentCell.x - 1;
		break;

	case Direction::kSouthWest:
		adjacentCell.x = adjacentCell.x - 1;
		adjacentCell.y = adjacentCell.y + 1;
		break;

	case Direction::kSouth:
		adjacentCell.y = adjacentCell.y + 1;
		break;

	case Direction::kSouthEast:
		adjacentCell.y = adjacentCell.y + 1;
		adjacentCell.x = adjacentCell.x + 1;
		break;

	default:
		error("%s: Got bad direction token %d", __func__, static_cast<uint>(direction));
	}

	return adjacentCell;
}

StalkingField::StalkingField(int rowCount, int columnCount, const Common::Point &graphicalSize) {
	_graphicalSize = graphicalSize;
	_columns = columnCount;
	_rows = rowCount;
	_values.resize(rowCount * columnCount);
}

uint16 &StalkingField::at(int row, int column) {
	assert(row < _rows);
	assert(column < _columns);
	return _values[(row * _columns) + column];
}

const uint16 &StalkingField::at(int row, int column) const {
	assert(row < _rows);
	assert(column < _columns);
	return _values[(row * _columns) + column];
}

uint16 &StalkingField::at(const CellCoord &coord) {
	assert(coord.y >= 0);
	assert(coord.x >= 0);
	return at(coord.y, coord.x);
}

const uint16 &StalkingField::at(const CellCoord &coord) const {
	assert(coord.y >= 0);
	assert(coord.x >= 0);
	return at(coord.y, coord.x);
}

bool StalkingField::isInBounds(int row, int column) const {
	return (row >= 0) && (column >= 0) &&
		(row < _rows) && (column < _columns);
}

bool StalkingField::isInBounds(const CellCoord &coord) const {
	return isInBounds(coord.y, coord.x);
}

void StalkingField::clear() {
	Common::fill(_values.begin(), _values.end(), 0);
}

Common::Point StalkingField::bottomCenterPositionInFieldCoordinates() {
	Common::Point bottomCenterPosition = Common::Point(
		_graphicalSize.x / 2.0,
		_graphicalSize.y - (cellSize().y / 2.0));

	debugC(5, kDebugMinigame, "%s: (%d, %d) [graphicalSize: (%d, %d); cellSize: (%d, %d)]",
		__func__, bottomCenterPosition.x, bottomCenterPosition.y,
		_graphicalSize.x, _graphicalSize.y,
		cellSize().x, cellSize().y);

	return bottomCenterPosition;
}

CellCoord StalkingField::cellAtPoint(const Common::Point &pointInGraphicalCoordinates) {
	// Get the logical cell at the given graphical point.
	if ((_columns == 0) || (_rows == 0) || (_graphicalSize.x == 0) || (_graphicalSize.y == 0)) {
		error("%s: Attempted to get cell in empty field", __func__);
	}

	const int16 column = MIN<int16>(pointInGraphicalCoordinates.x * _columns / _graphicalSize.x, _columns - 1);
	const int16 row = MIN<int16>(pointInGraphicalCoordinates.y * _rows / _graphicalSize.y, _rows - 1);
	return CellCoord(column, row);
}

Common::Point StalkingField::cellSize() {
	// Get the size of each cell in graphical units.
	return Common::Point(
		static_cast<int16>(_graphicalSize.x  / (double)_columns),
		static_cast<int16>(_graphicalSize.y / (double)_rows));
}

Common::Point StalkingField::centerOfCell(const CellCoord &coord) {
	// Get the center of each cell in graphical units.
	Common::Point size = cellSize();
	return Common::Point(
		static_cast<int16>(coord.x * size.x  + size.x  / 2.0),
		static_cast<int16>(coord.y * size.y + size.y / 2.0));
}

bool StalkingField::obstacleCanBePlaced(const CellCoord &coord) {
	Direction direction(Direction::kNone);
	Common::Array<CellCoord> scratch;
	return obstacleCanBePlaced(coord, direction, scratch);
}

bool StalkingField::obstacleCanBePlaced(CellCoord currentPos, Direction direction, Common::Array<CellCoord> &visitedCells) {
	// Don't allow placing an obstacle along any border.
	bool canPlace = true;
	visitedCells.push_back(currentPos);
	if ((currentPos.y == 0) || (currentPos.x == 0) || \
	 		(currentPos.y == _rows - 1) || \
			(currentPos.x == _columns - 1)) {
		return false;
	}

	// Don't allow placing an obstacle directly above or below an existing one.
	if ((direction == Direction::kSouth) || (direction == Direction::kNorth)) {
		return false;
	}

	Direction startDirection;
	Direction stopDirection;
	if (direction == Direction::kNone) {
		startDirection = Direction::kEast;
		stopDirection = Direction::kEast;
	} else {
		startDirection = direction.counterClockwise();
		stopDirection = direction;
	}

	Direction currentDirection = startDirection;
	Direction rotatedDirection;
	do {
		CellCoord adjacentCell = currentPos.getAdjacentCell(currentDirection);
		if (isInBounds(adjacentCell)) {
			if (!isEmpty(adjacentCell)) {
				bool adjacentCellWasVisited = false;
				// TODO: Can we use a hashmap here?
				for (const CellCoord &visitedCell : visitedCells) {
					if (visitedCell == adjacentCell) {
						adjacentCellWasVisited = true;
						break;
					}
				}

				if (!adjacentCellWasVisited) {
					const Direction reverseDirection = currentDirection.opposite();
					canPlace = obstacleCanBePlaced(adjacentCell, reverseDirection, visitedCells);
				}
			}
		}

		rotatedDirection = currentDirection.counterClockwise();
		currentDirection = rotatedDirection;
		if (rotatedDirection == stopDirection) {
			break;
		}
	} while (canPlace);
	return canPlace;
}


void StalkingField::placeObstacles(const Common::Array<uint> &obstacleActorIds) {
	// Randomly partition the obstacles across the playable rows.
	// Each partition boundary marks the end of the current row's allotment of obstacles.
	// Each row receives a consecutive range of obstacles while randomly varying
	// how many obstacles each row contains.
	Common::Array<uint16> partitionBoundaryIndices = calcPartitionBoundaries(_rows - 2, obstacleActorIds.size());
	uint obstacleIndex = 0;
	uint row = 1;

	for (uint16 partitionBoundaryIndex : partitionBoundaryIndices) {
		while (obstacleIndex < partitionBoundaryIndex) {
			bool obstacleAdded = addObstacleToRow(static_cast<uint16>(obstacleActorIds[obstacleIndex]), row);
			if (!obstacleAdded) {
				// Give up on this row.
				break;
			}

			obstacleIndex++;
		}
		row++;
	}
}

Common::Array<uint16> StalkingField::calcPartitionBoundaries(uint totalRows, uint totalObstacles) {
	// Generate random cumulative partition boundaries for an ordered list.
	Common::Array<uint16> boundaryIndices;
	for (uint i = 0; i < totalRows; i++) {
		uint16 breakPoint = static_cast<uint16>(g_engine->_randomSource.getRandomNumber(totalObstacles));
		boundaryIndices.push_back(breakPoint);
	}

	Common::sort(boundaryIndices.begin(), boundaryIndices.end());
	return boundaryIndices;
}


bool StalkingField::addObstacleToRow(uint16 obstacleActorId, uint16 row) {
	Common::Array<CellCoord> availableCellsOnRow = calcAvailableCellsOnRow(row);

	bool obstacleAdded = false;
	while (availableCellsOnRow.size() > 0) {
		uint16 index = g_engine->_randomSource.getRandomNumber(availableCellsOnRow.size() - 1);
		CellCoord &coord = availableCellsOnRow[index];
		if (obstacleCanBePlaced(coord)) {
			at(coord) = obstacleActorId;
			obstacleAdded = true;
			break;
		} else {
			availableCellsOnRow.remove_at(index);
		}
	}

	return obstacleAdded;
}

Common::Array<CellCoord> StalkingField::calcAvailableCellsOnRow(uint row) {
	Common::Array<CellCoord> cells;
	for (int column = 0; column < _columns; ++column) {
		CellCoord cellCoord(column, row);
		if (isEmpty(cellCoord)) {
			cells.push_back(cellCoord);
		}
	}
	return cells;
}

void GameStateMachine::executeNextState(GameStateEvent event) {
	switch (event) {
	case kStalkingZazuResetStateEvent:
		switch (_currentState) {
		case kStalkingZazuStateStart:
		case kStalkingZazuStateCrouched:
		case kStalkingZazuStateCanMove:
		case kStalkingZazuStateProcessMove:
		case kStalkingZazuStateDefeat:
		case kStalkingZazuStateSuccess:
			_currentState = kStalkingZazuStateReset;
			_actor->stalkingState_1_reset();
			break;

		default:
			warnOnInvalidTransition(event);
		}
		break;

	case kStalkingZazuCrouchStateEvent:
		switch (_currentState) {
		case kStalkingZazuStateReset:
		case kStalkingZazuStateCanMove:
		case kStalkingZazuStateProcessMove:
			_currentState = kStalkingZazuStateCrouched;
			_actor->stalkingState_2_crouched();
			break;

		default:
			warnOnInvalidTransition(event);
		}
		break;

	case kStalkingZazuUncrouchStateEvent:
		switch (_currentState) {
		case kStalkingZazuStateCrouched:
			_currentState = kStalkingZazuStateCanMove;
			_actor->stalkingState_3_canMove();
			break;

		default:
			warnOnInvalidTransition(event);
		}
		break;

	case kStalkingZazuMoveStateEvent:
		switch (_currentState) {
		case kStalkingZazuStateCanMove:
		case kStalkingZazuStateProcessMove:
			_currentState = kStalkingZazuStateProcessMove;
			_actor->stalkingState_4_processMove();
			break;

		default:
			warnOnInvalidTransition(event);
		}
		break;

	case kStalkingZazuLoseStateEvent:
		switch (_currentState) {
		case kStalkingZazuStateCanMove:
		case kStalkingZazuStateProcessMove:
			_currentState = kStalkingZazuStateDefeat;
			_actor->stalkingState_5_defeat();
			break;

		default:
			warnOnInvalidTransition(event);
		}
		break;

	case kStalkingZazuWinStateEvent:
		switch (_currentState) {
		case kStalkingZazuStateCanMove:
		case kStalkingZazuStateProcessMove:
			_currentState = kStalkingZazuStateSuccess;
			_actor->stalkingState_6_success();
			break;

		default:
			warnOnInvalidTransition(event);
		}
		break;

	default:
		warnOnInvalidTransition(event);
	}
}

void TimerStateMachine::executeNextState(TimerEvent eventType) {
	switch (eventType) {
	case kStalkingZazuActivateTimerEvent:
		switch (_currentState) {
		case kStalkingZazuExpiredInactiveTimerState:
			_currentState = kStalkingZazuExpiredActiveTimerState;
			// This state doesn't have a callback.
			break;

		case kStalkingZazuQueuedInactiveTimerState:
			_currentState = kStalkingZazuQueuedTimerState;
			_actor->timerState_1_queued();
			break;

		default:
			warnOnInvalidTransition(eventType);
		}
		break;

	case kStalkingZazuInactivateTimerEvent:
		switch (_currentState) {
		case kStalkingZazuExpiredTimerState:
		case kStalkingZazuExpiredActiveTimerState:
			_currentState = kStalkingZazuExpiredInactiveTimerState;
			// This state doesn't have a callback.
			break;

		case kStalkingZazuQueuedTimerState:
			_currentState = kStalkingZazuQueuedInactiveTimerState;
			// This state doesn't have a callback.
			break;

		default:
			warnOnInvalidTransition(eventType);
		}
		break;

	case kStalkingZazuExpireTimerEvent:
		switch (_currentState) {
		case kStalkingZazuQueuedTimerState:
			_currentState = kStalkingZazuExpiredTimerState;
			_actor->timerState_0_expired();
			break;

		default:
			warnOnInvalidTransition(eventType);
		}
		break;

	case kStalkingZazuQueueTimerEvent:
		switch (_currentState) {
		case kStalkingZazuExpiredTimerState:
		case kStalkingZazuExpiredActiveTimerState:
			_currentState = kStalkingZazuQueuedTimerState;
			_actor->timerState_1_queued();
			break;

		case kStalkingZazuExpiredInactiveTimerState:
			_currentState = kStalkingZazuQueuedInactiveTimerState;
			// This state doesn't have a callback.
			break;

		default:
			warnOnInvalidTransition(eventType);
		}
		break;

	default:
		warnOnInvalidTransition(eventType);
	}
}

} // End of namespace StalkingZazuMinigame

using namespace StalkingZazuMinigame;
void StalkingZazuActor::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderCursorResourceId:
		_cursorResourceId = chunk.readTypedUint16();
		break;

	case kActorHeaderFrameRate: {
		double timerFiresPerSecond = chunk.readTypedDouble();
		_timerDurationInSeconds = 1.0 / timerFiresPerSecond;
		break;
	}

	case kActorHeaderEditable:
		_isActive = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderStalkingZazuSpriteId:
		_simbaActorId = chunk.readTypedUint16();
		break;

	case kActorHeaderStalkingZazuDirections:
		_stalkClips.resize(StalkClips::SIZE);
		for (uint i = 0; i < 5; i++) {
			uint directionToken = chunk.readTypedUint16();
			Direction direction(directionToken);

			uint nonCrouchedClipId = chunk.readTypedUint16();
			uint nonCrouchedClipIndex = _stalkClips.indexOf(direction, false);
			_stalkClips[nonCrouchedClipIndex] = nonCrouchedClipId;

			uint crouchedClipId = chunk.readTypedUint16();
			uint crouchedClipIndex = _stalkClips.indexOf(direction, true);
			_stalkClips[crouchedClipIndex] = crouchedClipId;
		}
		break;

	case kActorHeaderStalkingZazuField: {
		uint rowCount = chunk.readTypedUint16();
		uint columnCount = chunk.readTypedUint16();
		Common::Point graphicalSize = Common::Point(getBbox().width(), getBbox().height());
		_stalkingField = StalkingField(rowCount, columnCount, graphicalSize);
		_targetPosInFieldCoordinates = Common::Point(graphicalSize.x / 2, 0);
		break;
	}

	case kActorHeaderStalkingZazuObstacleActorId: {
		uint obstacleActorId = chunk.readTypedUint16();
		_obstacleActorIds.push_back(obstacleActorId);
		break;
	}

	case kActorHeaderStalkingZazuUnkX:
		_scalingFactor = chunk.readTypedGraphicUnit();
		break;

	case kActorHeaderStalkingZazuUnkY:
		_successRadius = chunk.readTypedGraphicUnit();
		break;

	case kActorHeaderStalkingZazuUnkSound1:
		_soundEffectId1 = chunk.readTypedUint16();
		break;

	case kActorHeaderStalkingZazuUnkSound2:
		_soundEffectId2 = chunk.readTypedUint16();
		break;

	case kActorHeaderStalkingZazuAudioEnabled:
		_soundEffectsEnabled = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderStalkingZazuObstaclesToShow:
		_obstaclesToShow = chunk.readTypedSint16();
		break;

	case kActorHeaderStalkingZazuUnkPoint:
		// This is read into offset 0xDE in the original
		// but doesn't seem to ever be accessed, so we will throw it away.
		chunk.readTypedPoint();
		break;

	default:
		SpatialEntity::readParameter(chunk, paramType);
	}
}

ScriptValue StalkingZazuActor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;
	switch (methodId) {
	case kStalkingStartZazuLookingMethod:
		ARGCOUNTCHECK(0);
		setZazuLook(true);
		break;

	case kStalkingStopZazuLookingMethod:
		ARGCOUNTCHECK(0);
		setZazuLook(false);
		break;

	case kStalkingGetZazuLookDirectionMethod: {
		ARGCOUNTCHECK(0);
		switch (directionZazuWillLook()) {
		case Direction::kEast:
			returnValue.setToParamToken(0x450);
			break;

		case Direction::kWest:
			returnValue.setToParamToken(0x44c);
			break;

		default:
			returnValue.setToParamToken(0x450);
		}
		break;
	}

	case kMinigameResetMethod:
		ARGCOUNTCHECK(1);
		_obstaclesToShow = static_cast<int>(args[0].asFloat());
		generateStalkingEvent(kStalkingZazuResetStateEvent);
		_gameMachine.runIfNotNested();
		break;

	case kMinigameActivateMethod:
		ARGCOUNTCHECK(0);
		setActive(true);
		break;

	case kMinigameDeactivateMethod:
		ARGCOUNTCHECK(0);
		setActive(false);
		break;

	case kStalkingEnableAudio:
		ARGCOUNTCHECK(0);
		setAudio(true);
		break;

	case kStalkingDisableAudio:
		ARGCOUNTCHECK(0);
		setAudio(false);
		break;

	default:
		returnValue = SpatialEntity::callMethod(methodId, args);
	}

	return returnValue;
}

bool StalkingZazuActor::isVisible() const {
	// This actor doesn't actually draw anything by itself. It controls other actors
	// to show the user the game. However, drawing the stalking grid and such is extremely
	// useful for debugging. But we only want to draw this grid if debugging is enabled.
	return debugChannelSet(4, kDebugMinigame);
}

void StalkingZazuActor::draw(DisplayContext &displayContext) {
	Graphics::ManagedSurface *targetSurface = displayContext._destImage;
	if (targetSurface == nullptr) {
		warning("[%s] %s: No target surface to draw", debugName(), __func__);
		return;
	}

	const int rowCount = _stalkingField.rowCount();
	const int columnCount = _stalkingField.columnCount();
	if (rowCount == 0 || columnCount == 0) {
		return;
	}

	// Outline each cell boundary and the center of each cell to visualize the stalking field.
	const Common::Point drawOrigin =  getBbox().origin() + displayContext._origin;
	const int16 markerRadius = 4;
	const Graphics::Palette currentPalette = g_system->getPaletteManager()->grabPalette(0, Graphics::PALETTE_COUNT);
	const byte blackPaletteIndex = currentPalette.findBestColor(0, 0, 0);
	const byte greenPaletteIndex = currentPalette.findBestColor(0, 255, 0);
	const byte redPaletteIndex = currentPalette.findBestColor(255, 0, 0);
	for (int row = 0; row < rowCount; ++row) {
		for (int column = 0; column < columnCount; ++column) {
			Common::Rect cellBounds(
				(static_cast<int32>(column) *  getBbox().width()) / columnCount,
				(static_cast<int32>(row) *  getBbox().height()) / rowCount,
				(static_cast<int32>(column + 1) *  getBbox().width()) / columnCount,
				(static_cast<int32>(row + 1) *  getBbox().height()) / rowCount
			);
			cellBounds.translate(drawOrigin.x, drawOrigin.y);
			const bool cellContainsObstacle = _stalkingField.at(row, column) != 0;
			const byte cellOutlinePaletteIndex = cellContainsObstacle ? redPaletteIndex : blackPaletteIndex;
			targetSurface->frameRect(cellBounds, cellOutlinePaletteIndex);

			Common::Point cellBoundsCenterInScreenCoordinates = cellBounds.center();
			targetSurface->fillRect(
				Common::Rect::center(
					cellBoundsCenterInScreenCoordinates.x, cellBoundsCenterInScreenCoordinates.y,
					markerRadius, markerRadius),
				redPaletteIndex);
		}
	}

	// Outline the area where Simba is allowed to move.
	Common::Rect movementBoundsOnScreen = _simbaMovementBoundsInFieldCoordinates;
	movementBoundsOnScreen.translate(drawOrigin.x, drawOrigin.y);
	targetSurface->frameRect(movementBoundsOnScreen, greenPaletteIndex);

	// Outline the bottom center position (Simba's starting point).
	const Common::Point bottomCenterPositionInFieldCoordinates = _stalkingField.bottomCenterPositionInFieldCoordinates();
	const Common::Point markerCenter = drawOrigin + bottomCenterPositionInFieldCoordinates;
	targetSurface->fillRect(
		Common::Rect::center(markerCenter.x, markerCenter.y, markerRadius, markerRadius),
		redPaletteIndex);

	// Draw the target point (Simba's ending point).
	const Common::Point targetMarkerCenter = drawOrigin + _targetPosInFieldCoordinates;
	targetSurface->fillRect(
		Common::Rect::center(targetMarkerCenter.x, targetMarkerCenter.y, markerRadius, markerRadius),
		redPaletteIndex);
}

void StalkingZazuActor::loadIsComplete() {
	SpatialEntity::loadIsComplete();

	// Shrink the movement bounds by half Simba's width. This is to keep Simba's
	// center from reaching the center rather than its left edge.
	_simbaMovementBoundsInFieldCoordinates = Common::Rect(0, 0, getBbox().width(), getBbox().height());
	SpriteMovieActor *simbaActor = static_cast<SpriteMovieActor *>(g_engine->getImtGod()->getActorByIdAndType(_simbaActorId, kActorTypeSprite));
	if (simbaActor != nullptr) {
		// This was in the original to handle the case where the movement bounds are smaller than
		// the simba's width after the inset. Since this actor is only used for one minigame with
		// constant dimensions, we don't really need this code, but it is kept in here for robustness.
		const int16 halfSimbaWidth = static_cast<int16>(simbaActor->getBbox().width() / 2.0);
		_simbaMovementBoundsInFieldCoordinates.left += halfSimbaWidth;
		_simbaMovementBoundsInFieldCoordinates.right -= halfSimbaWidth;
		if (_simbaMovementBoundsInFieldCoordinates.left > _simbaMovementBoundsInFieldCoordinates.right) {
			const int16 centerX = (_simbaMovementBoundsInFieldCoordinates.left + _simbaMovementBoundsInFieldCoordinates.right) / 2;
			_simbaMovementBoundsInFieldCoordinates.left = centerX;
			_simbaMovementBoundsInFieldCoordinates.right = centerX;
		}
	}

	if (_obstaclesToShow != 0) {
		generateStalkingEvent(kStalkingZazuResetStateEvent);
		_gameMachine.runIfNotNested();
	}
}

void StalkingZazuActor::onEvent(const ActorEvent &event) {
	switch (event.type) {
	case kMinigameSuccessEvent:
	case kMinigameDefeatEvent:
	case kMinigameCrouchedEvent:
		runScriptResponseIfExists(event.type);
		break;

	default:
		Actor::onEvent(event);
	}
}

void StalkingZazuActor::timerEvent(const MediaStation::TimerEvent &event) {
	generateTimerEvent(kStalkingZazuExpireTimerEvent);
	_timerMachine.runIfNotNested();
}

void StalkingZazuActor::setZazuLook(bool zazuIsLooking) {
	if (_zazuIsLooking != zazuIsLooking) {
		_zazuIsLooking = zazuIsLooking;
		if (zazuSeesSimba()) {
			generateStalkingEvent(kStalkingZazuLoseStateEvent);
			_gameMachine.runIfNotNested();
		}
	}
}

void StalkingZazuActor::setAudio(bool isEnabled) {
	_soundEffectsEnabled = isEnabled;
	if (!isEnabled) {
		stopSound(_soundEffectId1);
		stopSound(_soundEffectId2);
	}
}

void StalkingZazuActor::startSound(uint soundActorId) {
	if (soundActorId != 0) {
		SoundActor *sound = static_cast<SoundActor *>(g_engine->getImtGod()->getActorByIdAndType(soundActorId, kActorTypeSound));
		if (sound != nullptr) {
			sound->start();
		} else {
			warning("[%s] %s: Null sound %d", debugName(), __func__, soundActorId);
		}
	}
}

void StalkingZazuActor::stopSound(uint soundActorId) {
	if (soundActorId != 0) {
		SoundActor *sound = static_cast<SoundActor *>(g_engine->getImtGod()->getActorByIdAndType(soundActorId, kActorTypeSound));
		if (sound != nullptr) {
			sound->stop();
		} else {
			warning("[%s] %s: Null sound %d", debugName(), __func__, soundActorId);
		}
	}
}

void StalkingZazuActor::setActive(bool isActive) {
	if (_isActive != isActive) {
		_isActive = isActive;
		if (_isActive) {
			generateTimerEvent(kStalkingZazuActivateTimerEvent);
		} else {
			_mouseIsInside = false;
			_mouseIsDown = false;
			generateTimerEvent(kStalkingZazuInactivateTimerEvent);
		}
		_timerMachine.runIfNotNested();

		g_engine->generateMouseUpdateEvent();
	}
}

void StalkingZazuActor::generateStalkingEvent(GameStateEvent event) {
	_gameMachine.queueEvent(event);
}

void StalkingZazuActor::generateTimerEvent(StalkingZazuMinigame::TimerEvent event) {
	_timerMachine.queueEvent(event);
}

StalkingZazuMinigame::Direction StalkingZazuActor::directionZazuWillLook() {
	Common::Point delta = _targetPosInFieldCoordinates - _simbaPosInFieldCoordinates;
	const double angleRadians = atan2(delta.y, delta.x);
	const double angle = Math::rad2deg<double>(angleRadians);

	if (angle >= -90 && angle < 90) {
		return Direction::kEast;
	} else {
		return Direction::kWest;
	}
}

void StalkingZazuActor::calcWhichObstaclesToShow(uint16 obstaclesToShow) {
	int obstaclesToRemove = _obstacleActorIds.size() - obstaclesToShow;
	if (obstaclesToRemove > 0) {
		// The original removes obstacles at evenly distributed intervals. This ensures
		// the remaining obstacles are spread out rather than clustered. However, this
		// code doesn't seem to be reached in the original because scripts never seem to
		// request to show fewer obstacles.
		warning("%s: Showing all %d obstacles", __func__, obstaclesToShow);
	}
}

void StalkingZazuActor::calcObstaclePlacements() {
	_stalkingField.clear();
	_stalkingField.placeObstacles(_obstacleActorIds);
}

void StalkingZazuActor::placeObstaclesOnStage() {
	for (int row = 0; row < _stalkingField.rowCount(); ++row) {
		for (int column = 0; column < _stalkingField.columnCount(); ++column) {
			const uint16 obstacleActorId = _stalkingField.at(row, column);
			if (obstacleActorId != 0) {
				const CellCoord cell(static_cast<int16>(column), static_cast<int16>(row));
				placeAndShowActor(obstacleActorId, _stalkingField.centerOfCell(cell));
			}
		}
	}
}

void StalkingZazuActor::positionSimbaStartOnStage() {
	_simbaPosInFieldCoordinates = _stalkingField.bottomCenterPositionInFieldCoordinates();
	// This magic constant is in the original; not sure exactly why such a small offset is needed.
	_simbaPosInFieldCoordinates.x += 3;
	placeAndShowActor(_simbaActorId, _simbaPosInFieldCoordinates);

	const Direction startDirection(Direction::kNorth);
	setSimbaClipFromDirection(startDirection);
	setSimbaCrouchClip(true);
}

void StalkingZazuActor::placeAndShowActor(uint actorId, const Common::Point &destInFieldCoordinates) {
	SpatialEntity *actorToPlace = g_engine->getImtGod()->getSpatialEntityById(actorId);
	if (actorToPlace == nullptr) {
		warning("[%s] %s: Got null actor %d", debugName(), __func__, actorId);
		return;
	}

	uint zCoord = zCoordOfPoint(destInFieldCoordinates);
	actorToPlace->setZIndex(zCoord);

	// Convert field coordinates to screen coordinates and center the actor directly over
	// the destInFieldCoordinates.
	Common::Point destInScreenCoordinates = destInFieldCoordinates + getBbox().origin();
	// We could use moveToCentered here, but the engine for this minigame didn't seem
	// to have it and did the centering calculation manually. So to avoid confusion when comparing
	// with disassembly, we will also do the centering calculation manually here.
	destInScreenCoordinates.x -= actorToPlace->getBbox().width() / 2;
	destInScreenCoordinates.y -= actorToPlace->getBbox().height() / 2;
	actorToPlace->moveTo(destInScreenCoordinates.x, destInScreenCoordinates.y);

	Common::Array<ScriptValue> emptyArgs(0);
	actorToPlace->callMethod(kSpatialShowMethod, emptyArgs);
}

int StalkingZazuActor::zCoordOfPoint(const Common::Point &point) {
	// Items lower in the field (higher y) should appear in front of items higher up.
	return zIndex() - point.y;
}

bool StalkingZazuActor::simbaIsInSuccessPosition() {
	Common::Point delta = _targetPosInFieldCoordinates - _simbaPosInFieldCoordinates;
	const double distanceToTarget = Math::hypotenuse<double>(static_cast<double>(delta.x), static_cast<double>(delta.y));
	return (distanceToTarget <= _successRadius);
}

void StalkingZazuActor::setSimbaCrouchClip(bool isCrouchClipSet) {
	if (isCrouchClipSet != _crouchClipIsSet) {
		uint index = _stalkClips.indexOf(_currentDirection, isCrouchClipSet);
		setClipOfMovie(_simbaActorId, _stalkClips[index]);
		_crouchClipIsSet = isCrouchClipSet;
	}
}

void StalkingZazuActor::processMove(Common::Point mousePos) {
	Common::Point targetPosInFieldCoordinates = mousePos - getBbox().origin();
	debugC(5, kDebugMinigame, "[%s] %s: currentPos: (%d, %d); dest: (%d, %d)", debugName(), __func__,
		_simbaPosInFieldCoordinates.x, _simbaPosInFieldCoordinates.y, targetPosInFieldCoordinates.x, targetPosInFieldCoordinates.y);

	Common::Point newPosInFieldCoordinates;
	Direction direction;
	bool simbaMoved = getMoveInformation(targetPosInFieldCoordinates, newPosInFieldCoordinates, direction);
	if (simbaMoved) {
		setSimbaClipFromDirection(direction);
		if (isClearAtPoint(newPosInFieldCoordinates)) {
			placeAndShowActor(_simbaActorId, newPosInFieldCoordinates);
			SpriteMovieActor *actor = static_cast<SpriteMovieActor *>(g_engine->getImtGod()->getActorByIdAndType(_simbaActorId, kActorTypeSprite));
			if (actor != nullptr) {
				actor->activateNextFrame();
			}

			_simbaPosInFieldCoordinates = newPosInFieldCoordinates;
		}
	}
}

bool StalkingZazuActor::getMoveInformation(const Common::Point &targetPosInFieldCoordinates, Common::Point &nextPosInFieldCoordinates, Direction &direction) {
	if (targetPosInFieldCoordinates == _simbaPosInFieldCoordinates) {
		return false;
	}

	const Common::Point delta = targetPosInFieldCoordinates - _simbaPosInFieldCoordinates;
	direction = directionOfVector(delta);
	nextPosInFieldCoordinates = desiredPositionFromVectorInFieldCoordinates(delta);
	if (nextPosInFieldCoordinates == _simbaPosInFieldCoordinates) {
		return false;
	}

	return true;
}

Direction StalkingZazuActor::directionOfVector(const Common::Point &dest) {
	const double angleRadians = atan2(static_cast<double>(dest.y), static_cast<double>(dest.x));
	const double angle = Math::rad2deg<double>(angleRadians);

	// This actor cannot move south. Any southerly vector is treated as
	// either East or West depending on its horizontal component.
	if (angle >= 90.0) return Direction::kWest;
	else if (angle >= -22.5) return Direction::kEast;
	else if (angle >= -67.5) return Direction::kNorthEast;
	else if (angle >=-112.5) return Direction::kNorth;
	else if (angle >=-157.5) return Direction::kNorthWest;
	else return Direction::kWest;
}

Common::Point StalkingZazuActor::desiredPositionFromVectorInFieldCoordinates(const Common::Point &delta) {
	const double length = Math::hypotenuse<double>(static_cast<double>(delta.x), static_cast<double>(delta.y));
	double scale = 0.0;
	if (length != 0.0) {
		scale = static_cast<double>(_scalingFactor) / length;
	}

	int16 dx = static_cast<int16>(round(delta.x * scale));
	int16 dy = static_cast<int16>(round(delta.y * scale));

	// Don't allow the normalized vector to overshoot the original.
	if (dy < 0) {
		dy = MAX(dy, delta.y);
	} else {
		// Simba isn't allowed to backtrack.
		dy = 0;
	}

	if (dx > 0) {
		dx = MIN(dx, delta.x);
	} else {
		dx = MAX(dx, delta.x);
	}

	Common::Point nextPosInFieldCoordinates(_simbaPosInFieldCoordinates.x + dx, _simbaPosInFieldCoordinates.y + dy);
	debugC(4, kDebugMinigame, "[%s] %s: (%d, %d): (%d, %d) + (%d, %d) = (%d, %d)",
		debugName(), __func__, delta.x, delta.y,
		_simbaPosInFieldCoordinates.x, _simbaPosInFieldCoordinates.y, dx, dy,
		nextPosInFieldCoordinates.x, nextPosInFieldCoordinates.y);

	bool nextPosSuccessfullyConstrained = _simbaMovementBoundsInFieldCoordinates.constrain(nextPosInFieldCoordinates.x, nextPosInFieldCoordinates.y, 0, 0);
	if (!nextPosSuccessfullyConstrained) {
		warning("[%s] %s: Point constraint failed", debugName(), __func__);
	}

	return nextPosInFieldCoordinates;
}

void StalkingZazuActor::setSimbaClipFromDirection(Direction direction) {
	if (_currentDirection != direction || _crouchClipIsSet) {
		uint stalkClipIndex = _stalkClips.indexOf(direction, false);
		setClipOfMovie(_simbaActorId, _stalkClips[stalkClipIndex]);
		_crouchClipIsSet = false;
		_currentDirection = direction;
		debugC(5, kDebugMinigame, "[%s] %s: Simba is facing %d", debugName(), __func__, static_cast<uint>(direction));
	}
}

void StalkingZazuActor::setClipOfMovie(uint spriteActorId, uint clipId) {
	SpriteMovieActor *movie = static_cast<SpriteMovieActor *>(g_engine->getImtGod()->getActorByIdAndType(_simbaActorId, kActorTypeSprite));
	if (movie != nullptr) {
		movie->setCurrentClip(clipId);
	}
}

bool StalkingZazuActor::isClearAtPoint(const Common::Point &posInFieldCoordinates) {
	// Rather than testing every tile the rectangle overlaps, the code assumes the rectangle is
	// at most one tile wide/high (here it’s half a tile), and walls occupy whole tiles. Under those
	// assumptions, checking the four corners completely determines whether the rectangle intersects
	// any blocked tile.
	const Common::Point cellSize = _stalkingField.cellSize();
	const int16 halfWidth = cellSize.x / 4;
	const int16 halfHeight = cellSize.y / 4;

	Common::Rect box(
		Common::Point(posInFieldCoordinates.x - halfWidth, posInFieldCoordinates.y - halfHeight),
		halfWidth * 2, halfHeight * 2);
	box.clip(_simbaMovementBoundsInFieldCoordinates);

	const int16 rightEdge = MAX<int16>(box.right - 1, box.left);
	const int16 bottomEdge = MAX<int16>(box.bottom - 1, box.top);
	const Common::Point corners[4] = {
		Common::Point(box.left, box.top),
		Common::Point(box.left, bottomEdge),
		Common::Point(rightEdge, bottomEdge),
		Common::Point(rightEdge, box.top)
	};

	for (const Common::Point &corner : corners) {
		const CellCoord cell = _stalkingField.cellAtPoint(corner);
		if (_stalkingField.at(cell) != 0) {
			return false;
		}
	}

	return true;
}

void StalkingZazuActor::stalkingState_1_reset() {
	_obstaclesToShow = MIN(_obstaclesToShow, (int)_obstacleActorIds.size());
	if (_obstaclesToShow >= 0) {
		calcWhichObstaclesToShow(_obstaclesToShow);
		calcObstaclePlacements();
	}

	// hideObstacles
	for (uint obstacleActorId : _obstacleActorIds) {
		Actor *obstacle = g_engine->getImtGod()->getActorById(obstacleActorId);
		if (obstacle != nullptr) {
			Common::Array<ScriptValue> emptyArgs(0);
			obstacle->callMethod(kSpatialHideMethod, emptyArgs);
		}
	}

	placeObstaclesOnStage();
	positionSimbaStartOnStage();
	_simbaIsCrouching = false;
	g_engine->generateMouseUpdateEvent();

	setZazuLook(false);
	_acceptingMouseEvents = true;
}

void StalkingZazuActor::stalkingState_2_crouched() {
	if (!_simbaIsCrouching) {
		_simbaIsCrouching = true;
		g_engine->generateMouseUpdateEvent();

		ActorEvent actorEvent(_id, kMinigameCrouchedEvent);
		g_engine->getEventLoop()->queueEvent(actorEvent);
	}

	setSimbaCrouchClip(true);
	if (_soundEffectsEnabled) {
		startSound(_soundEffectId1);
	}
	_acceptingMouseEvents = false;
}

void StalkingZazuActor::stalkingState_3_canMove() {
	setSimbaCrouchClip(false);
	if (_soundEffectsEnabled) {
		startSound(_soundEffectId1);
	}

	if (zazuSeesSimba()) {
		generateStalkingEvent(kStalkingZazuLoseStateEvent);
		_gameMachine.runIfNotNested();
		_acceptingMouseEvents = false;
	}

	generateTimerEvent(kStalkingZazuQueueTimerEvent);
	_timerMachine.runIfNotNested();
}

void StalkingZazuActor::stalkingState_4_processMove() {
	if (zazuSeesSimba()) {
		debugC(5, kDebugMinigame, "[%s] %s: LOSE", debugName(), __func__);
		generateStalkingEvent(kStalkingZazuLoseStateEvent);
		_gameMachine.runIfNotNested();

	} else if (simbaIsInSuccessPosition()) {
		debugC(5, kDebugMinigame, "[%s] %s: WIN", debugName(), __func__);
		generateStalkingEvent(kStalkingZazuWinStateEvent);
		_gameMachine.runIfNotNested();

	} else {
		Common::Point mousePos;
		currentMousePosition(mousePos);
		processMove(mousePos);
		if (_soundEffectsEnabled) {
			startSound(_soundEffectId2);
		}

		generateTimerEvent(kStalkingZazuQueueTimerEvent);
		_timerMachine.runIfNotNested();
	}
}

void StalkingZazuActor::stalkingState_5_defeat() {
	ActorEvent actorEvent(_id, kMinigameDefeatEvent);
	g_engine->getEventLoop()->queueEvent(actorEvent);
}

void StalkingZazuActor::stalkingState_6_success() {
	ActorEvent actorEvent(_id, kMinigameSuccessEvent);
	g_engine->getEventLoop()->queueEvent(actorEvent);
}

void StalkingZazuActor::timerState_0_expired() {
	generateStalkingEvent(kStalkingZazuMoveStateEvent);
	_gameMachine.runIfNotNested();
}

void StalkingZazuActor::timerState_1_queued() {
	g_engine->getTimerService()->startTimer(_timer, _timerDurationInSeconds);
}

uint16 StalkingZazuActor::findActorToAcceptMouseEvents(
	const Common::Point &point,
	uint16 eventMask,
	MouseActorState &state,
	bool clipMouseEvents) {
	(void)clipMouseEvents;

	uint16 result = 0;
	if (_isActive) {
		const bool isMousePositionInteresting = mousePositionIsInteresting(point);
		if (isMousePositionInteresting) {
			if (eventMask & kMouseDownFlag) {
				state.mouseDown = this;
				result |= kMouseDownFlag;
			}

			if (eventMask & kMouseEnterFlag) {
				state.mouseEnter = this;
				result |= kMouseEnterFlag;
			}

			if (eventMask & kMouseMovedFlag) {
				state.mouseMoved = this;
				result |= kMouseMovedFlag;
			}
		}

		if (_mouseIsInside && (eventMask & kMouseExitFlag)) {
			state.mouseExit = this;
			result |= kMouseExitFlag;
		}

		if (_mouseIsDown && (eventMask & kMouseUpFlag)) {
			state.mouseUp = this;
			result |= kMouseUpFlag;
		}
	}

	return result;
}

bool StalkingZazuActor::mousePositionIsInteresting(const Common::Point &pos) {
	bool isInteresting = true;
	if (_acceptingMouseEvents) {
		SpriteMovieActor *simba = static_cast<SpriteMovieActor *>(g_engine->getImtGod()->getActorByIdAndType(_simbaActorId, kActorTypeSprite));
		if (simba == nullptr) {
			isInteresting = false;
		}
		isInteresting = simba->getBbox().contains(pos);
	}
	return isInteresting;
}

void StalkingZazuActor::mouseDownEvent(const MouseEvent &event) {
	_mouseIsDown = true;
	generateStalkingEvent(kStalkingZazuCrouchStateEvent);
	_gameMachine.runIfNotNested();
}

void StalkingZazuActor::mouseUpEvent(const MouseEvent &event) {
	_mouseIsDown = false;
	generateStalkingEvent(kStalkingZazuUncrouchStateEvent);
	_gameMachine.runIfNotNested();
}

void StalkingZazuActor::mouseEnteredEvent(const MouseEvent &event) {
	_mouseIsInside = true;
	if (_cursorResourceId != 0) {
		g_engine->getCursorManager()->setAsTemporary(_cursorResourceId);
	} else {
		g_engine->getCursorManager()->unsetTemporary();
	}
}

void StalkingZazuActor::mouseExitedEvent(const MouseEvent &event) {
	_mouseIsInside = false;
}

bool StalkingZazuActor::zazuSeesSimba() {
	return _zazuIsLooking && !simbaIsBehindRock();
}

bool StalkingZazuActor::simbaIsBehindRock() {
	// Simba is behind the rock iff the rock is directly
	// north of the field cell he is currently in.
	bool isBehindRock = false;
	CellCoord currentCell = _stalkingField.cellAtPoint(_simbaPosInFieldCoordinates);

	CellCoord cellNorthOfCurrent = currentCell.getAdjacentCell(Direction::kNorth);
	if (_stalkingField.isInBounds(cellNorthOfCurrent)) {
		uint16 cellValue = _stalkingField.at(cellNorthOfCurrent);
		if (cellValue != 0) {
			isBehindRock = true;
		}
	}

	return isBehindRock;
}

Common::String StalkingZazuActor::debugString() const {
	return Common::String::format("simbaActorId: %u\n", _simbaActorId);
}

} // End namespace MediaStation
