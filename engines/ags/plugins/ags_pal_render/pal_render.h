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

#ifndef AGS_PLUGINS_AGS_PAL_RENDER_PAL_RENDER_H
#define AGS_PLUGINS_AGS_PAL_RENDER_PAL_RENDER_H

#include "ags/lib/allegro.h"
#include "ags/plugins/ags_plugin.h"
#include "ags/plugins/plugin_base.h"
#include "common/algorithm.h"

namespace AGS3 {
namespace Plugins {
namespace AGSPalRender {

struct PALSTRUCT {
	byte r;
	byte b;
	byte g;
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
	static unsigned char MixColorAlpha(unsigned char fg, unsigned char bg, unsigned char alpha, int use_objpal = 0) {
		unsigned char rfg = cycle_remap[fg]; //Automatic remapping of palette slots.
		//unsigned char rbg = cycle_remap [bg]; //Saves on typing elsewhere.
		AGSColor *palette = engine->GetPalette();
		int i = 0;
		//int out_r = (palette[fg].r>>1) * alpha + (palette[bg].r>>1) * (255 - alpha);
		//int out_g = palette[fg].g * alpha + palette[bg].g * (255 - alpha);
		//int out_b = (palette[fg].b>>1) * alpha + (palette[bg].b>>1) * (255 - alpha);
		int out_r, out_g, out_b;
		if (use_objpal == 0) {
			out_r = (objectivepal[rfg].r >> 1) *alpha + (palette[bg].r >> 1) *(255 - alpha);
			out_g = objectivepal[rfg].g * alpha + palette[bg].g * (255 - alpha);
			out_b = (objectivepal[rfg].b >> 1) *alpha + (palette[bg].b >> 1) *(255 - alpha);
		} else {
			out_r = (objectivepal[rfg].r >> 1) *alpha + (objectivepal[bg].r >> 1) *(255 - alpha);
			out_g = objectivepal[rfg].g * alpha + objectivepal[bg].g * (255 - alpha);
			out_b = (objectivepal[rfg].b >> 1) *alpha + (objectivepal[bg].b >> 1) *(255 - alpha);
		}
		//char ralpha = MAX(0,MIN(63,alpha>>2));
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
		unsigned char *clutp = clut;
		//unsigned char result = cycle_remap [clut[i>>8][i%256]]; //Once again, to make sure that the palette slot used is the right one.
		return cycle_remap[*(clutp + i)]; //Once again, to make sure that the palette slot used is the right one.
		//engine->ReleaseBitmapSurface (clutspr);
	}

	static unsigned char MixColorLightLevel(unsigned char fg, unsigned char intensity) {
		unsigned char rfg = cycle_remap [fg]; //Automatic remapping of palette slots.
		int i = 0;
		//int dark_r = (((palette[fg].r>>1) * (intensity))>>8);
		//int dark_b = (((palette[fg].b>>1) * (intensity))>>8);
		//int dark_g = (((palette[fg].g)    * (intensity))>>8);
		int dark_r = (((objectivepal[rfg].r >> 1) * (intensity)) >> 8);
		int dark_b = (((objectivepal[rfg].b >> 1) * (intensity)) >> 8);
		int dark_g = (((objectivepal[rfg].g)    * (intensity)) >> 8);
		i = ((dark_r << 11) | (dark_g << 5) | dark_b);
		unsigned char *clutp = clut;
		return cycle_remap [*(clutp + i)]; //Once again, to make sure that the palette slot used is the right one.
	}

	static unsigned char MixColorAdditive(unsigned char fg, unsigned char bg, unsigned char alpha, int use_objpal = 0) {
		unsigned char rfg = cycle_remap[fg]; //Automatic remapping of palette slots.
		//unsigned char rbg = cycle_remap[bg]; //Saves on typing elsewhere.
		//BITMAP *clutspr = engine->GetSpriteGraphic (clutslot);
		//if (!clutspr) engine->AbortGame ("MixColorAlpha: Can't load CLUT sprite into memory.");
		//uint8 *clutarray = engine->GetRawBitmapSurface (clutspr);
		AGSColor *palette = engine->GetPalette();
		int i = 0;
		int add_r, add_b, add_g = 0;
		//char ralpha = MAX(0,MIN(63,alpha>>2));
		//add_r = (((palette[fg].r>>1) * (alpha))>>8);
		//add_b = (((palette[fg].b>>1) * (alpha))>>8);
		//add_g = (((palette[fg].g)    * (alpha))>>8);
		add_r = (((objectivepal[rfg].r >> 1) * (alpha)) >> 8);
		add_b = (((objectivepal[rfg].b >> 1) * (alpha)) >> 8);
		add_g = (((objectivepal[rfg].g)    * (alpha)) >> 8);
		//int a_g = MAX(0,MIN(63,alpha>>2));
		//add_r = ((alphamultiply[(palette[fg].r>>1)<<6)+ralpha])>>6);
		//add_b = ((alphamultiply[(palette[fg].b>>1)<<6)+ralpha])>>6);
		//add_g = ((alphamultiply[(palette[fg].g)   <<6)+ralpha])>>6);
		//int out_r = MIN(31,(palette[bg].r>>1) + add_r);
		//int out_g = MIN(63, palette[bg].g     + add_g);
		//int out_b = MIN(31,(palette[bg].b>>1) + add_b);
		int out_r, out_g, out_b;
		if (use_objpal == 0) {
			out_r = MIN(31, (palette[bg].r >> 1) + add_r);
			out_g = MIN(63, palette[bg].g     + add_g);
			out_b = MIN(31, (palette[bg].b >> 1) + add_b);
		} else {
			out_r = MIN(31, (objectivepal [bg].r >> 1) + add_r);
			out_g = MIN(63, objectivepal [bg].g     + add_g);
			out_b = MIN(31, (objectivepal [bg].b >> 1) + add_b);
		}
		i = ((out_r << 11) | (out_g << 5) | out_b);
		unsigned char *clutp = clut;
		unsigned char result = cycle_remap [*(clutp + i)]; //Once again, to make sure that the palette slot used is the right one.
		//unsigned char result = cycle_remap [clut[i>>8][i%256]]; //Once again, to make sure that the palette slot used is the right one.
		//engine->ReleaseBitmapSurface (clutspr);
		return result;
	}

	static unsigned char MixColorMultiply(unsigned char fg, unsigned char bg, unsigned char alpha, int use_objpal = 0) {
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
		unsigned char *clutp = clut;
		unsigned char result = cycle_remap [*(clutp + i)]; //Once again, to make sure that the palette slot used is the right one.
		//unsigned char result = cycle_remap [clut[i>>8][i%256]]; //Once again, to make sure that the palette slot used is the right one.
		//engine->ReleaseBitmapSurface (clutspr);
		return result;
	}
};

void GetColor565(ScriptMethodParams &params);

unsigned short root(unsigned short x);
float FastSin(float x);
float FastCos(float x);

} // namespace AGSPalRender
} // namespace Plugins
} // namespace AGS3

#endif
