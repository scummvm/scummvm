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

#include "queen/graphics.h"


QueenGraphics::QueenGraphics(QueenResource *resource)
	:_resource(resource) {
		
	memset(_frames, 0, sizeof(_frames));
	memset(_banks, 0, sizeof(_banks));
	memset(_sortedBobs, 0, sizeof(_sortedBobs));

}


void QueenGraphics::bankLoad(const char *bankname, uint32 bankslot) {
	
	int16 i;
	
	if (!_resource->exists(bankname)) {
	  error("Unable to open bank '%s'", bankname);	
	}
	bankErase(bankslot);
	_banks[bankslot].data = _resource->loadFile(bankname);
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


void QueenGraphics::bankUnpack(uint32 srcframe, uint32 dstframe, uint32 bankslot) {
	
	uint8 *p = _banks[bankslot].data + _banks[bankslot].indexes[srcframe];
		
	BobFrame *pbf = &_frames[dstframe];
	delete[] pbf->data;

	pbf->width    = READ_LE_UINT16(p + 0);
	pbf->height   = READ_LE_UINT16(p + 2);
	pbf->xhotspot = READ_LE_UINT16(p + 4);
	pbf->yhotspot = READ_LE_UINT16(p + 6);
	
	uint32 size = pbf->width * pbf->height;
	pbf->data = new uint8[ size ];
	memcpy(pbf->data, p, size);
	
	debug(5, "Unpacked frame %d from bank slot %d to frame slot %d", srcframe, bankslot, dstframe);
}


void QueenGraphics::bankOverpack(uint32 srcframe, uint32 dstframe, uint32 bankslot) {
	
	uint8 *p = _banks[bankslot].data + _banks[bankslot].indexes[srcframe];
	uint16 src_w = READ_LE_UINT16(p + 0);
	uint16 src_h = READ_LE_UINT16(p + 2);
	
	// unpack if destination frame is smaller than source one
	if (_frames[dstframe].width < src_w || _frames[dstframe].height < src_h) {
		bankUnpack(srcframe, dstframe, bankslot);
	}
	else {
		// copy data 'over' destination frame (without changing frame header)
		memcpy(_frames[dstframe].data, p, src_w * src_h);
	}
	
	debug(5, "Overpacked frame %d from bank slot %d to frame slot %d", srcframe, bankslot, dstframe);
}


void QueenGraphics::bankErase(uint32 bankslot) {
	
	delete[] _banks[bankslot].data;
	_banks[bankslot].data = 0;
	
	debug(5, "Erased bank in slot %d", bankslot);
}


void QueenGraphics::bobAnimString(uint32 bobnum, uint8* animBuf) {

	BobSlot *pbs = &_bobs[bobnum];
	pbs->active = true;
	pbs->animating = true;
	pbs->anim.string.buffer = animBuf;
	pbs->anim.string.curPos = animBuf;
	pbs->frameNum = READ_LE_UINT16(animBuf);
	pbs->anim.speed = READ_LE_UINT16(animBuf + 2) / 4;
}


void QueenGraphics::bobAnimNormal(uint32 bobnum, uint16 firstFrame, uint16 lastFrame, uint16 speed, bool rebound, bool xflip) {

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


void QueenGraphics::bobMove(uint32 bobnum, uint16 endx, uint16 endy, int16 speed) {

	BobSlot *pbs = &_bobs[bobnum];

	pbs->active = true;
	pbs->moving = true;

	pbs->endx = endx;
	pbs->endy = endy;

	// FIXME: can speed be negative or =0 ? the original sources does the check
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
			anim.string.curPos += 4;
			uint16 nextFrame = READ_LE_UINT16(anim.string.curPos);
			if (nextFrame == 0) {
				anim.string.curPos = anim.string.buffer;
				frameNum = READ_LE_UINT16(anim.string.curPos);
			}
			else {
				frameNum = nextFrame;
			}
			anim.speed = READ_LE_UINT16(anim.string.curPos + 2) / 4;

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
					frameNum = anim.normal.lastFrame - 1;
				}
				frameNum += frameDir;
			}
		}
	}
}
            

