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

// ---------------------------------------------------------------------------

buildit	bgp0_list[MAX_bgp0_sprites];
buildit	bgp1_list[MAX_bgp1_sprites];
buildit	back_list[MAX_back_sprites];
buildit	sort_list[MAX_sort_sprites];
buildit	fore_list[MAX_fore_sprites];
buildit	fgp0_list[MAX_fgp0_sprites];
buildit	fgp1_list[MAX_fgp1_sprites];

// holds the order of the sort list
// i.e the list stays static and we sort this

uint16 sort_order[MAX_sort_sprites];

uint32 cur_bgp0;
uint32 cur_bgp1;
uint32 cur_back;
uint32 cur_sort;
uint32 cur_fore;
uint32 cur_fgp0;
uint32 cur_fgp1;

uint32 largest_layer_area = 0;	// should be reset to zero at start of each screen change
uint32 largest_sprite_area = 0;	// - " -
char largest_layer_info[128]	= { "largest layer:  none registered" };
char largest_sprite_info[128]	= { "largest sprite: none registered" };

// ---------------------------------------------------------------------------
// last palette used - so that we can restore the correct one after a pause
// (which dims the screen) and it's not always the main screen palette that we
// want, eg. during the eclipse

// This flag gets set in Start_new_palette() and SetFullPalette()

uint32 lastPaletteRes = 0;

// ---------------------------------------------------------------------------
// 'frames per second' counting stuff

uint32 fps = 0;
uint32 cycleTime = 0;
uint32 frameCount = 0;
// So I know if the control Panel can be activated - CJR 1-5-97
extern uint32 mouse_status;

// ---------------------------------------------------------------------------
// function prototypes not needed externally

void Start_new_palette(void);

void Register_frame(int32 *params, buildit *build_unit);
void Process_layer(uint32 layer_number);
void Sort_the_sort_list(void);

void Send_back_par0_frames(void);
void Send_back_par1_frames(void);
void Send_back_frames(void);
void Send_sort_frames(void);
void Send_fore_frames(void);
void Send_fore_par0_frames(void);
void Send_fore_par1_frames(void);

// ---------------------------------------------------------------------------
//
// PC Build_display
//
// ---------------------------------------------------------------------------

