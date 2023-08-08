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
#include "graphics/blit/blit-blend-normal.h"
#include "graphics/blit/blit-blend-neon.h"
#include "graphics/blit/blit-blend-sse2.h"
#include "graphics/blit/blit-blend-avx2.h"

namespace Graphics {

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

// This is just a macro to expand it because its a pretty simple function where
// readabiliy doesn't matter too much and macros tend to work faster better than functors
#define BLIT_FUNC(ext) \
    void BlendBlit::blit##ext(Args &args, const TSpriteBlendMode &blendMode, const AlphaType &alphaType) { \
        bool rgbmod   = ((args.color & kRGBModMask) != kRGBModMask); \
        bool alphamod = ((args.color & kAModMask)   != kAModMask); \
        if (args.scaleX == SCALE_THRESHOLD && args.scaleY == SCALE_THRESHOLD) { \
            if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_OPAQUE) { \
                doBlitOpaqueBlendLogic##ext<false>(args); \
            } else if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_BINARY) { \
                doBlitBinaryBlendLogic##ext<false>(args); \
            } else { \
                if (blendMode == BLEND_ADDITIVE) { \
                    if (rgbmod) { \
                        if (alphamod) { \
                            doBlitAdditiveBlendLogic##ext<false, true, true>(args); \
                        } else { \
                            doBlitAdditiveBlendLogic##ext<false, true, false>(args); \
                        } \
                    } else { \
                        if (alphamod) { \
                            doBlitAdditiveBlendLogic##ext<false, false, true>(args); \
                        } else { \
                            doBlitAdditiveBlendLogic##ext<false, false, false>(args); \
                        } \
                    } \
                } else if (blendMode == BLEND_SUBTRACTIVE) { \
                    if (rgbmod) { \
                        doBlitSubtractiveBlendLogic##ext<false, true>(args); \
                    } else { \
                        doBlitSubtractiveBlendLogic##ext<false, false>(args); \
                    } \
                } else if (blendMode == BLEND_MULTIPLY) { \
                    if (rgbmod) { \
                        if (alphamod) { \
                            doBlitMultiplyBlendLogic##ext<false, true, true>(args); \
                        } else { \
                            doBlitMultiplyBlendLogic##ext<false, true, false>(args); \
                        } \
                    } else { \
                        if (alphamod) { \
                            doBlitMultiplyBlendLogic##ext<false, false, true>(args); \
                        } else { \
                            doBlitMultiplyBlendLogic##ext<false, false, false>(args); \
                        } \
                    } \
                } else { \
                    assert(blendMode == BLEND_NORMAL); \
                    if (rgbmod) { \
                        if (alphamod) { \
                            doBlitAlphaBlendLogic##ext<false, true, true>(args); \
                        } else { \
                            doBlitAlphaBlendLogic##ext<false, true, false>(args); \
                        } \
                    } else { \
                        if (alphamod) { \
                            doBlitAlphaBlendLogic##ext<false, false, true>(args); \
                        } else { \
                            doBlitAlphaBlendLogic##ext<false, false, false>(args); \
                        } \
                    } \
                } \
            } \
        } else { \
            if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_OPAQUE) { \
                doBlitOpaqueBlendLogic##ext<true>(args); \
            } else if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_BINARY) { \
                doBlitBinaryBlendLogic##ext<true>(args); \
            } else { \
                if (blendMode == BLEND_ADDITIVE) { \
                    if (rgbmod) { \
                        if (alphamod) { \
                            doBlitAdditiveBlendLogic##ext<true, true, true>(args); \
                        } else { \
                            doBlitAdditiveBlendLogic##ext<true, true, false>(args); \
                        } \
                    } else { \
                        if (alphamod) { \
                            doBlitAdditiveBlendLogic##ext<true, false, true>(args); \
                        } else { \
                            doBlitAdditiveBlendLogic##ext<true, false, false>(args); \
                        } \
                    } \
                } else if (blendMode == BLEND_SUBTRACTIVE) { \
                    if (rgbmod) { \
                        doBlitSubtractiveBlendLogic##ext<true, true>(args); \
                    } else { \
                        doBlitSubtractiveBlendLogic##ext<true, false>(args); \
                    } \
                } else if (blendMode == BLEND_MULTIPLY) { \
                    if (rgbmod) { \
                        if (alphamod) { \
                            doBlitMultiplyBlendLogic##ext<true, true, true>(args); \
                        } else { \
                            doBlitMultiplyBlendLogic##ext<true, true, false>(args); \
                        } \
                    } else { \
                        if (alphamod) { \
                            doBlitMultiplyBlendLogic##ext<true, false, true>(args); \
                        } else { \
                            doBlitMultiplyBlendLogic##ext<true, false, false>(args); \
                        } \
                    } \
                } else { \
                    assert(blendMode == BLEND_NORMAL); \
                    if (rgbmod) { \
                        if (alphamod) { \
                            doBlitAlphaBlendLogic##ext<true, true, true>(args); \
                        } else { \
                            doBlitAlphaBlendLogic##ext<true, true, false>(args); \
                        } \
                    } else { \
                        if (alphamod) { \
                            doBlitAlphaBlendLogic##ext<true, false, true>(args); \
                        } else { \
                            doBlitAlphaBlendLogic##ext<true, false, false>(args); \
                        } \
                    } \
                } \
            } \
        } \
    }
BLIT_FUNC(Generic)
#ifdef SCUMMVM_NEON
BLIT_FUNC(NEON)
#endif
#ifdef SCUMMVM_SSE2
BLIT_FUNC(SSE2)
#endif
#ifdef SCUMMVM_AVX2
BLIT_FUNC(AVX2)
#endif

} // end of namespace Graphics