void QueenGraphics::bobDraw(uint32 bobnum, uint16 x, uint16 y, uint16 scale, bool xflip, const Box& box) {

	uint16 w, h;
	uint8 *src;

	BobFrame *pbf = &_frames[bobnum];
	if (scale < 100) {  // Note: scale is unsigned, hence always >= 0
		bobShrink(pbf, scale);
		src = _shrinkBuffer.data;
		w   = _shrinkBuffer.width;
		h   = _shrinkBuffer.height;
	}
	else {
		src = pbf->data;
		w   = pbf->width;
		h   = pbf->height;
	}

	if(w != 0 && h != 0 && box.intersects(x, y, w, h)) {

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

		uint16 j;
		for (j = 0; j < h_new; ++j) {
			memset(_ulines[y + j], 2, w_new / 16);
		}

		src += w * y_skip;
		if (!xflip) {
			src += x_skip;
//			_display->blit(hidden, x, y, 320, src, w_new, h_new, w, xflip, true);
		}
		else {
			src += w - w_new - x_skip;
			x += w_new - 1;
//			_display->blit(hidden, x, y, 320, src, x_new, h_new, w, xflip, true);
		}
    }

}


void QueenGraphics::bobDrawInventoryItem(uint32 bobnum, uint16 x, uint16 y) {
	if (bobnum == 0) {
		// clear panel area
		uint8 *p = _panel + y * 320 + x;
		for(uint32 j = 0; j < 32; ++j) {
			for(uint32 i = 0; i < 32; ++i) {
				*p++ = INK_BG_PANEL;
			}
			p += 320 - 32;
		}
	}
	else {
//		BobFrame *pbf = &_frames[bobnum];
//		_display->blit(panel, x, y, 320, pbf->data, pbf->width, pbf->height, false, false);
	}
}


void QueenGraphics::bobPaste(uint32 bobnum, uint16 x, uint16 y) {

//	BobFrame *pbf = &_frames[bobnum];
//	_display->blit(backdrop, x, y, 640, pbf->data, pbf->width, pbf->height, pbf->width, false, true);
	frameErase(bobnum);
}


void QueenGraphics::bobShrink(const BobFrame* pbf, uint16 percentage) {

	// computing new size, rounding to upper value
	uint16 new_w = (pbf->width  * percentage + 50) / 100;
	uint16 new_h = (pbf->height * percentage + 50) / 100;

	if (new_w != 0 && new_h != 0) {

		_shrinkBuffer.width  = new_w;
		_shrinkBuffer.height = new_h;

		uint32 shrinker = (100 << 0x10) / percentage;
		uint16 x_scale = shrinker >> 0x10;
		uint16 y_scale = x_scale * pbf->width;   
		shrinker &= 0xFFFF;

		uint8* src = pbf->data;
		uint8* dst = _shrinkBuffer.data;

		for (uint32 y_count = 0; new_h != 0; --new_h) {

			uint8 *p = src;
			for (uint32 x_count = 0; new_w != 0; --new_w) {
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


void QueenGraphics::bobClear(uint32 bobnum) {

	BobSlot *pbs = &_bobs[bobnum];

	pbs->active = 0;
	pbs->xflip  = false;
	pbs->anim.string.buffer = NULL;
	pbs->moving = false;
	pbs->scale  = 100;
	pbs->box.x1 = 0;
	pbs->box.y1 = 0;
	pbs->box.y1 = 319;
	pbs->box.y2 = (bobnum == 16) ? 199 : 149; // FIXME: does bob number 16 really used ?
}


void QueenGraphics::bobSortAll() {

	int32 _sorted = 0;

	// animate/move the bobs
	for (int32 i = 0; i < ARRAYSIZE(_bobs); ++i) {

		BobSlot *pbs = &_bobs[i];
		if (pbs->active) {
			_sortedBobs[_sorted] = pbs;
			++_sorted;

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

	_sortedBobs[_sorted] = 0;

	// bubble sort the bobs
	for (int32 index = 0; index < _sorted - 1; ++index) {
		int32 smallest = index;
		for (int32 compare = index + 1; compare <= _sorted - 1; ++compare) {
			if (_sortedBobs[compare]->y < _sortedBobs[compare]->y) {
				smallest = compare;
			}
		}
		if (index != smallest) {
			SWAP(_sortedBobs[index], _sortedBobs[smallest]);
		}
	}

}


void QueenGraphics::bobDrawAll() {
	
	for (BobSlot *pbs = _sortedBobs[0]; pbs; ++pbs) {
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


void QueenGraphics::bobClearAll() {

	for(int32 i = 0; i < ARRAYSIZE(_bobs); ++i) {
		bobClear(i);
	}
}


void QueenGraphics::frameErase(uint32 fslot) {

	BobFrame *pbf = &_frames[fslot];
	pbf->width  = 0;
	pbf->height = 0;
	delete[] pbf->data;

}
