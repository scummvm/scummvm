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

#include "common/timer.h"
#include "common/util.h"

#include "sci/sci.h"
#include "sci/debug.h"	// for g_debug_sleeptime_factor
#include "sci/event.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/graphics/gui32.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/frameout.h"
#include "sci/graphics/paint32.h"
#include "sci/graphics/picture.h"
#include "sci/graphics/robot.h"
#include "sci/graphics/view.h"

namespace Sci {

SciGui32::SciGui32(SegManager *segMan, SciEvent *event, GfxScreen *screen, GfxPalette *palette, GfxCache *cache, GfxCursor *cursor)
	: _screen(screen), _palette(palette), _cache(cache), _cursor(cursor) {

	_coordAdjuster = new GfxCoordAdjuster32(segMan);
	g_sci->_gfxCoordAdjuster = _coordAdjuster;
	_cursor->init(_coordAdjuster, event);
	_compare = new GfxCompare(segMan, g_sci->getKernel(), _cache, _screen, _coordAdjuster);
	g_sci->_gfxCompare = _compare;
	_paint32 = new GfxPaint32(g_sci->getResMan(), segMan, g_sci->getKernel(), _coordAdjuster, _cache, _screen, _palette);
	g_sci->_gfxPaint = _paint32;
	_frameout = new GfxFrameout(segMan, g_sci->getResMan(), _coordAdjuster, _cache, _screen, _palette, _paint32);
	g_sci->_gfxFrameout = _frameout;
}

SciGui32::~SciGui32() {
	delete _frameout;
	delete _paint32;
	delete _compare;
	delete _coordAdjuster;
}

void SciGui32::init() {
}

void SciGui32::textSize(const char *text, int16 font, int16 maxWidth, int16 *textWidth, int16 *textHeight) {
	*textWidth = 0;
	*textHeight = 0;
}

void SciGui32::drawRobot(GuiResourceId robotId) {
	Robot *test = new Robot(g_sci->getResMan(), _screen, robotId);
	test->draw();
	delete test;
}

} // End of namespace Sci
