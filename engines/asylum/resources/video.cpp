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
 */

#include "asylum/resources/video.h"

namespace Asylum {

Video::Video(AsylumEngine *vm) : _vm(vm) {
	Common::Event stopEvent;
	_stopEvents.clear();
	stopEvent.type = Common::EVENT_KEYDOWN;
	stopEvent.kbd = Common::KEYCODE_ESCAPE;
	_stopEvents.push_back(stopEvent);

	_smkDecoder = new Graphics::SmackerDecoder(_vm->_mixer);
	_player = new Graphics::VideoPlayer(_smkDecoder);
}

Video::~Video() {
	delete _player;
	delete _smkDecoder;
}

bool Video::playVideo(const char *filename) {
	bool result = _smkDecoder->loadFile(filename);

	// TODO: hide mouse
	if (result)
		_player->playVideo(_stopEvents);
	_smkDecoder->closeFile();
	// TODO: show mouse

	return result;
}

bool Video::playVideo(int number) {
	char filename[50];
	sprintf(filename, "mov%03d.smk", number);
	return playVideo(filename);
}

} // end of namespace Asylum
