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
#include "bladerunner/police_maze.h"
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/script/scene_script.h"

namespace BladeRunner {

PoliceMaze::PoliceMaze(BladeRunnerEngine *vm) {
	_vm = vm;

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
			_vm->_sceneScript->actorVoiceOver(320, kActorAnsweringMachine);
		} else {
			_vm->_sceneScript->actorVoiceOver(310, kActorAnsweringMachine);
		}
	}
}

PoliceMazeTargetTrack::PoliceMazeTargetTrack(BladeRunnerEngine *vm) {
	_vm = vm;

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
	_pmt_var1 = 0;
	_pmt_var2 = 0;
	_time = 0;
	_pmt_var3 = 0;
	_pmt_var4 = 0;
	_pointIndex = 0;
	_pmt_var5 = 0;
	_rotating = 0;
	_maxAngle = 0;
	_angleChange = 0;
	_visible = true;
}

void PoliceMazeTargetTrack::clear(bool isLoadingGame) {
	reset();
}

void PoliceMazeTargetTrack::add(int trackId, float startX, float startY, float startZ, float endX, float endY, float endZ, int count, void *list, bool a11) {
	warning("PoliceMazeTargetTrack::add(%d, %f, %f, %f, %f, %f, %f, %d, %p, %d)", trackId,  startX,  startY,  startZ,  endX,  endY,  endZ,  count,  (void *)list, a11);
}

void PoliceMazeTargetTrack::tick() {
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
