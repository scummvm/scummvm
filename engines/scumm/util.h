/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#ifndef SCUMM_UTIL_H
#define SCUMM_UTIL_H

#include "common/stdafx.h"
#include "common/scummsys.h"

namespace Scumm {

#define revBitMask(x)	(0x80 >> (x))

/* Direction conversion functions (between old dir and new dir format) */
int newDirToOldDir(int dir);
int oldDirToNewDir(int dir);

int normalizeAngle(int angle);
int fromSimpleDir(int dirtype, int dir);
int toSimpleDir(int dirtype, int dir);

void checkRange(int max, int min, int no, const char *str);

const char *tag2str(uint32 tag);

} // End of namespace Scumm

#endif
