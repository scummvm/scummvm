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

#include "ags/lib/allegro.h"
#include "ags/plugins/ags_pal_render/ags_pal_render.h"
#include "ags/plugins/ags_pal_render/pal_render.h"
#include "ags/plugins/ags_pal_render/raycast.h"
#include "ags/ags.h"

namespace AGS3 {
namespace Plugins {
namespace AGSPalRender {

#define MAX_OVERLAYS 128
#define MAX_STARS 1024
#define MAX_DEPTH 64

#define PI         (3.1415926535f)
#define HALF_PI    (0.5f * PI)
#define TWO_PI     (2.0f * PI)
#define TWO_PI_INV (1.0f / TWO_PI)

const float halfpi = (0.5f * PI);
const float twopi  = (2.0f * PI);
const float twopi_inv = (1.0f / TWO_PI);
const float pisquared = PI * PI;
const float picubed = PI * PI * PI;

IAGSEngine *engine;
//unsigned char clut[256][256];
unsigned char clut[65536];

struct transoverlaytype {
	int sprite;
	int spritemask;
	int blendtype;
	int x;
	int y;
	int trans;
	int level;
	bool enabled;
} overlay[MAX_OVERLAYS];

int clutslot;
int drawreflections;
byte cycle_remap[256];

struct starstype {
	float x;
	float y;
	float z;
	unsigned char color;
	long sprite;
	int maxrad;
	int scaleboost;
};
starstype *stars;

struct starsoptions {
	float speed;
	int maxstars;
	int depthmultiplier;
	int originx;
	int originy;
	int overscan;
} Starfield;
long *reflectionmap;
BITMAP *rcolormap;
BITMAP *ralphamap;

struct charrefopt {
	int8 reflect;
	int replaceview;
};

struct objrefopt {
	int8 reflect;
	int8 ignorescaling;
};

struct reflectionopt {
	charrefopt *Characters;
	objrefopt *Objects;
	int blendslot;
	int blendamount;
} Reflection;

int dummy;

#define LENS_WIDTH 150

struct LensDistort {
	int xoffset;
	int yoffset;
};

LensDistort *lens;
struct LensOpt {
	bool draw;
	int lenswidth;
	int lenszoom;
	int level;
	int x;
	int y;
	int clampoffset;
} LensOption;

const int alphamultiply [4096] = {
//#include "alphamultiply.txt"
};
float rot_sine_LUT[360];
float rot_cos_LUT[360];

BITMAP *backgroundimage;

PALSTRUCT objectivepal[256];
int bgimgspr;

void AGSPalRender::WriteObjectivePalette(ScriptMethodParams &params) {
	PARAMS4(unsigned char, index, unsigned char, r, unsigned char, b, unsigned char, g);
	objectivepal[index].r = r;
	objectivepal[index].b = b;
	objectivepal[index].g = g;
}

void AGSPalRender::ReadObjectivePaletteR(ScriptMethodParams &params) {
	PARAMS1(unsigned char, index);
	params._result = (int)objectivepal[index].r;
}

void AGSPalRender::ReadObjectivePaletteB(ScriptMethodParams &params) {
	PARAMS1(unsigned char, index);
	params._result = (int)objectivepal[index].b;
}
void AGSPalRender::ReadObjectivePaletteG(ScriptMethodParams &params) {
	PARAMS1(unsigned char, index);
	params._result = (int)objectivepal[index].g;
}


#define SQRT_MAGIC_F 0x5f3759df
float  q3sqrt(const float x) {
	const float xhalf = 0.5f * x;

	union { // get bits for floating value
		float x;
		int i;
	} u;
	u.x = x;
	u.i = SQRT_MAGIC_F - (u.i >> 1);  // gives initial guess y0
	return x * u.x * (1.5f - xhalf * u.x * u.x); // Newton step, repeating increases accuracy
}

void Make_Sin_Lut() {
	for (int angle = 0; angle < 360; angle++) {
		double rad = (angle * PI) / 180.0;
		rot_sine_LUT [angle] = static_cast<float>(sin(rad));
		rot_cos_LUT [angle]  = static_cast<float>(cos(rad));
	}
}

/*
void PreMultiply_Alphas () //Ha ha, this isn't the kind of premultiplcation you're thinking of.
{
	for (int y=0;y<64;y++)
		for (int x=0;x<64;x++)
			alphamultiply [y*64+x] = y*x;
}
*/

void AGSPalRender::GetModifiedBackgroundImage(ScriptMethodParams &params) {
	params._result = bgimgspr;
}

unsigned short root(unsigned short x) {
	unsigned short a, b;
	b = x;
	a = x = 0x3f;
	x = b / x;
	a = x = (x + a) >> 1;
	x = b / x;
	a = x = (x + a) >> 1;
	x = b / x;
	x = (x + a) >> 1;
	return (x);
}


float Hill(float x) {
	const float a0 = 1.0f;
	const float a2 = 2.0f / PI - 12.0f / (pisquared);
	const float a3 = 16.0f / (picubed) - 4.0f / (pisquared);
	const float xx = x * x;
	const float xxx = xx * x;

	return a0 + a2 * xx + a3 * xxx;
}

float FastSin(float x) {
	// wrap x within [0, TWO_PI)
	const float a = x * twopi_inv;
	x -= static_cast<int>(a) * twopi;
	if (x < 0.0f)
		x += twopi;

	// 4 pieces of hills
	if (x < halfpi)
		return Hill(halfpi - x);
	else if (x < PI)
		return Hill(x - halfpi);
	else if (x < 3.0f * halfpi)
		return -Hill(3.0f * halfpi - x);
	else
		return -Hill(x - 3.0f * halfpi);
}

float FastCos(float x) {
	return FastSin(x + halfpi);
}

void AGSPalRender::AGSFastRoot(ScriptMethodParams &params) {
	PARAMS1(unsigned short, x);
	x = root(x);
	params._result = (int)x;
}

void AGSPalRender::AGSFastSin(ScriptMethodParams &params) {
	PARAMS1(int32, xi);
	float x = PARAM_TO_FLOAT(xi);
	x = FastSin(x);
	params._result = PARAM_FROM_FLOAT(x);
}

void AGSPalRender::AGSFastCos(ScriptMethodParams &params) {
	PARAMS1(int32, xi);
	float x = PARAM_TO_FLOAT(xi);
	x = FastSin(x + halfpi);
	params._result = PARAM_FROM_FLOAT(x);
}


void DrawLens(int ox, int oy) {
	int32 sh, sw = 0;
	engine->GetScreenDimensions(&sw, &sh, nullptr);
	BITMAP *virtsc = engine->GetVirtualScreen();
	if (!virtsc) engine->AbortGame("DrawLens: Cannot get virtual screen.");
	BITMAP *lenswrite = engine->CreateBlankBitmap(LensOption.lenswidth, LensOption.lenswidth, 8);
	uint8 *vScreen = engine->GetRawBitmapSurface(virtsc);
	uint8 *lensarray = engine->GetRawBitmapSurface(lenswrite);
	int virtscPitch = engine->GetBitmapPitch(virtsc);
	int lenswritePitch = engine->GetBitmapPitch(lenswrite);
	int radius = LensOption.lenswidth >> 1;
	for (int y = 0, lensy = 0; y < LensOption.lenswidth; y++, lensy += lenswritePitch) {
		int ypos = y * LensOption.lenswidth;
		for (int x = 0; x < LensOption.lenswidth; x++) {
			int lenspos = ypos + x;
			int coffx = lens[lenspos].xoffset;
			int coffy = lens[lenspos].yoffset;
			if (oy + coffy > 0 && oy + coffy < sh && ox + coffx > 0 && ox + coffx < sw) {
				lensarray[lensy + x] = vScreen[(oy + coffy) * virtscPitch + ox + coffx];
				//vScreen[(oy + coffy) * virtscPitch + ox + coffx] = ABS(coffy);
			}
		}
	}
	/*
	for (int y=0, lensy = 0;y<LensOption.lenswidth;y++, lensy += lenswritePitch)
	{
	    int ypos = y*LensOption.lenswidth;
	    for (int x=0;x<LensOption.lenswidth;x++)
	    {
	        if (oy+y > 0 && oy+y < sh && ox+x > 0 && ox+x < sw)
	        {
	            vScreen[(oy+y) * virtscPitch + ox+x] = lensarray[lensy + x];
	        }
	    }
	}
	*/
	int radsq = radius * radius;
	for (int cy = -radius, lensy = 0; cy <= radius; cy++, lensy += lenswritePitch) { //Draw a circle around the point, for the mask.
		int cysq = cy * cy;
		for (int cx = -radius; cx <= radius; cx++) {
			int cxsq = cx * cx;
			int dx = cx + ox;
			int dy = cy + oy;
			if ((cxsq + cysq <= radsq) && dx < sw && dx >= 0 && dy < sh && dy >= 0 && cy + radius < LensOption.lenswidth - 1 && cx + radius < LensOption.lenswidth - 1) {
				//if (cy+radius < 0 || cx+radius < 0) engine->AbortGame ("I did something wrong");
				vScreen[dy * virtscPitch + dx] = lensarray[lensy + cx + radius];
			}
		}
	}

	engine->ReleaseBitmapSurface(lenswrite);
	engine->ReleaseBitmapSurface(virtsc);
	engine->FreeBitmap(lenswrite);
}

void AGSPalRender::SetLensPos(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	LensOption.x = x;
	LensOption.y = y;
}

void AGSPalRender::GetLensX(ScriptMethodParams &params) {
	params._result = LensOption.x;
}

void AGSPalRender::GetLensY(ScriptMethodParams &params) {
	params._result =  LensOption.y;
}

void AGSPalRender::SetLensDrawn(ScriptMethodParams &params) {
	PARAMS1(int, toggle);
	if (toggle > 0) LensOption.draw = 1;
	else LensOption.draw = 0;
}

void AGSPalRender::GetLensDrawn(ScriptMethodParams &params) {
	params._result = LensOption.draw;
}

void AGSPalRender::SetLensOffsetClamp(ScriptMethodParams &params) {
	PARAMS1(int, clamp);
	if (clamp < 0) LensOption.clampoffset = LensOption.lenswidth;
	else LensOption.clampoffset = clamp;
}

void AGSPalRender::GetLensOffsetClamp(ScriptMethodParams &params) {
	params._result = LensOption.clampoffset;
}

void AGSPalRender::GetLensLevel(ScriptMethodParams &params) {
	params._result = LensOption.level;
}

void AGSPalRender::SetLensLevel(ScriptMethodParams &params) {
	PARAMS1(int, level);
	if (level < 0 || level > 4) engine->AbortGame("SetLensLevel: Invalid level.");
	else LensOption.level = level;
}

void AGSPalRender::LensInitialize(ScriptMethodParams &params) {
	PARAMS5(int, width, int, zoom, int, lensx, int, lensy, int, level);
	int clamp = -1;
	if (params.size() > 5)
		clamp = (int)params[5];

	int32 sw, sh, radius;
	if (width < 1) engine->AbortGame("Invalid lens dimension!");
	radius = width >> 1;
	lens = new LensDistort [width * width]();
	engine->GetScreenDimensions(&sw, &sh, nullptr);
	int radsq = radius * radius;
	int zoomsq = zoom * zoom;
	for (int y = 0; y < radius; y++) {
		int ysq = y * y;
		for (int x = 0; x < radius; x++) {
			int lx, ly;
			int xsq = x * x;
			if ((xsq + ysq) < (radsq)) {
				float shift = zoom / sqrt((float)(zoomsq - (xsq + ysq - radsq)));
				lx = (int)(x * shift - x);
				ly = (int)(y * shift - y);
			} else {
				lx = 0;
				ly = 0;
			}
			lens[(radius - y)*width + (radius - x)].xoffset =  lx;
			lens[(radius - y)*width + (radius - x)].yoffset =  ly;
			lens[(radius + y)*width + (radius + x)].xoffset = -lx;
			lens[(radius + y)*width + (radius + x)].yoffset = -ly;
			lens[(radius + y)*width + (radius - x)].xoffset =  lx;
			lens[(radius + y)*width + (radius - x)].yoffset = -ly;
			lens[(radius - y)*width + (radius + x)].xoffset = -lx;
			lens[(radius - y)*width + (radius + x)].yoffset =  ly;
		}
	}
	LensOption.lenswidth = width;
	LensOption.lenszoom = zoom;
	if (clamp < 0) LensOption.clampoffset = width;
	else LensOption.clampoffset = clamp;
	LensOption.x = lensx;
	LensOption.y = lensy;
	if (level < 0 || level > 4) engine->AbortGame("SetLensLevel: Invalid level.");
	else LensOption.level = level;
}

void AGSPalRender::ResetRemapping(ScriptMethodParams &) {
	for (int j = 0; j < 256; ++j) {
		cycle_remap [j] = j;
	}
}

#define MAX_PLASMA_COMPLEXITY 4
int plasmatype[MAX_PLASMA_COMPLEXITY];
int plasmadata [MAX_PLASMA_COMPLEXITY];
int plasmadata2 [MAX_PLASMA_COMPLEXITY];
int plasmadata3 [MAX_PLASMA_COMPLEXITY];
int plasmaroottype;


void AGSPalRender::SetPlasmaRootType(ScriptMethodParams &params) {
	PARAMS1(int, real);
	if (real) plasmaroottype = 1;
	else plasmaroottype = 0;
}

void AGSPalRender::GetPlasmaRootType(ScriptMethodParams &params) {
	params._result = plasmaroottype;
}

void AGSPalRender::SetPlasmaType(ScriptMethodParams &params) {
	PARAMS5(int, component, int, type, int, data, int, data2, int, data3);
	if (component >= MAX_PLASMA_COMPLEXITY) engine->AbortGame("Plasma too complex!");
	else {
		plasmatype [component] = type;
		plasmadata [component] = data;
		plasmadata2[component] = data2;
		plasmadata3[component] = data3;
	}

	//0 = None.
	//1 = Horizontal Bars (data=width)
	//2 = Vertical Bars (data=width)
	//3 = Circle (data=x,data2=y,data3=width)
	//4 = Diagonal Bars (data=width)
}

void AGSPalRender::ResetPlasmaSettings(ScriptMethodParams &) {
	int i = 0;
	while (i < MAX_PLASMA_COMPLEXITY) {
		plasmatype [i] = 0;
		plasmadata [i] = 0;
		plasmadata2[i] = 0;
		plasmadata3[i] = 0;
		i++;
	}
}

void AGSPalRender::DrawPlasma(ScriptMethodParams &params) {
	PARAMS3(int, slot, int, palstart, int, palend);
	BITMAP *plasmaspr = engine->GetSpriteGraphic(slot);
	if (!plasmaspr) engine->AbortGame("Plasma: Not a sprite I can load.");
	int32 w, h, basecol, range = 0;
	if (palend > palstart) {
		range = palend - palstart;
		basecol = palstart;
	} else {
		range = palstart - palend;
		basecol = palend;
	}
	engine->GetBitmapDimensions(plasmaspr, &w, &h, nullptr);
	uint8 *plasmarray = engine->GetRawBitmapSurface(plasmaspr);
	int plasmapitch = engine->GetBitmapPitch(plasmaspr);
	double frange = range / 2.0;
	int complex = 0;
	int color = 0;
	int i = 0;
	while (i < MAX_PLASMA_COMPLEXITY) {
		if (plasmatype[i] > 0) complex++;
		i++;
	}
	for (int x = 0; x < w; x++) {
		for (int y = 0, plasmay = 0; y < h; y++, plasmay += plasmapitch) {
			color = 0;
			for (int p = 0; p < MAX_PLASMA_COMPLEXITY; p++) {
				if (plasmatype[p] == 1) { //1 = Horizontal Bars (data=width)
					color += int(frange + (frange * FastSin(y / (float)plasmadata[p])));
				} else if (plasmatype[p] == 2) { //2 = Vertical Bars (data=width)
					color += int(frange + (frange * FastSin(x / (float)plasmadata[p])));
				} else if (plasmatype[p] == 3) { //3 = Circle (data=x,data2=y,data3=width)
					int cx, cy = 0;
					cx = plasmadata [p];
					cy = plasmadata2 [p];
					if (plasmaroottype == 1) color += int(frange + (frange * FastSin(q3sqrt((float)((x - cx) * (x - cx) + (y - cy) * (y - cy)) / plasmadata3[p]))));
					else color += int(frange + (frange * FastSin(root(((x - cx) * (x - cx) + (y - cy) * (y - cy)) / plasmadata3[p]))));
				} else if (plasmatype[p] == 4) { //4 = Diagonal Bars (data=width)
					color += int(frange + (frange * FastSin((x + y) / (float)plasmadata[p])));
				}
			}
			if (color > 0 && complex > 0) color = color / complex;
			plasmarray[plasmay + x] = static_cast<unsigned char>(basecol + color);
		}
	}
	engine->ReleaseBitmapSurface(plasmaspr);
	engine->NotifySpriteUpdated(slot);
}

void AGSPalRender::DoFire(ScriptMethodParams &params) {
	PARAMS8(int, spriteId, int, masksprite, int, palstart, int, palend, int, strength, int, seed, int, cutoff, int, windspeed);
	BITMAP *firespr = engine->GetSpriteGraphic(masksprite);
	BITMAP *firecolorspr = engine->GetSpriteGraphic(spriteId);
	BITMAP *seedspr;
	int32 w, h = 0;
	int range, basecol, dir = 0;
	if (palend > palstart) {
		range = palend - palstart;
		basecol = palstart;
		dir = 1;
	} else {
		range = palstart - palend;
		basecol = palend;
		dir = -1;
	}
	int divider = 256 / range;
	engine->GetBitmapDimensions(firespr, &w, &h, nullptr);
	uint8 *fire = engine->GetRawBitmapSurface(firespr);
	uint8 *color = engine->GetRawBitmapSurface(firecolorspr);
	int firePitch = engine->GetBitmapPitch(firespr);
	int colorPitch = engine->GetBitmapPitch(firecolorspr);
	int sparky = 0;
	//srand(time(NULL));
	for (int y = 0, firey = 0; y < h - 1; y++, firey += firePitch) {
		if ((int)::AGS::g_vm->getRandomNumber(9) > 7 - windspeed) { //Wind right
			for (int x = w - 1; x > 1; x--) {
				fire[firey + x] = fire[firey + x - 1];
			}
		} else if ((int)::AGS::g_vm->getRandomNumber(9) > 7 + windspeed) { // wind left
			for (int x = 0; x < w - 1; x++) {
				fire[firey + x] = fire[firey + x + 1];
			}
		}
	}
	for (int x = 0; x < w; x++) {
		sparky = ABS((int)::AGS::g_vm->getRandomNumber(0x7fffffff) % (h - 2));
		int firexy = (h - sparky) * firePitch + x;
		if (sparky < h && sparky > 0 && fire[firexy] > cutoff &&
				ABS((int)::AGS::g_vm->getRandomNumber(0x7fffffff) % 10) > 7)
			fire[firexy] = 255;
		sparky = ABS((int)::AGS::g_vm->getRandomNumber(0x7fffffff) % (h - 2));
		firexy = (h - sparky) * firePitch + x;
		if (sparky < h && sparky > 0 && fire[firexy] > cutoff &&
				ABS((int)::AGS::g_vm->getRandomNumber(0x7fffffff) % 10) > 7)
			fire[firexy] = 0;
	}
	if (seed == 0) {
		int firey = (h - 1) * firePitch;
		for (int x = 0; x < w; x++)
			fire[firey + x] = 255;
		firey = (h - 2) * firePitch;
		for (int x = 0; x < w; x++)
			fire[firey + x] = ::AGS::g_vm->getRandomNumber(255);
	} else if (seed > 0) {
		seedspr = engine->GetSpriteGraphic(seed);
		BITMAP *virtsc = engine->GetVirtualScreen();
		engine->SetVirtualScreen(firespr);
		engine->BlitBitmap(0, 0, seedspr, 1);
		engine->SetVirtualScreen(virtsc);
		engine->ReleaseBitmapSurface(virtsc);
		engine->ReleaseBitmapSurface(seedspr);
		engine->NotifySpriteUpdated(spriteId);
		engine->NotifySpriteUpdated(masksprite);
	}

	for (int y = 0, firey = 0, colory = 0; y < h - 1; y++, firey += firePitch, colory += colorPitch) {
		for (int x = 0; x < w; x++) {
			fire[firey + x] =
			    ((fire[((y + 1) % h) * firePitch + ((x - 1 + w) % w)]
			      + fire[((y + 1) % h) * firePitch + ((x) % w)]
			      + fire[((y + 1) % h) * firePitch + ((x + 1) % w)]
			      + fire[((y + 2) % h) * firePitch + ((x) % w)])
			     * 100) / (400 + (100 - strength));
			if (fire[firey + x] < cutoff) fire[firey + x] = 0;
			//if (fire[firey + x] ==255) color [colory + x] = palend;
			else color [colory + x] = static_cast<uint8>(basecol + (fire[firey + x] / divider) * dir);
		}
	}
	engine->ReleaseBitmapSurface(firespr);
	engine->ReleaseBitmapSurface(firecolorspr);
	engine->NotifySpriteUpdated(spriteId);
	engine->NotifySpriteUpdated(masksprite);
}

/*
unsigned char MixColorAlpha (unsigned char fg,unsigned char bg,unsigned char alpha)
{
	//unsigned char rfg = cycle_remap [fg]; //Automatic remapping of palette slots.
	//unsigned char rbg = cycle_remap [bg]; //Saves on typing elsewhere.
	//BITMAP *clutspr = engine->GetSpriteGraphic (clutslot);
	//if (!clutspr) engine->AbortGame ("MixColorAlpha: Can't load CLUT sprite into memory.");
	//uint8 *clutarray = engine->GetRawBitmapSurface (clutspr);
	AGSColor *palette = engine->GetPalette ();
	int i=0;
	int out_r = (palette[fg].r>>1) * alpha + (palette[bg].r>>1) * (255 - alpha);
	int out_g = palette[fg].g * alpha + palette[bg].g * (255 - alpha);
	int out_b = (palette[fg].b>>1) * alpha + (palette[bg].b>>1) * (255 - alpha);
	//unsigned char ralpha = alpha>>2;
	//unsigned char invralpha = 64-ralpha;
	//if (ralpha > alpha) engine->AbortGame ("wtf");
	//int out_r = alphamultiply[(palette[fg].r>>1)][ralpha] + alphamultiply[(palette[bg].r>>1)][(invralpha)];
	//int out_g = alphamultiply[(palette[fg].g)][ralpha] + alphamultiply[(palette[bg].g)][(invralpha)];
	//int out_b = alphamultiply[(palette[fg].b>>1)][ralpha] + alphamultiply[(palette[bg].b>>1)][(invralpha)];
	out_r = (out_r + 1 + (out_r >> 8)) >> 8;
	out_g = (out_g + 1 + (out_g >> 8)) >> 8;
	out_b = (out_b + 1 + (out_b >> 8)) >> 8;
	i = ((out_r << 11) | (out_g << 5) | out_b);
	unsigned char (*clutp) = clut;
	//unsigned char result = cycle_remap [clut[i>>8][i%256]]; //Once again, to make sure that the palette slot used is the right one.
	unsigned char result = cycle_remap [*(clutp+i)]; //Once again, to make sure that the palette slot used is the right one.
	//engine->ReleaseBitmapSurface (clutspr);
	return result;
}

unsigned char MixColorAdditive (unsigned char fg,unsigned char bg,unsigned char alpha)
{
	//unsigned char rfg = cycle_remap [fg]; //Automatic remapping of palette slots.
	//unsigned char rbg = cycle_remap [bg]; //Saves on typing elsewhere.
	//BITMAP *clutspr = engine->GetSpriteGraphic (clutslot);
	//if (!clutspr) engine->AbortGame ("MixColorAlpha: Can't load CLUT sprite into memory.");
	//uint8 *clutarray = engine->GetRawBitmapSurface (clutspr);
	AGSColor *palette = engine->GetPalette ();
	int i=0;
	int add_r,add_b,add_g = 0;
	char ralpha = alpha>>2;
	//if (ralpha > alpha) engine->AbortGame ("wtf");
	//add_r = (((palette[fg].r>>1) * (alpha))>>8);
	//add_b = (((palette[fg].b>>1) * (alpha))>>8);
	//add_g = (((palette[fg].g)    * (alpha))>>8);
	add_r = ((alphamultiply[(palette[fg].r>>1)*64+ralpha])>>6);
	add_b = ((alphamultiply[(palette[fg].b>>1)*64+ralpha])>>6);
	add_g = ((alphamultiply[(palette[fg].g   )*64+ralpha])>>6);
	int out_r = min(31,(palette[bg].r>>1) + add_r);
	int out_g = min(63, palette[bg].g     + add_g);
	int out_b = min(31,(palette[bg].b>>1) + add_b);
	i = ((out_r << 11) | (out_g << 5) | out_b);
	unsigned char (*clutp) = clut;
	unsigned char result = cycle_remap [*(clutp+i)]; //Once again, to make sure that the palette slot used is the right one.
	//unsigned char result = cycle_remap [clut[i>>8][i%256]]; //Once again, to make sure that the palette slot used is the right one.
	//engine->ReleaseBitmapSurface (clutspr);
	return result;
}
*/
void AGSPalRender::GetColor565(ScriptMethodParams &params) {
	PARAMS3(unsigned char, r, unsigned char, g, unsigned char, b);
	//BITMAP *clutspr = engine->GetSpriteGraphic (clutslot);
	//if (!clutspr) engine->AbortGame ("MixColorAlpha: Can't load CLUT sprite into memory.");
	//uint8 *clutarray = engine->GetRawBitmapSurface (clutspr);
	int i = ((r << 11) | (g << 5) | b);
	unsigned char *clutp = clut;
	unsigned char result = *(clutp + i);
	result = cycle_remap [result]; //Once again, to make sure that the palette slot used is the right one.
	//engine->ReleaseBitmapSurface (clutspr);
	params._result = (int)result;
}

void AGSPalRender::CycleRemap(ScriptMethodParams &params) {
	PARAMS2(int, start, int, end);
	if (end > start) {
		// Rotate left
		int wraparound = cycle_remap [start];
		for (; start < end; ++start) {
			cycle_remap [start] = cycle_remap [start + 1];
		}
		cycle_remap [end] = wraparound;
	} else if (end < start) {
		// Rotate right
		int wraparound = cycle_remap [start];
		for (; start > end; --start) {
			cycle_remap [start] = cycle_remap [start - 1];
		}
		cycle_remap [end] = wraparound;

	}
}

void AGSPalRender::GetRemappedSlot(ScriptMethodParams &params) {
	PARAMS1(unsigned char, slot);
	params._result = cycle_remap [slot];
}

void AGSPalRender::LoadCLUT(ScriptMethodParams &params) {
	PARAMS1(int, slot);
	if (engine->GetSpriteWidth(slot) != 256 || engine->GetSpriteHeight(slot) != 256) {
		params._result = 1;
		return;
	};
	BITMAP *clutimage = engine->GetSpriteGraphic(slot);
	uint8 *clutarray = engine->GetRawBitmapSurface(clutimage);
	int pitch = engine->GetBitmapPitch(clutimage);
	for (int y = 0, arrayy = 0, cluty = 0; y < 256; y++, arrayy += pitch, cluty += 256) {
		for (int x = 0; x < 256; x++) {
			clut[cluty + x] = clutarray[arrayy + x];
		}
	}
	clutslot = slot;
	engine->ReleaseBitmapSurface(clutimage);
	params._result = 0;
}

void AGSPalRender::SetReflections(ScriptMethodParams &params) {
	PARAMS1(int, toggle);
	drawreflections = toggle;
}

void AGSPalRender::IsReflectionsOn(ScriptMethodParams &params) {
	params._result = drawreflections;
}

void AGSPalRender::GetLuminosityFromPalette(ScriptMethodParams &params) {
	PARAMS1(int, slot);
	AGSColor *pal = engine->GetPalette();
	int lum = (pal[slot].r +
	           pal[slot].r +
	           pal[slot].r +
	           pal[slot].g +
	           pal[slot].g +
	           pal[slot].g +
	           pal[slot].g +
	           pal[slot].b) >> 3;
	params._result = lum;
}



void AGSPalRender::SetStarsOriginPoint(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	Starfield.originx = x;
	Starfield.originy = y;
}
void AGSPalRender::InitializeStars(ScriptMethodParams &params) {
	PARAMS2(int, slot, int, maxstars);
	int32 sw, sh = 0;
	BITMAP *canvas = engine->GetSpriteGraphic(slot);
	engine->GetBitmapDimensions(canvas, &sw, &sh, nullptr);
	Starfield.maxstars = maxstars;
	Starfield.overscan = 20;
	stars = new starstype [Starfield.maxstars];
	for (int i = 0; i < Starfield.maxstars; i++) {
		stars[i].x = (float)((::AGS::g_vm->getRandomNumber(0x7fffffff) % sw) << 1) - sw;
		if (stars[i].x < 1.0 && stars[i].x > -1.0) stars[i].x = (float)sw;
		stars[i].y = (float)((::AGS::g_vm->getRandomNumber(0x7fffffff) % sh) << 1) - sh;
		if (stars[i].y < 1.0 && stars[i].y > -1.0) stars[i].y = (float)sh;
		stars[i].z = (float)(MAX_DEPTH);
		stars[i].color = (::AGS::g_vm->getRandomNumber(0x7fffffff) % 240);
		stars[i].sprite = 0;
		stars[i].maxrad = (::AGS::g_vm->getRandomNumber(0x7fffffff) % 5);
	}
}

void AGSPalRender::IterateStars(ScriptMethodParams &params) {
	PARAMS1(int, slot);
	long sw, sh = 0;
	sw = engine->GetSpriteWidth(slot);
	sh = engine->GetSpriteHeight(slot);
	for (int i = 0; i < Starfield.maxstars; i++) {
		stars[i].z -= Starfield.speed;
		//if (stars[i].z < 1.0) stars[i].z = (double)MAX_DEPTH;
		float k = Starfield.depthmultiplier / stars[i].z;
		int px = static_cast<int>(stars[i].x * k + Starfield.originx);
		int py = static_cast<int>(stars[i].y * k + Starfield.originy);
		if (px >= sw + Starfield.overscan || px < 0 - Starfield.overscan || py >= sh + Starfield.overscan || py < 0 - Starfield.overscan) {
			stars[i].x = (float)((::AGS::g_vm->getRandomNumber(0x7fffffff) % sw) << 1) - sw;
			if (stars[i].x < 1.0 && stars[i].x > -1.0) stars[i].x = (float)sw;
			stars[i].y = (float)((::AGS::g_vm->getRandomNumber(0x7fffffff) % sh) << 1) - sh;
			if (stars[i].y < 1.0 && stars[i].y > -1.0) stars[i].y = (float)sh;
			stars[i].z = (float)MAX_DEPTH;
			//stars[i].color = (rand () %240);
		}
	}
}
void AGSPalRender::GetStarfieldOverscan(ScriptMethodParams &params) {
	params._result = Starfield.overscan;
}
void AGSPalRender::SetStarfieldOverscan(ScriptMethodParams &params) {
	PARAMS1(int, overscan);
	Starfield.overscan = overscan;
}

void AGSPalRender::GetStarfieldOriginX(ScriptMethodParams &params) {
	params._result = Starfield.originx;
}

void AGSPalRender::GetStarfieldOriginY(ScriptMethodParams &params) {
	params._result = Starfield.originy;
}

void AGSPalRender::SetStarfieldDepthMultiplier(ScriptMethodParams &params) {
	PARAMS1(int, multi);
	Starfield.depthmultiplier = multi;
}

void AGSPalRender::GetStarfieldDepthMultiplier(ScriptMethodParams &params) {
	params._result = Starfield.depthmultiplier;
}

void AGSPalRender::GetStarfieldMaxStars(ScriptMethodParams &params) {
	params._result = Starfield.maxstars;
}

void AGSPalRender::SetStarSpriteScaleBoost(ScriptMethodParams &params) {
	PARAMS2(int, star, int, boost);
	stars[star].scaleboost = boost;
}

void AGSPalRender::GetStarSpriteScaleBoost(ScriptMethodParams &params) {
	PARAMS1(int, star);
	params._result = stars[star].scaleboost;
}

void AGSPalRender::SetStarMaxRadius(ScriptMethodParams &params) {
	PARAMS2(int, star, int, radius);
	stars[star].maxrad = radius;
}

void AGSPalRender::GetStarMaxRadius(ScriptMethodParams &params) {
	PARAMS1(int, star);
	params._result = stars[star].maxrad;
}

void AGSPalRender::RotateStar(ScriptMethodParams &params) {
	PARAMS4(int, star, int, angle, int, px, int, py);
	float rsin = rot_sine_LUT[angle];
	float rcos = rot_cos_LUT[angle];
	float fPx = (float)px;
	float fPy = (float)py;
	float x1 = 0, y1 = 0, xRot = 0, yRot = 0;
	int i = star;
	x1 = stars[i].x;
	y1 = stars[i].y;
	xRot = fPx + rcos * (x1 - fPx) - rsin * (y1 - fPy);
	yRot = fPy + rsin * (x1 - fPx) + rcos * (y1 - fPy);
	stars[i].x = xRot;
	stars[i].y = yRot;
	i++;
}

void AGSPalRender::GetStarX(ScriptMethodParams &params) {
	PARAMS1(int, i);
	float starx = (float)stars[i].x;
	params._result = PARAM_FROM_FLOAT(starx);
}

void AGSPalRender::GetStarY(ScriptMethodParams &params) {
	PARAMS1(int, i);
	float stary = (float)stars[i].y;
	params._result = PARAM_FROM_FLOAT(stary);
}

void AGSPalRender::GetStarZ(ScriptMethodParams &params) {
	PARAMS1(int, i);
	float starz = (float)stars[i].z;
	params._result = PARAM_FROM_FLOAT(starz);
}

void AGSPalRender::SetStarPosition(ScriptMethodParams &params) {
	PARAMS4(int, star, int32, xi, int32, yi, int32, zi);
	float x = PARAM_TO_FLOAT(xi);
	float y = PARAM_TO_FLOAT(yi);
	float z = PARAM_TO_FLOAT(zi);
	stars[star].x = x;
	stars[star].y = y;
	stars[star].z = z;
}

void AGSPalRender::SetStarColor(ScriptMethodParams &params) {
	PARAMS2(int, star, unsigned char, color);
	stars[star].color = color;
}

void AGSPalRender::GetStarColor(ScriptMethodParams &params) {
	PARAMS1(int, star);
	params._result = (int)stars[star].color;
}

void AGSPalRender::SetStarSprite(ScriptMethodParams &params) {
	PARAMS2(int, star, int, slot);
	stars[star].sprite = slot;
}

void AGSPalRender::GetStarSprite(ScriptMethodParams &params) {
	PARAMS1(int, star);
	params._result = stars[star].sprite;
}

void AGSPalRender::SetStarSpriteRange(ScriptMethodParams &params) {
	PARAMS3(int, start, int, end, int, slot);
	int sfix = start;
	int efix = end;
	if (start > Starfield.maxstars) sfix = Starfield.maxstars - 1;
	if (end > Starfield.maxstars) efix = Starfield.maxstars;
	for (int i = sfix; i < efix; i++)
		stars[i].sprite = slot;
}

void AGSPalRender::DrawStars(ScriptMethodParams &params) {
	PARAMS2(int, slot, int, maskslot);
	int32 sw, sh = 0;
	BITMAP *canvas = engine->GetSpriteGraphic(slot);
	if (!canvas) engine->AbortGame("DrawStars: Can't load sprite slot.");
	BITMAP *maskcanvas = engine->GetSpriteGraphic(maskslot);
	if (!maskcanvas) engine->AbortGame("DrawStars: Can't load mask slot.");
	engine->GetBitmapDimensions(canvas, &sw, &sh, nullptr);
	uint8 *screenarray = engine->GetRawBitmapSurface(canvas);
	uint8 *maskarray = engine->GetRawBitmapSurface(maskcanvas);
	int screenPitch = engine->GetBitmapPitch(canvas);
	int maskPitch = engine->GetBitmapPitch(maskcanvas);
	for (int i = 0; i < Starfield.maxstars; i++) {
		//stars[i].z-= 0.5;
		//if (stars[i].z < 1.0) stars[i].z = (double)MAX_DEPTH;
		float k = (float)Starfield.depthmultiplier / stars[i].z;
		int px = static_cast<int>(stars[i].x * k + Starfield.originx);
		int py = static_cast<int>(stars[i].y * k + Starfield.originy);
		if (px >= sw + Starfield.overscan || px < 0 - Starfield.overscan || py >= sh + Starfield.overscan || py < 0 - Starfield.overscan) {
			stars[i].x = (float)((::AGS::g_vm->getRandomNumber(0x7fffffff) % sw) << 1) - sw;
			if (stars[i].x < 1.0 && stars[i].x > -1.0) stars[i].x = (float)sw;
			stars[i].y = (float)((::AGS::g_vm->getRandomNumber(0x7fffffff) % sh) << 1) - sh;
			if (stars[i].y < 1.0 && stars[i].y > -1.0) stars[i].y = (float)sh;
			stars[i].z = (float)MAX_DEPTH;
			//stars[i].color = (rand () %240);
		} else if (stars[i].z > 0) {
			int ivalue = (63 - (int)stars[i].z);
			if (ivalue > 63) ivalue = 63;
			else if (ivalue < 0) ivalue = 0;
			unsigned char value = (unsigned char)(ivalue);
			unsigned char maskcolor = value << 2;
			if (stars[i].sprite > 0) {
				BITMAP *origspr = engine->GetSpriteGraphic(stars[i].sprite);
				int scale = (ivalue + 1) * 100 >> 6;
				//int scale = 50;
				if (scale < 1) scale = 1;
				/*
				if (scale != 100)
				{
				uint8 * orig = engine->GetRawBitmapSurface (origspr);
				int origPitch = engine->GetBitmapPitch(origspr);
				int32 h1,h2,w1,w2=0;
				double fw2,fh2;
				engine->GetBitmapDimensions (origspr,&w1,&h1,NULL);
				fh2 = h1 * (scale / 100.0);
				fw2 = w1 * (scale / 100.0);
				h2 = static_cast<int>(fh2);
				w2 = static_cast<int>(fw2);
				if (w2 < 1) w2 = 1;
				if (h2 < 1) h2 = 1;
				resizspr = engine->CreateBlankBitmap (w2,h2,8);
				uint8 * resized = engine->GetRawBitmapSurface (resizspr);
				int resizePitch = engine->GetBitmapPitch(resizspr);
				int x_ratio = (int)((w1<<16)/w2) +1;
				int y_ratio = (int)((h1<<16)/h2) +1;
				int x2, y2 ;
				for (int i=0;i<h2;i++)
				{
				    for (int j=0;j<w2;j++)
				   {
				       x2 = ((j*x_ratio)>>16) ;
				       y2 = ((i*y_ratio)>>16) ;
				       resized [i * resizePitch + j] = orig [y2 * origPitch + x2];
				 }
				}
				engine->ReleaseBitmapSurface (resizspr);
				}
				//resizspr = origspr;
				int32 w,h=0;
				engine->GetBitmapDimensions (resizspr,&w,&h,NULL);
				uint8 *imagemap = engine->GetRawBitmapSurface (resizspr);
				int imagePitch = engine->GetBitmapPitch(resizspr);
				int ox = px - (w>>1);
				int oy = py - (h>>1);
				for (int dy=0;dy<h;dy++)
				{
				for (int dx=0;dx<w;dx++)
				{
				   int ex = ox+dx;
				   int ey = oy+dy;
				   if (ex < sw && ex >= 0 && ey < sh && ey >= 0)
				   {
				       if (maskcolor > maskarray [ey * maskPitch + ex] && imagemap[dy * imagePitch + dx] > 0)
				       {
				           maskarray [ey * maskPitch + ex] = maskcolor;
				           screenarray [ey * screenPitch + ex] = imagemap[dy][dx];
				       }
				   }
				}
				}
				*/

				uint8 *orig = engine->GetRawBitmapSurface(origspr);
				int origPitch = engine->GetBitmapPitch(origspr);
				int32 h1, h2, w1, w2 = 0;
				double fw2, fh2;
				engine->GetBitmapDimensions(origspr, &w1, &h1, nullptr);
				fh2 = h1 * (scale / 100.0);
				fw2 = w1 * (scale / 100.0);
				h2 = static_cast<int>(fh2);
				w2 = static_cast<int>(fw2);
				if (w2 < 1) w2 = 1;
				if (h2 < 1) h2 = 1;
				int x_ratio = (int)((w1 << 16) / w2) + 1;
				int y_ratio = (int)((h1 << 16) / h2) + 1;
				int x2, y2 ;
				int ox = px - (w2 >> 1);
				int oy = py - (h2 >> 1);
				for (int ii = 0; ii < h2; ii++) {
					int temprzy = ii * y_ratio;
					int ey = oy + ii;
					for (int j = 0; j < w2; j++) {
						x2 = ((j * x_ratio) >> 16);
						y2 = ((temprzy) >> 16);
						int ex = ox + j;
						if (ex < sw && ex >= 0 && ey < sh && ey >= 0) {
							if (maskcolor > maskarray [ey * maskPitch + ex] && orig[y2 * origPitch + x2] > 0) {
								maskarray [ey * maskPitch + ex] = maskcolor;
								screenarray [ey * screenPitch + ex] = orig[y2 * origPitch + x2];
							}
						}
						//resized [ii][j] = orig [y2 * origPitch + x2];
					}
				}
				engine->ReleaseBitmapSurface(origspr);
			} else if (stars[i].sprite == 0) {
				if (stars[i].maxrad == 1) {
					if (px < sw && px >= 0 && py < sh && py >= 0) {
						if (maskcolor > maskarray[py * maskPitch + px]) {
							maskarray[py * maskPitch + px] = maskcolor;
							screenarray[py * screenPitch + px] = stars[i].color;
						}
					}
				} else {
					int scale = ((((int)stars[i].z) * 100) / 63);
					if (scale < 1) scale = 1;
					int radius = (stars[i].maxrad * (100 - scale) / 100);
					int radsq = radius * radius;
					//unsigned char color = GetColor565 (value>>1,value,value>>1);
					unsigned char color = stars[i].color;
					for (int cy = -radius; cy <= radius; cy++) { //Draw a circle around the point, for the mask.
						int cysq = cy * cy;
						for (int cx = -radius; cx <= radius; cx++) {
							int cxsq = cx * cx;
							int dx = cx + px;
							int dy = cy + py;
							if ((cxsq + cysq <= radsq) && dx < sw && dx >= 0 && dy < sh && dy >= 0) {
								if (maskcolor > maskarray [dy * maskPitch + dx]) {
									maskarray [dy * maskPitch + dx] = maskcolor;
									screenarray [dy * screenPitch + dx] = color;
								}
							}
						}
					}
					/*
					for(int cy=-radius; cy<=radius; cy++) //Draw a circle around the point, for the color.
					{
					   int cysq = cy*cy;
					for(int cx=-radius; cx<=radius; cx++)
					   {
					       int cxsq = cx*cx;
					       int dx = cx+px;
					       int dy = cy+py;
					       if((cxsq+cysq <= radsq) && dx < sw && dx >= 0 && dy < sh && dy >= 0)
					       {
					           if (maskarray [dy * maskPitch + dx] == maskcolor)screenarray [dy * screenPitch + dx] = color;
					       }
					   }
					}
					*/
				}
			}
		}
	}
	engine->ReleaseBitmapSurface(canvas);
	engine->ReleaseBitmapSurface(maskcanvas);
	engine->NotifySpriteUpdated(slot);
	engine->NotifySpriteUpdated(maskslot);
}


void AGSPalRender::CreateTranslucentOverlay(ScriptMethodParams &params) {
	PARAMS6(int, id, int, spriteId, int, alpha, int, level, int, ox, int, oy);
	int mask = 0, blendmode = 0;
	if (params.size() > 6)
		mask = params[6];
	if (params.size() > 7)
		blendmode = params[7];
	BITMAP *testspr = engine->GetSpriteGraphic(spriteId);
	if (testspr) overlay[id].sprite = spriteId;
	else engine->AbortGame("CreateTranslucentOverlay: Invalid spriteId.");
	engine->ReleaseBitmapSurface(testspr);
	overlay[id].level = MAX(0, MIN(level, 4));
	overlay[id].trans = MAX(0, MIN(alpha, 255));
	overlay[id].spritemask = mask;
	overlay[id].x = ox;
	overlay[id].y = oy;
	overlay[id].enabled = true;
	overlay[id].blendtype = blendmode;
	params._result = 0;
}

void AGSPalRender::DeleteTranslucentOverlay(ScriptMethodParams &params) {
	PARAMS1(int, id);
	overlay[id].enabled = false;
	overlay[id].sprite = 0;
	overlay[id].x = 0;
	overlay[id].y = 0;
	overlay[id].level = 0;
	overlay[id].trans = 0;
	params._result = 0;
}

void AGSPalRender::MoveTranslucentOverlay(ScriptMethodParams &params) {
	PARAMS3(int, id, int, ox, int, oy);
	overlay[id].x = ox;
	overlay[id].y = oy;
	params._result = 0;
}

void AGSPalRender::GetTranslucentOverlayX(ScriptMethodParams &params) {
	PARAMS1(int, id);
	params._result = overlay[id].x;
}

void AGSPalRender::GetTranslucentOverlayY(ScriptMethodParams &params) {
	PARAMS1(int, id);
	params._result = overlay[id].y;
}

void AGSPalRender::GetTranslucentOverlaySprite(ScriptMethodParams &params) {
	PARAMS1(int, id);
	params._result = overlay[id].sprite;
}

void AGSPalRender::GetTranslucentOverlayLevel(ScriptMethodParams &params) {
	PARAMS1(int, id);
	params._result = overlay[id].level;
}

void AGSPalRender::GetTranslucentOverlayEnabled(ScriptMethodParams &params) {
	PARAMS1(int, id);
	params._result = overlay[id].enabled;
}

void AGSPalRender::GetTranslucentOverlayAlpha(ScriptMethodParams &params) {
	PARAMS1(int, id);
	params._result = overlay[id].trans;
}

void AGSPalRender::SetTranslucentOverlayAlpha(ScriptMethodParams &params) {
	PARAMS2(int, id, int, alpha);
	if (alpha >= 0 && alpha < 256)
		overlay[id].trans = alpha;
	else
		engine->AbortGame("CreateTranslucentOverlay: Invalid alpha selected.");
	params._result = 0;
}

void AGSPalRender::SetTranslucentOverlayEnabled(ScriptMethodParams &params) {
	PARAMS2(int, id, int, toggle);
	if (toggle > 0)
		overlay[id].enabled = true;
	else
		overlay[id].enabled = false;
	params._result = 0;
}

void AGSPalRender::SetCharacterReflected(ScriptMethodParams &params) {
	PARAMS2(int, id, int, refl);
	if (refl > 0) Reflection.Characters[id].reflect = 1;
	else Reflection.Characters[id].reflect = 0;
}

void AGSPalRender::SetObjectReflected(ScriptMethodParams &params) {
	PARAMS2(int, id, int, refl);
	if (refl > 0)
		Reflection.Objects[id].reflect = 1;
	else
		Reflection.Objects[id].reflect = 0;
}

void AGSPalRender::GetCharacterReflected(ScriptMethodParams &params) {
	PARAMS1(int, id);
	params._result = Reflection.Characters[id].reflect;
}

void AGSPalRender::GetObjectReflected(ScriptMethodParams &params) {
	PARAMS1(int, id);
	params._result = Reflection.Objects[id].reflect;
}

void AGSPalRender::ReplaceCharacterReflectionView(ScriptMethodParams &params) {
	PARAMS2(int, id, int, view);
	Reflection.Characters[id].replaceview = view - 1;
}

void AGSPalRender::SetObjectReflectionIgnoreScaling(ScriptMethodParams &params) {
	PARAMS2(int, id, int, wb);
	if (wb) Reflection.Objects[id].ignorescaling = 1;
	else    Reflection.Objects[id].ignorescaling = 0;
}

int DrawReflections(int id, int charobj = 0) {
	int32 screenw, screenh;
	int32 bgw, bgh;
	engine->GetScreenDimensions(&screenw, &screenh, nullptr);
	BITMAP *bgmask = engine->GetBackgroundScene(1);
	if (bgmask == nullptr) return 1;
	//BITMAP *virtsc = engine->GetVirtualScreen();
	BITMAP *walkbehind = engine->GetRoomMask(MASK_WALKBEHIND);
	//if (!virtsc) engine->AbortGame ("Can't load virtual screen.");
	if (!walkbehind) engine->AbortGame("DrawRelfections: Can't load Walkbehind into memory.");
	engine->GetBitmapDimensions(walkbehind, &bgw, &bgh, nullptr);
	if (!bgmask) engine->AbortGame("DrawReflections: Can't load reflection mask.");
	//unsigned char **charbuffer = engine->GetRawBitmapSurface (virtsc);
	uint8 *wbarray = engine->GetRawBitmapSurface(walkbehind);
	uint8 *maskarray = engine->GetRawBitmapSurface(bgmask);
	int wbPitch = engine->GetBitmapPitch(walkbehind);
	int maskPitch = engine->GetBitmapPitch(bgmask);
	//Initialize stuff
	BITMAP *charsprite = nullptr;
	BITMAP *charsprite2 = nullptr;
	AGSCharacter *currchar = nullptr;
	AGSObject *currobj;
	int cox = 0, coy = 0, coz = 0;
	int scale = 0;
	//Get character, and their sprite.
	if (charobj == 0) {
		currchar = engine->GetCharacter(id);
		/*int view = 0;
		if (Reflection.Characters[id].replaceview == 0) view = currchar->view + 1;
		else view = Reflection.Characters[id].replaceview;
		*/
		AGSViewFrame *vf = engine->GetViewFrame(currchar->view + 1, currchar->loop, currchar->frame);
		charsprite = engine->GetSpriteGraphic(vf->pic);
		long scaling = currchar->flags & CHF_NOSCALING;
		if (!scaling)scale = engine->GetAreaScaling(currchar->x, currchar->y);
		else scale = 100;
		cox = currchar->x;
		coy = currchar->y;
		coz = currchar->z;
	} else if (charobj == 1) {
		currobj = engine->GetObject(id);

		charsprite = engine->GetSpriteGraphic(currobj->num);
		if (Reflection.Objects[id].ignorescaling) scale = 100;
		else scale = engine->GetAreaScaling(currobj->x, currobj->y);
		cox = currobj->x;
		if (currobj->baseline < 0) coy = currobj->y;
		else coy = currobj->baseline;
		coz = 0;
	}
	bool scaled = false;
	int32 w, h;
	engine->GetBitmapDimensions(charsprite, &w, &h, nullptr);
	if (scale != 100) {
		uint8 *orig = engine->GetRawBitmapSurface(charsprite);
		int origPitch = engine->GetBitmapPitch(charsprite);
		int h1, h2, w1, w2;
		double fw2, fh2;
		h1 = h;
		w1 = w;
		fh2 = h1 * ((double)scale / 100.0);
		fw2 = w1 * ((double)scale / 100.0);
		h2 = static_cast<int>(fh2);
		w2 = static_cast<int>(fw2);
		charsprite2 = engine->CreateBlankBitmap(w2, h2, 8);
		uint8 *resized = engine->GetRawBitmapSurface(charsprite2);
		int resizedPitch = engine->GetBitmapPitch(charsprite2);
		int x_ratio = (int)((w1 << 16) / w2) + 1;
		int y_ratio = (int)((h1 << 16) / h2) + 1;
		int x2, y2 ;
		for (int i = 0; i < h2; i++) {
			for (int j = 0; j < w2; j++) {
				x2 = ((j * x_ratio) >> 16) ;
				y2 = ((i * y_ratio) >> 16) ;
				resized [i * resizedPitch + j] = orig [y2 * origPitch + x2];
			}
		}
		engine->ReleaseBitmapSurface(charsprite2);
		scaled = true;
		w = w2;
		h = h2;
	} else {
		charsprite2 = charsprite;
	}
	int transamount = 0;
	uint8 *spritearray = engine->GetRawBitmapSurface(charsprite2);
	uint8 *charbuffer = engine->GetRawBitmapSurface(rcolormap);
	uint8 *alphaarray = engine->GetRawBitmapSurface(ralphamap);
	int spritePitch = engine->GetBitmapPitch(charsprite2);
	int charPitch = engine->GetBitmapPitch(rcolormap);
	int alphaPitch = engine->GetBitmapPitch(ralphamap);
	int i = h - 1, j = 0;
	int32 ox = cox;
	if (charobj == 0) ox = ox - (w / 2);
	int32 oy = coy + coz - 1;
	engine->RoomToViewport(&ox, &oy);
	int yoffset = 0;
	int translevel = 7;
	//bool dither = false;
	//bool dodither = false;
	int counter = 0;
	int rowcount = 101 - (int)(50.0 * ((double)(scale) / 100.0));
	int delay = screenh / rowcount;
	int *obst;
	int flipped = 0;
	if (charobj == 0) {
		PluginMethod sfGetGameParameter = engine->GetScriptFunctionAddress("GetGameParameter");
		flipped = sfGetGameParameter(13, currchar->view + 1, currchar->loop, currchar->frame);
	} else flipped = 0;
	obst = new int [w];
	for (int k = 0; k < w; k++) {
		obst[k] = 0;
	}
	while (i > 0) {
		//if ((counter == delay/2-1 || counter == delay-1) && yoffset < 36) dodither = (!dodither);
		if (counter == delay) {
			counter = 0;
			if (translevel > 0) translevel--;
		} else counter++;
		yoffset++;
		while (j < w) {
			int xoffset;
			if (flipped == 1) xoffset = w - j - 1;
			else xoffset = j;
			int32 rx = ox + xoffset, ry = oy + yoffset;
			int wbb = 0;
			engine->ViewportToRoom(&rx, &ry);
			if (ry > 0 && ry < bgh && rx > 0 && rx < bgw) {
				if (wbarray [ry * wbPitch + rx] > 0) {
					wbb = engine->GetWalkbehindBaseline(wbarray[ry * wbPitch + rx]);
				}
				if (maskarray[ry * maskPitch + rx] == 21) obst[j] = 1;
			}

			//dither = (!dither);
			transamount = 32 * translevel;
			if (spritearray [i * spritePitch + j] != 0 && oy + yoffset < screenh && ox + xoffset < screenw && oy + yoffset >= 0 && ox + xoffset >= 0) { // If the sprite isn't transparent, and isn't drawn off the edge of the bg.
				if (wbb < ry && obst[j] == 0 && (oy > reflectionmap[(ox + xoffset) + (screenw * (oy + yoffset))])) {
					//charbuffer[(oy+yoffset) * charPitch + ox+xoffset] = MixColorAlpha (spritearray [i * spritePitch + j],charbuffer[(oy+yoffset) * charPitch + ox+xoffset],transamount);
					charbuffer [(oy + yoffset) * charPitch + ox + xoffset] = spritearray [i * spritePitch + j];
					alphaarray [(oy + yoffset) * alphaPitch + ox + xoffset] = transamount;
					reflectionmap[(ox + xoffset) + (screenw * (oy + yoffset))] = oy;
				}
			}
			j++;
		}
		//if (w % 2 == 0) dither = (!dither);
		i--;
		j = 0;
	}

	delete[] obst;
	if (scaled == true)engine->FreeBitmap(charsprite2);
	engine->ReleaseBitmapSurface(charsprite);
	//engine->ReleaseBitmapSurface (virtsc);
	//engine->ReleaseBitmapSurface (clutspr);
	engine->ReleaseBitmapSurface(bgmask);
	engine->ReleaseBitmapSurface(walkbehind);
	engine->ReleaseBitmapSurface(rcolormap);
	engine->ReleaseBitmapSurface(ralphamap);
	engine->MarkRegionDirty(ox, oy, ox + w, oy + h);
	return 0;
}


void AGSPalRender::DrawTransSprite(ScriptMethodParams &params) {
	PARAMS3(int, spriteId, int, bg, int, translevel);
	int mask = 0, blendmode = 0, use_objpal = 0;
	if (params.size() > 3)
		mask = params[3];
	if (params.size() > 4)
		blendmode = params[4];
	if (params.size() > 5)
		use_objpal = params[5];

	BITMAP *maskspr = nullptr;
	if (mask > 0) maskspr = engine->GetSpriteGraphic(mask);
	if (!maskspr && mask > 0) {
		char maskerr [100];
		snprintf(maskerr, 100, "DrawTransSprite: Can't load mask from slot %d.", mask);
		engine->AbortGame(maskerr);
	}
	// Get a reference to the screen we'll draw onto
	BITMAP *bgspr = engine->GetSpriteGraphic(bg);
	//BITMAP *clutspr = engine->GetSpriteGraphic (clutslot);
	BITMAP *spritespr = engine->GetSpriteGraphic(spriteId);
	if (!bgspr) engine->AbortGame("DrawTransSprite: Can't load background");
	//if (!clutspr) engine->AbortGame ("Can't load CLUT spriteId into memory.");
	if (!spritespr) engine->AbortGame("DrawTransSprite: Can't load overlay spriteId into memory.");
	// Get its surface
	int32 sprw, sprh, coldepth;
	int32 bgw, bgh;
	engine->GetBitmapDimensions(bgspr, &bgw, &bgh, &coldepth);
	engine->GetBitmapDimensions(spritespr, &sprw, &sprh, &coldepth);

	uint8 *bgarray = engine->GetRawBitmapSurface(bgspr);
	//uint8 *clutarray = engine->GetRawBitmapSurface (clutspr);
	uint8 *spritearray = engine->GetRawBitmapSurface(spritespr);
	int bgPitch = engine->GetBitmapPitch(bgspr);
	//int clutPitch = engine->GetBitmapPitch(clutspr);
	int spritePitch = engine->GetBitmapPitch(spritespr);

	unsigned char *maskarray = nullptr;
	int maskPitch = 0;
	if (mask > 0) {
		maskarray = engine->GetRawBitmapSurface(maskspr);
		maskPitch = engine->GetBitmapPitch(maskspr);
	}
	int tloffset = 255 - translevel;
	int x = 0;
	int y = 0;
	//int transamount = 256 * translevel; //old
	while (y < sprh) {
		while (x < sprw) {
			if (spritearray [y * spritePitch + x] != 0 && y < bgh && x < bgw && y >= 0 && x >= 0) { // If the spriteId isn't transparent, and isn't drawn off the edge of the bg.
				if (mask > 0) {
					translevel = MAX(maskarray [y * maskPitch + x] - tloffset, 0);
				}
				//spritearray[y * spritePitch + x] = cycle_remap[clutarray [(cycle_remap[bgarray[y * bgPitch + x]]+transamount) * clutPitch + (cycle_remap[spritearray [y * spritePitch + x]])]; //old
				if (blendmode == 0) spritearray[y * spritePitch + x] = Mix::MixColorAlpha(spritearray [y * spritePitch + x], bgarray[y * bgPitch + x], translevel, use_objpal);
				else if (blendmode == 1) spritearray[y * spritePitch + x] = Mix::MixColorAdditive(spritearray [y * spritePitch + x], bgarray[y * bgPitch + x], translevel, use_objpal);
			}
			x++;
		}
		x = 0;
		y++;
	}

	// Release the screen so that the engine can continue
	engine->ReleaseBitmapSurface(bgspr);
	//engine->ReleaseBitmapSurface (clutspr);
	engine->ReleaseBitmapSurface(spritespr);
	engine->NotifySpriteUpdated(spriteId);
	params._result = 0;
}

int DrawTranslucentOverlay(int spriteId, int translevel, int ox, int oy, int mask = 0, int blendmode = 0) {
	if (translevel == 0) return 0;
	BITMAP *maskspr = nullptr;
	// Get a reference to the screen we'll draw onto
	BITMAP *virtsc = engine->GetVirtualScreen();
	//BITMAP *clutspr = engine->GetSpriteGraphic (clutslot);
	BITMAP *spritespr = engine->GetSpriteGraphic(spriteId);
	if (mask > 0) maskspr = engine->GetSpriteGraphic(mask);
	if (!virtsc) engine->AbortGame("DrawTranslucentOverlay: Can't load virtual screen.");
	//if (!clutspr) engine->AbortGame ("Can't load CLUT spriteId into memory.");
	if (!spritespr) engine->AbortGame("DrawTranslucentOverlay: Can't load overlay spriteId into memory.");
	// Get its surface
	int32 sprw, sprh, coldepth;
	int32 screenw, screenh;
	engine->GetScreenDimensions(&screenw, &screenh, &coldepth);
	engine->GetBitmapDimensions(spritespr, &sprw, &sprh, &coldepth);
	uint8 *charbuffer = engine->GetRawBitmapSurface(virtsc);
	uint8 *spritearray = engine->GetRawBitmapSurface(spritespr);
	int charPitch = engine->GetBitmapPitch(virtsc);
	int spritePitch = engine->GetBitmapPitch(spritespr);
	uint8 *maskarray = nullptr;
	int maskPitch = 0;
	if (mask > 0) {
		if (!maskspr && mask > 0) {
			char maskerr [100];
			snprintf(maskerr, 100, "DrawTransSprite: Can't load mask from slot %d.", mask);
			engine->AbortGame(maskerr);
		}
		maskarray = engine->GetRawBitmapSurface(maskspr);
		maskPitch = engine->GetBitmapPitch(maskspr);
	}
	int tloffset = 255 - translevel;
	int x = 0;
	int y = 0;
	//int transamount = 256 * translevel; //old
	while (y < sprh) {
		while (x < sprw) {
			if (spritearray [y * spritePitch + x] != 0 && y + oy < screenh && x + ox < screenw && y + oy >= 0 && x + ox >= 0) { // If the spriteId isn't transparent, and isn't drawn off the edge of the screen.
				//charbuffer[(y+oy) * charPitch + x+ox] = cycle_remap[clutarray ([cycle_remap[charbuffer[(y+oy) * charPitch + x+ox]]+transamount) * clutPitch + (cycle_remap[spritearray [y * spritePitch + x]])]; //old
				if (mask > 0) {
					translevel = MAX(maskarray [y * maskPitch + x] - tloffset, 0);
				}
				if (blendmode == 0) {
					if (translevel == 255) {
						charbuffer[(y + oy) * charPitch + x + ox] = spritearray [y * spritePitch + x];
					} else charbuffer[(y + oy) * charPitch + x + ox] = Mix::MixColorAlpha(spritearray [y * spritePitch + x], charbuffer[(y + oy) * charPitch + x + ox], translevel);
				} else if (blendmode == 1) charbuffer[(y + oy) * charPitch + x + ox] = Mix::MixColorAdditive(spritearray [y * spritePitch + x], charbuffer[(y + oy) * charPitch + x + ox], translevel);
			}
			x++;
		}
		x = 0;
		y++;
	}

	// Release the screen so that the engine can continue
	long dirtywidth = ox + sprw;
	if (dirtywidth > screenw) dirtywidth = screenw - 1;
	long dirtyheight = oy + sprh;
	if (dirtyheight > screenh) dirtywidth = screenh - 1;
	engine->ReleaseBitmapSurface(virtsc);
	//engine->ReleaseBitmapSurface (clutspr);
	engine->ReleaseBitmapSurface(spritespr);
	if (mask > 0) engine->ReleaseBitmapSurface(maskspr);
	engine->MarkRegionDirty(ox, oy, dirtywidth, dirtyheight);

	return 0;
}

/*------------------------------------------------------------------*/

const char *AGSPalRender::AGS_GetPluginName() {
	return "PALgorithms Translucent Overlay Renderer";
}

void AGSPalRender::AGS_EngineStartup(IAGSEngine *lpEngine) {
	PluginBase::AGS_EngineStartup(lpEngine);
	engine = lpEngine;

	// Make sure it's got the version with the features we need
	if (engine->version < 3) {
		engine->AbortGame("Engine interface is too old, need newer version of AGS.");
	}

	SCRIPT_METHOD(PALInternal::LoadCLUT^1, AGSPalRender::LoadCLUT);
	SCRIPT_METHOD(PALInternal::CycleRemap^2, AGSPalRender::CycleRemap);
	SCRIPT_METHOD(PALInternal::GetColor565^3, AGSPalRender::GetColor565);
	SCRIPT_METHOD(PALInternal::GetLuminosityFromPalette^1, AGSPalRender::GetLuminosityFromPalette);
	SCRIPT_METHOD(PALInternal::FastSin^1, AGSPalRender::AGSFastSin);
	SCRIPT_METHOD(PALInternal::FastCos^1, AGSPalRender::AGSFastCos);
	SCRIPT_METHOD(PALInternal::FastRoot^1, AGSPalRender::AGSFastRoot);
	SCRIPT_METHOD(PALInternal::GetRemappedSlot^1, AGSPalRender::GetRemappedSlot);
	SCRIPT_METHOD(PALInternal::ResetRemapping^0, AGSPalRender::ResetRemapping);
	SCRIPT_METHOD(PALInternal::GetModifiedBackgroundImage, AGSPalRender::GetModifiedBackgroundImage);
	SCRIPT_METHOD(PALInternal::WriteObjectivePalette^4, AGSPalRender::WriteObjectivePalette);
	SCRIPT_METHOD(PALInternal::ReadObjectivePaletteR^1, AGSPalRender::ReadObjectivePaletteR);
	SCRIPT_METHOD(PALInternal::ReadObjectivePaletteB^1, AGSPalRender::ReadObjectivePaletteB);
	SCRIPT_METHOD(PALInternal::ReadObjectivePaletteG^1, AGSPalRender::ReadObjectivePaletteG);

	SCRIPT_METHOD(Raycast::Render^1, AGSPalRender::Raycast_Render);
	SCRIPT_METHOD(Raycast::LoadMap^4, AGSPalRender::LoadMap);
	SCRIPT_METHOD(Raycast::Initialize, AGSPalRender::Init_Raycaster);
	SCRIPT_METHOD(Raycast::MakeTextures^1, AGSPalRender::MakeTextures);
	SCRIPT_METHOD(Raycast::MoveForward^0, AGSPalRender::MoveForward);
	SCRIPT_METHOD(Raycast::MoveBackward^0, AGSPalRender::MoveBackward);
	SCRIPT_METHOD(Raycast::RotateLeft^0, AGSPalRender::RotateLeft);
	SCRIPT_METHOD(Raycast::RotateRight^0, AGSPalRender::RotateRight);
	SCRIPT_METHOD(Raycast::SetCameraPosition^2, AGSPalRender::Ray_SetPlayerPosition);
	SCRIPT_METHOD(Raycast::GetCameraX^0, AGSPalRender::Ray_GetPlayerX);
	SCRIPT_METHOD(Raycast::GetCameraY^0, AGSPalRender::Ray_GetPlayerY);
	SCRIPT_METHOD(Raycast::GetCameraAngle^0, AGSPalRender::Ray_GetPlayerAngle);
	SCRIPT_METHOD(Raycast::SetCameraAngle^1, AGSPalRender::Ray_SetPlayerAngle);
	SCRIPT_METHOD(Raycast::InitSprite^9, AGSPalRender::Ray_InitSprite);
	SCRIPT_METHOD(Raycast::UnloadEngine^0, AGSPalRender::QuitCleanup);
	SCRIPT_METHOD(Raycast::GetHotspotAtXY^2, AGSPalRender::Ray_GetHotspotAt);
	SCRIPT_METHOD(Raycast::GetObjectAtXY^2, AGSPalRender::Ray_GetObjectAt);
	SCRIPT_METHOD(Raycast::SetSpriteInteractObj^2, AGSPalRender::Ray_SetSpriteInteractObj);
	SCRIPT_METHOD(Raycast::GetSpriteInteractObj^1, AGSPalRender::Ray_GetSpriteInteractObj);
	SCRIPT_METHOD(Raycast::SetSpritePosition^3, AGSPalRender::Ray_SetSpritePosition);
	SCRIPT_METHOD(Raycast::SetSpriteVertOffset^2, AGSPalRender::Ray_SetSpriteVertOffset);
	SCRIPT_METHOD(Raycast::GetSpriteVertOffset^1, AGSPalRender::Ray_GetSpriteVertOffset);
	SCRIPT_METHOD(Raycast::GetSpriteX^1, AGSPalRender::Ray_GetSpriteX);
	SCRIPT_METHOD(Raycast::GetSpriteY^1, AGSPalRender::Ray_GetSpriteY);
	SCRIPT_METHOD(Raycast::SetWallHotspot^2, AGSPalRender::Ray_SetWallHotspot);
	SCRIPT_METHOD(Raycast::SetWallTextures^5, AGSPalRender::Ray_SetWallTextures);
	SCRIPT_METHOD(Raycast::SetWallSolid^5, AGSPalRender::Ray_SetWallSolid);
	SCRIPT_METHOD(Raycast::SetWallIgnoreLighting^5, AGSPalRender::Ray_SetWallIgnoreLighting);
	SCRIPT_METHOD(Raycast::SetWallAlpha^5, AGSPalRender::Ray_SetWallAlpha);
	SCRIPT_METHOD(Raycast::SetWallBlendType^5, AGSPalRender::Ray_SetWallBlendType);
	SCRIPT_METHOD(Raycast::GetMoveSpeed^0, AGSPalRender::Ray_GetMoveSpeed);
	SCRIPT_METHOD(Raycast::SetMoveSpeed^1, AGSPalRender::Ray_SetMoveSpeed);
	SCRIPT_METHOD(Raycast::GetRotSpeed^0, AGSPalRender::Ray_GetRotSpeed);
	SCRIPT_METHOD(Raycast::SetRotSpeed^1, AGSPalRender::Ray_SetRotSpeed);
	SCRIPT_METHOD(Raycast::GetWallAt^2, AGSPalRender::Ray_GetWallAt);
	SCRIPT_METHOD(Raycast::GetLightAt^2, AGSPalRender::Ray_GetLightAt);
	SCRIPT_METHOD(Raycast::SetLightAt^3, AGSPalRender::Ray_SetLightAt);
	SCRIPT_METHOD(Raycast::SetWallAt^3, AGSPalRender::Ray_SetWallAt);
	SCRIPT_METHOD(Raycast::SetPlaneY^1, AGSPalRender::Ray_SetPlaneY);
	SCRIPT_METHOD(Raycast::GetDistanceAt^2, AGSPalRender::Ray_GetDistanceAt);
	SCRIPT_METHOD(Raycast::GetSpriteAngle^1, AGSPalRender::Ray_GetSpriteAngle);
	SCRIPT_METHOD(Raycast::SetSpriteAngle^2, AGSPalRender::Ray_SetSpriteAngle);
	SCRIPT_METHOD(Raycast::SetSpriteView^2, AGSPalRender::Ray_SetSpriteView);
	SCRIPT_METHOD(Raycast::GetSpriteView^1, AGSPalRender::Ray_GetSpriteView);
	SCRIPT_METHOD(Raycast::SetSpriteFrame^2, AGSPalRender::Ray_SetSpriteFrame);
	SCRIPT_METHOD(Raycast::GetSpriteFrame^1, AGSPalRender::Ray_GetSpriteFrame);
	SCRIPT_METHOD(Raycast::SetSpritePic^2, AGSPalRender::Ray_SetSpritePic);
	SCRIPT_METHOD(Raycast::GetSpritePic^1, AGSPalRender::Ray_GetSpritePic);
	SCRIPT_METHOD(Raycast::SetSkyBox^1, AGSPalRender::Ray_SetSkyBox);
	SCRIPT_METHOD(Raycast::SetSpriteAlpha^2, AGSPalRender::Ray_SetSpriteAlpha);
	SCRIPT_METHOD(Raycast::GetSpriteAlpha^1, AGSPalRender::Ray_GetSpriteAlpha);
	SCRIPT_METHOD(Raycast::GetSkyBox^1, AGSPalRender::Ray_GetSkyBox);
	SCRIPT_METHOD(Raycast::SetAmbientLight^1, AGSPalRender::Ray_SetAmbientLight);
	SCRIPT_METHOD(Raycast::SetAmbientColor^2, AGSPalRender::Ray_SetAmbientColor);
	SCRIPT_METHOD(Raycast::GetAmbientLight^0, AGSPalRender::Ray_GetAmbientLight);
	SCRIPT_METHOD(Raycast::GetAmbientWeight^0, AGSPalRender::Ray_GetAmbientWeight);
	SCRIPT_METHOD(Raycast::GetTileX_At^2, AGSPalRender::Ray_GetTileX_At);
	SCRIPT_METHOD(Raycast::GetTileY_At^2, AGSPalRender::Ray_GetTileY_At);
	SCRIPT_METHOD(Raycast::DrawTile^2, AGSPalRender::Ray_DrawTile);
	SCRIPT_METHOD(Raycast::DrawOntoTile^2, AGSPalRender::Ray_DrawOntoTile);
	SCRIPT_METHOD(Raycast::SetNoClip^1, AGSPalRender::Ray_SetNoClip);
	SCRIPT_METHOD(Raycast::GetNoClip^0, AGSPalRender::Ray_GetNoClip);
	SCRIPT_METHOD(Raycast::GetSpriteScaleX^1, AGSPalRender::Ray_GetSpriteScaleX);
	SCRIPT_METHOD(Raycast::SetSpriteScaleX^2, AGSPalRender::Ray_SetSpriteScaleX);
	SCRIPT_METHOD(Raycast::GetSpriteScaleY^1, AGSPalRender::Ray_GetSpriteScaleY);
	SCRIPT_METHOD(Raycast::SetSpriteScaleY^2, AGSPalRender::Ray_SetSpriteScaleY);
	SCRIPT_METHOD(Raycast::GetSpriteBlendType^1, AGSPalRender::Ray_GetSpriteBlendType);
	SCRIPT_METHOD(Raycast::SetSpriteBlendType^2, AGSPalRender::Ray_SetSpriteBlendType);


	SCRIPT_METHOD(Raycast::SetFloorAt^3, AGSPalRender::Ray_SetFloorAt);
	SCRIPT_METHOD(Raycast::SetCeilingAt^3, AGSPalRender::Ray_SetCeilingAt);
	SCRIPT_METHOD(Raycast::GetCeilingAt^2, AGSPalRender::Ray_GetCeilingAt);
	SCRIPT_METHOD(Raycast::GetFloorAt^2, AGSPalRender::Ray_GetFloorAt);
	SCRIPT_METHOD(Raycast::GetLightingAt^2, AGSPalRender::Ray_GetLightingAt);
	SCRIPT_METHOD(Raycast::SetLightingAt^3, AGSPalRender::Ray_SetLightingAt);
	SCRIPT_METHOD(Raycast::GetWallHotspot^1, AGSPalRender::Ray_GetWallHotspot);
	SCRIPT_METHOD(Raycast::GetWallTexture^2, AGSPalRender::Ray_GetWallTexture);
	SCRIPT_METHOD(Raycast::GetWallSolid^2, AGSPalRender::Ray_GetWallSolid);
	SCRIPT_METHOD(Raycast::GetWallIgnoreLighting^2, AGSPalRender::Ray_GetWallIgnoreLighting);
	SCRIPT_METHOD(Raycast::GetWallAlpha^2, AGSPalRender::Ray_GetWallAlpha);
	SCRIPT_METHOD(Raycast::GetWallBlendType^2, AGSPalRender::Ray_GetWallBlendType);
	SCRIPT_METHOD(Raycast::SelectTile^3, AGSPalRender::Ray_SelectTile);
	SCRIPT_METHOD(Raycast::HasSeenTile^2, AGSPalRender::Ray_HasSeenTile);

	SCRIPT_METHOD(LensDistort::SetPos^2, AGSPalRender::SetLensPos);
	SCRIPT_METHOD(LensDistort::GetX^0, AGSPalRender::GetLensX);
	SCRIPT_METHOD(LensDistort::GetY^0, AGSPalRender::GetLensY);
	SCRIPT_METHOD(LensDistort::Set^1, AGSPalRender::SetLensDrawn);
	SCRIPT_METHOD(LensDistort::IsDrawn^0, AGSPalRender::GetLensDrawn);
	SCRIPT_METHOD(LensDistort::SetOffsetClamp^1, AGSPalRender::SetLensOffsetClamp);
	SCRIPT_METHOD(LensDistort::GetOffsetClamp^0, AGSPalRender::GetLensOffsetClamp);
	SCRIPT_METHOD(LensDistort::GetLevel^0, AGSPalRender::GetLensLevel);
	SCRIPT_METHOD(LensDistort::SetLevel^1, AGSPalRender::SetLensLevel);
	SCRIPT_METHOD(LensDistort::Initialize^6, AGSPalRender::LensInitialize);

	SCRIPT_METHOD(Translucence::CreateOverlay^8, AGSPalRender::CreateTranslucentOverlay);
	SCRIPT_METHOD(Translucence::DeleteOverlay^1, AGSPalRender::DeleteTranslucentOverlay);
	SCRIPT_METHOD(Translucence::Move^3, AGSPalRender::MoveTranslucentOverlay);
	SCRIPT_METHOD(Translucence::GetOverlayX^1, AGSPalRender::GetTranslucentOverlayX);
	SCRIPT_METHOD(Translucence::GetOverlayY^1, AGSPalRender::GetTranslucentOverlayY);
	SCRIPT_METHOD(Translucence::GetOverlaySprite^1, AGSPalRender::GetTranslucentOverlaySprite);
	SCRIPT_METHOD(Translucence::GetOverlayLevel^1, AGSPalRender::GetTranslucentOverlayLevel);
	SCRIPT_METHOD(Translucence::GetOverlayEnabled^1, AGSPalRender::GetTranslucentOverlayEnabled);
	SCRIPT_METHOD(Translucence::GetOverlayAlpha^1, AGSPalRender::GetTranslucentOverlayAlpha);
	SCRIPT_METHOD(Translucence::SetOverlayAlpha^2, AGSPalRender::SetTranslucentOverlayAlpha);
	SCRIPT_METHOD(Translucence::SetOverlayEnabled^2, AGSPalRender::SetTranslucentOverlayEnabled);
	SCRIPT_METHOD(Translucence::DrawTransSprite^6, AGSPalRender::DrawTransSprite);

	SCRIPT_METHOD(Starfield::GetOverscan^0, AGSPalRender::GetStarfieldOverscan);
	SCRIPT_METHOD(Starfield::SetOverscan^1, AGSPalRender::SetStarfieldOverscan);
	SCRIPT_METHOD(Starfield::GetOriginX^0, AGSPalRender::GetStarfieldOriginX);
	SCRIPT_METHOD(Starfield::GetOriginY^0, AGSPalRender::GetStarfieldOriginY);
	SCRIPT_METHOD(Starfield::SetDepthMultiplier^1, AGSPalRender::SetStarfieldDepthMultiplier);
	SCRIPT_METHOD(Starfield::GetDepthMultiplier^0, AGSPalRender::GetStarfieldDepthMultiplier);
	SCRIPT_METHOD(Starfield::GetMaxStars^0, AGSPalRender::GetStarfieldMaxStars);
	SCRIPT_METHOD(Starfield::SetStarSpriteScaleBoost^1, AGSPalRender::SetStarSpriteScaleBoost);
	SCRIPT_METHOD(Starfield::GetStarSpriteScaleBoost^0, AGSPalRender::GetStarSpriteScaleBoost);
	SCRIPT_METHOD(Starfield::SetStarMaxRadius^2, AGSPalRender::SetStarMaxRadius);
	SCRIPT_METHOD(Starfield::GetStarMaxRadius^0, AGSPalRender::GetStarMaxRadius);
	SCRIPT_METHOD(Starfield::GetStarX^1, AGSPalRender::GetStarX);
	SCRIPT_METHOD(Starfield::GetStarY^1, AGSPalRender::GetStarY);
	SCRIPT_METHOD(Starfield::GetStarZ^1, AGSPalRender::GetStarZ);
	SCRIPT_METHOD(Starfield::SetStarPosition^4, AGSPalRender::SetStarPosition);
	SCRIPT_METHOD(Starfield::RotateStar^4, AGSPalRender::RotateStar);
	SCRIPT_METHOD(Starfield::SetStarColor^2, AGSPalRender::SetStarColor);
	SCRIPT_METHOD(Starfield::GetStarColor^1, AGSPalRender::GetStarColor);
	SCRIPT_METHOD(Starfield::SetStarSprite^2, AGSPalRender::SetStarSprite);
	SCRIPT_METHOD(Starfield::GetStarSprite^1, AGSPalRender::GetStarSprite);
	SCRIPT_METHOD(Starfield::SetStarSpriteRange^3, AGSPalRender::SetStarSpriteRange);
	SCRIPT_METHOD(Starfield::Initialize^2, AGSPalRender::InitializeStars);
	SCRIPT_METHOD(Starfield::Iterate^1, AGSPalRender::IterateStars);
	SCRIPT_METHOD(Starfield::Draw^2, AGSPalRender::DrawStars);
	SCRIPT_METHOD(Starfield::SetOriginPoint^2, AGSPalRender::SetStarsOriginPoint);

	SCRIPT_METHOD(Plasma::DoFire^8, AGSPalRender::DoFire);
	SCRIPT_METHOD(Plasma::SetPlasmaType^5, AGSPalRender::SetPlasmaType);
	SCRIPT_METHOD(Plasma::ResetPlasmaSettings^0, AGSPalRender::ResetPlasmaSettings);
	SCRIPT_METHOD(Plasma::DrawPlasma^3, AGSPalRender::DrawPlasma);
	SCRIPT_METHOD(Plasma::SetRootType^1, AGSPalRender::SetPlasmaRootType);
	SCRIPT_METHOD(Plasma::GetRootType^0, AGSPalRender::GetPlasmaRootType);

	SCRIPT_METHOD(Reflections::Set^1, AGSPalRender::SetReflections);
	SCRIPT_METHOD(Reflections::IsReflecting^0, AGSPalRender::IsReflectionsOn);
	SCRIPT_METHOD(Reflections::SetCharacterReflected^2, AGSPalRender::SetCharacterReflected);
	SCRIPT_METHOD(Reflections::GetCharacterReflected^1, AGSPalRender::GetCharacterReflected);
	SCRIPT_METHOD(Reflections::SetObjectReflected^2, AGSPalRender::SetObjectReflected);
	SCRIPT_METHOD(Reflections::GetObjectReflected^1, AGSPalRender::GetObjectReflected);
	SCRIPT_METHOD(Reflections::ReplaceCharacterReflectionView^2, AGSPalRender::ReplaceCharacterReflectionView);
	SCRIPT_METHOD(Reflections::SetObjectReflectionIgnoreScaling^2, AGSPalRender::SetObjectReflectionIgnoreScaling);

	engine->RequestEventHook(AGSE_PRESCREENDRAW);
	engine->RequestEventHook(AGSE_PREGUIDRAW);
	engine->RequestEventHook(AGSE_POSTSCREENDRAW);
	engine->RequestEventHook(AGSE_SAVEGAME);
	engine->RequestEventHook(AGSE_RESTOREGAME);
	engine->RequestEventHook(AGSE_ENTERROOM);
	stars = new starstype [MAX_STARS];
	Starfield.maxstars = MAX_STARS;
	Starfield.depthmultiplier = 256;
	Starfield.speed = 0.5;
	Starfield.originx = 160;
	Starfield.originy = 100;
	Reflection.Characters = new charrefopt [engine->GetNumCharacters()]();
	lens = new LensDistort [LENS_WIDTH * LENS_WIDTH]();
	//PreMultiply_Alphas ();
	plasmaroottype = 0;
	Make_Sin_Lut();
	ScriptMethodParams params;
	Init_Raycaster(params);
}

void AGSPalRender::AGS_EngineShutdown() {
	// no work to do here - but if we had created any dynamic sprites,
	// we should delete them here
	delete[] Reflection.Characters;
	delete[] Reflection.Objects;
	//QuitCleanup();
}

int64 AGSPalRender::AGS_EngineOnEvent(int event, NumberPtr data) {
	if (event == AGSE_PRESCREENDRAW && clutslot > 0) {
		if (drawreflections) {
			int32 sh, sw = 0;
			engine->GetScreenDimensions(&sw, &sh, nullptr);
			reflectionmap = new long[sw * sh]();
			rcolormap = engine->CreateBlankBitmap(sw, sh, 8);
			ralphamap = engine->CreateBlankBitmap(sw, sh, 8);
			for (int i = 0; i < engine->GetNumCharacters(); i++) {
				if (Reflection.Characters[i].reflect == 0) continue;
				AGSCharacter *tempchar = engine->GetCharacter(i);
				if (tempchar->room != engine->GetCurrentRoom()) continue;  //if character isn't even in the room, go to next iteration.
				int32 vx = tempchar->x;
				int32 vy = tempchar->y;
				engine->RoomToViewport(&vx, &vy);
				AGSViewFrame *vf = engine->GetViewFrame(tempchar->view + 1, tempchar->loop, tempchar->frame);
				int w = engine->GetSpriteWidth(vf->pic);
				int h = engine->GetSpriteHeight(vf->pic);
				vx = vx - (w / 2);
				int32 vxmax = vx + w;
				int32 vymax = vy + h;
				if (vxmax < 0 || vy > sh || vx > sw || vymax < 0) continue; //if all of the sprite is off screen in any direction, go to next iteration
				DrawReflections(i, 0);
			}
			for (int i = 0; i < engine->GetNumObjects(); i++) {
				if (Reflection.Objects[i].reflect == 0) continue;
				AGSObject *tempobj = engine->GetObject(i);
				if (!tempobj->on) continue;
				int32 vx = tempobj->x;
				int32 vy = tempobj->baseline - tempobj->y;
				engine->RoomToViewport(&vx, &vy);
				int32 w = engine->GetSpriteWidth(tempobj->num);
				int32 h = engine->GetSpriteHeight(tempobj->num);
				int32 vxmax = vx + w;
				int32 vymax = vy + h;
				if (vxmax < 0 || vy > sh || vx > sw || vymax < 0) continue; //if all of the sprite is off screen in any direction, go to next iteration
				DrawReflections(i, 1);
			}
			BITMAP *virtsc = engine->GetVirtualScreen();
			uint8 *screenbuffer = engine->GetRawBitmapSurface(virtsc);
			uint8 *colorbuffer = engine->GetRawBitmapSurface(rcolormap);
			uint8 *alphabuffer = engine->GetRawBitmapSurface(ralphamap);
			int screenPitch = engine->GetBitmapPitch(virtsc);
			int colorPitch = engine->GetBitmapPitch(rcolormap);
			int alphaPitch = engine->GetBitmapPitch(ralphamap);
			for (int y = 0, screeny = 0, colory = 0, alphay = 0; y < sh; y++, screeny += screenPitch, colory += colorPitch, alphay += alphaPitch)
				for (int x = 0; x < sw; x++)
					screenbuffer[screeny+x] = Mix::MixColorAlpha(colorbuffer[colory+x], screenbuffer[screeny+x], alphabuffer[alphay+x]);
			engine->ReleaseBitmapSurface(rcolormap);
			engine->ReleaseBitmapSurface(ralphamap);
			engine->ReleaseBitmapSurface(virtsc);
			engine->FreeBitmap(rcolormap);
			engine->FreeBitmap(ralphamap);
			delete[] reflectionmap;
		}
		int i = 0;
		if (LensOption.draw == 1 && LensOption.level == 0) DrawLens(LensOption.x, LensOption.y);
		while (i < MAX_OVERLAYS) {
			if (overlay[i].enabled && overlay[i].level == 0) {
				DrawTranslucentOverlay(overlay[i].sprite, overlay[i].trans, overlay[i].x, overlay[i].y, overlay[i].spritemask, overlay[i].blendtype);
			}
			i++;
		}
		if (LensOption.draw == 1 && LensOption.level == 1) DrawLens(LensOption.x, LensOption.y);
	}
	if (event == AGSE_PREGUIDRAW && clutslot > 0) {
		int i = 0;
		if (LensOption.draw == 1 && LensOption.level == 1) DrawLens(LensOption.x, LensOption.y);
		while (i < MAX_OVERLAYS) {
			if (overlay[i].enabled && overlay[i].level == 1) {
				DrawTranslucentOverlay(overlay[i].sprite, overlay[i].trans, overlay[i].x, overlay[i].y, overlay[i].spritemask, overlay[i].blendtype);
			}
			i++;
		}
		if (LensOption.draw == 1 && LensOption.level == 2) DrawLens(LensOption.x, LensOption.y);
	}
	if (event == AGSE_POSTSCREENDRAW && clutslot > 0) {
		int i = 0;
		if (LensOption.draw == 1 && LensOption.level == 3) DrawLens(LensOption.x, LensOption.y);
		while (i < MAX_OVERLAYS) {
			if (overlay[i].enabled && overlay[i].level == 2) {
				DrawTranslucentOverlay(overlay[i].sprite, overlay[i].trans, overlay[i].x, overlay[i].y, overlay[i].spritemask, overlay[i].blendtype);
			}
			i++;
		}
		if (LensOption.draw == 1 && LensOption.level == 4) DrawLens(LensOption.x, LensOption.y);
	}
	if (event == AGSE_SAVEGAME) {
		Serializer s(engine, data, false);
		syncGame(s);
	}
	if (event == AGSE_RESTOREGAME) {
		Serializer s(engine, data, true);
		syncGame(s);
	}
	if (event == AGSE_ENTERROOM) {
		ScriptMethodParams params;
		ResetRemapping(params);
		delete[] Reflection.Objects;
		Reflection.Objects = new objrefopt [engine->GetNumObjects()]();
	}
	return 0;
}

void AGSPalRender::syncGame(Serializer &s) {
	for (int i = 0; i < MAX_OVERLAYS; ++i) {
		s.syncAsInt(overlay[i].sprite);
		s.syncAsInt(overlay[i].spritemask);
		s.syncAsInt(overlay[i].x);
		s.syncAsInt(overlay[i].y);
		s.syncAsInt(overlay[i].level);
		s.syncAsInt(overlay[i].trans);
		s.syncAsInt(overlay[i].blendtype);
		s.syncAsBool(overlay[i].enabled);
	}
	s.syncAsInt(clutslot);
	s.syncAsInt(drawreflections);

	for (int j = 0; j < 256; ++j)
		s.syncAsByte(cycle_remap[j]);

	for (int j = 0; j < 256; ++j) {
		s.syncAsByte(objectivepal[j].r);
		s.syncAsByte(objectivepal[j].b);
		s.syncAsByte(objectivepal[j].g);
	}

	for (int j = 0; j < 256; ++j) {
		s.syncAsDouble(sprite[j].x);
		s.syncAsDouble(sprite[j].y);
		s.syncAsInt(sprite[j].texture);
		s.syncAsByte(sprite[j].alpha);
		s.syncAsDouble(sprite[j].uDivW);
		s.syncAsDouble(sprite[j].uDivH);
		s.syncAsDouble(sprite[j].vMove);
		s.syncAsDouble(sprite[j].hMove);
		s.syncAsInt8(sprite[j].objectinteract);
		s.syncAsInt(sprite[j].view);
		s.syncAsInt(sprite[j].frame);
		s.syncAsInt(sprite[j].angle);
	}

	for (int j = 0; j < 256; ++j) {
		for (int k = 0; k < 4; ++k) {
			s.syncAsInt(wallData[j].texture[k]);
			s.syncAsInt(wallData[j].solid[k]);
			s.syncAsInt(wallData[j].ignorelighting[k]);
			s.syncAsInt(wallData[j].alpha[k]);
			s.syncAsInt(wallData[j].blendtype[k]);
			s.syncAsInt(wallData[j].mask[k]);
		}

		s.syncAsByte(wallData[j].hotspotinteract);
	}

	s.syncAsBool(raycastOn);
	s.syncAsBool(heightmapOn);
	s.syncAsDouble(posX);
	s.syncAsDouble(posY);
	s.syncAsDouble(dirX);
	s.syncAsDouble(dirY);
	s.syncAsDouble(planeX);
	s.syncAsDouble(planeY);
	s.syncAsDouble(moveSpeed);
	s.syncAsDouble(rotSpeed);

	if (raycastOn) { //If the raycaster is active, we have additional data to save.
		for (int i = 0; i < MAP_WIDTH; ++i)
			for (int j = 0; j < MAP_HEIGHT; ++j) {
				s.syncAsByte(worldMap[i][j]);
				s.syncAsByte(lightMap[i][j]);
				s.syncAsInt(ceilingMap[i][j]);
				s.syncAsInt(floorMap[i][j]);
				s.syncAsInt(heightMap[i][j]);
			}
	}

	s.syncAsInt(textureSlot);
	if (s.isLoading() && textureSlot) {
		ScriptMethodParams params;
		params.push_back(textureSlot);
		MakeTextures(params);
	}

	s.syncAsInt(skybox);
	s.syncAsInt(ambientlight);

	if (s.isLoading()) {
		ScriptMethodParams params;
		params.push_back(clutslot);
		LoadCLUT(params);
	}
}

} // namespace AGSPalRender
} // namespace Plugins
} // namespace AGS3
