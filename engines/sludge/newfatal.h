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
#ifndef SLUDGE_NEWFATAL_H
#define SLUDGE_NEWFATAL_H

#include "common/str.h"

#include "sludge/errors.h"

namespace Sludge {

bool hasFatal();

int fatal(const Common::String &str);
int fatal(const Common::String &str1, const Common::String &str2);
int checkNew(const void *mem);
void setFatalInfo(const Common::String &userFunc, const Common::String &BIF);
void setResourceForFatal(int n);
const Common::String resourceNameFromNum(int i);

} // End of namespace Sludge

#endif
