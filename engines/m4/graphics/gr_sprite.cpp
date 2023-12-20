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

#include "common/textconsole.h"
#include "m4/graphics/gr_sprite.h"
#include "m4/graphics/gr_surface.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/core/term.h"
#include "m4/mem/memman.h"
#include "m4/platform/draw.h"

namespace M4 {

/**
 * ScaleX and ScaleY are supposed to be percents, where 100 means 100%
 * S and D are Raw encoded (unencoded!) buffers.
 */
static uint8 scale_sprite(Buffer *S, Buffer *D, uint32 ScaleX, uint32 ScaleY) {
	uint16 ErrX, ErrY, i, j;
	uint8 *pScaled, *pData = S->data;

	if (!D || !S)
		error_show(FL, 'BUF!', "scale sprite h:%ld w:%ld sx:%uld sy:%uld", D->h, D->w, ScaleX, ScaleY);

	/* calculate new x size */
	D->w = S->w * ScaleX / 100;
	if (S->w * ScaleX % 100 >= 50)
		++D->w;

	/* calculate new y size */
	D->h = S->h * ScaleY / 100;
	if (S->h * ScaleY % 100 >= 50)
		++D->h;

	D->stride = D->w;

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
			for (j = 0; j < S->w; ++j)
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
			pData -= S->w;
		}
		pData += S->w;
	}
	return 0;
}

#define Scaled	((drawReq->scaleY != 100) || (drawReq->scaleX != 100 && drawReq->scaleX != -100))
#define Rle	(source.encoding == RLE8)
#define Clipped ((drawReq->x < 0) || (drawReq->y < 0) || (drawReq->x + source.w > drawReq->Dest->w) || (drawReq->y + source.h > drawReq->Dest->h))
#define Forward (drawReq->scaleX > 0)
#define Depthed (drawReq->srcDepth)
#define Shadow	(source.encoding & 0x80)
#define ClipD	(leftOffset || rightOffset || bottomCut)

