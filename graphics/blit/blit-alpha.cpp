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

#include "common/system.h"
#include "graphics/blit.h"
#include "graphics/pixelformat.h"

namespace Graphics {

namespace {

template<typename Size, bool overwriteAlpha>
inline void applyColorKeyLogic(byte *dst, const byte *src, const uint w, const uint h,
                               const uint srcDelta, const uint dstDelta,
                               const Graphics::PixelFormat &format,
                               const uint8 rKey, const uint8 gKey, const uint8 bKey,
                               const uint8 rNew, const uint8 gNew, const uint8 bNew) {

    const uint32 keyPix    = format.ARGBToColor(0,   rKey, gKey, bKey);
    const uint32 newPix    = format.ARGBToColor(0,   rNew, gNew, bNew);
    const uint32 rgbMask   = format.ARGBToColor(0,   255,  255,  255);
    const uint32 alphaMask = format.ARGBToColor(255, 0,    0,    0);

    for (uint y = 0; y < h; ++y) {
        for (uint x = 0; x < w; ++x) {
            uint32 pix = *(const Size *)src;

            if ((pix & rgbMask) == keyPix) {
                *(Size *)dst = newPix;
            } else if (overwriteAlpha) {
                *(Size *)dst = pix | alphaMask;
            }

            src += sizeof(Size);
            dst += sizeof(Size);
        }

        src += srcDelta;
        dst += dstDelta;
    }
}

template<typename Size, bool skipTransparent>
inline void setAlphaLogic(byte *dst, const byte *src, const uint w, const uint h,
                          const uint srcDelta, const uint dstDelta,
                          const Graphics::PixelFormat &format, const uint8 alpha) {

    const uint32 newAlpha  = format.ARGBToColor(alpha, 0,   0,   0);
    const uint32 rgbMask   = format.ARGBToColor(0,     255, 255, 255);
    const uint32 alphaMask = format.ARGBToColor(255,   0,   0,   0);

    for (uint y = 0; y < h; ++y) {
        for (uint x = 0; x < w; ++x) {
            uint32 pix = *(const Size *)src;

            if (!skipTransparent || (pix & alphaMask))
                *(Size *)dst = (pix & rgbMask) | newAlpha;
            else
                *(Size *)dst = pix;

            src += sizeof(Size);
            dst += sizeof(Size);
        }

        src += srcDelta;
        dst += dstDelta;
    }
}

} // End of anonymous namespace

// Function to merge a transparent color key with the alpha channel
bool applyColorKey(byte *dst, const byte *src,
                   const uint dstPitch, const uint srcPitch,
                   const uint w, const uint h,
                   const Graphics::PixelFormat &format, const bool overwriteAlpha,
                   const uint8 rKey, const uint8 gKey, const uint8 bKey,
                   const uint8 rNew, const uint8 gNew, const uint8 bNew) {

    // Faster, but larger, to provide optimized handling for each case.
    const uint srcDelta = (srcPitch - w * format.bytesPerPixel);
    const uint dstDelta = (dstPitch - w * format.bytesPerPixel);

    if (format.aBits() == 0) {
        return false;
    }

    if (overwriteAlpha) {
        if (format.bytesPerPixel == 1) {
            applyColorKeyLogic<uint8, true>(dst, src, w, h, srcDelta, dstDelta, format, rKey, gKey, bKey, rNew, gNew, bNew);
        } else if (format.bytesPerPixel == 2) {
            applyColorKeyLogic<uint16, true>(dst, src, w, h, srcDelta, dstDelta, format, rKey, gKey, bKey, rNew, gNew, bNew);
        } else if (format.bytesPerPixel == 4) {
            applyColorKeyLogic<uint32, true>(dst, src, w, h, srcDelta, dstDelta, format, rKey, gKey, bKey, rNew, gNew, bNew);
        } else {
            return false;
        }
    } else {
        if (format.bytesPerPixel == 1) {
            applyColorKeyLogic<uint8, false>(dst, src, w, h, srcDelta, dstDelta, format, rKey, gKey, bKey, rNew, gNew, bNew);
        } else if (format.bytesPerPixel == 2) {
            applyColorKeyLogic<uint16, false>(dst, src, w, h, srcDelta, dstDelta, format, rKey, gKey, bKey, rNew, gNew, bNew);
        } else if (format.bytesPerPixel == 4) {
            applyColorKeyLogic<uint32, false>(dst, src, w, h, srcDelta, dstDelta, format, rKey, gKey, bKey, rNew, gNew, bNew);
        } else {
            return false;
        }
    }

    return true;
}

// Function to set the alpha channel for all pixels to the specified value
bool setAlpha(byte *dst, const byte *src,
              const uint dstPitch, const uint srcPitch,
              const uint w, const uint h,
              const Graphics::PixelFormat &format,
              const bool skipTransparent, const uint8 alpha) {

    // Faster, but larger, to provide optimized handling for each case.
    const uint srcDelta = (srcPitch - w * format.bytesPerPixel);
    const uint dstDelta = (dstPitch - w * format.bytesPerPixel);

    if (format.aBits() == 0) {
        return false;
    }

    if (skipTransparent) {
        if (format.bytesPerPixel == 1) {
            setAlphaLogic<uint8, true>(dst, src, w, h, srcDelta, dstDelta, format, alpha);
        } else if (format.bytesPerPixel == 2) {
            setAlphaLogic<uint16, true>(dst, src, w, h, srcDelta, dstDelta, format, alpha);
        } else if (format.bytesPerPixel == 4) {
            setAlphaLogic<uint32, true>(dst, src, w, h, srcDelta, dstDelta, format, alpha);
        } else {
            return false;
        }
    } else {
        if (format.bytesPerPixel == 1) {
            setAlphaLogic<uint8, false>(dst, src, w, h, srcDelta, dstDelta, format, alpha);
        } else if (format.bytesPerPixel == 2) {
            setAlphaLogic<uint16, false>(dst, src, w, h, srcDelta, dstDelta, format, alpha);
        } else if (format.bytesPerPixel == 4) {
            setAlphaLogic<uint32, false>(dst, src, w, h, srcDelta, dstDelta, format, alpha);
        } else {
            return false;
        }
    }

    return true;
}

BlendBlit::Args::Args(byte *dst, const byte *src,
    const uint _dstPitch, const uint _srcPitch,
    const int posX, const int posY,
    const uint _width, const uint _height,
    const int _scaleX, const int _scaleY,
    const int scaleXsrcOff, const int scaleYsrcOff,
    const uint32 colorMod, const uint _flipping) :
        xp(0), yp(0), dstPitch(_dstPitch),
        width(_width), height(_height), color(colorMod),
        scaleX(_scaleX), scaleY(_scaleY), flipping(_flipping),
        scaleXoff(scaleXsrcOff), scaleYoff(scaleYsrcOff) {
    bool doScale = scaleX != SCALE_THRESHOLD || scaleY != SCALE_THRESHOLD;
    
    rgbmod   = ((colorMod & kRGBModMask) != kRGBModMask);
    alphamod = ((colorMod & kAModMask)   != kAModMask);
    inStep = 4;
    inoStep = _srcPitch;
    if (flipping & FLIP_H) {
        inStep = -inStep;
        xp = width - 1;
        if (doScale) xp = xp * scaleX / SCALE_THRESHOLD;
    }

    if (flipping & FLIP_V) {
        inoStep = -inoStep;
        yp = height - 1;
        if (doScale) yp = yp * scaleY / SCALE_THRESHOLD;
    }

    ino = src + yp * _srcPitch + xp * 4;
    outo = dst + posY * _dstPitch + posX * 4;
}

// Initialize this to nullptr at the start
BlendBlit::BlitFunc BlendBlit::blitFunc = nullptr;

// Only blits to and from 32bpp images
// So this function is just here to jump to whatever function is in
// BlendBlit::blitFunc. This way, we can detect at runtime whether or not
// the cpu has certain SIMD feature enabled or not.
void BlendBlit::blit(byte *dst, const byte *src,
                     const uint dstPitch, const uint srcPitch,
                     const int posX, const int posY,
                     const uint width, const uint height,
                     const int scaleX, const int scaleY,
                     const int scaleXsrcOff, const int scaleYsrcOff,
                     const uint32 colorMod, const uint flipping,
                     const TSpriteBlendMode blendMode,
                     const AlphaType alphaType) {
    if (width == 0 || height == 0) return;

    // If no function has been selected yet, detect and select
    if (!blitFunc) {
        // Get the correct blit function
        blitFunc = blitGeneric;
#ifdef SCUMMVM_NEON
        if (g_system->hasFeature(OSystem::kFeatureCpuNEON)) blitFunc = blitNEON;
#endif
#ifdef SCUMMVM_SSE2
        if (g_system->hasFeature(OSystem::kFeatureCpuSSE2)) blitFunc = blitSSE2;
#endif
#ifdef SCUMMVM_AVX2
        if (g_system->hasFeature(OSystem::kFeatureCpuAVX2)) blitFunc = blitAVX2;
#endif
    }
    
    Args args(dst, src, dstPitch, srcPitch, posX, posY, width, height, scaleX, scaleY, scaleXsrcOff, scaleYsrcOff, colorMod, flipping);
    blitFunc(args, blendMode, alphaType);
}

class BlendBlitImpl {
public:
/**
 * Optimized version of doBlit to be used with multiply blended blitting
 */
template<bool doscale, bool rgbmod, bool alphamod>
static void doBlitMultiplyBlendLogicGeneric(BlendBlit::Args &args) {
    const byte *in;
    byte *out;

    int scaleXCtr, scaleYCtr = args.scaleYoff;
    const byte *inBase;

    const byte rawcr = (args.color >> BlendBlit::kRModShift) & 0xFF;
    const byte rawcg = (args.color >> BlendBlit::kGModShift) & 0xFF;
    const byte rawcb = (args.color >> BlendBlit::kBModShift) & 0xFF;
    const byte ca = alphamod ? ((args.color >> BlendBlit::kAModShift) & 0xFF) : 255;
    const uint32 cr = rgbmod   ? (rawcr == 255 ? 256 : rawcr) : 256;
    const uint32 cg = rgbmod   ? (rawcg == 255 ? 256 : rawcg) : 256;
    const uint32 cb = rgbmod   ? (rawcb == 255 ? 256 : rawcb) : 256;

    for (uint32 i = 0; i < args.height; i++) {
        if (doscale) {
            inBase = args.ino + scaleYCtr / BlendBlit::SCALE_THRESHOLD * args.inoStep;
            scaleXCtr = args.scaleXoff;
        } else {
            in = args.ino;
        }
        out = args.outo;
        for (uint32 j = 0; j < args.width; j++) {
            if (doscale) {
                in = inBase + scaleXCtr / BlendBlit::SCALE_THRESHOLD * args.inStep;
            }

            uint32 ina = in[BlendBlit::kAIndex] * ca >> 8;

            if (ina != 0) {
                out[BlendBlit::kBIndex] = out[BlendBlit::kBIndex] * ((in[BlendBlit::kBIndex] * cb * ina) >> 16) >> 8;
                out[BlendBlit::kGIndex] = out[BlendBlit::kGIndex] * ((in[BlendBlit::kGIndex] * cg * ina) >> 16) >> 8;
                out[BlendBlit::kRIndex] = out[BlendBlit::kRIndex] * ((in[BlendBlit::kRIndex] * cr * ina) >> 16) >> 8;
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
static void doBlitAlphaBlendLogicGeneric(BlendBlit::Args &args) {
    const byte *in;
    byte *out;

    int scaleXCtr, scaleYCtr = args.scaleYoff;
    const byte *inBase;

    const byte ca = alphamod ? ((args.color >> BlendBlit::kAModShift) & 0xFF) : 255;
    const byte cr = rgbmod   ? ((args.color >> BlendBlit::kRModShift) & 0xFF) : 255;
    const byte cg = rgbmod   ? ((args.color >> BlendBlit::kGModShift) & 0xFF) : 255;
    const byte cb = rgbmod   ? ((args.color >> BlendBlit::kBModShift) & 0xFF) : 255;

    for (uint32 i = 0; i < args.height; i++) {
        if (doscale) {
            inBase = args.ino + scaleYCtr / BlendBlit::SCALE_THRESHOLD * args.inoStep;
            scaleXCtr = args.scaleXoff;
        } else {
            in = args.ino;
        }
        out = args.outo;
        for (uint32 j = 0; j < args.width; j++) {
            if (doscale) {
                in = inBase + scaleXCtr / BlendBlit::SCALE_THRESHOLD * args.inStep;
            }

            uint32 ina = in[BlendBlit::kAIndex] * ca >> 8;

            if (ina != 0) {
                if (rgbmod) {
                    const uint outb = (out[BlendBlit::kBIndex] * (255 - ina) >> 8);
                    const uint outg = (out[BlendBlit::kGIndex] * (255 - ina) >> 8);
                    const uint outr = (out[BlendBlit::kRIndex] * (255 - ina) >> 8);

                    out[BlendBlit::kAIndex] = 255;
                    out[BlendBlit::kBIndex] = outb + (in[BlendBlit::kBIndex] * ina * cb >> 16);
                    out[BlendBlit::kGIndex] = outg + (in[BlendBlit::kGIndex] * ina * cg >> 16);
                    out[BlendBlit::kRIndex] = outr + (in[BlendBlit::kRIndex] * ina * cr >> 16);
                } else {
                    out[BlendBlit::kAIndex] = 255;
                    out[BlendBlit::kBIndex] = (out[BlendBlit::kBIndex] * (255 - ina) + in[BlendBlit::kBIndex] * ina) >> 8;
                    out[BlendBlit::kGIndex] = (out[BlendBlit::kGIndex] * (255 - ina) + in[BlendBlit::kGIndex] * ina) >> 8;
                    out[BlendBlit::kRIndex] = (out[BlendBlit::kRIndex] * (255 - ina) + in[BlendBlit::kRIndex] * ina) >> 8;
                    
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
static void doBlitSubtractiveBlendLogicGeneric(BlendBlit::Args &args) {
    const byte *in;
    byte *out;

    int scaleXCtr, scaleYCtr = args.scaleYoff;
    const byte *inBase;

    const byte rawcr = (args.color >> BlendBlit::kRModShift) & 0xFF;
    const byte rawcg = (args.color >> BlendBlit::kGModShift) & 0xFF;
    const byte rawcb = (args.color >> BlendBlit::kBModShift) & 0xFF;
    const uint32 cr = rgbmod   ? (rawcr == 255 ? 256 : rawcr) : 256;
    const uint32 cg = rgbmod   ? (rawcg == 255 ? 256 : rawcg) : 256;
    const uint32 cb = rgbmod   ? (rawcb == 255 ? 256 : rawcb) : 256;

    for (uint32 i = 0; i < args.height; i++) {
        if (doscale) {
            inBase = args.ino + scaleYCtr / BlendBlit::SCALE_THRESHOLD * args.inoStep;
            scaleXCtr = args.scaleXoff;
        } else {
            in = args.ino;
        }
        out = args.outo;
        for (uint32 j = 0; j < args.width; j++) {
            if (doscale) {
                in = inBase + scaleXCtr / BlendBlit::SCALE_THRESHOLD * args.inStep;
            }

            out[BlendBlit::kAIndex] = 255;
            out[BlendBlit::kBIndex] = MAX<int32>(out[BlendBlit::kBIndex] - ((in[BlendBlit::kBIndex] * cb  * (out[BlendBlit::kBIndex]) * in[BlendBlit::kAIndex]) >> 24), 0);
            out[BlendBlit::kGIndex] = MAX<int32>(out[BlendBlit::kGIndex] - ((in[BlendBlit::kGIndex] * cg  * (out[BlendBlit::kGIndex]) * in[BlendBlit::kAIndex]) >> 24), 0);
            out[BlendBlit::kRIndex] = MAX<int32>(out[BlendBlit::kRIndex] - ((in[BlendBlit::kRIndex] * cr * (out[BlendBlit::kRIndex]) * in[BlendBlit::kAIndex]) >> 24), 0);

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
static void doBlitAdditiveBlendLogicGeneric(BlendBlit::Args &args) {
    const byte *in;
    byte *out;

    int scaleXCtr, scaleYCtr = args.scaleYoff;
    const byte *inBase;

    const byte rawcr = (args.color >> BlendBlit::kRModShift) & 0xFF;
    const byte rawcg = (args.color >> BlendBlit::kGModShift) & 0xFF;
    const byte rawcb = (args.color >> BlendBlit::kBModShift) & 0xFF;
    const byte ca = alphamod ? ((args.color >> BlendBlit::kAModShift) & 0xFF) : 255;
    const uint32 cr = rgbmod   ? (rawcr == 255 ? 256 : rawcr) : 256;
    const uint32 cg = rgbmod   ? (rawcg == 255 ? 256 : rawcg) : 256;
    const uint32 cb = rgbmod   ? (rawcb == 255 ? 256 : rawcb) : 256;

    for (uint32 i = 0; i < args.height; i++) {
        if (doscale) {
            inBase = args.ino + scaleYCtr / BlendBlit::SCALE_THRESHOLD * args.inoStep;
            scaleXCtr = args.scaleXoff;
        } else {
            in = args.ino;
        }
        out = args.outo;
        for (uint32 j = 0; j < args.width; j++) {
            if (doscale) {
                in = inBase + scaleXCtr / BlendBlit::SCALE_THRESHOLD * args.inStep;
            }

            uint32 ina = in[BlendBlit::kAIndex] * ca >> 8;

            if (ina != 0) {
                out[BlendBlit::kBIndex] = out[BlendBlit::kBIndex] + ((in[BlendBlit::kBIndex] * cb * ina) >> 16);
                out[BlendBlit::kGIndex] = out[BlendBlit::kGIndex] + ((in[BlendBlit::kGIndex] * cg * ina) >> 16);
                out[BlendBlit::kRIndex] = out[BlendBlit::kRIndex] + ((in[BlendBlit::kRIndex] * cr * ina) >> 16);
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
static void doBlitOpaqueBlendLogicGeneric(BlendBlit::Args &args) {
    const byte *in;
    byte *out;

    int scaleXCtr, scaleYCtr = args.scaleYoff;
    const byte *inBase;

    for (uint32 i = 0; i < args.height; i++) {
        if (doscale) {
            inBase = args.ino + (scaleYCtr + 1) / BlendBlit::SCALE_THRESHOLD * args.inoStep;
            scaleXCtr = args.scaleXoff;
        } else {
            in = args.ino;
        }
        out = args.outo;

        if (doscale) {
            for (uint32 j = 0; j < args.width; j++) {
                in = inBase + scaleXCtr / BlendBlit::SCALE_THRESHOLD * args.inStep;
                *(uint32 *)out = *(const uint32 *)in | BlendBlit::kAModMask;
                scaleXCtr += args.scaleX;
                out += 4;
            }
        } else {
            for (uint32 j = 0; j < args.width; j++) {
                *(uint32 *)out = *(const uint32 *)in | BlendBlit::kAModMask;
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
static void doBlitBinaryBlendLogicGeneric(BlendBlit::Args &args) {
    const byte *in;
    byte *out;

    int scaleXCtr, scaleYCtr = args.scaleYoff;
    const byte *inBase;

    for (uint32 i = 0; i < args.height; i++) {
        if (doscale) {
            inBase = args.ino + scaleYCtr / BlendBlit::SCALE_THRESHOLD * args.inoStep;
            scaleXCtr = args.scaleXoff;
        } else {
            in = args.ino;
        }
        out = args.outo;
        for (uint32 j = 0; j < args.width; j++) {
            if (doscale) {
                in = inBase + scaleXCtr / BlendBlit::SCALE_THRESHOLD * args.inStep;
            }

            uint32 pix = *(const uint32 *)in, pixout = *(const uint32 *)out;
            uint32 mask = (pix & BlendBlit::kAModMask) ? 0xffffffff : 0;
            pixout &= ~mask;
            pix = (pix | BlendBlit::kAModMask) & mask;
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

}; // end of class BlendBlitImpl

void BlendBlit::blitGeneric(Args &args, const TSpriteBlendMode &blendMode, const AlphaType &alphaType) {
    bool rgbmod   = ((args.color & kRGBModMask) != kRGBModMask);
    bool alphamod = ((args.color & kAModMask)   != kAModMask);
    if (args.scaleX == BlendBlit::SCALE_THRESHOLD && args.scaleY == BlendBlit::SCALE_THRESHOLD) {
        if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_OPAQUE) {
            BlendBlitImpl::doBlitOpaqueBlendLogicGeneric<false>(args);
        } else if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_BINARY) {
            BlendBlitImpl::doBlitBinaryBlendLogicGeneric<false>(args);
        } else {
            if (blendMode == BLEND_ADDITIVE) {
                if (rgbmod) {
                    if (alphamod) {
                        BlendBlitImpl::doBlitAdditiveBlendLogicGeneric<false, true, true>(args);
                    } else {
                        BlendBlitImpl::doBlitAdditiveBlendLogicGeneric<false, true, false>(args);
                    }
                } else {
                    if (alphamod) {
                        BlendBlitImpl::doBlitAdditiveBlendLogicGeneric<false, false, true>(args);
                    } else {
                        BlendBlitImpl::doBlitAdditiveBlendLogicGeneric<false, false, false>(args);
                    }
                }
            } else if (blendMode == BLEND_SUBTRACTIVE) {
                if (rgbmod) {
                    BlendBlitImpl::doBlitSubtractiveBlendLogicGeneric<false, true>(args);
                } else {
                    BlendBlitImpl::doBlitSubtractiveBlendLogicGeneric<false, false>(args);
                }
            } else if (blendMode == BLEND_MULTIPLY) {
                if (rgbmod) {
                    if (alphamod) {
                        BlendBlitImpl::doBlitMultiplyBlendLogicGeneric<false, true, true>(args);
                    } else {
                        BlendBlitImpl::doBlitMultiplyBlendLogicGeneric<false, true, false>(args);
                    }
                } else {
                    if (alphamod) {
                        BlendBlitImpl::doBlitMultiplyBlendLogicGeneric<false, false, true>(args);
                    } else {
                        BlendBlitImpl::doBlitMultiplyBlendLogicGeneric<false, false, false>(args);
                    }
                }
            } else {
                assert(blendMode == BLEND_NORMAL);
                if (rgbmod) {
                    if (alphamod) {
                        BlendBlitImpl::doBlitAlphaBlendLogicGeneric<false, true, true>(args);
                    } else {
                        BlendBlitImpl::doBlitAlphaBlendLogicGeneric<false, true, false>(args);
                    }
                } else {
                    if (alphamod) {
                        BlendBlitImpl::doBlitAlphaBlendLogicGeneric<false, false, true>(args);
                    } else {
                        BlendBlitImpl::doBlitAlphaBlendLogicGeneric<false, false, false>(args);
                    }
                }
            }
        }
    } else {
        if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_OPAQUE) {
            BlendBlitImpl::doBlitOpaqueBlendLogicGeneric<true>(args);
        } else if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_BINARY) {
            BlendBlitImpl::doBlitBinaryBlendLogicGeneric<true>(args);
        } else {
            if (blendMode == BLEND_ADDITIVE) {
                if (rgbmod) {
                    if (alphamod) {
                        BlendBlitImpl::doBlitAdditiveBlendLogicGeneric<true, true, true>(args);
                    } else {
                        BlendBlitImpl::doBlitAdditiveBlendLogicGeneric<true, true, false>(args);
                    }
                } else {
                    if (alphamod) {
                        BlendBlitImpl::doBlitAdditiveBlendLogicGeneric<true, false, true>(args);
                    } else {
                        BlendBlitImpl::doBlitAdditiveBlendLogicGeneric<true, false, false>(args);
                    }
                }
            } else if (blendMode == BLEND_SUBTRACTIVE) {
                if (rgbmod) {
                    BlendBlitImpl::doBlitSubtractiveBlendLogicGeneric<true, true>(args);
                } else {
                    BlendBlitImpl::doBlitSubtractiveBlendLogicGeneric<true, false>(args);
                }
            } else if (blendMode == BLEND_MULTIPLY) {
                if (rgbmod) {
                    if (alphamod) {
                        BlendBlitImpl::doBlitMultiplyBlendLogicGeneric<true, true, true>(args);
                    } else {
                        BlendBlitImpl::doBlitMultiplyBlendLogicGeneric<true, true, false>(args);
                    }
                } else {
                    if (alphamod) {
                        BlendBlitImpl::doBlitMultiplyBlendLogicGeneric<true, false, true>(args);
                    } else {
                        BlendBlitImpl::doBlitMultiplyBlendLogicGeneric<true, false, false>(args);
                    }
                }
            } else {
                assert(blendMode == BLEND_NORMAL);
                if (rgbmod) {
                    if (alphamod) {
                        BlendBlitImpl::doBlitAlphaBlendLogicGeneric<true, true, true>(args);
                    } else {
                        BlendBlitImpl::doBlitAlphaBlendLogicGeneric<true, true, false>(args);
                    }
                } else {
                    if (alphamod) {
                        BlendBlitImpl::doBlitAlphaBlendLogicGeneric<true, false, true>(args);
                    } else {
                        BlendBlitImpl::doBlitAlphaBlendLogicGeneric<true, false, false>(args);
                    }
                }
            }
        }
    }
}

} // End of namespace Graphics
