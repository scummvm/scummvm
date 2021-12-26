/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#include "ags/plugins/ags_blend/ags_blend.h"
#include "ags/shared/core/platform.h"
#include "common/algorithm.h"

namespace AGS3 {
namespace Plugins {
namespace AGSBlend {

#pragma region Defines_and_Includes

#define MIN_EDITOR_VERSION 1
#define MIN_ENGINE_VERSION 3

#define DEFAULT_RGB_R_SHIFT_32  16
#define DEFAULT_RGB_G_SHIFT_32  8
#define DEFAULT_RGB_B_SHIFT_32  0
#define DEFAULT_RGB_A_SHIFT_32  24

#if !AGS_PLATFORM_OS_WINDOWS
#define min(x,y) (((x) < (y)) ? (x) : (y))
#define max(x,y) (((x) > (y)) ? (x) : (y))
#endif

#define abs(a)                       ((a)<0 ? -(a) : (a))
#define ChannelBlend_Normal(B,L)     ((uint8)(B))
#define ChannelBlend_Lighten(B,L)    ((uint8)((L > B) ? L:B))
#define ChannelBlend_Darken(B,L)     ((uint8)((L > B) ? B:L))
#define ChannelBlend_Multiply(B,L)   ((uint8)((B * L) / 255))
#define ChannelBlend_Average(B,L)    ((uint8)((B + L) / 2))
#define ChannelBlend_Add(B,L)        ((uint8)(min(255, (B + L))))
#define ChannelBlend_Subtract(B,L)   ((uint8)((B + L < 255) ? 0:(B + L - 255)))
#define ChannelBlend_Difference(B,L) ((uint8)(abs(B - L)))
#define ChannelBlend_Negation(B,L)   ((uint8)(255 - abs(255 - B - L)))
#define ChannelBlend_Screen(B,L)     ((uint8)(255 - (((255 - B) * (255 - L)) >> 8)))
#define ChannelBlend_Exclusion(B,L)  ((uint8)(B + L - 2 * B * L / 255))
#define ChannelBlend_Overlay(B,L)    ((uint8)((L < 128) ? (2 * B * L / 255):(255 - 2 * (255 - B) * (255 - L) / 255)))
#define ChannelBlend_SoftLight(B,L)  ((uint8)((L < 128)?(2*((B>>1)+64))*((float)L/255):(255-(2*(255-((B>>1)+64))*(float)(255-L)/255))))
#define ChannelBlend_HardLight(B,L)  (ChannelBlend_Overlay(L,B))
#define ChannelBlend_ColorDodge(B,L) ((uint8)((L == 255) ? L:min(255, ((B << 8 ) / (255 - L)))))
#define ChannelBlend_ColorBurn(B,L)  ((uint8)((L == 0) ? L:max(0, (255 - ((255 - B) << 8 ) / L))))
#define ChannelBlend_LinearDodge(B,L)(ChannelBlend_Add(B,L))
#define ChannelBlend_LinearBurn(B,L) (ChannelBlend_Subtract(B,L))
#define ChannelBlend_LinearLight(B,L)((uint8)(L < 128)?ChannelBlend_LinearBurn(B,(2 * L)):ChannelBlend_LinearDodge(B,(2 * (L - 128))))
#define ChannelBlend_VividLight(B,L) ((uint8)(L < 128)?ChannelBlend_ColorBurn(B,(2 * L)):ChannelBlend_ColorDodge(B,(2 * (L - 128))))
#define ChannelBlend_PinLight(B,L)   ((uint8)(L < 128)?ChannelBlend_Darken(B,(2 * L)):ChannelBlend_Lighten(B,(2 * (L - 128))))
#define ChannelBlend_HardMix(B,L)    ((uint8)((ChannelBlend_VividLight(B,L) < 128) ? 0:255))
#define ChannelBlend_Reflect(B,L)    ((uint8)((L == 255) ? L:min(255, (B * B / (255 - L)))))
#define ChannelBlend_Glow(B,L)       (ChannelBlend_Reflect(L,B))
#define ChannelBlend_Phoenix(B,L)    ((uint8)(min(B,L) - max(B,L) + 255))
#define ChannelBlend_Alpha(B,L,O)    ((uint8)(O * B + (1 - O) * L))
#define ChannelBlend_AlphaF(B,L,F,O) (ChannelBlend_Alpha(F(B,L),B,O))

#define STRINGIFY(s) STRINGIFY_X(s)
#define STRINGIFY_X(s) #s

#pragma endregion

const char *AGSBlend::AGS_GetPluginName() {
	return "AGSBlend";
}

void AGSBlend::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	// Make sure it's got the version with the features we need
	if (_engine->version < MIN_ENGINE_VERSION)
		_engine->AbortGame("Plugin needs engine version " STRINGIFY(MIN_ENGINE_VERSION) " or newer.");