uint8 gr_sprite_draw(DrawRequest *drawReq) {
	Buffer source;
#if 0
	int32 leftOffset = 0, rightOffset = 0, YPos;
	uint8 bottomCut = 0;
#endif
	uint8 *shadowBuff = nullptr, *scaledBuff = nullptr;
	Buffer afterScaled = { 0, 0, nullptr, 0, 0 };

	if (!drawReq->Src) {
		term_message("nullptr source data in sprite_draw");
		return 0;
	}

	// Negative scaleY means don't bother drawing this sprite
	if (drawReq->scaleY <= 0)
		return 0;

	if (!drawReq->Src->w || !drawReq->Src->h)
		return 1;

	// Copy DrawReq->Src to source buffer
	source = *drawReq->Src;

	// if it's RLE encoded, ensure the sprite will decode to match the expected size
	if (source.encoding & RLE8) {
		if (RLE8Decode_Size(source.data, source.stride) != (size_t)(source.stride * source.h))
			error_show(FL, 'RLE8', "RLE8 sprite suspected BAD!");
	}

	// Check for RLE encoding in case of shadows
	// There is no RLE shadow draw routine, so we have to decode shadows ahead of time.
	if ((source.encoding & RLE8) && (source.encoding & SHADOW)) {
		if (!(shadowBuff = (uint8 *)mem_alloc(source.stride * source.h, "shadow buff")))
			error_show(FL, 'OOM!', "buffer w:%uld, h:%uld", source.w, source.h);

		RLE8Decode(source.data, shadowBuff, source.stride);
		source.data = shadowBuff;
		source.encoding &= ~RLE8;
	}

	// Check for scaling
	// We scale before we draw
	if (Scaled) {
		// Check if input is RLE8 encoded
		// If it's scaled we decode it first
		if (Rle) {
			if (!(scaledBuff = (uint8 *)mem_alloc(source.stride * source.h, "scaled buffer")))
				error_show(FL, 'OOM!', "no mem: buffer w:%ld, h:%ld", source.w, source.h);

			RLE8Decode(source.data, scaledBuff, source.stride);
			source.data = scaledBuff;
			source.encoding &= ~RLE8;
		}

		if (scale_sprite(&source, &afterScaled, imath_abs(drawReq->scaleX), imath_abs(drawReq->scaleY))) {
			if (shadowBuff)
				mem_free(shadowBuff);
			if (scaledBuff)
				mem_free(scaledBuff);

			error_show(FL, 'SPSF', "gr_sprite_draw");
		}

		// Preserve encoding
		afterScaled.encoding = source.encoding;

		// Copy AfterScaled to source buffer
		source = afterScaled;
	}

	if ((drawReq->Src->encoding & SHADOW) != 0)
		warning("TODO: shadowed sprites drawing");

	M4Surface dst(*drawReq->Dest);
	dst.draw(source, drawReq->x, drawReq->y, drawReq->scaleX > 0,
		drawReq->srcDepth ? drawReq->depthCode : nullptr, drawReq->srcDepth);

#if 0
	YPos = drawReq->y;

	// Check for clipping
	// If sprite is off edge of destination buffer, do something special
	if (Clipped) {
		if (-YPos >= source.h)
			goto truly_done;

		if (YPos >= drawReq->Dest->h)
			goto truly_done;

		if (-drawReq->x >= source.w)
			goto truly_done;

		if (drawReq->x >= drawReq->Dest->w)
			goto truly_done;

		// if clipped off top, scan into sprite
		if (YPos < 0) {
			if (Rle)
				source.data = SkipRLE_Lines(-YPos, source.data);
			else
				source.data += -YPos * source.stride;

			source.h += YPos;
			YPos = 0;
		}

		// Find out if we're losing pixels on left or right
		if (drawReq->x < 0)
			leftOffset = -drawReq->x;

		if (drawReq->x + source.w > drawReq->Dest->w)
			rightOffset = drawReq->x + source.w - drawReq->Dest->w;

		if (YPos + source.h > drawReq->Dest->h) {
			source.h -= YPos + source.h - drawReq->Dest->h;
			bottomCut = 1;
		}
	}

	// After all the necessary changes (scaling/shadow RLE expansion)
	if (Rle) {
		// Will be RLE_Draw
		if (ClipD)
			if (Depthed)
				if (Forward)
					RLE_DrawDepthOffs(&source, drawReq->Dest, drawReq->x, YPos, drawReq->srcDepth, drawReq->depthCode, leftOffset, rightOffset);
				else
					RLE_DrawDepthRevOffs(&source, drawReq->Dest, drawReq->x, YPos, drawReq->srcDepth, drawReq->depthCode, leftOffset, rightOffset);
			else
				if (Forward)
					RLE_DrawOffs(&source, drawReq->Dest, drawReq->x, YPos, leftOffset, rightOffset);
				else
					RLE_DrawRevOffs(&source, drawReq->Dest, drawReq->x, YPos, leftOffset, rightOffset);
		else
			if (Depthed)
				if (Forward)
					RLE_DrawDepth(&source, drawReq->Dest, drawReq->x, YPos, drawReq->srcDepth, drawReq->depthCode);
				else
					RLE_DrawDepthRev(&source, drawReq->Dest, drawReq->x, YPos, drawReq->srcDepth, drawReq->depthCode);
			else
				if (Forward)
					RLE_Draw(&source, drawReq->Dest, drawReq->x, YPos);
				else
					RLE_DrawRev(&source, drawReq->Dest, drawReq->x, YPos);
	} else if (Shadow) {
		// Will be Raw_SDraw
		if (ClipD)
			if (Depthed)
				if (Forward)
					Raw_SDrawDepthOffs(&source, drawReq->Dest, drawReq->x, YPos, drawReq->srcDepth, drawReq->depthCode, leftOffset, rightOffset, drawReq->Pal, drawReq->ICT);
				else
					Raw_SDrawDepthRevOffs(&source, drawReq->Dest, drawReq->x, YPos, drawReq->srcDepth, drawReq->depthCode, leftOffset, rightOffset, drawReq->Pal, drawReq->ICT);
			else
				if (Forward)
					Raw_SDrawOffs(&source, drawReq->Dest, drawReq->x, YPos, leftOffset, rightOffset, drawReq->Pal, drawReq->ICT);
				else
					Raw_SDrawRevOffs(&source, drawReq->Dest, drawReq->x, YPos, leftOffset, rightOffset, drawReq->Pal, drawReq->ICT);
		else
			if (Depthed)
				if (Forward)
					Raw_SDrawDepth(&source, drawReq->Dest, drawReq->x, YPos, drawReq->srcDepth, drawReq->depthCode, drawReq->Pal, drawReq->ICT);
				else
					Raw_SDrawDepthRev(&source, drawReq->Dest, drawReq->x, YPos, drawReq->srcDepth, drawReq->depthCode, drawReq->Pal, drawReq->ICT);
			else
				if (Forward)
					Raw_SDraw(&source, drawReq->Dest, drawReq->x, YPos, drawReq->Pal, drawReq->ICT);
				else
					Raw_SDrawRev(&source, drawReq->Dest, drawReq->x, YPos, drawReq->Pal, drawReq->ICT);
	} else {
		// Will be Raw_Draw
		if (ClipD)
			if (Depthed)
				if (Forward)
					Raw_DrawDepthOffs(&source, drawReq->Dest, drawReq->x, YPos, drawReq->srcDepth, drawReq->depthCode, leftOffset, rightOffset);
				else
					Raw_DrawDepthRevOffs(&source, drawReq->Dest, drawReq->x, YPos, drawReq->srcDepth, drawReq->depthCode, leftOffset, rightOffset);
			else
				if (Forward)
					Raw_DrawOffs(&source, drawReq->Dest, drawReq->x, YPos, leftOffset, rightOffset);
				else
					Raw_DrawRevOffs(&source, drawReq->Dest, drawReq->x, YPos, leftOffset, rightOffset);
		else
			if (Depthed)
				if (Forward)
					Raw_DrawDepth(&source, drawReq->Dest, drawReq->x, YPos, drawReq->srcDepth, drawReq->depthCode);
				else
					Raw_DrawDepthRev(&source, drawReq->Dest, drawReq->x, YPos, drawReq->srcDepth, drawReq->depthCode);
			else
				if (Forward)
					Raw_Draw(&source, drawReq->Dest, drawReq->x, YPos);
				else
					Raw_DrawRev(&source, drawReq->Dest, drawReq->x, YPos);
	}

truly_done:
#endif

	if (shadowBuff)
		mem_free(shadowBuff);

	if (scaledBuff)
		mem_free(scaledBuff);

	if (afterScaled.data)
		mem_free(afterScaled.data);

	return 0;
}

} // namespace M4
