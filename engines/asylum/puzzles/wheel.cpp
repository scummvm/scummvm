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

#include "asylum/puzzles/wheel.h"

#include "asylum/resources/special.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

PuzzleWheel::PuzzleWheel(AsylumEngine *engine) : Puzzle(engine) {
	_currentRect = -1;
	_resourceIndex = 0;
	_resourceIndex10 = 13;

	_frameIndex30 = 0;
	memset(&_frameIndexes, -1, sizeof(_frameIndexes));
	memset(&_frameCounts, 0, sizeof(_frameCounts));

	_flag1 = false;
	_flag2 = false;
	_flag3 = false;
}

PuzzleWheel::~PuzzleWheel() {
}

void PuzzleWheel::reset() {
	getSpecial()->reset(true);

	_resourceIndex = 0;
	_resourceIndex10 = 13;

	_frameIndex30 = 0;

	_frameCounts[0] = 0;
	_frameCounts[9] = 0;
	_frameCounts[10] = 0;
	_frameCounts[11] = 0;

	_flag1 = false;
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleWheel::init(const AsylumEvent &evt)  {
	getSpecial()->reset(false);

	getScreen()->setPalette(getWorld()->graphicResourceIds[1]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[1], 0);

	updateCursor(evt);
	getCursor()->show();

	_currentRect = -2;

	memset(&_frameIndexes, -1, sizeof(_frameIndexes));

	for (uint32 i = 0; i < 8; i++) {
		if (_vm->isGameFlagSet((GameFlag)(kGameFlag253 + i)))
			_frameCounts[i + 1] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[i + 14]) - 1;
		else
			_frameCounts[i + 1] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[i + 4]) - 1;
	}

	return true;
}

bool PuzzleWheel::update(const AsylumEvent &evt)  {
	error("[PuzzleWheel::update] Not implemented!");
}

bool PuzzleWheel::mouseLeftDown(const AsylumEvent &evt) {
	error("[PuzzleWheel::mouseLeftDown] Not implemented!");
}

bool PuzzleWheel::mouseRightDown(const AsylumEvent &evt) {
	getScreen()->clear();
	_vm->switchEventHandler(getScene());

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
void PuzzleWheel::updateCursor(const AsylumEvent &evt) {
	warning("[PuzzleWheel::updateCursor] Not implemented!");
}

int32 PuzzleWheel::findRect(Common::Point mousePos) {
	error("[PuzzleWheel::findRect] Not implemented!");
}

void PuzzleWheel::checkFlags() {
	error("[PuzzleWheel::checkFlags] Not implemented!");
}

void PuzzleWheel::playSound() {
	error("[PuzzleWheel::playSound] Not implemented!");
}

void PuzzleWheel::playSoundReset() {
	error("[PuzzleWheel::playSoundReset] Not implemented!");
}


} // End of namespace Asylum
