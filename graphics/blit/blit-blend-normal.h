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

#ifndef GRAPHICS_BLIT_BLIT_BLEND_NORMAL_H
#define GRAPHICS_BLIT_BLIT_BLEND_NORMAL_H
#include "graphics/blit.h"

namespace Graphics {

/**
 * Optimized version of doBlit to be used with multiply blended blitting
 */
template<bool doscale, bool rgbmod, bool alphamod>
void BlendBlit::doBlitMultiplyBlendLogicGeneric(Args &args) {
    const byte *in;
    byte *out;

    int scaleXCtr, scaleYCtr = args.scaleYoff;
    const byte *inBase;

    const byte rawcr = (args.color >> kRModShift) & 0xFF;
    const byte rawcg = (args.color >> kGModShift) & 0xFF;
    const byte rawcb = (args.color >> kBModShift) & 0xFF;
    const byte ca = alphamod ? ((args.color >> kAModShift) & 0xFF) : 255;
    const uint32 cr = rgbmod   ? (rawcr == 255 ? 256 : rawcr) : 256;
    const uint32 cg = rgbmod   ? (rawcg == 255 ? 256 : rawcg) : 256;
    const uint32 cb = rgbmod   ? (rawcb == 255 ? 256 : rawcb) : 256;

    for (uint32 i = 0; i < args.height; i++) {
        if (doscale) {
            inBase = args.ino + scaleYCtr / SCALE_THRESHOLD * args.inoStep;
            scaleXCtr = args.scaleXoff;
        } else {
            in = args.ino;
        }
        out = args.outo;
        for (uint32 j = 0; j < args.width; j++) {
            if (doscale) {
                in = inBase + scaleXCtr / SCALE_THRESHOLD * args.inStep;
            }

            uint32 ina = in[kAIndex] * ca >> 8;

            if (ina != 0) {
                out[kBIndex] = out[kBIndex] * ((in[kBIndex] * cb * ina) >> 16) >> 8;
                out[kGIndex] = out[kGIndex] * ((in[kGIndex] * cg * ina) >> 16) >> 8;
                out[kRIndex] = out[kRIndex] * ((in[kRIndex] * cr * ina) >> 16) >> 8;
            }

            if (doscale)
                scaleXCtr += args.scaleX;
            else
                in += args.inStep;
            out += 4;
        }
        if (doscale)
            scaleYCtr += args.scaleY;
        else
            args.ino += args.inoStep;
        args.outo += args.dstPitch;
    }

}

template<bool doscale, bool rgbmod, bool alphamod>
void BlendBlit::doBlitAlphaBlendLogicGeneric(Args &args) {
    const byte *in;
    byte *out;

    int scaleXCtr, scaleYCtr = args.scaleYoff;
    const byte *inBase;

    const byte ca = alphamod ? ((args.color >> kAModShift) & 0xFF) : 255;
    const byte cr = rgbmod   ? ((args.color >> kRModShift) & 0xFF) : 255;
    const byte cg = rgbmod   ? ((args.color >> kGModShift) & 0xFF) : 255;
    const byte cb = rgbmod   ? ((args.color >> kBModShift) & 0xFF) : 255;

    for (uint32 i = 0; i < args.height; i++) {
        if (doscale) {
            inBase = args.ino + scaleYCtr / SCALE_THRESHOLD * args.inoStep;
            scaleXCtr = args.scaleXoff;
        } else {
            in = args.ino;
        }
        out = args.outo;
        for (uint32 j = 0; j < args.width; j++) {
            if (doscale) {
                in = inBase + scaleXCtr / SCALE_THRESHOLD * args.inStep;
            }

            uint32 ina = in[kAIndex] * ca >> 8;

            if (ina != 0) {
                if (rgbmod) {
                    const uint outb = (out[kBIndex] * (255 - ina) >> 8);
                    const uint outg = (out[kGIndex] * (255 - ina) >> 8);
                    const uint outr = (out[kRIndex] * (255 - ina) >> 8);

                    out[kAIndex] = 255;
                    out[kBIndex] = outb + (in[kBIndex] * ina * cb >> 16);
                    out[kGIndex] = outg + (in[kGIndex] * ina * cg >> 16);
                    out[kRIndex] = outr + (in[kRIndex] * ina * cr >> 16);
                } else {
                    out[kAIndex] = 255;
                    out[kBIndex] = (out[kBIndex] * (255 - ina) + in[kBIndex] * ina) >> 8;
                    out[kGIndex] = (out[kGIndex] * (255 - ina) + in[kGIndex] * ina) >> 8;
                    out[kRIndex] = (out[kRIndex] * (255 - ina) + in[kRIndex] * ina) >> 8;
                    
                }
            }

            if (doscale)
                scaleXCtr += args.scaleX;
            else
                in += args.inStep;
            out += 4;
        }

        if (doscale)
            scaleYCtr += args.scaleY;
        else
            args.ino += args.inoStep;
        args.outo += args.dstPitch;
    }
}

/**
 * Optimized version of doBlit to be used with subtractive blended blitting
 */
template<bool doscale, bool rgbmod>
void BlendBlit::doBlitSubtractiveBlendLogicGeneric(Args &args) {
    const byte *in;
    byte *out;

    int scaleXCtr, scaleYCtr = args.scaleYoff;
    const byte *inBase;

    const byte rawcr = (args.color >> kRModShift) & 0xFF;
    const byte rawcg = (args.color >> kGModShift) & 0xFF;
    const byte rawcb = (args.color >> kBModShift) & 0xFF;
    const uint32 cr = rgbmod   ? (rawcr == 255 ? 256 : rawcr) : 256;
    const uint32 cg = rgbmod   ? (rawcg == 255 ? 256 : rawcg) : 256;
    const uint32 cb = rgbmod   ? (rawcb == 255 ? 256 : rawcb) : 256;

    for (uint32 i = 0; i < args.height; i++) {
        if (doscale) {
            inBase = args.ino + scaleYCtr / SCALE_THRESHOLD * args.inoStep;
            scaleXCtr = args.scaleXoff;
        } else {
            in = args.ino;
        }
        out = args.outo;
        for (uint32 j = 0; j < args.width; j++) {
            if (doscale) {
                in = inBase + scaleXCtr / SCALE_THRESHOLD * args.inStep;
            }

            out[kAIndex] = 255;
            out[kBIndex] = MAX<int32>(out[kBIndex] - ((in[kBIndex] * cb  * (out[kBIndex]) * in[kAIndex]) >> 24), 0);
            out[kGIndex] = MAX<int32>(out[kGIndex] - ((in[kGIndex] * cg  * (out[kGIndex]) * in[kAIndex]) >> 24), 0);
            out[kRIndex] = MAX<int32>(out[kRIndex] - ((in[kRIndex] * cr * (out[kRIndex]) * in[kAIndex]) >> 24), 0);

            if (doscale)
                scaleXCtr += args.scaleX;
            else
                in += args.inStep;
            out += 4;
        }
        if (doscale)
            scaleYCtr += args.scaleY;
        else
            args.ino += args.inoStep;
        args.outo += args.dstPitch;
    }
}

/**
 * Optimized version of doBlit to be used with additive blended blitting
 */
template<bool doscale, bool rgbmod, bool alphamod>
void BlendBlit::doBlitAdditiveBlendLogicGeneric(Args &args) {
    const byte *in;
    byte *out;

    int scaleXCtr, scaleYCtr = args.scaleYoff;
    const byte *inBase;

    const byte rawcr = (args.color >> kRModShift) & 0xFF;
    const byte rawcg = (args.color >> kGModShift) & 0xFF;
    const byte rawcb = (args.color >> kBModShift) & 0xFF;
    const byte ca = alphamod ? ((args.color >> kAModShift) & 0xFF) : 255;
    const uint32 cr = rgbmod   ? (rawcr == 255 ? 256 : rawcr) : 256;
    const uint32 cg = rgbmod   ? (rawcg == 255 ? 256 : rawcg) : 256;
    const uint32 cb = rgbmod   ? (rawcb == 255 ? 256 : rawcb) : 256;

    for (uint32 i = 0; i < args.height; i++) {
        if (doscale) {
            inBase = args.ino + scaleYCtr / SCALE_THRESHOLD * args.inoStep;
            scaleXCtr = args.scaleXoff;
        } else {
            in = args.ino;
        }
        out = args.outo;
        for (uint32 j = 0; j < args.width; j++) {
            if (doscale) {
                in = inBase + scaleXCtr / SCALE_THRESHOLD * args.inStep;
            }

            uint32 ina = in[kAIndex] * ca >> 8;

            if (ina != 0) {
                out[kBIndex] = out[kBIndex] + ((in[kBIndex] * cb * ina) >> 16);
                out[kGIndex] = out[kGIndex] + ((in[kGIndex] * cg * ina) >> 16);
                out[kRIndex] = out[kRIndex] + ((in[kRIndex] * cr * ina) >> 16);
            }

            if (doscale)
                scaleXCtr += args.scaleX;
            else
                in += args.inStep;
            out += 4;
        }

        if (doscale)
            scaleYCtr += args.scaleY;
        else
            args.ino += args.inoStep;
        args.outo += args.dstPitch;
    }
}

template<bool doscale>
void BlendBlit::doBlitOpaqueBlendLogicGeneric(Args &args) {
    const byte *in;
    byte *out;

    int scaleXCtr, scaleYCtr = args.scaleYoff;
    const byte *inBase;

    for (uint32 i = 0; i < args.height; i++) {
        if (doscale) {
            inBase = args.ino + (scaleYCtr + 1) / SCALE_THRESHOLD * args.inoStep;
            scaleXCtr = args.scaleXoff;
        } else {
            in = args.ino;
        }
        out = args.outo;

        if (doscale) {
            for (uint32 j = 0; j < args.width; j++) {
                in = inBase + scaleXCtr / SCALE_THRESHOLD * args.inStep;
                *(uint32 *)out = *(const uint32 *)in | kAModMask;
                scaleXCtr += args.scaleX;
                out += 4;
            }
        } else {
            for (uint32 j = 0; j < args.width; j++) {
                *(uint32 *)out = *(const uint32 *)in | kAModMask;
                in += args.inStep;
                out += 4;
            }
        }

        if (doscale)
            scaleYCtr += args.scaleY;
        else
            args.ino += args.inoStep;
        args.outo += args.dstPitch;
    }
}

template<bool doscale>
void BlendBlit::doBlitBinaryBlendLogicGeneric(Args &args) {
    const byte *in;
    byte *out;

    int scaleXCtr, scaleYCtr = args.scaleYoff;
    const byte *inBase;

    for (uint32 i = 0; i < args.height; i++) {
        if (doscale) {
            inBase = args.ino + scaleYCtr / SCALE_THRESHOLD * args.inoStep;
            scaleXCtr = args.scaleXoff;
        } else {
            in = args.ino;
        }
        out = args.outo;
        for (uint32 j = 0; j < args.width; j++) {
            if (doscale) {
                in = inBase + scaleXCtr / SCALE_THRESHOLD * args.inStep;
            }

            uint32 pix = *(const uint32 *)in, pixout = *(const uint32 *)out;
            uint32 mask = (pix & kAModMask) ? 0xffffffff : 0;
            pixout &= ~mask;
            pix = (pix | kAModMask) & mask;
            *(uint32 *)out = pixout | pix;
            
            if (doscale)
                scaleXCtr += args.scaleX;
            else
                in += args.inStep;
            out += 4;
        }
        if (doscale)
            scaleYCtr += args.scaleY;
        else
            args.ino += args.inoStep;
        args.outo += args.dstPitch;
    }
}

} // end of namespace Graphics

#endif // GRAPHICS_BLIT_BLIT_BLEND_NORMAL_H
