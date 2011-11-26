/* Copyright (C) 2003, 2004, 2005, 2006, 2008, 2009 Dean Beeler, Jerome Fisher
 * Copyright (C) 2011 Dean Beeler, Jerome Fisher, Sergey V. Mikayev
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mt32emu.h"

using namespace MT32Emu;

bool File::readBit16u(Bit16u *in) {
	Bit8u b[2];
	if (read(&b[0], 2) != 2) {
		return false;
	}
	*in = ((b[0] << 8) | b[1]);
	return true;
}

bool File::readBit32u(Bit32u *in) {
	Bit8u b[4];
	if (read(&b[0], 4) != 4) {
		return false;
	}
	*in = ((b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3]);
	return true;
}
