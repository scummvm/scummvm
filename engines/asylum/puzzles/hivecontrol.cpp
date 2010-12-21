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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "asylum/puzzles/hivecontrol.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

PuzzleHiveControl::PuzzleHiveControl(AsylumEngine *engine) : Puzzle(engine) {
	_rectIndex = 0;
	_soundVolume = 0;
	_counter = 0;

	_data_457260 = 0;
	_data_457264 = 0;
}

PuzzleHiveControl::~PuzzleHiveControl() {
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleHiveControl::init(const AsylumEvent &evt)  {
	error("[PuzzleHiveControl::init] Not implemented!");

	_rectIndex = -2;
	updateCursor();

	getScreen()->setPalette(getWorld()->graphicResourceIds[29]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[29], 0);

	getCursor()->show();

	getScreen()->setupTransTables(1, getWorld()->graphicResourceIds[70]);
	getScreen()->selectTransTable(0);

	getSound()->playSound(getWorld()->graphicResourceIds[73], true, _soundVolume);
	getSound()->playSound(getWorld()->graphicResourceIds[74], true, Config.ambientVolume);

	return true;
}

bool PuzzleHiveControl::update(const AsylumEvent &evt)  {
	updateCursor();
	updateScreen();

	if (!_data_457260 && !_data_457264)
		playSound();

	if (_counter) {
		if (_counter < 30 || getSound()->isPlaying(getWorld()->graphicResourceIds[83])) {
			++ _counter;
		} else {
			mouseRightDown(evt);
			getCursor()->show();
		}
	}

	return true;
}

bool PuzzleHiveControl::mouseLeftDown(const AsylumEvent &evt) {
	error("[PuzzleHiveControl::mouseLeftDown] Not implemented!");
}

bool PuzzleHiveControl::mouseRightDown(const AsylumEvent &evt) {
	getSound()->stop(getWorld()->graphicResourceIds[73]);
	getSound()->stop(getWorld()->graphicResourceIds[74]);

	getScreen()->clear();
	getScreen()->setupTransTables(3, getWorld()->cellShadeMask1, getWorld()->cellShadeMask2, getWorld()->cellShadeMask3);
	getScreen()->selectTransTable(1);

	_vm->switchEventHandler(getScene());

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
void PuzzleHiveControl::updateCursor() {
	int32 index = findRect();

	if (_rectIndex == index)
		return;

	_rectIndex = index;
	if (index == -1)
		getCursor()->set(getWorld()->graphicResourceIds[30], -1, kCursorAnimationNone);
	else
		getCursor()->set(getWorld()->graphicResourceIds[30], -1);
}

int32 PuzzleHiveControl::findRect() {
	error("[PuzzleHiveControl::findRect] Not implemented!");
}

void PuzzleHiveControl::updateScreen() {
	error("[PuzzleHiveControl::updateScreen] Not implemented!");
}

void PuzzleHiveControl::playSound() {
	error("[PuzzleHiveControl::playSound] Not implemented!");
}

} // End of namespace Asylum
