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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/matchpuzzle.h"

namespace Nancy {
namespace Action {

void MatchPuzzle::init() {
	// TODO
	//_screenPosition = _displayBounds;

	//_drawSurface.create(_screenPosition.width(), _screenPosition.height(), g_nancy->_graphics->getInputPixelFormat());
	//_drawSurface.clear(g_nancy->_graphics->getTransColor());

	setTransparent(true);

	g_nancy->_resource->loadImage(_overlayName, _image);
	RenderActionRecord::init();
}

void MatchPuzzle::execute() {
	if (_state == kBegin) {
		init();
		registerGraphics();
		_state = kRun;
	}

	// TODO
	// Stub - return to the main menu
	warning("STUB - Nancy 8 flag game");
	_exitSceneChange.execute();
}

void MatchPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _overlayName);
	readFilename(stream, _flagPointBackgroundName);

	stream.skip(2);   // TODO (value: 5)
	stream.skip(2);   // TODO (value: 7)
	stream.skip(2);   // 26 flags

	readRect(stream,_shuffleButtonRect);
	readRectArray(stream, _flagRects, 26);

	stream.skip(103); // TODO (mostly zeroes)

	readFilenameArray(stream, _flagNames, 26);

	stream.skip(132); // TODO (zeroes)
	stream.skip(173); // TODO

	_slotWinSound.readNormal(stream);
	_shuffleSound.readNormal(stream);
	_cardPlaceSound.readNormal(stream);

	_solveSceneChange.readData(stream);
	stream.skip(2);
	_matchSuccessSound.readNormal(stream);
	_exitSceneChange.readData(stream);

	stream.skip(16); // TODO
}

void MatchPuzzle::handleInput(NancyInput &input) {
	// TODO
}

} // End of namespace Action
} // End of namespace Nancy
