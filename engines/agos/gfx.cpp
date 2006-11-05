/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"

#include "agos/agos.h"
#include "agos/intern.h"
#include "agos/vga.h"

namespace AGOS {

byte *vc10_depackColumn(VC10_state * vs) {
	int8 a = vs->depack_cont;
	const byte *src = vs->depack_src;
	byte *dst = vs->depack_dest;
	uint16 dh = vs->dh;
	byte color;

	if (a == -0x80)
		a = *src++;

	for (;;) {
		if (a >= 0) {
			color = *src++;
			do {
				*dst++ = color;
				if (!--dh) {
					if (--a < 0)
						a = -0x80;
					else
						src--;
					goto get_out;
				}
			} while (--a >= 0);
		} else {
			do {
				*dst++ = *src++;
				if (!--dh) {
					if (++a == 0)
						a = -0x80;
					goto get_out;
				}
			} while (++a != 0);
		}
		a = *src++;
	}

get_out:;
	vs->depack_src = src;
	vs->depack_cont = a;
	return vs->depack_dest + vs->y_skip;
}

void vc10_skip_cols(VC10_state *vs) {
	while (vs->x_skip) {
		vc10_depackColumn(vs);
		vs->x_skip--;
	}
}

void AGOSEngine::decodeColumn(byte *dst, const byte *src, int height) {
	const uint pitch = _dxSurfacePitch;
	int8 reps = (int8)0x80;
	byte color;
	byte *dstPtr = dst;
	uint h = height, w = 8;

	for (;;) {
		reps = *src++;
		if (reps >= 0) {
			color = *src++;

			do {
				*dst = color;
				dst += pitch;

				/* reached bottom? */
				if (--h == 0) {
					/* reached right edge? */
					if (--w == 0)
						return;
					dst = ++dstPtr;
					h = height;
				}
			} while (--reps >= 0);
		} else {

			do {
				*dst = *src++;
				dst += pitch;

				/* reached bottom? */
				if (--h == 0) {
					/* reached right edge? */
					if (--w == 0)
						return;
					dst = ++dstPtr;
					h = height;
				}
			} while (++reps != 0);
		}
	}
}

void AGOSEngine::decodeRow(byte *dst, const byte *src, int width) {
	const uint pitch = _dxSurfacePitch;
	int8 reps = (int8)0x80;
	byte color;
	byte *dstPtr = dst;
	uint w = width, h = 8;

	for (;;) {
		reps = *src++;
		if (reps >= 0) {
			color = *src++;

			do {
				*dst++ = color;

				/* reached right edge? */
				if (--w == 0) {
					/* reached bottom? */
					if (--h == 0)
						return;
					dstPtr += pitch;
					dst = dstPtr;
					w = width;
				}
			} while (--reps >= 0);
		} else {

			do {
				*dst++ = *src++;

				/* reached right edge? */
				if (--w == 0) {
					/* reached bottom? */
					if (--h == 0)
						return;
					dstPtr += pitch;
					dst = dstPtr;
					w = width;
				}
			} while (++reps != 0);
		}
	}
}

bool AGOSEngine::drawImages_clip(VC10_state *state) {
	const uint16 *vlut;
	uint maxWidth, maxHeight;
	int cur;

	vlut = &_videoWindows[_windowNum * 4];

	if (getGameType() != GType_FF && getGameType() != GType_PP) {
		state->draw_width = state->width * 2;
	} 

	cur = state->x;
	if (cur < 0) {
		do {
			if (!--state->draw_width)
				return 0;
			state->x_skip++;
		} while (++cur);
	}
	state->x = cur;

	maxWidth = (getGameType() == GType_FF || getGameType() == GType_PP) ? _screenWidth : (vlut[2] * 2);
	cur += state->draw_width - maxWidth;
	if (cur > 0) {
		do {
			if (!--state->draw_width)
				return 0;
		} while (--cur);
	}

	cur = state->y;
	if (cur < 0) {
		do {
			if (!--state->draw_height)
				return 0;
			state->y_skip++;
		} while (++cur);
	}
	state->y = cur;

	maxHeight = (getGameType() == GType_FF || getGameType() == GType_PP) ? _screenHeight : vlut[3];
	cur += state->draw_height - maxHeight;
	if (cur > 0) {
		do {
			if (!--state->draw_height)
				return 0;
		} while (--cur);
	}

	assert(state->draw_width != 0 && state->draw_height != 0);

	if (getGameType() != GType_FF && getGameType() != GType_PP) {
		state->draw_width *= 4;
	}

	return 1;
}

void AGOSEngine::drawImages_Feeble(VC10_state *state) {
	if (state->flags & kDFCompressed) {
		if (state->flags & kDFScaled) {
			state->surf_addr = getScaleBuf();
			state->surf_pitch = _dxSurfacePitch;

			uint w, h;
			byte *src, *dst, *dstPtr;

			state->dl = state->width;
			state->dh = state->height;

			dstPtr = state->surf_addr;
			w = 0;
			do {
				src = vc10_depackColumn(state);
				dst = dstPtr;

				h = 0;
				do {
					*dst = *src;
					dst += _screenWidth;
					src++;
				} while (++h != state->draw_height);
				dstPtr++;
			} while (++w != state->draw_width);

			if (_vgaCurSpritePriority % 10 != 9) {
				_scaleX = state->x;
				_scaleY = state->y;
				_scaleWidth = state->width;
				_scaleHeight = state->height;
			} else {
				scaleClip(state->height, state->width, state->y, state->x, state->y + _scrollY);
			}
		} else if (state->flags & kDFOverlayed) {
			state->surf_addr = getScaleBuf();
			state->surf_pitch = _dxSurfacePitch;
			state->surf_addr += (state->x + _scrollX) + (state->y + _scrollY) * state->surf_pitch;

			uint w, h;
			byte *src, *dst, *dstPtr;

			state->dl = state->width;
			state->dh = state->height;

			dstPtr = state->surf_addr;
			w = 0;
			do {
				byte color;

				src = vc10_depackColumn(state);
				dst = dstPtr;

				h = 0;
				do {
					color = *src;
					if (color != 0)
						*dst = color;
					dst += _screenWidth;
					src++;
				} while (++h != state->draw_height);
				dstPtr++;
			} while (++w != state->draw_width);

			if (_vgaCurSpritePriority % 10 == 9) {
				scaleClip(_scaleHeight, _scaleWidth, _scaleY, _scaleX, _scaleY + _scrollY);
			}
		} else {
			if (drawImages_clip(state) == 0)
				return;

			state->surf_addr += state->x + state->y * state->surf_pitch;

			uint w, h;
			byte *src, *dst, *dstPtr;

			state->dl = state->width;
			state->dh = state->height;

			vc10_skip_cols(state);


			if (state->flags & kDFMasked) {
				if (getGameType() == GType_FF && !getBitFlag(81)) {
					if (state->x  > _feebleRect.right)
						return;
					if (state->y > _feebleRect.bottom)
						return;
					if (state->x + state->width < _feebleRect.left)
						return;
					if (state->y + state->height < _feebleRect.top)
						return;
				}

				dstPtr = state->surf_addr;
				w = 0;
				do {
					byte color;

					src = vc10_depackColumn(state);
					dst = dstPtr;

					h = 0;
					do {
						color = *src;
						if (color)
							*dst = color;
						dst += _screenWidth;
						src++;
					} while (++h != state->draw_height);
					dstPtr++;
				} while (++w != state->draw_width);
			} else {
				dstPtr = state->surf_addr;
				w = 0;
				do {
					byte color;

					src = vc10_depackColumn(state);
					dst = dstPtr;

					h = 0;
					do {
						color = *src;
						if ((state->flags & kDFNonTrans) || color != 0)
							*dst = color;
						dst += _screenWidth;
						src++;
					} while (++h != state->draw_height);
					dstPtr++;
				} while (++w != state->draw_width);
			}
		}
	} else {
		if (drawImages_clip(state) == 0)
			return;

		state->surf_addr += state->x + state->y * state->surf_pitch;

		const byte *src;
		byte *dst;
		uint count;

		src = state->depack_src + state->width * state->y_skip;
		dst = state->surf_addr;
		do {
			for (count = 0; count != state->draw_width; count++) {
				byte color;
				color = src[count + state->x_skip];
				if (color) {
					if ((state->flags & kDFShaded) && color == 220)
						color = 244;

					dst[count] = color;
				}
			}
			dst += _screenWidth;
			src += state->width;
		} while (--state->draw_height);
	} 
}

void AGOSEngine::drawImages(VC10_state *state) {
	const uint16 *vlut = &_videoWindows[_windowNum * 4];

	if (drawImages_clip(state) == 0)
		return;

	uint xoffs, yoffs;
	if (getGameType() == GType_ELVIRA1) {
		//if (_windowNum != 2 && _windowNum != 3 && _windowNum != 6) {
		//	xoffs = ((vlut[0] - _videoWindows[16]) * 2 + state->x) * 8;
		//	yoffs = (vlut[1] - _videoWindows[17] + state->y);
		//} else {
			xoffs = (vlut[0] * 2 + state->x) * 8;
			yoffs = vlut[1] + state->y;
		//}
	} else if (getGameType() == GType_ELVIRA2) {
		//if (_windowNum == 4 || _windowNum >= 10) {
		//	xoffs = ((vlut[0] - _videoWindows[16]) * 2 + state->x) * 8;
		//	yoffs = (vlut[1] - _videoWindows[17] + state->y);
		//} else {
			xoffs = (vlut[0] * 2 + state->x) * 8;
			yoffs = vlut[1] + state->y;
		//}
	} else if (getGameType() == GType_WW) {
		//if (_windowNum == 4 || (_windowNum >= 10 && _windowNum < 28)) {
		//	xoffs = ((vlut[0] - _videoWindows[16]) * 2 + state->x) * 8;
		//	yoffs = (vlut[1] - _videoWindows[17] + state->y);
		//} else {
			xoffs = (vlut[0] * 2 + state->x) * 8;
			yoffs = vlut[1] + state->y;
		//}
	} else if (getGameType() == GType_SIMON1 && (_subroutine == 2923 || _subroutine == 2926)) {
		// Allow one section of Simon the Sorcerer 1 introduction to be displayed
		// in lower half of screen
		xoffs = state->x * 8;
		yoffs = state->y;
	} else {
		xoffs = ((vlut[0] - _videoWindows[16]) * 2 + state->x) * 8;
		yoffs = (vlut[1] - _videoWindows[17] + state->y);
	}

	state->surf2_addr += xoffs + yoffs * state->surf_pitch;
	state->surf_addr += xoffs + yoffs * state->surf2_pitch;

	if (state->flags & kDFMasked) {
		byte *mask, *src, *dst;
		byte h;
		uint w;

		state->x_skip *= 4;
		state->dl = state->width;
		state->dh = state->height;

		vc10_skip_cols(state);

		w = 0;
		do {
			mask = vc10_depackColumn(state);	/* esi */
			src = state->surf2_addr + w * 2;	/* ebx */
			dst = state->surf_addr + w * 2;		/* edi */

			h = state->draw_height;
			if ((getGameType() == GType_SIMON1) && getBitFlag(88)) {
				/* transparency */
				do {
					if (mask[0] & 0xF0) {
						if ((dst[0] & 0x0F0) == 0x20)
							dst[0] = src[0];
					}
					if (mask[0] & 0x0F) {
						if ((dst[1] & 0x0F0) == 0x20)
							dst[1] = src[1];
					}
					mask++;
					dst += state->surf_pitch;
					src += state->surf2_pitch;
				} while (--h);
			} else {
				/* no transparency */
				do {
					if (mask[0] & 0xF0)
						dst[0] = src[0];
					if (mask[0] & 0x0F)
						dst[1] = src[1];
					mask++;
					dst += state->surf_pitch;
					src += state->surf2_pitch;
				} while (--h);
			}
		} while (++w != state->draw_width);
	} else if ((((_lockWord & 0x20) && state->palette == 0) || state->palette == 0xC0) &&
		(getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) &&
		getPlatform() != Common::kPlatformAmiga) {
		const byte *src;
		byte *dst;
		uint h, i;

		if (state->flags & kDFCompressed) {
			byte *dstPtr = state->surf_addr;
			src = state->depack_src;
			/* AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD EEEEEEEE
			 * aaaaabbb bbcccccd ddddeeee efffffgg ggghhhhh
			 */

			do {
				uint count = state->draw_width / 4;

				dst = dstPtr;
				do {
					uint32 bits = (src[0] << 24) | (src[1] << 16) | (src[2] << 8) | (src[3]);
					byte color;

					color = (byte)((bits >> (32 - 5)) & 31);
					if ((state->flags & kDFNonTrans) || color)
						dst[0] = color;
					color = (byte)((bits >> (32 - 10)) & 31);
					if ((state->flags & kDFNonTrans) || color)
						dst[1] = color;
					color = (byte)((bits >> (32 - 15)) & 31);
					if ((state->flags & kDFNonTrans) || color)
						dst[2] = color;
					color = (byte)((bits >> (32 - 20)) & 31);
					if ((state->flags & kDFNonTrans) || color)
						dst[3] = color;
					color = (byte)((bits >> (32 - 25)) & 31);
					if ((state->flags & kDFNonTrans) || color)
						dst[4] = color;
					color = (byte)((bits >> (32 - 30)) & 31);
					if ((state->flags & kDFNonTrans) || color)
						dst[5] = color;

					bits = (bits << 8) | src[4];

					color = (byte)((bits >> (40 - 35)) & 31);
					if ((state->flags & kDFNonTrans) || color)
						dst[6] = color;
					color = (byte)((bits) & 31);
					if ((state->flags & kDFNonTrans) || color)
						dst[7] = color;

					dst += 8;
					src += 5;
				} while (--count);
				dstPtr += _screenWidth;
			} while (--state->draw_height);
		} else {
			src = state->depack_src + (state->width * state->y_skip * 16) + (state->x_skip * 8);
			dst = state->surf_addr;

			state->draw_width *= 2;

			h = state->draw_height;
			do {
				for (i = 0; i != state->draw_width; i++)
					if ((state->flags & kDFNonTrans) || src[i])
						dst[i] = src[i];
				dst += _screenWidth;
				src += state->width * 16;
			} while (--h);
		}
	} else {
		if (state->flags & kDFUseFrontBuf) {
			state->surf_addr = state->surf2_addr;
			state->surf_pitch = state->surf2_pitch;
		}

		if (getGameType() == GType_ELVIRA2 || getGameType() == GType_WW)
			state->palette = state->surf_addr[0] & 0xF0;

		if (state->flags & kDFCompressed) {
			uint w, h;
			byte *src, *dst, *dstPtr;

			state->x_skip *= 4;				/* reached */

			state->dl = state->width;
			state->dh = state->height;

			vc10_skip_cols(state);

			dstPtr = state->surf_addr;
			if (!(state->flags & kDFNonTrans) && (state->flags & 0x40)) { /* reached */
				dstPtr += vcReadVar(252);
			}
			w = 0;
			do {
				byte color;

				src = vc10_depackColumn(state);
				dst = dstPtr;

				h = 0;
				do {
					color = (*src / 16);
					if ((state->flags & kDFNonTrans) || color != 0)
						dst[0] = color | state->palette;
					color = (*src & 15);
					if ((state->flags & kDFNonTrans) || color != 0)
						dst[1] = color | state->palette;
					dst += _screenWidth;
					src++;
				} while (++h != state->draw_height);
				dstPtr += 2;
			} while (++w != state->draw_width);
		} else {
			const byte *src;
			byte *dst;
			uint count;

			src = state->depack_src + (state->width * state->y_skip) * 8;
			dst = state->surf_addr;
			state->x_skip *= 4;

			do {
				for (count = 0; count != state->draw_width; count++) {
					byte color;
					color = (src[count + state->x_skip] / 16);
					if ((state->flags & kDFNonTrans) || color)
						dst[count * 2] = color | state->palette;
					color = (src[count + state->x_skip] & 15);
					if ((state->flags & kDFNonTrans) || color)
						dst[count * 2 + 1] = color | state->palette;
				}
				dst += _screenWidth;
				src += state->width * 8;
			} while (--state->draw_height);
		}
	}
}

void AGOSEngine::horizontalScroll(VC10_state *state) {
	const byte *src;
	byte *dst;
	int w;

	if (getGameType() == GType_FF)
		_scrollXMax = state->width - 640;
	else
		_scrollXMax = state->width * 2 - 40;
	_scrollYMax = 0;
	_scrollImage = state->depack_src;
	_scrollHeight = state->height;
	if (_variableArrayPtr[34] < 0)
		state->x = _variableArrayPtr[251];

	_scrollX = state->x;

	vcWriteVar(251, _scrollX);

	dst = getBackBuf();

	if (getGameType() == GType_FF)
		src = state->depack_src + _scrollX / 2;
	else
		src = state->depack_src + _scrollX * 4;

	for (w = 0; w < _screenWidth; w += 8) {
		decodeColumn(dst, src + readUint32Wrapper(src), state->height);
		dst += 8;
		src += 4;
	}
}

void AGOSEngine::verticalScroll(VC10_state *state) {
	const byte *src;
	byte *dst;
	int h;

	_scrollXMax = 0;
	_scrollYMax = state->height - 480;
	_scrollImage = state->depack_src;
	_scrollWidth = state->width;
	if (_variableArrayPtr[34] < 0)
		state->y = _variableArrayPtr[250];

	_scrollY = state->y;

	vcWriteVar(250, _scrollY);

	dst = getBackBuf();
	src = state->depack_src + _scrollY / 2;

	for (h = 0; h < _screenHeight; h += 8) {
		decodeRow(dst, src + READ_LE_UINT32(src), state->width);
		dst += 8 * state->width;
		src += 4;
	}
}

void AGOSEngine::scaleClip(int16 h, int16 w, int16 y, int16 x, int16 scrollY) {
	Common::Rect srcRect, dstRect;
	float factor, xscale;

	srcRect.left = 0;
	srcRect.top = 0;
	srcRect.right = w;
	srcRect.bottom = h;

	if (scrollY > _baseY)
		factor = 1 + ((scrollY - _baseY) * _scale);
	else
		factor = 1 - ((_baseY - scrollY) * _scale);

	xscale = ((w * factor) / 2);

	dstRect.left   = (int16)(x - xscale);
	if (dstRect.left > _screenWidth - 1)
		return;
	dstRect.top    = (int16)(y - (h * factor));
	if (dstRect.top > _screenHeight - 1)
		return;

	dstRect.right  = (int16)(x + xscale);
	dstRect.bottom = y;

	_feebleRect = dstRect;

	_variableArray[20] = _feebleRect.top;
	_variableArray[21] = _feebleRect.left;
	_variableArray[22] = _feebleRect.bottom;
	_variableArray[23] = _feebleRect.right;

	debug(5, "Left %d Right %d Top %d Bottom %d", dstRect.left, dstRect.right, dstRect.top, dstRect.bottom);

	// Unlike normal rectangles in ScummVM, it seems that in the case of
	// the destination rectangle the bottom and right coordinates are
	// considered to be inside the rectangle. For the source rectangle,
	// I believe that they are not.

	int scaledW = dstRect.width() + 1;
	int scaledH = dstRect.height() + 1;

	byte *src = getScaleBuf();
	byte *dst = getBackBuf();

	dst += _dxSurfacePitch * dstRect.top + dstRect.left;

	for (int dstY = 0; dstY < scaledH; dstY++) {
		if (dstRect.top + dstY >= 0 && dstRect.top + dstY < _screenHeight) {
			int srcY = (dstY * h) / scaledH;
			byte *srcPtr = src + _dxSurfacePitch * srcY;
			byte *dstPtr = dst + _dxSurfacePitch * dstY;
			for (int dstX = 0; dstX < scaledW; dstX++) {
				if (dstRect.left + dstX >= 0 && dstRect.left + dstX < _screenWidth) {
					int srcX = (dstX * w) / scaledW;
					if (srcPtr[srcX])
						dstPtr[dstX] = srcPtr[srcX];
				}
			}
		}
	}
}

void AGOSEngine::paletteFadeOut(byte *palPtr, uint num, uint size) {
	byte *p = palPtr;

	do {
		if (p[0] >= size)
			p[0] -= size;
		else
			p[0] = 0;
		if (p[1] >= size)
			p[1] -= size;
		else
			p[1] = 0;
		if (p[2] >= size)
			p[2] -= size;
		else
			p[2] = 0;
		p += 4;
	} while (--num);
}

void AGOSEngine::animate(uint windowNum, uint zoneNum, uint vgaSpriteId, uint x, uint y, uint palette, bool vgaScript) {
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	byte *p, *pp;
	uint count;

	if (isSpriteLoaded(vgaSpriteId, zoneNum))
		return;

	vsp = _vgaSprites;
	while (vsp->id != 0)
		vsp++;

	vsp->windowNum = windowNum;
	vsp->priority = 0;
	vsp->flags = 0;

	vsp->y = y;
	vsp->x = x;
	vsp->image = 0;
	if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2 || getGameType() == GType_WW)
		vsp->palette = 0;
	else
		vsp->palette = palette;
	vsp->id = vgaSpriteId;

