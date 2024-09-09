/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/remap.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/view.h"

#include "sci/graphics/scifx.h"

namespace Sci {

GfxView::GfxView(ResourceManager *resMan, GfxScreen *screen, GfxPalette *palette, GuiResourceId resourceId)
	: _resMan(resMan), _screen(screen), _palette(palette), _resourceId(resourceId) {
	assert(resourceId != -1);
	_coordAdjuster = g_sci->_gfxCoordAdjuster;
	initData();
}

GfxView::~GfxView() {
	_loop.clear();
	_resMan->unlockResource(_resource);
}

static const byte EGAmappingStraight[SCI_VIEW_EGAMAPPING_SIZE] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

static const byte ViewInject_LauraBow2_Both[] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x37,0x37,0x37,0x37,0x37,0x00,0x00,0x00,0x37,0x37,0x37,0x37,0x00,0x00,0x37,0x37,0x37,0x37,0x37,0x37,0x00,0x37,0x37,0x00,0x00,0x37,0x37,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x37,0x08,0x08,0x08,0x08,0x37,0x00,0x37,0x37,0x08,0x08,0x08,0x32,0x00,0x37,0x08,0x08,0x08,0x08,0x08,0x32,0x37,0x08,0x32,0x00,0x37,0x08,0x32,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x37,0x08,0x32,0x32,0x00,0x08,0x32,0x37,0x08,0x32,0x32,0x00,0x08,0x32,0x00,0x00,0x32,0x08,0x32,0x32,0x32,0x37,0x08,0x32,0x00,0x37,0x08,0x32,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x37,0x08,0x32,0x00,0x37,0x08,0x32,0x37,0x08,0x32,0x00,0x37,0x08,0x32,0x00,0x00,0x37,0x08,0x32,0x00,0x00,0x37,0x08,0x00,0x37,0x37,0x08,0x32,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x37,0x08,0x32,0x08,0x08,0x32,0x00,0x37,0x08,0x32,0x00,0x37,0x08,0x32,0x00,0x00,0x37,0x08,0x32,0x00,0x00,0x37,0x08,0x08,0x08,0x08,0x08,0x32,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x37,0x08,0x32,0x32,0x00,0x08,0x32,0x37,0x08,0x32,0x00,0x37,0x08,0x32,0x00,0x00,0x37,0x08,0x32,0x00,0x00,0x37,0x08,0x32,0x32,0x37,0x08,0x32,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x37,0x08,0x00,0x37,0x37,0x08,0x32,0x37,0x08,0x00,0x37,0x37,0x08,0x32,0x00,0x00,0x37,0x08,0x32,0x00,0x00,0x37,0x08,0x32,0x00,0x37,0x08,0x32,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x37,0x08,0x08,0x08,0x08,0x32,0x00,0x00,0x37,0x08,0x08,0x08,0x32,0x00,0x00,0x00,0x37,0x08,0x32,0x00,0x00,0x37,0x08,0x32,0x00,0x37,0x08,0x32,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x32,0x32,0x32,0x32,0x00,0x00,0x00,0x00,0x32,0x32,0x32,0x00,0x00,0x00,0x00,0x00,0x32,0x32,0x00,0x00,0x00,0x32,0x32,0x00,0x00,0x32,0x32,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const byte ViewInject_KingsQuest6_Both1[] = {
	0x17,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x13,
	0x17,0x17,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x13,0x11,
	0x16,0x17,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x11,0x11,
	0x16,0x16,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x17,0x16,0x16,0x16,0x16,0x13,0x13,0x13,0x17,0x16,0x16,0x16,0x13,0x13,0x17,0x16,0x16,0x16,0x16,0x16,0x13,0x17,0x16,0x13,0x13,0x17,0x16,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x11,0x11,
	0x16,0x16,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x16,0x10,0x10,0x10,0x10,0x16,0x13,0x16,0x16,0x10,0x10,0x10,0x11,0x13,0x16,0x10,0x10,0x10,0x10,0x10,0x11,0x16,0x10,0x11,0x13,0x16,0x10,0x11,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x11,0x11,
	0x16,0x16,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x16,0x10,0x11,0x11,0x13,0x10,0x11,0x16,0x10,0x11,0x11,0x13,0x10,0x11,0x13,0x13,0x11,0x10,0x11,0x11,0x11,0x16,0x10,0x11,0x13,0x16,0x10,0x11,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x11,0x11,
	0x16,0x16,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x16,0x10,0x11,0x13,0x16,0x10,0x11,0x16,0x10,0x11,0x13,0x16,0x10,0x11,0x13,0x13,0x16,0x10,0x11,0x13,0x13,0x16,0x10,0x13,0x16,0x16,0x10,0x11,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x11,0x11,
	0x16,0x16,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x16,0x10,0x11,0x10,0x10,0x11,0x13,0x16,0x10,0x11,0x13,0x16,0x10,0x11,0x13,0x13,0x16,0x10,0x11,0x13,0x13,0x16,0x10,0x10,0x10,0x10,0x10,0x11,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x11,0x11,
	0x16,0x16,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x16,0x10,0x11,0x11,0x13,0x10,0x11,0x16,0x10,0x11,0x13,0x16,0x10,0x11,0x13,0x13,0x16,0x10,0x11,0x13,0x13,0x16,0x10,0x11,0x11,0x13,0x10,0x11,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x11,0x11,
	0x16,0x16,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x16,0x10,0x13,0x16,0x16,0x10,0x11,0x16,0x10,0x13,0x16,0x16,0x10,0x11,0x13,0x13,0x16,0x10,0x11,0x13,0x13,0x16,0x10,0x11,0x13,0x16,0x10,0x11,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x11,0x11,
	0x16,0x16,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x16,0x10,0x10,0x10,0x10,0x11,0x13,0x13,0x16,0x10,0x10,0x10,0x11,0x13,0x13,0x13,0x16,0x10,0x11,0x13,0x13,0x16,0x10,0x11,0x13,0x16,0x10,0x11,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x11,0x11,
	0x16,0x16,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x11,0x11,0x11,0x11,0x13,0x13,0x13,0x13,0x11,0x11,0x11,0x13,0x13,0x13,0x13,0x13,0x11,0x11,0x13,0x13,0x13,0x11,0x11,0x13,0x13,0x11,0x11,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x11,0x11,
	0x16,0x16,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x11,0x11,
	0x16,0x13,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
	0x13,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x11
};

static const byte ViewInject_KingsQuest6_Both2[] = {
	0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
	0x10,0x16,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x11,0x10,
	0x10,0x13,0x16,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x13,0x11,0x10,0x10,
	0x10,0x13,0x13,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x16,0x13,0x13,0x13,0x13,0x11,0x11,0x11,0x16,0x13,0x13,0x13,0x11,0x11,0x16,0x13,0x13,0x13,0x13,0x13,0x11,0x16,0x13,0x11,0x11,0x16,0x13,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x10,0x10,
	0x10,0x13,0x13,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x13,0x16,0x16,0x16,0x16,0x13,0x11,0x13,0x13,0x16,0x16,0x16,0x13,0x11,0x13,0x16,0x16,0x16,0x16,0x16,0x10,0x13,0x16,0x10,0x11,0x13,0x16,0x10,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x10,0x10,
	0x10,0x13,0x13,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x13,0x16,0x10,0x10,0x11,0x16,0x10,0x13,0x16,0x11,0x10,0x13,0x16,0x10,0x11,0x11,0x10,0x16,0x10,0x10,0x10,0x13,0x16,0x10,0x11,0x13,0x16,0x10,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x10,0x10,
	0x10,0x13,0x13,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x13,0x16,0x10,0x11,0x13,0x16,0x10,0x13,0x16,0x10,0x11,0x13,0x16,0x10,0x11,0x11,0x13,0x16,0x10,0x11,0x11,0x13,0x16,0x11,0x13,0x13,0x16,0x10,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x10,0x10,
	0x10,0x13,0x13,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x13,0x16,0x10,0x16,0x16,0x10,0x10,0x13,0x16,0x10,0x11,0x13,0x16,0x10,0x11,0x11,0x13,0x16,0x10,0x11,0x11,0x13,0x16,0x16,0x16,0x16,0x16,0x10,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x10,0x10,
	0x10,0x13,0x13,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x13,0x16,0x10,0x10,0x11,0x16,0x10,0x13,0x16,0x10,0x11,0x13,0x16,0x10,0x11,0x11,0x13,0x16,0x10,0x11,0x11,0x13,0x16,0x10,0x10,0x11,0x16,0x10,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x10,0x10,
	0x10,0x13,0x13,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x13,0x16,0x11,0x13,0x13,0x16,0x10,0x13,0x16,0x11,0x13,0x13,0x16,0x10,0x11,0x11,0x13,0x16,0x10,0x11,0x11,0x13,0x16,0x10,0x11,0x13,0x16,0x10,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x10,0x10,
	0x10,0x13,0x13,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x13,0x16,0x16,0x16,0x16,0x10,0x11,0x11,0x13,0x16,0x16,0x16,0x10,0x11,0x11,0x11,0x13,0x16,0x10,0x11,0x11,0x13,0x16,0x10,0x11,0x13,0x16,0x10,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x10,0x10,
	0x10,0x13,0x13,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x10,0x10,0x10,0x10,0x11,0x11,0x11,0x11,0x10,0x10,0x10,0x11,0x11,0x11,0x11,0x11,0x10,0x10,0x11,0x11,0x11,0x10,0x10,0x11,0x11,0x10,0x10,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x10,0x10,
	0x10,0x13,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x10,0x10,
	0x10,0x11,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
	0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10
};

void GfxView::initData() {
	_resource = _resMan->findResource(ResourceId(kResourceTypeView, _resourceId), true);
	if (!_resource) {
		error("view resource %d not found", _resourceId);
	}

	_loop.resize(0);
	_embeddedPal = false;
	_EGAmapping.clear();
	_isScaleable = true;

	// we adjust inside getCelRect for SCI0EARLY (that version didn't have the +1 when calculating bottom)
	_adjustForSci0Early = getSciVersion() == SCI_VERSION_0_EARLY ? -1 : 0;

	// If we find an SCI1/SCI1.1 view (not amiga), we switch to that type for
	// EGA. This could get used to make view patches for EGA games, where the
	// new views include more colors. Users could manually adjust old views to
	// make them look better (like removing dithered colors that aren't caught
	// by our undithering or even improve the graphics overall).
	ViewType curViewType = _resMan->getViewType();
	if (curViewType == kViewEga) {
		if (_resource->getUint8At(1) == 0x80) {
			curViewType = kViewVga;
		} else if (_resource->getUint16LEAt(4) == 1) {
			curViewType = kViewVga11;
		}
	}

	bool isEGA = false;
	switch (curViewType) {
	case kViewEga: // SCI0 (and Amiga 16 colors)
		isEGA = true;
		// fall through
	case kViewAmiga: // Amiga ECS (32 colors)
	case kViewAmiga64: // Amiga AGA (64 colors)
	case kViewVga: { // View-format SCI1
		// LoopCount:WORD MirrorMask:WORD Version:WORD PaletteOffset:WORD LoopOffset0:WORD LoopOffset1:WORD...

		_loop.resize(_resource->getUint8At(0));
		// bit 0x8000 of _resourceData[1] means palette is set
		bool isCompressed = true;
		if (_resource->getUint8At(1) & 0x40)
			isCompressed = false;
		uint16 mirrorBits = _resource->getUint16LEAt(2);
		uint16 palOffset = _resource->getUint16LEAt(6);

		if (palOffset && palOffset != 0x100) {
			// Some SCI0/SCI01 games also have an offset set. It seems that it
			// points to a 16-byte mapping table but on those games using that
			// mapping will actually screw things up. On the other side: VGA
			// SCI1 games have this pointing to a VGA palette and EGA SCI1 games
			// have this pointing to a 8x16 byte mapping table that needs to get
			// applied then.
			if (!isEGA) {
				_palette->createFromData(_resource->subspan(palOffset), &_viewPalette);
				_embeddedPal = true;
			} else {
				// Only use the EGA-mapping, when being SCI1 EGA
				//  SCI1 VGA conversion games (which will get detected as SCI1EARLY/MIDDLE/LATE) have some views
				//  with broken mapping tables. I guess those games won't use the mapping, so I rather disable it
				//  for them
				if (getSciVersion() == SCI_VERSION_1_EGA_ONLY) {
					uint EGAmapNr;
					for (EGAmapNr = 0; EGAmapNr < SCI_VIEW_EGAMAPPING_COUNT; EGAmapNr++) {
						const SciSpan<const byte> mapping = _resource->subspan(palOffset + EGAmapNr * SCI_VIEW_EGAMAPPING_SIZE, SCI_VIEW_EGAMAPPING_SIZE);
						if (memcmp(mapping.getUnsafeDataAt(0, SCI_VIEW_EGAMAPPING_SIZE), EGAmappingStraight, SCI_VIEW_EGAMAPPING_SIZE) != 0)
							break;
					}
					// If all mappings are "straight", then we actually ignore the mapping
					if (EGAmapNr == SCI_VIEW_EGAMAPPING_COUNT)
						_EGAmapping.clear();
					else
						_EGAmapping = _resource->subspan(palOffset, SCI_VIEW_EGAMAPPING_COUNT * SCI_VIEW_EGAMAPPING_SIZE);
				}
			}
		}

		for (uint loopNo = 0; loopNo < _loop.size(); loopNo++) {
			SciSpan<const byte> loopData = _resource->subspan(_resource->getUint16LEAt(8 + loopNo * 2));
			// CelCount:WORD Unknown:WORD CelOffset0:WORD CelOffset1:WORD...

			uint16 celCount = loopData.getUint16LEAt(0);
			_loop[loopNo].cel.resize(celCount);
			_loop[loopNo].mirrorFlag = mirrorBits & 1 ? true : false;
			mirrorBits >>= 1;

			// read cel info
			for (uint celNo = 0; celNo < celCount; celNo++) {
				uint16 celOffset = loopData.getUint16LEAt(4 + celNo * 2);
				SciSpan<const byte> celData = _resource->subspan(celOffset);

				// For VGA
				// Width:WORD Height:WORD DisplaceX:BYTE DisplaceY:BYTE ClearKey:BYTE Unknown:BYTE RLEData starts now directly
				// For EGA
				// Width:WORD Height:WORD DisplaceX:BYTE DisplaceY:BYTE ClearKey:BYTE EGAData starts now directly
				CelInfo *cel = &_loop[loopNo].cel[celNo];
				cel->scriptWidth = cel->width = celData.getUint16LEAt(0);
				cel->scriptHeight = cel->height = celData.getUint16LEAt(2);
				cel->displaceX = (signed char)celData[4];
				cel->displaceY = celData[5];
				cel->clearKey = celData[6];

				// HACK: Fix Ego's odd displacement in the QFG3 demo, scene 740.
				// For some reason, ego jumps above the rope, so we fix his rope
				// hanging view by displacing it down by 40 pixels. Fixes bug
				// #5009.
				// FIXME: Remove this once we figure out why Ego jumps so high.
				// Likely culprits include kInitBresen, kDoBresen and kCantBeHere.
				// The scripts have the y offset that hero reaches (11) hardcoded,
				// so it might be collision detection. However, since this requires
				// extensive work to fix properly for very little gain, this hack
				// here will suffice until the actual issue is found.
				if (g_sci->getGameId() == GID_QFG3 && g_sci->isDemo() && _resourceId == 39)
					cel->displaceY = 98;

				if (isEGA) {
					cel->offsetEGA = celOffset + 7;
					cel->offsetRLE = 0;
					cel->offsetLiteral = 0;
				} else {
					cel->offsetEGA = 0;
					if (isCompressed) {
						cel->offsetRLE = celOffset + 8;
						cel->offsetLiteral = 0;
					} else {
						cel->offsetRLE = 0;
						cel->offsetLiteral = celOffset + 8;
					}
				}
				cel->rawBitmap.clear();
				if (_loop[loopNo].mirrorFlag)
					cel->displaceX = -cel->displaceX;
			}
		}
		break;
	}

	case kViewVga11: { // View-format SCI1.1+
		// HeaderSize:WORD LoopCount:BYTE Flags:BYTE Version:WORD Unknown:WORD PaletteOffset:WORD
		uint16 headerSize = _resource->getUint16SEAt(0) + 2; // headerSize is not part of the header, so it's added
		assert(headerSize >= 16);
		const uint8 loopCount = _resource->getUint8At(2);
		assert(loopCount);
		uint32 palOffset = _resource->getUint32SEAt(8);

		// flags is actually a bit-mask
		//  it seems it was only used for some early sci1.1 games (or even just laura bow 2)
		//  later interpreters dont support it at all anymore
		// we assume that if flags is 0h the view does not support flags and default to scalable
		// if it's 1h then we assume that the view is not to be scaled
		// if it's 40h then we assume that the view is scalable
		switch (_resource->getUint8At(3)) {
		case 1:
			_isScaleable = false;
			break;
		case 0x40:
		case 0:
			break; // don't do anything, we already have _isScaleable set
		default:
			error("unsupported flags byte (%d) inside sci1.1 view", _resource->getUint8At(3));
			break;
		}

		uint16 loopSize = _resource->getUint8At(12);
		assert(loopSize >= 16);
		uint16 celSize = _resource->getUint8At(13);
		assert(celSize >= 32);

		if (palOffset) {
			_palette->createFromData(_resource->subspan(palOffset), &_viewPalette);
			_embeddedPal = true;
		}

		_loop.resize(loopCount);
		for (uint loopNo = 0; loopNo < loopCount; loopNo++) {
			SciSpan<const byte> loopData = _resource->subspan(headerSize + (loopNo * loopSize));

			byte seekEntry = loopData[0];
			if (seekEntry != 255) {
				_loop[loopNo].mirrorFlag = true;

				// use the root loop for mirroring. this handles rare loops that
				//  mirror loops that mirror loops. (FPFP view 844, bug #10953)
				do {
					if (seekEntry >= loopCount)
						error("Bad loop-pointer in sci 1.1 view");
					loopData = _resource->subspan(headerSize + (seekEntry * loopSize));
				} while ((seekEntry = loopData[0]) != 255);
			} else {
				_loop[loopNo].mirrorFlag = false;
			}

			uint16 celCount = loopData[2];
			_loop[loopNo].cel.resize(celCount);

			const uint32 celDataOffset = loopData.getUint32SEAt(12);

			// read cel info
			for (uint celNo = 0; celNo < celCount; celNo++) {
				SciSpan<const byte> celData = _resource->subspan(celDataOffset + celNo * celSize, celSize);

				CelInfo *cel = &_loop[loopNo].cel[celNo];
				cel->scriptWidth = cel->width = celData.getInt16SEAt(0);
				cel->scriptHeight = cel->height = celData.getInt16SEAt(2);
				cel->displaceX = celData.getInt16SEAt(4);
				cel->displaceY = celData.getInt16SEAt(6);
				if (cel->displaceY < 0)
					cel->displaceY += 255; // sierra did this adjust in their sci1.1 getCelRect() - not sure about sci32

				assert(cel->width && cel->height);

				cel->clearKey = celData[8];
				cel->offsetEGA = 0;
				cel->offsetRLE = celData.getUint32SEAt(24);
				cel->offsetLiteral = celData.getUint32SEAt(28);

				// GK1-hires content is actually uncompressed, we need to swap both so that we process it as such
				if ((cel->offsetRLE) && (!cel->offsetLiteral))
					SWAP(cel->offsetRLE, cel->offsetLiteral);

				cel->rawBitmap.clear();
				if (_loop[loopNo].mirrorFlag)
					cel->displaceX = -cel->displaceX;
			}
		}
		break;
	}

	default:
		error("ViewType was not detected, can't continue");
	}

	// Inject our own views
	//  Currently only used for Dual mode (speech + text) for games, that do not have a "BOTH" icon already
	//  Which is Laura Bow 2 + King's Quest 6
	switch (g_sci->getGameId()) {
	case GID_LAURABOW2:
		// View 995, Loop 13, Cel 0 = "TEXT"
		// View 995, Loop 13, Cel 1 = "SPEECH"
		// View 995, Loop 13, Cel 2 = "BOTH" (<- our injected view)
		if (g_sci->isCD() && _resourceId == 995) {
			// security checks
			if (_loop.size() >= 14 &&
				_loop[13].cel.size() == 2 &&
				_loop[13].cel[0].width == 46 &&
				_loop[13].cel[0].height == 11) {

				_loop[13].cel.resize(3);
				// Duplicate cel 0 to cel 2
				_loop[13].cel[2] = _loop[13].cel[0];
				// use our data (which is uncompressed bitmap data)
				_loop[13].cel[2].rawBitmap->allocateFromSpan(SciSpan<const byte>(ViewInject_LauraBow2_Both, sizeof(ViewInject_LauraBow2_Both)));
			}
		}
		break;
	case GID_KQ6:
		// View 947, Loop 8, Cel 0 = "SPEECH" (not pressed)
		// View 947, Loop 8, Cel 1 = "SPEECH" (pressed)
		// View 947, Loop 9, Cel 0 = "TEXT" (not pressed)
		// View 947, Loop 9, Cel 1 = "TEXT" (pressed)
		// View 947, Loop 12, Cel 0 = "BOTH" (not pressed) (<- our injected view)
		// View 947, Loop 12, Cel 1 = "BOTH" (pressed) (<- our injected view)
		if (g_sci->isCD() && _resourceId == 947) {
			// security checks
			if (_loop.size() == 12 &&
				_loop[8].cel.size() == 2 &&
				_loop[8].cel[0].width == 50 &&
				_loop[8].cel[0].height == 15) {

				// add another loop
				_loop.resize(_loop.size() + 1);
				// copy loop 8 to loop 12
				_loop[12] = _loop[8];
				// use our data (which is uncompressed bitmap data)
				_loop[12].cel[0].rawBitmap->allocateFromSpan(SciSpan<const byte>(ViewInject_KingsQuest6_Both1, sizeof(ViewInject_KingsQuest6_Both1)));
				_loop[12].cel[1].rawBitmap->allocateFromSpan(SciSpan<const byte>(ViewInject_KingsQuest6_Both2, sizeof(ViewInject_KingsQuest6_Both2)));
			}
		}
		break;
	default:
		break;
	}
}

GuiResourceId GfxView::getResourceId() const {
	return _resourceId;
}

int16 GfxView::getWidth(int16 loopNo, int16 celNo) const {
	return _loop.size() ? getCelInfo(loopNo, celNo)->width : 0;
}

int16 GfxView::getHeight(int16 loopNo, int16 celNo) const {
	return _loop.size() ? getCelInfo(loopNo, celNo)->height : 0;
}

const CelInfo *GfxView::getCelInfo(int16 loopNo, int16 celNo) const {
	assert(_loop.size());
	loopNo = CLIP<int16>(loopNo, 0, _loop.size() - 1);
	celNo = CLIP<int16>(celNo, 0, _loop[loopNo].cel.size() - 1);
	return &_loop[loopNo].cel[celNo];
}

uint16 GfxView::getCelCount(int16 loopNo) const {
	assert(_loop.size());
	loopNo = CLIP<int16>(loopNo, 0, _loop.size() - 1);
	return _loop[loopNo].cel.size();
}

Palette *GfxView::getPalette() {
	return _embeddedPal ? &_viewPalette : nullptr;
}

bool GfxView::isScaleable() {
	return _isScaleable;
}

void GfxView::getCelRect(int16 loopNo, int16 celNo, int16 x, int16 y, int16 z, Common::Rect &outRect) const {
	const CelInfo *celInfo = getCelInfo(loopNo, celNo);
	outRect.left = x + celInfo->displaceX - (celInfo->width >> 1);
	outRect.right = outRect.left + celInfo->width;
	outRect.bottom = y + celInfo->displaceY - z + 1 + _adjustForSci0Early;
	outRect.top = outRect.bottom - celInfo->height;
}

void GfxView::getCelSpecialHoyle4Rect(int16 loopNo, int16 celNo, int16 x, int16 y, int16 z, Common::Rect &outRect) const {
	const CelInfo *celInfo = getCelInfo(loopNo, celNo);
	int16 adjustY = y + celInfo->displaceY - celInfo->height + 1;
	int16 adjustX = x + celInfo->displaceX - ((celInfo->width - 1) >> 1);
	outRect.translate(adjustX, adjustY);
}

void GfxView::getCelScaledRect(int16 loopNo, int16 celNo, int16 x, int16 y, int16 z, int16 scaleX, int16 scaleY, Common::Rect &outRect) const {
	const CelInfo *celInfo = getCelInfo(loopNo, celNo);

	// Scaling displaceX/Y, Width/Height
	int16 scaledDisplaceX = (celInfo->displaceX * scaleX) / 128;
	int16 scaledDisplaceY = (celInfo->displaceY * scaleY) / 128;
	int16 scaledWidth = (celInfo->width * scaleX) >> 7;
	int16 scaledHeight = (celInfo->height * scaleY) >> 7;
	scaledWidth = CLIP<int16>(scaledWidth, 0, _screen->getWidth());
	scaledHeight = CLIP<int16>(scaledHeight, 0, _screen->getHeight());

	outRect.left = x + scaledDisplaceX - (scaledWidth >> 1);
	outRect.right = outRect.left + scaledWidth;
	outRect.bottom = y + scaledDisplaceY - z + 1;
	outRect.top = outRect.bottom - scaledHeight;
}

void unpackCelData(const SciSpan<const byte> &inBuffer, SciSpan<byte> &celBitmap, byte clearColor, int rlePos, int literalPos, ViewType viewType, uint16 width, bool isMacSci11View) {
	const int pixelCount = celBitmap.size();
	byte *outPtr = celBitmap.getUnsafeDataAt(0);
	byte curByte, runLength;
	// TODO: Calculate correct maximum dimensions
	const byte *rlePtr = inBuffer.getUnsafeDataAt(rlePos);
	// The existence of a literal position pointer signifies data with two
	// separate streams, most likely a SCI1.1 view
	const byte *literalPtr = inBuffer.getUnsafeDataAt(literalPos, inBuffer.size() - literalPos);
	const byte *const endOfResource = inBuffer.getUnsafeDataAt(inBuffer.size(), 0);
	int pixelNr = 0;

	memset(celBitmap.getUnsafeDataAt(0), clearColor, celBitmap.size());

	// View unpacking:
	//
	// EGA:
	// Each byte is like XXXXYYYY (XXXX: 0 - 15, YYYY: 0 - 15)
	// Set the next XXXX pixels to YYYY
	//
	// Amiga:
	// Each byte is like XXXXXYYY (XXXXX: 0 - 31, YYY: 0 - 7)
	// - Case A: YYY != 0
	//   Set the next YYY pixels to XXXXX
	// - Case B: YYY == 0
	//   Skip the next XXXXX pixels (i.e. transparency)
	//
	// Amiga 64:
	// Each byte is like XXYYYYYY (XX: 0 - 3, YYYYYY: 0 - 63)
	// - Case A: XX != 0
	//   Set the next XX pixels to YYYYYY
	// - Case B: XX == 0
	//   Skip the next YYYYYY pixels (i.e. transparency)
	//
	// VGA:
	// Each byte is like XXYYYYYY (YYYYY: 0 - 63)
	// - Case A: XX == 00 (binary)
	//   Copy next YYYYYY bytes as-is
	// - Case B: XX == 01 (binary)
	//   Same as above, copy YYYYYY + 64 bytes as-is
	// - Case C: XX == 10 (binary)
	//   Set the next YYYYY pixels to the next byte value
	// - Case D: XX == 11 (binary)
	//   Skip the next YYYYY pixels (i.e. transparency)

	if (literalPos && isMacSci11View) {
		// KQ6/Freddy Pharkas/Slater use byte lengths, all others use uint16
		// The SCI devs must have realized that a max of 255 pixels wide
		// was not very good for 320 or 640 width games.
		bool hasByteLengths =
			g_sci->getGameId() == GID_KQ6 ||
			g_sci->getGameId() == GID_FREDDYPHARKAS ||
			g_sci->getGameId() == GID_SLATER;

		// compression for SCI1.1+ Mac
		while (pixelNr < pixelCount) {
			uint32 pixelLine = pixelNr;

			if (hasByteLengths) {
				assert (rlePtr + 2 <= endOfResource);
				pixelNr += *rlePtr++;
				runLength = *rlePtr++;
			} else {
				assert (rlePtr + 4 <= endOfResource);
				pixelNr += READ_BE_UINT16(rlePtr);
				runLength = READ_BE_UINT16(rlePtr + 2);
				rlePtr += 4;
			}

			assert(literalPtr + MIN<int>(runLength, pixelCount - pixelNr) <= endOfResource);
			while (runLength-- && pixelNr < pixelCount)
				outPtr[pixelNr++] = *literalPtr++;

			pixelNr = pixelLine + width;
		}
		return;
	}

	switch (viewType) {
	case kViewEga:
		while (pixelNr < pixelCount) {
			curByte = *rlePtr++;
			runLength = curByte >> 4;
			memset(outPtr + pixelNr, curByte & 0x0F, MIN<uint16>(runLength, pixelCount - pixelNr));
			pixelNr += runLength;
		}
		break;
	case kViewAmiga:
		while (pixelNr < pixelCount) {
			curByte = *rlePtr++;
			if (curByte & 0x07) { // fill with color
				runLength = curByte & 0x07;
				curByte = curByte >> 3;
				memset(outPtr + pixelNr, curByte, MIN<uint16>(runLength, pixelCount - pixelNr));
			} else { // skip the next pixels (transparency)
				runLength = curByte >> 3;
			}
			pixelNr += runLength;
		}
		break;
	case kViewAmiga64:
		while (pixelNr < pixelCount) {
			curByte = *rlePtr++;
			if (curByte & 0xC0) { // fill with color
				runLength = curByte >> 6;
				curByte = curByte & 0x3F;
				memset(outPtr + pixelNr, curByte, MIN<uint16>(runLength, pixelCount - pixelNr));
			} else { // skip the next pixels (transparency)
				runLength = curByte & 0x3F;
			}
			pixelNr += runLength;
		}
		break;
	case kViewVga:
	case kViewVga11:
		// If we have no RLE data, the image is just uncompressed
		if (rlePos == 0) {
			memcpy(outPtr, literalPtr, pixelCount);
			break;
		}

		while (pixelNr < pixelCount) {
			curByte = *rlePtr++;
			runLength = curByte & 0x3F;

			switch (curByte & 0xC0) {
			case 0x40: // copy bytes as is (In copy case, runLength can go up to 127 i.e. pixel & 0x40). Fixes bug #5551.
				runLength += 64;
				// fall through
			case 0x00: // copy bytes as-is
				if (!literalPos) {
					memcpy(outPtr + pixelNr,        rlePtr, MIN<uint16>(runLength, pixelCount - pixelNr));
					rlePtr += runLength;
				} else {
					memcpy(outPtr + pixelNr,    literalPtr, MIN<uint16>(runLength, pixelCount - pixelNr));
					literalPtr += runLength;
				}
				break;
			case 0x80: // fill with color
				if (!literalPos)
					memset(outPtr + pixelNr,     *rlePtr++, MIN<uint16>(runLength, pixelCount - pixelNr));
				else
					memset(outPtr + pixelNr, *literalPtr++, MIN<uint16>(runLength, pixelCount - pixelNr));
				break;
			case 0xC0: // skip the next pixels (transparency)
			default:
				break;
			}

			pixelNr += runLength;
		}
		break;
	default:
		error("Unsupported picture viewtype");
	}
}

void GfxView::unpackCel(int16 loopNo, int16 celNo, SciSpan<byte> &outPtr) {
	const CelInfo *celInfo = getCelInfo(loopNo, celNo);

	if (celInfo->offsetEGA) {
		// decompression for EGA views
		unpackCelData(*_resource, outPtr, 0, celInfo->offsetEGA, 0, _resMan->getViewType(), celInfo->width, false);
	} else {
		// We fill the buffer with transparent pixels, so that we can later skip
		//  over pixels to automatically have them transparent
		// Also some RLE compressed cels are possibly ending with the last
		// non-transparent pixel (is this even possible with the current code?)
		byte clearColor = _loop[loopNo].cel[celNo].clearKey;

		// Since Mac OS required palette index 0 to be white and 0xff to be black, the
		// Mac SCI devs decided that rather than change scripts and various pieces of
		// code, that they would just put a little snippet of code to swap these colors
		// in various places around the SCI codebase. We figured that it would be less
		// hacky to swap pixels instead and run the Mac games with a PC palette.
		bool isMacSci11View = g_sci->getPlatform() == Common::kPlatformMacintosh && getSciVersion() == SCI_VERSION_1_1;
		if (isMacSci11View) {
			// clearColor is based on PC palette, but the literal data is not.
			// We flip clearColor here to make it match the literal data. All
			// these pixels will be flipped back again below.
			if (clearColor == 0)
				clearColor = 0xff;
			else if (clearColor == 0xff)
				clearColor = 0;
		}

		unpackCelData(*_resource, outPtr, clearColor, celInfo->offsetRLE, celInfo->offsetLiteral, _resMan->getViewType(), celInfo->width, isMacSci11View);

		// Swap 0 and 0xff pixels for Mac SCI1.1+ games (see above)
		if (isMacSci11View) {
			for (uint32 i = 0; i < outPtr.size(); i++) {
				if (outPtr[i] == 0)
					outPtr[i] = 0xff;
				else if (outPtr[i] == 0xff)
					outPtr[i] = 0;
			}
		}
	}
}

const SciSpan<const byte> &GfxView::getBitmap(int16 loopNo, int16 celNo) {
	loopNo = CLIP<int16>(loopNo, 0, _loop.size() - 1);
	celNo = CLIP<int16>(celNo, 0, _loop[loopNo].cel.size() - 1);

	CelInfo &cel = _loop[loopNo].cel[celNo];

	if (cel.rawBitmap)
		return *cel.rawBitmap;

	const uint16 width = cel.width;
	const uint16 height = cel.height;
	const uint pixelCount = width * height;
	const Common::String sourceName = Common::String::format("%s loop %d cel %d", _resource->name().c_str(), loopNo, celNo);

	SciSpan<byte> outBitmap = cel.rawBitmap->allocate(pixelCount, sourceName);

	// unpack the actual cel bitmap data
	unpackCel(loopNo, celNo, outBitmap);

	if (_resMan->getViewType() == kViewEga)
		unditherBitmap(outBitmap, width, height, _loop[loopNo].cel[celNo].clearKey);

	// mirroring the cel if needed
	if (_loop[loopNo].mirrorFlag) {
		byte *pBitmap = outBitmap.getUnsafeDataAt(0, width * height);
		for (int i = 0; i < height; i++, pBitmap += width)
			for (int j = 0; j < width / 2; j++)
				SWAP(pBitmap[j], pBitmap[width - j - 1]);
	}

	return *cel.rawBitmap;
}

/**
 * Called after unpacking an EGA cel, this will try to undither (parts) of the
 * cel if the dithering in here matches dithering used by the current picture.
 */
void GfxView::unditherBitmap(SciSpan<byte> &bitmapPtr, int16 width, int16 height, byte clearKey) {
	int16 *ditheredPicColors = _screen->unditherGetDitheredBgColors();

	// It makes no sense to go further, if there isn't any dithered color data
	// available for the current picture
	if (!ditheredPicColors)
		return;

	// We need at least a 4x2 bitmap for this algorithm to work
	if (width < 4 || height < 2)
		return;

	// If EGA mapping is used for this view, dont do undithering as well
	if (_EGAmapping)
		return;

	// Walk through the bitmap and remember all combinations of colors
	int16 ditheredBitmapColors[DITHERED_BG_COLORS_SIZE];

	memset(&ditheredBitmapColors, 0, sizeof(ditheredBitmapColors));

	// Count all seemingly dithered pixel-combinations as soon as at least 4
	// pixels are adjacent and check pixels in the following line as well to
	// be the reverse pixel combination
	int16 checkHeight = height - 1;
	byte *curPtr = bitmapPtr.getUnsafeDataAt(0, checkHeight * width);
	const byte *nextPtr = bitmapPtr.getUnsafeDataAt(width, checkHeight * width);
	for (int16 y = 0; y < checkHeight; y++) {
		byte color1 = curPtr[0];
		byte color2 = (curPtr[1] << 4) | curPtr[2];
		byte nextColor1 = nextPtr[0] << 4;
		byte nextColor2 = (nextPtr[2] << 4) | nextPtr[1];
		curPtr += 3;
		nextPtr += 3;
		for (int16 x = 3; x < width; x++) {
			color1 = (color1 << 4) | (color2 >> 4);
			color2 = (color2 << 4) | *curPtr++;
			nextColor1 = (nextColor1 >> 4) | (nextColor2 << 4);
			nextColor2 = (nextColor2 >> 4) | *nextPtr++ << 4;
			if ((color1 == color2) && (color1 == nextColor1) && (color1 == nextColor2))
				ditheredBitmapColors[color1]++;
		}
	}

	// Now compare both dither color tables to find out matching dithered color
	// combinations
	bool unditherTable[DITHERED_BG_COLORS_SIZE];
	byte unditherCount = 0;
	memset(&unditherTable, false, sizeof(unditherTable));
	for (byte color = 0; color < 255; color++) {
		if ((ditheredBitmapColors[color] > 5) && (ditheredPicColors[color] > 200)) {
			// match found, check if colorKey is contained -> if so, we ignore
			// of course
			byte color1 = color & 0x0F;
			byte color2 = color >> 4;
			if ((color1 != clearKey) && (color2 != clearKey) && (color1 != color2)) {
				// so set this and the reversed color-combination for undithering
				unditherTable[color] = true;
				unditherTable[(color1 << 4) | color2] = true;
				unditherCount++;
			}
		}
	}

	// Nothing found to undither -> exit straight away
	if (!unditherCount)
		return;

	// We now need to replace color-combinations
	curPtr = bitmapPtr.getUnsafeDataAt(0, height * width);
	for (int16 y = 0; y < height; y++) {
		byte color = curPtr[0];
		for (int16 x = 1; x < width; x++) {
			color = (color << 4) | curPtr[1];
			if (unditherTable[color]) {
				// Some color with black? Turn colors around, otherwise it won't
				// be the right color at all.
				byte unditheredColor = color;
				if ((color & 0xF0) == 0)
					unditheredColor = (color << 4) | (color >> 4);
				curPtr[0] = unditheredColor;
				curPtr[1] = unditheredColor;
			}
			curPtr++;
		}
		curPtr++;
	}
}

byte GfxView::getMappedColor(byte color, uint16 scaleSignal, const Palette *palette, int x2, int y2) {
	byte outputColor = palette->mapping[color];
	// SCI16 remapping (QFG4 demo)
	if (g_sci->_gfxRemap16 && g_sci->_gfxRemap16->isRemapped(outputColor))
		outputColor = g_sci->_gfxRemap16->remapColor(outputColor, _screen->getVisual(x2, y2));
	// SCI11+ remapping (Catdate)
	if ((scaleSignal & 0xFF00) && g_sci->_gfxRemap16 && _resMan->testResource(ResourceId(kResourceTypeVocab, 184))) {
		if ((scaleSignal >> 8) == 1) // all black
			outputColor = 0;
		else if ((scaleSignal >> 8) == 2) // darken
			outputColor = g_sci->_gfxRemap16->remapColor(253, outputColor);
		else if ((scaleSignal >> 8) == 3) // shadow
			outputColor = g_sci->_gfxRemap16->remapColor(253, _screen->getVisual(x2, y2));
	}
	return outputColor;
}

void GfxView::draw(const Common::Rect &rect, const Common::Rect &clipRect, const Common::Rect &clipRectTranslated,
			int16 loopNo, int16 celNo, byte priority, uint16 EGAmappingNr, bool upscaledHires, uint16 scaleSignal) {
	const Palette *palette = _embeddedPal ? &_viewPalette : &_palette->_sysPalette;
	const CelInfo *celInfo = getCelInfo(loopNo, celNo);
	const SciSpan<const byte> &bitmap = getBitmap(loopNo, celNo);
	const int16 celHeight = celInfo->height;
	const int16 celWidth = celInfo->width;
	const byte clearKey = celInfo->clearKey;
	const byte drawMask = priority > 15 ? GFX_SCREEN_MASK_VISUAL : GFX_SCREEN_MASK_VISUAL|GFX_SCREEN_MASK_PRIORITY;

	if (_embeddedPal)
		// Merge view palette in...
		_palette->set(&_viewPalette, false);

	const int16 width = MIN(clipRect.width(), celWidth);
	const int16 height = MIN(clipRect.height(), celHeight);

	if (!width || !height) {
		return;
	}

	const byte *bitmapData = bitmap.getUnsafeDataAt((clipRect.top - rect.top) * celWidth + (clipRect.left - rect.left), celWidth * (height - 1) + width);

	// Set up custom per-view palette mod
	byte oldpalvalue = _screen->getCurPaletteMapValue();
	doCustomViewPalette(_screen, _resourceId, loopNo, celNo);

	if (_EGAmapping) {
		const SciSpan<const byte> EGAmapping = _EGAmapping.subspan(EGAmappingNr * SCI_VIEW_EGAMAPPING_SIZE, SCI_VIEW_EGAMAPPING_SIZE);
		for (int y = 0; y < height; y++, bitmapData += celWidth) {
			for (int x = 0; x < width; x++) {
				const byte color = EGAmapping[bitmapData[x]];
				const int x2 = clipRectTranslated.left + x;
				const int y2 = clipRectTranslated.top + y;
				if (color != clearKey && priority >= _screen->getPriority(x2, y2))
					_screen->putPixel(x2, y2, drawMask, color, priority, 0);
			}
		}
	} else if (upscaledHires) {
		// UpscaledHires means view is hires and is supposed to
		// get drawn onto lowres screen.
		for (int y = 0; y < height; y++, bitmapData += celWidth) {
			for (int x = 0; x < width; x++) {
				const byte color = bitmapData[x];
				const int x2 = clipRectTranslated.left + x;
				const int y2 = clipRectTranslated.top + y;
				_screen->putPixelOnDisplay(x2, y2, palette->mapping[color]);
			}
		}
	} else {
		for (int y = 0; y < height; y++, bitmapData += celWidth) {
			for (int x = 0; x < width; x++) {
				const byte color = bitmapData[x];
				if (color != clearKey) {
					const int x2 = clipRectTranslated.left + x;
					const int y2 = clipRectTranslated.top + y;
					if (priority >= _screen->getPriority(x2, y2)) {
						_screen->putPixel(x2, y2, drawMask, getMappedColor(color, scaleSignal, palette, x2, y2), priority, 0);
					}
				}
			}
		}
	}

	// Reset custom per-view palette mod
	_screen->setCurPaletteMapValue(oldpalvalue);
}

void GfxView::drawScaled(const Common::Rect &rect, const Common::Rect &clipRect, const Common::Rect &clipRectTranslated,
			int16 loopNo, int16 celNo, byte priority, int16 scaleX, int16 scaleY, uint16 scaleSignal) {
	const Palette *palette = _embeddedPal ? &_viewPalette : &_palette->_sysPalette;
	const CelInfo *celInfo = getCelInfo(loopNo, celNo);
	const SciSpan<const byte> &bitmap = getBitmap(loopNo, celNo);
	const int16 celHeight = celInfo->height;
	const int16 celWidth = celInfo->width;
	const byte clearKey = celInfo->clearKey;
	const byte drawMask = priority > 15 ? GFX_SCREEN_MASK_VISUAL : GFX_SCREEN_MASK_VISUAL|GFX_SCREEN_MASK_PRIORITY;

	if (_embeddedPal)
		// Merge view palette in...
		_palette->set(&_viewPalette, false);

	Common::Array<uint16> scalingX, scalingY;
	const bool mirrorFlag = _loop[CLIP<int16>(loopNo, 0, _loop.size() - 1)].mirrorFlag;
	createScalingTable(scalingX, celWidth, _screen->getWidth(), scaleX, mirrorFlag);
	if (mirrorFlag) {
		// reverse the table when mirroring; we already reversed the bitmap
		uint scaleTableSize = scalingX.size();
		for (uint i = 0; i < scaleTableSize / 2; i++) {
			SWAP(scalingX[i], scalingX[scaleTableSize - i - 1]);
		}
	}
	createScalingTable(scalingY, celHeight, _screen->getHeight(), scaleY, false);

	int16 scaledWidth = MIN(clipRect.width(), (int16)scalingX.size());
	int16 scaledHeight = MIN(clipRect.height(), (int16)scalingY.size());

	const int16 offsetY = clipRect.top - rect.top;
	const int16 offsetX = clipRect.left - rect.left;

	const byte *bitmapData = bitmap.getUnsafeDataAt(0, celWidth * celHeight);
	for (int y = 0; y < scaledHeight; y++) {
		for (int x = 0; x < scaledWidth; x++) {
			const byte color = bitmapData[scalingY[y + offsetY] * celWidth + scalingX[x + offsetX]];
			const int x2 = clipRectTranslated.left + x;
			const int y2 = clipRectTranslated.top + y;
			if (color != clearKey && priority >= _screen->getPriority(x2, y2)) {
				_screen->putPixel(x2, y2, drawMask, getMappedColor(color, scaleSignal, palette, x2, y2), priority, 0);
			}
		}
	}
}

void GfxView::createScalingTable(Common::Array<uint16> &table, int16 celSize, uint16 maxSize, int16 scale, bool mirrorFlag) {
	const int16 scaledSize = (celSize * scale) >> 7;
	const int16 clippedScaledSize = CLIP<int16>(scaledSize, 0, maxSize);
	const int16 stepCount = scaledSize - 1;

	if (clippedScaledSize <= 0) {
		table.clear();
		return;
	}

	// Note that the table produced by this algorithm when mirroring
	// is slightly different than simply reversing the normal table.
	const int16 start = mirrorFlag ? (celSize - 1) : 0;
	const int16 end   = mirrorFlag ? 0 : (celSize - 1);

	int32 acc;
	int32 inc;
	bool negative = false;
	if (stepCount == 0) {
		acc = start << 16;
		inc = 0;
	} else {
		acc = start << 16;
		inc = end << 16;
		inc -= acc;
		inc /= stepCount;
		if (inc < 0) {
			inc = -inc;
			negative = true;
		}
		if ((inc & 0xffff8000) == 0) {
			acc = (acc & 0xffff0000) | 0x8000;
		} else {
			acc = (acc & 0xffff0000) | (inc & 0xffff);
		}
	}
	if (negative) {
		inc = -inc;
	}

	table.resize(clippedScaledSize);
	for (uint16 i = 0; i < clippedScaledSize; ++i) {
		table[i] = acc >> 16;
		acc += inc;
	}
}

void GfxView::adjustToUpscaledCoordinates(int16 &y, int16 &x) {
	_screen->adjustToUpscaledCoordinates(y, x);
}

void GfxView::adjustBackUpscaledCoordinates(int16 &y, int16 &x) {
	_screen->adjustBackUpscaledCoordinates(y, x);
}

} // End of namespace Sci
