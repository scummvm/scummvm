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

#include "ags/shared/debugging/out.h"
#include "common/scummsys.h"
#include "common/debug.h"
#include "ags/shared/core/platform.h"
#include "ags/shared/gfx/gfx_def.h"
#include "common/scummsys.h"
#include "ags/lib/allegro/color.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/gfx/image.h"
#include "ags/lib/allegro/surface.h"
#include "ags/shared/debugging/debug_manager.h"
#include "ags/globals.h"
#include "graphics/managed_surface.h"
#include "graphics/pixelformat.h"

namespace AGS3 {

namespace GfxDef = AGS::Shared::GfxDef;
using namespace AGS::Shared;

void Test_GfxSpeed(bool enableSimd, size_t blenderModeStart, size_t blenderModeEnd) {
	uint oldSimdFlags = _G(simd_flags);
	if (!enableSimd) _G(simd_flags) = AGS3::Globals::SIMD_NONE;
	if (enableSimd) debug("SIMD optimizations: true\n");
	else debug("SIMD optmizations: false\n");
	Bitmap *benchgfx32 = BitmapHelper::CreateBitmap(100, 100, 32);
	Bitmap *benchgfx16 = BitmapHelper::CreateBitmapCopy(benchgfx32, 16);
	Bitmap *benchgfx8 = BitmapHelper::CreateBitmap(100, 100, 8);
	Bitmap *dest32 = BitmapHelper::CreateBitmap(100, 100, 32);
	Bitmap *dest16 = BitmapHelper::CreateBitmap(100, 100, 16);
	Bitmap *dest8 = BitmapHelper::CreateBitmap(100, 100, 8);
	int benchRuns[] = {1000, 10000, 100000};
	int blenderModes[] = {kRgbToRgbBlender, kSourceAlphaBlender, kArgbToArgbBlender, kOpaqueBlenderMode, kTintLightBlenderMode};
	//const char *modeNames[] = {"RGB to RGB", "Source Alpha", "ARGB to ARGB", "Opaque", "Tint with Light"};
	Bitmap *destinations[] = {dest32, dest16, dest8};
	Bitmap *graphics[] = {benchgfx32, benchgfx16, benchgfx8};
	uint64 time = 0, numIters = 0, timeNotStretched = 0, numItersNotStretched = 0, timeCommon = 0, numItersCommon = 0;
	//int bpps[] = {32, 16, 8};
	if (blenderModeEnd >= sizeof(blenderModes) / sizeof(blenderModes[0])) blenderModeEnd = (sizeof(blenderModes) / sizeof(blenderModes[0])) - 1;
	for (int dest = 0; dest < 3; dest++) {
		for (int gfx = 0; gfx < 3; gfx++) {
			if (dest == 2 && gfx != 2) continue;
			for (size_t mode = blenderModeStart; mode <= blenderModeEnd; mode++) {
				for (int runs = 0; (size_t)runs < sizeof(benchRuns)/sizeof(int); runs++) {
					uint32 start, end;
					_G(_blender_mode) = (AGS3::BlenderMode)blenderModes[mode];
					//if (runs == 2) debug("Dest: %d bpp, Gfx: %d bpp, Blender: %s, Stretched: false, Iters: %d\n", bpps[dest], bpps[gfx], modeNames[mode], benchRuns[runs]);
					start = std::chrono::high_resolution_clock::now();
					for (int i = 0; i < benchRuns[runs]; i++)
						destinations[dest]->Blit(graphics[gfx], 0, 0, kBitmap_Transparency);
					end = std::chrono::high_resolution_clock::now();
					timeNotStretched += end - start;
					numItersNotStretched += benchRuns[runs];
					if (mode == kArgbToArgbBlender || mode == kRgbToRgbBlender || mode == kRgbToArgbBlender || mode == kArgbToRgbBlender) {
						timeCommon += end - start;
						numItersCommon += benchRuns[runs];
					}
					time += end - start;
					//if (runs == 2) debug("exec time (mills): %u\n\n", end - start);
					//if (runs == 2) debug("Dest: %d bpp, Gfx: %d bpp, Blender: %s, Stretched: true, Iters: %d\n", bpps[dest], bpps[gfx], modeNames[mode], benchRuns[runs]);
					start = std::chrono::high_resolution_clock::now();
					for (int i = 0; i < benchRuns[runs]; i++)
						destinations[dest]->StretchBlt(graphics[gfx], Rect(0, 0, 99, 99), kBitmap_Transparency);
					end = std::chrono::high_resolution_clock::now();
					time += end - start;
					numIters += benchRuns[runs] * 2;
					//if (runs == 2) debug("exec time (mills): %u\n\n", end - start);
				}
			}
		}
	}

	debug("Over all blender modes, pixel formats, and stretching sizes (%f) avg millis per call.", (double)time / (double)numIters);
	debug("Over all blender modes, pixel formats, but only unstretched (%f) avg millis per call.", (double)timeNotStretched / (double)numItersNotStretched);
	debug("Over most common blender modes, all pixel formats, but only unstretched (%f) avg millis per call.", (double)timeCommon / (double)numItersCommon);
	
	delete benchgfx32;
	delete benchgfx16;
	delete benchgfx8;
	delete dest32;
	delete dest16;
	delete dest8;

	if (!enableSimd) _G(simd_flags) = oldSimdFlags;
}


void printInfo(uint8 srcA, uint8 srcR, uint8 srcG, uint8 srcB, uint8 destA, uint8 destR, uint8 destG, uint8 destB, uint32 alpha, uint32 controlCol, uint32 simdCol) {
}
void Test_BlenderModes() {
	constexpr int depth = 2;
	Graphics::ManagedSurface owner(16, 16, Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24));
	BITMAP dummy(&owner);
	Graphics::ManagedSurface owner16(16, 16, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
	BITMAP dummy16(&owner16);
	Graphics::ManagedSurface ownerDest(16, 16, Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24));
	BITMAP dummyDest(&ownerDest);
	Graphics::ManagedSurface ownerDest16(16, 16, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
	BITMAP dummyDest16(&ownerDest16);
	for (int blenderMode = (int)kSourceAlphaBlender; blenderMode <= (int)kTintLightBlenderMode; blenderMode++) {
		_G(_blender_mode) = (BlenderMode)blenderMode;
		for (int srcR = 0; srcR < 255; srcR += (1 << (8 - depth))) {
			for (int srcG = 0; srcG < 255; srcG += (1 << (8 - depth))) {
				for (int srcB = 0; srcB < 255; srcB += (1 << (8 - depth))) {
					for (int srcA = 0; srcA < 255; srcA += (1 << (8 - depth))) {
						for (int destR = 0; destR < 255; destR += (1 << (8 - depth))) {
							for (int destG = 0; destG < 255; destG += (1 << (8 - depth))) {
								for (int destB = 0; destB < 255; destB += (1 << (8 - depth))) {
									for (int destA = 0; destA < 255; destA += (1 << (8 - depth))) {
										for (uint32 alpha = 0; alpha < 255; alpha += (1 << (8 - depth))) {
											// First run the non-vectorized version of the code
											uint32 controlCol = 0, simdCol = 0;
											uint16 control2bppCol = 0, simd2bppCol = 0;
											uint8 a, r, g, b, a16, r16, g16, b16;
											a = r = g = b = a16 = r16 = g16 = b16 = 0;
											
											auto printInfo = [&]() {
												debug("src argb: %d, %d, %d, %d dest argb: %d, %d, %d, %d a: %d", srcA, srcR, srcG, srcB, destA, destR, destG, destB, alpha);
												switch ((BlenderMode)blenderMode) {
													case kSourceAlphaBlender: debug("blenderMode: kSourceAlphaBlender"); break;
													case kArgbToArgbBlender: debug("blenderMode: kArgbToArgbBlender"); break;
													case kArgbToRgbBlender: debug("blenderMode: kArgbToRgbBlender"); break;
													case kRgbToArgbBlender: debug("blenderMode: kRgbToArgbBlender"); break;
													case kRgbToRgbBlender: debug("blenderMode: kRgbToRgbBlender"); break;
													case kAlphaPreservedBlenderMode: debug("blenderMode: kAlphaPreservedBlenderMode"); break;
													case kOpaqueBlenderMode: debug("blenderMode: kOpaqueBlenderMode"); break;
													case kAdditiveBlenderMode: debug("blenderMode: kAdditiveBlenderMode"); break;
													case kTintBlenderMode: debug("blenderMode: kTintBlenderMode"); break;
													case kTintLightBlenderMode: debug("blenderMode: kTintLightBlenderMode"); break;
												}
												debug("controlCol %x argb: %d, %d, %d, %d", controlCol, a, r, g, b);
												debug("simdCol %x argb: %d, %d, %d, %d", simdCol, (simdCol >> 24), ((simdCol >> 16) & 0xff), ((simdCol >> 8) & 0xff), (simdCol & 0xff));
												debug("control2bppCol %x rgb: %d, %d, %d", control2bppCol, r16, g16, b16);
												debug("simd2bppCol %x rgb: %d, %d, %d", simd2bppCol, (simd2bppCol >> 11), ((simd2bppCol >> 5) & 0x3f), (simd2bppCol & 0x1f));
											};

											uint oldSimdFlags = _G(simd_flags);
											_G(simd_flags) = AGS3::Globals::SIMD_NONE;
											*(uint32 *)dummy.getBasePtr(0, 0) = dummy.format.ARGBToColor(srcA, srcR, srcG, srcB);
											*(uint32 *)dummyDest.getBasePtr(0, 0) = dummyDest.format.ARGBToColor(destA, destR, destG, destB);
											dummyDest.draw(&dummy, Common::Rect(16, 16), 0, 0, false, false, false, alpha);
											controlCol = dummyDest.getpixel(0, 0);
											dummyDest.format.colorToARGB(dummyDest.getpixel(0, 0), a, r, g, b);

											*(uint16 *)dummy16.getBasePtr(0, 0) = dummy16.format.ARGBToColor(srcA, srcR, srcG, srcB);
											*(uint16 *)dummyDest16.getBasePtr(0, 0) = dummyDest16.format.ARGBToColor(destA, destR, destG, destB);
											dummyDest16.draw(&dummy16, Common::Rect(16, 16), 0, 0, false, false, false, alpha);
											control2bppCol = dummyDest16.getpixel(0, 0);
											dummyDest16.format.colorToARGB(dummyDest16.getpixel(0, 0), a16, r16, g16, b16);
											a16 >>= 3; r16 >>= 3; g16 >>= 2; b16 >>= 3;
											_G(simd_flags) = oldSimdFlags;

											*(uint32 *)dummy.getBasePtr(0, 0) = dummy.format.ARGBToColor(srcA, srcR, srcG, srcB);
											*(uint32 *)dummyDest.getBasePtr(0, 0) = dummyDest.format.ARGBToColor(destA, destR, destG, destB);
											dummyDest.draw(&dummy, Common::Rect(16, 16), 0, 0, false, false, false, alpha);
											simdCol = dummyDest.getpixel(0, 0);

											*(uint16 *)dummy16.getBasePtr(0, 0) = dummy16.format.ARGBToColor(srcA, srcR, srcG, srcB);
											*(uint16 *)dummyDest16.getBasePtr(0, 0) = dummyDest16.format.ARGBToColor(destA, destR, destG, destB);
											dummyDest16.draw(&dummy16, Common::Rect(16, 16), 0, 0, false, false, false, alpha);
											simd2bppCol = dummyDest16.getpixel(0, 0);

											int tolerance, tolerance16;
											switch ((BlenderMode)blenderMode) {
												// These need to be IDENTICAL for lamplight city to work
												// It would be nice to get tolerance16 down to 0 though...
												case kRgbToRgbBlender:
												case kArgbToRgbBlender:
												case kSourceAlphaBlender:
												case kAlphaPreservedBlenderMode:
												tolerance = 0;
												tolerance16 = 1;
												break;

												// These can be 1 or 2 off, as long as they look the same
												case kArgbToArgbBlender:
												case kRgbToArgbBlender:
												case kOpaqueBlenderMode:
												case kAdditiveBlenderMode:
												tolerance = 1;
												tolerance16 = 1;
												break;

												case kTintBlenderMode:
												case kTintLightBlenderMode:
												tolerance = 2;
												tolerance16 = 1;
												break;

												default:
												tolerance = 0;
											}
											if (std::abs((int)a - (int)(simdCol >> 24)) > tolerance) {
												printInfo();
												assert(false && "a is over the tolerance");
											}
											if (std::abs((int)r - (int)((simdCol >> 16) & 0xff)) > tolerance) {
												printInfo();
												assert(false && "r is over the tolerance");
											}
											if (std::abs((int)g - (int)((simdCol >> 8) & 0xff)) > tolerance) {
												printInfo();
												assert(false && "g is over the tolerance");
											}
											if (std::abs((int)b - (int)(simdCol & 0xff)) > tolerance) {
												printInfo();
												assert(false && "b is over the tolerance");
											}
											if (std::abs((int)b16 - (int)(simd2bppCol & 0x1f)) > tolerance16) {
												printInfo();
												assert(false && "b16 is over the tolerance");
											}
											if (std::abs((int)g16 - (int)((simd2bppCol >> 5) & 0x3f)) > tolerance16) {
												printInfo();
												assert(false && "g16 is over the tolerance");
											}
											if (std::abs((int)r16 - (int)(simd2bppCol >> 11)) > tolerance16) {
												printInfo();
												assert(false && "r16 is over the tolerance");
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void Test_GfxTransparency() {
	// Test that every transparency which is a multiple of 10 is converted
	// forth and back without loosing precision
	const size_t arr_sz = 11;
	const int trans100[arr_sz] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };
	int trans255[arr_sz] = { 0 };
	int trans100_back[arr_sz] = { 0 };

	for (size_t i = 0; i < arr_sz; ++i) {
		trans255[i] = GfxDef::Trans100ToLegacyTrans255(trans100[i]);
		trans100_back[i] = GfxDef::LegacyTrans255ToTrans100(trans255[i]);
		assert(trans100[i] == trans100_back[i]);
	}
}

#define SLOW_TESTS
void Test_Gfx() {
	Test_GfxTransparency();
#if (defined(SCUMMVM_AVX2) || defined(SCUMMVM_SSE2) || defined(SCUMMVM_NEON)) && defined(SLOW_TESTS)
	Test_BlenderModes();
	// This could take a LONG time
	Test_GfxSpeed(true, kSourceAlphaBlender, kTintLightBlenderMode);
	Test_GfxSpeed(false, kSourceAlphaBlender, kTintLightBlenderMode);
#endif
}

} // namespace AGS3
