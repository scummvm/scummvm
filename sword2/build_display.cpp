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

// ---------------------------------------------------------------------------
// BUILD_DISPLAY.CPP	like the old spr_engi but slightly more aptly named
// ---------------------------------------------------------------------------

#include "common/stdafx.h"
#include "sword2/sword2.h"
#include "sword2/console.h"
#include "sword2/defs.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/resman.h"
#include "sword2/driver/d_draw.h"

namespace Sword2 {

void Sword2Engine::buildDisplay(void) {
	if (_thisScreen.new_palette) {
		// start the layer palette fading up
		startNewPalette();

		// should be reset to zero at start of each screen change
		_largestLayerArea = 0;
		_largestSpriteArea = 0;
	}

	// Does this ever happen?
	if (!_thisScreen.background_layer_id)
		return;

	// there is a valid screen to run

	_graphics->setScrollTarget(_thisScreen.scroll_offset_x, _thisScreen.scroll_offset_y);
	_graphics->animateMouse();
	_graphics->startRenderCycle();

	byte *file = _resman->openResource(_thisScreen.background_layer_id);
	MultiScreenHeader *screenLayerTable = (MultiScreenHeader *) (file + sizeof(StandardHeader));

	// Render at least one frame, but if the screen is scrolling, and if
	// there is time left, we will render extra frames to smooth out the
	// scrolling.

	do {
		// first background parallax + related anims
		if (screenLayerTable->bg_parallax[0]) {
			_graphics->renderParallax(fetchBackgroundParallaxLayer(file, 0), 0);
			drawBackPar0Frames();
		}

		// second background parallax + related anims
		if (screenLayerTable->bg_parallax[1]) {
			_graphics->renderParallax(fetchBackgroundParallaxLayer(file, 1), 1);
			drawBackPar1Frames();
		}

		// normal backround layer (just the one!)
		_graphics->renderParallax(fetchBackgroundLayer(file), 2);

		// sprites & layers
		drawBackFrames();	// background sprites
		drawSortFrames(file);	// sorted sprites & layers
		drawForeFrames();	// foreground sprites

		// first foreground parallax + related anims

		if (screenLayerTable->fg_parallax[0]) {
			_graphics->renderParallax(fetchForegroundParallaxLayer(file, 0), 3);
			drawForePar0Frames();
		}

		// second foreground parallax + related anims

		if (screenLayerTable->fg_parallax[1]) {
			_graphics->renderParallax(fetchForegroundParallaxLayer(file, 1), 4);
			drawForePar1Frames();
		}

		_debugger->drawDebugGraphics();
		_fontRenderer->printTextBlocs();
		_graphics->processMenu();

		_graphics->updateDisplay();

		_frameCount++;
		if (_system->get_msecs() > _cycleTime) {
			_fps = _frameCount;
			_frameCount = 0;
			_cycleTime = _system->get_msecs() + 1000;
		}
	} while (!_graphics->endRenderCycle());

	_resman->closeResource(_thisScreen.background_layer_id);
}

/**
 * Fades down and displays a message on the screen.
 * @param text The message
 * @param time The number of seconds to display the message
 */

void Sword2Engine::displayMsg(byte *text, int time) {
	PalEntry pal[256];
	PalEntry oldPal[256];

	debug(2, "DisplayMsg: %s", text);
	
	if (_graphics->getFadeStatus() != RDFADE_BLACK) {
		_graphics->fadeDown();
		_graphics->waitForFade();
	}

	setMouse(0);
	setLuggage(0);

	_graphics->closeMenuImmediately();
	_graphics->clearScene();

	byte *text_spr = _fontRenderer->makeTextSprite(text, 640, 187, _speechFontId);
	FrameHeader *frame = (FrameHeader *) text_spr;

	SpriteInfo spriteInfo;

	spriteInfo.x = _graphics->_screenWide / 2 - frame->width / 2;
	if (!time)
		spriteInfo.y = _graphics->_screenDeep / 2 - frame->height / 2 - RDMENU_MENUDEEP;
	else
		spriteInfo.y = 400 - frame->height;
	spriteInfo.w = frame->width;
	spriteInfo.h = frame->height;
	spriteInfo.scale = 0;
	spriteInfo.scaledWidth = 0;
	spriteInfo.scaledHeight	= 0;
	spriteInfo.type = RDSPR_DISPLAYALIGN | RDSPR_NOCOMPRESSION | RDSPR_TRANS;
	spriteInfo.blend = 0;
	spriteInfo.data = text_spr + sizeof(FrameHeader);
	spriteInfo.colourTable = 0;

	uint32 rv = _graphics->drawSprite(&spriteInfo);
	if (rv)
		error("Driver Error %.8x (in DisplayMsg)", rv);

	memcpy((char *) oldPal, (char *) _graphics->_palCopy, 256 * sizeof(PalEntry));

	memset(pal, 0, 256 * sizeof(PalEntry));
	pal[187].red = 255;
	pal[187].green = 255;
	pal[187].blue = 255;

	_graphics->setPalette(0, 256, (byte *) pal, RDPAL_FADE);
	_graphics->fadeUp();
	free(text_spr);
	_graphics->waitForFade();

	uint32 targetTime = _system->get_msecs() + (time * 1000);

	sleepUntil(targetTime);
	_graphics->setPalette(0, 256, (byte *) oldPal, RDPAL_FADE);
}

/**
 * Fades message down and removes it, fading up again afterwards
 */

void Sword2Engine::removeMsg(void) {
	_graphics->fadeDown();
	_graphics->waitForFade();
	_graphics->clearScene();

	// _graphics->fadeUp();	
	// removed by JEL (08oct97) to prevent "eye" smacker corruption when
	// restarting game from CD2 and also to prevent palette flicker when
	// restoring game to a different CD since the "insert CD" message uses
	// this routine to clean up!
}

void Sword2Engine::drawBackPar0Frames(void) {
	// frame attached to 1st background parallax
	for (uint i = 0; i < _curBgp0; i++)
		processImage(&_bgp0List[i]);
}

void Sword2Engine::drawBackPar1Frames(void) {
	// frame attached to 2nd background parallax
	for (uint i = 0; i < _curBgp1; i++)
		processImage(&_bgp1List[i]);
}

void Sword2Engine::drawBackFrames(void) {
	// background sprite, fixed to main background
	for (uint i = 0; i < _curBack; i++)
		processImage(&_backList[i]);
}

void Sword2Engine::drawSortFrames(byte *file) {
	uint i, j;

	// Sort the sort list. Used to be a separate function, but it was only
	// called once, right before calling drawSortFrames().

	if (_curSort > 1) {
		for (i = 0; i < _curSort - 1; i++) {
			for (j = 0; j < _curSort - 1; j++) {
				if (_sortList[_sortOrder[j]].sort_y > _sortList[_sortOrder[j + 1]].sort_y) {
					SWAP(_sortOrder[j], _sortOrder[j + 1]);
				}
			}
		}
	}

	// Draw the sorted frames - layers, shrinkers & normal flat sprites

	for (i = 0; i < _curSort; i++) {
		if (_sortList[_sortOrder[i]].layer_number) {
			// it's a layer - minus 1 for true layer number
			// we need to know from the BuildUnit because the
			// layers will have been sorted in random order
			processLayer(file, _sortList[_sortOrder[i]].layer_number - 1);
		} else {
			// it's a sprite
			processImage(&_sortList[_sortOrder[i]]);
		}
	}
}

void Sword2Engine::drawForeFrames(void) {
	// foreground sprite, fixed to main background
	for (uint i = 0; i < _curFore; i++)
		processImage(&_foreList[i]);
}

void Sword2Engine::drawForePar0Frames(void) {
	// frame attached to 1st foreground parallax
	for (uint i = 0; i < _curFgp0; i++)
		processImage(&_fgp0List[i]);
}

void Sword2Engine::drawForePar1Frames(void) {
	// frame attached to 2nd foreground parallax
	for (uint i = 0; i < _curFgp1; i++)
		processImage(&_fgp1List[i]);
}

void Sword2Engine::processLayer(byte *file, uint32 layer_number) {
	LayerHeader *layer_head = fetchLayerHeader(file, layer_number);

 	SpriteInfo spriteInfo;

	spriteInfo.x = layer_head->x;
	spriteInfo.y = layer_head->y;
	spriteInfo.w = layer_head->width;
	spriteInfo.scale = 0;
	spriteInfo.scaledWidth = 0;
	spriteInfo.scaledHeight = 0;
	spriteInfo.h = layer_head->height;
	spriteInfo.type = RDSPR_TRANS | RDSPR_RLE256FAST;
	spriteInfo.blend = 0;
	spriteInfo.data = file + sizeof(StandardHeader) + layer_head->offset;
	spriteInfo.colourTable = 0;

	// check for largest layer for debug info

	uint32 current_layer_area = layer_head->width * layer_head->height;

	if (current_layer_area > _largestLayerArea) {
		byte buf[NAME_LEN];

		_largestLayerArea = current_layer_area;
		sprintf(_largestLayerInfo,
			"largest layer:  %s layer(%d) is %dx%d",
			fetchObjectName(_thisScreen.background_layer_id, buf),
			layer_number, layer_head->width, layer_head->height);
	}

	uint32 rv = _graphics->drawSprite(&spriteInfo);
	if (rv)
		error("Driver Error %.8x in processLayer(%d)", rv, layer_number);
}

void Sword2Engine::processImage(BuildUnit *build_unit) {
	byte *file = _resman->openResource(build_unit->anim_resource);
	byte *colTablePtr = NULL;

	AnimHeader *anim_head = fetchAnimHeader(file);
	CdtEntry *cdt_entry = fetchCdtEntry(file, build_unit->anim_pc);
	FrameHeader *frame_head = fetchFrameHeader(file, build_unit->anim_pc);

	// so that 0-colour is transparent
	uint32 spriteType = RDSPR_TRANS;

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
			colTablePtr = (byte *) (anim_head + 1) + anim_head->noAnimFrames * sizeof(CdtEntry);
			break;
		}
	}

	// if we want this frame to be affected by the shading mask,
	// add the status bit
	if (build_unit->shadingFlag)
		spriteType |= RDSPR_SHADOW;

	SpriteInfo spriteInfo;

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
	spriteInfo.data = (byte *) (frame_head + 1);
	spriteInfo.colourTable	= colTablePtr;

	// check for largest layer for debug info
	uint32 current_sprite_area = frame_head->width * frame_head->height;

	if (current_sprite_area > _largestSpriteArea) {
		byte buf[NAME_LEN];

		_largestSpriteArea = current_sprite_area;
		sprintf(_largestSpriteInfo,
			"largest sprite: %s frame(%d) is %dx%d",
			fetchObjectName(build_unit->anim_resource, buf),
			build_unit->anim_pc,
			frame_head->width,
			frame_head->height);
	}

	if (Logic::_scriptVars[SYSTEM_TESTING_ANIMS]) { // see anims.cpp
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

	uint32 rv = _graphics->drawSprite(&spriteInfo);
	if (rv) {
		byte buf[NAME_LEN];

		error("Driver Error %.8x with sprite %s (%d) in processImage",
			rv, fetchObjectName(build_unit->anim_resource, buf),
			build_unit->anim_resource);
	}

	// release the anim resource
	_resman->closeResource(build_unit->anim_resource);
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
		// there are some layers - so rebuild the sort order list
		for (uint i = 0; i < _curSort; i++)
			_sortOrder[i] = i;
	}
}

