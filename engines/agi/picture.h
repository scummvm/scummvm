/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
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

#ifndef __AGI_PICTURE_H
#define __AGI_PICTURE_H

#include "agi/agi.h"

namespace Agi {

/**
 * AGI picture resource.
 */
struct agi_picture {
	uint32 flen;			/**< size of raw data */
	uint8 *rdata;			/**< raw vector image data */
};

int decode_picture(int, int);
int unload_picture(int);
void show_pic(void);
uint8 *convert_v3_pic(uint8 *data, uint32 len);

}                             // End of namespace Agi

#endif				/* __AGI_PICTURE_H */
