/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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
#include "queen/logic.h"
#include "queen/queen.h"
#include "queen/resource.h"
#include "queen/sound.h"

namespace Queen {


void BobSlot::curPos(int16 xx, int16 yy) {
	active = true;
	x = xx;
	y = yy;
}


void BobSlot::move(int16 dstx, int16 dsty, int16 spd) {
	active = true;
	moving = true;

	endx = dstx;
	endy = dsty;

	speed = (spd < 1) ? 1 : spd;

	int16 deltax = endx - x;
	if (deltax < 0) {
		dx   = -deltax;
		xdir = -1;
	} else {
		dx   = deltax;
		xdir = 1;
	}
	int16 deltay = endy - y;
	if (deltay < 0) {
		dy   = -deltay;
		ydir = -1;
	} else {
		dy   = deltay;
		ydir = 1;
	}

	if (dx > dy) {
		total = dy / 2;
		xmajor = true;
	} else {
		total = dx / 2;
		xmajor = false;
	}

	// move one step along line to avoid glitching
	moveOneStep();
}


void BobSlot::moveOneStep() {
	if(xmajor) {
		if(x == endx) {
			y = endy;
			moving = false;
		} else {
			x += xdir;
			total += dy;
			if(total > dx) {
				y += ydir;
				total -= dx;
			}
		}
	} else {
		if(y == endy) {
			x = endx;
			moving = false;
		} else {
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
		if(anim.speed <= 0) {
			// jump to next entry
			++anim.string.curPos;
			uint16 nextFrame = anim.string.curPos->frame;
			if (nextFrame == 0) {
				anim.string.curPos = anim.string.buffer;
				frameNum = anim.string.curPos->frame;
			} else {
				frameNum = nextFrame;
			}
			anim.speed = anim.string.curPos->speed / 4;
		}
	} else {
		// normal looping animation
		--anim.speed;
		if(anim.speed == 0) {
			anim.speed = anim.speedBak;

			int16 nextFrame = frameNum + frameDir;
			if (nextFrame > anim.normal.lastFrame || nextFrame < anim.normal.firstFrame) {
				if (anim.normal.rebound) {
					frameDir *= -1;
				} else {
					frameNum = anim.normal.firstFrame - 1;
				}
			}
			frameNum += frameDir;
		}
	}
}


void BobSlot::animString(const AnimFrame *animBuf) {
	active = true;
	animating = true;
	anim.string.buffer = animBuf;
	anim.string.curPos = animBuf;
	frameNum = animBuf->frame;
	anim.speed = animBuf->speed / 4;
}


void BobSlot::animNormal(uint16 firstFrame, uint16 lastFrame, uint16 spd, bool rebound, bool flip) {
	active = true;
	animating = true;
	frameNum = firstFrame;
	anim.speed = spd;
	anim.speedBak = spd;
	anim.string.buffer = NULL;
	anim.normal.firstFrame = firstFrame;
	anim.normal.lastFrame = lastFrame;
	anim.normal.rebound = rebound;
	frameDir = 1;
	xflip = flip;
}


void BobSlot::clear() {
	active = false;
	xflip  = false;
	animating = false;
	anim.string.buffer = NULL;
	moving = false;
	scale  = 100;
	box.x1 = 0;
	box.y1 = 0;
	box.x2 = GAME_SCREEN_WIDTH - 1;
	box.y2 = ROOM_ZONE_HEIGHT - 1;
}



BankManager::BankManager(Resource *res) 
	: _res(res) {
	memset(_frames, 0, sizeof(_frames));
	memset(_banks, 0, sizeof(_banks));
}


BankManager::~BankManager() {
	for(uint32 i = 0; i < MAX_BANKS_NUMBER; ++i) {
		close(i);
	}
	eraseAllFrames(true);
}


void BankManager::load(const char *bankname, uint32 bankslot) {
	close(bankslot);

	_banks[bankslot].data = _res->loadFile(bankname);
	if (!_banks[bankslot].data) {
		error("Unable to open bank '%s'", bankname);	
	}

	int16 entries = (int16)READ_LE_UINT16(_banks[bankslot].data);
	if (entries < 0 || entries >= MAX_BANK_SIZE) {
		error("Maximum bank size exceeded or negative bank size : %d", entries);
	}

	debug(9, "BankManager::load(%s, %d) - entries = %d", bankname, bankslot, entries); 

	uint32 offset = 2;
	uint8 *p = _banks[bankslot].data;
	for (int16 i = 1; i <= entries; ++i) {
		_banks[bankslot].indexes[i] = offset;
		uint16 w = READ_LE_UINT16(p + offset + 0);
		uint16 h = READ_LE_UINT16(p + offset + 2);
		// jump to next entry, skipping data & header
		offset += w * h + 8; 
	}
}


void BankManager::unpack(uint32 srcframe, uint32 dstframe, uint32 bankslot) {
	debug(9, "BankManager::unpack(%d, %d, %d)", srcframe, dstframe, bankslot);
	if (!_banks[bankslot].data)
		error("BankManager::unpack() _banks[bankslot].data is NULL!");
		
	BobFrame *pbf = &_frames[dstframe];
	uint8 *p = _banks[bankslot].data + _banks[bankslot].indexes[srcframe];
	pbf->width    = READ_LE_UINT16(p + 0);
	pbf->height   = READ_LE_UINT16(p + 2);
	pbf->xhotspot = READ_LE_UINT16(p + 4);
	pbf->yhotspot = READ_LE_UINT16(p + 6);

	uint32 size = pbf->width * pbf->height;
	delete[] pbf->data;
	pbf->data = new uint8[ size ];
	memcpy(pbf->data, p + 8, size);
}


void BankManager::overpack(uint32 srcframe, uint32 dstframe, uint32 bankslot) {
	debug(9, "BankManager::overpack(%d, %d, %d)", srcframe, dstframe, bankslot);
	if (!_banks[bankslot].data)
		error("BankManager::overpack() _banks[bankslot].data is NULL!");

	uint8 *p = _banks[bankslot].data + _banks[bankslot].indexes[srcframe];
	uint16 src_w = READ_LE_UINT16(p + 0);
	uint16 src_h = READ_LE_UINT16(p + 2);

	// unpack if destination frame is smaller than source one
	if (_frames[dstframe].width < src_w || _frames[dstframe].height < src_h) {
		unpack(srcframe, dstframe, bankslot);
	} else {
		// copy data 'over' destination frame (without changing frame header)
		memcpy(_frames[dstframe].data, p + 8, src_w * src_h);
	}
}


void BankManager::close(uint32 bankslot) {
	debug(9, "BankManager::close(%d)", bankslot);
	delete[] _banks[bankslot].data;
	memset(&_banks[bankslot], 0, sizeof(_banks[bankslot]));
}


BobFrame *BankManager::fetchFrame(uint32 index) {
	debug(9, "BankManager::fetchFrame(%d)", index);
	if (index >= MAX_FRAMES_NUMBER) {
		error("BankManager::fetchFrame() invalid frame index = %d", index);
	}
	return &_frames[index];
}


void BankManager::eraseFrame(uint32 index) {
	debug(9, "BankManager::eraseFrame(%d)", index);
	BobFrame *pbf = &_frames[index];
	delete[] pbf->data;
	memset(pbf, 0, sizeof(BobFrame));
}


void BankManager::eraseAllFrames(bool joe) {
    uint32 i = 0;
	if (!joe) {
		i = FRAMES_JOE + FRAMES_JOE_XTRA;
	}
	while (i < 256) {
		eraseFrame(i);
		++i;
	}
}



Graphics::Graphics(QueenEngine *vm)
	: _cameraBob(0), _vm(vm) {
	memset(_bobs, 0, sizeof(_bobs));
	memset(_sortedBobs, 0, sizeof(_sortedBobs));
	_sortedBobsCount = 0;
	_shrinkBuffer.data = new uint8[ BOB_SHRINK_BUF_SIZE ];
}


Graphics::~Graphics() {
	delete[] _shrinkBuffer.data;
}


void Graphics::bobSetupControl() {
	_vm->bankMan()->load("control.BBK",17);
	_vm->bankMan()->unpack(1, 1, 17); // Mouse pointer
	_vm->bankMan()->unpack(3, 3, 17); // Up arrow dialogue
	_vm->bankMan()->unpack(4, 4, 17); // Down arrow dialogue
	_vm->bankMan()->close(17);

	BobFrame *bf = _vm->bankMan()->fetchFrame(1);
	_vm->display()->setMouseCursor(bf->data, bf->width, bf->height, bf->xhotspot, bf->yhotspot);
}


void Graphics::bobDraw(const BobSlot *bs, int16 x, int16 y) {
	debug(9, "Graphics::bobDraw(%d, %d, %d)", bs->frameNum, x, y);

	uint16 w, h;
	BobFrame *pbf = _vm->bankMan()->fetchFrame(bs->frameNum);
	if (bs->scale < 100) {
		bobShrink(pbf, bs->scale);
		pbf = &_shrinkBuffer;
	}
	w = pbf->width;
	h = pbf->height;

	const Box *box = &bs->box;

	if(w != 0 && h != 0 && box->intersects(x, y, w, h)) {
		uint8 *src = pbf->data;
		uint16 x_skip = 0;
		uint16 y_skip = 0;
		uint16 w_new = w;
		uint16 h_new = h;

		// compute bounding box intersection with frame
		if (x < box->x1) {
			x_skip = box->x1 - x;
			w_new -= x_skip;
			x = box->x1;
		}

		if (y < box->y1) {
			y_skip = box->y1 - y;
			h_new -= y_skip;
			y = box->y1;
		}

		if (x + w_new > box->x2 + 1) {
			w_new = box->x2 - x + 1;
		}

		if (y + h_new > box->y2 + 1) {
			h_new = box->y2 - y + 1;
		}

		src += w * y_skip;
		if (!bs->xflip) {
			src += x_skip;
		} else {
			src += w - w_new - x_skip;
			x += w_new - 1;
		}
		_vm->display()->drawBobSprite(src, x, y, w_new, h_new, w, bs->xflip);
	}
}


void Graphics::bobDrawInventoryItem(uint32 frameNum, uint16 x, uint16 y) {
	if (frameNum != 0) {
		BobFrame *bf = _vm->bankMan()->fetchFrame(frameNum);
		_vm->display()->drawInventoryItem(bf->data, x, y, bf->width, bf->height);
	} else {
		_vm->display()->drawInventoryItem(NULL, x, y, 32, 32);
	}
}


void Graphics::bobPaste(uint16 objNum, uint16 image) {
	GraphicData *pgd = _vm->logic()->graphicData(objNum);
	_vm->bankMan()->unpack(pgd->firstFrame, image, 15);
	BobFrame *bf = _vm->bankMan()->fetchFrame(image);
	_vm->display()->drawBobPasteDown(bf->data, pgd->x, pgd->y, bf->width, bf->height);
	_vm->bankMan()->eraseFrame(image);
}


void Graphics::bobShrink(const BobFrame *bf, uint16 percentage) {
	// computing new size, rounding to upper value
	uint16 new_w = (bf->width  * percentage + 50) / 100;
	uint16 new_h = (bf->height * percentage + 50) / 100;

	debug(9, "Graphics::bobShrink() - scale = %d, bufsize = %d", percentage, new_w * new_h);

	if (new_w != 0 && new_h != 0) {

		_shrinkBuffer.width  = new_w;
		_shrinkBuffer.height = new_h;

		uint32 shrinker = (100 << 0x10) / percentage;
		uint16 x_scale = shrinker >> 0x10;
		uint16 y_scale = x_scale * bf->width;   
		shrinker &= 0xFFFF;

		uint8* src = bf->data;
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
				src += bf->width;
				y_count &= 0xFFFF;
			}
		}
	}
}


