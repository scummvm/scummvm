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

#include "common/system.h"
#include "video/qt_decoder.h"

#include "director/movie.h"
#include "director/score.h"
#include "director/util.h"

namespace Director {

Movie::Movie(Common::String fileName, DirectorEngine *vm) {
	_vm = vm;
	_currentVideo = new Video::QuickTimeDecoder();
	if (!_currentVideo->loadFile(fileName)) {
		warning("Can not open file %s", fileName.c_str());
		return;
	}
}

void Movie::play(Common::Point dest) {

	_currentVideo->start();

	uint16 width = _currentVideo->getWidth();
	uint16 height = _currentVideo->getHeight();

	while (!_currentVideo->endOfVideo()) {
		if (_currentVideo->needsUpdate()) {
			const Graphics::Surface *frame = _currentVideo->decodeNextFrame();
			g_system->copyRectToScreen(frame->getPixels(), frame->pitch, dest.x, dest.y, width, height);
			g_system->updateScreen();
		}
		g_system->delayMillis(10);
		processQuitEvent();
	}
}

void Movie::stop() {
	_currentVideo->stop();
}

Movie::~Movie() {
	delete _currentVideo;
}

} // End of namespace Director
