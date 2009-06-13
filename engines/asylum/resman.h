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

#ifndef ASYLUM_RESOURCEMANAGER_H_
#define ASYLUM_RESOURCEMANAGER_H_

#include "common/str.h"
#include "common/array.h"

#include "asylum/bundle.h"
#include "asylum/graphicbundle.h"
#include "asylum/video.h"
#include "asylum/asylum.h"

namespace Asylum {

// forward declarations
class AsylumEngine;
class Video;

class ResourceManager {
public:
	ResourceManager(AsylumEngine *vm);
	virtual ~ResourceManager();

	GraphicResource* getGraphic(uint8 fileNum, uint32 offset, uint32 index);
	bool loadCursor(uint8 fileNum, uint32 offset, uint32 index);
	bool loadVideo(uint8 fileNum);

private:
	Common::Array<Bundle> _bundleCache;

	Bundle* getBundle(uint8 fileNum);

	AsylumEngine *_vm;
	Video		 *_video;

}; // end of class ResourceManager


} // end of namespace Asylum

#endif
