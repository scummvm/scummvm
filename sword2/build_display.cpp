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

// ---------------------------------------------------------------------------
// BUILD_DISPLAY.CPP	like the old spr_engi but slightly more aptly named
// ---------------------------------------------------------------------------

#include "stdafx.h"
#include "sword2/sword2.h"
#include "sword2/debug.h"
#include "sword2/build_display.h"
#include "sword2/console.h"
#include "sword2/defs.h"
#include "sword2/interpreter.h"
#include "sword2/layers.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/mouse.h"
#include "sword2/object.h"
#include "sword2/protocol.h"
#include "sword2/resman.h"

namespace Sword2 {

void Sword2Engine::buildDisplay(void) {
	uint8 *file;
	_multiScreenHeader *screenLayerTable;

	if (_thisScreen.new_palette) {
		// start the layer palette fading up
		startNewPalette();

		// should be reset to zero at start of each screen change
		_largestLayerArea = 0;
		_largestSpriteArea = 0;
	}

	// there is a valid screen to run
	if (_thisScreen.background_layer_id) {
		// set the scroll position
		g_display->setScrollTarget(_thisScreen.scroll_offset_x, _thisScreen.scroll_offset_y);
		// increment the mouse frame
		g_display->animateMouse();

		g_display->startRenderCycle();

		while (1) {
			// clear the back buffer, before building up the new
			// screen from the back forwards

			// FIXME: I'm not convinced that this is needed. Isn't
			// the whole screen redrawn each time?

			// g_display->clearScene();

			// first background parallax + related anims

			// open the screen resource
			file = res_man->openResource(_thisScreen.background_layer_id);
			screenLayerTable = (_multiScreenHeader *) ((uint8 *) file + sizeof(_standardHeader));

			if (screenLayerTable->bg_parallax[0]) {
				g_display->renderParallax(fetchBackgroundParallaxLayer(file, 0), 0);
				// release the screen resource before cacheing
				// the sprites
	 			res_man->closeResource(_thisScreen.background_layer_id);
				sendBackPar0Frames();
			} else {
				// release the screen resource
 	 			res_man->closeResource(_thisScreen.background_layer_id);
			}

			// second background parallax + related anims

			// open the screen resource
			file = res_man->openResource(_thisScreen.background_layer_id);
			screenLayerTable = (_multiScreenHeader *) ((uint8 *) file + sizeof(_standardHeader));

			if (screenLayerTable->bg_parallax[1]) {
				g_display->renderParallax(fetchBackgroundParallaxLayer(file, 1), 1);
				// release the screen resource before cacheing
				// the sprites
	 			res_man->closeResource(_thisScreen.background_layer_id);
				sendBackPar1Frames();
			} else {
				// release the screen resource
 	 			res_man->closeResource(_thisScreen.background_layer_id);
			}

			// normal backround layer (just the one!)

			// open the screen resource
			file = res_man->openResource(_thisScreen.background_layer_id);
			g_display->renderParallax(fetchBackgroundLayer(file), 2);
			// release the screen resource
			res_man->closeResource(_thisScreen.background_layer_id);

			// sprites & layers

			sendBackFrames();	// background sprites
			sortTheSortList();
			sendSortFrames();	// sorted sprites & layers
			sendForeFrames();	// foreground sprites

			// first foreground parallax + related anims

			// open the screen resource
			file = res_man->openResource(_thisScreen.background_layer_id);
			screenLayerTable = (_multiScreenHeader *) ((uint8 *) file + sizeof(_standardHeader));

			if (screenLayerTable->fg_parallax[0]) {
				g_display->renderParallax(fetchForegroundParallaxLayer(file, 0), 3);
				// release the screen resource before cacheing
				// the sprites
	 			res_man->closeResource(_thisScreen.background_layer_id);
				sendForePar0Frames();
			} else {
				// release the screen resource
 	 			res_man->closeResource(_thisScreen.background_layer_id);
			}

			// second foreground parallax + related anims

			// open the screen resource
			file = res_man->openResource(_thisScreen.background_layer_id);
			screenLayerTable = (_multiScreenHeader *) ((uint8 *) file + sizeof(_standardHeader));

			if (screenLayerTable->fg_parallax[1]) {
				g_display->renderParallax(fetchForegroundParallaxLayer(file, 1), 4);
				// release the screen resource before cacheing
				// the sprites
	 			res_man->closeResource(_thisScreen.background_layer_id);
				sendForePar1Frames();
			} else {
				// release the screen resource
 	 			res_man->closeResource(_thisScreen.background_layer_id);
			}

			// walkgrid, mouse & player markers & mouse area
			// rectangle

			_debugger->drawDebugGraphics();

			// text blocks

			// speech blocks and headup debug text
			fontRenderer.printTextBlocs();

			// menu bar & icons

			g_display->processMenu();

			// ready - blit to screen

			g_display->updateDisplay();

			// update our fps reading

			_frameCount++;
			if (SVM_timeGetTime() > _cycleTime) {
				_fps = _frameCount;
				debug(2, "FPS: %d", _fps);
				_frameCount = 0;
				_cycleTime = SVM_timeGetTime() + 1000;
			}

			// Check if we've got time to render the screen again
			// this cycle (so drivers can smooth out the scrolling
			// in between normal game cycles)
			//
			// If we haven't got time to render again this cycle,
			// drop out of 'render cycle' while-loop

			if (g_display->endRenderCycle())
				break;
		}
	}
}

//
// Fades down and displays a message on the screen for time seconds
//

void Sword2Engine::displayMsg(uint8 *text, int time) {
	mem *text_spr;
	_frameHeader *frame;
	_spriteInfo spriteInfo;
	_palEntry pal[256];
	_palEntry oldPal[256];
	uint32 rv;	// drivers error return value

	debug(2, "DisplayMsg: %s", (char *) text);
	
	if (g_display->getFadeStatus() != RDFADE_BLACK) {
		g_display->fadeDown();
		g_display->waitForFade();
	}

	setMouse(0);
	setLuggage(0);

	g_display->closeMenuImmediately();
	g_display->clearScene();

	text_spr = fontRenderer.makeTextSprite(text, 640, 187, g_sword2->_speechFontId);

	frame = (_frameHeader *) text_spr->ad;

	spriteInfo.x = g_display->_screenWide / 2 - frame->width / 2;
	if (!time)
		spriteInfo.y = g_display->_screenDeep / 2 - frame->height / 2 - RDMENU_MENUDEEP;
	else
		spriteInfo.y = 400 - frame->height;
	spriteInfo.w = frame->width;
	spriteInfo.h = frame->height;
	spriteInfo.scale = 0;
	spriteInfo.scaledWidth = 0;
	spriteInfo.scaledHeight	= 0;
	spriteInfo.type = RDSPR_DISPLAYALIGN | RDSPR_NOCOMPRESSION | RDSPR_TRANS;
	spriteInfo.blend = 0;
	spriteInfo.data = text_spr->ad + sizeof(_frameHeader);
	spriteInfo.colourTable = 0;

	rv = g_display->drawSprite(&spriteInfo);
	if (rv)
		error("Driver Error %.8x (in DisplayMsg)", rv);

	memcpy((char *) oldPal, (char *) g_display->_palCopy, 256 * sizeof(_palEntry));

	memset(pal, 0, 256 * sizeof(_palEntry));
	pal[187].red = 255;
	pal[187].green = 255;
	pal[187].blue = 255;
	g_display->setPalette(0, 256, (uint8 *) pal, RDPAL_FADE);

	g_display->fadeUp();

	memory->freeMemory(text_spr);

	g_display->waitForFade();

	uint32 targetTime = SVM_timeGetTime() + (time * 1000);

	// Keep the message there even when the user task swaps.
	rv = g_display->drawSprite(&spriteInfo);
	if (rv)
		error("Driver Error %.8x (in DisplayMsg)", rv);

	sleepUntil(targetTime);

	g_display->setPalette(0, 256, (uint8 *) oldPal, RDPAL_FADE);
}

//
// Fades message down and removes it, fading up again afterwards
//

void Sword2Engine::removeMsg(void) {
	g_display->fadeDown();
	g_display->waitForFade();
	g_display->clearScene();

	// g_display->fadeUp();	
	// removed by JEL (08oct97) to prevent "eye" smacker corruption when
	// restarting game from CD2 and also to prevent palette flicker when
	// restoring game to a different CD since the "insert CD" message uses
	// this routine to clean up!
}

void Sword2Engine::sendBackPar0Frames(void) {
	// could be none at all - theoretically at least
	for (uint i = 0; i < _curBgp0; i++) {
		// frame attached to 1st background parallax
		processImage(&_bgp0List[i]);
	}
}

void Sword2Engine::sendBackPar1Frames(void) {
	// could be none at all - theoretically at least
	for (uint i = 0; i < _curBgp1; i++) {
		// frame attached to 2nd background parallax
		processImage(&_bgp1List[i]);
	}
}

void Sword2Engine::sendBackFrames(void) {
	// could be none at all - theoretically at least
	for (uint i = 0; i < _curBack; i++) {
		processImage(&_backList[i]);
	}
}

void Sword2Engine::sendSortFrames(void) {
	// send the sort frames for printing - layers, shrinkers & normal flat
	// sprites

	// could be none at all - theoretically at least
	for (uint i = 0; i < _curSort; i++) {
		if (_sortList[_sortOrder[i]].layer_number) {
			// its a layer  - minus 1 for true layer number
			// we need to know from the buildit because the layers
			// will have been sorted in random order
			processLayer(_sortList[_sortOrder[i]].layer_number - 1);
		} else {
			// sprite
			processImage(&_sortList[_sortOrder[i]]);
		}
	}
}

void Sword2Engine::sendForeFrames(void) {
	// could be none at all - theoretically at least
	for (uint i = 0; i < _curFore; i++) {
		processImage(&_foreList[i]);
	}
}

void Sword2Engine::sendForePar0Frames(void) {
	// could be none at all - theoretically at least
	for (uint i = 0; i < _curFgp0; i++) {
		// frame attached to 1st foreground parallax
		processImage(&_fgp0List[i]);
	}
}

void Sword2Engine::sendForePar1Frames(void) {
	// could be none at all - theoretically at least
	for (uint i = 0; i < _curFgp1; i++) {
		// frame attached to 2nd foreground parallax
		processImage(&_fgp1List[i]);
	}
}

void Sword2Engine::processLayer(uint32 layer_number) {
	uint8 *file;
	_layerHeader *layer_head;
 	_spriteInfo spriteInfo;
	uint32 rv;

	uint32 current_layer_area = 0;

	// file points to 1st byte in the layer file
	file = res_man->openResource(_thisScreen.background_layer_id);

	// point to layer header
	layer_head = fetchLayerHeader(file,layer_number);

	spriteInfo.x = layer_head->x;
	spriteInfo.y = layer_head->y;
	spriteInfo.w = layer_head->width;
	spriteInfo.scale = 0;
	spriteInfo.scaledWidth = 0;
	spriteInfo.scaledHeight = 0;
	spriteInfo.h = layer_head->height;
	spriteInfo.type = RDSPR_TRANS | RDSPR_RLE256FAST;
	spriteInfo.blend = 0;
	spriteInfo.data = file + sizeof(_standardHeader) + layer_head->offset;
	spriteInfo.colourTable = 0;

	// check for largest layer for debug info

	current_layer_area = layer_head->width * layer_head->height;

	if (current_layer_area > _largestLayerArea) {
		_largestLayerArea = current_layer_area;
		sprintf(_largestLayerInfo,
			"largest layer:  %s layer(%d) is %dx%d",
			fetchObjectName(_thisScreen.background_layer_id),
			layer_number, layer_head->width, layer_head->height);
	}

	rv = g_display->drawSprite(&spriteInfo);
	if (rv)
		error("Driver Error %.8x in Process_layer(%d)", rv, layer_number);

	res_man->closeResource(_thisScreen.background_layer_id);
}

void Sword2Engine::processImage(buildit *build_unit) {
	uint8 *file, *colTablePtr = NULL;
	_animHeader *anim_head;
	_frameHeader *frame_head;
	_cdtEntry *cdt_entry;
	_spriteInfo spriteInfo;
	uint32 spriteType;
	uint32 rv;

	uint32 current_sprite_area = 0;

	// open anim resource file & point to base
	file = res_man->openResource(build_unit->anim_resource);

	anim_head = fetchAnimHeader(file);
	cdt_entry = fetchCdtEntry(file, build_unit->anim_pc);
	frame_head = fetchFrameHeader(file, build_unit->anim_pc);

	// so that 0-colour is transparent
	spriteType = RDSPR_TRANS;

	if (anim_head->blend)
		spriteType |= RDSPR_BLEND;

	// if the frame is to be flipped (only really applicable to frames
	// using offsets)
	if (cdt_entry->frameType & FRAME_FLIPPED)
		spriteType |= RDSPR_FLIP;

	if (cdt_entry->frameType & FRAME_256_FAST) {
		// scaling, shading & blending don't work with RLE256FAST
		// but the same compression can be decompressed using the
		// RLE256 routines!

		// NOTE: If this restriction refers to drawSprite(), I don't
		// think we have it any more. But I'm not sure.

		if (build_unit->scale || anim_head->blend || build_unit->shadingFlag)
			spriteType |= RDSPR_RLE256;
		else
			spriteType |= RDSPR_RLE256FAST;
	} else {
		// what compression was used?
		switch (anim_head->runTimeComp) {
		case NONE:
			spriteType |= RDSPR_NOCOMPRESSION;
			break;
		case RLE256:
			spriteType |= RDSPR_RLE256;
			break;
		case RLE16:
			spriteType |= RDSPR_RLE16;
			// points to just after last cdt_entry, ie.
			// start of colour table
			colTablePtr = (uint8*) (anim_head + 1) + anim_head->noAnimFrames * sizeof(_cdtEntry);
			break;
		}
	}

	// if we want this frame to be affected by the shading mask,
	// add the status bit
	if (build_unit->shadingFlag)
		spriteType |= RDSPR_SHADOW;

	spriteInfo.x = build_unit->x;
	spriteInfo.y = build_unit->y;
	spriteInfo.w = frame_head->width;
	spriteInfo.h = frame_head->height;
	spriteInfo.scale = build_unit->scale;
	spriteInfo.scaledWidth = build_unit->scaled_width;
	spriteInfo.scaledHeight	= build_unit->scaled_height;
	spriteInfo.type = spriteType;
	spriteInfo.blend = anim_head->blend;
	// points to just after frame header, ie. start of sprite data
	spriteInfo.data = (uint8 *) (frame_head + 1);
	spriteInfo.colourTable	= colTablePtr;

	// check for largest layer for debug info

	current_sprite_area = frame_head->width * frame_head->height;

	if (current_sprite_area > _largestSpriteArea) {
		_largestSpriteArea = current_sprite_area;
		sprintf(_largestSpriteInfo,
			"largest sprite: %s frame(%d) is %dx%d",
			fetchObjectName(build_unit->anim_resource),
			build_unit->anim_pc,
			frame_head->width,
			frame_head->height);
	}

	if (SYSTEM_TESTING_ANIMS) {	// see anims.cpp
		// bring the anim into the visible screen
		// but leave extra pixel at edge for box
		if (spriteInfo.x + spriteInfo.scaledWidth >= 639)
			spriteInfo.x = 639 - spriteInfo.scaledWidth;

		if (spriteInfo.y + spriteInfo.scaledHeight >= 399)
			spriteInfo.y = 399 - spriteInfo.scaledHeight;

		if (spriteInfo.x < 1)
			spriteInfo.x = 1;

		if (spriteInfo.y < 1)
			spriteInfo.y = 1;

		// create box to surround sprite - just outside sprite box
		_debugger->_rectX1 = spriteInfo.x - 1;
		_debugger->_rectY1 = spriteInfo.y - 1;
		_debugger->_rectX2 = spriteInfo.x + spriteInfo.scaledWidth;
		_debugger->_rectY2 = spriteInfo.y + spriteInfo.scaledHeight;
	}

// #ifdef _SWORD2_DEBUG
//	if (frame_head->width <= 1) {
//		debug(5, "WARNING: 1-pixel-wide frame found in %s (%d)", fetchObjectName(build_unit->anim_resource), build_unit->anim_resource);
//	}
// #endif

	rv = g_display->drawSprite(&spriteInfo);
	if (rv)
		error("Driver Error %.8x with sprite %s (%d) in processImage",
			rv, fetchObjectName(build_unit->anim_resource),
			build_unit->anim_resource);

	// release the anim resource
	res_man->closeResource(build_unit->anim_resource);
}

void Sword2Engine::resetRenderLists(void) {
	// reset the sort lists - do this before a logic loop
	// takes into account the fact that the start of the list is pre-built
	// with the special sortable layers

	_curBgp0 = 0;
	_curBgp1 = 0;
	_curBack = 0;
	// beginning of sort list is setup with the special sort layers
	_curSort = _thisScreen.number_of_layers;
	_curFore = 0;
	_curFgp0 = 0;
	_curFgp1 = 0;

	if (_curSort) {
		// there are some layers - so rebuild the sort order
		// positioning
		for (uint i = 0; i < _curSort; i++)
			_sortOrder[i] = i;	//rebuild the order list
	}
}

void Sword2Engine::sortTheSortList(void) {
	// sort the list

	// cannot bubble sort 0 or 1 items!
	if (_curSort <= 1)
		return;

	for (uint i = 0; i < _curSort - 1; i++) {
		for (uint j = 0; j < _curSort - 1; j++) {
			if (_sortList[_sortOrder[j]].sort_y > _sortList[_sortOrder[j + 1]].sort_y) {
				SWAP(_sortOrder[j], _sortOrder[j + 1]);
			}
		}
	}
}

void Sword2Engine::registerFrame(int32 *params, buildit *build_unit) {
	// params: 0 pointer to mouse structure or NULL for no write to mouse
	//           list (non-zero means write sprite-shape to mouse list)
	//         1 pointer to graphic structure
	//         2 pointer to mega structure

	Object_mega *ob_mega;
	Object_graphic *ob_graph;
	Object_mouse *ob_mouse;
	uint8 *file;
	_frameHeader *frame_head;
	_animHeader *anim_head;
	_cdtEntry *cdt_entry;
	int scale = 0;

	// open animation file & set up the necessary pointers

	ob_graph = (Object_graphic *) params[1];

	assert(ob_graph->anim_resource);

	file = res_man->openResource(ob_graph->anim_resource);

	anim_head = fetchAnimHeader(file);
	cdt_entry = fetchCdtEntry(file, ob_graph->anim_pc);
	frame_head = fetchFrameHeader(file, ob_graph->anim_pc);

	// update player graphic details for on-screen debug info
	if (ID == CUR_PLAYER_ID) {
		_debugger->_playerGraphic.type = ob_graph->type;
		_debugger->_playerGraphic.anim_resource = ob_graph->anim_resource;
		// counting 1st frame as 'frame 1'
		_debugger->_playerGraphic.anim_pc = ob_graph->anim_pc + 1;
		_debugger->_playerGraphicNoFrames = anim_head->noAnimFrames;
	}

	// fill in the buildit structure for this frame

	// retrieve the resource
 	build_unit->anim_resource = ob_graph->anim_resource;
	// retrieve the frame
	build_unit->anim_pc = ob_graph->anim_pc;
	// not a layer
	build_unit->layer_number = 0;

	// Affected by shading mask?
	if (ob_graph->type & SHADED_SPRITE)
		build_unit->shadingFlag = true;
	else
		build_unit->shadingFlag = false;

	// check if this frame has offsets ie. this is a scalable mega frame

	if (cdt_entry->frameType & FRAME_OFFSET) {
		// param 2 is pointer to mega structure
		ob_mega = (Object_mega *) params[2];

		// calc scale at which to print the sprite, based on feet
		// y-coord & scaling constants (NB. 'scale' is actually
		// 256 * true_scale, to maintain accuracy)

		// Ay+B gives 256 * scale ie. 256 * 256 * true_scale for even
		// better accuracy, ie. scale = (Ay + B) / 256
		scale = (ob_mega->scale_a * ob_mega->feet_y + ob_mega->scale_b) / 256;

		// calc final render coordinates (top-left of sprite), based
		// on feet coords & scaled offsets

		// add scaled offsets to feet coords
		build_unit->x = ob_mega->feet_x + (cdt_entry->x * scale) / 256;
		build_unit->y = ob_mega->feet_y + (cdt_entry->y * scale) / 256;

		// work out new width and height
		// always divide by 256 after everything else, to maintain
		// accurary
		build_unit->scaled_width = ((scale * frame_head->width) / 256);
		build_unit->scaled_height = ((scale * frame_head->height) / 256);
	} else {
		// it's a non-scaling anim
		// get render coords for sprite, from cdt
		build_unit->x = cdt_entry->x;
		build_unit->y = cdt_entry->y;

 		// get width and height
		build_unit->scaled_width = frame_head->width;
		build_unit->scaled_height = frame_head->height;
	}

	// either 0 or required scale, depending on whether 'scale' computed
	build_unit->scale = scale;

	// calc the bottom y-coord for sorting purposes
	build_unit->sort_y = build_unit->y + build_unit->scaled_height - 1;

	if (params[0]) {
		// passed a mouse structure, so add to the _mouseList
		ob_mouse = (Object_mouse *) params[0];

		// only if 'pointer' isn't NULL
		if (ob_mouse->pointer) {
			assert(_curMouse < TOTAL_mouse_list);

			_mouseList[_curMouse].x1 = build_unit->x;
			_mouseList[_curMouse].y1 = build_unit->y;
			_mouseList[_curMouse].x2 = build_unit->x + build_unit->scaled_width;
			_mouseList[_curMouse].y2 = build_unit->y + build_unit->scaled_height;

 			_mouseList[_curMouse].priority = ob_mouse->priority;
			_mouseList[_curMouse].pointer = ob_mouse->pointer;

			// check if pointer text field is set due to previous
			// object using this slot (ie. not correct for this
			// one)

			// if 'pointer_text' field is set, but the 'id' field
			// isn't same is current id
			// then we don't want this "left over" pointer text

			if (_mouseList[_curMouse].pointer_text && _mouseList[_curMouse].id != (int32) ID)
				_mouseList[_curMouse].pointer_text=0;

			_mouseList[_curMouse].id = ID;
			// not using sprite as detection mask
			_mouseList[_curMouse].anim_resource = 0;
			_mouseList[_curMouse].anim_pc = 0;

			_curMouse++;
		}
	}

	// close animation file
	res_man->closeResource(ob_graph->anim_resource);
}

int32 Logic::fnRegisterFrame(int32 *params) {
	// this call would be made from an objects service script 0

	// params:	0 pointer to mouse structure or NULL for no write to
	//		  mouse list (non-zero means write sprite-shape to
	//		  mouse list)
	//		1 pointer to graphic structure
	//		2 pointer to mega structure or NULL if not a mega

	return g_sword2->registerFrame(params);
}

int32 Sword2Engine::registerFrame(int32 *params) {
	Object_graphic *ob_graph = (Object_graphic *) params[1];

	// check low word for sprite type
	switch (ob_graph->type & 0x0000ffff) {
	case BGP0_SPRITE:
		assert(_curBgp0 < MAX_bgp0_sprites);
		registerFrame(params, &_bgp0List[_curBgp0]);
		_curBgp0++;
		break;
	case BGP1_SPRITE:
		assert(_curBgp1 < MAX_bgp1_sprites);
		registerFrame(params, &_bgp1List[_curBgp1]);
		_curBgp1++;
		break;
	case BACK_SPRITE:
		assert(_curBack < MAX_back_sprites);
		registerFrame(params, &_backList[_curBack]);
		_curBack++;
		break;
	case SORT_SPRITE:
		assert(_curSort < MAX_sort_sprites);
		_sortOrder[_curSort] = _curSort;
		registerFrame(params, &_sortList[_curSort]);
		_curSort++;
		break;
	case FORE_SPRITE:
		assert(_curFore < MAX_fore_sprites);
		registerFrame(params, &_foreList[_curFore]);
		_curFore++;
		break;
	case FGP0_SPRITE:
		assert(_curFgp0 < MAX_fgp0_sprites);
		registerFrame(params, &_fgp0List[_curFgp0]);
		_curFgp0++;
		break;
	case FGP1_SPRITE:
		assert(_curFgp1 < MAX_fgp1_sprites);
		registerFrame(params, &_fgp1List[_curFgp1]);
		_curFgp1++;
		break;
	default:
		// NO_SPRITE no registering!
		break;
	}

	return IR_CONT;
}

void Sword2Engine::startNewPalette(void) {
	// start layer palette fading up

	uint8 *screenFile;

	// if the screen is still fading down then wait for black - could
	// happen when everythings cached into a large memory model
	g_display->waitForFade();

	// open the screen file
	screenFile = res_man->openResource(_thisScreen.background_layer_id);

	g_display->updatePaletteMatchTable((uint8 *) fetchPaletteMatchTable(screenFile));

	g_display->setPalette(0, 256, fetchPalette(screenFile), RDPAL_FADE);

	// indicating that it's a screen palette
	_lastPaletteRes = 0;

	// close screen file
  	res_man->closeResource(_thisScreen.background_layer_id);

	// start fade up
	g_display->fadeUp();

	// reset
 	_thisScreen.new_palette = 0;
}

int32 Logic::fnUpdatePlayerStats(int32 *params) {
	// engine needs to know certain info about the player

	// params:	0 pointer to mega structure

	Object_mega *ob_mega = (Object_mega *) params[0];

	g_sword2->_thisScreen.player_feet_x = ob_mega->feet_x;
	g_sword2->_thisScreen.player_feet_y = ob_mega->feet_y;

	// for the script
	PLAYER_FEET_X = ob_mega->feet_x;
	PLAYER_FEET_Y = ob_mega->feet_y;
	PLAYER_CUR_DIR = ob_mega->current_dir;

	SCROLL_OFFSET_X = g_sword2->_thisScreen.scroll_offset_x;

	debug(5, "fnUpdatePlayerStats: %d %d", ob_mega->feet_x, ob_mega->feet_y);

	return IR_CONT;
}

int32 Logic::fnFadeDown(int32 *params) {
	// NONE means up! can only be called when screen is fully faded up -
	// multiple calls wont have strange effects

	// params:	none

	if (g_display->getFadeStatus() == RDFADE_NONE)
		g_display->fadeDown();

	return IR_CONT;
}

int32 Logic::fnFadeUp(int32 *params) {
	// params:	none

	g_display->waitForFade();

	if (g_display->getFadeStatus() == RDFADE_BLACK)
		g_display->fadeUp();

	return IR_CONT;
}

int32 Logic::fnSetPalette(int32 *params) {
	// params:	0 resource number of palette file, or 0 if it's to be
	//		  the palette from the current screen

	g_sword2->setFullPalette(params[0]);
	return IR_CONT;
}

void Sword2Engine::setFullPalette(int32 palRes) {
	uint8 *file;
	_standardHeader *head;

	// fudge for hut interior
	// - unpausing should restore last palette as normal (could be screen
	// palette or 'dark_palette_13')
	// - but restoring the screen palette after 'dark_plaette_13' should
	// now work properly too!

	// hut interior
	if (LOCATION == 13) {
		// unpausing
		if (palRes == -1) {
			// restore whatever palette was last set (screen
			// palette or 'dark_palette_13')
			palRes = _lastPaletteRes;
		}
	} else {
		// check if we're just restoring the current screen palette
		// because we might actually need to use a separate palette
		// file anyway eg. for pausing & unpausing during the eclipse

		// unpausing (fudged for location 13)
 		if (palRes == -1) {
			// we really meant '0'
			palRes = 0;
		}

		if (palRes == 0 && _lastPaletteRes)
			palRes = _lastPaletteRes;
	}

	// non-zero: set palette to this separate palette file
	if (palRes) {
		// open the palette file
		head = (_standardHeader *) res_man->openResource(palRes);

		assert(head->fileType == PALETTE_FILE);

		file = (uint8 *) (head + 1);

		// always set colour 0 to black because most background screen
		// palettes have a bright colour 0 although it should come out
		// as black in the game!

		file[0] = 0;
		file[1] = 0;
		file[2] = 0;
		file[3] = 0;

		// not yet in separate palette files
		// g_display->updatePaletteMatchTable(file + (256 * 4));

		g_display->setPalette(0, 256, file, RDPAL_INSTANT);

		if (palRes != CONTROL_PANEL_PALETTE) {	// (James 03sep97)
			// indicating that it's a separate palette resource
			_lastPaletteRes = palRes;
		}

		// close palette file
	  	res_man->closeResource(palRes);
	} else {
		// 0: set palette to current screen palette
		if (_thisScreen.background_layer_id) {
			// open the screen file
			file = res_man->openResource(_thisScreen.background_layer_id);
			g_display->updatePaletteMatchTable((uint8 *) fetchPaletteMatchTable(file));

			g_display->setPalette(0, 256, fetchPalette(file), RDPAL_INSTANT);

			// indicating that it's a screen palette
			_lastPaletteRes = 0;

			// close screen file
	  		res_man->closeResource(_thisScreen.background_layer_id);
		} else
			error("setFullPalette(0) called, but no current screen available!");
	}
}

int32 Logic::fnRestoreGame(int32 *params) {
	// params:	none
	return IR_CONT;
}

int32 Logic::fnChangeShadows(int32 *params) {
	// params:	none

	// if last screen was using a shading mask (see below)
	if (g_sword2->_thisScreen.mask_flag) {
		uint32 rv = g_display->closeLightMask();

		if (rv)
			error("Driver Error %.8x [%s line %u]", rv);

		g_sword2->_thisScreen.mask_flag = 0;
	}

	return IR_CONT;
}

} // End of namespace Sword2
