/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2005 The ScummVM project
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
#include "common/system.h"

#include "sword2/sword2.h"
#include "sword2/console.h"
#include "sword2/defs.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/mouse.h"
#include "sword2/resman.h"

namespace Sword2 {

Screen::Screen(Sword2Engine *vm, int16 width, int16 height) {
	_vm = vm;

	_dirtyGrid = _buffer = NULL;

	_vm->_system->initSize(width, height);

	_screenWide = width;
	_screenDeep = height;

	_gridWide = width / CELLWIDE;
	_gridDeep = height / CELLDEEP;

	if ((width % CELLWIDE) || (height % CELLDEEP))
		error("Bad cell size");

	_dirtyGrid = (byte *) calloc(_gridWide, _gridDeep);
	if (!_dirtyGrid)
		error("Could not initialise dirty grid");

	_buffer = (byte *) malloc(width * height);
	if (!_buffer)
		error("Could not initialise display");

	for (int i = 0; i < ARRAYSIZE(_blockSurfaces); i++)
		_blockSurfaces[i] = NULL;

	_lightMask = NULL;
	_needFullRedraw = false;

	memset(&_thisScreen, 0, sizeof(_thisScreen));

	_fps = 0;
	_frameCount = 0;
	_cycleTime = 0;

	_lastPaletteRes = 0;

	_scrollFraction = 16;

	_largestLayerArea = 0;
	_largestSpriteArea = 0;

	strcpy(_largestLayerInfo,  "largest layer:  none registered");
	strcpy(_largestSpriteInfo, "largest sprite: none registered");

	_fadeStatus = RDFADE_NONE;
	_renderAverageTime = 60;

	_layer = 0;
}

Screen::~Screen() {
	free(_buffer);
	free(_dirtyGrid);
	closeBackgroundLayer();
	free(_lightMask);
}

void Screen::buildDisplay() {
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

	setScrollTarget(_thisScreen.scroll_offset_x, _thisScreen.scroll_offset_y);
	_vm->_mouse->animateMouse();
	startRenderCycle();

	byte *file = _vm->_resman->openResource(_thisScreen.background_layer_id);
	MultiScreenHeader *screenLayerTable = (MultiScreenHeader *) (file + sizeof(StandardHeader));

	// Render at least one frame, but if the screen is scrolling, and if
	// there is time left, we will render extra frames to smooth out the
	// scrolling.

	do {
		// first background parallax + related anims
		if (screenLayerTable->bg_parallax[0]) {
			renderParallax(_vm->fetchBackgroundParallaxLayer(file, 0), 0);
			drawBackPar0Frames();
		}

		// second background parallax + related anims
		if (screenLayerTable->bg_parallax[1]) {
			renderParallax(_vm->fetchBackgroundParallaxLayer(file, 1), 1);
			drawBackPar1Frames();
		}

		// normal backround layer (just the one!)
		renderParallax(_vm->fetchBackgroundLayer(file), 2);

		// sprites & layers
		drawBackFrames();	// background sprites
		drawSortFrames(file);	// sorted sprites & layers
		drawForeFrames();	// foreground sprites

		// first foreground parallax + related anims

		if (screenLayerTable->fg_parallax[0]) {
			renderParallax(_vm->fetchForegroundParallaxLayer(file, 0), 3);
			drawForePar0Frames();
		}

		// second foreground parallax + related anims

		if (screenLayerTable->fg_parallax[1]) {
			renderParallax(_vm->fetchForegroundParallaxLayer(file, 1), 4);
			drawForePar1Frames();
		}

		_vm->_debugger->drawDebugGraphics();
		_vm->_fontRenderer->printTextBlocs();
		_vm->_mouse->processMenu();

		updateDisplay();

		_frameCount++;
		if (_vm->getMillis() > _cycleTime) {
			_fps = _frameCount;
			_frameCount = 0;
			_cycleTime = _vm->getMillis() + 1000;
		}
	} while (!endRenderCycle());

	_vm->_resman->closeResource(_thisScreen.background_layer_id);
}

/**
 * Fades down and displays a message on the screen.
 * @param text The message
 * @param time The number of seconds to display the message, or 0 to display it
 *             until the user clicks the mouse or presses a key.
 */

void Screen::displayMsg(byte *text, int time) {
	byte pal[256 * 4];
	byte oldPal[256 * 4];

	debug(2, "DisplayMsg: %s", text);
	
	if (getFadeStatus() != RDFADE_BLACK) {
		fadeDown();
		waitForFade();
	}

	_vm->_mouse->setMouse(0);
	_vm->_mouse->setLuggage(0);
	_vm->_mouse->closeMenuImmediately();

	clearScene();

	byte *text_spr = _vm->_fontRenderer->makeTextSprite(text, 640, 187, _vm->_speechFontId);
	FrameHeader *frame = (FrameHeader *) text_spr;

	SpriteInfo spriteInfo;

	spriteInfo.x = _screenWide / 2 - frame->width / 2;
	if (!time)
		spriteInfo.y = _screenDeep / 2 - frame->height / 2 - RDMENU_MENUDEEP;
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

	uint32 rv = drawSprite(&spriteInfo);
	if (rv)
		error("Driver Error %.8x (in DisplayMsg)", rv);

	memcpy(oldPal, _palette, sizeof(oldPal));
	memset(pal, 0, sizeof(pal));

	pal[187 * 4 + 0] = 255;
	pal[187 * 4 + 1] = 255;
	pal[187 * 4 + 2] = 255;

	setPalette(0, 256, pal, RDPAL_FADE);
	fadeUp();
	free(text_spr);
	waitForFade();

	if (time > 0) {
		uint32 targetTime = _vm->getMillis() + (time * 1000);
		_vm->sleepUntil(targetTime);
	} else {
		while (!_vm->_quit) {
			MouseEvent *me = _vm->mouseEvent();
			if (me && (me->buttons & (RD_LEFTBUTTONDOWN | RD_RIGHTBUTTONDOWN)))
				break;

			if (_vm->keyboardEvent())
				break;

			updateDisplay();
			_vm->_system->delayMillis(50);
		}
	}

	fadeDown();
	waitForFade();
	clearScene();
	setPalette(0, 256, oldPal, RDPAL_FADE);
	fadeUp();
}

void Screen::drawBackPar0Frames() {
	// frame attached to 1st background parallax
	for (uint i = 0; i < _curBgp0; i++)
		processImage(&_bgp0List[i]);
}

void Screen::drawBackPar1Frames() {
	// frame attached to 2nd background parallax
	for (uint i = 0; i < _curBgp1; i++)
		processImage(&_bgp1List[i]);
}

void Screen::drawBackFrames() {
	// background sprite, fixed to main background
	for (uint i = 0; i < _curBack; i++)
		processImage(&_backList[i]);
}

void Screen::drawSortFrames(byte *file) {
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

void Screen::drawForeFrames() {
	// foreground sprite, fixed to main background
	for (uint i = 0; i < _curFore; i++)
		processImage(&_foreList[i]);
}

void Screen::drawForePar0Frames() {
	// frame attached to 1st foreground parallax
	for (uint i = 0; i < _curFgp0; i++)
		processImage(&_fgp0List[i]);
}

void Screen::drawForePar1Frames() {
	// frame attached to 2nd foreground parallax
	for (uint i = 0; i < _curFgp1; i++)
		processImage(&_fgp1List[i]);
}

void Screen::processLayer(byte *file, uint32 layer_number) {
	LayerHeader *layer_head = _vm->fetchLayerHeader(file, layer_number);

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
			_vm->fetchObjectName(_thisScreen.background_layer_id, buf),
			layer_number, layer_head->width, layer_head->height);
	}

