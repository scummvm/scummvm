#include "ags/lib/allegro/surface.h"
#include "ags/lib/allegro/surface_simd_neon.h"
#include "ags/lib/allegro/surface_simd_sse.h"

// There is no SIMD implementation on this platform
#ifndef AGS_LIB_ALLEGRO_SURFACE_SIMD_IMPL

namespace AGS3 {

template<int DestBytesPerPixel, int SrcBytesPerPixel, int ScaleThreshold>
void BITMAP::drawInner4BppWithConv(int yStart, int xStart, uint32 transColor, uint32 alphaMask, PALETTE palette, bool useTint, bool sameFormat, const ::Graphics::ManagedSurface &src, ::Graphics::Surface &destArea, bool horizFlip, bool vertFlip, bool skipTrans, int srcAlpha, int tintRed, int tintGreen, int tintBlue, const Common::Rect &dstRect, const Common::Rect &srcArea, const BlenderMode blenderMode, int scaleX, int scaleY) {
	drawInnerGeneric<DestBytesPerPixel, SrcBytesPerPixel, ScaleThreshold>(yStart, xStart, transColor, alphaMask, palette, useTint, sameFormat, src, destArea, horizFlip, vertFlip, skipTrans, srcAlpha, tintRed, tintGreen, tintBlue, dstRect, srcArea, blenderMode, scaleX, scaleY);
}
template<int ScaleThreshold>
void BITMAP::drawInner2Bpp(int yStart, int xStart, uint32 transColor, uint32 alphaMask, PALETTE palette, bool useTint, bool sameFormat, const ::Graphics::ManagedSurface &src, ::Graphics::Surface &destArea, bool horizFlip, bool vertFlip, bool skipTrans, int srcAlpha, int tintRed, int tintGreen, int tintBlue, const Common::Rect &dstRect, const Common::Rect &srcArea, const BlenderMode blenderMode, int scaleX, int scaleY) {
	drawInnerGeneric<2, 2, ScaleThreshold>(yStart, xStart, transColor, alphaMask, palette, useTint, sameFormat, src, destArea, horizFlip, vertFlip, skipTrans, srcAlpha, tintRed, tintGreen, tintBlue, dstRect, srcArea, blenderMode, scaleX, scaleY);
}
template<int ScaleThreshold>
void BITMAP::drawInner1Bpp(int yStart, int xStart, uint32 transColor, uint32 alphaMask, PALETTE palette, bool useTint, bool sameFormat, const ::Graphics::ManagedSurface &src, ::Graphics::Surface &destArea, bool horizFlip, bool vertFlip, bool skipTrans, int srcAlpha, int tintRed, int tintGreen, int tintBlue, const Common::Rect &dstRect, const Common::Rect &srcArea, const BlenderMode blenderMode, int scaleX, int scaleY) {
	drawInnerGeneric<1, 1, ScaleThreshold>(yStart, xStart, transColor, alphaMask, palette, useTint, sameFormat, src, destArea, horizFlip, vertFlip, skipTrans, srcAlpha, tintRed, tintGreen, tintBlue, dstRect, srcArea, blenderMode, scaleX, scaleY);
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
