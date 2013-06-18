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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/*
  €ﬂ‹ €ﬂ‹ ‹ﬂﬂ‹  ﬂ€ﬂ €ﬂﬂ  ‹ﬂ ﬂ€ﬂ      ‹ﬂ€ﬂ‹  ﬂ€ﬂ €‹  € ‹€ﬂﬂ  ﬂ€ﬂ €ﬂ‹ €ﬂﬂ €
  €ﬂ  €€  €  € ‹ €  €ﬂﬂ ﬂ‹   €      €  €  €  €  € €‹€  ﬂﬂﬂ‹  €  €€  €ﬂﬂ €
  ﬂ   ﬂ ﬂ  ﬂﬂ   ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ      ﬂ  ﬂ  ﬂ ﬂﬂﬂ ﬂ  ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ ﬂ ﬂﬂﬂ ﬂﬂﬂ

                 STICKS           The joystick handler. */

#define __sticks_implementation__


#include "sticks.h"

namespace Avalanche {

enum lmc {l, m, r, last_lmc}; /* Left, middle & right */

file<joysetup> jf;

byte joyway();


static lmc getlmc(word n, word max, word min) {
	lmc getlmc_result;
	if (n < min)  getlmc_result = l;
	else if (n > max)  getlmc_result = r;
	else
		getlmc_result = m;
	return getlmc_result;
}

byte joyway() {
	word x, y;
	lmc xx, yy;
	byte joyway_result;
	if (~ dna.user_moves_avvy)  return joyway_result;
	if (use_joy_a)  readjoya(x, y);
	else readjoyb(x, y);

	{
		xx = getlmc(x, cxmax, cxmin);
		yy = getlmc(y, cymax, cymin);
	}

	switch (xx) {
	case l:
		switch (yy) {
		case l:
			joyway_result = ul;
			break;
		case m:
			joyway_result = left;
			break;
		case r:
			joyway_result = dl;
			break;
		}
		break;
	case m:
		switch (yy) {
		case l:
			joyway_result = up;
			break;
		case m:
			joyway_result = stopped;
			break;
		case r:
			joyway_result = down;
			break;
		}
		break;
	case r:
		switch (yy) {
		case l:
			joyway_result = ur;
			break;
		case m:
			joyway_result = right;
			break;
		case r:
			joyway_result = dr;
			break;
		}
		break;
	}
	return joyway_result;
}

void joykeys()
/* The 2 joystick keys may be reprogrammed. This parses them. */
{
	byte v;
	if (use_joy_a) {
		v = (byte)(buttona1);
		v += (byte)(buttona2) * 2;
	} else {
		v = (byte)(buttonb1);
		v += (byte)(buttonb2) * 2;
	}

	switch (v) {
	case 0:
		return;
		break; /* No buttons pressed. */
	case 1:
		opendoor;
		break; /* Button 1 pressed: open door. */
		/*  2: blip; { Button 2 pressed: nothing (joylock?). }*/
	case 2:
	case 3:
		lookaround;
		break; /* Both buttons pressed: look around. */
	}
}

/* No init code. */

} // End of namespace Avalanche.