	uint32 rv = drawSprite(&spriteInfo);
	if (rv)
		error("Driver Error %.8x in processLayer(%d)", rv, layer_number);
}

void Screen::processImage(BuildUnit *build_unit) {
	byte *file = _vm->_resman->openResource(build_unit->anim_resource);
	byte *colTablePtr = NULL;

	AnimHeader *anim_head = _vm->fetchAnimHeader(file);
	CdtEntry *cdt_entry = _vm->fetchCdtEntry(file, build_unit->anim_pc);
	FrameHeader *frame_head = _vm->fetchFrameHeader(file, build_unit->anim_pc);

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
			_vm->fetchObjectName(build_unit->anim_resource, buf),
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
		_vm->_debugger->_rectX1 = spriteInfo.x - 1;
		_vm->_debugger->_rectY1 = spriteInfo.y - 1;
		_vm->_debugger->_rectX2 = spriteInfo.x + spriteInfo.scaledWidth;
		_vm->_debugger->_rectY2 = spriteInfo.y + spriteInfo.scaledHeight;
	}

	uint32 rv = drawSprite(&spriteInfo);
	if (rv) {
		byte buf[NAME_LEN];

		error("Driver Error %.8x with sprite %s (%d) in processImage",
			rv,
			_vm->fetchObjectName(build_unit->anim_resource, buf),
			build_unit->anim_resource);
	}

	// release the anim resource
	_vm->_resman->closeResource(build_unit->anim_resource);
}

