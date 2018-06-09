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

#include "common/debug.h"

#include "pink/archive.h"
#include "pink/cel_decoder.h"
#include "pink/pink.h"
#include "pink/objects/actions/action_cel.h"
#include "pink/objects/actors/actor.h"
#include "pink/objects/pages/game_page.h"

namespace Pink {

ActionCEL::ActionCEL()
	: _decoder(nullptr) {}

ActionCEL::~ActionCEL() {
	end();
}

void ActionCEL::deserialize(Archive &archive) {
	Action::deserialize(archive);
	_fileName = archive.readString();
	_z = archive.readDWORD();
}

bool ActionCEL::initPalette(Director *director) {
	if (!_decoder)
		_decoder = _actor->getPage()->loadCel(_fileName);
	if (_decoder->getCurFrame() == -1) {
		_decoder->decodeNextFrame();
		_decoder->rewind();
	}
	debug("%u", _decoder->isPaused());
	director->setPallette(_decoder->getPalette());
	return true;
}

void ActionCEL::start() {
	if (!_decoder)
		_decoder = _actor->getPage()->loadCel(_fileName);
	this->onStart();
	_actor->getPage()->getGame()->getDirector()->addSprite(this);
}

void ActionCEL::end() {
	_actor->getPage()->getGame()->getDirector()->removeSprite(this);
	delete _decoder;
	_decoder = nullptr;
}

void ActionCEL::update() {
	if (_decoder->endOfVideo()) {
		_decoder->stop();
		_actor->endAction();
	}
}

void ActionCEL::pause(bool paused) {
	_decoder->pauseVideo(paused);
}

uint32 ActionCEL::getZ() {
	return _z;
}

CelDecoder *ActionCEL::getDecoder() {
	return _decoder;
}

Coordinates ActionCEL::getCoordinates() {
	if (!_decoder)
		_decoder = _actor->getPage()->loadCel(_fileName);

	Coordinates coords;
	coords.x = _decoder->getX() + _decoder->getWidth() / 2;
	coords.y = _decoder->getY() + _decoder->getHeight() / 2;
	coords.z = getZ();

	return coords;
}

} // End of namespace Pink
