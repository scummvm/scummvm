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

#include "chewy/music/tmf_stream.h"

#include "chewy/music/module_tmf.h"

Chewy::TMFStream::TMFStream(Common::SeekableReadStream *stream, int offs) : ProtrackerStream(44100, true) {
	_module = new Module_TMF();
	bool result = _module->load(*stream, offs);
	assert(result);

	// Channel panning used by TMF is L-R-L-R instead of MOD's L-R-R-L.
	setChannelPanning(0, PANNING_LEFT);
	setChannelPanning(1, PANNING_RIGHT);
	setChannelPanning(2, PANNING_LEFT);
	setChannelPanning(3, PANNING_RIGHT);

	startPaula();
}
