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

#include "common/util.h"
#include "common/stack.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/font.h"
#include "sci/graphics/view.h"

namespace Sci {

GfxCache::GfxCache(ResourceManager *resMan, Screen *screen, SciPalette *palette)
	: _resMan(resMan), _screen(screen), _palette(palette) {
}

GfxCache::~GfxCache() {
	purgeCache();
}

void GfxCache::purgeCache() {
	for (ViewCache::iterator iter = _cachedViews.begin(); iter != _cachedViews.end(); ++iter) {
		delete iter->_value;
		iter->_value = 0;
	}

	_cachedViews.clear();
}

View *GfxCache::getView(GuiResourceId viewNum) {
	if (_cachedViews.size() >= MAX_CACHED_VIEWS)
		purgeCache();

	if (!_cachedViews.contains(viewNum))
		_cachedViews[viewNum] = new View(_resMan, _screen, _palette, viewNum);

	return _cachedViews[viewNum];
}

} // End of namespace Sci
