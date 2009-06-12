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
 */

#ifndef ASYLUM_RESOURCE_H_
#define ASYLUM_RESOURCE_H_

#include "common/str.h"

namespace Asylum {

class Resource {
public:
	Resource() : size(0), offset(0), initialized(false) {}
	virtual ~Resource(){}

	void save(Common::String filename) {
		FILE *fd = fopen(filename.c_str(), "wb+");
		fwrite(data, size, 1, fd);
		fclose(fd);
	}

	Common::String type;
	uint32         size;
	uint32         offset;
	uint8*         data;
	bool		   initialized;

}; // end of class Resource

class GraphicResource: public Resource {
public:
	GraphicResource() {}
	~GraphicResource() {}

	uint32 size;
	uint32 flag;
	uint16 x;
	uint16 y;
	uint16 width;
	uint16 height;

};	// end of class GraphicResource


} // end of namespace Asylum

#endif
