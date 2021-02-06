/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
unsigned char cycle_remap [256];

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
	char reflect;
	int replaceview;
};

struct objrefopt {
	char reflect;
	char ignorescaling;
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

void WriteObjectivePalette(unsigned char index, unsigned char r, unsigned char b, unsigned char g) {
	objectivepal[index].r = r;
	objectivepal[index].b = b;
	objectivepal[index].g = g;
}

int ReadObjectivePaletteR(unsigned char index) {
	return objectivepal[index].r;
}

int ReadObjectivePaletteB(unsigned char index) {
	return objectivepal[index].b;
}
int ReadObjectivePaletteG(unsigned char index) {
	return objectivepal[index].g;
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

int GetModifiedBackgroundImage() {
	return bgimgspr;
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

FLOAT_RETURN_TYPE AGSFastSin(SCRIPT_FLOAT(x)) {
	INIT_SCRIPT_FLOAT(x);
	x = FastSin(x);
	RETURN_FLOAT(x);
}

FLOAT_RETURN_TYPE AGSFastCos(SCRIPT_FLOAT(x)) {
	INIT_SCRIPT_FLOAT(x);
	x = FastSin(x + halfpi);
	RETURN_FLOAT(x);
}


void DrawLens(int ox, int oy) {
	int32 sh, sw = 0;
	engine->GetScreenDimensions(&sw, &sh, nullptr);
	BITMAP *virtsc = engine->GetVirtualScreen();
	if (!virtsc) engine->AbortGame("DrawLens: Cannot get virtual screen.");
	BITMAP *lenswrite = engine->CreateBlankBitmap(LensOption.lenswidth, LensOption.lenswidth, 8);
	unsigned char **vScreen = engine->GetRawBitmapSurface(virtsc);
	unsigned char **lensarray = engine->GetRawBitmapSurface(lenswrite);
	int radius = LensOption.lenswidth >> 1;
	for (int y = 0; y < LensOption.lenswidth; y++) {
		int ypos = y * LensOption.lenswidth;
		for (int x = 0; x < LensOption.lenswidth; x++) {
			int lenspos = ypos + x;
			int coffx = lens[lenspos].xoffset;
			int coffy = lens[lenspos].yoffset;
			if (oy + coffy > 0 && oy + coffy < sh && ox + coffx > 0 && ox + coffx < sw) {
				lensarray[y][x] = vScreen[oy + coffy][ox + coffx];
				//vScreen[oy+coffy][ox+coffx] = ABS(coffy);
			}
		}
	}
	/*
	for (int y=0;y<LensOption.lenswidth;y++)
	{
	    int ypos = y*LensOption.lenswidth;
	    for (int x=0;x<LensOption.lenswidth;x++)
	    {
	        if (oy+y > 0 && oy+y < sh && ox+x > 0 && ox+x < sw)
	        {
	            vScreen[oy+y][ox+x] = lensarray[y][x];
	        }
	    }
	}
	*/
	int radsq = radius * radius;
	for (int cy = -radius; cy <= radius; cy++) { //Draw a circle around the point, for the mask.
		int cysq = cy * cy;
		for (int cx = -radius; cx <= radius; cx++) {
			int cxsq = cx * cx;
			int dx = cx + ox;
			int dy = cy + oy;
			if ((cxsq + cysq <= radsq) && dx < sw && dx >= 0 && dy < sh && dy >= 0 && cy + radius < LensOption.lenswidth - 1 && cx + radius < LensOption.lenswidth - 1) {
				//if (cy+radius < 0 || cx+radius < 0) engine->AbortGame ("I did something wrong");
				vScreen[dy][dx] = lensarray[cy + radius][cx + radius];
			}
		}
	}

	engine->ReleaseBitmapSurface(lenswrite);
	engine->ReleaseBitmapSurface(virtsc);
	engine->FreeBitmap(lenswrite);
}

void SetLensPos(int x, int y) {
	LensOption.x = x;
	LensOption.y = y;
}

int GetLensX() {
	return LensOption.x;
}

int GetLensY() {
	return LensOption.y;
}

void SetLensDrawn(int toggle) {
	if (toggle > 0) LensOption.draw = 1;
	else LensOption.draw = 0;
}

int GetLensDrawn() {
	return LensOption.draw;
}

void SetLensOffsetClamp(int clamp) {
	if (clamp < 0) LensOption.clampoffset = LensOption.lenswidth;
	else LensOption.clampoffset = clamp;
}

int GetLensOffsetClamp() {
	return LensOption.clampoffset;
}

int GetLensLevel() {
	return LensOption.level;
}

void SetLensLevel(int level) {
	if (level < 0 || level > 4) engine->AbortGame("SetLensLevel: Invalid level.");
	else LensOption.level = level;
}

void LensInitialize(int width, int zoom, int lensx, int lensy, int level, int clamp = -1) {
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

void ResetRemapping() {
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


void SetPlasmaRootType(int real) {
	if (real) plasmaroottype = 1;
	else plasmaroottype = 0;
}

int GetPlasmaRootType() {
	return plasmaroottype;
}

void SetPlasmaType(int component, int type, int data, int data2, int data3) {
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

void ResetPlasmaSettings() {
	int i = 0;
	while (i < MAX_PLASMA_COMPLEXITY) {
		plasmatype [i] = 0;
		plasmadata [i] = 0;
		plasmadata2[i] = 0;
		plasmadata3[i] = 0;
		i++;
	}
}

void DrawPlasma(int slot, int palstart, int palend) {
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
	unsigned char **plasmarray = engine->GetRawBitmapSurface(plasmaspr);
	double frange = range / 2.0;
	int complex = 0;
	int color = 0;
	int i = 0;
	while (i < MAX_PLASMA_COMPLEXITY) {
		if (plasmatype[i] > 0) complex++;
		i++;
	}
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
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
			plasmarray[y][x] = static_cast<unsigned char>(basecol + color);
		}
	}
	engine->ReleaseBitmapSurface(plasmaspr);
	engine->NotifySpriteUpdated(slot);
}

void DoFire(int spriteId, int masksprite, int palstart, int palend, int strength, int seed, int cutoff, int windspeed) {
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
	unsigned char **fire = engine->GetRawBitmapSurface(firespr);
	unsigned char **color = engine->GetRawBitmapSurface(firecolorspr);
	int sparky = 0;
	//srand(time(NULL));
	for (int y = 0; y < h - 1; y++) {
		if ((int)::AGS::g_vm->getRandomNumber(9) > 7 - windspeed) { //Wind right
			for (int x = w - 1; x > 1; x--) {
				fire[y][x] = fire[y][x - 1];
			}
		} else if ((int)::AGS::g_vm->getRandomNumber(9) > 7 + windspeed) { // wind left
			for (int x = 0; x < w - 1; x++) {
				fire[y][x] = fire[y][x + 1];
			}
		}
	}
	for (int x = 0; x < w; x++) {
		sparky = ABS((int)::AGS::g_vm->getRandomNumber(0x7fffffff) % (h - 2));
		if (sparky < h && sparky > 0 && fire[h - sparky][x] > cutoff &&
				ABS((int)::AGS::g_vm->getRandomNumber(0x7fffffff) % 10) > 7)
			fire[h - sparky][x] = 255;
		sparky = ABS((int)::AGS::g_vm->getRandomNumber(0x7fffffff) % (h - 2));
		if (sparky < h && sparky > 0 && fire[h - sparky][x] > cutoff &&
				ABS((int)::AGS::g_vm->getRandomNumber(0x7fffffff) % 10) > 7)
			fire[h - sparky][x] = 0;
	}
	if (seed == 0) {
		for (int x = 0; x < w; x++)
			fire[h - 1][x] = 255;
		for (int x = 0; x < w; x++)
			fire[h - 2][x] = ::AGS::g_vm->getRandomNumber(255);
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

	for (int y = 0; y < h - 1; y++) {
		for (int x = 0; x < w; x++) {
			fire[y][x] =
			    ((fire[(y + 1) % h][(x - 1 + w) % w]
			      + fire[(y + 1) % h][(x) % w]
			      + fire[(y + 1) % h][(x + 1) % w]
			      + fire[(y + 2) % h][(x) % w])
			     * 100) / (400 + (100 - strength));
			if (fire[y][x] < cutoff) fire[y][x] = 0;
			//if (fire[y][x] ==255) color [y][x] = palend;
			else color [y][x] = static_cast<unsigned char>(basecol + (fire[y][x] / divider) * dir);
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
    //unsigned char **clutarray = engine->GetRawBitmapSurface (clutspr);
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
    //unsigned char **clutarray = engine->GetRawBitmapSurface (clutspr);
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
unsigned char GetColor565(unsigned char r, unsigned char g, unsigned char b) {
	//BITMAP *clutspr = engine->GetSpriteGraphic (clutslot);
	//if (!clutspr) engine->AbortGame ("MixColorAlpha: Can't load CLUT sprite into memory.");
	//unsigned char **clutarray = engine->GetRawBitmapSurface (clutspr);
	int i = ((r << 11) | (g << 5) | b);
	unsigned char (*clutp) = clut;
	unsigned char result = *(clutp + i);
	result = cycle_remap [result]; //Once again, to make sure that the palette slot used is the right one.
	//engine->ReleaseBitmapSurface (clutspr);
	return result;
}

void CycleRemap(int start, int end) {
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

unsigned char GetRemappedSlot(unsigned char slot) {
	return cycle_remap [slot];
}

int LoadCLUT(int slot) {
	if (engine->GetSpriteWidth(slot) != 256 || engine->GetSpriteHeight(slot) != 256) return 1;
	BITMAP *clutimage = engine->GetSpriteGraphic(slot);
	unsigned char **clutarray = engine->GetRawBitmapSurface(clutimage);
	for (int y = 0; y < 256; y++) {
		for (int x = 0; x < 256; x++) {
			clut[y * 256 + x] = clutarray[y][x];
		}
	}
	clutslot = slot;
	engine->ReleaseBitmapSurface(clutimage);
	return 0;
}

void SetReflections(int toggle) {
	drawreflections = toggle;
}

int IsReflectionsOn() {
	return drawreflections;
}

int GetLuminosityFromPalette(int slot) {
	AGSColor *pal = engine->GetPalette();
	int lum = (pal[slot].r +
	           pal[slot].r +
	           pal[slot].r +
	           pal[slot].g +
	           pal[slot].g +
	           pal[slot].g +
	           pal[slot].g +
	           pal[slot].b) >> 3;
	return lum;
}



void SetStarsOriginPoint(int x, int y) {
	Starfield.originx = x;
	Starfield.originy = y;
}
void InitializeStars(int slot, int maxstars) {
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

void IterateStars(int slot) {
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
			if (stars[i].y < 1.0 && stars[i].y > 1.0) stars[i].y = (float)sh;
			stars[i].z = (float)MAX_DEPTH;
			//stars[i].color = (rand () %240);
		}
	}
}
int GetStarfieldOverscan() {
	return Starfield.overscan;
}
void SetStarfieldOverscan(int overscan) {
	Starfield.overscan = overscan;
}

int GetStarfieldOriginX() {
	return Starfield.originx;
}

int GetStarfieldOriginY() {
	return Starfield.originy;
}

void SetStarfieldDepthMultiplier(int multi) {
	Starfield.depthmultiplier = multi;
}

int GetStarfieldDepthMultiplier() {
	return Starfield.depthmultiplier;
}

int GetStarfieldMaxStars() {
	return Starfield.maxstars;
}

void SetStarSpriteScaleBoost(int star, int boost) {
	stars[star].scaleboost = boost;
}

int GetStarSpriteScaleBoost(int star) {
	return stars[star].scaleboost;
}

void SetStarMaxRadius(int star, int radius) {
	stars[star].maxrad = radius;
}

int GetStarMaxRadius(int star) {
	return stars[star].maxrad;
}

void RotateStar(int star, int angle, int px, int py) {
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

FLOAT_RETURN_TYPE GetStarX(int i) {
	float starx = (float)stars[i].x;
	RETURN_FLOAT(starx);
}

FLOAT_RETURN_TYPE GetStarY(int i) {
	float stary = (float)stars[i].y;
	RETURN_FLOAT(stary);
}

FLOAT_RETURN_TYPE GetStarZ(int i) {
	float starz = (float)stars[i].z;
	RETURN_FLOAT(starz);
}

void SetStarPosition(int star, SCRIPT_FLOAT(x), SCRIPT_FLOAT(y), SCRIPT_FLOAT(z)) {
	INIT_SCRIPT_FLOAT(x);
	INIT_SCRIPT_FLOAT(y);
	INIT_SCRIPT_FLOAT(z);
	stars[star].x = x;
	stars[star].y = y;
	stars[star].z = z;
}

void SetStarColor(int star, unsigned char color) {
	stars[star].color = color;
}

unsigned char GetStarColor(int star) {
	return stars[star].color;
}

void SetStarSprite(int star, int slot) {
	stars[star].sprite = slot;
}

int GetStarSprite(int star) {
	return stars[star].sprite;
}

void SetStarSpriteRange(int start, int end, int slot) {
	int sfix = start;
	int efix = end;
	if (start > Starfield.maxstars) sfix = Starfield.maxstars - 1;
	if (end > Starfield.maxstars) efix = Starfield.maxstars;
	for (int i = sfix; i < efix; i++)
		stars[i].sprite = slot;
}

void DrawStars(int slot, int maskslot) {
	int32 sw, sh = 0;
	BITMAP *canvas = engine->GetSpriteGraphic(slot);
	if (!canvas) engine->AbortGame("DrawStars: Can't load sprite slot.");
	BITMAP *maskcanvas = engine->GetSpriteGraphic(maskslot);
	if (!maskcanvas) engine->AbortGame("DrawStars: Can't load mask slot.");
	engine->GetBitmapDimensions(canvas, &sw, &sh, nullptr);
	unsigned char **screenarray = engine->GetRawBitmapSurface(canvas);
	unsigned char **maskarray = engine->GetRawBitmapSurface(maskcanvas);
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
			if (stars[i].y < 1.0 && stars[i].y > 1.0) stars[i].y = (float)sh;
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
				unsigned char** orig = engine->GetRawBitmapSurface (origspr);
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
				unsigned char** resized = engine->GetRawBitmapSurface (resizspr);
				int x_ratio = (int)((w1<<16)/w2) +1;
				int y_ratio = (int)((h1<<16)/h2) +1;
				int x2, y2 ;
				for (int i=0;i<h2;i++)
				{
				    for (int j=0;j<w2;j++)
				   {
				       x2 = ((j*x_ratio)>>16) ;
				       y2 = ((i*y_ratio)>>16) ;
				       resized [i][j] = orig [y2][x2];
				 }
				}
				engine->ReleaseBitmapSurface (resizspr);
				}
				//resizspr = origspr;
				int32 w,h=0;
				engine->GetBitmapDimensions (resizspr,&w,&h,NULL);
				unsigned char **imagemap = engine->GetRawBitmapSurface (resizspr);
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
				       if (maskcolor > maskarray [ey][ex] && imagemap[dy][dx] > 0)
				       {
				           maskarray [ey][ex] = maskcolor;
				           screenarray [ey][ex] = imagemap[dy][dx];
				       }
				   }
				}
				}
				*/

				unsigned char **orig = engine->GetRawBitmapSurface(origspr);
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
							if (maskcolor > maskarray [ey][ex] && orig[y2][x2] > 0) {
								maskarray [ey][ex] = maskcolor;
								screenarray [ey][ex] = orig[y2][x2];
							}
						}
						//resized [ii][j] = orig [y2][x2];
					}
				}
				engine->ReleaseBitmapSurface(origspr);
			} else if (stars[i].sprite == 0) {
				if (stars[i].maxrad == 1) {
					if (px < sw && px >= 0 && py < sh && py >= 0) {
						if (maskcolor > maskarray[py][px]) {
							maskarray[py][px] = maskcolor;
							screenarray[py][px] = stars[i].color;
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
								if (maskcolor > maskarray [dy][dx]) {
									maskarray [dy][dx] = maskcolor;
									screenarray [dy][dx] = color;
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
					           if (maskarray [dy][dx] == maskcolor)screenarray [dy][dx] = color;
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


int CreateTranslucentOverlay(int id, int spriteId, int alpha, int level, int ox, int oy, int mask = 0, int blendmode = 0) {
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
	return 0;
}

int DeleteTranslucentOverlay(int id) {
	overlay[id].enabled = false;
	overlay[id].sprite = 0;
	overlay[id].x = 0;
	overlay[id].y = 0;
	overlay[id].level = 0;
	overlay[id].trans = 0;
	return 0;
}

int MoveTranslucentOverlay(int id, int ox, int oy) {
	overlay[id].x = ox;
	overlay[id].y = oy;
	return 0;
}

int GetTranslucentOverlayX(int id) {
	return overlay[id].x;
}

int GetTranslucentOverlayY(int id) {
	return overlay[id].y;
}

int GetTranslucentOverlaySprite(int id) {
	return overlay[id].sprite;
}

int GetTranslucentOverlayLevel(int id) {
	return overlay[id].level;
}

int GetTranslucentOverlayEnabled(int id) {
	return overlay[id].enabled;
}

int GetTranslucentOverlayAlpha(int id) {
	return overlay[id].trans;
}

int SetTranslucentOverlayAlpha(int id, int alpha) {
	if (alpha >= 0 && alpha < 256) overlay[id].trans = alpha;
	else engine->AbortGame("CreateTranslucentOverlay: Invalid alpha selected.");
	return 0;
}

int SetTranslucentOverlayEnabled(int id, int toggle) {
	if (toggle > 0) overlay[id].enabled = true;
	else overlay[id].enabled = false;
	return 0;
}

void SetCharacterReflected(int id, int refl) {
	if (refl > 0) Reflection.Characters[id].reflect = 1;
	else Reflection.Characters[id].reflect = 0;
}

void SetObjectReflected(int id, int refl) {
	if (refl > 0) Reflection.Objects[id].reflect = 1;
	else Reflection.Objects[id].reflect = 0;
}

int GetCharacterReflected(int id) {
	return Reflection.Characters[id].reflect;
}

void GetObjectReflected(int id) {
	Reflection.Objects[id].reflect;
}

void ReplaceCharacterReflectionView(int id, int view) {
	Reflection.Characters[id].replaceview = view - 1;
}

void SetObjectReflectionIgnoreScaling(int id, int wb) {
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
	unsigned char **wbarray = engine->GetRawBitmapSurface(walkbehind);
	unsigned char **maskarray = engine->GetRawBitmapSurface(bgmask);
	//Initialize stuff
	BITMAP *charsprite = nullptr;
	BITMAP *charsprite2 = nullptr;
	AGSCharacter *currchar;
	AGSObject *currobj;
	int cox, coy, coz = 0;
	int scale = 0;
	//Get character, and their sprite.
	if (charobj == 0) {
		currchar = engine->GetCharacter(id);
		int view = 0;
		if (Reflection.Characters[id].replaceview == 0) view = currchar->view + 1;
		else view = Reflection.Characters[id].replaceview;
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
		unsigned char **orig = engine->GetRawBitmapSurface(charsprite);
		int h1, h2, w1, w2;
		double fw2, fh2;
		h1 = h;
		w1 = w;
		fh2 = h1 * ((double)scale / 100.0);
		fw2 = w1 * ((double)scale / 100.0);
		h2 = static_cast<int>(fh2);
		w2 = static_cast<int>(fw2);
		charsprite2 = engine->CreateBlankBitmap(w2, h2, 8);
		unsigned char **resized = engine->GetRawBitmapSurface(charsprite2);
		int x_ratio = (int)((w1 << 16) / w2) + 1;
		int y_ratio = (int)((h1 << 16) / h2) + 1;
		int x2, y2 ;
		for (int i = 0; i < h2; i++) {
			for (int j = 0; j < w2; j++) {
				x2 = ((j * x_ratio) >> 16) ;
				y2 = ((i * y_ratio) >> 16) ;
				resized [i][j] = orig [y2][x2];
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
	unsigned char **spritearray = engine->GetRawBitmapSurface(charsprite2);
	unsigned char **charbuffer = engine->GetRawBitmapSurface(rcolormap);
	unsigned char **alphaarray = engine->GetRawBitmapSurface(ralphamap);
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
		int (*sfGetGameParameter)(int, int, int, int);
		sfGetGameParameter = ((int(*)(int, int, int, int)) engine->GetScriptFunctionAddress("GetGameParameter"));
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
				if (wbarray [ry][rx] > 0) {
					wbb = engine->GetWalkbehindBaseline(wbarray[ry][rx]);
				}
				if (maskarray[ry][rx] == 21) obst[j] = 1;
			}

			//dither = (!dither);
			transamount = 32 * translevel;
			if (spritearray [i][j] != 0 && oy + yoffset < screenh && ox + xoffset < screenw && oy + yoffset >= 0 && ox + xoffset >= 0) { // If the sprite isn't transparent, and isn't drawn off the edge of the bg.
				if (wbb < ry && obst[j] == 0 && (oy > reflectionmap[(ox + xoffset) + (screenw * (oy + yoffset))])) {
					//charbuffer[oy+yoffset][ox+xoffset] = MixColorAlpha (spritearray [i][j],charbuffer[oy+yoffset][ox+xoffset],transamount);
					charbuffer [oy + yoffset][ox + xoffset] = spritearray [i][j];
					alphaarray [oy + yoffset][ox + xoffset] = transamount;
					reflectionmap[(ox + xoffset) + (screenw * (oy + yoffset))] = oy;
				}
			}
			j++;
		}
		//if (w % 2 == 0) dither = (!dither);
		i--;
		j = 0;
	}

	delete [] obst;
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


int DrawTransSprite(int spriteId, int bg, int translevel, int mask = 0, int blendmode = 0, int use_objpal = 0) {
	BITMAP *maskspr = nullptr;
	unsigned char **maskarray;
	if (mask > 0) maskspr = engine->GetSpriteGraphic(mask);
	if (!maskspr && mask > 0) {
		char maskerr [100];
		int cx;
		cx = snprintf(maskerr, 100, "DrawTransSprite: Can't load mask from slot %d.", mask);
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

	unsigned char **bgarray = engine->GetRawBitmapSurface(bgspr);
	//unsigned char **clutarray = engine->GetRawBitmapSurface (clutspr);
	unsigned char **spritearray = engine->GetRawBitmapSurface(spritespr);
	if (mask > 0) maskarray = engine->GetRawBitmapSurface(maskspr);
	int tloffset = 255 - translevel;
	int x = 0;
	int y = 0;
	//int transamount = 256 * translevel; //old
	while (y < sprh) {
		while (x < sprw) {
			if (spritearray [y][x] != 0 && y < bgh && x < bgw && y >= 0 && x >= 0) { // If the spriteId isn't transparent, and isn't drawn off the edge of the bg.
				if (mask > 0) {
					translevel = MAX(maskarray [y][x] - tloffset, 0);
				}
				//spritearray[y][x] = cycle_remap[clutarray [cycle_remap[bgarray[y][x]]+transamount][cycle_remap[spritearray [y][x]]]]; //old
				if (blendmode == 0) spritearray[y][x] = Mix::MixColorAlpha(spritearray [y][x], bgarray[y][x], translevel, use_objpal);
				else if (blendmode == 1) spritearray[y][x] = Mix::MixColorAdditive(spritearray [y][x], bgarray[y][x], translevel, use_objpal);
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
	return 0;
}

int DrawTranslucentOverlay(int spriteId, int translevel, int ox, int oy, int mask = 0, int blendmode = 0) {
	if (translevel == 0) return 0;
	BITMAP *maskspr;
	unsigned char **maskarray;
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
	unsigned char **charbuffer = engine->GetRawBitmapSurface(virtsc);
	unsigned char **spritearray = engine->GetRawBitmapSurface(spritespr);
	if (mask > 0) {
		if (!maskspr && mask > 0) {
			char maskerr [100];
			snprintf(maskerr, 100, "DrawTransSprite: Can't load mask from slot %d.", mask);
			engine->AbortGame(maskerr);
		}
		maskarray = engine->GetRawBitmapSurface(maskspr);
	}
	int tloffset = 255 - translevel;
	int x = 0;
	int y = 0;
	//int transamount = 256 * translevel; //old
	while (y < sprh) {
		while (x < sprw) {
			if (spritearray [y][x] != 0 && y + oy < screenh && x + ox < screenw && y + oy >= 0 && x + ox >= 0) { // If the spriteId isn't transparent, and isn't drawn off the edge of the screen.
				//charbuffer[y+oy][x+ox] = cycle_remap[clutarray [cycle_remap[charbuffer[y+oy][x+ox]]+transamount][cycle_remap[spritearray [y][x]]]]; //old
				if (mask > 0) {
					translevel = MAX(maskarray [y][x] - tloffset, 0);
				}
				if (blendmode == 0) {
					if (translevel == 255) {
						charbuffer[y + oy][x + ox] = spritearray [y][x];
					} else charbuffer[y + oy][x + ox] = Mix::MixColorAlpha(spritearray [y][x], charbuffer[y + oy][x + ox], translevel);
				} else if (blendmode == 1) charbuffer[y + oy][x + ox] = Mix::MixColorAdditive(spritearray [y][x], charbuffer[y + oy][x + ox], translevel);
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

AGSPalRender::AGSPalRender() : DLL() {
	DLL_METHOD(AGS_GetPluginName);
	DLL_METHOD(AGS_EngineStartup);
	DLL_METHOD(AGS_EngineShutdown);
	DLL_METHOD(AGS_EngineOnEvent);
}

const char *AGSPalRender::AGS_GetPluginName() {
	return "PALgorithms Translucent Overlay Renderer";
}

void AGSPalRender::AGS_EngineStartup(IAGSEngine *lpEngine) {
	engine = lpEngine;

	// Make sure it's got the version with the features we need
	if (engine->version < 3) {
		engine->AbortGame("Engine interface is too old, need newer version of AGS.");
	}
	engine->RegisterScriptFunction("PALInternal::LoadCLUT^1", (void *)LoadCLUT);
	engine->RegisterScriptFunction("PALInternal::CycleRemap^2", (void *)CycleRemap);
	engine->RegisterScriptFunction("PALInternal::GetColor565^3", (void *)GetColor565);
	engine->RegisterScriptFunction("PALInternal::GetLuminosityFromPalette^1", (void *)GetLuminosityFromPalette);
	engine->RegisterScriptFunction("PALInternal::FastSin^1", (void *)AGSFastSin);
	engine->RegisterScriptFunction("PALInternal::FastCos^1", (void *)AGSFastCos);
	engine->RegisterScriptFunction("PALInternal::FastRoot^1", (void *)root);
	engine->RegisterScriptFunction("PALInternal::GetRemappedSlot^1", (void *)GetRemappedSlot);
	engine->RegisterScriptFunction("PALInternal::ResetRemapping^0", (void *)ResetRemapping);
	engine->RegisterScriptFunction("PALInternal::GetModifiedBackgroundImage", (void *)GetModifiedBackgroundImage);
	engine->RegisterScriptFunction("PALInternal::WriteObjectivePalette^4", (void *)WriteObjectivePalette);
	engine->RegisterScriptFunction("PALInternal::ReadObjectivePaletteR^1", (void *)ReadObjectivePaletteR);
	engine->RegisterScriptFunction("PALInternal::ReadObjectivePaletteB^1", (void *)ReadObjectivePaletteB);
	engine->RegisterScriptFunction("PALInternal::ReadObjectivePaletteG^1", (void *)ReadObjectivePaletteG);

	engine->RegisterScriptFunction("Raycast::Render^1", (void *)Raycast_Render);
	engine->RegisterScriptFunction("Raycast::LoadMap^4", (void *)LoadMap);
	engine->RegisterScriptFunction("Raycast::Initialize", (void *)Init_Raycaster);
	engine->RegisterScriptFunction("Raycast::MakeTextures^1", (void *)MakeTextures);
	engine->RegisterScriptFunction("Raycast::MoveForward^0", (void *)MoveForward);
	engine->RegisterScriptFunction("Raycast::MoveBackward^0", (void *)MoveBackward);
	engine->RegisterScriptFunction("Raycast::RotateLeft^0", (void *)RotateLeft);
	engine->RegisterScriptFunction("Raycast::RotateRight^0", (void *)RotateRight);
	engine->RegisterScriptFunction("Raycast::SetCameraPosition^2", (void *)Ray_SetPlayerPosition);
	engine->RegisterScriptFunction("Raycast::GetCameraX^0", (void *)Ray_GetPlayerX);
	engine->RegisterScriptFunction("Raycast::GetCameraY^0", (void *)Ray_GetPlayerY);
	engine->RegisterScriptFunction("Raycast::GetCameraAngle^0", (void *)Ray_GetPlayerAngle);
	engine->RegisterScriptFunction("Raycast::SetCameraAngle^1", (void *)Ray_SetPlayerAngle);
	engine->RegisterScriptFunction("Raycast::InitSprite^9", (void *)Ray_InitSprite);
	engine->RegisterScriptFunction("Raycast::UnloadEngine^0", (void *)QuitCleanup);
	engine->RegisterScriptFunction("Raycast::GetHotspotAtXY^2", (void *)Ray_GetHotspotAt);
	engine->RegisterScriptFunction("Raycast::GetObjectAtXY^2", (void *)Ray_GetObjectAt);
	engine->RegisterScriptFunction("Raycast::SetSpriteInteractObj^2", (void *)Ray_SetSpriteInteractObj);
	engine->RegisterScriptFunction("Raycast::GetSpriteInteractObj^1", (void *)Ray_GetSpriteInteractObj);
	engine->RegisterScriptFunction("Raycast::SetSpritePosition^3", (void *)Ray_SetSpritePosition);
	engine->RegisterScriptFunction("Raycast::SetSpriteVertOffset^2", (void *)Ray_SetSpriteVertOffset);
	engine->RegisterScriptFunction("Raycast::GetSpriteVertOffset^1", (void *)Ray_GetSpriteVertOffset);
	engine->RegisterScriptFunction("Raycast::GetSpriteX^1", (void *)Ray_GetSpriteX);
	engine->RegisterScriptFunction("Raycast::GetSpriteY^1", (void *)Ray_GetSpriteY);
	engine->RegisterScriptFunction("Raycast::SetWallHotspot^2", (void *)Ray_SetWallHotspot);
	engine->RegisterScriptFunction("Raycast::SetWallTextures^5", (void *)Ray_SetWallTextures);
	engine->RegisterScriptFunction("Raycast::SetWallSolid^5", (void *)Ray_SetWallSolid);
	engine->RegisterScriptFunction("Raycast::SetWallIgnoreLighting^5", (void *)Ray_SetWallIgnoreLighting);
	engine->RegisterScriptFunction("Raycast::SetWallAlpha^5", (void *)Ray_SetWallAlpha);
	engine->RegisterScriptFunction("Raycast::SetWallBlendType^5", (void *)Ray_SetWallBlendType);
	engine->RegisterScriptFunction("Raycast::GetMoveSpeed^0", (void *)Ray_GetMoveSpeed);
	engine->RegisterScriptFunction("Raycast::SetMoveSpeed^1", (void *)Ray_SetMoveSpeed);
	engine->RegisterScriptFunction("Raycast::GetRotSpeed^0", (void *)Ray_GetRotSpeed);
	engine->RegisterScriptFunction("Raycast::SetRotSpeed^1", (void *)Ray_SetRotSpeed);
	engine->RegisterScriptFunction("Raycast::GetWallAt^2", (void *)Ray_GetWallAt);
	engine->RegisterScriptFunction("Raycast::GetLightAt^2", (void *)Ray_GetLightAt);
	engine->RegisterScriptFunction("Raycast::SetLightAt^3", (void *)Ray_SetLightAt);
	engine->RegisterScriptFunction("Raycast::SetWallAt^3", (void *)Ray_SetWallAt);
	engine->RegisterScriptFunction("Raycast::SetPlaneY^1", (void *)Ray_SetPlaneY);
	engine->RegisterScriptFunction("Raycast::GetDistanceAt^2", (void *)Ray_GetDistanceAt);
	engine->RegisterScriptFunction("Raycast::GetSpriteAngle^1", (void *)Ray_GetSpriteAngle);
	engine->RegisterScriptFunction("Raycast::SetSpriteAngle^2", (void *)Ray_SetSpriteAngle);
	engine->RegisterScriptFunction("Raycast::SetSpriteView^2", (void *)Ray_SetSpriteView);
	engine->RegisterScriptFunction("Raycast::GetSpriteView^1", (void *)Ray_GetSpriteView);
	engine->RegisterScriptFunction("Raycast::SetSpriteFrame^2", (void *)Ray_SetSpriteFrame);
	engine->RegisterScriptFunction("Raycast::GetSpriteFrame^1", (void *)Ray_GetSpriteFrame);
	engine->RegisterScriptFunction("Raycast::SetSpritePic^2", (void *)Ray_SetSpritePic);
	engine->RegisterScriptFunction("Raycast::GetSpritePic^1", (void *)Ray_GetSpritePic);
	engine->RegisterScriptFunction("Raycast::SetSkyBox^1", (void *)Ray_SetSkyBox);
	engine->RegisterScriptFunction("Raycast::SetSpriteAlpha^2", (void *)Ray_SetSpriteAlpha);
	engine->RegisterScriptFunction("Raycast::GetSpriteAlpha^1", (void *)Ray_GetSpriteAlpha);
	engine->RegisterScriptFunction("Raycast::GetSkyBox^1", (void *)Ray_GetSkyBox);
	engine->RegisterScriptFunction("Raycast::SetAmbientLight^1", (void *)Ray_SetAmbientLight);
	engine->RegisterScriptFunction("Raycast::SetAmbientColor^2", (void *)Ray_SetAmbientColor);
	engine->RegisterScriptFunction("Raycast::GetAmbientLight^0", (void *)Ray_GetAmbientLight);
	engine->RegisterScriptFunction("Raycast::GetAmbientWeight^0", (void *)Ray_GetAmbientWeight);
	engine->RegisterScriptFunction("Raycast::GetTileX_At^2", (void *)Ray_GetTileX_At);
	engine->RegisterScriptFunction("Raycast::GetTileY_At^2", (void *)Ray_GetTileY_At);
	engine->RegisterScriptFunction("Raycast::DrawTile^2", (void *)Ray_DrawTile);
	engine->RegisterScriptFunction("Raycast::DrawOntoTile^2", (void *)Ray_DrawOntoTile);
	engine->RegisterScriptFunction("Raycast::SetNoClip^1", (void *)Ray_SetNoClip);
	engine->RegisterScriptFunction("Raycast::GetNoClip^0", (void *)Ray_GetNoClip);
	engine->RegisterScriptFunction("Raycast::GetSpriteScaleX^1", (void *)Ray_GetSpriteScaleX);
	engine->RegisterScriptFunction("Raycast::SetSpriteScaleX^2", (void *)Ray_SetSpriteScaleX);
	engine->RegisterScriptFunction("Raycast::GetSpriteScaleY^1", (void *)Ray_GetSpriteScaleY);
	engine->RegisterScriptFunction("Raycast::SetSpriteScaleY^2", (void *)Ray_SetSpriteScaleY);
	engine->RegisterScriptFunction("Raycast::GetSpriteBlendType^1", (void *)Ray_GetSpriteBlendType);
	engine->RegisterScriptFunction("Raycast::SetSpriteBlendType^2", (void *)Ray_SetSpriteBlendType);


	engine->RegisterScriptFunction("Raycast::SetFloorAt^3", (void *)Ray_SetFloorAt);
	engine->RegisterScriptFunction("Raycast::SetCeilingAt^3", (void *)Ray_SetCeilingAt);
	engine->RegisterScriptFunction("Raycast::GetCeilingAt^2", (void *)Ray_GetCeilingAt);
	engine->RegisterScriptFunction("Raycast::GetFloorAt^2", (void *)Ray_GetFloorAt);
	engine->RegisterScriptFunction("Raycast::GetLightingAt^2", (void *)Ray_GetLightingAt);
	engine->RegisterScriptFunction("Raycast::SetLightingAt^3", (void *)Ray_SetLightingAt);
	engine->RegisterScriptFunction("Raycast::GetWallHotspot^1", (void *)Ray_GetWallHotspot);
	engine->RegisterScriptFunction("Raycast::GetWallTexture^2", (void *)Ray_GetWallTexture);
	engine->RegisterScriptFunction("Raycast::GetWallSolid^2", (void *)Ray_GetWallSolid);
	engine->RegisterScriptFunction("Raycast::GetWallIgnoreLighting^2", (void *)Ray_GetWallIgnoreLighting);
	engine->RegisterScriptFunction("Raycast::GetWallAlpha^2", (void *)Ray_GetWallAlpha);
	engine->RegisterScriptFunction("Raycast::GetWallBlendType^2", (void *)Ray_GetWallBlendType);
	engine->RegisterScriptFunction("Raycast::SelectTile^3", (void *)Ray_SelectTile);
	engine->RegisterScriptFunction("Raycast::HasSeenTile^2", (void *)Ray_HasSeenTile);

	engine->RegisterScriptFunction("LensDistort::SetPos^2", (void *)SetLensPos);
	engine->RegisterScriptFunction("LensDistort::GetX^0", (void *)GetLensX);
	engine->RegisterScriptFunction("LensDistort::GetY^0", (void *)GetLensY);
	engine->RegisterScriptFunction("LensDistort::Set^1", (void *)SetLensDrawn);
	engine->RegisterScriptFunction("LensDistort::IsDrawn^0", (void *)GetLensDrawn);
	engine->RegisterScriptFunction("LensDistort::SetOffsetClamp^1", (void *)SetLensOffsetClamp);
	engine->RegisterScriptFunction("LensDistort::GetOffsetClamp^0", (void *)GetLensOffsetClamp);
	engine->RegisterScriptFunction("LensDistort::GetLevel^0", (void *)GetLensLevel);
	engine->RegisterScriptFunction("LensDistort::SetLevel^1", (void *)GetLensLevel);
	engine->RegisterScriptFunction("LensDistort::Initialize^6", (void *)LensInitialize);

	engine->RegisterScriptFunction("Translucence::CreateOverlay^8", (void *)CreateTranslucentOverlay);
	engine->RegisterScriptFunction("Translucence::DeleteOverlay^1", (void *)DeleteTranslucentOverlay);
	engine->RegisterScriptFunction("Translucence::Move^3", (void *)MoveTranslucentOverlay);
	engine->RegisterScriptFunction("Translucence::GetOverlayX^1", (void *)GetTranslucentOverlayX);
	engine->RegisterScriptFunction("Translucence::GetOverlayY^1", (void *)GetTranslucentOverlayY);
	engine->RegisterScriptFunction("Translucence::GetOverlaySprite^1", (void *)GetTranslucentOverlaySprite);
	engine->RegisterScriptFunction("Translucence::GetOverlayLevel^1", (void *)GetTranslucentOverlayLevel);
	engine->RegisterScriptFunction("Translucence::GetOverlayEnabled^1", (void *)GetTranslucentOverlayEnabled);
	engine->RegisterScriptFunction("Translucence::GetOverlayAlpha^1", (void *)GetTranslucentOverlayAlpha);
	engine->RegisterScriptFunction("Translucence::SetOverlayAlpha^2", (void *)SetTranslucentOverlayAlpha);
	engine->RegisterScriptFunction("Translucence::SetOverlayEnabled^2", (void *)SetTranslucentOverlayEnabled);
	engine->RegisterScriptFunction("Translucence::DrawTransSprite^6", (void *)DrawTransSprite);

	engine->RegisterScriptFunction("Starfield::GetOverscan^0", (void *)GetStarfieldOverscan);
	engine->RegisterScriptFunction("Starfield::SetOverscan^1", (void *)SetStarfieldOverscan);
	engine->RegisterScriptFunction("Starfield::GetOriginX^0", (void *)GetStarfieldOriginX);
	engine->RegisterScriptFunction("Starfield::GetOriginY^0", (void *)GetStarfieldOriginY);
	engine->RegisterScriptFunction("Starfield::SetDepthMultiplier^1", (void *)SetStarfieldDepthMultiplier);
	engine->RegisterScriptFunction("Starfield::GetDepthMultiplier^0", (void *)GetStarfieldDepthMultiplier);
	engine->RegisterScriptFunction("Starfield::GetMaxStars^0", (void *)GetStarfieldMaxStars);
	engine->RegisterScriptFunction("Starfield::SetStarSpriteScaleBoost^1", (void *)SetStarSpriteScaleBoost);
	engine->RegisterScriptFunction("Starfield::GetStarSpriteScaleBoost^0", (void *)GetStarSpriteScaleBoost);
	engine->RegisterScriptFunction("Starfield::SetStarMaxRadius^2", (void *)SetStarMaxRadius);
	engine->RegisterScriptFunction("Starfield::GetStarMaxRadius^0", (void *)GetStarMaxRadius);
	engine->RegisterScriptFunction("Starfield::GetStarX^1", (void *)GetStarX);
	engine->RegisterScriptFunction("Starfield::GetStarY^1", (void *)GetStarY);
	engine->RegisterScriptFunction("Starfield::GetStarZ^1", (void *)GetStarZ);
	engine->RegisterScriptFunction("Starfield::SetStarPosition^4", (void *)SetStarPosition);
	engine->RegisterScriptFunction("Starfield::RotateStar^4", (void *)RotateStar);
	engine->RegisterScriptFunction("Starfield::SetStarColor^2", (void *)SetStarColor);
	engine->RegisterScriptFunction("Starfield::GetStarColor^1", (void *)GetStarColor);
	engine->RegisterScriptFunction("Starfield::SetStarSprite^2", (void *)SetStarSprite);
	engine->RegisterScriptFunction("Starfield::GetStarSprite^1", (void *)GetStarSprite);
	engine->RegisterScriptFunction("Starfield::SetStarSpriteRange^3", (void *)SetStarSpriteRange);
	engine->RegisterScriptFunction("Starfield::Initialize^2", (void *)InitializeStars);
	engine->RegisterScriptFunction("Starfield::Iterate^1", (void *)IterateStars);
	engine->RegisterScriptFunction("Starfield::Draw^2", (void *)DrawStars);
	engine->RegisterScriptFunction("Starfield::SetOriginPoint^2", (void *)SetStarsOriginPoint);

	engine->RegisterScriptFunction("Plasma::DoFire^8", (void *)DoFire);
	engine->RegisterScriptFunction("Plasma::SetPlasmaType^5", (void *)SetPlasmaType);
	engine->RegisterScriptFunction("Plasma::ResetPlasmaSettings^0", (void *)ResetPlasmaSettings);
	engine->RegisterScriptFunction("Plasma::DrawPlasma^3", (void *)DrawPlasma);
	engine->RegisterScriptFunction("Plasma::SetRootType^1", (void *)SetPlasmaRootType);
	engine->RegisterScriptFunction("Plasma::GetRootType^0", (void *)GetPlasmaRootType);

	engine->RegisterScriptFunction("Reflections::Set^1", (void *)SetReflections);
	engine->RegisterScriptFunction("Reflections::IsReflecting^0", (void *)IsReflectionsOn);
	engine->RegisterScriptFunction("Reflections::SetCharacterReflected^2", (void *)SetCharacterReflected);
	engine->RegisterScriptFunction("Reflections::GetCharacterReflected^1", (void *)GetCharacterReflected);
	engine->RegisterScriptFunction("Reflections::SetObjectReflected^2", (void *)SetObjectReflected);
	engine->RegisterScriptFunction("Reflections::GetObjectReflected^1", (void *)GetObjectReflected);
	engine->RegisterScriptFunction("Reflections::ReplaceCharacterReflectionView^2", (void *)ReplaceCharacterReflectionView);
	engine->RegisterScriptFunction("Reflections::SetObjectReflectionIgnoreScaling^2", (void *)SetObjectReflectionIgnoreScaling);
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
	Init_Raycaster();
}

void AGSPalRender::AGS_EngineShutdown() {
	// no work to do here - but if we had created any dynamic sprites,
	// we should delete them here
	delete[] Reflection.Characters;
	delete[] Reflection.Objects;
	//QuitCleanup();
}

int AGSPalRender::AGS_EngineOnEvent(int event, int data) {
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
			unsigned char **screenbuffer = engine->GetRawBitmapSurface(virtsc);
			unsigned char **colorbuffer = engine->GetRawBitmapSurface(rcolormap);
			unsigned char **alphabuffer = engine->GetRawBitmapSurface(ralphamap);
			for (int y = 0; y < sh; y++)
				for (int x = 0; x < sw; x++)
					screenbuffer[y][x] = Mix::MixColorAlpha(colorbuffer[y][x], screenbuffer[y][x], alphabuffer[y][x]);
			engine->ReleaseBitmapSurface(rcolormap);
			engine->ReleaseBitmapSurface(ralphamap);
			engine->ReleaseBitmapSurface(virtsc);
			engine->FreeBitmap(rcolormap);
			engine->FreeBitmap(ralphamap);
			delete [] reflectionmap;
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
		for (int i = 0; i < MAX_OVERLAYS; ++i) {
			engine->FWrite(&overlay[i].sprite, sizeof(int), data);
			engine->FWrite(&overlay[i].spritemask, sizeof(int), data);
			engine->FWrite(&overlay[i].x, sizeof(int), data);
			engine->FWrite(&overlay[i].y, sizeof(int), data);
			engine->FWrite(&overlay[i].level, sizeof(int), data);
			engine->FWrite(&overlay[i].trans, sizeof(int), data);
			engine->FWrite(&overlay[i].blendtype, sizeof(int), data);
			engine->FWrite(&overlay[i].enabled, sizeof(bool), data);
		}
		engine->FWrite(&clutslot, sizeof(int), data);
		engine->FWrite(&drawreflections, sizeof(int), data);
		for (int j = 0; j < 256; ++j) {
			engine->FWrite(&cycle_remap[j], sizeof(unsigned char), data);
		}
		for (int j = 0; j < 256; ++j) {
			engine->FWrite(&objectivepal[j].r, sizeof(unsigned char), data);
			engine->FWrite(&objectivepal[j].b, sizeof(unsigned char), data);
			engine->FWrite(&objectivepal[j].g, sizeof(unsigned char), data);
		}
		for (int j = 0; j < 256; ++j) {
			engine->FWrite(&sprite[j].x, sizeof(double), data);
			engine->FWrite(&sprite[j].y, sizeof(double), data);
			engine->FWrite(&sprite[j].texture, sizeof(int), data);
			engine->FWrite(&sprite[j].alpha, sizeof(unsigned char), data);
			engine->FWrite(&sprite[j].uDivW, sizeof(double), data);
			engine->FWrite(&sprite[j].uDivH, sizeof(double), data);
			engine->FWrite(&sprite[j].vMove, sizeof(double), data);
			engine->FWrite(&sprite[j].hMove, sizeof(double), data);
			engine->FWrite(&sprite[j].objectinteract, sizeof(char), data);
			engine->FWrite(&sprite[j].view, sizeof(int), data);
			engine->FWrite(&sprite[j].frame, sizeof(int), data);
			engine->FWrite(&sprite[j].angle, sizeof(int), data);
		}
		for (int j = 0; j < 256; ++j) {
			for (int k = 0; k < 4; ++k) {
				engine->FWrite(&wallData[j].texture[k], sizeof(int), data);
				engine->FWrite(&wallData[j].solid[k], sizeof(int), data);
				engine->FWrite(&wallData[j].ignorelighting[k], sizeof(int), data);
				engine->FWrite(&wallData[j].alpha[k], sizeof(int), data);
				engine->FWrite(&wallData[j].blendtype[k], sizeof(int), data);
				engine->FWrite(&wallData[j].mask[k], sizeof(int), data);
			}
			engine->FWrite(&wallData[j].hotspotinteract, sizeof(char), data);
		}
		engine->FWrite(&raycastOn, sizeof(bool), data);
		engine->FWrite(&heightmapOn, sizeof(bool), data);
		engine->FWrite(&posX, sizeof(double), data);
		engine->FWrite(&posY, sizeof(double), data);
		engine->FWrite(&dirX, sizeof(double), data);
		engine->FWrite(&dirY, sizeof(double), data);
		engine->FWrite(&planeX, sizeof(double), data);
		engine->FWrite(&planeY, sizeof(double), data);
		engine->FWrite(&moveSpeed, sizeof(double), data);
		engine->FWrite(&rotSpeed, sizeof(double), data);
		if (raycastOn) { //If the raycaster is active, we have additional data to save.
			for (int i = 0; i < mapWidth; ++i)
				for (int j = 0; j < mapHeight; ++j) {
					engine->FWrite(&worldMap [i][j], sizeof(unsigned char), data);
					engine->FWrite(&lightMap [i][j], sizeof(unsigned char), data);
					engine->FWrite(&ceilingMap [i][j], sizeof(int), data);
					engine->FWrite(&floorMap [i][j], sizeof(int), data);
					engine->FWrite(&heightMap [i][j], sizeof(int), data);
				}
		}
		engine->FWrite(&textureSlot, sizeof(int), data);
		engine->FWrite(&skybox, sizeof(int), data);
		engine->FWrite(&ambientlight, sizeof(int), data);
	}
	if (event == AGSE_RESTOREGAME) {


		for (int i = 0; i < MAX_OVERLAYS; ++i) {
			engine->FRead(&overlay[i].sprite, sizeof(int), data);
			engine->FRead(&overlay[i].spritemask, sizeof(int), data);
			engine->FRead(&overlay[i].x, sizeof(int), data);
			engine->FRead(&overlay[i].y, sizeof(int), data);
			engine->FRead(&overlay[i].level, sizeof(int), data);
			engine->FRead(&overlay[i].trans, sizeof(int), data);
			engine->FRead(&overlay[i].blendtype, sizeof(int), data);
			engine->FRead(&overlay[i].enabled, sizeof(bool), data);
		}
		engine->FRead(&clutslot, sizeof(int), data);
		engine->FRead(&drawreflections, sizeof(int), data);
		for (int j = 0; j < 256; ++j) {
			engine->FRead(&cycle_remap[j], sizeof(unsigned char), data);
		}
		for (int j = 0; j < 256; ++j) { //Save Objective Palette, for palette mixing.
			engine->FRead(&objectivepal[j].r, sizeof(unsigned char), data);
			engine->FRead(&objectivepal[j].b, sizeof(unsigned char), data);
			engine->FRead(&objectivepal[j].g, sizeof(unsigned char), data);
		}
		for (int j = 0; j < 256; ++j) { //Save Raycaster Sprite struct, 256 instances.
			engine->FRead(&sprite[j].x, sizeof(double), data);
			engine->FRead(&sprite[j].y, sizeof(double), data);
			engine->FRead(&sprite[j].texture, sizeof(int), data);
			engine->FRead(&sprite[j].alpha, sizeof(unsigned char), data);
			engine->FRead(&sprite[j].uDivW, sizeof(double), data);
			engine->FRead(&sprite[j].uDivH, sizeof(double), data);
			engine->FRead(&sprite[j].vMove, sizeof(double), data);
			engine->FRead(&sprite[j].hMove, sizeof(double), data);
			engine->FRead(&sprite[j].objectinteract, sizeof(char), data);
			engine->FRead(&sprite[j].view, sizeof(int), data);
			engine->FRead(&sprite[j].frame, sizeof(int), data);
			engine->FRead(&sprite[j].angle, sizeof(int), data);
		}
		for (int j = 0; j < 256; ++j) { //Save Raycaster wall type data.
			for (int k = 0; k < 4; ++k) {
				engine->FRead(&wallData[j].texture[k], sizeof(int), data);
				engine->FRead(&wallData[j].solid[k], sizeof(int), data);
				engine->FRead(&wallData[j].ignorelighting[k], sizeof(int), data);
				engine->FRead(&wallData[j].alpha[k], sizeof(int), data);
				engine->FRead(&wallData[j].blendtype[k], sizeof(int), data);
				engine->FRead(&wallData[j].mask[k], sizeof(int), data);
			}
			engine->FRead(&wallData[j].hotspotinteract, sizeof(char), data);
		}
		//Delete worldmap data if it exists.
		engine->FRead(&raycastOn, sizeof(bool), data);
		engine->FRead(&heightmapOn, sizeof(bool), data);
		engine->FRead(&posX, sizeof(double), data);
		engine->FRead(&posY, sizeof(double), data);
		engine->FRead(&dirX, sizeof(double), data);
		engine->FRead(&dirY, sizeof(double), data);
		engine->FRead(&planeX, sizeof(double), data);
		engine->FRead(&planeY, sizeof(double), data);
		engine->FRead(&moveSpeed, sizeof(double), data);
		engine->FRead(&rotSpeed, sizeof(double), data);
		if (raycastOn) { //If the raycaster is currently running, we have additional data to load.
			for (int i = 0; i < mapWidth; ++i) {
				for (int j = 0; j < mapHeight; ++j) {
					engine->FRead(&worldMap [i][j], sizeof(unsigned char), data);
					engine->FRead(&lightMap [i][j], sizeof(unsigned char), data);
					engine->FRead(&ceilingMap [i][j], sizeof(int), data);
					engine->FRead(&floorMap [i][j], sizeof(int), data);
					engine->FRead(&heightMap [i][j], sizeof(int), data);
					seenMap [i][j] = 0;
				}
			}

			//Reinitialize all the buffers and stuff.

		}
		engine->FRead(&textureSlot, sizeof(int), data);
		if (textureSlot) MakeTextures(textureSlot);
		engine->FRead(&skybox, sizeof(int), data);
		engine->FRead(&ambientlight, sizeof(int), data);
		LoadCLUT(clutslot);
	}
	if (event == AGSE_ENTERROOM) {
		ResetRemapping();
		delete[] Reflection.Objects;
		Reflection.Objects = new objrefopt [engine->GetNumObjects()]();
	}
	return 0;
}

} // namespace AGSPalRender
} // namespace Plugins
} // namespace AGS3
