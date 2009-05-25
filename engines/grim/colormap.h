/* Residual - Virtual machine to run LucasArts' 3D adventure games
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef COLORMAP_H
#define COLORMAP_H

#include "common/endian.h"

#include "engines/grim/resource.h"

namespace Grim {

class CMap : public Resource {
public:
	// Load a colormap from the given data.
	CMap(const char *filename, const char *data, int len) : Resource(filename) {
		if (len < 4 || READ_BE_UINT32(data) != MKID_BE('CMP '))
			error("Invalid magic loading colormap");
		memcpy(_colors, data + 64, sizeof(_colors));
	}

	// The color data, in RGB format
	char _colors[256 * 3];
};

} // end of namespace Grim

#endif
