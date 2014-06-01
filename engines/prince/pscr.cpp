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
 */

#include "common/archive.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/stream.h"

#include "graphics/surface.h"

#include "prince/pscr.h"

namespace Prince {

PScr::PScr() :_file(0), _x(0), _y(0), _step(0), _addr(0), _len(0), _surface(NULL)
{
}

PScr::~PScr() {
	if (_surface) {
		_surface->free();
		delete _surface;
		_surface = NULL;
	}
}

void PScr::loadSurface(Common::SeekableReadStream &stream) {
	//stream.skip(4);
	int x = stream.readUint16LE();
	int y = stream.readUint16LE();
	int width = stream.readUint16LE();
	int height = stream.readUint16LE();
	debug("x: %d, y: %d", x, y);
	debug("w: %d, h: %d", width, height);
	_surface = new Graphics::Surface();
	_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());

	for (int h = 0; h < _surface->h; h++) {
		stream.read(_surface->getBasePtr(0, h), _surface->w);
	}
}

bool PScr::loadFromStream(Common::SeekableReadStream &stream) {
	int32 pos = stream.pos();
	uint16 file = stream.readUint16LE();
	if (file == 0xFFFF)
		return false;
	_file = file;
	_x = stream.readUint16LE();
	_y = stream.readUint16LE();
	_step = stream.readUint16LE();
	_addr = stream.readUint32LE();

	const Common::String pscrStreamName = Common::String::format("PS%02d", _file);
	Common::SeekableReadStream *pscrStream = SearchMan.createReadStreamForMember(pscrStreamName);
	if (!pscrStream) {
		error("Can't load %s", pscrStreamName.c_str());
		return false;
	}

	loadSurface(*pscrStream);
	delete pscrStream;
	
	stream.seek(pos + 12); // size of PScrList struct

	debug("Parallex nr %d, x %d, y %d, step %d", _file, _x, _y, _step);

	return true;
}

}