void Screen::resetRenderLists() {
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

void Screen::registerFrame(ObjectMouse *ob_mouse, ObjectGraphic *ob_graph, ObjectMega *ob_mega, BuildUnit *build_unit) {
	assert(ob_graph->anim_resource);

	byte *file = _vm->_resman->openResource(ob_graph->anim_resource);

	AnimHeader *anim_head = _vm->fetchAnimHeader(file);
	CdtEntry *cdt_entry = _vm->fetchCdtEntry(file, ob_graph->anim_pc);
	FrameHeader *frame_head = _vm->fetchFrameHeader(file, ob_graph->anim_pc);

	// update player graphic details for on-screen debug info
	if (Logic::_scriptVars[ID] == CUR_PLAYER_ID) {
		_vm->_debugger->_playerGraphic.type = ob_graph->type;
		_vm->_debugger->_playerGraphic.anim_resource = ob_graph->anim_resource;
		// counting 1st frame as 'frame 1'
		_vm->_debugger->_playerGraphic.anim_pc = ob_graph->anim_pc + 1;
		_vm->_debugger->_playerGraphicNoFrames = anim_head->noAnimFrames;
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
		// Calc scale at which to print the sprite, based on feet
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

	if (ob_mouse) {
		// passed a mouse structure, so add to the _mouseList
		_vm->_mouse->registerMouse(ob_mouse, build_unit);

	}

	_vm->_resman->closeResource(ob_graph->anim_resource);
}

void Screen::registerFrame(ObjectMouse *ob_mouse, ObjectGraphic *ob_graph, ObjectMega *ob_mega) {
	// check low word for sprite type
	switch (ob_graph->type & 0x0000ffff) {
	case BGP0_SPRITE:
		assert(_curBgp0 < MAX_bgp0_sprites);
		registerFrame(ob_mouse, ob_graph, ob_mega, &_bgp0List[_curBgp0]);
		_curBgp0++;
		break;
	case BGP1_SPRITE:
		assert(_curBgp1 < MAX_bgp1_sprites);
		registerFrame(ob_mouse, ob_graph, ob_mega, &_bgp1List[_curBgp1]);
		_curBgp1++;
		break;
	case BACK_SPRITE:
		assert(_curBack < MAX_back_sprites);
		registerFrame(ob_mouse, ob_graph, ob_mega, &_backList[_curBack]);
		_curBack++;
		break;
	case SORT_SPRITE:
		assert(_curSort < MAX_sort_sprites);
		_sortOrder[_curSort] = _curSort;
		registerFrame(ob_mouse, ob_graph, ob_mega, &_sortList[_curSort]);
		_curSort++;
		break;
	case FORE_SPRITE:
		assert(_curFore < MAX_fore_sprites);
		registerFrame(ob_mouse, ob_graph, ob_mega, &_foreList[_curFore]);
		_curFore++;
		break;
	case FGP0_SPRITE:
		assert(_curFgp0 < MAX_fgp0_sprites);
		registerFrame(ob_mouse, ob_graph, ob_mega, &_fgp0List[_curFgp0]);
		_curFgp0++;
		break;
	case FGP1_SPRITE:
		assert(_curFgp1 < MAX_fgp1_sprites);
		registerFrame(ob_mouse, ob_graph, ob_mega, &_fgp1List[_curFgp1]);
		_curFgp1++;
		break;
	default:
		// NO_SPRITE no registering!
		break;
	}
}

} // End of namespace Sword2
