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

#ifndef ASYLUM_GRAPHICBUNDLE_H_
#define ASYLUM_GRAPHICBUNDLE_H_

#include "asylum/bundle.h"

namespace Asylum {

class GraphicBundle: public Bundle {
public:
	GraphicBundle() {}
	GraphicBundle(uint8 fileNum, uint32 index, uint32 length);
	~GraphicBundle() {}

	GraphicResource* getEntry(uint32 index);

protected:
	void update();
	Common::Array<GraphicResource*> entries;

private:
	uint32 _tagValue;
	uint32 _flag;
	uint32 _contentOffset;
	uint32 _unknown1;
	uint32 _unknown2;
	uint32 _unknown3;
	uint16 _maxWidth;
};

} // end of namespace Asylum

#endif
