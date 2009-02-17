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
 * $URL$
 * $Id$
 *
 */

/* SCI constants and definitions */

#ifndef _SCI_GRAPHICS_H_
#define _SCI_GRAPHICS_H_

#define MAX_TEXT_WIDTH_MAGIC_VALUE 192
/* This is the real width of a text with a specified width of 0 */

#define SELECTOR_STATE_SELECTABLE 1
#define SELECTOR_STATE_FRAMED 2
#define SELECTOR_STATE_DISABLED 4
#define SELECTOR_STATE_SELECTED 8
/* Internal states */
#define SELECTOR_STATE_DITHER_FRAMED 0x1000


#endif /* !_SCI_GRAPHICS_H_ */
