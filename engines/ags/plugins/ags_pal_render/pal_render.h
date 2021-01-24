#ifndef __PALGORITHMS_PALRENDER_H
#define __PALGORITHMS_PALRENDER_H

#include "agsplugin.h"
#include <stdint.h> // for int32_t types, __int32 is just Windows
#include <algorithm>  // for std::max and std::min
#include <cstring>  // for memcpy

#undef min
#undef max

#ifndef _WIN32
#define __forceinline __attribute__((always_inline))

#define SCRIPT_FLOAT(x) int32_t __script_float##x
#define INIT_SCRIPT_FLOAT(x) float x; std::memcpy(&x, &__script_float##x, sizeof(float))
#define FLOAT_RETURN_TYPE int32_t
#define RETURN_FLOAT(x) int32_t __ret##x; std::memcpy(&__ret##x, &x, sizeof(float)); return __ret##x
#else
#define SCRIPT_FLOAT(x) __int32 __script_float##x
#define INIT_SCRIPT_FLOAT(x) float x; memcpy(&x, &__script_float##x, sizeof(float))
#define FLOAT_RETURN_TYPE __int32
#define RETURN_FLOAT(x) __int32 __ret##x; memcpy(&__ret##x, &x, sizeof(float)); return __ret##x
#endif

#if defined(BUILTIN_PLUGINS)
namespace agspalrender {
#endif

struct PALSTRUCT {
	int r;
	int b;
	int g;
};

extern IAGSEngine *engine;
extern unsigned char clut[65536];
extern unsigned char cycle_remap [256];
extern const int alphamultiply [4096];
extern PALSTRUCT objectivepal[256];

// this class exists solely to take advantage of g++'s
// -fvisibility-inlines-hidden option, so that these
// methods can be inlined without any trace or complaint
class Mix {
public:

//unsigned char MixColorAlpha (unsigned char fg,unsigned char bg,unsigned char alpha);
//unsigned char MixColorAdditive (unsigned char fg,unsigned char bg,unsigned char alpha);
	__forceinline static unsigned char MixColorAlpha(unsigned char fg, unsigned char bg, unsigned char alpha, int use_objpal = 0) {
		unsigned char rfg = cycle_remap [fg]; //Automatic remapping of palette slots.
		//unsigned char rbg = cycle_remap [bg]; //Saves on typing elsewhere.
		AGSColor *palette = engine->GetPalette();
		int i = 0;
		//int out_r = (palette[fg].r>>1) * alpha + (palette[bg].r>>1) * (255 - alpha);
		//int out_g = palette[fg].g * alpha + palette[bg].g * (255 - alpha);
		//int out_b = (palette[fg].b>>1) * alpha + (palette[bg].b>>1) * (255 - alpha);
		int out_r, out_g, out_b;
		if (use_objpal == 0) {
			out_r = (objectivepal[rfg].r >> 1) * alpha + (palette[bg].r >> 1) * (255 - alpha);
			out_g = objectivepal[rfg].g * alpha + palette[bg].g * (255 - alpha);
			out_b = (objectivepal[rfg].b >> 1) * alpha + (palette[bg].b >> 1) * (255 - alpha);
		} else {
			out_r = (objectivepal[rfg].r >> 1) * alpha + (objectivepal[bg].r >> 1) * (255 - alpha);
			out_g = objectivepal[rfg].g * alpha + objectivepal[bg].g * (255 - alpha);
			out_b = (objectivepal[rfg].b >> 1) * alpha + (objectivepal[bg].b >> 1) * (255 - alpha);
		}
		//char ralpha = std::max(0,std::min(63,alpha>>2));
		//unsigned char invralpha = 64-ralpha;
		//if (ralpha > alpha) engine->AbortGame ("wtf");
		//int out_r = alphamultiply[((palette[fg].r>>1)<<6) +ralpha] + alphamultiply[((palette[bg].r>>1)<<6) +(invralpha)];
		//int out_g = alphamultiply[((palette[fg].g)   <<6) +ralpha] + alphamultiply[((palette[bg].g)   <<6) +(invralpha)];
		//int out_b = alphamultiply[((palette[fg].b>>1)<<6) +ralpha] + alphamultiply[((palette[bg].b>>1)<<6) +(invralpha)];
		out_r = (out_r + 1 + (out_r >> 8)) >> 8;
		out_g = (out_g + 1 + (out_g >> 8)) >> 8;
		out_b = (out_b + 1 + (out_b >> 8)) >> 8;
		//out_r = (out_r + 1 + (out_r >> 6)) >> 6;
		//out_g = (out_g + 1 + (out_g >> 6)) >> 6;
		//out_b = (out_b + 1 + (out_b >> 6)) >> 6;
		i = ((out_r << 11) | (out_g << 5) | out_b);
		unsigned char (*clutp) = clut;
		//unsigned char result = cycle_remap [clut[i>>8][i%256]]; //Once again, to make sure that the palette slot used is the right one.
		return cycle_remap[*(clutp + i)]; //Once again, to make sure that the palette slot used is the right one.
		//engine->ReleaseBitmapSurface (clutspr);
	}

