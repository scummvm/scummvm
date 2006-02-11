/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#ifdef LURE_DEBUG
#ifndef __lure_input_h__
#define __lure_input_h__

#include "common/stdafx.h"
#include "common/str.h"
#include "lure/surface.h"

namespace Lure {

bool get_string(char *buffer, uint32 maxSize, bool isNumeric, uint16 x, uint16 y);

bool input_integer(Common::String desc, uint32 &value);

bool input_string(Common::String desc, char *buffer, uint32 maxSize);

} // End of namespace Lure

#endif
#endif