	if (getGameType() == GType_SIMON2 || getGameType() == GType_FF || getGameType() == GType_PP)
		vsp->zoneNum = zoneNum;
	else
		vsp->zoneNum = zoneNum = vgaSpriteId / 100;

	for (;;) {
		vpe = &_vgaBufferPointers[zoneNum];
		_curVgaFile1 = vpe->vgaFile1;
		if (vgaScript) {
			if (vpe->vgaFile1 != NULL)
				break;
			if (_zoneNumber != zoneNum)
				_noOverWrite = _zoneNumber;

			loadZone(zoneNum);
			_noOverWrite = 0xFFFF;
		} else {
			_zoneNumber = zoneNum;
			if (vpe->vgaFile1 != NULL)
				break;
			loadZone(zoneNum);
		}
	}

	pp = _curVgaFile1;
	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		p = pp + READ_LE_UINT16(pp + 2);
		count = READ_LE_UINT16(&((VgaFileHeader2_Feeble *) p)->animationCount);
		p = pp + READ_LE_UINT16(&((VgaFileHeader2_Feeble *) p)->animationTable);

		while (count--) {
			if (READ_LE_UINT16(&((AnimationHeader_Feeble *) p)->id) == vgaSpriteId)
				break;
			p += sizeof(AnimationHeader_Feeble);
		}
		assert(READ_LE_UINT16(&((AnimationHeader_Feeble *) p)->id) == vgaSpriteId);
	} else if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		p = pp + READ_BE_UINT16(pp + 4);
		count = READ_BE_UINT16(&((VgaFileHeader2_Common *) p)->animationCount);
		p = pp + READ_BE_UINT16(&((VgaFileHeader2_Common *) p)->animationTable);

		while (count--) {
			if (READ_BE_UINT16(&((AnimationHeader_Simon *) p)->id) == vgaSpriteId)
				break;
			p += sizeof(AnimationHeader_Simon);
		}
		assert(READ_BE_UINT16(&((AnimationHeader_Simon *) p)->id) == vgaSpriteId);
	} else {
		p = pp + READ_BE_UINT16(pp + 10);
		p += 20;

		count = READ_BE_UINT16(&((VgaFileHeader2_Common *) p)->animationCount);
		p = pp + READ_BE_UINT16(&((VgaFileHeader2_Common *) p)->animationTable);

		while (count--) {
			if (READ_BE_UINT16(&((AnimationHeader_WW *) p)->id) == vgaSpriteId)
				break;
			p += sizeof(AnimationHeader_WW);
		}
		assert(READ_BE_UINT16(&((AnimationHeader_WW *) p)->id) == vgaSpriteId);
	}