void Sword2Engine::registerFrame(int32 *params, BuildUnit *build_unit) {
	// params: 0 pointer to mouse structure or NULL for no write to mouse
	//           list (non-zero means write sprite-shape to mouse list)
	//         1 pointer to graphic structure
	//         2 pointer to mega structure

	ObjectGraphic *ob_graph = (ObjectGraphic *) _memory->decodePtr(params[1]);
	assert(ob_graph->anim_resource);

	byte *file = _resman->openResource(ob_graph->anim_resource);

	AnimHeader *anim_head = fetchAnimHeader(file);
	CdtEntry *cdt_entry = fetchCdtEntry(file, ob_graph->anim_pc);
	FrameHeader *frame_head = fetchFrameHeader(file, ob_graph->anim_pc);

	// update player graphic details for on-screen debug info
	if (Logic::_scriptVars[ID] == CUR_PLAYER_ID) {
		_debugger->_playerGraphic.type = ob_graph->type;
		_debugger->_playerGraphic.anim_resource = ob_graph->anim_resource;
		// counting 1st frame as 'frame 1'
		_debugger->_playerGraphic.anim_pc = ob_graph->anim_pc + 1;
		_debugger->_playerGraphicNoFrames = anim_head->noAnimFrames;
	}

	// fill in the BuildUnit structure for this frame

 	build_unit->anim_resource = ob_graph->anim_resource;
	build_unit->anim_pc = ob_graph->anim_pc;
	build_unit->layer_number = 0;

	// Affected by shading mask?
	if (ob_graph->type & SHADED_SPRITE)
		build_unit->shadingFlag = true;
	else
		build_unit->shadingFlag = false;

	// Check if this frame has offsets ie. this is a scalable mega frame

	int scale = 0;

	if (cdt_entry->frameType & FRAME_OFFSET) {
		ObjectMega *ob_mega = (ObjectMega *) _memory->decodePtr(params[2]);

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

		// Work out new width and height. Always divide by 256 after
		// everything else, to maintain accurary
		build_unit->scaled_width = ((scale * frame_head->width) / 256);
		build_unit->scaled_height = ((scale * frame_head->height) / 256);
	} else {
		// It's a non-scaling anim. Get render coords for sprite, from cdt
		build_unit->x = cdt_entry->x;
		build_unit->y = cdt_entry->y;

		// Get width and height
		build_unit->scaled_width = frame_head->width;
		build_unit->scaled_height = frame_head->height;
	}

	// either 0 or required scale, depending on whether 'scale' computed
	build_unit->scale = scale;

	// calc the bottom y-coord for sorting purposes
	build_unit->sort_y = build_unit->y + build_unit->scaled_height - 1;

	if (params[0]) {
		// passed a mouse structure, so add to the _mouseList
		ObjectMouse *ob_mouse = (ObjectMouse *) _memory->decodePtr(params[0]);

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
			// isn't same is current id then we don't want this
			// "left over" pointer text

			if (_mouseList[_curMouse].pointer_text && _mouseList[_curMouse].id != (int32) Logic::_scriptVars[ID])
				_mouseList[_curMouse].pointer_text = 0;

			_mouseList[_curMouse].id = Logic::_scriptVars[ID];
			// not using sprite as detection mask
			_mouseList[_curMouse].anim_resource = 0;
			_mouseList[_curMouse].anim_pc = 0;

			_curMouse++;
		}
	}

	_resman->closeResource(ob_graph->anim_resource);
}