void Graphics::bobClear(uint32 bobNum) {
	BobSlot *pbs = bob(bobNum);
	pbs->clear();
	if (_vm->display()->fullscreen()) {
		pbs->box.y2 = GAME_SCREEN_HEIGHT - 1;
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
				if (pbs->frameNum > 500) { // SFX frame
					_vm->sound()->playSfx(_vm->logic()->currentRoomSfx());
					pbs->frameNum -= 500;
				}
			}
			if (pbs->moving) {
				int16 j;
				for (j = 0; pbs->moving && j < pbs->speed; ++j) {
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

			BobFrame *pbf = _vm->bankMan()->fetchFrame(pbs->frameNum);
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
			x = pbs->x - xh - _vm->display()->horizontalScroll();
			y = pbs->y - yh;

			bobDraw(pbs, x, y);
		}
	}
}


void Graphics::bobClearAll() {
	for(int32 i = 0; i < ARRAYSIZE(_bobs); ++i) {
		bobClear(i);
	}
}


void Graphics::bobStopAll() {
	for(int32 i = 0; i < ARRAYSIZE(_bobs); ++i) {
		_bobs[i].moving = false;
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
	uint16 screenScroll = _vm->display()->horizontalScroll();
	switch (roomNum) {
	case ROOM_AMAZON_HIDEOUT:
		_bobs[8].x = 250 - screenScroll / 2;
		break;
	case ROOM_TEMPLE_MAZE_5:
		_bobs[5].x = 410 - screenScroll / 2;
		_bobs[6].x = 790 - screenScroll / 2;
		break;
	case ROOM_TEMPLE_OUTSIDE:
		_bobs[5].x = 320 - screenScroll / 2;
		break;
	case ROOM_TEMPLE_TREE:
		_bobs[5].x = 280 - screenScroll / 2;
		break;
	case ROOM_VALLEY_CARCASS:
		_bobs[5].x = 600 - screenScroll / 2;
		break;
	case ROOM_HOTEL_LOBBY:
		if(_vm->display()->fullscreen()) {
			for(i = 1; i <= 3; ++i) {
				_bobs[i].box.y2 = 199;
			}
			_bobs[24].box.y2 = 199;
		}
		break;
	case ROOM_UNUSED_INTRO_1:
		_bobs[5].x = 340 - screenScroll / 2;
		_bobs[6].x = 50 - screenScroll / 2;
		_bobs[7].x = 79 - screenScroll / 2;
		for(i = 1; i <= 8; ++i) {
			_bobs[i].box.y2 = 199;
		}
		_bobs[20].box.y2 = 199;
		break;
	case ROOM_UNUSED_INTRO_5:
		for(i = 0; i < 3; ++i) {
			_bobs[i].box.y2 = 199;
		}
		break;
	case ROOM_CAR_CHASE:
		_vm->bam()->updateCarAnimation();
		break;
	case ROOM_FINAL_FIGHT:
		_vm->bam()->updateFightAnimation();
		break;
	case ROOM_INTRO_RITA_JOE_HEADS: // CR 2 - CD-Rom pan right while Rita talks...
		_cameraBob = -1;
		if (screenScroll < 80) {
			_vm->display()->horizontalScroll(screenScroll + 4);
			// Joe's body and head
			_bobs[ 1].x += 4;
			_bobs[20].x += 4;
			// Rita's body and head
			_bobs[ 2].x -= 2;
			_bobs[21].x -= 2;
		}
		break;
	case ROOM_INTRO_EXPLOSION: // CR 2 - CD-Rom the guys move off screen
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


void Graphics::textSetCentered(uint16 y, const char *text, bool outlined) {
	uint16 x = (GAME_SCREEN_WIDTH - textWidth(text)) / 2;
	textSet(x, y, text, outlined);
}


void Graphics::textDrawAll() {
	int y;
	for (y = GAME_SCREEN_HEIGHT - 1; y > 0; --y) {
		const TextSlot *pts = &_texts[y];
		if (!pts->text.isEmpty()) {
			_vm->display()->drawText(pts->x, y, pts->color, pts->text.c_str(), pts->outlined);
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
	return _vm->display()->textWidth(text);
}


void Graphics::fillAnimBuffer(const char *anim, AnimFrame *af) {
	while (true) {
		sscanf(anim, "%3hu,%3hu", &af->frame, &af->speed);
		if (af->frame == 0)
			break;
		anim += 8;
		++af;
	}
}


uint16 Graphics::countAnimFrames(const char *anim) {
	AnimFrame afbuf[30];
	fillAnimBuffer(anim, afbuf);

	bool frames[256];
	memset(frames, 0, sizeof(frames));
	uint16 count = 0;
	AnimFrame *af = afbuf;
	for ( ; af->frame != 0; ++af) {
		uint16 frameNum = af->frame;
		if (frameNum > 500) {
			frameNum -= 500;
		}
		if (!frames[frameNum]) {
			frames[frameNum] = true;
			++count;
		}
	}
	return count;
}


void Graphics::setupObjectAnim(const GraphicData *gd, uint16 firstImage, uint16 bobNum, bool visible) {
	int16 tempFrames[20];
	memset(tempFrames, 0, sizeof(tempFrames));
	uint16 numTempFrames = 0;
	uint16 i, j;
	for (i = 1; i <= _vm->logic()->graphicAnimCount(); ++i) {
		const GraphicAnim *pga = _vm->logic()->graphicAnim(i);
		if (pga->keyFrame == gd->firstFrame) {
			int16 frame = pga->frame;
			if (frame > 500) { // SFX
				frame -= 500;
			}
			bool foundMatchingFrame = false;
			for (j = 0; j < numTempFrames; ++j) {
				if (tempFrames[j] == frame) {
					foundMatchingFrame = true;
					break;
				}
			}
			if (!foundMatchingFrame) {
				assert(numTempFrames < 20);
				tempFrames[numTempFrames] = frame;
				++numTempFrames;
			}
		}
	}

	// sort found frames ascending
	bool swap = true;
	while (swap) {
		swap = false;
		for (i = 0; i < numTempFrames - 1; ++i) {
			if (tempFrames[i] > tempFrames[i + 1]) {
				SWAP(tempFrames[i], tempFrames[i + 1]);
				swap = true;
			}
		}
	}

	// queen.c l.962-980 / l.1269-1294
	for (i = 0; i < gd->lastFrame; ++i) {
		_vm->bankMan()->unpack(ABS(tempFrames[i]), firstImage + i, 15);
	}
	BobSlot *pbs = bob(bobNum);
	pbs->animating = false;
	if (visible) {
		pbs->curPos(gd->x, gd->y);
		if (tempFrames[0] < 0) {
			pbs->xflip = true;
		}
		AnimFrame *paf = _newAnim[bobNum];
		for (i = 1; i <= _vm->logic()->graphicAnimCount(); ++i) {
			const GraphicAnim *pga = _vm->logic()->graphicAnim(i);
			if (pga->keyFrame == gd->firstFrame) {
				uint16 frameNr = 0;
				for (j = 1; j <= gd->lastFrame; ++j) {
					if (pga->frame > 500) {
						if (pga->frame - 500 == tempFrames[j - 1]) {
							frameNr = j + firstImage - 1 + 500;
						}
					} else if (pga->frame == tempFrames[j - 1]) {
						frameNr = j + firstImage - 1;
					}
				}
				paf->frame = frameNr;
				paf->speed = pga->speed;
				++paf;
			}
		}
		paf->frame = 0;
		paf->speed = 0;
		pbs->animString(_newAnim[bobNum]);
	}
}


uint16 Graphics::setupPersonAnim(const ActorData *ad, const char *anim, uint16 curImage) {
	debug(9, "Graphics::setupPersonAnim(%s, %d)", anim, curImage);
	AnimFrame *animFrames = _newAnim[ad->bobNum];
	fillAnimBuffer(anim, animFrames);
	uint16 frameCount[256];
	memset(frameCount, 0, sizeof(frameCount));
	AnimFrame *af = animFrames;
	for ( ; af->frame != 0; ++af) {
		uint16 frameNum = af->frame;
		if (frameNum > 500) {
			frameNum -= 500;
		}
		if (!frameCount[frameNum]) {
			frameCount[frameNum] = 1;
		}
	}
	uint16 i, n = 1;
	for (i = 1; i < 256; ++i) {
		if (frameCount[i]) {
			frameCount[i] = n;
			++n;
		}
	}
	af = animFrames;
	for ( ; af->frame != 0; ++af) {
		if (af->frame > 500) {
			af->frame = curImage + frameCount[af->frame - 500] + 500;
		} else {
			af->frame = curImage + frameCount[af->frame];
		}
	}

	// unpack necessary frames
	for (i = 1; i < 256; ++i) {
		if (frameCount[i]) {
			++curImage;
			_vm->bankMan()->unpack(i, curImage, ad->bankNum);
		}
	}

	// start animation
	bob(ad->bobNum)->animString(animFrames);
	return curImage;
}


void Graphics::resetPersonAnim(uint16 bobNum) {
	if (_newAnim[bobNum][0].frame != 0) {
		bob(bobNum)->animString(_newAnim[bobNum]);
	}
}


void Graphics::erasePersonAnim(uint16 bobNum) {
	_newAnim[bobNum][0].frame = 0;
	BobSlot *pbs = bob(bobNum);
	pbs->animating = false;
	pbs->anim.string.buffer = NULL;
}


void Graphics::eraseAllAnims() {
	for (int i = 1; i <= 16; ++i) {
		_newAnim[i][0].frame = 0;
	}
}


void Graphics::loadPanel() {
	uint8 *pcxbuf = _vm->resource()->loadFile("panel.pcx");
	if (pcxbuf == NULL) {
		error("Unable to open panel file");
	}
	uint32 size = _vm->resource()->fileSize("panel.pcx");
	_vm->display()->readPCXPanel(pcxbuf, size);
	delete[] pcxbuf;
}


void BamScene::updateCarAnimation() {
	if (_flag != F_STOP) {
		const BamDataBlock *bdb = &_carData[_index];

		// Truck
		_obj1->curPos(bdb->obj1.x, bdb->obj1.y);
		_obj1->frameNum = 40 + bdb->obj1.frame;

		// Rico
		_obj2->curPos(bdb->obj2.x, bdb->obj2.y);
		_obj2->frameNum = 30 + bdb->obj2.frame;

		// FX
		_objfx->curPos(bdb->fx.x, bdb->fx.y);
		_objfx->frameNum = 41 + bdb->fx.frame;

		if (bdb->sfx < 0) {
			_vm->sound()->playSong(-bdb->sfx);
		}

		if (bdb->sfx == 99) {
			_index = 0;
		}
		else {
			++_index;
		}

		if (bdb->sfx == 2) {
			_vm->sound()->playSfx(_vm->logic()->currentRoomSfx());
		}
	}
}


void BamScene::updateFightAnimation() {
	if (_flag != F_STOP) {
		const BamDataBlock *bdb = &_fightData[_index];

		// Frank
		_obj1->curPos(bdb->obj1.x, bdb->obj1.y);
		_obj1->frameNum = 40 + ABS(bdb->obj1.frame);
		_obj1->xflip = (bdb->obj1.frame < 0);

		// Robot
		_obj2->curPos(bdb->obj2.x, bdb->obj2.y);
		_obj2->frameNum = 40 + ABS(bdb->obj2.frame);
		_obj2->xflip = (bdb->obj2.frame < 0);
	
		// FX
		_objfx->curPos(bdb->fx.x, bdb->fx.y);
		_objfx->frameNum = 40 + ABS(bdb->fx.frame);
		_objfx->xflip = (bdb->fx.frame < 0);

		if (bdb->sfx < 0) {
			_vm->sound()->playSong(-bdb->sfx);
		}

		++_index;
		switch (bdb->sfx) {
		case 0: // nothing, so reset shaked screen if necessary
			if (_screenShaked) {
				OSystem::instance()->set_shake_pos(0);
				_screenShaked = false;
			}
			break;
		case 1: // shake screen
			OSystem::instance()->set_shake_pos(3);
			_screenShaked = true;
			break;
		case 2: // play background sfx
			_vm->sound()->playSfx(_vm->logic()->currentRoomSfx());
			break;
		case 3: // play background sfx and shake screen
			_vm->sound()->playSfx(_vm->logic()->currentRoomSfx());
			OSystem::instance()->set_shake_pos(3);
			_screenShaked = true;
			break;
		case 99: // end of BAM data
			_index = 0;
			const BamDataBlock *data[] = {
				_fight1Data, 
				_fight2Data,
				_fight3Data
			};
			_fightData = data[_vm->randomizer.getRandomNumber(2)];
			if (_flag == F_REQ_STOP) {
				_flag = F_STOP;
			}
			break;
		}
	}
}


void Graphics::update(uint16 room) {
	bobSortAll();
	if (_cameraBob >= 0) {
		_vm->display()->horizontalScrollUpdate(_bobs[_cameraBob].x);
	}
	bobCustomParallax(room);
	_vm->display()->prepareUpdate();
	bobDrawAll();
	textDrawAll();
}

void Graphics::bobSetText(
		BobSlot *pbs, 
		const char *text, 
		int textX, int textY, 
		int color, int flags) {
	// function MAKE_SPEAK_BOB, lines 335-457 in talk.c

	if (text[0] == '\0')
		return;

	// debug(0, "makeSpeakBob('%s', (%i,%i), %i, %i, %i, %i);", 
	//		text, bob->x, bob->y, textX, textY, color, flags);

	// Duplicate string and append zero if needed

	char textCopy[MAX_STRING_SIZE];

	int length = strlen(text);
	memcpy(textCopy, text, length);

	if (textCopy[length - 1] >= 'A')
		textCopy[length++] = '.';

	textCopy[length] = '\0';

	// Split text into lines

	char lines[8][MAX_STRING_SIZE];
	int lineCount = 0;
	int wordCount = 0;
	int lineLength = 0;
	int i;

	for (i = 0; i < length; i++) {
		if (textCopy[i] == ' ')
			wordCount++;

		lineLength++;

		if ((lineLength > 20 && textCopy[i] == ' ') || i == (length-1)) {
			memcpy(lines[lineCount], textCopy + i + 1 - lineLength, lineLength);
			lines[lineCount][lineLength] = '\0';
			lineCount++;
			lineLength = 0;
		}
	}


	// Plan: write each line to Screen 2, put black outline around lines and
	// pick them up as a BOB.


	// Find width of widest line 

	int maxLineWidth = 0;

	for (i = 0; i < lineCount; i++) {
		int width = textWidth(lines[i]);
		if (maxLineWidth < width)
			maxLineWidth = width;
	}

	// Calc text position

	short x, y, width, height;

	if (flags) {
		if (flags == 2)
			x = 160 - maxLineWidth / 2;
		else
			x = textX;

		y = textY;

		width = 0;
	} else {
		x = pbs->x;
		y = pbs->y;

		BobFrame *pbf = _vm->bankMan()->fetchFrame(pbs->frameNum);

		width  = (pbf->width  * pbs->scale) / 100;
		height = (pbf->height * pbs->scale) / 100;

		y = y - height - 16 - lineCount * 9;
	}

	x -= _vm->display()->horizontalScroll();

	if (y < 0) {
		y = 0;

		if (x < 160)
			x += width / 2;
		else
			x -= width / 2 + maxLineWidth;
	} else if (!flags)
		x -= maxLineWidth / 2;

	if (x < 0)
		x = 4;
	else if ((x + maxLineWidth) > 320)
		x = 320 - maxLineWidth - 4;

	textCurrentColor(color);

	for (i = 0; i < lineCount; i++) {
		int lineX = x + (maxLineWidth - textWidth(lines[i])) / 2;

		//debug(0, "Setting text '%s' at (%i, %i)", lines[i], lineX, y + 9 * i);
		textSet(lineX, y + 9 * i, lines[i]);
	}
}

int Graphics::textCenterX(const char *text) const {
	return 160 - textWidth(text) / 2;
}



BamScene::BamScene(QueenEngine *vm)
	: _flag(F_STOP), _screenShaked(false), _fightData(_fight1Data), _vm(vm) {
}


void BamScene::prepareAnimation() {
	_obj1 = _vm->graphics()->bob(BOB_OBJ1);
	_vm->graphics()->bobClear(BOB_OBJ1);
	_obj1->active = true;

	_obj2 = _vm->graphics()->bob(BOB_OBJ2);
	_vm->graphics()->bobClear(BOB_OBJ2);
	_obj2->active = true;

	_objfx = _vm->graphics()->bob(BOB_FX);
	_vm->graphics()->bobClear(BOB_FX);
	_objfx->active = true;

	_index = 0;
}


const BamDataBlock BamScene::_carData[] = {
	{ { 310, 105, 1 }, { 314, 106, 17 }, { 366, 101,  1 },  0 },
	{ { 303, 105, 1 }, { 307, 106, 17 }, { 214,   0, 10 },  0 },
	{ { 297, 104, 1 }, { 301, 105, 17 }, { 214,   0, 10 },  0 },
	{ { 294, 103, 1 }, { 298, 104, 17 }, { 214,   0, 10 },  0 },
	{ { 291, 102, 1 }, { 295, 103, 18 }, { 214,   0, 10 },  0 },
	{ { 287, 101, 1 }, { 291, 102, 18 }, { 266,  51, 10 },  2 },
	{ { 283, 100, 1 }, { 287, 101, 19 }, { 279,  47, 11 },  0 },
	{ { 279,  99, 1 }, { 283, 100, 20 }, { 294,  46, 12 },  0 },
	{ { 274,  98, 1 }, { 278,  99, 20 }, { 305,  44, 13 },  0 },
	{ { 269,  98, 1 }, { 273,  99, 20 }, { 320,  42, 14 },  0 },
	{ { 264,  98, 1 }, { 268,  99, 17 }, { 214,   0, 10 },  0 },
	{ { 261,  98, 1 }, { 265,  99, 17 }, { 214,   0, 10 },  0 },
	{ { 259,  98, 1 }, { 263,  99, 17 }, { 214,   0, 10 },  0 },
	{ { 258,  98, 1 }, { 262,  99, 17 }, { 214,   0, 10 },  0 },
	{ { 257,  98, 2 }, { 260,  99, 17 }, { 214,   0, 10 },  0 },
	{ { 255,  99, 3 }, { 258, 100, 17 }, { 214,   0, 10 },  0 },
	{ { 258,  99, 4 }, { 257, 100, 17 }, { 214,   0, 10 },  0 },
	{ { 264, 102, 4 }, { 263, 103, 17 }, { 214,   0, 10 },  0 },
	{ { 272, 105, 5 }, { 274, 106, 17 }, { 214,   0, 10 },  0 },
	{ { 276, 107, 5 }, { 277, 108, 17 }, { 214,   0, 10 },  0 },
	{ { 283, 108, 5 }, { 284, 109, 17 }, { 214,   0, 10 },  0 },
	{ { 288, 109, 5 }, { 288, 110, 17 }, { 214,   0, 10 },  0 },
	{ { 293, 110, 5 }, { 293, 111, 18 }, { 266,  59, 10 },  2 },
	{ { 298, 110, 5 }, { 299, 111, 18 }, { 277,  56, 11 },  0 },
	{ { 303, 110, 5 }, { 304, 111, 19 }, { 285,  55, 12 },  0 },
	{ { 308, 110, 4 }, { 307, 111, 20 }, { 296,  54, 13 },  0 },
	{ { 309, 110, 3 }, { 312, 111, 20 }, { 304,  53, 14 },  0 },
	{ { 310, 110, 3 }, { 313, 111, 20 }, { 214,   0, 10 },  0 },
	{ { 311, 110, 3 }, { 314, 111, 17 }, { 214,   0, 10 },  0 },
	{ { 309, 110, 2 }, { 312, 111, 17 }, { 214,   0, 10 },  0 },
	{ { 304, 111, 2 }, { 307, 112, 17 }, { 214,   0, 10 },  0 },
	{ { 300, 110, 2 }, { 303, 111, 17 }, { 214,   0, 10 },  0 },
	{ { 296, 109, 2 }, { 299, 110, 17 }, { 214,   0, 10 },  0 },
	{ { 292, 108, 1 }, { 296, 109, 17 }, { 214,   0, 10 },  0 },
	{ { 285, 107, 2 }, { 289, 108, 17 }, { 214,   0, 10 },  0 },
	{ { 282, 107, 3 }, { 285, 108, 17 }, { 214,   0, 10 },  0 },
	{ { 278, 107, 4 }, { 277, 108, 18 }, { 214,   0, 10 },  0 },
	{ { 279, 108, 4 }, { 278, 109, 18 }, { 252,  57, 10 },  2 },
	{ { 281, 108, 5 }, { 283, 109, 18 }, { 265,  55, 11 },  0 },
	{ { 284, 109, 5 }, { 285, 110, 19 }, { 277,  55, 12 },  0 },
	{ { 287, 110, 5 }, { 288, 111, 20 }, { 288,  54, 13 },  0 },
	{ { 289, 111, 5 }, { 290, 112, 20 }, { 299,  54, 14 },  0 },
	{ { 291, 112, 4 }, { 290, 113, 20 }, { 214,   0, 10 },  0 },
	{ { 293, 113, 3 }, { 295, 114, 17 }, { 214,   0, 10 },  0 },
	{ { 296, 114, 2 }, { 299, 115, 17 }, { 214,   0, 10 },  0 },
	{ { 295, 115, 2 }, { 298, 116, 17 }, { 214,   0, 10 },  0 },
	{ { 293, 116, 1 }, { 297, 117, 17 }, { 214,   0, 10 },  0 },
	{ { 289, 116, 1 }, { 292, 117, 17 }, { 214,   0, 10 },  0 },
	{ { 285, 115, 1 }, { 289, 116, 17 }, { 214,   0, 10 },  0 },
	{ { 281, 114, 1 }, { 284, 115, 17 }, { 214,   0, 10 },  0 },
	{ { 277, 113, 1 }, { 280, 114, 17 }, { 214,   0, 10 },  0 },
	{ { 274, 112, 1 }, { 277, 113, 17 }, { 214,   0, 10 },  0 },
	{ { 271, 111, 1 }, { 274, 112, 17 }, { 214,   0, 10 },  0 },
	{ { 267, 110, 1 }, { 270, 111, 17 }, { 214,   0, 10 },  0 },
	{ { 263, 109, 1 }, { 266, 110, 17 }, { 214,   0, 10 },  0 },
	{ { 260, 108, 1 }, { 263, 109, 17 }, { 214,   0, 10 },  0 },
	{ { 254, 108, 2 }, { 256, 109, 17 }, { 214,   0, 10 },  0 },
	{ { 252, 107, 3 }, { 254, 108, 17 }, { 214,   0, 10 },  0 },
	{ { 253, 108, 3 }, { 255, 109, 17 }, { 214,   0, 10 },  0 },
	{ { 255, 109, 3 }, { 257, 110, 18 }, { 231,  59, 10 },  2 },
	{ { 258, 111, 3 }, { 260, 112, 18 }, { 242,  57, 11 },  0 },
	{ { 263, 112, 4 }, { 262, 113, 19 }, { 256,  57, 12 },  0 },
	{ { 270, 111, 4 }, { 269, 112, 20 }, { 267,  57, 13 },  0 },
	{ { 274, 112, 5 }, { 276, 113, 20 }, { 281,  56, 14 },  0 },
	{ { 280, 111, 6 }, { 282, 112, 19 }, { 214,   0, 10 },  0 },
	{ { 284, 109, 6 }, { 285, 110, 17 }, { 214,   0, 10 },  0 },
	{ { 289, 108, 6 }, { 291, 109, 17 }, { 214,   0, 10 },  0 },
	{ { 294, 107, 6 }, { 296, 108, 17 }, { 214,   0, 10 },  0 },
	{ { 294, 107, 5 }, { 296, 108, 18 }, { 272,  57, 10 },  2 },
	{ { 295, 107, 5 }, { 297, 108, 18 }, { 282,  57, 11 },  0 },
	{ { 296, 108, 5 }, { 298, 109, 19 }, { 295,  57, 12 },  0 },
	{ { 300, 108, 4 }, { 299, 109, 20 }, { 303,  57, 13 },  0 },
	{ { 303, 108, 3 }, { 306, 109, 20 }, { 313,  57, 14 },  0 },
	{ { 307, 109, 2 }, { 311, 110, 17 }, { 214,   0, 10 },  0 },
	{ { 310, 110, 1 }, { 314, 111, 17 }, { 214,   0, 10 }, 99 }
};

const BamDataBlock BamScene::_fight1Data[] = {
	{ {  75,  96,  1 }, { 187, 96, -23 }, {  58,  37, 46 },  0 },
	{ {  75,  96,  2 }, { 187, 96, -23 }, {  58,  37, 46 },  0 },
	{ {  75,  96,  3 }, { 187, 96, -23 }, {  58,  37, 46 },  0 },
	{ {  75,  96,  4 }, { 187, 96, -23 }, {  58,  37, 46 },  0 },
	{ {  75,  96,  1 }, { 187, 96, -23 }, {  58,  37, 46 },  0 },
	{ {  75,  96,  2 }, { 187, 96, -23 }, {  58,  37, 46 },  0 },
	{ {  75,  96,  3 }, { 187, 96, -23 }, {  58,  37, 46 },  0 },
	{ {  75,  96,  4 }, { 187, 96, -24 }, {  58,  37, 46 },  0 },
	{ {  79,  96,  1 }, { 187, 96, -24 }, {  58,  37, 46 },  0 },
	{ {  85,  96,  2 }, { 187, 96, -24 }, {  58,  37, 46 },  0 },
	{ {  94,  96,  3 }, { 187, 96, -24 }, {  58,  37, 46 },  0 },
	{ { 100,  96,  4 }, { 187, 96, -24 }, {  58,  37, 46 },  0 },
	{ { 113,  96,  1 }, { 187, 96, -25 }, {  58,  37, 46 },  0 },
	{ { 121,  96,  1 }, { 187, 96, -25 }, {  58,  37, 46 },  0 },
	{ { 136,  96, 16 }, { 187, 96, -26 }, {  58,  37, 46 },  0 },
	{ { 151,  93,  6 }, { 187, 96, -27 }, {  58,  37, 46 },  0 },
	{ { 159,  83, 16 }, { 187, 96, -28 }, {  58,  37, 46 },  0 },
	{ { 170,  73, 16 }, { 187, 96, -29 }, { 182,  96, 48 },  3 },
	{ { 176,  69, 13 }, { 187, 96, -31 }, { 182,  94, 49 },  1 },
	{ { 168,  66, 13 }, { 187, 98, -32 }, { 182,  92, 50 },  0 },
	{ { 155,  75, 13 }, { 187, 96, -32 }, { 182,  88, 51 },  3 },
	{ { 145,  86, 13 }, { 187, 98, -32 }, { 182,  85, 52 },  0 },
	{ { 127, 104, 13 }, { 187, 98, -32 }, { 182,  25, 52 },  1 },
	{ { 122, 108, 13 }, { 187, 98, -32 }, { 182,  25, 52 },  1 },
	{ { 120, 104, 14 }, { 187, 96, -34 }, { 107, 145, 42 },  2 },
	{ { 111, 103, 13 }, { 187, 96, -23 }, { 107, 144, 43 },  0 },
	{ { 102, 105, 13 }, { 187, 96, -23 }, { 107, 142, 43 },  0 },
	{ {  97, 107, 13 }, { 187, 96, -23 }, { 107, 139, 44 },  0 },
	{ {  92, 101, 14 }, { 187, 96, -23 }, { 107,  34, 47 },  3 },
	{ {  90, 105, 14 }, { 187, 96, -23 }, { 107,  34, 47 },  0 },
	{ {  88, 104, 14 }, { 187, 96, -23 }, { 107,  34, 47 },  0 },
	{ {  87, 105, 14 }, { 187, 96, -23 }, { 107,  34, 47 },  0 },
	{ {  86, 105, 14 }, { 187, 96, -23 }, { 107,  34, 47 },  0 },
	{ {  86, 105, 14 }, { 187, 96, -23 }, { 107,  34, 47 },  0 },
	{ {  86, 105, 15 }, { 187, 96, -23 }, {   0,   0,  0 },  0 },
	{ {  85,  98, 16 }, { 187, 96, -23 }, {   0,   0,  0 },  0 },
	{ {  92,  96,  1 }, { 187, 96, -23 }, {   0,   0,  0 },  0 },
	{ {  92,  96,  1 }, { 187, 96, -23 }, {   0,   0,  0 },  0 },
	{ {  89,  96,  4 }, { 187, 96, -23 }, {   0,   0,  0 },  0 },
	{ {  86,  96,  3 }, { 187, 96, -23 }, {   0,   0,  0 },  0 },
	{ {  83,  96,  2 }, { 187, 96, -23 }, {   0,   0,  0 },  0 },
	{ {  81,  96,  1 }, { 187, 96, -23 }, {   0,   0,  0 },  0 },
	{ {  78,  96,  4 }, { 187, 96, -23 }, {   0,   0,  0 },  0 },
	{ {  75,  96,  3 }, { 187, 96, -23 }, {   0,   0,  0 },  0 },
	{ {  75,  96,  1 }, { 187, 96, -23 }, {   0,   0,  0 },  0 },
	{ {  75,  96,  1 }, { 187, 96, -23 }, {   0,   0,  0 }, 99 }
};

const BamDataBlock BamScene::_fight2Data[] = {
	{ {  75, 96,  1 }, { 187, 96, -23 }, { 150,  45, 35 },  0 },
	{ {  78, 96,  2 }, { 187, 96, -23 }, { 150,  45, 35 },  0 },
	{ {  81, 96,  3 }, { 189, 96, -18 }, { 150,  45, 35 },  0 },
	{ {  84, 96,  4 }, { 183, 96, -19 }, { 150,  45, 35 },  0 },
	{ {  87, 96,  1 }, { 181, 96, -20 }, { 150,  45, 35 },  0 },
	{ {  90, 96,  2 }, { 177, 96, -21 }, { 150,  45, 35 },  0 },
	{ {  93, 96,  3 }, { 171, 96, -22 }, { 150,  45, 35 },  0 },
	{ {  96, 96,  4 }, { 169, 96, -17 }, { 150,  45, 35 },  0 },
	{ {  99, 96,  1 }, { 165, 96, -18 }, { 150,  45, 35 },  0 },
	{ { 102, 96,  2 }, { 159, 96, -19 }, { 150,  45, 35 },  0 },
	{ { 105, 96,  3 }, { 157, 96, -20 }, { 150,  45, 35 },  0 },
	{ { 108, 96,  4 }, { 153, 96, -21 }, { 150,  45, 35 },  0 },
	{ { 111, 96,  1 }, { 147, 96, -22 }, { 150,  45, 35 },  0 },
	{ { 114, 96,  2 }, { 147, 96, -23 }, { 150,  45, 35 },  0 },
	{ { 117, 96,  3 }, { 147, 96, -23 }, { 150,  45, 35 },  0 },
	{ { 120, 96,  4 }, { 147, 96, -24 }, { 150,  45, 35 },  0 },
	{ { 123, 96,  1 }, { 147, 96, -25 }, { 150,  45, 35 },  0 },
	{ { 125, 96,  2 }, { 147, 96, -25 }, { 150,  45, 35 },  0 },
	{ { 127, 96, 12 }, { 147, 96, -69 }, { 122,  94, 36 },  3 },
	{ { 127, 95, 11 }, { 147, 96, -70 }, { 122,  94, 41 },  0 },
	{ { 127, 96, 12 }, { 147, 96, -71 }, { 122, 100, 36 },  3 },
	{ { 127, 97, 11 }, { 147, 96, -69 }, { 122, 100, 41 },  0 },
	{ { 127, 96, 12 }, { 147, 96, -70 }, { 127, 103, 36 },  3 },
	{ { 127, 95, 11 }, { 147, 96, -71 }, { 127, 103, 41 },  0 },
	{ { 127, 94, 12 }, { 147, 96, -69 }, { 123,  94, 36 },  3 },
	{ { 127, 95, 11 }, { 147, 96, -70 }, { 123,  94, 41 },  0 },
	{ { 127, 96, 12 }, { 147, 96, -71 }, { 120,  99, 36 },  3 },
	{ { 127, 96, 12 }, { 147, 96, -71 }, { 115,  98, 41 },  0 },
	{ { 117, 93, 11 }, { 147, 96, -25 }, { 115, 134, 42 },  0 },
	{ { 110, 92, 11 }, { 147, 96, -25 }, { 114, 133, 42 },  0 },
	{ { 102, 93, 11 }, { 147, 96, -25 }, { 114, 131, 43 },  0 },
	{ {  92, 93, 11 }, { 147, 96, -25 }, { 114, 130, 43 },  0 },
	{ {  82, 94, 11 }, { 147, 96, -25 }, { 114, 128, 44 },  0 },
	{ {  76, 95, 11 }, { 147, 96, -25 }, { 114, 127, 44 },  0 },
	{ {  70, 96, 11 }, { 147, 96, -25 }, { 114, 126, 45 },  0 },
	{ {  75, 96,  5 }, { 147, 96, -25 }, { 114, 125, 46 },  1 },
	{ {  75, 96,  6 }, { 147, 96, -25 }, { 114,  43, 46 },  0 },
	{ {  75, 96,  6 }, { 147, 96, -25 }, { 114,  43, 46 },  0 },
	{ {  75, 96,  5 }, { 147, 96, -25 }, { 114,  43, 46 },  0 },
	{ {  75, 96,  7 }, { 147, 96, -25 }, { 114,  43, 46 },  0 },
	{ {  75, 96, 68 }, { 147, 96, -25 }, { 114,  43, 46 },  0 },
	{ {  75, 96, 68 }, { 147, 96, -25 }, {  89, 104, 36 },  2 },
	{ {  75, 96, 68 }, { 147, 96, -25 }, {  94, 103, 62 },  0 },
	{ {  75, 96, 68 }, { 147, 96, -25 }, { 122, 103, 63 },  0 },
	{ {  75, 96, 68 }, { 147, 96, -25 }, { 141, 103, 64 },  0 },
	{ {  75, 96, 68 }, { 147, 96, -30 }, { 150, 103, 65 },  3 },
	{ {  75, 96, 68 }, { 156, 96, -30 }, { 160, 103, 66 },  0 },
	{ {  75, 96,  7 }, { 164, 96, -30 }, { 169, 103, 67 },  0 },
	{ {  75, 96,  5 }, { 169, 96, -30 }, { 177, 103, 48 },  3 },
	{ {  75, 96,  5 }, { 173, 96, -30 }, { 185, 103, 49 },  0 },
	{ {  75, 96,  6 }, { 178, 96, -30 }, { 198, 103, 50 },  0 },
	{ {  75, 96,  6 }, { 181, 96, -30 }, { 207, 103, 51 },  1 },
	{ {  75, 96,  5 }, { 184, 96, -30 }, { 221, 103, 52 },  0 },
	{ {  75, 96,  5 }, { 186, 96, -30 }, { 224,  53, 53 },  0 },
	{ {  75, 96,  5 }, { 187, 96, -23 }, { 224,  53, 53 }, 99 }
};

const BamDataBlock BamScene::_fight3Data[] = {
	{ {  75, 96,  1 }, { 187,  96, -23 }, { 150,  45, 35 },  0 },
	{ {  77, 96,  2 }, { 187,  96, -22 }, { 150,  45, 35 },  0 },
	{ {  80, 96,  3 }, { 185,  96, -17 }, { 150,  45, 35 },  0 },
	{ {  83, 96,  4 }, { 181,  96, -18 }, { 150,  45, 35 },  0 },
	{ {  86, 96,  1 }, { 175,  96, -19 }, { 150,  45, 35 },  0 },
	{ {  88, 96,  2 }, { 173,  96, -20 }, { 150,  45, 35 },  0 },
	{ {  91, 96,  3 }, { 169,  96, -21 }, { 150,  45, 35 },  0 },
	{ {  94, 96,  4 }, { 163,  96, -22 }, { 150,  45, 35 },  0 },
	{ {  97, 96,  1 }, { 161,  96, -17 }, { 150,  45, 35 },  0 },
	{ {  99, 96,  2 }, { 157,  96, -18 }, { 150,  45, 35 },  0 },
	{ { 102, 96,  3 }, { 151,  96, -19 }, { 150,  45, 35 },  0 },
	{ { 105, 96,  4 }, { 149,  96, -20 }, { 150,  45, 35 },  0 },
	{ { 108, 96,  1 }, { 145,  96, -21 }, { 150,  45, 35 },  0 },
	{ { 110, 96,  2 }, { 145,  96, -25 }, { 150,  45, 35 },  0 },
	{ { 113, 96,  3 }, { 145,  96, -26 }, { 132,  96, 36 },  2 },
	{ { 117, 96,  7 }, { 145,  96, -27 }, { 122,  97, 36 },  0 },
	{ { 117, 96,  7 }, { 145,  96, -28 }, { 117,  97, 37 },  0 },
	{ { 116, 96, 12 }, { 145,  96, -24 }, { 110,  96, 38 },  3 },
	{ { 109, 96, 12 }, { 145,  96, -24 }, { 103,  95, 39 },  0 },
	{ { 105, 96, 12 }, { 145,  96, -24 }, {  95,  90, 40 },  1 },
	{ {  96, 96, 11 }, { 145,  96, -24 }, {  86,  80, 41 },  0 },
	{ {  92, 96, 11 }, { 145,  96, -24 }, {  86,  80, 41 },  0 },
	{ {  93, 96,  5 }, { 145,  96, -24 }, {  86,  38, 41 },  0 },
	{ {  91, 96,  5 }, { 145,  96, -24 }, {  86,  38, 41 },  0 },
	{ {  89, 96,  5 }, { 145,  96, -24 }, {  86,  38, 41 },  0 },
	{ {  88, 96,  5 }, { 145,  96, -24 }, {  86,  38, 41 },  0 },
	{ {  87, 96,  6 }, { 145,  96, -24 }, {  86,  38, 41 },  0 },
	{ {  87, 96,  6 }, { 145,  96, -23 }, {  86,  38, 41 },  0 },
	{ {  87, 96,  6 }, { 145,  96, -23 }, {  86,  38, 41 },  0 },
	{ {  87, 96,  5 }, { 145,  96, -23 }, {  86,  38, 41 },  0 },
	{ {  87, 96,  5 }, { 145,  96, -23 }, {  86,  38, 41 },  0 },
	{ {  87, 96,  6 }, { 145,  96, -23 }, {  86,  38, 41 },  0 },
	{ {  87, 96,  6 }, { 145,  96, -23 }, {  86,  38, 41 },  0 },
	{ {  87, 96,  5 }, { 145,  96, -23 }, {  86,  38, 41 },  0 },
	{ {  87, 96,  5 }, { 145,  96, -23 }, {  86,  38, 41 },  0 },
	{ {  87, 96,  6 }, { 145,  96, -23 }, {  86,  38, 41 },  0 },
	{ {  87, 96,  6 }, { 145,  96, -26 }, {  86,  38, 41 },  0 },
	{ {  87, 96,  6 }, { 145,  96, -27 }, { 132,  97, 36 },  2 },
	{ {  87, 96,  5 }, { 145,  96, -28 }, { 118,  97, 36 },  0 },
	{ {  87, 96,  7 }, { 145,  96, -24 }, { 107,  97, 36 },  0 },
	{ {  87, 96,  8 }, { 145,  96, -24 }, { 101,  97, 36 },  0 },
	{ {  87, 96,  9 }, { 145,  96, -23 }, { 102,  97, 66 },  3 },
	{ {  87, 96, 10 }, { 145,  96, -23 }, { 120,  97, 67 },  0 },
	{ {  87, 96, 10 }, { 145,  96, -30 }, { 139,  97, 67 },  1 },
	{ {  87, 96,  7 }, { 146,  96, -30 }, { 144,  97, 62 },  2 },
	{ {  86, 96,  4 }, { 160,  96, -30 }, { 144,  97, 48 },  1 },
	{ {  83, 96,  3 }, { 170,  96, -31 }, { 154,  93, 49 },  0 },
	{ {  80, 96,  2 }, { 174,  96, -31 }, { 161,  89, 50 },  0 },
	{ {  78, 96,  1 }, { 178,  99, -31 }, { 169,  85, 51 },  0 },
	{ {  75, 96,  4 }, { 183, 104, -31 }, { 175,  79, 52 },  0 },
	{ {  75, 96,  1 }, { 185,  99, -32 }, { 180, 144, 42 },  3 },
	{ {  75, 96,  1 }, { 185, 106, -31 }, { 181, 141, 42 },  0 },
	{ {  75, 96,  5 }, { 185, 104, -31 }, { 181, 138, 43 },  0 },
	{ {  75, 96,  5 }, { 188, 106, -31 }, { 182, 135, 43 },  0 },
	{ {  75, 96,  6 }, { 191,  99, -32 }, { 183, 131, 44 },  3 },
	{ {  75, 96,  6 }, { 191,  99, -32 }, { 183, 127, 45 },  0 },
	{ {  75, 96,  5 }, { 191,  99, -32 }, { 184, 121, 46 },  0 },
	{ {  75, 96,  5 }, { 191,  99, -32 }, { 183, 115, 46 },  0 },
	{ {  75, 96,  6 }, { 191,  99, -32 }, { 183,  41, 47 },  0 },
	{ {  75, 96,  6 }, { 191,  99, -32 }, { 183,  41, 47 },  0 },
	{ {  75, 96,  6 }, { 191,  99, -32 }, { 183,  41, 47 },  0 },
	{ {  75, 96,  5 }, { 191,  99, -32 }, { 183,  41, 47 },  0 },
	{ {  75, 96,  5 }, { 191,  99, -32 }, { 183,  41, 47 },  0 },
	{ {  75, 96,  5 }, { 191,  99, -32 }, { 183,  41, 47 },  0 },
	{ {  75, 96,  5 }, { 191,  99, -32 }, { 183,  41, 47 },  0 },
	{ {  75, 96,  5 }, { 191,  99, -32 }, { 183,  41, 47 },  0 },
	{ {  75, 96,  6 }, { 191,  99, -32 }, { 183,  41, 47 },  0 },
	{ {  75, 96,  6 }, { 191,  99, -32 }, { 183,  41, 47 },  0 },
	{ {  75, 96,  5 }, { 195,  98, -33 }, { 183,  41, 47 },  0 },
	{ {  75, 96,  6 }, { 191,  96, -34 }, { 183,  41, 47 },  0 },
	{ {  75, 96,  6 }, { 193,  96, -25 }, { 183,  41, 47 },  0 },
	{ {  75, 96,  5 }, { 193,  96, -24 }, { 183,  41, 47 },  0 },
	{ {  75, 96,  5 }, { 193,  96, -24 }, { 183,  41, 47 },  0 },
	{ {  75, 96,  5 }, { 193,  96, -24 }, { 183,  41, 47 },  0 },
	{ {  75, 96,  6 }, { 191,  96, -18 }, { 183,  41, 47 },  0 },
	{ {  75, 96,  6 }, { 190,  96, -19 }, { 183,  41, 47 },  0 },
	{ {  75, 96,  6 }, { 187,  96, -20 }, { 183,  41, 47 },  0 },
	{ {  75, 96,  1 }, { 187,  96, -23 }, { 183,  41, 47 },  0 },
	{ {  75, 96,  1 }, { 187,  96, -23 }, { 183,  41, 47 }, 99 }
};


} // End of namespace Queen