	// Register functions
	SCRIPT_METHOD(GetAlpha, AGSBlend::GetAlpha);
	SCRIPT_METHOD(PutAlpha, AGSBlend::PutAlpha);
	SCRIPT_METHOD(DrawAlpha, AGSBlend::DrawAlpha);
	SCRIPT_METHOD(Blur, AGSBlend::Blur);
	SCRIPT_METHOD(HighPass, AGSBlend::HighPass);
	SCRIPT_METHOD(DrawAdd, AGSBlend::DrawAdd);
	SCRIPT_METHOD(DrawSprite, AGSBlend::DrawSprite);
}

//------------------------------------------------------------------------------

#pragma region Color_Functions

int getr32(int c) {
	return ((c >> DEFAULT_RGB_R_SHIFT_32) & 0xFF);
}

int getg32(int c) {
	return ((c >> DEFAULT_RGB_G_SHIFT_32) & 0xFF);
}

int getb32(int c) {
	return ((c >> DEFAULT_RGB_B_SHIFT_32) & 0xFF);
}

int geta32(int c) {
	return ((c >> DEFAULT_RGB_A_SHIFT_32) & 0xFF);
}

int makeacol32(int r, int g, int b, int a) {
	return ((r << DEFAULT_RGB_R_SHIFT_32) |
	        (g << DEFAULT_RGB_G_SHIFT_32) |
	        (b << DEFAULT_RGB_B_SHIFT_32) |
	        (a << DEFAULT_RGB_A_SHIFT_32));
}

#pragma endregion

#pragma region Pixel32_Definition

struct Pixel32 {
public:
	int Red = 0;
	int Green = 0;
	int Blue = 0;
	int Alpha = 0;

