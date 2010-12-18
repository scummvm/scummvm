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

#include "asylum/puzzles/pipes.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

PuzzlePipes::PuzzlePipes(AsylumEngine *engine) : Puzzle(engine) {
	_previousMusicVolume = 0;
	_rectIndex = -2;
}

PuzzlePipes::~PuzzlePipes() {
}

void PuzzlePipes::reset() {
	warning("[PuzzlePipes::reset] Not implemented!");
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzlePipes::init(const AsylumEvent &evt) {
	_previousMusicVolume = getSound()->getMusicVolume();

	if (_previousMusicVolume >= -1000)
		getSound()->setMusicVolume(-1000);

	getSound()->playSound(getWorld()->graphicResourceIds[41], true, Config.ambientVolume);
	getScreen()->setPalette(getWorld()->graphicResourceIds[0]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[0], 0);

	_rectIndex = -2;

	updateCursor();
	initResources();
	setup(true);

	return true;
}

bool PuzzlePipes::update(const AsylumEvent &evt) {
	error("[PuzzlePipes::update] Not implemented!");
}

bool PuzzlePipes::mouseLeftDown(const AsylumEvent &evt) {
	error("[PuzzlePipes::mouseLeftDown] Not implemented!");
}

bool PuzzlePipes::mouseRightDown(const AsylumEvent &evt) {
	getScreen()->clear();
	getSound()->stop(getWorld()->graphicResourceIds[41]);
	getSound()->setMusicVolume(_previousMusicVolume);

	_vm->switchEventHandler(getScene());

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
void PuzzlePipes::initResources() {
	error("[PuzzlePipes::initResources] Not implemented!");
}

void PuzzlePipes::setup(bool val) {
	error("[PuzzlePipes::setup] Not implemented!");
}

void PuzzlePipes::updateCursor() {
	error("[PuzzlePipes::updateCursor] Not implemented!");
}

int32 PuzzlePipes::findRect() {
	error("[PuzzlePipes::findRect] Not implemented!");
}

void PuzzlePipes::checkFlags() {
	error("[PuzzlePipes::checkFlags] Not implemented!");
}

} // End of namespace Asylum
