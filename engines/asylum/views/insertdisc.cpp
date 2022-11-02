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

#include "asylum/views/insertdisc.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"
#include "asylum/system/text.h"

#include "asylum/asylum.h"

namespace Asylum {

InsertDisc::InsertDisc(AsylumEngine *engine) : _vm(engine) {
	_handler = nullptr;
	_cdNumber = -1;
	_frameIndex = _frameCount = 0;
}

void InsertDisc::init() {
	getScreen()->setPalette(MAKE_RESOURCE(kResourcePackSound, 10 + _cdNumber));
	getScreen()->setGammaLevel(MAKE_RESOURCE(kResourcePackSound, 10 + _cdNumber));
	getText()->loadFont(MAKE_RESOURCE(kResourcePackSound, 19));

	_frameIndex = 0;
	_frameCount = GraphicResource::getFrameCount(_vm, MAKE_RESOURCE(kResourcePackSound, 13 + _cdNumber)); 
}

void InsertDisc::update() {
	getCursor()->hide();
	getScreen()->draw(MAKE_RESOURCE(kResourcePackSound,  7 + _cdNumber),           0, Common::Point(  0,   0));
	getScreen()->draw(MAKE_RESOURCE(kResourcePackSound, 13 + _cdNumber), _frameIndex, Common::Point(295, 206));
	getText()->drawCentered(Common::Point(0, 40), 640, MAKE_RESOURCE(kResourcePackText, 1416 + _cdNumber));
	getScreen()->copyBackBufferToScreen();

	_frameIndex = (_frameIndex + 1) % _frameCount;
}

bool InsertDisc::handleEvent(const AsylumEvent &evt) {
	switch ((int32)evt.type) {
	default:
		break;

	case EVENT_ASYLUM_INIT:
		init();
		return true;

	case EVENT_ASYLUM_UPDATE:
		update();
		return true;

	case Common::EVENT_KEYDOWN:
		_vm->switchEventHandler(_handler);
		return true;
	}

	return false;
}

} // End of namespace Asylum
