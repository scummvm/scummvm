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

#include "titanic/movie.h"
#include "titanic/titanic.h"

namespace Titanic {

CMovie::CMovie() : ListItem(), _state(0), _field10(0) {
}

bool CMovie::isActive() const {
	return g_vm->_movieList.contains(this);
}

bool CMovie::get10() {
	if (_field10) {
		_field10 = 0;
		return true;
	} else {
		return false;
	}
}

/*------------------------------------------------------------------------*/

OSMovie::OSMovie(const CResourceKey &name, CVideoSurface *surface) : _videoSurface(surface) {
//	_aviDecoder.loadFile(name.getString());
}

void OSMovie::proc8(int v1, CVideoSurface *surface) {
	warning("TODO: OSMovie::proc8");
}

void OSMovie::proc9() {
	warning("TODO: OSMovie::proc9");
}

void OSMovie::proc10() {
	warning("TODO: OSMovie::proc10");
}

void OSMovie::proc11() {
	warning("TODO: OSMovie::proc11");
}

void OSMovie::proc12() {
	warning("TODO: OSMovie::proc12");
}

void OSMovie::stop() {
	warning("TODO: OSMovie::proc13");
}

void OSMovie::proc14() {
	warning("TODO: OSMovie::proc14");
}

void OSMovie::setFrame(uint frameNumber) {
	warning("TODO: OSMovie::setFrame");
	/*
	_aviDecoder.seekToFrame(frameNumber);
	const Graphics::Surface *s = _aviDecoder.decodeNextFrame();

	_videoSurface->blitFrom(Common::Point(0, 0), s);
	*/
}

void OSMovie::proc16() {
	warning("TODO: OSMovie::proc16");
}

void OSMovie::proc17() {
	warning("TODO: OSMovie::proc17");
}

void OSMovie::proc18() {
	warning("TODO: OSMovie::proc18");
}

int OSMovie::proc19() {
	warning("TODO: OSMovie::proc19");
	return 0;
}

void OSMovie::proc20() {
	warning("TODO: OSMovie::proc20");
}

void *OSMovie::proc21() {
	warning("TODO: OSMovie::proc21");
	return nullptr;
}

} // End of namespace Titanic
