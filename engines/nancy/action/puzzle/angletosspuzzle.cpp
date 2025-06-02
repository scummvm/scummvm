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
#include "engines/nancy/action/puzzle/angletosspuzzle.h"

namespace Nancy {
namespace Action {

void AngleTossPuzzle::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	// TODO
}

void AngleTossPuzzle::execute() {
	if (_state == kBegin) {
		init();
		registerGraphics();
		_state = kRun;
	}

	// TODO
	// Stub - return to the winning screen
	warning("STUB - Nancy 8 Squid Toss game");
	SceneChangeDescription scene;
	scene.sceneID = 4465;
	NancySceneState.resetStateToInit();
	NancySceneState.changeScene(scene);
}

void AngleTossPuzzle::readData(Common::SeekableReadStream &stream) {
	Common::Path tmp;
	readFilename(stream, tmp);
	stream.skip(12);	// TODO

	for (uint i = 0; i < 22; ++i) {
		Common::Rect r;
		readRect(stream, r);

		/*
		Common::String desc = Common::String::format("AngleTossPuzzle rect %d", i);
		debug("%s %d, %d, %d, %d", desc.c_str(), r.left, r.top, r.right, r.bottom);

		Graphics::Surface *s = g_system->lockScreen();
		s->fillRect(r, 255);
		g_system->unlockScreen();
		g_system->updateScreen();
		g_system->delayMillis(1000);
		*/
	}

	_powerSound.readNormal(stream);
	_squeakSound.readNormal(stream);
	_chainSound.readNormal(stream);

	_throwSquidScene.readData(stream);
	stream.skip(7); // TODO
	_exitScene.readData(stream);

	stream.skip(16); // TODO
}

void AngleTossPuzzle::handleInput(NancyInput &input) {
	// TODO
}

} // End of namespace Action
} // End of namespace Nancy
