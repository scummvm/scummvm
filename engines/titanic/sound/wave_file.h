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

#ifndef TITANIC_WAVE_FILE_H
#define TITANIC_WAVE_FILE_H

#include "titanic/support/simple_file.h"

namespace Titanic {

class CSoundManager;

class WaveFile {
public:
	int _field0;
	int _field4;
	int _field8;
	uint _handle;
	CSoundManager *_owner;
	int _field14;
	int _field18;
	int _field1C;
	int _field20;
	int _field24;
	int _field28;
	int _field2C;
public:
	WaveFile() : _field0(2), _field4(0), _field8(0), _handle(0),
		_owner(nullptr), _field14(1), _field18(0), _field1C(0),
		_field20(0), _field24(0), _field28(0), _field2C(-1) {}
};

} // End of namespace Titanic

#endif /* TITANIC_WAVE_FILE_H */
