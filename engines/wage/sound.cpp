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
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "common/stream.h"

#include "wage/wage.h"
#include "wage/sound.h"

namespace Wage {

static const int8 deltas[] = { 0,-49,-36,-25,-16,-9,-4,-1,0,1,4,9,16,25,36,49 };

Sound::Sound(Common::String name, Common::SeekableReadStream *data) : _name(name) {
	int size = data->size() - 20;
	_data = (byte *)calloc(2 * size, 1);

	data->skip(20); // Skip header

	byte value = 0x80;
	for (int i = 0; i < size; i++) {
		byte d = data->readByte();
		value += deltas[d & 0xf];
		_data[i * 2] = value;
		value += deltas[(d >> 4) & 0xf];
		_data[i * 2 + 1] = value;
	}
}

Sound::~Sound() {
	free(_data);
}

void WageEngine::playSound(Common::String soundName) {
	warning("STUB: WageEngine::playSound(%s)", soundName.c_str());
}

void WageEngine::updateSoundTimerForScene(Scene *scene, bool firstTime) {
	//warning("STUB: WageEngine::updateSoundTimerForScene()");
}


} // End of namespace Wage
