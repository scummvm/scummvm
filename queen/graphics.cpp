/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "queen/graphics.h"
#include "queen/display.h"


namespace Queen {


Graphics::Graphics(Display *display, Resource *resource)
	: _cameraBob(0), _display(display), _resource(resource) {
		
	memset(_frames, 0, sizeof(_frames));
	memset(_banks, 0, sizeof(_banks));
	memset(_bobs, 0, sizeof(_bobs));
	memset(_sortedBobs, 0, sizeof(_sortedBobs));
	_sortedBobsCount = 0;
	_shrinkBuffer.data = new uint8[ BOB_SHRINK_BUF_SIZE ];
}


Graphics::~Graphics() {
	uint32 i;
	for(i = 0; i < ARRAYSIZE(_banks); ++i) {
		delete _banks[i].data;
	}
	frameEraseAll(true);
	delete[] _shrinkBuffer.data;
}


void Graphics::bankLoad(const char *bankname, uint32 bankslot) {
	
	int16 i;
	
	bankErase(bankslot);
	_banks[bankslot].data = _resource->loadFile(bankname);
	if (!_banks[bankslot].data) {
	  error("Unable to open bank '%s'", bankname);	
	}

	int16 entries = (int16)READ_LE_UINT16(_banks[bankslot].data);
	if (entries < 0 || entries >= MAX_BANK_SIZE) {
	  error("Maximum bank size exceeded or negative bank size : %d", entries);
	}
	
	uint32 offset = 2;
	uint8 *p = _banks[bankslot].data;
	for (i = 1; i <= entries; ++i) {
		_banks[bankslot].indexes[i] = offset;
		uint16 w = READ_LE_UINT16(p + offset + 0);
		uint16 h = READ_LE_UINT16(p + offset + 2);
		// jump to next entry, skipping data & header
		offset += w * h + 8; 
	}
	
	debug(5, "Loaded bank '%s' in slot %d, %d entries", bankname, bankslot, entries); 
}


void Graphics::bankUnpack(uint32 srcframe, uint32 dstframe, uint32 bankslot) {
	
	uint8 *p = _banks[bankslot].data + _banks[bankslot].indexes[srcframe];

	if (!_banks[bankslot].data)
		error("Graphics::bankUnpack(%i, %i, %i) called but _banks[bankslot].data is NULL!", 
				srcframe, dstframe, bankslot);
		
	BobFrame *pbf = &_frames[dstframe];
	delete[] pbf->data;

	pbf->width    = READ_LE_UINT16(p + 0);
	pbf->height   = READ_LE_UINT16(p + 2);
	pbf->xhotspot = READ_LE_UINT16(p + 4);
	pbf->yhotspot = READ_LE_UINT16(p + 6);
	
	uint32 size = pbf->width * pbf->height;
	pbf->data = new uint8[ size ];
	memcpy(pbf->data, p + 8, size);
	
	debug(5, "Unpacked frame %d from bank slot %d to frame slot %d", srcframe, bankslot, dstframe);
}


void Graphics::bankOverpack(uint32 srcframe, uint32 dstframe, uint32 bankslot) {
	
	uint8 *p = _banks[bankslot].data + _banks[bankslot].indexes[srcframe];
	uint16 src_w = READ_LE_UINT16(p + 0);
	uint16 src_h = READ_LE_UINT16(p + 2);
	
	// unpack if destination frame is smaller than source one
	if (_frames[dstframe].width < src_w || _frames[dstframe].height < src_h) {
		bankUnpack(srcframe, dstframe, bankslot);
	}
	else {
		// copy data 'over' destination frame (without changing frame header)
		memcpy(_frames[dstframe].data, p + 8, src_w * src_h);
	}
	
	debug(5, "Overpacked frame %d from bank slot %d to frame slot %d", srcframe, bankslot, dstframe);
}


void Graphics::bankErase(uint32 bankslot) {
	
	delete[] _banks[bankslot].data;
	_banks[bankslot].data = 0;
	
	debug(5, "Erased bank in slot %d", bankslot);
}


void Graphics::bobSetupControl() {

	bankLoad("control.BBK",17);
	bankUnpack(1, 1, 17); // Mouse pointer
    bankUnpack(3, 3, 17); // Up arrow dialogue
    bankUnpack(4, 4, 17); // Down arrow dialogue
    bankErase(17);
}


void Graphics::bobAnimString(uint32 bobnum, const AnimFrame *animBuf) {

	BobSlot *pbs = &_bobs[bobnum];
	pbs->active = true;
	pbs->animating = true;
	pbs->anim.string.buffer = animBuf;
	pbs->anim.string.curPos = animBuf;
	pbs->frameNum = animBuf->frame;
	pbs->anim.speed = animBuf->speed / 4;
}


void Graphics::bobAnimNormal(uint32 bobnum, uint16 firstFrame, uint16 lastFrame, uint16 speed, bool rebound, bool xflip) {

	BobSlot *pbs = &_bobs[bobnum];
	pbs->active = true;
	pbs->animating = true;
	pbs->frameNum = firstFrame;
	pbs->anim.speed = speed;
	pbs->anim.speedBak = speed;
	pbs->anim.string.buffer = NULL;
	pbs->anim.normal.firstFrame = firstFrame;
	pbs->anim.normal.lastFrame = lastFrame;
	pbs->anim.normal.rebound = rebound;
	pbs->frameDir = 1;
	pbs->xflip = xflip;
}


void Graphics::bobMove(uint32 bobnum, uint16 endx, uint16 endy, int16 speed) {

	debug(5, "Graphics::bobMove(%d, %d, %d, %d)", bobnum, endx, endy, speed);

	BobSlot *pbs = &_bobs[bobnum];

	pbs->active = true;
	pbs->moving = true;

	pbs->endx = endx;
	pbs->endy = endy;

	pbs->speed = (speed < 1) ? 1 : speed;

	int16 dx = endx - pbs->x;
	if (dx < 0) {
		pbs->dx   = -dx;
		pbs->xdir = -1;
	}
	else {
		pbs->dx   = dx;
		pbs->xdir = 1;
	}
	int16 dy = endy - pbs->y;
	if (dy < 0) {
		pbs->dy   = -dy;
		pbs->ydir = -1;
	}
	else {
		pbs->dy   = dy;
		pbs->ydir = 1;
	}

	if (pbs->dx > pbs->dy) {
		pbs->total = pbs->dy / 2;
		pbs->xmajor = true;
	}
	else {
		pbs->total = pbs->dx / 2;
		pbs->xmajor = false;
	}

	// move one step along line to avoid glitching
	pbs->moveOneStep();
}


void BobSlot::moveOneStep() {

	if(xmajor) {
		if(x == endx) {
			y = endy;
			moving = false;
		}
		else {
			x += xdir;
			total += dy;
			if(total > dx) {
				y += ydir;
				total -= dx;
			}
		}
	}
	else {
		if(y == endy) {
			x = endx;
			moving = false;
		}
		else {
			y += ydir;
			total += dx;
			if(total > dy) {
				x += xdir;
				total -= dy;
			}
		}
	}
}


void BobSlot::animOneStep() {

	if (anim.string.buffer != NULL) {
		--anim.speed;
		if(anim.speed == 0) {
			// jump to next entry
			++anim.string.curPos;
			uint16 nextFrame = anim.string.curPos->frame;
			if (nextFrame == 0) {
				anim.string.curPos = anim.string.buffer;
				frameNum = anim.string.curPos->frame;
			}
			else {
				frameNum = nextFrame;
			}
			anim.speed = anim.string.curPos->speed / 4;

			// FIXME: handle that when QueenSound class is ready
			// play memory sfx and move on to next frame
			if(frameNum > 500) {
				frameNum -= 500;
				// _sound->sfxplay(NULLstr);
			}
		}
	}
	else {
		// normal looping animation
		--anim.speed;
		if(anim.speed == 0) {
			anim.speed = anim.speedBak;

			uint16 nextFrame = frameNum + frameDir;
			if (nextFrame > anim.normal.lastFrame || nextFrame < anim.normal.firstFrame) {
				if (anim.normal.rebound) {
					frameDir *= -1;
				}
				else {
					frameNum = anim.normal.firstFrame - 1;
				}
			}
			frameNum += frameDir;
		}
	}
}
          

void Graphics::bobDraw(uint32 bobnum, uint16 x, uint16 y, uint16 scale, bool xflip, const Box& box) {

	uint16 w, h;

	debug(5, "Preparing to draw frame %d, scale = %d, coords = (%d,%d)", bobnum, scale, x, y);

	BobFrame *pbf = &_frames[bobnum];
	if (scale < 100) {
		bobShrink(pbf, scale);
		pbf = &_shrinkBuffer;
	}
	w = pbf->width;
	h = pbf->height;

	if(w != 0 && h != 0 && box.intersects(x, y, w, h)) {

		uint8 *src = pbf->data;
		uint16 x_skip = 0;
		uint16 y_skip = 0;
		uint16 w_new = w;
		uint16 h_new = h;

		// compute bounding box intersection with frame
		if (x < box.x1) {
			x_skip = box.x1 - x;
			w_new -= x_skip;
			x = box.x1;
		}

		if (y < box.y1) {
			y_skip = box.y1 - y;
			h_new -= y_skip;
			y = box.y1;
		}

		if (x + w_new > box.x2 + 1) {
			w_new = box.x2 - x + 1;
		}

		if (y + h_new > box.y2 + 1) {
			h_new = box.y2 - y + 1;
		}

		src += w * y_skip;
		if (!xflip) {
			src += x_skip;
			_display->blit(RB_SCREEN, x, y, src, w_new, h_new, w, xflip, true);
		}
		else {
			src += w - w_new - x_skip;
			x += w_new - 1;
			_display->blit(RB_SCREEN, x, y, src, w_new, h_new, w, xflip, true);
		}
    }

}


void Graphics::bobDrawInventoryItem(uint32 bobnum, uint16 x, uint16 y) {
	if (bobnum == 0) {
		// clear panel area
		_display->fill(RB_PANEL, x, y, 32, 32, INK_BG_PANEL);
	}
	else {
		BobFrame *pbf = &_frames[bobnum];
		_display->blit(RB_PANEL, x, y, pbf->data, pbf->width, pbf->height, pbf->width, false, false);
	}
}


void Graphics::bobPaste(uint32 bobnum, uint16 x, uint16 y) {

	BobFrame *pbf = &_frames[bobnum];
	_display->blit(RB_BACKDROP, x, y, pbf->data, pbf->width, pbf->height, pbf->width, false, true);
	frameErase(bobnum);
}


void Graphics::bobShrink(const BobFrame* pbf, uint16 percentage) {

	// computing new size, rounding to upper value
	uint16 new_w = (pbf->width  * percentage + 50) / 100;
	uint16 new_h = (pbf->height * percentage + 50) / 100;

	debug(5, "Shrinking bob, buffer size = %d", new_w * new_h);

	if (new_w != 0 && new_h != 0) {

		_shrinkBuffer.width  = new_w;
		_shrinkBuffer.height = new_h;

		uint32 shrinker = (100 << 0x10) / percentage;
		uint16 x_scale = shrinker >> 0x10;
		uint16 y_scale = x_scale * pbf->width;   
		shrinker &= 0xFFFF;

		uint8* src = pbf->data;
		uint8* dst = _shrinkBuffer.data;

		uint32 y_count = 0;
		while (new_h--) {
			uint16 i;
			uint32 x_count = 0;
			uint8 *p = src;
			for(i = 0; i < new_w; ++i) {
				*dst++ = *p;
				p += x_scale;
				x_count += shrinker;
				if (x_count > 0xFFFF) {
					++p;
					x_count &= 0xFFFF;
				}
			}
			src += y_scale;
			y_count += shrinker;
			if (y_count > 0xFFFF) {
				src += pbf->width;
				y_count &= 0xFFFF;
			}
		}
	}
}


void Graphics::bobClear(uint32 bobnum) {

	BobSlot *pbs = &_bobs[bobnum];

	pbs->active = 0;
	pbs->xflip  = false;
	pbs->anim.string.buffer = NULL;
	pbs->moving = false;
	pbs->scale  = 100;
	pbs->box.x1 = 0;
	pbs->box.y1 = 0;
	pbs->box.x2 = GAME_SCREEN_WIDTH - 1;
	if (_display->fullscreen() || bobnum == 16) { // FIXME: does bob number 16 really used ?
		pbs->box.y2 = GAME_SCREEN_HEIGHT - 1;
	}
	else {
		pbs->box.y2 = ROOM_ZONE_HEIGHT - 1; 
	}
}


void Graphics::bobSortAll() {

	_sortedBobsCount = 0;

	// animate/move the bobs
	for (int32 i = 0; i < ARRAYSIZE(_bobs); ++i) {

		BobSlot *pbs = &_bobs[i];
		if (pbs->active) {
			_sortedBobs[_sortedBobsCount] = pbs;
			++_sortedBobsCount;

			if (pbs->animating) {
				pbs->animOneStep();
			}
			if (pbs->moving) {
				for (uint32 j = 0; pbs->moving && j < pbs->speed; ++j) {
					pbs->moveOneStep();
				}
			}
		}
	}

	// bubble sort the bobs
	for (int32 index = 0; index < _sortedBobsCount - 1; ++index) {
		int32 smallest = index;
		for (int32 compare = index + 1; compare <= _sortedBobsCount - 1; ++compare) {
			if (_sortedBobs[compare]->y < _sortedBobs[smallest]->y) {
				smallest = compare;
			}
		}
		if (index != smallest) {
			SWAP(_sortedBobs[index], _sortedBobs[smallest]);
		}
	}

}


void Graphics::bobDrawAll() {

	int i;
	for (i = 0; i < _sortedBobsCount; ++i) {
		BobSlot *pbs = _sortedBobs[i];
		if (pbs->active) {
			BobFrame *pbf = &_frames[ pbs->frameNum ];
			uint16 xh, yh, x, y;

			xh = pbf->xhotspot;
			yh = pbf->yhotspot;

			if (pbs->xflip) {
				xh = pbf->width - xh;
			}

			// adjusts hot spots when object is scaled
			if (pbs->scale != 100) {
				xh = (xh * pbs->scale) / 100;
				yh = (yh * pbs->scale) / 100;
			}

			// adjusts position to hot-spot and screen scroll
			x = pbs->x - xh; // - _display->scrollx;
			y = pbs->y - yh;

			bobDraw(pbs->frameNum, x, y, pbs->scale, pbs->xflip, pbs->box);
		}
	}
}


void Graphics::bobClearAll() {

	for(int32 i = 0; i < ARRAYSIZE(_bobs); ++i) {
		bobClear(i);
	}
}


BobSlot *Graphics::bob(int index) {

	if (index < MAX_BOBS_NUMBER)
		return _bobs + index;
	else {
		error("QueenGraphics::bob called with index = %i but MAX_BOBS_NUMBER = %i", 
				index, MAX_BOBS_NUMBER);
	}
}


void Graphics::bobCustomParallax(uint16 roomNum) {
	
	int i;
	uint16 screenScroll = _display->horizontalScroll();
	switch (roomNum) {
	case 17:
		_bobs[8].x = 250 - screenScroll + screenScroll / 2;
		break;
	case 100:
		_bobs[5].x = 410 - screenScroll + screenScroll / 2;
		_bobs[6].x = 790 - screenScroll + screenScroll / 2;
		break;
	case 43:
		_bobs[5].x = 320 - screenScroll + screenScroll / 2;
		break;
	case 51:
		_bobs[5].x = 280 - screenScroll + screenScroll / 2;
		break;
	case 67:
		_bobs[5].x = 600 - screenScroll + screenScroll / 2;
		break;
	case 73 :
		if(_display->fullscreen()) {
			for(i = 1; i <= 3; ++i) {
				_bobs[i].box.y2 = 199;
			}
			_bobs[24].box.y2 = 199;
		}
		break;
	case 90 :
		_bobs[5].x = 340 - screenScroll + screenScroll / 2;
		_bobs[6].x = 50 - screenScroll + screenScroll / 2;
		_bobs[7].x = 79 - screenScroll + screenScroll / 2;
		for(i = 1; i <= 8; ++i) {
			_bobs[i].box.y2 = 199;
		}
		_bobs[20].box.y2 = 199;
		break;
	case 94 :
		for(i = 0; i < 3; ++i) {
			_bobs[i].box.y2=199;
		}
		break;
	case 74 : // Carbam
		warning("Graphics::bobCustomParallax() - room 74 not handled");
		break;
	case 69 : // Fight1
		warning("Graphics::bobCustomParallax() - room 69 not handled");
		break;
	case 116: // CR 2 - CD-Rom pan right while Rita talks...
		_cameraBob = -1;
		if (screenScroll < 80) {
			_display->horizontalScroll(screenScroll + 4);
			// Joe's body and head
			_bobs[ 1].x += 4;
			_bobs[20].x += 4;
			// Rita's body and head
			_bobs[ 2].x -= 2;
			_bobs[21].x -= 2;
		}
		break;
	case 123: // CR 2 - CD-Rom the guys move off screen
		_bobs[21].x += 2;
		_bobs[21].y += 2;
		break;
	}
}


void Graphics::textCurrentColor(uint8 color) {
	_curTextColor = color;
}


void Graphics::textSet(uint16 x, uint16 y, const char *text, bool outlined) {
	if (y < GAME_SCREEN_HEIGHT) {
		if (x == 0) x = 1;
		if (y == 0) y = 1;
		TextSlot *pts = &_texts[y];

		pts->x = x;
		pts->color = _curTextColor;
		pts->outlined = outlined;
		pts->text = text;
	}
}


void Graphics::textDrawAll() {
	int y;
	for (y = GAME_SCREEN_HEIGHT - 1; y > 0; --y) {
		const TextSlot *pts = &_texts[y];
		if (!pts->text.isEmpty()) {
			_display->textDraw(pts->x, y, pts->color, pts->text.c_str(), pts->outlined);
		}
	}
}


void Graphics::textClear(uint16 y1, uint16 y2) {

	while (y1 <= y2) {
		_texts[y1].text.clear();
		++y1;
	}
}


uint16 Graphics::textWidth(const char* text) const {

	return _display->textWidth(text);
}


void Graphics::frameErase(uint32 fslot) {

	BobFrame *pbf = &_frames[fslot];
	pbf->width  = 0;
	pbf->height = 0;
	delete[] pbf->data;
	pbf->data = 0;
}


void Graphics::frameEraseAll(bool joe) {

    int i = 0;
	if (!joe) {
		i = FRAMES_JOE + FRAMES_JOE_XTRA;
	}
	while (i < 256) {
		frameErase(i);
		++i;
	}
}


void Graphics::loadBackdrop(const char* name, uint16 room) {

	// init Dynalum
	char roomPrefix[20];
	strcpy(roomPrefix, name);
	roomPrefix[ strlen(roomPrefix) - 4 ] = '\0';
	_display->dynalumInit(_resource, roomPrefix, room);

	uint8 *pcxbuf = _resource->loadFile(name);
	if (pcxbuf == NULL) {
		error("Unable to load backdrop : '%s'", name);
	}
	uint32 size = _resource->fileSize(name);
	_display->pcxReadBackdrop(pcxbuf, size, room > 114);
	delete[] pcxbuf;

	if (room >= 90) {
		_cameraBob = 0;
	}
	_lastRoom = room; // TEMP
}


void Graphics::loadPanel() {

	uint8 *pcxbuf = _resource->loadFile("panel.pcx");
	if (pcxbuf == NULL) {
		error("Unable to open panel file");
	}
	uint32 size = _resource->fileSize("panel.pcx");
	_display->pcxReadPanel(pcxbuf, size);
	delete[] pcxbuf;
}


void Graphics::useJournal() { // GameSettings* pgs

	int i;

	bobClearAll();
	loadBackdrop("journal.pcx", 160);
	_display->palFadeOut(0, 255, 160);

	// load and unpack journal frames
	frameEraseAll(false);
    bankLoad("journal.BBK", 8);
	for(i = 1; i <= 20; ++i) {
		bankUnpack(i, FRAMES_JOURNAL + i, 8);
		// set hot spots to zero
		_frames[FRAMES_JOURNAL + i].xhotspot = 0;
		_frames[FRAMES_JOURNAL + i].yhotspot = 0;
	}
	// adjust info box hot spot to put it on top always
	_frames[FRAMES_JOURNAL + 20].yhotspot = 200;
    bankErase(8);

	// TODO: setup zones

	journalBobPreDraw();
	_display->palFadeIn(0, 255, 160);
}


void Graphics::journalBobSetup(uint32 bobnum, uint16 x, uint16 y, uint16 frameNum) 
{
	BobSlot *pbs = &_bobs[bobnum];
	pbs->active = true;
	pbs->x = x;
	pbs->y = y;
	pbs->frameNum = FRAMES_JOURNAL + frameNum;
	pbs->box.y2 = GAME_SCREEN_HEIGHT - 1;
}


void Graphics::journalBobPreDraw() { // GameSettings* pgs

	journalBobSetup(1, 32, 8, 1); // Review entry
	journalBobSetup(2, 32, 56, 2); // Make entry
	journalBobSetup(3, 32, 104, 1); // Close book
	journalBobSetup(4, 32, 152, 3); // Give up
//	journalBobSetup(5, 136 + pgs->talkSpeed * 4 - 4, 164, 18); // Text speed
	journalBobSetup(6, 221, 155, 16); // SFX on/off
//	_bobs[6].active = pgs->sfxToggle;
//	journalBobSetup(7, 136 + (pgs->volume * 130) / 100 - 4, 177, 19); // Music volume
	journalBobSetup(10, 158, 155, 16); // Voice on/off
//	_bobs[10].active = pgs->voiceToggle;
	journalBobSetup(11, 125, 167, 16); // Text on/off
//	_bobs[11].active = pgs->textToggle;
	journalBobSetup(12, 125, 181, 16); // Music on/off
//	_bobs[12].active = pgs->musicToggle;
}


void Graphics::setCameraBob(int bobNum) {
	_cameraBob = bobNum;
}


void Graphics::update() {
	// FIXME: temporary code, move to Logic::update()
	bobSortAll();
	if (_cameraBob >= 0) {
		_display->horizontalScrollUpdate(_bobs[_cameraBob].x);
	}
	// FIXME: currently, we use the _lastRoom variable only
	// to know in which current room we are. This is necessary
	// for the parallax stuff as it relies on the room number.
	// When we switch to the Logic::update() method, we will be
	// able to get rid of this variable...
	bobCustomParallax(_lastRoom);
	_display->prepareUpdate();
	bobDrawAll();
	textDrawAll();
	g_queen->delay(100);
	_display->palCustomScroll(0); //_currentRoom
	_display->update(_bobs[0].active, _bobs[0].x, _bobs[0].y);
}


} // End of namespace Queen