	int GetColorAsInt() const {
		return makeacol32(Red, Green, Blue, Alpha);
	}
};

#pragma endregion

void AGSBlend::GetAlpha(ScriptMethodParams &params) {
	PARAMS3(int, sprite, int, x, int, y);
	BITMAP *engineSprite = _engine->GetSpriteGraphic(sprite);

	uint8 *charbuffer = _engine->GetRawBitmapSurface(engineSprite);
	uint32 *longbuffer = (uint32 *)charbuffer;
	int pitch = _engine->GetBitmapPitch(engineSprite) / 4;

	int alpha = geta32(longbuffer[y * pitch + x]);

	_engine->ReleaseBitmapSurface(engineSprite);

	params._result = alpha;
}

void AGSBlend::PutAlpha(ScriptMethodParams &params) {
	PARAMS4(int, sprite, int, x, int, y, int, alpha);
	BITMAP *engineSprite = _engine->GetSpriteGraphic(sprite);

	uint8 *charbuffer = _engine->GetRawBitmapSurface(engineSprite);
	uint32 *longbuffer = (uint32 *)charbuffer;
	int pitch = _engine->GetBitmapPitch(engineSprite) / 4;

	int pixel = y * pitch + x;
	int r = getr32(longbuffer[pixel]);
	int g = getg32(longbuffer[pixel]);
	int b = getb32(longbuffer[pixel]);
	longbuffer[pixel] = makeacol32(r, g, b, alpha);

	_engine->ReleaseBitmapSurface(engineSprite);

	params._result = alpha;
}

int AGSBlend::xytolocale(int x, int y, int width) {
	return (y * width + x);
}

void AGSBlend::HighPass(ScriptMethodParams &params) {
	PARAMS2(int, sprite, int, threshold);
	BITMAP *src = _engine->GetSpriteGraphic(sprite);
	int32 srcWidth, srcHeight;

	_engine->GetBitmapDimensions(src, &srcWidth, &srcHeight, nullptr);

	uint8 *srccharbuffer = _engine->GetRawBitmapSurface(src);
	uint32 *srclongbuffer = (uint32 *)srccharbuffer;
	int pitch = _engine->GetBitmapPitch(src) / 4;

	for (int y = 0, yi = 0; y < srcHeight; y++, yi += pitch) {

		for (int x = 0; x < srcWidth; x++) {

			int srcr = getb32(srclongbuffer[yi + x]);
			int srcg = getg32(srclongbuffer[yi + x]);
			int srcb = getr32(srclongbuffer[yi + x]);
			int tempmaxim = max(srcr, srcg);
			int maxim = max(tempmaxim, srcb);
			int tempmin = min(srcr, srcg);
			int minim = min(srcb, tempmin);
			int light = (maxim + minim) / 2 ;
			if (light < threshold)
				srclongbuffer[yi + x] = makeacol32(0, 0, 0, 0);

		}

	}

	params._result = 0;

}

void AGSBlend::Blur(ScriptMethodParams &params) {
	PARAMS2(int, sprite, int, radius);
	BITMAP *src = _engine->GetSpriteGraphic(sprite);

	int32 srcWidth, srcHeight;
	_engine->GetBitmapDimensions(src, &srcWidth, &srcHeight, nullptr);

	uint8 *srccharbuffer = _engine->GetRawBitmapSurface(src);
	uint32 *srclongbuffer = (uint32 *)srccharbuffer;
	int pitch = _engine->GetBitmapPitch(src) / 4;
	int negrad = -1 * radius;

	//use a 1Dimensional array since the array is on the free store, not the stack
	Pixel32 *Pixels = new Pixel32[(srcWidth + (radius * 2)) * (srcHeight + (radius * 2))];  // this defines a copy of the individual channels in class form.
	Pixel32 *Dest = new Pixel32[(srcWidth + (radius * 2)) * (srcHeight + (radius * 2))];  // this is the destination sprite. both have a border all the way round equal to the radius for the blurring.
	Pixel32 *Temp = new Pixel32[(srcWidth + (radius * 2)) * (srcHeight + (radius * 2))];

	int arraywidth = srcWidth + (radius * 2); //define the array width since its used many times in the algorithm

	for (int y = 0, yi = 0; y < srcHeight; y++, yi += pitch) { //copy the sprite to the Pixels class array
		for (int x = 0; x < srcWidth; x++) {
			int locale = xytolocale(x + radius, y + radius, arraywidth);

			Pixels[locale].Red = getr32(srclongbuffer[yi + x]);
			Pixels[locale].Green = getg32(srclongbuffer[yi + x]);
			Pixels[locale].Blue = getb32(srclongbuffer[yi + x]);
			Pixels[locale].Alpha = geta32(srclongbuffer[yi + x]);
		}
	}

	int  numofpixels = (radius * 2 + 1);
	for (int y = 0; y < srcHeight; y++) {
		int totalr = 0;
		int totalg = 0;
		int totalb = 0;
		int totala = 0;

		// Process entire window for first pixel
		for (int kx = negrad; kx <= radius; kx++) {
			int locale = xytolocale(kx + radius, y + radius, arraywidth);
			totala += Pixels[locale].Alpha;
			totalr += (Pixels[locale].Red * Pixels[locale].Alpha) / 255;
			totalg += (Pixels[locale].Green * Pixels[locale].Alpha) / 255;
			totalb += (Pixels[locale].Blue * Pixels[locale].Alpha) / 255;
		}

		int locale = xytolocale(radius, y + radius, arraywidth);
		Temp[locale].Red = totalr / numofpixels; // take an average and assign it to the destination array
		Temp[locale].Green = totalg / numofpixels;
		Temp[locale].Blue = totalb / numofpixels;
		Temp[locale].Alpha = totala / numofpixels;

		// Subsequent pixels just update window total
		for (int x = 1; x < srcWidth; x++) {
			// Subtract pixel leaving window
			locale = xytolocale(x - 1, y + radius, arraywidth);
			totala -= Pixels[locale].Alpha;
			totalr -= (Pixels[locale].Red * Pixels[locale].Alpha) / 255;
			totalg -= (Pixels[locale].Green * Pixels[locale].Alpha) / 255;
			totalb -= (Pixels[locale].Blue * Pixels[locale].Alpha) / 255;

			// Add pixel entering window
			locale = xytolocale(x + radius + radius, y + radius, arraywidth);
			totala += Pixels[locale].Alpha;
			totalr += (Pixels[locale].Red * Pixels[locale].Alpha) / 255;
			totalg += (Pixels[locale].Green * Pixels[locale].Alpha) / 255;
			totalb += (Pixels[locale].Blue * Pixels[locale].Alpha) / 255;


			locale = xytolocale(x + radius, y + radius, arraywidth);
			Temp[locale].Red = totalr / numofpixels; // take an average and assign it to the destination array
			Temp[locale].Green = totalg / numofpixels;
			Temp[locale].Blue = totalb / numofpixels;
			Temp[locale].Alpha = totala / numofpixels;
		}
	}

	for (int x = 0; x < srcWidth; x++) {
		int totalr = 0;
		int totalg = 0;
		int totalb = 0;
		int totala = 0;

		// Process entire window for first pixel
		for (int ky = negrad; ky <= radius; ky++) {
			int locale = xytolocale(x + radius, ky + radius, arraywidth);
			totala += Temp[locale].Alpha;
			totalr += (Temp[locale].Red * Temp[locale].Alpha) / 255;
			totalg += (Temp[locale].Green * Temp[locale].Alpha) / 255;
			totalb += (Temp[locale].Blue * Temp[locale].Alpha) / 255;
		}

		int locale = xytolocale(x + radius, radius, arraywidth);
		Dest[locale].Red = totalr / numofpixels; // take an average and assign it to the destination array
		Dest[locale].Green = totalg / numofpixels;
		Dest[locale].Blue = totalb / numofpixels;
		Dest[locale].Alpha = totala / numofpixels;

		// Subsequent pixels just update window total
		for (int y = 1; y < srcHeight; y++) {
			// Subtract pixel leaving window
			locale = xytolocale(x + radius, y - 1, arraywidth);
			totala -= Temp[locale].Alpha;
			totalr -= (Temp[locale].Red * Temp[locale].Alpha) / 255;
			totalg -= (Temp[locale].Green * Temp[locale].Alpha) / 255;
			totalb -= (Temp[locale].Blue * Temp[locale].Alpha) / 255;


			// Add pixel entering window

			locale = xytolocale(x + radius, y + radius + radius, arraywidth);
			totala += Temp[locale].Alpha;
			totalr += (Temp[locale].Red * Temp[locale].Alpha) / 255;
			totalg += (Temp[locale].Green * Temp[locale].Alpha) / 255;
			totalb += (Temp[locale].Blue * Temp[locale].Alpha) / 255;


			locale = xytolocale(x + radius, y + radius, arraywidth);
			Dest[locale].Red = totalr / numofpixels; // take an average and assign it to the destination array
			Dest[locale].Green = totalg / numofpixels;
			Dest[locale].Blue = totalb / numofpixels;
			Dest[locale].Alpha = totala / numofpixels;
		}
	}

	for (int y = 0, yi = 0; y < srcHeight; y++, yi += pitch) {

		for (int x = 0; x < srcWidth; x++) {
			int locale = xytolocale(x + radius, y + radius, arraywidth);
			srclongbuffer[yi + x] = Dest[locale].GetColorAsInt(); //write the destination array to the main buffer
		}
	}

	delete[] Pixels;
	delete[] Dest;
	delete[] Temp;
	_engine->ReleaseBitmapSurface(src);
	delete srclongbuffer;
	delete srccharbuffer;

	params._result = 0;
}

void AGSBlend::DrawSprite(ScriptMethodParams &params) {
	PARAMS6(int, destination, int, sprite, int, x, int, y, int, DrawMode, int, trans);
	trans = 100 - trans;
	int32 srcWidth, srcHeight, destWidth, destHeight;

	BITMAP *src = _engine->GetSpriteGraphic(sprite);
	BITMAP *dest = _engine->GetSpriteGraphic(destination);

	_engine->GetBitmapDimensions(src, &srcWidth, &srcHeight, nullptr);
	_engine->GetBitmapDimensions(dest, &destWidth, &destHeight, nullptr);

	if (x > destWidth || y > destHeight || x + srcWidth < 0 || y + srcHeight < 0) {
		// offscreen
		params._result = 1;
		return;
	}

	uint8 *srccharbuffer = _engine->GetRawBitmapSurface(src);
	uint32 *srclongbuffer = (uint32 *)srccharbuffer;
	int srcPitch = _engine->GetBitmapPitch(src) / 4;

	uint8 *destcharbuffer = _engine->GetRawBitmapSurface(dest);
	uint32 *destlongbuffer = (uint32 *)destcharbuffer;
	int destPitch = _engine->GetBitmapPitch(dest) / 4;

	if (srcWidth + x > destWidth) srcWidth = destWidth - x - 1;
	if (srcHeight + y > destHeight) srcHeight = destHeight - y - 1;

	int srcr, srcg, srcb, srca, destr, destg, destb, desta;
	int finalr = 0, finalg = 0, finalb = 0, finala = 0;
	unsigned int col;
	int starty = 0;
	int startx = 0;

	if (x < 0) startx = -1 * x;
	if (y < 0) starty = -1 * y;

	int ycount = 0;
	int xcount = 0;
	int srcy = starty * srcPitch;
	int desty = (starty + y) * destPitch;
	for (ycount = starty; ycount < srcHeight; ycount ++, srcy += srcPitch, desty += destPitch) {
		for (xcount = startx; xcount < srcWidth; xcount ++) {
			int destx = xcount + x;

			srca = (geta32(srclongbuffer[srcy + xcount]));

			if (srca != 0) {
				srca = srca * trans / 100;
				srcr =  getr32(srclongbuffer[srcy + xcount]);
				srcg =  getg32(srclongbuffer[srcy + xcount]);
				srcb =  getb32(srclongbuffer[srcy + xcount]);

				destr =  getr32(destlongbuffer[desty + destx]);
				destg =  getg32(destlongbuffer[desty + destx]);
				destb =  getb32(destlongbuffer[desty + destx]);
				desta =  geta32(destlongbuffer[desty + destx]);

				switch (DrawMode) {
				case 0:
					finalr = srcr;
					finalg = srcg;
					finalb = srcb;
					break;

				case 1:
					finalr = ChannelBlend_Lighten(srcr, destr);
					finalg = ChannelBlend_Lighten(srcg, destg);
					finalb = ChannelBlend_Lighten(srcb, destb);
					break;

				case 2:
					finalr = ChannelBlend_Darken(srcr, destr);
					finalg = ChannelBlend_Darken(srcg, destg);
					finalb = ChannelBlend_Darken(srcb, destb);
					break;

				case 3:
					finalr = ChannelBlend_Multiply(srcr, destr);
					finalg = ChannelBlend_Multiply(srcg, destg);
					finalb = ChannelBlend_Multiply(srcb, destb);
					break;

				case 4:
					finalr = ChannelBlend_Add(srcr, destr);
					finalg = ChannelBlend_Add(srcg, destg);
					finalb = ChannelBlend_Add(srcb, destb);
					break;

				case 5:
					finalr = ChannelBlend_Subtract(srcr, destr);
					finalg = ChannelBlend_Subtract(srcg, destg);
					finalb = ChannelBlend_Subtract(srcb, destb);
					break;

				case 6:
					finalr = ChannelBlend_Difference(srcr, destr);
					finalg = ChannelBlend_Difference(srcg, destg);
					finalb = ChannelBlend_Difference(srcb, destb);
					break;

				case 7:
					finalr = ChannelBlend_Negation(srcr, destr);
					finalg = ChannelBlend_Negation(srcg, destg);
					finalb = ChannelBlend_Negation(srcb, destb);
					break;

				case 8:
					finalr = ChannelBlend_Screen(srcr, destr);
					finalg = ChannelBlend_Screen(srcg, destg);
					finalb = ChannelBlend_Screen(srcb, destb);
					break;

				case 9:
					finalr = ChannelBlend_Exclusion(srcr, destr);
					finalg = ChannelBlend_Exclusion(srcg, destg);
					finalb = ChannelBlend_Exclusion(srcb, destb);
					break;

				case 10:
					finalr = ChannelBlend_Overlay(srcr, destr);
					finalg = ChannelBlend_Overlay(srcg, destg);
					finalb = ChannelBlend_Overlay(srcb, destb);
					break;

				case 11:
					finalr = ChannelBlend_SoftLight(srcr, destr);
					finalg = ChannelBlend_SoftLight(srcg, destg);
					finalb = ChannelBlend_SoftLight(srcb, destb);
					break;

				case 12:
					finalr = ChannelBlend_HardLight(srcr, destr);
					finalg = ChannelBlend_HardLight(srcg, destg);
					finalb = ChannelBlend_HardLight(srcb, destb);
					break;

				case 13:
					finalr = ChannelBlend_ColorDodge(srcr, destr);
					finalg = ChannelBlend_ColorDodge(srcg, destg);
					finalb = ChannelBlend_ColorDodge(srcb, destb);
					break;

				case 14:
					finalr = ChannelBlend_ColorBurn(srcr, destr);
					finalg = ChannelBlend_ColorBurn(srcg, destg);
					finalb = ChannelBlend_ColorBurn(srcb, destb);
					break;

				case 15:
					finalr = ChannelBlend_LinearDodge(srcr, destr);
					finalg = ChannelBlend_LinearDodge(srcg, destg);
					finalb = ChannelBlend_LinearDodge(srcb, destb);
					break;

				case 16:
					finalr = ChannelBlend_LinearBurn(srcr, destr);
					finalg = ChannelBlend_LinearBurn(srcg, destg);
					finalb = ChannelBlend_LinearBurn(srcb, destb);
					break;

				case 17:
					finalr = ChannelBlend_LinearLight(srcr, destr);
					finalg = ChannelBlend_LinearLight(srcg, destg);
					finalb = ChannelBlend_LinearLight(srcb, destb);
					break;

				case 18:
					finalr = ChannelBlend_VividLight(srcr, destr);
					finalg = ChannelBlend_VividLight(srcg, destg);
					finalb = ChannelBlend_VividLight(srcb, destb);
					break;

				case 19:
					finalr = ChannelBlend_PinLight(srcr, destr);
					finalg = ChannelBlend_PinLight(srcg, destg);
					finalb = ChannelBlend_PinLight(srcb, destb);
					break;

				case 20:
					finalr = ChannelBlend_HardMix(srcr, destr);
					finalg = ChannelBlend_HardMix(srcg, destg);
					finalb = ChannelBlend_HardMix(srcb, destb);
					break;

				case 21:
					finalr = ChannelBlend_Reflect(srcr, destr);
					finalg = ChannelBlend_Reflect(srcg, destg);
					finalb = ChannelBlend_Reflect(srcb, destb);
					break;

				case 22:
					finalr = ChannelBlend_Glow(srcr, destr);
					finalg = ChannelBlend_Glow(srcg, destg);
					finalb = ChannelBlend_Glow(srcb, destb);
					break;

				case 23:
					finalr = ChannelBlend_Phoenix(srcr, destr);
					finalg = ChannelBlend_Phoenix(srcg, destg);
					finalb = ChannelBlend_Phoenix(srcb, destb);
					break;

				default:
					break;
				}

				finala = 255 - (255 - srca) * (255 - desta) / 255;
				finalr = srca * finalr / finala + desta * destr * (255 - srca) / finala / 255;
				finalg = srca * finalg / finala + desta * destg * (255 - srca) / finala / 255;
				finalb = srca * finalb / finala + desta * destb * (255 - srca) / finala / 255;
				col = makeacol32(finalr, finalg, finalb, finala);
				destlongbuffer[desty + destx] = col;

			}


		}

	}

	_engine->ReleaseBitmapSurface(src);
	_engine->ReleaseBitmapSurface(dest);
	_engine->NotifySpriteUpdated(destination);
	params._result = 0;

}

void AGSBlend::DrawAdd(ScriptMethodParams &params) {
	PARAMS5(int, destination, int, sprite, int, x, int, y, float, scale);
	int32 srcWidth, srcHeight, destWidth, destHeight;

	BITMAP *src = _engine->GetSpriteGraphic(sprite);
	BITMAP *dest = _engine->GetSpriteGraphic(destination);

	_engine->GetBitmapDimensions(src, &srcWidth, &srcHeight, nullptr);
	_engine->GetBitmapDimensions(dest, &destWidth, &destHeight, nullptr);

	if (x > destWidth || y > destHeight) {
		// offscreen
		params._result = 1;
		return;
	}

	uint8 *srccharbuffer = _engine->GetRawBitmapSurface(src);
	uint32 *srclongbuffer = (uint32 *)srccharbuffer;
	int srcPitch = _engine->GetBitmapPitch(src) / 4;

	uint8 *destcharbuffer = _engine->GetRawBitmapSurface(dest);
	uint32 *destlongbuffer = (uint32 *)destcharbuffer;
	int destPitch = _engine->GetBitmapPitch(dest) / 4;

	if (srcWidth + x > destWidth) srcWidth = destWidth - x - 1;
	if (srcHeight + y > destHeight) srcHeight = destHeight - y - 1;

	int srcr, srcg, srcb, srca, destr, destg, destb, desta, finalr, finalg, finalb, finala;
	unsigned int col;
	int ycount = 0;
	int xcount = 0;

	int starty = 0;
	int startx = 0;

	if (x < 0) startx = -1 * x;
	if (y < 0) starty = -1 * y;

	int srcy = starty * srcPitch;
	int desty = (starty + y) * destPitch;
	for (ycount = starty; ycount < srcHeight; ycount ++, srcy += srcPitch, desty += destPitch) {
		for (xcount = startx; xcount < srcWidth; xcount ++) {
			int destx = xcount + x;

			srca = (geta32(srclongbuffer[srcy + xcount]));

			if (srca != 0) {
				srcr =  getr32(srclongbuffer[srcy + xcount]) * srca / 255 * scale;
				srcg =  getg32(srclongbuffer[srcy + xcount]) * srca / 255 * scale;
				srcb =  getb32(srclongbuffer[srcy + xcount]) * srca / 255 * scale;
				desta =  geta32(destlongbuffer[desty + destx]);

				if (desta == 0) {
					destr = 0;
					destg = 0;
					destb = 0;

				} else {
					destr =  getr32(destlongbuffer[desty + destx]);
					destg =  getg32(destlongbuffer[desty + destx]);
					destb =  getb32(destlongbuffer[desty + destx]);
				}

				finala = 255 - (255 - srca) * (255 - desta) / 255;
				finalr = CLIP(srcr + destr, 0, 255);
				finalg = CLIP(srcg + destg, 0, 255);
				finalb = CLIP(srcb + destb, 0, 255);
				col = makeacol32(finalr, finalg, finalb, finala);
				destlongbuffer[desty + destx] = col;
			}
		}
	}

	_engine->ReleaseBitmapSurface(src);
	_engine->ReleaseBitmapSurface(dest);
	_engine->NotifySpriteUpdated(destination);

	params._result = 0;
}

void AGSBlend::DrawAlpha(ScriptMethodParams &params) {
	PARAMS5(int, destination, int, sprite, int, x, int, y, int, trans);
	trans = 100 - trans;

	int32 srcWidth, srcHeight, destWidth, destHeight;

	BITMAP *src = _engine->GetSpriteGraphic(sprite);
	BITMAP *dest = _engine->GetSpriteGraphic(destination);

	_engine->GetBitmapDimensions(src, &srcWidth, &srcHeight, nullptr);
	_engine->GetBitmapDimensions(dest, &destWidth, &destHeight, nullptr);

	if (x > destWidth || y > destHeight) {
		// offscreen
		params._result = 1;
		return;
	}

	uint8 *srccharbuffer = _engine->GetRawBitmapSurface(src);
	uint32 *srclongbuffer = (uint32 *)srccharbuffer;
	int srcPitch = _engine->GetBitmapPitch(src) / 4;

	uint8 *destcharbuffer = _engine->GetRawBitmapSurface(dest);
	uint32 *destlongbuffer = (uint32 *)destcharbuffer;
	int destPitch = _engine->GetBitmapPitch(dest) / 4;

	if (srcWidth + x > destWidth) srcWidth = destWidth - x - 1;
	if (srcHeight + y > destHeight) srcHeight = destHeight - y - 1;

	int srcr, srcg, srcb, srca, destr, destg, destb, desta, finalr, finalg, finalb, finala;

	int ycount = 0;
	int xcount = 0;

	int starty = 0;
	int startx = 0;

	if (x < 0) startx = -1 * x;
	if (y < 0) starty = -1 * y;

	int srcy = starty * srcPitch;
	int desty = (starty + y) * destPitch;
	for (ycount = starty; ycount < srcHeight; ycount ++, srcy += srcPitch, desty += destPitch) {
		for (xcount = startx; xcount < srcWidth; xcount ++) {
			int destx = xcount + x;

			srca = (geta32(srclongbuffer[srcy + xcount])) * trans / 100;

			if (srca != 0) {
				srcr =  getr32(srclongbuffer[srcy + xcount]);
				srcg =  getg32(srclongbuffer[srcy + xcount]);
				srcb =  getb32(srclongbuffer[srcy + xcount]);

				destr =  getr32(destlongbuffer[desty + destx]);
				destg =  getg32(destlongbuffer[desty + destx]);
				destb =  getb32(destlongbuffer[desty + destx]);
				desta =  geta32(destlongbuffer[desty + destx]);

				finala = 255 - (255 - srca) * (255 - desta) / 255;
				finalr = srca * srcr / finala + desta * destr * (255 - srca) / finala / 255;
				finalg = srca * srcg / finala + desta * destg * (255 - srca) / finala / 255;
				finalb = srca * srcb / finala + desta * destb * (255 - srca) / finala / 255;

				destlongbuffer[desty + destx] = makeacol32(finalr, finalg, finalb, finala);
			}
		}
	}

	_engine->ReleaseBitmapSurface(src);
	_engine->ReleaseBitmapSurface(dest);
	_engine->NotifySpriteUpdated(destination);

	params._result = 0;
}

} // namespace AGSBlend
} // namespace Plugins
} // namespace AGS3
