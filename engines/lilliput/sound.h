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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef LILLIPUT_SOUND_H
#define LILLIPUT_SOUND_H

namespace Lilliput {

class LilliputEngine;

class LilliputSound {
public:
	LilliputSound(LilliputEngine *vm);
	~LilliputSound();

	void contentFct1();
	void contentFct2();
	void contentFct3();
	void contentFct4();
	void contentFct5();
	void contentFct6();

private:
	LilliputEngine *_vm;
};

} // End of namespace Lilliput

#endif