	__forceinline static unsigned char MixColorLightLevel(unsigned char fg, unsigned char intensity) {
		unsigned char rfg = cycle_remap [fg]; //Automatic remapping of palette slots.
		int i = 0;
		//int dark_r = (((palette[fg].r>>1) * (intensity))>>8);
		//int dark_b = (((palette[fg].b>>1) * (intensity))>>8);
		//int dark_g = (((palette[fg].g)    * (intensity))>>8);
		int dark_r = (((objectivepal[rfg].r >> 1) * (intensity)) >> 8);
		int dark_b = (((objectivepal[rfg].b >> 1) * (intensity)) >> 8);
		int dark_g = (((objectivepal[rfg].g)    * (intensity)) >> 8);
		i = ((dark_r << 11) | (dark_g << 5) | dark_b);
		unsigned char (*clutp) = clut;
		return cycle_remap [*(clutp + i)]; //Once again, to make sure that the palette slot used is the right one.
	}

	__forceinline static unsigned char MixColorAdditive(unsigned char fg, unsigned char bg, unsigned char alpha, int use_objpal = 0) {
		unsigned char rfg = cycle_remap [fg]; //Automatic remapping of palette slots.
		unsigned char rbg = cycle_remap [bg]; //Saves on typing elsewhere.
		//BITMAP *clutspr = engine->GetSpriteGraphic (clutslot);
		//if (!clutspr) engine->AbortGame ("MixColorAlpha: Can't load CLUT sprite into memory.");
		//unsigned char **clutarray = engine->GetRawBitmapSurface (clutspr);
		AGSColor *palette = engine->GetPalette();
		int i = 0;
		int add_r, add_b, add_g = 0;
		//char ralpha = std::max(0,std::min(63,alpha>>2));
		//add_r = (((palette[fg].r>>1) * (alpha))>>8);
		//add_b = (((palette[fg].b>>1) * (alpha))>>8);
		//add_g = (((palette[fg].g)    * (alpha))>>8);
		add_r = (((objectivepal[rfg].r >> 1) * (alpha)) >> 8);
		add_b = (((objectivepal[rfg].b >> 1) * (alpha)) >> 8);
		add_g = (((objectivepal[rfg].g)    * (alpha)) >> 8);
		//int a_g = std::max(0,std::min(63,alpha>>2));
		//add_r = ((alphamultiply[(palette[fg].r>>1)<<6)+ralpha])>>6);
		//add_b = ((alphamultiply[(palette[fg].b>>1)<<6)+ralpha])>>6);
		//add_g = ((alphamultiply[(palette[fg].g)   <<6)+ralpha])>>6);
		//int out_r = std::min(31,(palette[bg].r>>1) + add_r);
		//int out_g = std::min(63, palette[bg].g     + add_g);
		//int out_b = std::min(31,(palette[bg].b>>1) + add_b);
		int out_r, out_g, out_b;
		if (use_objpal == 0) {
			out_r = std::min(31, (palette[bg].r >> 1) + add_r);
			out_g = std::min(63, palette[bg].g     + add_g);
			out_b = std::min(31, (palette[bg].b >> 1) + add_b);
		} else {
			out_r = std::min(31, (objectivepal [bg].r >> 1) + add_r);
			out_g = std::min(63, objectivepal [bg].g     + add_g);
			out_b = std::min(31, (objectivepal [bg].b >> 1) + add_b);
		}
		i = ((out_r << 11) | (out_g << 5) | out_b);
		unsigned char (*clutp) = clut;
		unsigned char result = cycle_remap [*(clutp + i)]; //Once again, to make sure that the palette slot used is the right one.
		//unsigned char result = cycle_remap [clut[i>>8][i%256]]; //Once again, to make sure that the palette slot used is the right one.
		//engine->ReleaseBitmapSurface (clutspr);
		return result;
	}

