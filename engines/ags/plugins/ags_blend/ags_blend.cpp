/***********************************************************
 * AGSBlend                                                *
 *                                                         *
 * Author: Steven Poulton                                  *
 *                                                         *
 * Date: 09/01/2011                                        *
 *                                                         *
 * Description: An AGS Plugin to allow true Alpha Blending *
 *                                                         *
 ***********************************************************/

#pragma region Defines_and_Includes

#include "core/platform.h"

#define MIN_EDITOR_VERSION 1
#define MIN_ENGINE_VERSION 3

#if AGS_PLATFORM_OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#if !defined(BUILTIN_PLUGINS)
#define THIS_IS_THE_PLUGIN
#endif

#include "plugin/agsplugin.h"

#if defined(BUILTIN_PLUGINS)
namespace agsblend {
#endif

typedef unsigned char uint8;

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


#pragma endregion

#if AGS_PLATFORM_OS_WINDOWS
// The standard Windows DLL entry point

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved) {

	switch (ul_reason_for_call)   {
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#endif

//define engine

IAGSEngine *engine;


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
	Pixel32();
	~Pixel32() = default;
	int GetColorAsInt();
	int Red;
	int Green;
	int Blue;
	int Alpha;

};

Pixel32::Pixel32() {
	Red = 0;
	Blue = 0;
	Green = 0;
	Alpha = 0;
}

int Pixel32::GetColorAsInt() {

	return makeacol32(Red, Green, Blue, Alpha);

}

#pragma endregion

/// <summary>
/// Gets the alpha value at coords x,y
/// </summary>
int GetAlpha(int sprite, int x, int y) {


	BITMAP *engineSprite = engine->GetSpriteGraphic(sprite);

	unsigned char **charbuffer = engine->GetRawBitmapSurface(engineSprite);
	unsigned int **longbuffer = (unsigned int **)charbuffer;

	int alpha = geta32(longbuffer[y][x]);

	engine->ReleaseBitmapSurface(engineSprite);

	return alpha;

}

/// <summary>
/// Sets the alpha value at coords x,y
/// </summary>
int PutAlpha(int sprite, int x, int y, int alpha) {

	BITMAP *engineSprite = engine->GetSpriteGraphic(sprite);

	unsigned char **charbuffer = engine->GetRawBitmapSurface(engineSprite);
	unsigned int **longbuffer = (unsigned int **)charbuffer;


	int r = getr32(longbuffer[y][x]);
	int g = getg32(longbuffer[y][x]);
	int b = getb32(longbuffer[y][x]);
	longbuffer[y][x] = makeacol32(r, g, b, alpha);

	engine->ReleaseBitmapSurface(engineSprite);

	return alpha;

}


/// <summary>
///  Translates index from a 2D array to a 1D array
/// </summary>
int xytolocale(int x, int y, int width) {

	return (y * width + x);


}

int HighPass(int sprite, int threshold) {

	BITMAP *src = engine->GetSpriteGraphic(sprite);
	int srcWidth, srcHeight;

	engine->GetBitmapDimensions(src, &srcWidth, &srcHeight, nullptr);

	unsigned char **srccharbuffer = engine->GetRawBitmapSurface(src);
	unsigned int **srclongbuffer = (unsigned int **)srccharbuffer;

	for (int y = 0; y < srcHeight; y++) {

		for (int x = 0; x < srcWidth; x++) {

			int srcr = getb32(srclongbuffer[y][x]);
			int srcg = getg32(srclongbuffer[y][x]);
			int srcb = getr32(srclongbuffer[y][x]);
			int tempmaxim = max(srcr, srcg);
			int maxim = max(tempmaxim, srcb);
			int tempmin = min(srcr, srcg);
			int minim = min(srcb, tempmin);
			int light = (maxim + minim) / 2 ;
			if (light < threshold) srclongbuffer[y][x] = makeacol32(0, 0, 0, 0);

		}

	}

	return 0;

}


int Blur(int sprite, int radius) {

	BITMAP *src = engine->GetSpriteGraphic(sprite);

	int srcWidth, srcHeight;
	engine->GetBitmapDimensions(src, &srcWidth, &srcHeight, nullptr);

	unsigned char **srccharbuffer = engine->GetRawBitmapSurface(src);
	unsigned int **srclongbuffer = (unsigned int **)srccharbuffer;
	int negrad = -1 * radius;

	//use a 1Dimensional array since the array is on the free store, not the stack
	Pixel32 *Pixels = new Pixel32[(srcWidth + (radius * 2)) * (srcHeight + (radius * 2))];  // this defines a copy of the individual channels in class form.
	Pixel32 *Dest = new Pixel32[(srcWidth + (radius * 2)) * (srcHeight + (radius * 2))];  // this is the destination sprite. both have a border all the way round equal to the radius for the blurring.
	Pixel32 *Temp = new Pixel32[(srcWidth + (radius * 2)) * (srcHeight + (radius * 2))];


	int arraywidth = srcWidth + (radius * 2); //define the array width since its used many times in the algorithm


	for (int y = 0; y < srcHeight; y++) { //copy the sprite to the Pixels class array

		for (int x = 0; x < srcWidth; x++) {

			int locale = xytolocale(x + radius, y + radius, arraywidth);

			Pixels[locale].Red = getr32(srclongbuffer[y][x]);
			Pixels[locale].Green = getg32(srclongbuffer[y][x]);
			Pixels[locale].Blue = getb32(srclongbuffer[y][x]);
			Pixels[locale].Alpha = geta32(srclongbuffer[y][x]);



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
			int locale = xytolocale(x - 1, y + radius, arraywidth);
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
			int locale = xytolocale(x + radius, y - 1, arraywidth);
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



	for (int y = 0; y < srcHeight; y++) {

		for (int x = 0; x < srcWidth; x++) {
			int locale = xytolocale(x + radius, y + radius, arraywidth);
			srclongbuffer[y][x] = Dest[locale].GetColorAsInt(); //write the destination array to the main buffer

		}

	}
	delete [] Pixels;
	delete [] Dest;
	delete [] Temp;
	engine->ReleaseBitmapSurface(src);
	delete srclongbuffer;
	delete srccharbuffer;
	return 0;
}

int Clamp(int val, int min, int max) {

	if (val < min) return min;
	else if (val > max) return max;
	else return val;

}

int DrawSprite(int destination, int sprite, int x, int y, int DrawMode, int trans) {

	trans = 100 - trans;
	int srcWidth, srcHeight, destWidth, destHeight;

	BITMAP *src = engine->GetSpriteGraphic(sprite);
	BITMAP *dest = engine->GetSpriteGraphic(destination);

	engine->GetBitmapDimensions(src, &srcWidth, &srcHeight, nullptr);
	engine->GetBitmapDimensions(dest, &destWidth, &destHeight, nullptr);

	if (x > destWidth || y > destHeight || x + srcWidth < 0 || y + srcHeight < 0) return 1; // offscreen

	unsigned char **srccharbuffer = engine->GetRawBitmapSurface(src);
	unsigned int **srclongbuffer = (unsigned int **)srccharbuffer;

	unsigned char **destcharbuffer = engine->GetRawBitmapSurface(dest);
	unsigned int **destlongbuffer = (unsigned int **)destcharbuffer;



	if (srcWidth + x > destWidth) srcWidth = destWidth - x - 1;
	if (srcHeight + y > destHeight) srcHeight = destHeight - y - 1;

	int destx, desty;
	int srcr, srcg, srcb, srca, destr, destg, destb, desta, finalr, finalg, finalb, finala;
	unsigned int col;
	int starty = 0;
	int startx = 0;

	if (x < 0) startx = -1 * x;
	if (y < 0) starty = -1 * y;

	int ycount = 0;
	int xcount = 0;
	for (ycount = starty; ycount < srcHeight; ycount ++) {

		for (xcount = startx; xcount < srcWidth; xcount ++) {

			destx = xcount + x;
			desty = ycount + y;

			srca = (geta32(srclongbuffer[ycount][xcount]));

			if (srca != 0) {
				srca = srca * trans / 100;
				srcr =  getr32(srclongbuffer[ycount][xcount]);
				srcg =  getg32(srclongbuffer[ycount][xcount]);
				srcb =  getb32(srclongbuffer[ycount][xcount]);

				destr =  getr32(destlongbuffer[desty][destx]);
				destg =  getg32(destlongbuffer[desty][destx]);
				destb =  getb32(destlongbuffer[desty][destx]);
				desta =  geta32(destlongbuffer[desty][destx]);




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

				}

				finala = 255 - (255 - srca) * (255 - desta) / 255;
				finalr = srca * finalr / finala + desta * destr * (255 - srca) / finala / 255;
				finalg = srca * finalg / finala + desta * destg * (255 - srca) / finala / 255;
				finalb = srca * finalb / finala + desta * destb * (255 - srca) / finala / 255;
				col = makeacol32(finalr, finalg, finalb, finala);
				destlongbuffer[desty][destx] = col;

			}


		}

	}

	engine->ReleaseBitmapSurface(src);
	engine->ReleaseBitmapSurface(dest);
	engine->NotifySpriteUpdated(destination);
	return 0;

}


int DrawAdd(int destination, int sprite, int x, int y, float scale) {


	int srcWidth, srcHeight, destWidth, destHeight;

	BITMAP *src = engine->GetSpriteGraphic(sprite);
	BITMAP *dest = engine->GetSpriteGraphic(destination);

	engine->GetBitmapDimensions(src, &srcWidth, &srcHeight, nullptr);
	engine->GetBitmapDimensions(dest, &destWidth, &destHeight, nullptr);

	if (x > destWidth || y > destHeight) return 1; // offscreen

	unsigned char **srccharbuffer = engine->GetRawBitmapSurface(src);
	unsigned int **srclongbuffer = (unsigned int **)srccharbuffer;

	unsigned char **destcharbuffer = engine->GetRawBitmapSurface(dest);
	unsigned int **destlongbuffer = (unsigned int **)destcharbuffer;

	if (srcWidth + x > destWidth) srcWidth = destWidth - x - 1;
	if (srcHeight + y > destHeight) srcHeight = destHeight - y - 1;

	int destx, desty;
	int srcr, srcg, srcb, srca, destr, destg, destb, desta, finalr, finalg, finalb, finala;
	unsigned int col;
	int ycount = 0;
	int xcount = 0;

	int starty = 0;
	int startx = 0;

	if (x < 0) startx = -1 * x;
	if (y < 0) starty = -1 * y;



	for (ycount = starty; ycount < srcHeight; ycount ++) {

		for (xcount = startx; xcount < srcWidth; xcount ++) {

			destx = xcount + x;
			desty = ycount + y;

			srca = (geta32(srclongbuffer[ycount][xcount]));

			if (srca != 0) {


				srcr =  getr32(srclongbuffer[ycount][xcount]) * srca / 255 * scale;
				srcg =  getg32(srclongbuffer[ycount][xcount]) * srca / 255 * scale;
				srcb =  getb32(srclongbuffer[ycount][xcount]) * srca / 255 * scale;
				desta =  geta32(destlongbuffer[desty][destx]);

				if (desta == 0) {
					destr = 0;
					destg = 0;
					destb = 0;

				} else {
					destr =  getr32(destlongbuffer[desty][destx]);
					destg =  getg32(destlongbuffer[desty][destx]);
					destb =  getb32(destlongbuffer[desty][destx]);
				}

				finala = 255 - (255 - srca) * (255 - desta) / 255;
				finalr = Clamp(srcr + destr, 0, 255);
				finalg = Clamp(srcg + destg, 0, 255);
				finalb = Clamp(srcb + destb, 0, 255);
				col = makeacol32(finalr, finalg, finalb, finala);
				destlongbuffer[desty][destx] = col;

			}

		}

	}

	engine->ReleaseBitmapSurface(src);
	engine->ReleaseBitmapSurface(dest);
	engine->NotifySpriteUpdated(destination);
	return 0;



}



int DrawAlpha(int destination, int sprite, int x, int y, int trans) {

	trans = 100 - trans;

	int srcWidth, srcHeight, destWidth, destHeight;

	BITMAP *src = engine->GetSpriteGraphic(sprite);
	BITMAP *dest = engine->GetSpriteGraphic(destination);

	engine->GetBitmapDimensions(src, &srcWidth, &srcHeight, nullptr);
	engine->GetBitmapDimensions(dest, &destWidth, &destHeight, nullptr);

	if (x > destWidth || y > destHeight) return 1; // offscreen

	unsigned char **srccharbuffer = engine->GetRawBitmapSurface(src);
	unsigned int **srclongbuffer = (unsigned int **)srccharbuffer;

	unsigned char **destcharbuffer = engine->GetRawBitmapSurface(dest);
	unsigned int **destlongbuffer = (unsigned int **)destcharbuffer;

	if (srcWidth + x > destWidth) srcWidth = destWidth - x - 1;
	if (srcHeight + y > destHeight) srcHeight = destHeight - y - 1;

	int destx, desty;
	int srcr, srcg, srcb, srca, destr, destg, destb, desta, finalr, finalg, finalb, finala;

	int ycount = 0;
	int xcount = 0;

	int starty = 0;
	int startx = 0;

	if (x < 0) startx = -1 * x;
	if (y < 0) starty = -1 * y;


	for (ycount = starty; ycount < srcHeight; ycount ++) {

		for (xcount = startx; xcount < srcWidth; xcount ++) {

			destx = xcount + x;
			desty = ycount + y;

			srca = (geta32(srclongbuffer[ycount][xcount])) * trans / 100;

			if (srca != 0) {

				srcr =  getr32(srclongbuffer[ycount][xcount]);
				srcg =  getg32(srclongbuffer[ycount][xcount]);
				srcb =  getb32(srclongbuffer[ycount][xcount]);

				destr =  getr32(destlongbuffer[desty][destx]);
				destg =  getg32(destlongbuffer[desty][destx]);
				destb =  getb32(destlongbuffer[desty][destx]);
				desta =  geta32(destlongbuffer[desty][destx]);

				finala = 255 - (255 - srca) * (255 - desta) / 255;
				finalr = srca * srcr / finala + desta * destr * (255 - srca) / finala / 255;
				finalg = srca * srcg / finala + desta * destg * (255 - srca) / finala / 255;
				finalb = srca * srcb / finala + desta * destb * (255 - srca) / finala / 255;

				destlongbuffer[desty][destx] = makeacol32(finalr, finalg, finalb, finala);

			}

		}

	}

	engine->ReleaseBitmapSurface(src);
	engine->ReleaseBitmapSurface(dest);
	engine->NotifySpriteUpdated(destination);

	return 0;
}


#if AGS_PLATFORM_OS_WINDOWS

//==============================================================================

// ***** Design time *****

IAGSEditor *editor; // Editor interface

const char *ourScriptHeader =
    "import int DrawAlpha(int destination, int sprite, int x, int y, int transparency);\r\n"
    "import int GetAlpha(int sprite, int x, int y);\r\n"
    "import int PutAlpha(int sprite, int x, int y, int alpha);\r\n"
    "import int Blur(int sprite, int radius);\r\n"
    "import int HighPass(int sprite, int threshold);\r\n"
    "import int DrawAdd(int destination, int sprite, int x, int y, float scale);\r\n"
    "import int DrawSprite(int destination, int sprite, int x, int y, int DrawMode, int trans);";




//------------------------------------------------------------------------------

LPCSTR AGS_GetPluginName() {
	return ("AGSBlend");
}

//------------------------------------------------------------------------------

int AGS_EditorStartup(IAGSEditor *lpEditor) {
	// User has checked the plugin to use it in their game

	// If it's an earlier version than what we need, abort.
	if (lpEditor->version < MIN_EDITOR_VERSION)
		return (-1);

	editor = lpEditor;
	editor->RegisterScriptHeader(ourScriptHeader);

	// Return 0 to indicate success
	return (0);
}

//------------------------------------------------------------------------------

void AGS_EditorShutdown() {
	// User has un-checked the plugin from their game
	editor->UnregisterScriptHeader(ourScriptHeader);
}

//------------------------------------------------------------------------------

void AGS_EditorProperties(HWND parent) {                      //*** optional ***
	// User has chosen to view the Properties of the plugin
	// We could load up an options dialog or something here instead
	/*  MessageBox(parent,
	               L"AGSBlend v1.0 By Calin Leafshade",
	               L"About",
	               MB_OK | MB_ICONINFORMATION);
	         */
}

//------------------------------------------------------------------------------

int AGS_EditorSaveGame(char *buffer, int bufsize) {           //*** optional ***
	// Called by the editor when the current game is saved to disk.
	// Plugin configuration can be stored in [buffer] (max [bufsize] bytes)
	// Return the amount of bytes written in the buffer
	return (0);
}

//------------------------------------------------------------------------------

void AGS_EditorLoadGame(char *buffer, int bufsize) {          //*** optional ***
	// Called by the editor when a game is loaded from disk
	// Previous written data can be read from [buffer] (size [bufsize]).
	// Make a copy of the data, the buffer is freed after this function call.
}

//==============================================================================

#endif

// ***** Run time *****

// Engine interface

//------------------------------------------------------------------------------

#define REGISTER(x) engine->RegisterScriptFunction(#x, (void *) (x));
#define STRINGIFY(s) STRINGIFY_X(s)
#define STRINGIFY_X(s) #s

void AGS_EngineStartup(IAGSEngine *lpEngine) {
	engine = lpEngine;

	// Make sure it's got the version with the features we need
	if (engine->version < MIN_ENGINE_VERSION)
		engine->AbortGame("Plugin needs engine version " STRINGIFY(MIN_ENGINE_VERSION) " or newer.");

	//register functions

	REGISTER(GetAlpha)
	REGISTER(PutAlpha)
	REGISTER(DrawAlpha)
	REGISTER(Blur)
	REGISTER(HighPass)
	REGISTER(DrawAdd)
	REGISTER(DrawSprite)


}

//------------------------------------------------------------------------------

void AGS_EngineShutdown() {
	// Called by the game engine just before it exits.
	// This gives you a chance to free any memory and do any cleanup
	// that you need to do before the engine shuts down.
}

//------------------------------------------------------------------------------

int AGS_EngineOnEvent(int event, int data) {                  //*** optional ***
	switch (event) {
	/*
	        case AGSE_KEYPRESS:
	        case AGSE_MOUSECLICK:
	        case AGSE_POSTSCREENDRAW:
	        case AGSE_PRESCREENDRAW:
	        case AGSE_SAVEGAME:
	        case AGSE_RESTOREGAME:
	        case AGSE_PREGUIDRAW:
	        case AGSE_LEAVEROOM:
	        case AGSE_ENTERROOM:
	        case AGSE_TRANSITIONIN:
	        case AGSE_TRANSITIONOUT:
	        case AGSE_FINALSCREENDRAW:
	        case AGSE_TRANSLATETEXT:
	        case AGSE_SCRIPTDEBUG:
	        case AGSE_SPRITELOAD:
	        case AGSE_PRERENDER:
	        case AGSE_PRESAVEGAME:
	        case AGSE_POSTRESTOREGAME:
	*/
	default:
		break;
	}

	// Return 1 to stop event from processing further (when needed)
	return (0);
}

//------------------------------------------------------------------------------

int AGS_EngineDebugHook(const char *scriptName,
                        int lineNum, int reserved) {          //*** optional ***
	// Can be used to debug scripts, see documentation
	return 0;
}

//------------------------------------------------------------------------------

void AGS_EngineInitGfx(const char *driverID, void *data) {    //*** optional ***
	// This allows you to make changes to how the graphics driver starts up.
	// See documentation
}

//..............................................................................


#if defined(BUILTIN_PLUGINS)
}
#endif