#ifdef DUMP_FILE_NR
	{
		static bool dumped = false;
		if (res == DUMP_FILE_NR && !dumped) {
			dumped = true;
			dumpVgaFile(_curVgaFile1);
		}
	}
#endif

#ifdef DUMP_BITMAPS_FILE_NR
	{
		static bool dumped = false;
		if (res == DUMP_BITMAPS_FILE_NR && !dumped) {
			dumped = true;
			dumpVgaBitmaps(_curVgaFile2, _curVgaFile1, zoneNum);
		}
	}
#endif

	if (_startVgaScript) {
		if (getGameType() == GType_FF || getGameType() == GType_PP) {
			dumpVgaScript(_curVgaFile1 + READ_LE_UINT16(&((AnimationHeader_Feeble*)p)->scriptOffs), zoneNum, vgaSpriteId);
		} else if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
			dumpVgaScript(_curVgaFile1 + READ_BE_UINT16(&((AnimationHeader_Simon*)p)->scriptOffs), zoneNum, vgaSpriteId);
		} else {
			dumpVgaScript(_curVgaFile1 + READ_BE_UINT16(&((AnimationHeader_WW*)p)->scriptOffs), zoneNum, vgaSpriteId);
		}
	}

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		addVgaEvent(_vgaBaseDelay, _curVgaFile1 + READ_LE_UINT16(&((AnimationHeader_Feeble *) p)->scriptOffs), vgaSpriteId, zoneNum);
	} else if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		addVgaEvent(_vgaBaseDelay, _curVgaFile1 + READ_BE_UINT16(&((AnimationHeader_Simon *) p)->scriptOffs), vgaSpriteId, zoneNum);
	} else {
		addVgaEvent(_vgaBaseDelay, _curVgaFile1 + READ_BE_UINT16(&((AnimationHeader_WW *) p)->scriptOffs), vgaSpriteId, zoneNum);
	}
}

