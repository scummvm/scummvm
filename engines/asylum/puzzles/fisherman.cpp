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

#include "asylum/puzzles/fisherman.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

PuzzleFisherman::PuzzleFisherman(AsylumEngine *engine) : Puzzle(engine) {
	memset(&_state, 0, sizeof(_state));

	_dword_45AAD4 = false;
	_counter = 0;

	_dword_45A130 = false;

}

PuzzleFisherman::~PuzzleFisherman() {
}

//////////////////////////////////////////////////////////////////////////
// Reset
//////////////////////////////////////////////////////////////////////////
void PuzzleFisherman::reset() {
	memset(&_state, 0, sizeof(_state));
	_dword_45AAD4 = false;

	// Original setups polygons here

	_dword_45A130 = false;
	_counter = 0;

	// TODO update scene fields
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleFisherman::init()  {
	getCursor()->set(getWorld()->graphicResourceIds[47], -1, kCursorAnimationMirror, 7);

	for (uint32 i = 0; i < ARRAYSIZE(_state); i++)
		if (_vm->isGameFlagNotSet((GameFlag)(kGameFlag801 + i)))
			_state[i] = 0;

	if (_counter == 6) {
		_vm->clearGameFlag(kGameFlag619);
		_counter = 0;
	}

	_dword_45A130 = false;
	getScreen()->setPalette(getWorld()->graphicResourceIds[39]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[39], 0);

	return mouseDown();
}

bool PuzzleFisherman::update()  {
	error("[PuzzleFisherman::update] Not implemented!");
}

bool PuzzleFisherman::key(const AsylumEvent &evt) {
	switch (evt.kbd.keycode) {
	default:
		_vm->switchEventHandler(getScene());
		break;

	case Common::KEYCODE_TAB:
		getScreen()->takeScreenshot();
		break;
	}

	return false;
}

bool PuzzleFisherman::mouse(const AsylumEvent &evt) {
	switch (evt.type) {
	case Common::EVENT_LBUTTONDOWN:
		return mouseDown();
		break;

	case Common::EVENT_RBUTTONDOWN:
		getCursor()->hide();
		getSharedData()->setFlag(kFlag1, true);
		getScreen()->setupPaletteAndStartFade(0, 0, 0);

		_vm->switchEventHandler(getScene());
		break;
	}

	return false;
}

bool PuzzleFisherman::mouseDown() {
	if (!_dword_45A130)
		return false;

	error("[PuzzleFisherman::mouseDown] Not implemented!");
}

} // End of namespace Asylum
