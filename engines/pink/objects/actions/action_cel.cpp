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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/debug.h"
#include "common/substream.h"

#include "pink/archive.h"
#include "pink/cel_decoder.h"
#include "pink/screen.h"
#include "pink/pink.h"
#include "pink/objects/actions/action_cel.h"
#include "pink/objects/actors/actor.h"
#include "pink/objects/pages/game_page.h"

namespace Pink {

ActionCEL::~ActionCEL() {
	end();
}

void ActionCEL::deserialize(Archive &archive) {
	Action::deserialize(archive);
	_fileName = archive.readString();
	_z = archive.readDWORD();
}

bool ActionCEL::initPalette(Screen *screen) {
	loadDecoder();
	if (_decoder.getCurFrame() == -1) {
		_decoder.decodeNextFrame();
		_decoder.rewind();
	}
	screen->setPalette(_decoder.getPalette());
	return true;
}

void ActionCEL::start() {
	loadDecoder();
	_decoder.start();
	this->onStart();
	_actor->getPage()->getGame()->getScreen()->addSprite(this);
}

void ActionCEL::end() {
	_actor->getPage()->getGame()->getScreen()->removeSprite(this);
	_decoder.close();
}

void ActionCEL::pause(bool paused) {
	_decoder.pauseVideo(paused);
}

Coordinates ActionCEL::getCoordinates() {
	loadDecoder();

	Coordinates coords;
	coords.point = _decoder.getCenter();
	coords.z = getZ();

	return coords;
}

void ActionCEL::loadDecoder() {
	if (!_decoder.isVideoLoaded()) {
		_decoder.loadStream(_actor->getPage()->getResourceStream(_fileName));
		Common::Point point = _decoder.getCenter();
		_bounds = Common::Rect::center(point.x, point.y, _decoder.getWidth(), _decoder.getHeight());
	}
}

void ActionCEL::setFrame(uint frame) {
	_decoder.rewind();

	for (uint i = 0; i < frame; ++i) {
		_decoder.skipFrame();
	}

	_decoder.clearDirtyRects();
	_actor->getPage()->getGame()->getScreen()->addDirtyRect(_bounds);
}

void ActionCEL::decodeNext() {
	_decoder.decodeNextFrame();
	_actor->getPage()->getGame()->getScreen()->addDirtyRects(this);
}

void ActionCEL::setCenter(Common::Point center) {
	_actor->getPage()->getGame()->getScreen()->addDirtyRect(_bounds);
	_bounds = Common::Rect::center(center.x, center.y, _decoder.getWidth(), _decoder.getHeight());
	_actor->getPage()->getGame()->getScreen()->addDirtyRect(_bounds);
}

} // End of namespace Pink