void AGOSEngine::setImage(uint16 vga_res_id, bool vgaScript) {
	uint zoneNum;
	VgaPointersEntry *vpe;
	byte *bb, *b;
	uint16 count;
	const byte *vc_ptr_org;

	zoneNum = vga_res_id / 100;

	for (;;) {
		vpe = &_vgaBufferPointers[zoneNum];
		_curVgaFile1 = vpe->vgaFile1;
		_curVgaFile2 = vpe->vgaFile2;

		if (vgaScript) {
			if (vpe->vgaFile1 != NULL)
				break;
			if (_zoneNumber != zoneNum)
				_noOverWrite = _zoneNumber;

			loadZone(zoneNum);
			_noOverWrite = 0xFFFF;
		} else {
			_curSfxFile = vpe->sfxFile;
			_zoneNumber = zoneNum;

			if (vpe->vgaFile1 != NULL)
				break;

			loadZone(zoneNum);
		}
	}

	bb = _curVgaFile1;
	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		b = bb + READ_LE_UINT16(bb + 2);
		count = READ_LE_UINT16(&((VgaFileHeader2_Feeble *) b)->imageCount);
		b = bb + READ_LE_UINT16(&((VgaFileHeader2_Feeble *) b)->imageTable);

		while (count--) {
			if (READ_LE_UINT16(&((ImageHeader_Feeble *) b)->id) == vga_res_id)
				break;
			b += sizeof(ImageHeader_Feeble);
		}
		assert(READ_LE_UINT16(&((ImageHeader_Feeble *) b)->id) == vga_res_id);
	} else if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		b = bb + READ_BE_UINT16(bb + 4);
		count = READ_BE_UINT16(&((VgaFileHeader2_Common *) b)->imageCount);
		b = bb + READ_BE_UINT16(&((VgaFileHeader2_Common *) b)->imageTable);

		while (count--) {
			if (READ_BE_UINT16(&((ImageHeader_Simon *) b)->id) == vga_res_id)
				break;
			b += sizeof(ImageHeader_Simon);
		}
		assert(READ_BE_UINT16(&((ImageHeader_Simon *) b)->id) == vga_res_id);
	} else {
		b = bb + READ_BE_UINT16(bb + 10);
		b += 20;

		count = READ_BE_UINT16(&((VgaFileHeader2_Common *) b)->imageCount);
		b = bb + READ_BE_UINT16(&((VgaFileHeader2_Common *) b)->imageTable);

		while (count--) {
			if (READ_BE_UINT16(&((ImageHeader_WW *) b)->id) == vga_res_id)
				break;
			b += sizeof(ImageHeader_WW);
		}
		assert(READ_BE_UINT16(&((ImageHeader_WW *) b)->id) == vga_res_id);

		if (!vgaScript)
			clearWindow(_windowNum, READ_BE_UINT16(&((ImageHeader_WW *) b)->color));
	}

	if (_startVgaScript) {
		if (getGameType() == GType_FF || getGameType() == GType_PP) {
			dumpVgaScript(_curVgaFile1 + READ_LE_UINT16(&((ImageHeader_Feeble*)b)->scriptOffs), zoneNum, vga_res_id);
		} else if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
			dumpVgaScript(_curVgaFile1 + READ_BE_UINT16(&((ImageHeader_Simon*)b)->scriptOffs), zoneNum, vga_res_id);
		} else {
			dumpVgaScript(_curVgaFile1 + READ_BE_UINT16(&((ImageHeader_WW*)b)->scriptOffs), zoneNum, vga_res_id);
		}
	}

	vc_ptr_org = _vcPtr;

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		_vcPtr = _curVgaFile1 + READ_LE_UINT16(&((ImageHeader_Feeble *) b)->scriptOffs);
	} else if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		_vcPtr = _curVgaFile1 + READ_BE_UINT16(&((ImageHeader_Simon *) b)->scriptOffs);
	} else {
		_vcPtr = _curVgaFile1 + READ_BE_UINT16(&((ImageHeader_WW *) b)->scriptOffs);
	}

	runVgaScript();
	_vcPtr = vc_ptr_org;
}

