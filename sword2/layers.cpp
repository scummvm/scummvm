/* Copyright (C) 1994-2004 Revolution Software Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

// high level layer initialising

// the system supports:
//	1 optional background parallax layer
//	1 not optional normal backdrop layer
//	3 normal sorted layers
//	up to 2 foreground parallax layers

#include "common/stdafx.h"
#include "sword2/sword2.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/resman.h"
#include "sword2/driver/d_draw.h"

namespace Sword2 {

int32 Logic::fnInitBackground(int32 *params) {
	// this screen defines the size of the back buffer

	// params:	0 res id of normal background layer - cannot be 0
	//		1 1 yes 0 no for a new palette

	return _vm->initBackground(params[0], params[1]);
}

/**
 * This function is called when entering a new room.
 * @param res resource id of the normal background layer
 * @param new_palette 1 for new palette, otherwise 0
 */

int32 Sword2Engine::initBackground(int32 res, int32 new_palette) {
	byte buf[NAME_LEN];
	int i;

	assert(res);
	debug(1, "CHANGED TO LOCATION \"%s\"", fetchObjectName(res, buf));

	// The resources age every time a new room is entered.
	_resman->passTime();
	_resman->expireOldResources();

	clearFxQueue();
	_graphics->waitForFade();

	// if last screen was using a shading mask (see below)
	if (_thisScreen.mask_flag) {
		if (_graphics->closeLightMask() != RD_OK)
			error("Could not close light mask");
	}

	// Close the previous screen, if one is open
	if (_thisScreen.background_layer_id)
		_graphics->closeBackgroundLayer();

	_thisScreen.background_layer_id = res;
	_thisScreen.new_palette = new_palette;

	// ok, now read the resource and pull out all the normal sort layer
	// info/and set them up at the beginning of the sort list - why do it
	// each cycle

	byte *file = _resman->openResource(_thisScreen.background_layer_id);
	ScreenHeader *screen_head = fetchScreenHeader(file);

	// set number of special sort layers
	_thisScreen.number_of_layers = screen_head->noLayers;
	_thisScreen.screen_wide = screen_head->width;
	_thisScreen.screen_deep = screen_head->height;

	debug(2, "layers=%d width=%d depth=%d", screen_head->noLayers, screen_head->width, screen_head->height);

	// initialise the driver back buffer
	_graphics->setLocationMetrics(screen_head->width, screen_head->height);

	for (i = 0; i < screen_head->noLayers; i++) {
		debug(3, "init layer %d", i);

		LayerHeader *layer = fetchLayerHeader(file, i);

		// Add the layer to the sort list. We only provide just enough
		// information so that it's clear that it's a layer, and where
		// to sort it in relation to other things in the list.

		_sortList[i].layer_number = i + 1;
		_sortList[i].sort_y = layer->y + layer->height;
	}

	// reset scroll offsets
	_thisScreen.scroll_offset_x = 0;
	_thisScreen.scroll_offset_y = 0;

	if (screen_head->width > _graphics->_screenWide || screen_head->height > _graphics->_screenDeep) {
		// The layer is larger than the physical screen. Switch on
		// scrolling. (2 means first time on screen)
		_thisScreen.scroll_flag = 2;

		// Note: if we've already set the player up then we could do
		// the initial scroll set here

		// Calculate the maximum scroll offsets to prevent scrolling
		// off the edge. The minimum offsets are both 0.

		_thisScreen.max_scroll_offset_x = screen_head->width - _graphics->_screenWide;
		_thisScreen.max_scroll_offset_y = screen_head->height - (_graphics->_screenDeep - (RDMENU_MENUDEEP * 2));
	} else {
		// The later fits on the phyiscal screen. Switch off scrolling.
		_thisScreen.scroll_flag = 0;
	}

	_graphics->resetRenderEngine();

	// These are the physical screen coords where the system will try to
	// maintain George's actual feet coords.

	_thisScreen.feet_x = 320;
	_thisScreen.feet_y = 340;

	// shading mask

	MultiScreenHeader *screenLayerTable = (MultiScreenHeader *) (file + sizeof(StandardHeader));

	if (screenLayerTable->maskOffset) {
	 	SpriteInfo spriteInfo;

		spriteInfo.x = 0;
		spriteInfo.y = 0;
		spriteInfo.w = screen_head->width;
		spriteInfo.h = screen_head->height;
		spriteInfo.scale = 0;
		spriteInfo.scaledWidth = 0;
		spriteInfo.scaledHeight = 0;
		spriteInfo.type = 0;
		spriteInfo.blend = 0;
		spriteInfo.data = fetchShadingMask(file);
		spriteInfo.colourTable = 0;

		if (_graphics->openLightMask(&spriteInfo) != RD_OK)
			error("Could not open light mask");

		// so we know to close it later! (see above)
		_thisScreen.mask_flag = true;
	} else {
		// no need to close a mask later
		_thisScreen.mask_flag = false;
	}

	// Background parallax layers

	for (i = 0; i < 2; i++) {
		if (screenLayerTable->bg_parallax[i])
			_graphics->initialiseBackgroundLayer(fetchBackgroundParallaxLayer(file, i));
		else
			_graphics->initialiseBackgroundLayer(NULL);
	}

	// Normal backround layer

	_graphics->initialiseBackgroundLayer(fetchBackgroundLayer(file));

	// Foreground parallax layers

	for (i = 0; i < 2; i++) {
		if (screenLayerTable->fg_parallax[i])
			_graphics->initialiseBackgroundLayer(fetchForegroundParallaxLayer(file, i));
		else
			_graphics->initialiseBackgroundLayer(NULL);
	}

   	_resman->closeResource(_thisScreen.background_layer_id);
	return IR_CONT;
}

} // End of namespace Sword2
