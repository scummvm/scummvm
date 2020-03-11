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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "bladerunner/bladerunner.h"
#include "bladerunner/game_constants.h"
#include "bladerunner/items.h"
#include "bladerunner/mouse.h"
#include "bladerunner/savefile.h"
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/script/police_maze.h"
#include "bladerunner/script/scene_script.h"
#include "bladerunner/time.h"
#include "bladerunner/subtitles.h"
#include "bladerunner/debugger.h"
#include "bladerunner/settings.h"
// ----------------------
// Maze point system info
// ----------------------
// Maze score starts at zero (0) points at the first room and it can get a negative value later on.
// Exiting each room deducts from maze score the number of targets that were not activated for that room
// Each room has a max number of 20 targets that can be activated for it in total (kPoliceMazePS1xTargetCount).
// Entering a room always auto-activates a set of predefined targets: 4 for PS10, PS11, PS12 and 5 for PS12
//
// - Leaving a room from the forward exit (moving properly through the maze)
//   will mark the old room as complete. So, returning to an old room
//   (which McCoy had exited from the *forward* exit) won't affect the score.
//
// - Leaving a room from the *backwards* exit (moving backwards through the maze)
//   will NOT mark the room that McCoy just left as complete, if it was not already.
//   So returning to a previous room (which McCoy had exited from the *backwards* exit)
//   may still affect the score and combat may resume. However, upon re-entering that room,
//   it will again activate the predefined set of targets for it and those will count
//   additively to the total activated targets for that room. So, the room can be resumed at most
//   four (PS12) or five (PS10, PS11, PS13) times until it becomes completed (reaches its max activated targets).
//
// Running quickly through the maze (not stopping to shoot targets) amounts to a negative score of:
//      0 - (3 * (20 - 4)) - (20 - 5) = -63 points
//
// However, that is not the lowest score McCoy can get, since points are deducted when:
//  a) shooting innocents
//  b) shooting unrevealed enemies
//  c) getting shot
//
// Combat Point System:
//      + 1: gain a point when an enemy (revealed) is shot             (at Item_Spin_In_World)
//      - 1: lose a point when an innocent or unrevealed enemy is shot (at Item_Spin_In_World)
//      + 1: gain a point when an innocent escapes                     (at kPMTILeave instruction)
//      - 1: lose a point when an enemy shoots McCoy                   (at kPMTIShoot)
//
// For the maximum score, all 4 * 20 = 80 targets have to get activated and handled properly.
// Since McCoy always gains one (1) point per target (enemy or innocent) for that,
// the maximum score *should be*: 80 points.
//
// However, there are some *special* target types:
//      1. Targets that will count as multiple targets without increasing
//         the active target count more than once.
//      2. Targets that won't increase the active target count at all,
//         and act as bonus points themselves. (eg. kItemPS12Target10, kItemPS12Target14)
//
// With the account of special targets the maximum achievable score will be greater than 80 points.
// Since the special targets can appear randomly (by use of the kPMTIPausedReset1of2, kPMTIPausedReset1of3 instructions)
// the highest score is not a predefined fixed number and will differ per run. It will always be above 80 points.
//
namespace BladeRunner {

PoliceMaze::PoliceMaze(BladeRunnerEngine *vm) : ScriptBase(vm) {
	_isPaused = false;
	_isActive = false;
	_isEnding = false;

	_pm_var1 = 0;
	_pm_var2 = 0;

	for (int i = 0; i < kNumMazeTracks; ++i) {
		_tracks[i] = new PoliceMazeTargetTrack(vm);
	}
}

PoliceMaze::~PoliceMaze() {
	for (int i = 0; i < kNumMazeTracks; ++i) {
		delete _tracks[i];
		_tracks[i] = nullptr;
	}
}

void PoliceMaze::clear(bool isLoadingGame) {
	for (int i = 0; i < kNumMazeTracks; ++i) {
		if (_tracks[i]->isPresent()) {
			_tracks[i]->clear(isLoadingGame);
		}
	}
}

void PoliceMaze::activate() {
	_isActive = true;
	_isEnding = false;
}

void PoliceMaze::setPauseState(bool state) {
	warning("PAUSE: %d", state);
	_isPaused = state;

	uint32 timeNow = _vm->_time->current();

	for (int i = 0; i < kNumMazeTracks; ++i) {
		_tracks[i]->setTime(timeNow);
	}
}

void PoliceMaze::tick() {
	if (_isPaused) {
		return;
	}

	if (_vm->_scene->getSetId() != kSetPS10_PS11_PS12_PS13) {
		return;
	}

	if (_isEnding) {
		_isActive = false;
		return;
	}

	for (int i = 0; i < kNumMazeTracks; ++i) {
		_tracks[i]->tick();
	}

	bool notFound = true;
	for (int i = 0; i < kNumMazeTracks; ++i) {
		if (!_tracks[i]->isPaused()) {
			notFound = false;
			break;
		}
	}

	if (_vm->_debugger->_showMazeScore && _isActive && !_isEnding) {
		_vm->_subtitles->setGameSubsText(Common::String::format("Score: %02d", Global_Variable_Query(kVariablePoliceMazeScore)), true);
		_vm->_subtitles->show();
	}

	if (notFound && _isActive && !_isEnding) {
		_isActive = false;
		_isEnding = true;

		if (_vm->_scene->getSceneId() == kScenePS13) {
			Actor_Voice_Over(320, kActorAnsweringMachine);
		} else {
			Actor_Voice_Over(310, kActorAnsweringMachine);
		}
	}

}

void PoliceMaze::save(SaveFileWriteStream &f) {
	f.writeBool(_isPaused);
	f.writeBool(_isActive);
	f.writeBool(_isEnding);
	for (int i = 0; i < kNumMazeTracks; ++i) {
		_tracks[i]->save(f);
	}
}

void PoliceMaze::load(SaveFileReadStream &f) {
	_isPaused = f.readBool();
	_isActive = f.readBool();
	_isEnding = f.readBool();
	for (int i = 0; i < kNumMazeTracks; ++i) {
		_tracks[i]->load(f);
	}
}

PoliceMazeTargetTrack::PoliceMazeTargetTrack(BladeRunnerEngine *vm) : ScriptBase(vm) {
	reset();
}

PoliceMazeTargetTrack::~PoliceMazeTargetTrack() {
	reset();
}

void PoliceMazeTargetTrack::reset() {
	_isPresent      = false;
	_itemId         = -1;
	_pointCount     = 0;
	_data           = nullptr;
	_dataIndex      = 0;
	_timeLeftUpdate = 0;
	_timeLeftWait   = 0;
	_time           = 0;
	_isWaiting      = false;
	_isMoving       = false;
	_pointIndex     = 0;
	_pointTarget    = 0;
	_isRotating     = false;
	_angleTarget    = 0;
	_angleDelta     = 0;
	_isPaused       = true;
}

void PoliceMazeTargetTrack::clear(bool isLoadingGame) {
	reset();
}

void PoliceMazeTargetTrack::add(int trackId, float startX, float startY, float startZ, float endX, float endY, float endZ, int steps, const int *instructions, bool isActive) {
	_data = (const int *)instructions;

	if (!_vm->_gameIsLoading) {
		_itemId = trackId;
		_pointCount = steps;
		_dataIndex = 0;

		double coef = 1.0f / (long double)steps;

		double coefX = (endX - startX) * coef;
		double coefY = (endY - startY) * coef;
		double coefZ = (endZ - startZ) * coef;

		for (int i = 0; i < steps - 1; ++i) {
			_points[i].x = i * coefX + startX;
			_points[i].y = i * coefY + startY;
			_points[i].z = i * coefZ + startZ;
		}

		_points[steps - 1].x = endX;
		_points[steps - 1].y = endY;
		_points[steps - 1].z = endZ;

		_isPaused = !isActive;
	}
	_isPresent = true;
}

bool PoliceMazeTargetTrack::tick() {
	if (!_isPresent) {
		return false;
	}

	uint32 oldTime = _time;
	_time = _vm->_time->current();
	uint32 timeDiff = _time - oldTime;  // unsigned difference is intentional
	_timeLeftUpdate = _timeLeftUpdate - (int32)timeDiff; // should be ok

	if (_timeLeftUpdate > 0) {
		return false;
	}

#if BLADERUNNER_ORIGINAL_BUGS
#else
	// here _timeLeftUpdate is <= 0
	if (_vm->_settings->getDifficulty() > kGameDifficultyEasy) {
		timeDiff = abs(_timeLeftUpdate);
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
	_timeLeftUpdate = 66; // update the target track 15 times per second

	if (_isPaused) {
		return false;
	}

	if (_isWaiting) {
#if BLADERUNNER_ORIGINAL_BUGS
		_timeLeftWait -= timeDiff;
#else
		if (_vm->_settings->getDifficulty() == kGameDifficultyEasy) {
			_timeLeftWait -= timeDiff; // original behavior
		} else {
			_timeLeftWait = _timeLeftWait - (int32)(timeDiff + _timeLeftUpdate); // this deducts an amount >= 66 // should be ok
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		if (_timeLeftWait > 0) {
			return true;
		}

		_isWaiting = false;
		_timeLeftWait = 0;
	}

	if (_vm->_items->isSpinning(_itemId)) {
		return true;
	}

	if (_isRotating) {
		float angle = _vm->_items->getFacing(_itemId) + _angleDelta;

		if (_angleDelta > 0) {
			if (angle >= _angleTarget) {
				angle = _angleTarget;
				_isRotating = false;
			}
		} else if (_angleDelta < 0) {
			if (angle <= _angleTarget) {
				angle = _angleTarget;
				_isRotating = false;
			}
		} else {
			_isRotating = false;
		}

		_vm->_items->setFacing(_itemId, angle);

		if (_isRotating) {
			return true;
		}
	}

	bool advancePoint = false;

	if (_isMoving) {
		if (_pointIndex < _pointTarget) {
			++_pointIndex;
			advancePoint = true;
		} else if (_pointIndex > _pointTarget) {
			--_pointIndex;
			advancePoint = true;
		} else {
			_isMoving = 0;
		}
	}

	if (advancePoint) {
		_vm->_items->setXYZ(_itemId, _points[_pointIndex]);
		readdObject(_itemId);

		return true;
	}

	bool cont = true;

	while (cont) {
		++_dataIndex;

		switch (_data[_dataIndex - 1]) {
		case kPMTIActivate:
			{
				int variableId = _data[_dataIndex++];
				int maxValue = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Activate, VariableId: %i, Max value: %i", _itemId, variableId, maxValue);
#endif
				if (Global_Variable_Query(variableId) >= maxValue) {
					setPaused();
					cont = false;
				} else {
					cont = true;
				}
				break;
			}

		case kPMTILeave:
			{
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Leave", _itemId);
#endif
				if (!_vm->_items->isPoliceMazeEnemy(_itemId) && _vm->_items->isTarget(_itemId)) {
					Police_Maze_Increment_Score(1);
				}
				break;
			}

		case kPMTIShoot:
			{
				int soundId = _data[_dataIndex++];
				++_dataIndex; // second argument is not used
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Shoot, SoundId: %i", _itemId, soundId);
#endif
				if (_vm->_items->isTarget(_itemId)) {
					Sound_Play(soundId, 90, 0, 0, 50);
					Police_Maze_Decrement_Score(1);
					Actor_Force_Stop_Walking(kActorMcCoy);

					if (Player_Query_Combat_Mode()) {
						Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeCombatHit);
					} else {
						Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeHit);
					}

					int snd;

					if (Random_Query(1, 2) == 1) {
						snd = 9900;
					} else {
						snd = 9905;
					}
					Sound_Play_Speech_Line(kActorMcCoy, snd, 75, 0, 99);

					_vm->_mouse->setMouseJitterDown();
				}

				cont = false;
				break;
			}

		case kPMTIEnemyReset:
			{
				int itemId = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Enemy reset, OtherItemId: %i", _itemId, itemId);
#endif
				_vm->_items->setPoliceMazeEnemy(itemId, false);
				break;
			}

		case kPMTIEnemySet:
			{
				int itemId = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Enemy set, OtherItemId: %i", _itemId, itemId);
#endif
				_vm->_items->setPoliceMazeEnemy(itemId, true);
				break;
			}

		case kPMTIFlagReset:
			{
				int gameFlagId = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Flag reset, FlagId: %i", _itemId, gameFlagId);
#endif
				Game_Flag_Reset(gameFlagId);
				break;
			}

		case kPMTIFlagSet:
			{
				int gameFlagId = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Flag set, FlagId: %i", _itemId, gameFlagId);
#endif
				Game_Flag_Set(gameFlagId);
				break;
			}

		case kPMTIVariableDec:
			{
				int variableId = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Variable decrement, VariableId: %i", _itemId, variableId);
#endif
				Global_Variable_Decrement(variableId, 1);
				break;
			}

		case kPMTIVariableInc:
			{
				int variableId = _data[_dataIndex++];
				int maxValue = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Variable increment, VariableId: %i, Max value: %i", _itemId, variableId, maxValue);
#endif
				if (Global_Variable_Query(variableId) < maxValue) {
					Global_Variable_Increment(variableId, 1);
				}
				break;
			}

		case kPMTIVariableReset:
			{
				int variableId = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Variable reset, VariableId: %i", _itemId, variableId);
#endif
				Global_Variable_Reset(variableId);
				break;
			}

		case kPMTIVariableSet:
			{
				int variableId = _data[_dataIndex++];
				int value = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Variable set, VariableId: %i, Value: %i", _itemId, variableId, value);
#endif
				Global_Variable_Set(variableId, value);
				break;
			}

		case kPMTITargetSet:
			{
				int itemId = _data[_dataIndex++];
				int value = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Target set, OtherItemId: %i, Value: %i", _itemId, itemId, value);
#endif
				_vm->_items->setIsTarget(itemId, value);
				break;
			}

		case kPMTIPausedReset1of3:
			{
				int trackId1 = _data[_dataIndex++];
				int trackId2 = _data[_dataIndex++];
				int trackId3 = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Pause reset 1 of 3, OtherItemId1: %i, OtherItemId2: %i, OtherItemId3: %i", _itemId, trackId1, trackId2, trackId3);
#endif
				switch (Random_Query(1, 3)) {
				case 1:
					_vm->_policeMaze->_tracks[trackId1]->resetPaused();
					break;

				case 2:
					_vm->_policeMaze->_tracks[trackId2]->resetPaused();
					break;

				case 3:
					_vm->_policeMaze->_tracks[trackId3]->resetPaused();
					break;

				default:
					break;
				}

				break;
			}

		case kPMTIPausedReset1of2:
			{
				int trackId1 = _data[_dataIndex++];
				int trackId2 = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Pause reset 1 of 2, OtherItemId1: %i, OtherItemId2: %i", _itemId, trackId1, trackId2);
#endif
				if (Random_Query(1, 2) == 1) {
					_vm->_policeMaze->_tracks[trackId1]->resetPaused();
				} else {
					_vm->_policeMaze->_tracks[trackId2]->resetPaused();
				}
				break;
			}

		case kPMTIPausedSet:
			{
				int trackId = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Pause set, OtherItemId: %i", _itemId, trackId);
#endif
				_vm->_policeMaze->_tracks[trackId]->setPaused();
				break;
			}

		case kPMTIPausedReset:
			{
				int trackId = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Pause reset, OtherItemId: %i", _itemId, trackId);
#endif
				_vm->_policeMaze->_tracks[trackId]->resetPaused();
				break;
			}

		case kPMTIPlaySound:
			{
				int soundId = _data[_dataIndex++];
				int volume = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Sound, SoundId: %i, Volume: %i", _itemId, soundId, volume);
#endif
				Sound_Play(soundId, volume, 0, 0, 50);
				break;
			}

		case kPMTIObstacleReset:
			{
				int itemId = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Obstacle reset, OtherItemId: %i", _itemId, itemId);
#endif
				_vm->_items->setIsObstacle(itemId, 0);
				break;
			}

		case kPMTIObstacleSet:
			{
				int itemId = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Obstacle set, OtherItemId: %i", _itemId, itemId);
#endif
				_vm->_items->setIsObstacle(itemId, 1);
				break;
			}

		case kPMTIWaitRandom:
			{
				int randomMin = _data[_dataIndex++];
				int randomMax = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Wait random, Min: %i, Max: %i", _itemId, randomMin, randomMax);
#endif
				_timeLeftWait = Random_Query(randomMin, randomMax);
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Wait for = %i", _itemId, _timeLeftWait);
#endif
				_isWaiting = true;

				cont = false;
				break;
			}

		case kPMTIRotate:
			{
				_angleTarget = _data[_dataIndex++];
				_angleDelta = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Rotate, Target: %i, Delta: %i", _itemId, _angleTarget, _angleDelta);
#endif
				_isRotating = true;

				cont = false;
				break;
			}

		case kPMTIFacing:
			{
				int angle = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Set facing, Angle: %i", _itemId, angle);
#endif
				_vm->_items->setFacing(_itemId, angle);
				break;
			}

		case kPMTIRestart:
			{
				_dataIndex = 0;
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Restart", _itemId);
#endif
				cont = false;
				break;
			}

		case kPMTIWait:
			{
				_timeLeftWait = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Wait, Time: %i", _itemId, _timeLeftWait);
#endif
				_isWaiting = true;

				cont = false;
				break;
			}

		case kPMTIMove:
			{
				_pointTarget = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Move, Target: %i", _itemId, _pointTarget);
#endif
				_isMoving = true;

				cont = false;
				break;
			}

		case kPMTIPosition:
			{
				_pointIndex = _data[_dataIndex++];
#if BLADERUNNER_DEBUG_CONSOLE
				debug("ItemId: %3i, Position, Index: %i", _itemId, _pointIndex);
#endif
				_isMoving = false;
				_vm->_items->setXYZ(_itemId, _points[_pointIndex]);
				readdObject(_itemId);
				break;
			}

		default:
			return false;
		}

		if (_isPaused || _isWaiting) {
			cont = false;
		}
	}

	return true;
}

void PoliceMazeTargetTrack::readdObject(int itemId) {
	if (_vm->_sceneObjects->remove(itemId + kSceneObjectOffsetItems)) {
		const BoundingBox &boundingBox = _vm->_items->getBoundingBox(itemId);
		const Common::Rect &screenRect = _vm->_items->getScreenRectangle(itemId);
		bool targetable = _vm->_items->isTarget(itemId);
		bool obstacle = _vm->_items->isVisible(itemId);

		_vm->_sceneObjects->addItem(itemId + kSceneObjectOffsetItems, boundingBox, screenRect, targetable, obstacle);
	}
}

void PoliceMazeTargetTrack::save(SaveFileWriteStream &f) {
	f.writeBool(_isPresent);
	f.writeInt(_itemId);
	f.writeInt(_pointCount);
	f.writeInt(_dataIndex);
	f.writeBool(_isWaiting);
	f.writeBool(_isMoving);
	f.writeInt(_pointIndex);
	f.writeInt(_pointTarget);
	f.writeBool(_isRotating);
	f.writeInt(_angleTarget);
	f.writeInt(_angleDelta);
	f.writeBool(_isPaused);

	for (int i = 0; i < kNumTrackPoints; ++i) {
		f.writeVector3(_points[i]);
	}

	f.writeInt(_timeLeftUpdate);
	f.writeInt(_timeLeftWait);
}

void PoliceMazeTargetTrack::load(SaveFileReadStream &f) {
	_isPresent = f.readBool();
	_itemId = f.readInt();
	_pointCount = f.readInt();
	_dataIndex = f.readInt();
	_isWaiting = f.readBool();
	_isMoving = f.readBool();
	_pointIndex = f.readInt();
	_pointTarget = f.readInt();
	_isRotating = f.readBool();
	_angleTarget = f.readInt();
	_angleDelta = f.readInt();
	_isPaused = f.readBool();

	for (int i = 0; i < kNumTrackPoints; ++i) {
		_points[i] = f.readVector3();
	}

	_timeLeftUpdate = f.readInt();
	_timeLeftWait = f.readInt();
}

} // End of namespace BladeRunner