void Build_display(void) {
	uint8 *file;
	_multiScreenHeader *screenLayerTable;

	if (this_screen.new_palette) {
		// start the layer palette fading up
		Start_new_palette();

		largest_layer_area = 0;		// should be reset to zero at start of each screen change
		largest_sprite_area = 0;	// - " -
	}

	// there is a valid screen to run
	if (this_screen.background_layer_id) {
		// set the scroll position
		g_display->setScrollTarget(this_screen.scroll_offset_x, this_screen.scroll_offset_y);
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
			file = res_man.open(this_screen.background_layer_id);
			screenLayerTable = (_multiScreenHeader *) ((uint8 *) file + sizeof(_standardHeader));

			if (screenLayerTable->bg_parallax[0]) {
				g_display->renderParallax(FetchBackgroundParallaxLayer(file, 0), 0);
				// release the screen resource before cacheing
				// the sprites
	 			res_man.close(this_screen.background_layer_id);
				Send_back_par0_frames();
			} else {
				// release the screen resource
 	 			res_man.close(this_screen.background_layer_id);
			}

			// second background parallax + related anims

			// open the screen resource
			file = res_man.open(this_screen.background_layer_id);
			screenLayerTable = (_multiScreenHeader *) ((uint8 *) file + sizeof(_standardHeader));

			if (screenLayerTable->bg_parallax[1]) {
				g_display->renderParallax(FetchBackgroundParallaxLayer(file, 1), 1);
				// release the screen resource before cacheing
				// the sprites
	 			res_man.close(this_screen.background_layer_id);
				Send_back_par1_frames();
			} else {
				// release the screen resource
 	 			res_man.close(this_screen.background_layer_id);
			}

			// normal backround layer (just the one!)

			// open the screen resource
			file = res_man.open(this_screen.background_layer_id);
			g_display->renderParallax(FetchBackgroundLayer(file), 2);
			// release the screen resource
			res_man.close(this_screen.background_layer_id);

			// sprites & layers

			Send_back_frames();	// background sprites
			Sort_the_sort_list();
			Send_sort_frames();	// sorted sprites & layers
			Send_fore_frames();	// foreground sprites

			// first foreground parallax + related anims

			// open the screen resource
			file = res_man.open(this_screen.background_layer_id);
			screenLayerTable = (_multiScreenHeader *) ((uint8 *) file + sizeof(_standardHeader));

			if (screenLayerTable->fg_parallax[0]) {
				g_display->renderParallax(FetchForegroundParallaxLayer(file, 0), 3);
				// release the screen resource before cacheing
				// the sprites
	 			res_man.close(this_screen.background_layer_id);
				Send_fore_par0_frames();
			} else {
				// release the screen resource
 	 			res_man.close(this_screen.background_layer_id);
			}

			// second foreground parallax + related anims

			// open the screen resource
			file = res_man.open(this_screen.background_layer_id);
			screenLayerTable = (_multiScreenHeader *) ((uint8 *) file + sizeof(_standardHeader));

			if (screenLayerTable->fg_parallax[1]) {
				g_display->renderParallax(FetchForegroundParallaxLayer(file, 1), 4);
				// release the screen resource before cacheing
				// the sprites
	 			res_man.close(this_screen.background_layer_id);
				Send_fore_par1_frames();
			} else {
				// release the screen resource
 	 			res_man.close(this_screen.background_layer_id);
			}

			// walkgrid, mouse & player markers & mouse area
			// rectangle

			Draw_debug_graphics();

			// text blocks

			// speech blocks and headup debug text
			fontRenderer.printTextBlocs();

			// menu bar & icons

			g_display->processMenu();

			// ready - blit to screen

			g_display->updateDisplay();

			// update our fps reading

			frameCount++;
			if (SVM_timeGetTime() > cycleTime) {
				fps = frameCount;
				debug(2, "FPS: %d", fps);
				frameCount = 0;
				cycleTime = SVM_timeGetTime() + 1000;
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

// ---------------------------------------------------------------------------
//
// Fades down and displays a message on the screen for time seconds
//

void DisplayMsg(uint8 *text, int time) {
	mem *text_spr;
	_frameHeader *frame;
	_spriteInfo spriteInfo;
	_palEntry pal[256];
	_palEntry oldPal[256];
	uint32 rv;	// drivers error return value

	warning("DisplayMsg: %s", (char *) text);
	
	if (g_display->getFadeStatus() != RDFADE_BLACK) {
		g_display->fadeDown();
		g_display->waitForFade();
	}

	Set_mouse(0);
	Set_luggage(0);

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

	memory.freeMemory(text_spr);

	g_display->waitForFade();

	uint32 targetTime = SVM_timeGetTime() + (time * 1000);

	// Keep the message there even when the user task swaps.
	rv = g_display->drawSprite(&spriteInfo);
	if (rv)
		error("Driver Error %.8x (in DisplayMsg)", rv);

	sleepUntil(targetTime);

	g_display->setPalette(0, 256, (uint8 *) oldPal, RDPAL_FADE);
}

// ---------------------------------------------------------------------------
//
// Fades message down and removes it, fading up again afterwards
//

void RemoveMsg(void) {
	g_display->fadeDown();

	g_display->waitForFade();

	g_display->clearScene();

	// g_display->fadeUp();	
	// removed by JEL (08oct97) to prevent "eye" smacker corruption when
	// restarting game from CD2 and also to prevent palette flicker when
	// restoring game to a different CD since the "insert CD" message uses
	// this routine to clean up!
}

void Send_back_par0_frames(void) {
	// could be none at all - theoretically at least
	for (uint i = 0; i < cur_bgp0; i++) {
		// frame attached to 1st background parallax
		Process_image(&bgp0_list[i]);
	}
}

void Send_back_par1_frames(void) {
	// could be none at all - theoretically at least
	for (uint i = 0; i < cur_bgp1; i++) {
		// frame attached to 2nd background parallax
		Process_image(&bgp1_list[i]);
	}
}

void Send_back_frames(void) {
	// could be none at all - theoretically at least
	for (uint i = 0; i < cur_back; i++) {
		Process_image(&back_list[i]);
	}
}

void Send_sort_frames(void) {
	// send the sort frames for printing - layers, shrinkers & normal flat
	// sprites

	// could be none at all - theoretically at least
	for (uint i = 0; i < cur_sort; i++) {
		if (sort_list[sort_order[i]].layer_number) {
			// its a layer  - minus 1 for true layer number
			// we need to know from the buildit because the layers
			// will have been sorted in random order
			Process_layer(sort_list[sort_order[i]].layer_number - 1);
		} else {
			// sprite
			Process_image(&sort_list[sort_order[i]]);
		}
	}
}

void Send_fore_frames(void) {
	// could be none at all - theoretically at least
	for (uint i = 0; i < cur_fore; i++) {
		Process_image(&fore_list[i]);
	}
}

void Send_fore_par0_frames(void) {
	// could be none at all - theoretically at least
	for (uint i = 0; i < cur_fgp0; i++) {
		// frame attached to 1st foreground parallax
		Process_image(&fgp0_list[i]);
	}
}

void Send_fore_par1_frames(void) {
	// could be none at all - theoretically at least
	for (uint i = 0; i < cur_fgp1; i++) {
		// frame attached to 2nd foreground parallax
		Process_image(&fgp1_list[i]);
	}
}

void Process_layer(uint32 layer_number) {
	uint8 *file;
	_layerHeader *layer_head;
 	_spriteInfo spriteInfo;
	uint32 rv;

	uint32 current_layer_area = 0;

	// file points to 1st byte in the layer file
	file = res_man.open(this_screen.background_layer_id);

	// point to layer header
	layer_head = FetchLayerHeader(file,layer_number);

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


	//------------------------------------------
	// check for largest layer for debug info

	current_layer_area = layer_head->width * layer_head->height;

	if (current_layer_area > largest_layer_area) {
		largest_layer_area = current_layer_area;
		sprintf(largest_layer_info,
			"largest layer:  %s layer(%d) is %dx%d",
			FetchObjectName(this_screen.background_layer_id),
			layer_number, layer_head->width, layer_head->height);
	}

	//------------------------------------------

	rv = g_display->drawSprite(&spriteInfo);
	if (rv)
		error("Driver Error %.8x in Process_layer(%d)", rv, layer_number);

	res_man.close(this_screen.background_layer_id);
}

void Process_image(buildit *build_unit) {
	uint8 *file, *colTablePtr = NULL;
	_animHeader *anim_head;
	_frameHeader *frame_head;
	_cdtEntry *cdt_entry;
	_spriteInfo spriteInfo;
	uint32 spriteType;
	uint32 rv;

	uint32 current_sprite_area = 0;

	// open anim resource file & point to base
	file = res_man.open(build_unit->anim_resource);

	anim_head = FetchAnimHeader(file);
	cdt_entry = FetchCdtEntry(file, build_unit->anim_pc);
	frame_head = FetchFrameHeader(file, build_unit->anim_pc);

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
	if (build_unit->shadingFlag == 1)
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

	//------------------------------------------
	// check for largest layer for debug info

	current_sprite_area = frame_head->width * frame_head->height;

	if (current_sprite_area > largest_sprite_area) {
		largest_sprite_area = current_sprite_area;
		sprintf(largest_sprite_info,
			"largest sprite: %s frame(%d) is %dx%d",
			FetchObjectName(build_unit->anim_resource),
			build_unit->anim_pc,
			frame_head->width,
			frame_head->height);
	}

#ifdef _SWORD2_DEBUG
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
		rect_x1 = spriteInfo.x - 1;
		rect_y1 = spriteInfo.y - 1;
		rect_x2 = spriteInfo.x + spriteInfo.scaledWidth;
		rect_y2 = spriteInfo.y + spriteInfo.scaledHeight;
	}
#endif

// #ifdef _SWORD2_DEBUG
//	if (frame_head->width <= 1) {
//		debug(5, "WARNING: 1-pixel-wide frame found in %s (%d)", FetchObjectName(build_unit->anim_resource), build_unit->anim_resource);
//	}
// #endif

	rv = g_display->drawSprite(&spriteInfo);
	if (rv)
		error("Driver Error %.8x with sprite %s (%d) in Process_image",
			rv, FetchObjectName(build_unit->anim_resource),
			build_unit->anim_resource);

	// release the anim resource
	res_man.close(build_unit->anim_resource);
}

void Reset_render_lists(void) {
	// reset the sort lists - do this before a logic loop
	// takes into account the fact that the start of the list is pre-built
	// with the special sortable layers

	cur_bgp0 = 0;
	cur_bgp1 = 0;
	cur_back = 0;
	// beginning of sort list is setup with the special sort layers
	cur_sort = this_screen.number_of_layers;
	cur_fore = 0;
	cur_fgp0 = 0;
	cur_fgp1 = 0;

	if (cur_sort) {
		// there are some layers - so rebuild the sort order
		// positioning
		for (uint i = 0; i < cur_sort; i++)
			sort_order[i] = i;	//rebuild the order list
	}
}

void Sort_the_sort_list(void) {
	//sort the list

	//cannot bubble sort 0 or 1 items!
	if (cur_sort <= 1)
		return;

	for (uint i = 0; i < cur_sort - 1; i++) {
		for (uint j = 0; j < cur_sort - 1; j++) {
			if (sort_list[sort_order[j]].sort_y > sort_list[sort_order[j + 1]].sort_y) {
				SWAP(sort_order[j], sort_order[j + 1]);
			}
		}
	}
}

void Register_frame(int32 *params, buildit *build_unit)	{
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

	//-------------------------------------------	
	// open animation file & set up the necessary pointers

	ob_graph = (Object_graphic *) params[1];

#ifdef _SWORD2_DEBUG
	if (ob_graph->anim_resource == 0)
		error("ERROR: %s(%d) has no anim resource in Register_frame", FetchObjectName(ID), ID);
#endif

	file = res_man.open(ob_graph->anim_resource);

	anim_head = FetchAnimHeader(file);
	cdt_entry = FetchCdtEntry(file, ob_graph->anim_pc);
	frame_head = FetchFrameHeader(file, ob_graph->anim_pc);

#ifdef _SWORD2_DEBUG
	// update player graphic details for on-screen debug info
	if (ID == CUR_PLAYER_ID) {
		playerGraphic.type = ob_graph->type;
		playerGraphic.anim_resource = ob_graph->anim_resource;
		// counting 1st frame as 'frame 1'
		playerGraphic.anim_pc = ob_graph->anim_pc + 1;
		player_graphic_no_frames = anim_head->noAnimFrames;
	}
#endif

	//-------------------------------------------	
	// fill in the buildit structure for this frame

	//retrieve the resource
 	build_unit->anim_resource = ob_graph->anim_resource;
	//retrieve the frame
	build_unit->anim_pc = ob_graph->anim_pc;
	//not a layer
	build_unit->layer_number = 0;

	// Affected by shading mask?
	if (ob_graph->type & SHADED_SPRITE)
		build_unit->shadingFlag = 1;
	else
		build_unit->shadingFlag = 0;

	//-------------------------------------------	
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
		// passed a mouse structure, so add to the mouse_list
		ob_mouse = (Object_mouse *) params[0];

		// only if 'pointer' isn't NULL
		if (ob_mouse->pointer) {
#ifdef _SWORD2_DEBUG
			if (cur_mouse == TOTAL_mouse_list)
				error("ERROR: mouse_list full");
#endif

			mouse_list[cur_mouse].x1 = build_unit->x;
			mouse_list[cur_mouse].y1 = build_unit->y;
			mouse_list[cur_mouse].x2 = build_unit->x + build_unit->scaled_width;
			mouse_list[cur_mouse].y2 = build_unit->y + build_unit->scaled_height;

 			mouse_list[cur_mouse].priority = ob_mouse->priority;
			mouse_list[cur_mouse].pointer = ob_mouse->pointer;

			// check if pointer text field is set due to previous
			// object using this slot (ie. not correct for this
			// one)

			// if 'pointer_text' field is set, but the 'id' field
			// isn't same is current id
			// then we don't want this "left over" pointer text

			if (mouse_list[cur_mouse].pointer_text && mouse_list[cur_mouse].id != (int32) ID)
				mouse_list[cur_mouse].pointer_text=0;

			mouse_list[cur_mouse].id = ID;
			// not using sprite as detection mask
			mouse_list[cur_mouse].anim_resource = 0;
			mouse_list[cur_mouse].anim_pc = 0;

			cur_mouse++;
		}
	}

	// close animation file
	res_man.close(ob_graph->anim_resource);
}

int32 Logic::fnRegisterFrame(int32 *params) {
	// this call would be made from an objects service script 0

	// params:	0 pointer to mouse structure or NULL for no write to
	//		  mouse list (non-zero means write sprite-shape to
	//		  mouse list)
	//		1 pointer to graphic structure
	//		2 pointer to mega structure or NULL if not a mega

	Object_graphic *ob_graph = (Object_graphic *) params[1];

	// check low word for sprite type
	switch (ob_graph->type & 0x0000ffff) {
	case BGP0_SPRITE:
#ifdef _SWORD2_DEBUG
		if (cur_bgp0 == MAX_bgp0_sprites)
			error("ERROR: bgp0_list full in fnRegisterFrame");
#endif

		Register_frame(params, &bgp0_list[cur_bgp0]);
		cur_bgp0++;
		break;
	case BGP1_SPRITE:
#ifdef _SWORD2_DEBUG
		if (cur_bgp1 == MAX_bgp1_sprites)
			error("ERROR: bgp1_list full in fnRegisterFrame");
#endif

		Register_frame(params, &bgp1_list[cur_bgp1]);
		cur_bgp1++;
		break;
	case BACK_SPRITE:
#ifdef _SWORD2_DEBUG
		if (cur_back == MAX_back_sprites)
			error("ERROR: back_list full in fnRegisterFrame");
#endif

		Register_frame(params, &back_list[cur_back]);
		cur_back++;
		break;
	case SORT_SPRITE:
#ifdef _SWORD2_DEBUG
		if (cur_sort == MAX_sort_sprites)
			error("ERROR: sort_list full in fnRegisterFrame");
#endif

		sort_order[cur_sort] = cur_sort;
		Register_frame(params, &sort_list[cur_sort]);
		cur_sort++;
		break;
	case FORE_SPRITE:
#ifdef _SWORD2_DEBUG
		if (cur_fore == MAX_fore_sprites)
			error("ERROR: fore_list full in fnRegisterFrame");
#endif

		Register_frame(params, &fore_list[cur_fore]);
		cur_fore++;
		break;
	case FGP0_SPRITE:
#ifdef _SWORD2_DEBUG
		if (cur_fgp0 == MAX_fgp0_sprites)
			error("ERROR: fgp0_list full in fnRegisterFrame");
#endif

		Register_frame(params, &fgp0_list[cur_fgp0]);
		cur_fgp0++;
		break;
	case FGP1_SPRITE:
#ifdef _SWORD2_DEBUG
		if (cur_fgp1 == MAX_fgp1_sprites)
			error("ERROR: fgp1_list full in fnRegisterFrame");
#endif

		Register_frame(params, &fgp1_list[cur_fgp1]);
		cur_fgp1++;
		break;
	default:
		// NO_SPRITE no registering!
		break;
	}

	return IR_CONT;
}

void Start_new_palette(void) {
	//start layer palette fading up

	uint8 *screenFile;

	// if the screen is still fading down then wait for black - could
	// happen when everythings cached into a large memory model
	g_display->waitForFade();

	// open the screen file
	screenFile = res_man.open(this_screen.background_layer_id);

	g_display->updatePaletteMatchTable((uint8 *) FetchPaletteMatchTable(screenFile));

	g_display->setPalette(0, 256, FetchPalette(screenFile), RDPAL_FADE);

	// indicating that it's a screen palette
	lastPaletteRes = 0;

	// close screen file
  	res_man.close(this_screen.background_layer_id);

	// start fade up
	g_display->fadeUp();

	// reset
 	this_screen.new_palette = 0;
}

int32 Logic::fnUpdatePlayerStats(int32 *params) {
	// engine needs to know certain info about the player

	// params:	0 pointer to mega structure

	Object_mega *ob_mega = (Object_mega *) params[0];

	this_screen.player_feet_x = ob_mega->feet_x;
	this_screen.player_feet_y = ob_mega->feet_y;

	// for the script
	PLAYER_FEET_X = ob_mega->feet_x;
	PLAYER_FEET_Y = ob_mega->feet_y;
	PLAYER_CUR_DIR = ob_mega->current_dir;

	SCROLL_OFFSET_X = this_screen.scroll_offset_x;

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

// ---------------------------------------------------------------------------
// typedef struct {
//	uint8 red;
//	uint8 green;
//	uint8 blue;
//	uint8 alpha;
// } _palEntry;

// ---------------------------------------------------------------------------
// typedef struct {
//	// first colour number in this palette (0..255)
//	uint8 firstEntry;
//	// number of Entries-1 (0..255) to be taken as (1..256)
//	uint8 noEntries;
// } _paletteHeader;

int32 Logic::fnSetPalette(int32 *params) {
	// params:	0 resource number of palette file, or 0 if it's to be
	//		  the palette from the current screen
	
	SetFullPalette(params[0]);
	return IR_CONT;
}

void SetFullPalette(int32 palRes) {
	uint8 *file;
	_standardHeader *head;

	//----------------------------------
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
			palRes = lastPaletteRes;
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

		if (palRes == 0 && lastPaletteRes)
			palRes = lastPaletteRes;
	}
	//----------------------------------

	// non-zero: set palette to this separate palette file
	if (palRes) {
		// open the palette file
		head = (_standardHeader *) res_man.open(palRes);

#ifdef _SWORD2_DEBUG
		if (head->fileType != PALETTE_FILE)
 			error("fnSetPalette() called with invalid resource!");
#endif

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
			lastPaletteRes = palRes;
		}

		// close palette file
	  	res_man.close(palRes);
	} else {
		// 0: set palette to current screen palette
		if (this_screen.background_layer_id) {
			// open the screen file
			file = res_man.open(this_screen.background_layer_id);
			g_display->updatePaletteMatchTable((uint8 *) FetchPaletteMatchTable(file));

			g_display->setPalette(0, 256, FetchPalette(file), RDPAL_INSTANT);

			// indicating that it's a screen palette
			lastPaletteRes = 0;

			// close screen file
	  		res_man.close(this_screen.background_layer_id);
		} else
			error("fnSetPalette(0) called, but no current screen available!");
	}
}

int32 Logic::fnRestoreGame(int32 *params) {
	// params:	none
	return IR_CONT;
}

int32 Logic::fnChangeShadows(int32 *params) {
	// params:	none

	uint32 rv;

	// if last screen was using a shading mask (see below)
	if (this_screen.mask_flag) {
		rv = g_display->closeLightMask();

		if (rv)
			error("Driver Error %.8x [%s line %u]", rv);

		this_screen.mask_flag = 0;
	}

	return IR_CONT;
}

} // End of namespace Sword2
