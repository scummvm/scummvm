/*

This is not the AGS Flashlight plugin,
but a workalike plugin created by JJS for the AGS engine PSP port.

*/

#include "core/platform.h"

#if AGS_PLATFORM_OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma warning(disable : 4244)
#endif

#if !defined(BUILTIN_PLUGINS)
#define THIS_IS_THE_PLUGIN
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#if defined(PSP_VERSION)
#include <pspsdk.h>
#include <pspmath.h>
#include <pspdisplay.h>
#define sin(x) vfpu_sinf(x)
#endif

#include "plugin/agsplugin.h"

#if defined(BUILTIN_PLUGINS)
namespace agsflashlight {
#endif

#if defined(__GNUC__)
inline unsigned long _blender_alpha16_bgr(unsigned long y) __attribute__((always_inline));
inline void calc_x_n(unsigned long bla) __attribute__((always_inline));
#endif

const unsigned int Magic = 0xBABE0000;
const unsigned int Version = 2;
const unsigned int SaveMagic = Magic + Version;
const float PI = 3.14159265f;

int screen_width = 320;
int screen_height = 200;
int screen_color_depth = 16;

IAGSEngine *engine;

bool g_BitmapMustBeUpdated = true;

int g_RedTint = 0;
int g_GreenTint = 0;
int g_BlueTint = 0;

int g_DarknessLightLevel = 100;
int g_BrightnessLightLevel = 100;
int g_DarknessSize = 0;
int g_DarknessDiameter = 0;
int g_BrightnessSize = 0;

int g_FlashlightX = 0;
int g_FlashlightY = 0;
int g_FlashlightDrawAtX = 0;
int g_FlashlightDrawAtY = 0;

bool g_FlashlightFollowMouse = false;

int g_FollowCharacterId = 0;
int g_FollowCharacterDx = 0;
int g_FollowCharacterDy = 0;
int g_FollowCharacterHorz = 0;
int g_FollowCharacterVert = 0;

AGSCharacter *g_FollowCharacter = nullptr;

BITMAP *g_LightBitmap = nullptr;


// This function is from Allegro, split for more performance.

/* _blender_alpha16_bgr
 *  Combines a 32 bit RGBA sprite with a 16 bit RGB destination, optimised
 *  for when one pixel is in an RGB layout and the other is BGR.
 */

unsigned long x, n;

inline void calc_x_n(unsigned long _x) {
	x = _x;

	n = x >> 24;

	if (n)
		n = (n + 1) / 8;

	x = ((x >> 19) & 0x001F) | ((x >> 5) & 0x07E0) | ((x << 8) & 0xF800);

	x = (x | (x << 16)) & 0x7E0F81F;
}


inline unsigned long _blender_alpha16_bgr(unsigned long y) {
	unsigned long result;

	y = ((y & 0xFFFF) | (y << 16)) & 0x7E0F81F;

	result = ((x - y) * n / 32 + y) & 0x7E0F81F;

	return ((result & 0xFFFF) | (result >> 16));
}



inline void setPixel(int x, int y, int color, unsigned int *pixel) {
	if ((x >= g_DarknessDiameter) || (y >= g_DarknessDiameter) || (x < 0) || (y < 0))
		return;

	*(pixel + (y * g_DarknessDiameter) + x) = color;
}


void plotCircle(int xm, int ym, int r, unsigned int color) {
	unsigned int *pixel = *(unsigned int **)engine->GetRawBitmapSurface(g_LightBitmap);

	int x = -r;
	int y = 0;
	int err = 2 - 2 * r;

	do {
		setPixel(xm - x, ym + y, color, pixel); //   I. Quadrant
		setPixel(xm - x - 1, ym + y, color, pixel);

		setPixel(xm - y, ym - x, color, pixel); // II. Quadrant
		setPixel(xm - y, ym - x - 1, color, pixel);

		setPixel(xm + x, ym - y, color, pixel); // III. Quadrant
		setPixel(xm + x + 1, ym - y, color, pixel);

		setPixel(xm + y, ym + x, color, pixel); //  IV. Quadrant
		setPixel(xm + y, ym + x + 1, color, pixel);

		r = err;
		if (r > x)
			err +=  ++x * 2 + 1;

		if (r <= y)
			err +=  ++y * 2 + 1;
	} while (x < 0);

	engine->ReleaseBitmapSurface(g_LightBitmap);
}


void ClipToRange(int &variable, int min, int max) {
	if (variable < min)
		variable = min;

	if (variable > max)
		variable = max;
}


void AlphaBlendBitmap() {
	unsigned short *destpixel = *(unsigned short **)engine->GetRawBitmapSurface(engine->GetVirtualScreen());
	unsigned int *sourcepixel = *(unsigned int **)engine->GetRawBitmapSurface(g_LightBitmap);

	unsigned short *currentdestpixel = destpixel;
	unsigned int *currentsourcepixel = sourcepixel;

	int x, y;

	int targetX = (g_FlashlightDrawAtX > -1) ? g_FlashlightDrawAtX : 0;
	int targetY = (g_FlashlightDrawAtY > -1) ? g_FlashlightDrawAtY : 0;

	int startX = (g_FlashlightDrawAtX < 0) ? -1 * g_FlashlightDrawAtX : 0;
	int endX = (g_FlashlightDrawAtX + g_DarknessDiameter < screen_width) ? g_DarknessDiameter : g_DarknessDiameter - ((g_FlashlightDrawAtX + g_DarknessDiameter) - screen_width);

	int startY = (g_FlashlightDrawAtY < 0) ? -1 * g_FlashlightDrawAtY : 0;
	int endY = (g_FlashlightDrawAtY + g_DarknessDiameter < screen_height) ? g_DarknessDiameter :  g_DarknessDiameter - ((g_FlashlightDrawAtY + g_DarknessDiameter) - screen_height);

	for (y = 0; y < endY - startY; y++) {
		currentdestpixel = destpixel + (y + targetY) * screen_width + targetX;
		currentsourcepixel = sourcepixel + (y + startY) * g_DarknessDiameter + startX;

		for (x = 0; x < endX - startX; x++) {
			calc_x_n(*currentsourcepixel);
			*currentdestpixel = (unsigned short)_blender_alpha16_bgr(*currentdestpixel);

			currentdestpixel++;
			currentsourcepixel++;
		}
	}

	engine->ReleaseBitmapSurface(engine->GetVirtualScreen());
	engine->ReleaseBitmapSurface(g_LightBitmap);
}


void DrawTint() {
	int x, y;
	BITMAP *screen = engine->GetVirtualScreen();
	unsigned short *destpixel = *(unsigned short **)engine->GetRawBitmapSurface(screen);

	int32 red, blue, green, alpha;

	for (y = 0; y < screen_height; y++) {
		for (x = 0; x < screen_width; x++) {
			engine->GetRawColorComponents(16, *destpixel, &red, &green, &blue, &alpha);

			if (g_RedTint != 0) {
				red += g_RedTint * 8;
				if (red > 255)
					red = 255;
				else if (red < 0)
					red = 0;
			}

			if (g_BlueTint != 0) {
				blue += g_BlueTint * 8;
				if (blue > 255)
					blue = 255;
				else if (blue < 0)
					blue = 0;
			}

			if (g_GreenTint != 0) {
				green += g_GreenTint * 8;
				if (green > 255)
					green = 255;
				else if (green < 0)
					green = 0;
			}

			*destpixel = engine->MakeRawColorPixel(16, red, green, blue, alpha);
			destpixel++;
		}
	}

	engine->ReleaseBitmapSurface(screen);
}


void DrawDarkness() {
	int x, y;
	unsigned int color = (255 - (int)((float)g_DarknessLightLevel * 2.55f)) << 24;
	BITMAP *screen = engine->GetVirtualScreen();
	unsigned short *destpixel = *(unsigned short **)engine->GetRawBitmapSurface(screen);
	unsigned short *currentpixel;

	calc_x_n(color);

	if (g_DarknessSize == 0) {
		// Whole screen.
		for (x = 0; x < screen_width * screen_height; x++) {
			*destpixel = (unsigned short)_blender_alpha16_bgr(*destpixel);
			destpixel++;
		}
	} else {
		// Top.
		if (g_FlashlightDrawAtY > -1) {
			currentpixel = destpixel;
			for (y = 0; y < g_FlashlightDrawAtY; y++) {
				for (x = 0; x < screen_width; x++) {
					*currentpixel = (unsigned short)_blender_alpha16_bgr(*currentpixel);
					currentpixel++;
				}
			}
		}

		// Bottom.
		if (g_FlashlightDrawAtY + g_DarknessDiameter < screen_height) {
			currentpixel = destpixel + (g_FlashlightDrawAtY + g_DarknessDiameter) * screen_width;
			for (y = g_FlashlightDrawAtY + g_DarknessDiameter; y < screen_height; y++) {
				for (x = 0; x < screen_width; x++) {
					*currentpixel = (unsigned short)_blender_alpha16_bgr(*currentpixel);
					currentpixel++;
				}
			}
		}

		// Left.
		if (g_FlashlightDrawAtX > 0) {
			currentpixel = destpixel;
			int startpoint = (g_FlashlightDrawAtY > 0) ? g_FlashlightDrawAtY : 0;
			int endpoint = (g_FlashlightDrawAtY + g_DarknessDiameter >= screen_height) ? screen_height + 1 : g_FlashlightDrawAtY + g_DarknessDiameter + 1;
			for (y = startpoint; y < endpoint; y++) {
				for (x = 0; x < g_FlashlightDrawAtX; x++) {
					*currentpixel = (unsigned short)_blender_alpha16_bgr(*currentpixel);
					currentpixel++;
				}

				currentpixel = destpixel + screen_width * y;
			}
		}

		// Right.
		if (g_FlashlightDrawAtX + g_DarknessDiameter < screen_width) {
			currentpixel = destpixel + (g_FlashlightDrawAtX + g_DarknessDiameter);
			int startpoint = (g_FlashlightDrawAtY > 0) ? g_FlashlightDrawAtY : 0;
			int endpoint = (g_FlashlightDrawAtY + g_DarknessDiameter >= screen_height) ? screen_height + 1 : g_FlashlightDrawAtY + g_DarknessDiameter + 1;
			for (y = startpoint; y < endpoint; y++) {
				for (x = g_FlashlightDrawAtX + g_DarknessDiameter; x < screen_width; x++) {
					*currentpixel = (unsigned short)_blender_alpha16_bgr(*currentpixel);
					currentpixel++;
				}

				currentpixel = destpixel + screen_width * y + (g_FlashlightDrawAtX + g_DarknessDiameter);
			}
		}
	}

	engine->ReleaseBitmapSurface(screen);
}


void CreateLightBitmap() {
	if (g_DarknessSize == 0)
		return;

	if (g_LightBitmap)
		engine->FreeBitmap(g_LightBitmap);

	g_LightBitmap = engine->CreateBlankBitmap(g_DarknessDiameter, g_DarknessDiameter, 32);

	// Fill with darkness color.
	unsigned int color = (255 - (int)((float)g_DarknessLightLevel * 2.55f)) << 24;
	unsigned int *pixel = *(unsigned int **)engine->GetRawBitmapSurface(g_LightBitmap);

	int i;
	for (i = 0; i < g_DarknessDiameter * g_DarknessDiameter; i++)
		*pixel++ = (unsigned int)color;

	// Draw light circle if wanted.
	if (g_DarknessSize > 0) {
		int current_value = 0;
		color = (255 - (int)((float)g_BrightnessLightLevel * 2.55f));
		unsigned int targetcolor = ((255 - (int)((float)g_DarknessLightLevel * 2.55f)));

		int increment = (targetcolor - color) / (g_DarknessSize - g_BrightnessSize);
		float perfect_increment = (float)(targetcolor - color) / (float)(g_DarknessSize - g_BrightnessSize);

		float error_term;

		for (i = g_BrightnessSize; i < g_DarknessSize; i++) {
			error_term = (perfect_increment * (i - g_BrightnessSize)) - current_value;

			if (error_term >= 1.0f)
				increment++;
			else if (error_term <= -1.0f)
				increment--;

			current_value += increment;

			if (current_value > targetcolor)
				current_value = targetcolor;

			plotCircle(g_DarknessSize, g_DarknessSize, i, (current_value << 24) + color);
		}
	}

	// Draw inner fully lit circle.
	if (g_BrightnessSize > 0) {
		color = (255 - (int)((float)g_BrightnessLightLevel * 2.55f)) << 24;

		for (i = 0; i < g_BrightnessSize; i++)
			plotCircle(g_DarknessSize, g_DarknessSize, i, color);
	}

	engine->ReleaseBitmapSurface(g_LightBitmap);
}


void Update() {
	if (g_BitmapMustBeUpdated) {
		CreateLightBitmap();
		g_BitmapMustBeUpdated = false;
	}

	if (g_FlashlightFollowMouse) {
		engine->GetMousePosition(&g_FlashlightX, &g_FlashlightY);
	} else if (g_FollowCharacter != nullptr) {
		g_FlashlightX = g_FollowCharacter->x + g_FollowCharacterDx;
		g_FlashlightY = g_FollowCharacter->y + g_FollowCharacterDy;

		if ((g_FollowCharacterHorz != 0) || (g_FollowCharacterVert != 0)) {
			switch (g_FollowCharacter->loop) {
			// Down
			case 0:
			case 4:
			case 6:
				g_FlashlightY += g_FollowCharacterVert;
				break;

			// Up
			case 3:
			case 5:
			case 7:
				g_FlashlightY -= g_FollowCharacterVert;
				break;

			// Left
			case 1:
				g_FlashlightX -= g_FollowCharacterHorz;
				break;

			// Right:
			case 2:
				g_FlashlightX += g_FollowCharacterHorz;
				break;
			}
		}
	}

	g_FlashlightDrawAtX = g_FlashlightX - g_DarknessSize;
	g_FlashlightDrawAtY = g_FlashlightY - g_DarknessSize;


	if ((g_GreenTint != 0) || (g_RedTint != 0) || (g_BlueTint != 0))
		DrawTint();

	if (g_DarknessSize > 0)
		AlphaBlendBitmap();

	if (g_DarknessLightLevel != 100)
		DrawDarkness();

	engine->MarkRegionDirty(0, 0, screen_width, screen_height);
}

static size_t engineFileRead(void *ptr, size_t size, size_t count, long fileHandle) {
	auto totalBytes = engine->FRead(ptr, size * count, fileHandle);
	return totalBytes / size;
}

static size_t engineFileWrite(const void *ptr, size_t size, size_t count, long fileHandle) {
	auto totalBytes = engine->FWrite(const_cast<void *>(ptr), size * count, fileHandle);
	return totalBytes / size;
}

void RestoreGame(long file) {
	unsigned int SaveVersion = 0;
	engineFileRead(&SaveVersion, sizeof(SaveVersion), 1, file);

	if (SaveVersion != SaveMagic) {
		engine->AbortGame("agsflashlight: bad save.");
	}

	// Current version
	engineFileRead(&g_RedTint, 4, 1, file);
	engineFileRead(&g_GreenTint, 4, 1, file);
	engineFileRead(&g_BlueTint, 4, 1, file);

	engineFileRead(&g_DarknessLightLevel, 4, 1, file);
	engineFileRead(&g_BrightnessLightLevel, 4, 1, file);
	engineFileRead(&g_DarknessSize, 4, 1, file);
	engineFileRead(&g_DarknessDiameter, 4, 1, file);
	engineFileRead(&g_BrightnessSize, 4, 1, file);

	engineFileRead(&g_FlashlightX, 4, 1, file);
	engineFileRead(&g_FlashlightY, 4, 1, file);

	engineFileRead(&g_FlashlightFollowMouse, 4, 1, file);

	engineFileRead(&g_FollowCharacterId, 4, 1, file);
	engineFileRead(&g_FollowCharacterDx, 4, 1, file);
	engineFileRead(&g_FollowCharacterDy, 4, 1, file);
	engineFileRead(&g_FollowCharacterHorz, 4, 1, file);
	engineFileRead(&g_FollowCharacterVert, 4, 1, file);

	if (g_FollowCharacterId != 0)
		g_FollowCharacter = engine->GetCharacter(g_FollowCharacterId);

	g_BitmapMustBeUpdated = true;
}


void SaveGame(long file) {
	engineFileWrite(&SaveMagic, sizeof(SaveMagic), 1, file);

	engineFileWrite(&g_RedTint, 4, 1, file);
	engineFileWrite(&g_GreenTint, 4, 1, file);
	engineFileWrite(&g_BlueTint, 4, 1, file);

	engineFileWrite(&g_DarknessLightLevel, 4, 1, file);
	engineFileWrite(&g_BrightnessLightLevel, 4, 1, file);
	engineFileWrite(&g_DarknessSize, 4, 1, file);
	engineFileWrite(&g_DarknessDiameter, 4, 1, file);
	engineFileWrite(&g_BrightnessSize, 4, 1, file);

	engineFileWrite(&g_FlashlightX, 4, 1, file);
	engineFileWrite(&g_FlashlightY, 4, 1, file);

	engineFileWrite(&g_FlashlightFollowMouse, 4, 1, file);

	engineFileWrite(&g_FollowCharacterId, 4, 1, file);
	engineFileWrite(&g_FollowCharacterDx, 4, 1, file);
	engineFileWrite(&g_FollowCharacterDy, 4, 1, file);
	engineFileWrite(&g_FollowCharacterHorz, 4, 1, file);
	engineFileWrite(&g_FollowCharacterVert, 4, 1, file);
}


// ********************************************
// ************  AGS Interface  ***************
// ********************************************



// tint screen
void SetFlashlightTint(int RedTint, int GreenTint, int BlueTint) {
	ClipToRange(RedTint, -31, 31);
	ClipToRange(GreenTint, -31, 31);
	ClipToRange(BlueTint, -31, 31);

	if ((RedTint != g_RedTint) || (GreenTint != g_GreenTint) || (BlueTint != g_BlueTint))
		g_BitmapMustBeUpdated = true;

	g_RedTint = RedTint;
	g_GreenTint = GreenTint;
	g_BlueTint = BlueTint;
}

int GetFlashlightTintRed() {
	return g_RedTint;
}

int GetFlashlightTintGreen() {
	return g_GreenTint;
}

int GetFlashlightTintBlue() {
	return g_BlueTint;
}

int GetFlashlightMinLightLevel() {
	return 0;
}

int GetFlashlightMaxLightLevel() {
	return 100;
}

void SetFlashlightDarkness(int LightLevel) {
	ClipToRange(LightLevel, 0, 100);

	if (LightLevel != g_DarknessLightLevel) {
		g_BitmapMustBeUpdated = true;
		g_DarknessLightLevel = LightLevel;

		if (g_DarknessLightLevel > g_BrightnessLightLevel)
			g_BrightnessLightLevel = g_DarknessLightLevel;
	}
}

int GetFlashlightDarkness() {
	return g_DarknessLightLevel;
}

void SetFlashlightDarknessSize(int Size) {
	if (Size != g_DarknessSize) {
		g_BitmapMustBeUpdated = true;
		g_DarknessSize = Size;
		g_DarknessDiameter = g_DarknessSize * 2;
	}
}

int GetFlashlightDarknessSize() {
	return g_DarknessSize;
}


void SetFlashlightBrightness(int LightLevel) {
	ClipToRange(LightLevel, 0, 100);

	if (LightLevel != g_BrightnessLightLevel) {
		g_BitmapMustBeUpdated = true;
		g_BrightnessLightLevel = LightLevel;

		if (g_BrightnessLightLevel < g_DarknessLightLevel)
			g_DarknessLightLevel = g_BrightnessLightLevel;
	}
}

int GetFlashlightBrightness() {
	return g_BrightnessLightLevel;
}

void SetFlashlightBrightnessSize(int Size) {
	if (Size != g_BrightnessSize) {
		g_BitmapMustBeUpdated = true;
		g_BrightnessSize = Size;
	}
}

int GetFlashlightBrightnessSize() {
	return g_BrightnessSize;
}

void SetFlashlightPosition(int X, int Y) {
	g_FlashlightX = X;
	g_FlashlightY = Y;
}

int GetFlashlightPositionX() {
	return g_FlashlightX;
}

int GetFlashlightPositionY() {
	return g_FlashlightY;
}

void SetFlashlightFollowMouse(int OnOff) {
	g_FlashlightFollowMouse = (OnOff != 0);
}

int GetFlashlightFollowMouse() {
	return g_FlashlightFollowMouse ? 1 : 0;
}

void SetFlashlightFollowCharacter(int CharacterId, int dx, int dy, int horz, int vert) {
	g_FollowCharacterId = CharacterId;
	g_FollowCharacterDx = dx;
	g_FollowCharacterDy = dy;
	g_FollowCharacterHorz = horz;
	g_FollowCharacterVert = vert;

	g_FollowCharacter = engine->GetCharacter(CharacterId);
}

int GetFlashlightFollowCharacter() {
	return g_FollowCharacterId;
}

int GetFlashlightCharacterDX() {
	return g_FollowCharacterDx;
}

int GetFlashlightCharacterDY() {
	return g_FollowCharacterDy;
}

int GetFlashlightCharacterHorz() {
	return g_FollowCharacterHorz;
}

int GetFlashlightCharacterVert() {
	return g_FollowCharacterVert;
}

void SetFlashlightMask(int SpriteSlot) {
	// Not implemented.
}

int GetFlashlightMask() {
	return 0;
}

void AGS_EngineStartup(IAGSEngine *lpEngine) {
	engine = lpEngine;

	if (engine->version < 13)
		engine->AbortGame("Engine interface is too old, need newer version of AGS.");

	engine->RegisterScriptFunction("SetFlashlightTint", (void *)&SetFlashlightTint);
	engine->RegisterScriptFunction("GetFlashlightTintRed", (void *)&GetFlashlightTintRed);
	engine->RegisterScriptFunction("GetFlashlightTintGreen", (void *)&GetFlashlightTintGreen);
	engine->RegisterScriptFunction("GetFlashlightTintBlue", (void *)&GetFlashlightTintBlue);

	engine->RegisterScriptFunction("GetFlashlightMinLightLevel", (void *)&GetFlashlightMinLightLevel);
	engine->RegisterScriptFunction("GetFlashlightMaxLightLevel", (void *)&GetFlashlightMaxLightLevel);

	engine->RegisterScriptFunction("SetFlashlightDarkness", (void *)&SetFlashlightDarkness);
	engine->RegisterScriptFunction("GetFlashlightDarkness", (void *)&GetFlashlightDarkness);
	engine->RegisterScriptFunction("SetFlashlightDarknessSize", (void *)&SetFlashlightDarknessSize);
	engine->RegisterScriptFunction("GetFlashlightDarknessSize", (void *)&GetFlashlightDarknessSize);

	engine->RegisterScriptFunction("SetFlashlightBrightness", (void *)&SetFlashlightBrightness);
	engine->RegisterScriptFunction("GetFlashlightBrightness", (void *)&GetFlashlightBrightness);
	engine->RegisterScriptFunction("SetFlashlightBrightnessSize", (void *)&SetFlashlightBrightnessSize);
	engine->RegisterScriptFunction("GetFlashlightBrightnessSize", (void *)&GetFlashlightBrightnessSize);

	engine->RegisterScriptFunction("SetFlashlightPosition", (void *)&SetFlashlightPosition);
	engine->RegisterScriptFunction("GetFlashlightPositionX", (void *)&GetFlashlightPositionX);
	engine->RegisterScriptFunction("GetFlashlightPositionY", (void *)&GetFlashlightPositionY);


	engine->RegisterScriptFunction("SetFlashlightFollowMouse", (void *)&SetFlashlightFollowMouse);
	engine->RegisterScriptFunction("GetFlashlightFollowMouse", (void *)&GetFlashlightFollowMouse);

	engine->RegisterScriptFunction("SetFlashlightFollowCharacter", (void *)&SetFlashlightFollowCharacter);
	engine->RegisterScriptFunction("GetFlashlightFollowCharacter", (void *)&GetFlashlightFollowCharacter);
	engine->RegisterScriptFunction("GetFlashlightCharacterDX", (void *)&GetFlashlightCharacterDX);
	engine->RegisterScriptFunction("GetFlashlightCharacterDY", (void *)&GetFlashlightCharacterDY);
	engine->RegisterScriptFunction("GetFlashlightCharacterHorz", (void *)&GetFlashlightCharacterHorz);
	engine->RegisterScriptFunction("GetFlashlightCharacterVert", (void *)&GetFlashlightCharacterVert);

	engine->RegisterScriptFunction("SetFlashlightMask", (void *)&SetFlashlightMask);
	engine->RegisterScriptFunction("GetFlashlightMask", (void *)&GetFlashlightMask);

	engine->RequestEventHook(AGSE_PREGUIDRAW);
	engine->RequestEventHook(AGSE_PRESCREENDRAW);
	engine->RequestEventHook(AGSE_SAVEGAME);
	engine->RequestEventHook(AGSE_RESTOREGAME);
}

void AGS_EngineShutdown() {

}

int AGS_EngineOnEvent(int event, int data) {
	if (event == AGSE_PREGUIDRAW) {
		Update();
	} else if (event == AGSE_RESTOREGAME) {
		RestoreGame(data);
	} else if (event == AGSE_SAVEGAME) {
		SaveGame(data);
	} else if (event == AGSE_PRESCREENDRAW) {
		// Get screen size once here.
		engine->GetScreenDimensions(&screen_width, &screen_height, &screen_color_depth);
		engine->UnrequestEventHook(AGSE_PRESCREENDRAW);

		// Only 16 bit color depth is supported.
		if (screen_color_depth != 16) {
			engine->UnrequestEventHook(AGSE_PREGUIDRAW);
			engine->UnrequestEventHook(AGSE_PRESCREENDRAW);
			engine->UnrequestEventHook(AGSE_SAVEGAME);
			engine->UnrequestEventHook(AGSE_RESTOREGAME);
		}
	}

	return 0;
}

int AGS_EngineDebugHook(const char *scriptName, int lineNum, int reserved) {
	return 0;
}

void AGS_EngineInitGfx(const char *driverID, void *data) {
}



#if AGS_PLATFORM_OS_WINDOWS && !defined(BUILTIN_PLUGINS)

// ********************************************
// ***********  Editor Interface  *************
// ********************************************

const char *scriptHeader =
    "import void SetFlashlightTint(int RedTint, int GreenTint, int BlueTint);\r\n"
    "import int GetFlashlightTintRed();\r\n"
    "import int GetFlashlightTintGreen();\r\n"
    "import int GetFlashlightTintBlue();\r\n"
    "import int GetFlashlightMinLightLevel();\r\n"
    "import int GetFlashlightMaxLightLevel();\r\n"
    "import void SetFlashlightDarkness(int LightLevel);\r\n"
    "import int GetFlashlightDarkness();\r\n"
    "import void SetFlashlightDarknessSize(int Size);\r\n"
    "import int GetFlashlightDarknessSize();\r\n"
    "import void SetFlashlightBrightness(int LightLevel);\r\n"
    "import int GetFlashlightBrightness();\r\n"
    "import void SetFlashlightBrightnessSize(int Size);\r\n"
    "import int GetFlashlightBrightnessSize();\r\n"
    "import void SetFlashlightPosition(int X, int Y);\r\n"
    "import int GetFlashlightPositionX();\r\n"
    "import int GetFlashlightPositionY();\r\n"
    "import void SetFlashlightFollowMouse(int OnOff);\r\n"
    "import int GetFlashlightFollowMouse ();\r\n"
    "import void SetFlashlightFollowCharacter(int CharacterId, int dx, int dy, int horz, int vert);\r\n"
    "import int GetFlashlightFollowCharacter();\r\n"
    "import int GetFlashlightCharacterDX();\r\n"
    "import int GetFlashlightCharacterDY();\r\n"
    "import int GetFlashlightCharacterHorz();\r\n"
    "import int GetFlashlightCharacterVert();\r\n";


IAGSEditor *editor;


LPCSTR AGS_GetPluginName(void) {
	// Return the plugin description
	return "Flashlight plugin recreation";
}

int  AGS_EditorStartup(IAGSEditor *lpEditor) {
	// User has checked the plugin to use it in their game

	// If it's an earlier version than what we need, abort.
	if (lpEditor->version < 1)
		return -1;

	editor = lpEditor;
	editor->RegisterScriptHeader(scriptHeader);

	// Return 0 to indicate success
	return 0;
}

void AGS_EditorShutdown() {
	// User has un-checked the plugin from their game
	editor->UnregisterScriptHeader(scriptHeader);
}

void AGS_EditorProperties(HWND parent) {
	// User has chosen to view the Properties of the plugin
	// We could load up an options dialog or something here instead
	MessageBoxA(parent, "Flashlight plugin recreation by JJS", "About", MB_OK | MB_ICONINFORMATION);
}

int AGS_EditorSaveGame(char *buffer, int bufsize) {
	// We don't want to save any persistent data
	return 0;
}

void AGS_EditorLoadGame(char *buffer, int bufsize) {
	// Nothing to load for this plugin
}

#endif


#if defined(BUILTIN_PLUGINS)
} // namespace agsflashlight
#endif
