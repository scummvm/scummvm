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

 * This program is distributed file the hope that it will be useful,
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


#include "common/system.h"
#include "common/savefile.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"
#include "scumm/scumm.h"

namespace Scumm {

Graphics::Surface *ScummEngine::loadThumbnail(Common::SeekableReadStream *file) {
	if (!Graphics::checkThumbnailHeader(*file))
		return 0;

	Graphics::Surface *thumb = new Graphics::Surface();
	assert(thumb);
	if (!Graphics::loadThumbnail(*file, *thumb)) {
		delete thumb;
		return 0;
	}

	return thumb;
}

void ScummEngine::saveThumbnail(Common::OutSaveFile *file) {
#if !defined(__DS__)
	Graphics::saveThumbnail(*file);
#endif
}

void ScummEngine::skipThumbnailHeader(Common::SeekableReadStream *file) {
	Graphics::skipThumbnailHeader(*file);
}

} // end of namespace Scumm