	__forceinline static unsigned char MixColorMultiply(unsigned char fg, unsigned char bg, unsigned char alpha, int use_objpal = 0) {
		unsigned char rfg = cycle_remap [fg]; //Automatic remapping of palette slots.
		unsigned char rbg = cycle_remap [bg]; //Saves on typing elsewhere.
		AGSColor *palette = engine->GetPalette();
		int i = 0;
		int mul_r, mul_b, mul_g = 0;
		int out_r, out_g, out_b = 0;
		if (use_objpal == 0) {
			mul_r = ((objectivepal[rfg].r >> 1) * (palette[rbg].r >> 1)) / 64;
			mul_b = ((objectivepal[rfg].b >> 1) * (palette[rbg].b >> 1)) / 64;
			mul_g = ((objectivepal[rfg].g * palette[rbg].g) / 64);
			out_r = ((palette[rbg].r >> 1) * (63 - (alpha / 4)) + (mul_r * (alpha / 4))) / 63;
			out_g = (palette[rbg].g * (63 - (alpha / 4)) + (mul_g * (alpha / 4))) / 63;
			out_b = ((palette[rbg].b >> 1) * (63 - (alpha / 4)) + (mul_b * (alpha / 4))) / 63;
		} else {
			mul_r = ((objectivepal[rfg].r >> 1) * (objectivepal[rbg].r >> 1)) / 64;
			mul_b = ((objectivepal[rfg].b >> 1) * (objectivepal[rbg].b >> 1)) / 64;
			mul_g = ((objectivepal[rfg].g * objectivepal[rbg].g) / 64);

			out_r = ((objectivepal[rbg].r >> 1) * (63 - (alpha / 4)) + (mul_r * (alpha / 4))) / 63;
			out_g = (objectivepal[rbg].g * (63 - (alpha / 4)) + (mul_g * (alpha / 4))) / 63;
			out_b = ((objectivepal[rbg].b >> 1) * (63 - (alpha / 4)) + (mul_b * (alpha / 4))) / 63;
		}
		i = ((out_r << 11) | (out_g << 5) | out_b);
		unsigned char (*clutp) = clut;
		unsigned char result = cycle_remap [*(clutp + i)]; //Once again, to make sure that the palette slot used is the right one.
		//unsigned char result = cycle_remap [clut[i>>8][i%256]]; //Once again, to make sure that the palette slot used is the right one.
		//engine->ReleaseBitmapSurface (clutspr);
		return result;
	}

};

unsigned char GetColor565(unsigned char r, unsigned char g, unsigned char b);

unsigned short root(unsigned short x);
float FastSin(float x);
float FastCos(float x);

#if defined(BUILTIN_PLUGINS)
} // namespace agspalrender
#endif

#endif