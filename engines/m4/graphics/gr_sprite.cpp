/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "m4/graphics/gr_sprite.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/core/term.h"
#include "m4/mem/memman.h"
#include "m4/platform/draw.h"
//include "m4/vars.h"

namespace M4 {

#define NO_COMPRESS     0x00
#define RLE8            0x01
#define SHADOW			0x80

/**
 * ScaleX and ScaleY are supposed to be percents, where 100 means 100%
 * S and D are Raw encoded (unencoded!) buffers.
 */
static uint8 scale_sprite(Buffer *S, Buffer *D, uint32 ScaleX, uint32 ScaleY) {
	uint16 ErrX, ErrY, i, j;
	uint8 *pScaled, *pData = S->data;

	if (!D || !S)
		error_show(FL, 'BUF!', "scale sprite h:%ld w:%ld sx:%uld sy:%uld", D->h, D->W, ScaleX, ScaleY);

	/* calculate new x size */
	D->W = S->W * ScaleX / 100;
	if (S->W * ScaleX % 100 >= 50)
		++D->W;

	/* calculate new y size */
	D->h = S->h * ScaleY / 100;
	if (S->h * ScaleY % 100 >= 50)
		++D->h;

	D->stride = D->W;

	/* allocate 'scaled' buffer */
	if (!(D->data = pScaled = (uint8 *)mem_alloc(D->h * D->stride, "scaled buffer")))
		error_show(FL, 'OOM!', "scaled buffer h:%uld w:%uld", D->h, D->stride);

	ErrY = 50;
	for (i = 0; i < S->h; ++i)
	{
		ErrY += ScaleY;
		while (ErrY >= 100)
		{
			ErrX = 50;
			for (j = 0; j < S->W; ++j)
			{
				ErrX += ScaleX;
				while (ErrX >= 100)
				{
					*pScaled++ = *pData;
					ErrX -= 100;
				}
				++pData;
			}
			ErrY -= 100;
			pData -= S->W;
		}
		pData += S->W;
	}
	return 0;
}

#define Scaled	((DrawReq -> scaleY != 100) || (DrawReq -> scaleX != 100 && DrawReq -> scaleX != -100))
#define Rle	(Source.encoding == RLE8)
#define Clipped ((DrawReq -> x < 0) || (DrawReq -> y < 0) || (DrawReq -> x + Source.W > DrawReq -> Dest->W) || (DrawReq -> y + Source.h > DrawReq -> Dest -> h))
#define Forward (DrawReq -> scaleX > 0)
#define Depthed (DrawReq -> srcDepth)
#define Shadow	(Source.encoding & 0x80)

#define ClipD	(LeftOffset || RightOffset || BottomCut)
#ifndef __MAC

uint8 gr_sprite_draw(DrawRequest *DrawReq) {
	Buffer Source;
	int32 LeftOffset = 0, RightOffset = 0, YPos;
	uint8 *ShadowBuff = NULL, *ScaledBuff = NULL;
	Buffer AfterScaled;
	uint8 BottomCut = 0;

	if (!DrawReq->Src) {
		term_message("NULL source data in sprite_draw");
		//error_fatal_abort(0, "no source data", __FILE__, __LINE__, 0,0,0);
		return 0;
	}

	// - scaleY means don't bother drawing this sprite

	if (DrawReq->scaleY <= 0)
		return 0;

	if ((!DrawReq->Src->W) || (!DrawReq->Src->h)) {
		//error_fatal_abort(0, "src->w, src->h zero", __FILE__, __LINE__, 0,0,0);
		return 1;
	}

	AfterScaled.W = 0;
	AfterScaled.h = 0;
	AfterScaled.data = NULL;
	AfterScaled.encoding = 0;
	AfterScaled.stride = 0;

	/* copy DrawReq -> Src to  Source buffer */
	memcpy(&Source, DrawReq->Src, sizeof(Buffer));

	/* check for RLE encoding in case of shadows */
	// there is no RLE shadow draw routine, so we have to decode shadows ahead of time.
	if ((Source.encoding & RLE8) && (Source.encoding & SHADOW))
	{
		if (!(ShadowBuff = (uint8 *)mem_alloc(Source.stride * Source.h, "shadow buff")))
			error_show(FL, 'OOM!', "buffer w:%uld, h:%uld", Source.W, Source.h);

		RLE8Decode(Source.data, ShadowBuff, Source.stride);
		Source.data = ShadowBuff;
		Source.encoding &= ~RLE8;
	}

	/* check for scaling */
	// we scale before we draw
	if (Scaled)
	{
		/* check if input is RLE8 encoded */
		// if it's scaled we decode it first
		if (Rle)
		{
			if (!(ScaledBuff = (uint8 *)mem_alloc(Source.stride * Source.h, "scaled buffer")))
				error_show(FL, 'OOM!', "no mem: buffer w:%ld, h:%ld", Source.W, Source.h);

			RLE8Decode(Source.data, ScaledBuff, Source.stride);
			Source.data = ScaledBuff;
			Source.encoding &= ~RLE8;
		}
		if (scale_sprite(&Source, &AfterScaled, imath_abs(DrawReq->scaleX), imath_abs(DrawReq->scaleY)))
		{
			if (ShadowBuff) mem_free(ShadowBuff);
			if (ScaledBuff) mem_free(ScaledBuff);
			error_show(FL, 'SPSF', "gr_sprite_draw");
		}

		/* preserve encoding */
		AfterScaled.encoding = Source.encoding;

		/* copy AfterScaled to	Source buffer */
		memcpy(&Source, &AfterScaled, sizeof(Buffer));
	}

	YPos = DrawReq->y;

	/* check for clipping */
	// if sprite is off edge of destination buffer, do something special
	if (Clipped)
	{
		if (-YPos >= Source.h)
			goto truly_done;

		if (YPos >= DrawReq->Dest->h)
			goto truly_done;

		if (-DrawReq->x >= Source.W)
			goto truly_done;

		if (DrawReq->x >= DrawReq->Dest->W)
			goto truly_done;

		// if clipped off top, scan into sprite
		if (YPos < 0)
		{
			if (Rle)
				Source.data = SkipRLE_Lines(-YPos, Source.data);
			else
				Source.data += -YPos * Source.stride;
			Source.h += YPos;
			YPos = 0;
		}

		// find out if we're losing pixels on left or right
		if (DrawReq->x < 0)
			LeftOffset = -DrawReq->x;

		if (DrawReq->x + Source.W > DrawReq->Dest->W)
			RightOffset = DrawReq->x + Source.W - DrawReq->Dest->W;

		if (YPos + Source.h > DrawReq->Dest->h)
		{
			Source.h -= YPos + Source.h - DrawReq->Dest->h;
			BottomCut = 1;
		}
	}

	/* after all the necessary changes (scaling/shadow RLE expansion) */
	if (Rle)					/* will be RLE_Draw */
		if (ClipD)
			if (Depthed)
				if (Forward)
					RLE_DrawDepthOffs(&Source, DrawReq->Dest, DrawReq->x, YPos, DrawReq->srcDepth, DrawReq->depthCode, LeftOffset, RightOffset);
				else
					RLE_DrawDepthRevOffs(&Source, DrawReq->Dest, DrawReq->x, YPos, DrawReq->srcDepth, DrawReq->depthCode, LeftOffset, RightOffset);
			else
				if (Forward)
					RLE_DrawOffs(&Source, DrawReq->Dest, DrawReq->x, YPos, LeftOffset, RightOffset);
				else
					RLE_DrawRevOffs(&Source, DrawReq->Dest, DrawReq->x, YPos, LeftOffset, RightOffset);
		else
			if (Depthed)
				if (Forward)
					RLE_DrawDepth(&Source, DrawReq->Dest, DrawReq->x, YPos, DrawReq->srcDepth, DrawReq->depthCode);
				else
					RLE_DrawDepthRev(&Source, DrawReq->Dest, DrawReq->x, YPos, DrawReq->srcDepth, DrawReq->depthCode);
			else
				if (Forward)
					RLE_Draw(&Source, DrawReq->Dest, DrawReq->x, YPos);
				else
					RLE_DrawRev(&Source, DrawReq->Dest, DrawReq->x, YPos);
	else if (Shadow)                            /* will be Raw_SDraw */
		if (ClipD)
			if (Depthed)
				if (Forward)
					Raw_SDrawDepthOffs(&Source, DrawReq->Dest, DrawReq->x, YPos, DrawReq->srcDepth, DrawReq->depthCode, LeftOffset, RightOffset, DrawReq->Pal, DrawReq->ICT);
				else
					Raw_SDrawDepthRevOffs(&Source, DrawReq->Dest, DrawReq->x, YPos, DrawReq->srcDepth, DrawReq->depthCode, LeftOffset, RightOffset, DrawReq->Pal, DrawReq->ICT);
			else
				if (Forward)
					Raw_SDrawOffs(&Source, DrawReq->Dest, DrawReq->x, YPos, LeftOffset, RightOffset, DrawReq->Pal, DrawReq->ICT);
				else
					Raw_SDrawRevOffs(&Source, DrawReq->Dest, DrawReq->x, YPos, LeftOffset, RightOffset, DrawReq->Pal, DrawReq->ICT);
		else
			if (Depthed)
				if (Forward)
					Raw_SDrawDepth(&Source, DrawReq->Dest, DrawReq->x, YPos, DrawReq->srcDepth, DrawReq->depthCode, DrawReq->Pal, DrawReq->ICT);
				else
					Raw_SDrawDepthRev(&Source, DrawReq->Dest, DrawReq->x, YPos, DrawReq->srcDepth, DrawReq->depthCode, DrawReq->Pal, DrawReq->ICT);
			else
				if (Forward)
					Raw_SDraw(&Source, DrawReq->Dest, DrawReq->x, YPos, DrawReq->Pal, DrawReq->ICT);
				else
					Raw_SDrawRev(&Source, DrawReq->Dest, DrawReq->x, YPos, DrawReq->Pal, DrawReq->ICT);
	else                                        /* will be Raw_Draw */
		if (ClipD)
			if (Depthed)
				if (Forward)
					Raw_DrawDepthOffs(&Source, DrawReq->Dest, DrawReq->x, YPos, DrawReq->srcDepth, DrawReq->depthCode, LeftOffset, RightOffset);
				else
					Raw_DrawDepthRevOffs(&Source, DrawReq->Dest, DrawReq->x, YPos, DrawReq->srcDepth, DrawReq->depthCode, LeftOffset, RightOffset);
			else
				if (Forward)
					Raw_DrawOffs(&Source, DrawReq->Dest, DrawReq->x, YPos, LeftOffset, RightOffset);
				else
					Raw_DrawRevOffs(&Source, DrawReq->Dest, DrawReq->x, YPos, LeftOffset, RightOffset);
		else
			if (Depthed)
				if (Forward)
					Raw_DrawDepth(&Source, DrawReq->Dest, DrawReq->x, YPos, DrawReq->srcDepth, DrawReq->depthCode);
				else
					Raw_DrawDepthRev(&Source, DrawReq->Dest, DrawReq->x, YPos, DrawReq->srcDepth, DrawReq->depthCode);
			else
				if (Forward)
					Raw_Draw(&Source, DrawReq->Dest, DrawReq->x, YPos);
				else
					Raw_DrawRev(&Source, DrawReq->Dest, DrawReq->x, YPos);

truly_done:
	if (ShadowBuff)
		mem_free(ShadowBuff);

	if (ScaledBuff)
		mem_free(ScaledBuff);

	if (AfterScaled.data)
		mem_free(AfterScaled.data);
	return 0;
}
#endif
//----------------------------------------------------------------------------------------
//Q 'n' D c version - soon to become obselete...
// NOT USED!!!!!!
/*
void gr_sprite_draw_scaled(M4sprite *srcSprite, Buffer *destBuf, int32 destX, int32 destY, int32 scale)
{
	int32		errX, errY, x, y, scanX, scanY;
	uint8		*srcPtr, *destPtr, *destRowPtr, *srcRowPtr;

	if ((! srcSprite) || (! destBuf))
		return;

	if ((destX >= destBuf->W) || (destY >= destBuf->h))
		return;

	srcPtr = srcSprite->data;

	if (destY <= 0) {
		if (destX <= 0)
			destRowPtr = destBuf->data;
		else
			destRowPtr = (uint8*)((long)destBuf->data + destX);
	}
	else {
		if (destX <= 0)
			destRowPtr = (uint8*)((long)destBuf->data + (destBuf->stride * destY));
		else
			destRowPtr = (uint8*)((long)destBuf->data + (destBuf->stride * destY) + destX);
	}
	scanY = destY;
	destPtr = destRowPtr;
	errY = 50;

	for (y=0; y<srcSprite->h; y++)
	{
		errY += scale;
		if ((errY >= 100) && (scanY >= 0))
		{
			srcRowPtr = srcPtr;
			while (errY >= 100) {
				scanX = destX;
				errX = 50;
				srcPtr = srcRowPtr;
				for (x=0; x<srcSprite->w; x++) {
					errX += scale;
					while (errX >= 100) {
						if ((scanX >= 0) && (scanX < destBuf->W)) {
							if (*srcPtr) *destPtr = *srcPtr;
							destPtr++;
						}
						scanX++;
						errX -= 100;
					}
					srcPtr++;
				}
				destRowPtr += destBuf->stride;	// next line
				destPtr = destRowPtr;
				errY -= 100;
				scanY++;
				if (scanY >= destBuf->h) {
					return;
				}
			}
		}
		else {
			while ((errY >= 100) && (scanY < 0)) {
				errY -= 100;
				scanY++;
			}
			if (errY < 100) srcPtr += srcSprite->w;
			else errY -= scale;
		}
	}
}
*/
//----------------------------------------------------------------------------------------
//RLE8 COMPRESSION CODE...

#define ESC     ((uint8)0)
#define EOL	((uint8)0)
#define EOB	((uint8)1)
#define DELTA	((uint8)2)

#define OutBuffSize(x)	((x) + (((x) + 254) / 255 + 1) * 2 + 2)

static uint16 EncodeScan(uint8 *pi, uint8 *po, uint16 scanlen, uint8 EndByte) {
	uint8 *ps = pi + 1;
	uint16 outlen = 0, limit, run;

	while (scanlen)
	{
		limit = (scanlen < 255) ? scanlen : 255;
		//imath_min(scanlen, 255);
		for (run = 1; run < limit && *pi == *ps; ++run, ++ps)
			;
		if (run > 1)
		{
			scanlen -= run;
			*po++ = run;
			*po++ = *pi;
			outlen += 2;
			pi = ps++;
		} else if (scanlen < 3)
		{
			for (; scanlen; --scanlen)
			{
				*po++ = 1;
				*po++ = *pi++;
				outlen += 2;
			}
		} else
		{
			--ps;
			do
			{
				++ps;
				while ((*ps != *(ps + 1) || *ps != *(ps + 2) || *ps != *(ps + 3)) && (ps - pi) < limit)
					++ps;
			} while ((run = ps - pi) < 3);
			scanlen -= run;
			*po++ = ESC;
			*po++ = run;
			outlen += (run + 2);
			for (limit = 0; limit < run; ++limit)
				*po++ = *pi++;
			++ps;
		}
	}
	*po++ = ESC;
	*po = EndByte;
	outlen += 2;
	return outlen;
}

uint32 gr_sprite_RLE8_encode(Buffer *Source, Buffer *Dest);
uint32 gr_sprite_RLE8_encode(Buffer *Source, Buffer *Dest) {
	int i;
	uint32 Offset = 0;

	Dest->W = Source->W;
	Dest->h = Source->h;
	Dest->encoding = RLE8;
	Dest->stride = Source->stride;

	if (!(Dest->data = (uint8 *)mem_alloc(Source->h * OutBuffSize(Source->stride), "sprite data")))
	{
		return 0;
	}
	for (i = 0; i < Source->h - 1; ++i)
		Offset += EncodeScan(Source->data + i * Source->stride, Dest->data + Offset, Source->W, EOL);

	Offset += EncodeScan(Source->data + i * Source->stride, Dest->data + Offset, Source->W, EOB);

	Dest->data = (uint8 *)mem_realloc(Dest->data, Offset, "rle8 sprite data");

	return Offset;
}

} // namespace M4
