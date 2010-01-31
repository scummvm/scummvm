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

GfxCache::GfxCache(ResourceManager *resMan, GfxScreen *screen, GfxPalette *palette)
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

View *GfxCache::getView(GuiResourceId viewId) {
	if (_cachedViews.size() >= MAX_CACHED_VIEWS)
		purgeCache();

	if (!_cachedViews.contains(viewId))
		_cachedViews[viewId] = new View(_resMan, _screen, _palette, viewId);

	return _cachedViews[viewId];
}

int16 GfxCache::kernelViewGetCelWidth(GuiResourceId viewId, int16 loopNo, int16 celNo) {
	return getView(viewId)->getCelInfo(loopNo, celNo)->width;
}

int16 GfxCache::kernelViewGetCelHeight(GuiResourceId viewId, int16 loopNo, int16 celNo) {
	return getView(viewId)->getCelInfo(loopNo, celNo)->height;
}

int16 GfxCache::kernelViewGetLoopCount(GuiResourceId viewId) {
	return getView(viewId)->getLoopCount();
}

int16 GfxCache::kernelViewGetCelCount(GuiResourceId viewId, int16 loopNo) {
	return getView(viewId)->getLoopInfo(loopNo)->celCount;
}

} // End of namespace Sci
