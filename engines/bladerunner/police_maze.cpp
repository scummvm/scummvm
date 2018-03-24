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
#include "bladerunner/police_maze.h"
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/script/scene_script.h"

namespace BladeRunner {

PoliceMaze::PoliceMaze(BladeRunnerEngine *vm) : ScriptBase(vm) {
	reset();

	for (int i = 0; i < kNumMazeTracks; i++) {
		_tracks[i] = new PoliceMazeTargetTrack(vm);
	}
}

PoliceMaze::~PoliceMaze() {
	for (int i = 0; i < kNumMazeTracks; i++) {
		delete _tracks[i];
	}

	reset();
}

void PoliceMaze::reset() {
	_isPaused = false;
	_needAnnouncement = false;
	_announcementRead = false;

	for (int i = 0; i < kNumMazeTracks; i++) {
		_tracks[i] = 0;
	}

	_pm_var1 = 0;
	_pm_var2 = 0;
}

void PoliceMaze::clear(bool isLoadingGame) {
	for (int i = 0; i < kNumMazeTracks; i++) {
		if (_tracks[i]->isPresent())
			_tracks[i]->clear(isLoadingGame);
	}
}

void PoliceMaze::activate() {
	_needAnnouncement = true;
	_announcementRead = false;
}

void PoliceMaze::setPauseState(bool state) {
	_isPaused = state;

	uint32 t = _vm->getTotalPlayTime();

	for (int i = 0; i < kNumMazeTracks; i++)
		_tracks[i]->setTime(t);
}

void PoliceMaze::tick() {
	if (_isPaused)
		return;

	if (_vm->_scene->getSetId() != kSetPS10_PS11_PS12_PS13)
		return;

	if (_announcementRead) {
		_needAnnouncement = false;

		return;
	}

	for (int i = 0; i < kNumMazeTracks; i++)
		_tracks[i]->tick();

	bool notFound = true;
	for (int i = 0; i < kNumMazeTracks; i++) {
		if (!_tracks[i]->isVisible()) {
			notFound = false;
			break;
		}
	}

	if (notFound && _needAnnouncement && !_announcementRead) {
		_needAnnouncement = false;
		_announcementRead = true;

		if (_vm->_scene->getSceneId() == kScenePS13) {
			Actor_Voice_Over(320, kActorAnsweringMachine);
		} else {
			Actor_Voice_Over(310, kActorAnsweringMachine);
		}
	}
}

PoliceMazeTargetTrack::PoliceMazeTargetTrack(BladeRunnerEngine *vm) : ScriptBase(vm) {
	reset();
}

PoliceMazeTargetTrack::~PoliceMazeTargetTrack() {
	reset();
}

void PoliceMazeTargetTrack::reset() {
	_isPresent = 0;
	_itemId = -1;
	_count = 0;
	_data = 0;
	_dataIndex = 0;
	_updateDelay = 0;
	_waitTime = 0;
	_time = 0;
	_haveToWait = false;
	_pmt_var4 = 0;
	_pointIndex = 0;
	_pmt_var5 = 0;
	_rotating = false;
	_maxAngle = 0;
	_angleChange = 0;
	_visible = true;
}

void PoliceMazeTargetTrack::clear(bool isLoadingGame) {
	reset();
}

void PoliceMazeTargetTrack::add(int trackId, float startX, float startY, float startZ, float endX, float endY, float endZ, int count, void *list, bool a11) {
	_data = (int *)list;

	if (true /* !GameIsLoading */) { // FIXME
		_itemId = trackId;
		_count = count;
		_dataIndex = 0;

		double coef = 1.0f / (long double)count;

		double coefX = (endX - startX) * coef;
		double coefY = (endY - startY) * coef;
		double coefZ = (endZ - startZ) * coef;

		for (int i = 0; i < count; i++) {
			_points[i].x = i * coefX + startX;
			_points[i].y = i * coefY + startY;
			_points[i].z = i * coefZ + startZ;
		}

		_points[count].x = endX;
		_points[count].y = endY;
		_points[count].z = endZ;

		_visible = !a11;
	}
	_isPresent = true;
}

bool PoliceMazeTargetTrack::tick() {
	if (!_isPresent) {
		return false;
	}

#if 0

	uint32 oldTime = _time;
	_time = _vm->getTotalPlayTime();
	int32 timeDiff = _time - oldTime;
	_updateDelay -= timeDiff;

	if (_updateDelay > 0)
		return false;

	_updateDelay = 66;

	if (_visible)
		return false;

	if (_haveToWait) {
		_waitTime -= timeDiff;

		if (_waitTime > 0)
			return true;

		_haveToWait = false;
		_waitTime = 0;
	}

	if (_vm->_items->isTarget(_itemId))
		return true;

	if (_rotating) {
		float angle = _vm->_items->getFacing(_itemId) + _angleChange;

		if (_angleChange > 0) {
			if (angle >= _maxAngle) {
				angle = _maxAngle;
				_rotating = false;
			}
		} else if (_angleChange < 0) {
			if (angle <= _maxAngle) {
				angle = _maxAngle;
				_rotating = false;
			}
		} else {
			_rotating = false;
		}

		_vm->_items->setFacing(_itemId, angle);

		if (_rotating)
			return false;
	}

	bool advancePoint = false;

	if (_pmt_var4) {
		if (_pointIndex < _pmt_var5) {
			_pointIndex++;
			advancePoint = true;
		} else if (_pointIndex > _pmt_var5) {
			_pointIndex--;
			advancePoint = true;
		} else {
			_pmt_var4 = 0;
		}
	}

	if (advancePoint) {
		_vm->_items->setXYZ(_itemId, _points[_pointIndex].x, _points[_pointIndex].y, _points[_pointIndex].z);
		readdObject(_itemId);

		return true;
	}

	bool cont = true;
	int var1 = 0, var2 = 0, var3 = 0, varRes = 0;

	while (cont) {
		_dataIndex++;

		switch (_data[_dataIndex - 1] + 26) {
		case 0:
			var1 = _data[_dataIndex++];
			var2 = _data[_dataIndex++];

			if (Global_Variable_Query(var1) >= Global_Variable_Query(var2)) {
				setVisible();
				cont = false;
			} else {
				cont = true;
			}
			break;

		case 1:
			if (!_vm->_items->isPoliceMazeEnemy(_itemId) && _vm->_items->isTarget(_itemId)) {
				Police_Maze_Increment_Score(1);
			}
			break;

		case 2:
			var1 = _data[_dataIndex++];
			_dataIndex++;

			if (_vm->_items->isTarget(_itemId)) {
				Sound_Play(var1, 90, 0, 0, 50, 0);
				Police_Maze_Decrement_Score(1);
				Actor_Force_Stop_Walking(0);

				if (Player_Query_Combat_Mode() == 1) {
					Actor_Change_Animation_Mode(0, 22);
				} else {
					Actor_Change_Animation_Mode(0, 21);
				}

				int snd;

				if (Random_Query(1, 2) == 1) {
					snd = 9900;
				} else {
					snd = 9905;
				}
				Sound_Play_Speech_Line(0, snd, 75, 0, 99);

				_vm->_mouse->setRandomY();
			}

			cont = false;
			break;

		case 3:
			var1 = _data[_dataIndex++];
			_vm->_items->setPoliceMazeEnemy(var1, 0);
			break;

		case 4:
			var1 = _data[_dataIndex++];
			_vm->_items->setPoliceMazeEnemy(var1, 1);
			break;

		case 5:
			var1 = _data[_dataIndex++];
			Game_Flag_Reset(var1);
			break;

		case 6:
			var1 = _data[_dataIndex++];
			Game_Flag_Set(var1);
			break;

		case 7:
			var1 = _data[_dataIndex++];
			Global_Variable_Decrement(var1, 1);
			break;

		case 8:
			var1 = _data[_dataIndex++];
			var2 = _data[_dataIndex++];
			if (Global_Variable_Query(var1) < var2) {
				Global_Variable_Increment(var1, 1);
			}
			break;

		case 9:
			var1 = _data[_dataIndex++];
			Global_Variable_Reset(var1);
			break;

		case 10:
			var1 = _data[_dataIndex++];
			var2 = _data[_dataIndex++];
			Global_Variable_Set(var1, var2);
			break;

		case 11:
			var1 = _data[_dataIndex++];
			var2 = _data[_dataIndex++];
			_vm->_items->setIsTarget(var1, var2);
			break;

		case 12:
			var1 = _data[_dataIndex++];
			var2 = _data[_dataIndex++];
			var3 = _data[_dataIndex++];

			switch (Random_Query(1, 3)) {
			case 1:
				varRes = var1;
				break;

			case 2:
				varRes = var2;
				break;

			case 3:
				varRes = var3;
				break;
			}

			_vm->_policeMaze->_tracks[varRes]->resetVisible();
			break;

		case 13:
			var1 = _data[_dataIndex++];
			var2 = _data[_dataIndex++];

			if (Random_Query(1, 2) == 1) {
				varRes = var1;
			} else {
				varRes = var2;
			}
			_vm->_policeMaze->_tracks[varRes]->resetVisible();
			break;

		case 14:
			var1 = _data[_dataIndex++];
			_vm->_policeMaze->_tracks[var1]->setVisible();
			break;

		case 15:
			var1 = _data[_dataIndex++];
			_vm->_policeMaze->_tracks[var1]->resetVisible();
			break;

		case 16:
			var1 = _data[_dataIndex++];
			var2 = _data[_dataIndex++];
			Sound_Play(var1, var2, 0, 0, 50, 0);
			break;

		case 17:
			var1 = _data[_dataIndex++];
			_vm->_items->setIsObstacle(var1, 0);
			break;

		case 18:
			var1 = _data[_dataIndex++];
			_vm->_items->setIsObstacle(var1, 1);
			break;

		case 19:
			var1 = _data[_dataIndex++];
			var2 = _data[_dataIndex++];
			_waitTime = Random_Query(var1, var2);
			_haveToWait = true;

			cont = false;
			break;

		case 20:
			_maxAngle = _data[_dataIndex++];
			_angleChange = _data[_dataIndex++];
			_rotating = true;

			cont = false;
			break;

		case 21:
			var1 = _data[_dataIndex++];
			_vm->_items->setFacing(_itemId, var1);
			break;

		case 22:
			_dataIndex = 0;

			cont = false;
			break;

		case 23:
			_waitTime = _data[_dataIndex++];
			_haveToWait = true;

			cont = false;
			break;

		case 24:
			_pmt_var5 = _data[_dataIndex++];
			_pmt_var4 = 1;

			cont = false;
			break;

		case 25:
			_pointIndex = _data[_dataIndex++];
			_pmt_var4 = 0;
			_vm->_items->setXYZ(_itemId, _points[_pointIndex].x, _points[_pointIndex].y, _points[_pointIndex].z);
			readdObject(_itemId);
			break;

		default:
			return false;
		}

		if (_visible || _haveToWait) {
			cont = false;
		}
	}
#endif

	return true;
}

void PoliceMazeTargetTrack::readdObject(int itemId) {
	if (_vm->_sceneObjects->remove(itemId + kSceneObjectOffsetItems)) {
		BoundingBox *boundingBox = _vm->_items->getBoundingBox(itemId);
		Common::Rect *screenRect = _vm->_items->getScreenRectangle(itemId);
		bool targetable = _vm->_items->isTarget(itemId);
		bool obstacle = _vm->_items->isVisible(itemId);

		_vm->_sceneObjects->addItem(itemId + kSceneObjectOffsetItems, boundingBox, screenRect, targetable, obstacle);
	}
}


} // End of namespace BladeRunner
