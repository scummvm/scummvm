/* Copyright (C) 1994-2003 Revolution Software Ltd
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

namespace Sword2 {

int32 Logic::fnInitBackground(int32 *params) {
	// this screen defines the size of the back buffer

	// params:	0 res id of normal background layer - cannot be 0
	//		1 1 yes 0 no for a new palette

	return _vm->initBackground(params[0], params[1]);
}

int32 Sword2Engine::initBackground(int32 res, int32 new_palette) {
	_multiScreenHeader *screenLayerTable;
	_screenHeader *screen_head;
	_layerHeader *layer;
 	_spriteInfo spriteInfo;
	uint8 *file;
	uint32 rv;

	debug(5, "CHANGED TO LOCATION \"%s\"", fetchObjectName(res));

	// stop all fx & clears the queue
	clearFxQueue();

#ifdef _SWORD2_DEBUG
	debug(5, "fnInitBackground(%d)", res);

	if (!res) {
		error("ERROR: fnInitBackground cannot have 0 for background layer id!");
	}
#endif

	// if the screen is still fading down then wait for black
	_graphics->waitForFade();

	// if last screen was using a shading mask (see below)
	if (_thisScreen.mask_flag) {
		rv = _graphics->closeLightMask();
		if (rv)
			error("Driver Error %.8x", rv);
	}
	
	// New stuff for faster screen drivers

	// for drivers: close the previous screen if one is open
	if (_thisScreen.background_layer_id)
		_graphics->closeBackgroundLayer();

	_thisScreen.background_layer_id = res;
	_thisScreen.new_palette = new_palette;

	// ok, now read the resource and pull out all the normal sort layer
	// info/and set them up at the beginning of the sort list - why do it
	// each cycle

	// file points to 1st byte in the layer file
	file = _resman->openResource(_thisScreen.background_layer_id);
	
	screen_head = fetchScreenHeader(file);

	//set number of special sort layers
	_thisScreen.number_of_layers = screen_head->noLayers;
	_thisScreen.screen_wide = screen_head->width;
	_thisScreen.screen_deep = screen_head->height;

	debug(5, "res test layers=%d width=%d depth=%d", screen_head->noLayers, screen_head->width, screen_head->height);

	//initialise the driver back buffer
	_graphics->setLocationMetrics(screen_head->width, screen_head->height);

	if (screen_head->noLayers) {
		for (int i = 0; i < screen_head->noLayers; i++) {
			// get layer header for layer i
			layer = fetchLayerHeader(file, i);

			// add into the sort list

			// need this for sorting - but leave the rest blank,
			// we'll take from the header at print time
			_sortList[i].sort_y = layer->y + layer->height;
			// signifies a layer
			_sortList[i].layer_number = i + 1;

			debug(5, "init layer %d", i);
		}
	}

	// using the screen size setup the scrolling variables

	// if layer is larger than physical screen
	if (screen_head->width > _graphics->_screenWide || screen_head->height > _graphics->_screenDeep) {
		// switch on scrolling (2 means first time on screen)
		_thisScreen.scroll_flag = 2;

		// note, if we've already set the player up then we could do
		// the initial scroll set here

		// reset scroll offsets

		_thisScreen.scroll_offset_x = 0;
		_thisScreen.scroll_offset_y = 0;

		// calc max allowed offsets (to prevent scrolling off edge) -
		// MOVE TO NEW_SCREEN in GTM_CORE.C !!
		// NB. min scroll offsets are both zero
		_thisScreen.max_scroll_offset_x = screen_head->width - _graphics->_screenWide;
		// 'screenDeep' includes the menu's, so take away 80 pixels
		_thisScreen.max_scroll_offset_y = screen_head->height - (_graphics->_screenDeep - (RDMENU_MENUDEEP * 2));
	} else {
		// layer fits on physical screen - scrolling not required
		_thisScreen.scroll_flag = 0;		// switch off scrolling
		_thisScreen.scroll_offset_x = 0;	// reset scroll offsets
		_thisScreen.scroll_offset_y = 0;
	}

	// no inter-cycle scroll between new screens (see setScrollTarget in
	// build display)
	_graphics->resetRenderEngine();

	// these are the physical screen coords where the system
	// will try to maintain George's actual feet coords
	_thisScreen.feet_x = 320;
	_thisScreen.feet_y = 340;

	// shading mask

	screenLayerTable = (_multiScreenHeader *) ((uint8 *) file + sizeof(_standardHeader));

	if (screenLayerTable->maskOffset) {
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

		rv = _graphics->openLightMask(&spriteInfo);
		if (rv)
			error("Driver Error %.8x", rv);

		// so we know to close it later! (see above)
		_thisScreen.mask_flag = 1;
	} else {
		// no need to close a mask later
		_thisScreen.mask_flag = 0;
	}

	// close the screen file
   	_resman->closeResource(_thisScreen.background_layer_id);

	setUpBackgroundLayers();

	debug(5, "end init");
	return 1;
}

// called from fnInitBackground and also from control panel

void Sword2Engine::setUpBackgroundLayers(void) {
	_multiScreenHeader *screenLayerTable;
	_screenHeader *screen_head;
	uint8 *file;
	int i;

	// if we actually have a screen to initialise (in case not called from
	// control panel)
	if (_thisScreen.background_layer_id) {
		// open resource & set pointers to headers
		// file points to 1st byte in the layer file

		file = _resman->openResource(_thisScreen.background_layer_id);

		screen_head = fetchScreenHeader(file);

		screenLayerTable = (_multiScreenHeader *) ((uint8 *) file + sizeof(_standardHeader));

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

		// close the screen file
		_resman->closeResource(_thisScreen.background_layer_id);
	}
}

} // End of namespace Sword2