int32 Logic::fnRegisterFrame(int32 *params) {
	// this call would be made from an objects service script 0

	// params:	0 pointer to mouse structure or NULL for no write to
	//		  mouse list (non-zero means write sprite-shape to
	//		  mouse list)
	//		1 pointer to graphic structure
	//		2 pointer to mega structure or NULL if not a mega

	return _vm->registerFrame(params);
}

int32 Sword2Engine::registerFrame(int32 *params) {
	ObjectGraphic *ob_graph = (ObjectGraphic *) _memory->decodePtr(params[1]);

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

/**
 * Start layer palette fading up
 */

void Sword2Engine::startNewPalette(void) {
	// if the screen is still fading down then wait for black - could
	// happen when everythings cached into a large memory model
	_graphics->waitForFade();

	byte *screenFile = _resman->openResource(_thisScreen.background_layer_id);

	_graphics->updatePaletteMatchTable((byte *) fetchPaletteMatchTable(screenFile));
	_graphics->setPalette(0, 256, fetchPalette(screenFile), RDPAL_FADE);

	// indicating that it's a screen palette
	_lastPaletteRes = 0;

  	_resman->closeResource(_thisScreen.background_layer_id);
	_graphics->fadeUp();
 	_thisScreen.new_palette = 0;
}

int32 Logic::fnUpdatePlayerStats(int32 *params) {
	// engine needs to know certain info about the player

	// params:	0 pointer to mega structure

	ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->decodePtr(params[0]);

	_vm->_thisScreen.player_feet_x = ob_mega->feet_x;
	_vm->_thisScreen.player_feet_y = ob_mega->feet_y;

	// for the script
	_scriptVars[PLAYER_FEET_X] = ob_mega->feet_x;
	_scriptVars[PLAYER_FEET_Y] = ob_mega->feet_y;
	_scriptVars[PLAYER_CUR_DIR] = ob_mega->current_dir;
	_scriptVars[SCROLL_OFFSET_X] = _vm->_thisScreen.scroll_offset_x;

	debug(5, "fnUpdatePlayerStats: %d %d", ob_mega->feet_x, ob_mega->feet_y);

	return IR_CONT;
}

int32 Logic::fnFadeDown(int32 *params) {
	// NONE means up! can only be called when screen is fully faded up -
	// multiple calls wont have strange effects

	// params:	none

	if (_vm->_graphics->getFadeStatus() == RDFADE_NONE)
		_vm->_graphics->fadeDown();

	return IR_CONT;
}

int32 Logic::fnFadeUp(int32 *params) {
	// params:	none

	_vm->_graphics->waitForFade();

	if (_vm->_graphics->getFadeStatus() == RDFADE_BLACK)
		_vm->_graphics->fadeUp();

	return IR_CONT;
}

int32 Logic::fnSetPalette(int32 *params) {
	// params:	0 resource number of palette file, or 0 if it's to be
	//		  the palette from the current screen

	_vm->setFullPalette(params[0]);
	return IR_CONT;
}

void Sword2Engine::setFullPalette(int32 palRes) {
	// fudge for hut interior
	// - unpausing should restore last palette as normal (could be screen
	// palette or 'dark_palette_13')
	// - but restoring the screen palette after 'dark_palette_13' should
	// now work properly too!

	// "Hut interior" refers to the watchman's hut in Marseille, and this
	// is apparently needed for the palette to be restored properly when
	// you turn the light off. (I didn't even notice the light switch!)

	if (Logic::_scriptVars[LOCATION] == 13) {
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

	// If non-zero, set palette to this separate palette file. Otherwise,
	// set palette to current screen palette.

	byte *file;

	if (palRes) {
		file = _resman->openResource(palRes);

		StandardHeader *head = (StandardHeader *) file;
		assert(head->fileType == PALETTE_FILE);

		file += sizeof(StandardHeader);

		// always set colour 0 to black because most background screen
		// palettes have a bright colour 0 although it should come out
		// as black in the game!

		file[0] = 0;
		file[1] = 0;
		file[2] = 0;
		file[3] = 0;

		_graphics->setPalette(0, 256, file, RDPAL_INSTANT);
	  	_resman->closeResource(palRes);
	} else {
		if (_thisScreen.background_layer_id) {
			file = _resman->openResource(_thisScreen.background_layer_id);
			_graphics->updatePaletteMatchTable(fetchPaletteMatchTable(file));
			_graphics->setPalette(0, 256, fetchPalette(file), RDPAL_INSTANT);
	  		_resman->closeResource(_thisScreen.background_layer_id);
		} else
			error("setFullPalette(0) called, but no current screen available!");
	}

	if (palRes != CONTROL_PANEL_PALETTE)
		_lastPaletteRes = palRes;
}

int32 Logic::fnRestoreGame(int32 *params) {
	// params:	none
	return IR_CONT;
}

int32 Logic::fnChangeShadows(int32 *params) {
	// params:	none

	// if last screen was using a shading mask (see below)
	if (_vm->_thisScreen.mask_flag) {
		uint32 rv = _vm->_graphics->closeLightMask();
		if (rv)
			error("Driver Error %.8x", rv);
		_vm->_thisScreen.mask_flag = false;
	}

	return IR_CONT;
}

} // End of namespace Sword2
