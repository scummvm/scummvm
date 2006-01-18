/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
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
 * $Header$
 *
 */
#ifndef GOB_PALANIM_H
#define GOB_PALANIM_H

namespace Gob {

extern int16 pal_fadeValue;

char pal_fadeColor(char from, char to);
char pal_fadeStep(int16 oper);	// oper == 0 - fade all colors, 1, 2, 3 - red,green, blue
void pal_fade(PalDesc * palDesc, int16 fade, int16 all);

}				// End of namespace Gob

#endif	/* __PALANIM_H */