void AGOSEngine::setWindowImageEx(uint16 mode, uint16 vga_res) {
	if (mode == 4) {
		vc29_stopAllSounds();

		if (!_initMouse) {
			_initMouse = 1;
			vc33_setMouseOn();
		}

		if (getGameType() == GType_ELVIRA1) {
			if (_variableArray[299] == 0) {
				_variableArray[293] = 0;
				_wallOn = 0;
			}
		} else if (getGameType() == GType_ELVIRA2) {
			if (_variableArray[70] == 0) {
				_variableArray[71] = 0;
				_wallOn = 0;
			}
		}

	}

	if (_lockWord & 0x10)
		error("setWindowImageEx: _lockWord & 0x10");

	setWindowImage(mode, vga_res);
}

void AGOSEngine::setWindowImage(uint16 mode, uint16 vga_res_id) {
	uint num_lines;
	uint16 updateWindow;

	_windowNum = updateWindow = mode;
	_lockWord |= 0x20;

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		vc27_resetSprite();
	}

	if (vga_res_id == 0) {
		if (getGameType() == GType_SIMON1) {
			_unkPalFlag = true;
		} else if (getGameType() == GType_SIMON2) {
			_useBackGround = true;
			_restoreWindow6 = true;
		}
	}

	if (getGameType() == GType_SIMON1) {
		if (vga_res_id == 16300) {
			clearBackFromTop(134);
			_usePaletteDelay = true;
		}
	} else if (getGameType() == GType_SIMON2 || getGameType() == GType_FF) {
		_scrollX = 0;
		_scrollY = 0;
		_scrollXMax = 0;
		_scrollYMax = 0;
		_scrollCount = 0;
		_scrollFlag = 0;
		_scrollHeight = 134;
		_variableArrayPtr = _variableArray;
		if (_variableArray[34] >= 0) {
			if (getGameType() == GType_FF)
				_variableArray[250] = 0;
			_variableArray[251] = 0;
		}
	}

	setImage(vga_res_id);

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		fillFrontFromBack(0, 0, _screenWidth, _screenHeight);
		fillBackGroundFromBack(_screenHeight);
		_syncFlag2 = 1;
	} else if (getGameType() == GType_SIMON2) {
		if (!_useBackGround) {
			num_lines = _windowNum == 4 ? 134 : 200;
			_boxStarHeight = num_lines;
			fillFrontFromBack(0, 0, _screenWidth, num_lines);
			fillBackGroundFromBack(num_lines);
			_syncFlag2 = 1;
		}
		_useBackGround = false;
	} else if (getGameType() == GType_SIMON1) {
		// Allow one section of Simon the Sorcerer 1 introduction to be displayed
		// in lower half of screen
		if (_subroutine == 2923 || _subroutine == 2926)
			num_lines = 200;
		else
			num_lines = _windowNum == 4 ? 134 : 200;

		fillFrontFromBack(0, 0, _screenWidth, num_lines);
		fillBackGroundFromBack(num_lines);
		_syncFlag2 = 1;
		_timer5 = 0;
	} else {
		num_lines = _windowNum == 4 ? 134 : 200;
		fillFrontFromBack(0, 0, _screenWidth, num_lines);
		fillBackGroundFromBack(num_lines);
		_syncFlag2 = 1;
		_timer5 = 0;
	}

	if (getGameType() == GType_ELVIRA1 && updateWindow == 3 && _bottomPalette != 0) {
		byte *dst = getBackBuf() + 42560;
		int size = 21440;

		while (size--) {
			*dst += 0x10;
			dst++;
		}
	}

	_lockWord &= ~0x20;

	if (getGameType() == GType_SIMON1) {
		if (_unkPalFlag) {
			_unkPalFlag = false;
			while (_fastFadeInFlag != 0) {
				delay(10);
			}
		}
	}
}

} // End of namespace AGOS
