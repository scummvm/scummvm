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

#include "asylum/puzzles/puzzle11.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"


namespace Asylum {

Puzzle11::Puzzle11(AsylumEngine *engine) : Puzzle(engine) {
	_counter = 0;
	memset(&_frameCounts, 0, sizeof(_frameCounts));
	memset(&_frameIndexes, 0, sizeof(_frameIndexes));
	_data_457298 = false;
	_data_45AA34 = false;
}

Puzzle11::~Puzzle11() {
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool Puzzle11::init()  {
	getPuzzleData()->timeMachineCounter = 0;

	getScreen()->setPalette(getWorld()->graphicResourceIds[12]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[12], 0);
	getCursor()->set(getWorld()->graphicResourceIds[60], -1, kCursorAnimationNone, 7);

	_data_45AA34 = false;

	_frameCounts[0] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[59]);
	_frameCounts[1] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[54]);
	_frameCounts[2] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[55]);
	_frameCounts[3] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[56]);
	_frameCounts[4] = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[13]);

	mouseDown();

	return true;
}

bool Puzzle11::update()  {
	getScreen()->clearGraphicsInQueue();
	getScreen()->draw(getWorld()->graphicResourceIds[11]);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[15], _frameIndexes[0], Common::Point(122, 269), 0, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[16], _frameIndexes[1], Common::Point(524, 175), 0, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[18], _frameIndexes[2], Common::Point(359,  61), 0, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[14], _frameIndexes[3], Common::Point(115,  80), 0, 0, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[17], _frameIndexes[4], Common::Point(238,  94), 0, 0, 1);

	getScreen()->drawGraphicsInQueue();
	getScreen()->copyBackBufferToScreen();
	warning("[Puzzle11::update] Not implemented!");

	return true;
}

bool Puzzle11::key(const AsylumEvent &evt) {
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

bool Puzzle11::mouse(const AsylumEvent &evt) {
	switch (evt.type) {
	default:
		break;

	case Common::EVENT_RBUTTONDOWN:
		getCursor()->hide();
		getSharedData()->setFlag(kFlag1, true);
		getScreen()->setupPaletteAndStartFade(0, 0, 0);

		_vm->switchEventHandler(getScene());
		break;

	case Common::EVENT_LBUTTONUP:
		mouseUp();
		break;

	case Common::EVENT_LBUTTONDOWN:
		mouseDown();
		return true;
	}

	return false;
}

void Puzzle11::mouseUp() {
	warning("[Puzzle11::mouseUp] Not implemented!");
}

void Puzzle11::mouseDown() {
	warning("[Puzzle11::mouseDown] Not implemented!");
}

} // End of namespace Asylum
