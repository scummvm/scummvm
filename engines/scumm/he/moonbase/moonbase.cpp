/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/winexe_pe.h"

#include "scumm/he/intern_he.h"
#include "scumm/he/moonbase/moonbase.h"
#include "scumm/he/moonbase/ai_main.h"
#ifdef USE_LIBCURL
#include "scumm/he/moonbase/net_main.h"
#endif

namespace Scumm {

Moonbase::Moonbase(ScummEngine_v100he *vm) : _vm(vm) {
	_exe = new Common::PEResources();

	initFOW();

	_ai = new AI(_vm);
#ifdef USE_LIBCURL
	_net = new Net(_vm);
#endif
}

Moonbase::~Moonbase() {
	delete _exe;
	delete _ai;
#ifdef USE_LIBCURL
	delete _net;
#endif
}

int Moonbase::readFromArray(int array, int y, int x) {
	_vm->VAR(_vm->VAR_U32_ARRAY_UNK) = array;

	return _vm->readArray(_vm->VAR_U32_ARRAY_UNK, y, x);
}

void Moonbase::deallocateArray(int array) {
	_vm->VAR(_vm->VAR_U32_ARRAY_UNK) = array;

	return _vm->nukeArray(_vm->VAR_U32_ARRAY_UNK);
}

int Moonbase::callScummFunction(int scriptNumber, int paramCount,...) {
	va_list va_params;
	va_start(va_params, paramCount);
	int args[25];

	memset(args, 0, sizeof(args));

	Common::String str;
	str = Common::String::format("Moonbase::callScummFunction(%d, [", scriptNumber);

	for (int i = 0; i < paramCount; i++) {
		args[i] = va_arg(va_params, int);

		str += Common::String::format("%d ", args[i]);
	}
	str += "])";

	debug(3, "%s", str.c_str());


	va_end(va_params);

	_vm->runScript(scriptNumber, 0, 1, args);

	return _vm->pop();
}


void Moonbase::blitT14WizImage(uint8 *dst, int dstw, int dsth, int dstPitch, const Common::Rect *clipBox,
		 uint8 *wizd, int x, int y, int rawROP, int paramROP) {
	bool premulAlpa = false;

	if (rawROP == 1)
		premulAlpa = true;

	Common::Rect clippedDstRect(dstw, dsth);
	if (clipBox) {
		Common::Rect clip(clipBox->left, clipBox->top, clipBox->right, clipBox->bottom);
		if (clippedDstRect.intersects(clip)) {
			clippedDstRect.clip(clip);
		} else {
			return;
		}
	}

	int width = READ_LE_UINT16(wizd + 0x8 + 0);
	int height = READ_LE_UINT16(wizd + 0x8 + 2);

	Common::Rect srcLimitsRect(width, height);
	Common::Rect dstOperation(x, y, x + width, y + height);
	if (!clippedDstRect.intersects(dstOperation))
		return;
	Common::Rect clippedRect = clippedDstRect.findIntersectingRect(dstOperation);

	int cx = clippedRect.right - clippedRect.left;
	int cy = clippedRect.bottom - clippedRect.top;

	int sx = ((clippedRect.left - x) + srcLimitsRect.left);
	int sy = ((clippedRect.top - y) + srcLimitsRect.top);

	dst += clippedRect.top * dstPitch + clippedRect.left * 2;

	int headerSize = READ_LE_UINT32(wizd + 0x4);
	uint8 *dataPointer = wizd + 0x8 + headerSize;

	for (int i = 0; i < sy; i++) {
		uint16 lineSize = READ_LE_UINT16(dataPointer + 0);

		dataPointer += lineSize;
	}

	for (int i = 0; i < cy; i++) {
		uint16 lineSize      = READ_LE_UINT16(dataPointer + 0);
		uint8 *singlesOffset = READ_LE_UINT16(dataPointer + 2) + dataPointer;
		uint8 *quadsOffset   = READ_LE_UINT16(dataPointer + 4) + dataPointer;

		int pixels = 0;
		byte *dst1 = dst;
		byte *codes = dataPointer + 6;

		while (1) {
			int code = *codes - 2;
			codes++;

			if (code <= 0) { // quad or single
				uint8 *src;
				int cnt;
				if (code == 0) { // quad
					src = quadsOffset;
					quadsOffset += 8;
					cnt = 4; // 4 pixels
				} else { // single
					src = singlesOffset;
					singlesOffset += 2;
					cnt = 1;
				}

				for (int c = 0; c < cnt; c++) {
					if (pixels >= sx) {
						if (rawROP == 1) { // MMX_PREMUL_ALPHA_COPY
							WRITE_LE_UINT16(dst1, READ_LE_UINT16(src));
						} else if (rawROP == 2) { // MMX_ADDITIVE
							uint16 color = READ_LE_UINT16(src);
							uint16 orig = READ_LE_UINT16(dst1);

							uint32 r = MIN<uint32>(0x7c00, (orig & 0x7c00) + (color & 0x7c00));
							uint32 g = MIN<uint32>(0x03e0, (orig & 0x03e0) + (color & 0x03e0));
							uint32 b = MIN<uint32>(0x001f, (orig & 0x001f) + (color & 0x001f));
							WRITE_LE_UINT16(dst1, (r | g | b));
						} else if (rawROP == 5) { // MMX_CHEAP_50_50
							uint16 color = (READ_LE_UINT16(src) >> 1) & 0x3DEF;
							uint16 orig = (READ_LE_UINT16(dst1) >> 1) & 0x3DEF;
							WRITE_LE_UINT16(dst1, (color + orig));
						}
						dst1 += 2;
					}
					src += 2;
					pixels++;
					if (pixels >= cx + sx)
						break;
				}
			} else { // skip
				if ((code & 1) == 0) {
					code >>= 1;

					for (int j = 0; j < code; j++) {
						if (pixels >= sx)
							dst1 += 2;
						pixels++;
						if (pixels >= cx + sx)
							break;
					}
				} else { // special case
					if (pixels >= sx) {
						int alpha = code >> 1;
						uint16 color = READ_LE_UINT16(singlesOffset);
						uint32 orig = READ_LE_UINT16(dst1);

						if (!premulAlpa) {
							WRITE_LE_UINT16(dst1, color); // ENABLE_PREMUL_ALPHA = 0
						} else {
							if (alpha > 32) {
								alpha -= 32;

								uint32 oR = orig & 0x7c00;
								uint32 oG = orig & 0x03e0;
								uint32 oB = orig & 0x1f;
								uint32 dR = ((((color & 0x7c00) - oR) * alpha) >> 5) + oR;
								uint32 dG = ((((color &  0x3e0) - oG) * alpha) >> 5) + oG;
								uint32 dB = ((((color &   0x1f) - oB) * alpha) >> 5) + oB;

								WRITE_LE_UINT16(dst1, (dR & 0x7c00) | (dG & 0x3e0) | (dB & 0x1f));
							} else {
								uint32 pix = ((orig << 16) | orig) & 0x3e07c1f;
								pix = (((pix * alpha) & 0xffffffff) >> 5) & 0x3e07c1f;
								pix = ((pix >> 16) + pix + color) & 0xffff;
								WRITE_LE_UINT16(dst1, pix);
							}
						}

						dst1 += 2;
					}
					singlesOffset += 2;
					pixels++;
				}
			}

			if (pixels >= cx + sx)
				break;
		}

		dataPointer += lineSize;
		dst += dstPitch;
	}
}

} // End of namespace Scumm
