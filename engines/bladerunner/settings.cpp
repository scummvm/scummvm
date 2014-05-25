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

#include "bladerunner/settings.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/chapters.h"
#include "bladerunner/scene.h"

#include "common/debug.h"

namespace BladeRunner {

Settings::Settings(BladeRunnerEngine *vm)
	: _vm(vm)
{
	_chapter = 1;
	_gamma = 1.0f;

	_chapterChanged = false;
	_newChapter = -1;
	_newScene = -1;
	_newSet = -1;

	_startingGame = true;
	_loadingGame = false;

	_fullHDFrames = true;
	_mst3k = false;
}

bool Settings::openNewScene() {
	if (_newSet == -1) {
		assert(_newScene == -1);
		return true;
	}
	assert(_newScene != -1);

	if (_startingGame) {
		// Stop ambient audio and music
	}

	int currentSet = _vm->_scene->_setId;
	int newSet     = _newSet;
	int newScene   = _newScene;

	_newSet = -1;
	_newScene = -1;

	if (_chapterChanged) {
		if (_vm->_chapters->hasOpenResources())
			_vm->_chapters->closeResources();

		int newChapter = _newChapter;
		_chapterChanged = false;
		_newChapter = 0;
		if (!_vm->_chapters->enterChapter(newChapter)) {
			_vm->_gameIsRunning = false;
			return false;
		}
		_chapter = newChapter;
		if (_startingGame)
			_startingGame = false;
	}

	if (!_vm->_scene->open(newSet, newScene, _loadingGame)) {
		_vm->_gameIsRunning = false;
		return false;
	}

	if (!_loadingGame && currentSet != newSet) {
		// TODO: Reset actors for new set
	}

	_loadingGame = false;
	return true;
}

} // End of namespace BladeRunner
