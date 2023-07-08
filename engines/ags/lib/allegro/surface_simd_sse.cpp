#if defined(__x86_64__) || defined(__i686__)

#include "ags/lib/allegro/gfx.h"
#include "ags/lib/allegro/color.h"
#include "ags/lib/allegro/flood.h"
#include "ags/ags.h"
#include "ags/globals.h"
#include "common/textconsole.h"
#include "graphics/screen.h"

#include "ags/lib/allegro/surface_simd_neon.h"

namespace AGS3 {

// This template handles 2bpp and 4bpp, the other specializations handle 1bpp and format conversion blits
template<int DestBytesPerPixel, int SrcBytesPerPixel, int ScaleThreshold>
void BITMAP::drawInner4BppWithConv(int yStart, int xStart, uint32 transColor, uint32 alphaMask, PALETTE palette, bool useTint, bool sameFormat, const ::Graphics::ManagedSurface &src, ::Graphics::Surface &destArea, bool horizFlip, bool vertFlip, bool skipTrans, int srcAlpha, int tintRed, int tintGreen, int tintBlue, const Common::Rect &dstRect, const Common::Rect &srcArea, const BlenderMode blenderMode, int scaleX, int scaleY) {
    drawInnerGeneric<ScaleThreshold>(yStart, xStart, transColor, alphaMask, palette, useTint, sameFormat, src, destArea, horizFlip, vertFlip, skipTrans, srcAlpha, tintRed, tintGreen, tintBlue, dstRect, srcArea, blenderMode, scaleX, scaleY);
}

template<int ScaleThreshold>
void BITMAP::drawInner2Bpp(int yStart, int xStart, uint32 transColor, uint32 alphaMask, PALETTE palette, bool useTint, bool sameFormat, const ::Graphics::ManagedSurface &src, ::Graphics::Surface &destArea, bool horizFlip, bool vertFlip, bool skipTrans, int srcAlpha, int tintRed, int tintGreen, int tintBlue, const Common::Rect &dstRect, const Common::Rect &srcArea, const BlenderMode blenderMode, int scaleX, int scaleY) {
    drawInnerGeneric<ScaleThreshold>(yStart, xStart, transColor, alphaMask, palette, useTint, sameFormat, src, destArea, horizFlip, vertFlip, skipTrans, srcAlpha, tintRed, tintGreen, tintBlue, dstRect, srcArea, blenderMode, scaleX, scaleY);
}

template<int ScaleThreshold>
void BITMAP::drawInner1Bpp(int yStart, int xStart, uint32 transColor, uint32 alphaMask, PALETTE palette, bool useTint, bool sameFormat, const ::Graphics::ManagedSurface &src, ::Graphics::Surface &destArea, bool horizFlip, bool vertFlip, bool skipTrans, int srcAlpha, int tintRed, int tintGreen, int tintBlue, const Common::Rect &dstRect, const Common::Rect &srcArea, const BlenderMode blenderMode, int scaleX, int scaleY) {
    drawInnerGeneric<ScaleThreshold>(yStart, xStart, transColor, alphaMask, palette, useTint, sameFormat, src, destArea, horizFlip, vertFlip, skipTrans, srcAlpha, tintRed, tintGreen, tintBlue, dstRect, srcArea, blenderMode, scaleX, scaleY);
}


template void BITMAP::drawInner4BppWithConv<4, 4, 0>(int, int, uint32, uint32, PALETTE, bool, bool, const ::Graphics::ManagedSurface &, ::Graphics::Surface &, bool, bool, bool, int, int, int, int, const Common::Rect &, const Common::Rect &, const BlenderMode, int, int);
template void BITMAP::drawInner4BppWithConv<4, 4, 0x100>(int, int, uint32, uint32, PALETTE, bool, bool, const ::Graphics::ManagedSurface &, ::Graphics::Surface &, bool, bool, bool, int, int, int, int, const Common::Rect &, const Common::Rect &, const BlenderMode, int, int);
template void BITMAP::drawInner4BppWithConv<4, 2, 0>(int, int, uint32, uint32, PALETTE, bool, bool, const ::Graphics::ManagedSurface &, ::Graphics::Surface &, bool, bool, bool, int, int, int, int, const Common::Rect &, const Common::Rect &, const BlenderMode, int, int);
template void BITMAP::drawInner4BppWithConv<4, 2, 0x100>(int, int, uint32, uint32, PALETTE, bool, bool, const ::Graphics::ManagedSurface &, ::Graphics::Surface &, bool, bool, bool, int, int, int, int, const Common::Rect &, const Common::Rect &, const BlenderMode, int, int);
template void BITMAP::drawInner4BppWithConv<2, 4, 0>(int, int, uint32, uint32, PALETTE, bool, bool, const ::Graphics::ManagedSurface &, ::Graphics::Surface &, bool, bool, bool, int, int, int, int, const Common::Rect &, const Common::Rect &, const BlenderMode, int, int);
template void BITMAP::drawInner4BppWithConv<2, 4, 0x100>(int, int, uint32, uint32, PALETTE, bool, bool, const ::Graphics::ManagedSurface &, ::Graphics::Surface &, bool, bool, bool, int, int, int, int, const Common::Rect &, const Common::Rect &, const BlenderMode, int, int);
template void BITMAP::drawInner2Bpp<0>(int, int, uint32, uint32, PALETTE, bool, bool, const ::Graphics::ManagedSurface &, ::Graphics::Surface &, bool, bool, bool, int, int, int, int, const Common::Rect &, const Common::Rect &, const BlenderMode, int, int);
template void BITMAP::drawInner2Bpp<0x100>(int, int, uint32, uint32, PALETTE, bool, bool, const ::Graphics::ManagedSurface &, ::Graphics::Surface &, bool, bool, bool, int, int, int, int, const Common::Rect &, const Common::Rect &, const BlenderMode, int, int);
template void BITMAP::drawInner1Bpp<0>(int, int, uint32, uint32, PALETTE, bool, bool, const ::Graphics::ManagedSurface &, ::Graphics::Surface &, bool, bool, bool, int, int, int, int, const Common::Rect &, const Common::Rect &, const BlenderMode, int, int);
template void BITMAP::drawInner1Bpp<0x100>(int, int, uint32, uint32, PALETTE, bool, bool, const ::Graphics::ManagedSurface &, ::Graphics::Surface &, bool, bool, bool, int, int, int, int, const Common::Rect &, const Common::Rect &, const BlenderMode, int, int);

} // namespace AGS3

#endif
