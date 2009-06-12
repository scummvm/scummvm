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

#ifndef ASYLUM_RESOURCEMANAGER_H_
#define ASYLUM_RESOURCEMANAGER_H_

#include "common/str.h"
#include "common/array.h"

#include "asylum/bundle.h"
#include "asylum/graphicbundle.h"

namespace Asylum {

class ResourceManager {
public:
	ResourceManager() {};
	~ResourceManager() {};

	GraphicBundle* getGraphic(uint8 fileNum, uint32 offset);
	void getPalette(uint8 fileNum, uint32 offset, byte *palette);

private:
	Common::Array<Bundle> _bundleCache;

	Bundle* getBundle(uint8 fileNum);

}; // end of class ResourceManager


} // end of namespace Asylum

#endif
