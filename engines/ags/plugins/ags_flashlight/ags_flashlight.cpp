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
#include "ags/plugins/ags_flashlight/ags_flashlight.h"
#include "ags/shared/core/platform.h"
#include "common/str.h"

namespace AGS3 {
namespace Plugins {
namespace AGSFlashlight {

const unsigned int Magic = 0xBABE0000;
const unsigned int Version = 2;
const unsigned int SaveMagic = Magic + Version;
const float PI = 3.14159265f;

int AGSFlashlight::AGSFlashlight::screen_width = 320;
int AGSFlashlight::screen_height = 200;
int AGSFlashlight::screen_color_depth = 16;

bool AGSFlashlight::g_BitmapMustBeUpdated = true;

int AGSFlashlight::g_RedTint = 0;
int AGSFlashlight::g_GreenTint = 0;
int AGSFlashlight::g_BlueTint = 0;

int AGSFlashlight::g_DarknessLightLevel = 100;
int AGSFlashlight::g_BrightnessLightLevel = 100;
int AGSFlashlight::g_DarknessSize = 0;
int AGSFlashlight::g_DarknessDiameter = 0;
int AGSFlashlight::g_BrightnessSize = 0;

int AGSFlashlight::g_FlashlightX = 0;
int AGSFlashlight::g_FlashlightY = 0;
int AGSFlashlight::g_FlashlightDrawAtX = 0;
int AGSFlashlight::g_FlashlightDrawAtY = 0;

bool AGSFlashlight::g_FlashlightFollowMouse = false;

int AGSFlashlight::g_FollowCharacterId = 0;
int AGSFlashlight::g_FollowCharacterDx = 0;
int AGSFlashlight::g_FollowCharacterDy = 0;
int AGSFlashlight::g_FollowCharacterHorz = 0;
int AGSFlashlight::g_FollowCharacterVert = 0;

AGSCharacter *AGSFlashlight::g_FollowCharacter = nullptr;

BITMAP *AGSFlashlight::g_LightBitmap = nullptr;

unsigned long AGSFlashlight::flashlight_x;
unsigned long AGSFlashlight::flashlight_n;


IAGSEngine *AGSFlashlight::_engine;

AGSFlashlight::AGSFlashlight() : DLL() {
	_engine = nullptr;
	screen_width = 320;
	screen_height = 200;
	screen_color_depth = 16;

	g_BitmapMustBeUpdated = true;

	g_RedTint = 0;
	g_GreenTint = 0;
	g_BlueTint = 0;

	g_DarknessLightLevel = 100;
	g_BrightnessLightLevel = 100;
	g_DarknessSize = 0;
	g_DarknessDiameter = 0;
	g_BrightnessSize = 0;

	g_FlashlightX = 0;
	g_FlashlightY = 0;
	g_FlashlightDrawAtX = 0;
	g_FlashlightDrawAtY = 0;

	g_FlashlightFollowMouse = false;

	g_FollowCharacterId = 0;
	g_FollowCharacterDx = 0;
	g_FollowCharacterDy = 0;
	g_FollowCharacterHorz = 0;
	g_FollowCharacterVert = 0;

	g_FollowCharacter = nullptr;
	g_LightBitmap = nullptr;
	flashlight_x = 0;
	flashlight_n = 0;


	DLL_METHOD(AGS_GetPluginName);
	DLL_METHOD(AGS_EngineStartup);
	DLL_METHOD(AGS_EngineShutdown);
	DLL_METHOD(AGS_EngineOnEvent);
}

const char *AGSFlashlight::AGS_GetPluginName() {
	return "Flashlight plugin recreation";
}

void AGSFlashlight::AGS_EngineStartup(IAGSEngine *engine) {
	_engine = engine;

	if (_engine->version < 13)
		_engine->AbortGame("Engine interface is too old, need newer version of AGS.");

	SCRIPT_METHOD(SetFlashlightTint);
	SCRIPT_METHOD(GetFlashlightTintRed);
	SCRIPT_METHOD(GetFlashlightTintGreen);
	SCRIPT_METHOD(GetFlashlightTintBlue);

	SCRIPT_METHOD(GetFlashlightMinLightLevel);
	SCRIPT_METHOD(GetFlashlightMaxLightLevel);

	SCRIPT_METHOD(SetFlashlightDarkness);
	SCRIPT_METHOD(GetFlashlightDarkness);
	SCRIPT_METHOD(SetFlashlightDarknessSize);
	SCRIPT_METHOD(GetFlashlightDarknessSize);

	SCRIPT_METHOD(SetFlashlightBrightness);
	SCRIPT_METHOD(GetFlashlightBrightness);
	SCRIPT_METHOD(SetFlashlightBrightnessSize);
	SCRIPT_METHOD(GetFlashlightBrightnessSize);

	SCRIPT_METHOD(SetFlashlightPosition);
	SCRIPT_METHOD(GetFlashlightPositionX);
	SCRIPT_METHOD(GetFlashlightPositionY);


	SCRIPT_METHOD(SetFlashlightFollowMouse);
	SCRIPT_METHOD(GetFlashlightFollowMouse);

	SCRIPT_METHOD(SetFlashlightFollowCharacter);
	SCRIPT_METHOD(GetFlashlightFollowCharacter);
	SCRIPT_METHOD(GetFlashlightCharacterDX);
	SCRIPT_METHOD(GetFlashlightCharacterDY);
	SCRIPT_METHOD(GetFlashlightCharacterHorz);
	SCRIPT_METHOD(GetFlashlightCharacterVert);

	SCRIPT_METHOD(SetFlashlightMask);
	SCRIPT_METHOD(GetFlashlightMask);

	_engine->RequestEventHook(AGSE_PREGUIDRAW);
	_engine->RequestEventHook(AGSE_PRESCREENDRAW);
	_engine->RequestEventHook(AGSE_SAVEGAME);
	_engine->RequestEventHook(AGSE_RESTOREGAME);
}

void AGSFlashlight::AGS_EngineShutdown() {
}

NumberPtr AGSFlashlight::AGS_EngineOnEvent(int event, NumberPtr data) {
	if (event == AGSE_PREGUIDRAW) {
		Update();
	} else if (event == AGSE_RESTOREGAME) {
		RestoreGame(data);
	} else if (event == AGSE_SAVEGAME) {
		SaveGame(data);
	} else if (event == AGSE_PRESCREENDRAW) {
		// Get screen size once here.
		_engine->GetScreenDimensions(&screen_width, &screen_height, &screen_color_depth);
		_engine->UnrequestEventHook(AGSE_PRESCREENDRAW);

		// Only 16 bit color depth is supported.
		if (screen_color_depth != 16) {
			_engine->UnrequestEventHook(AGSE_PREGUIDRAW);
			_engine->UnrequestEventHook(AGSE_PRESCREENDRAW);
			_engine->UnrequestEventHook(AGSE_SAVEGAME);
			_engine->UnrequestEventHook(AGSE_RESTOREGAME);
		}
	}

	return 0;
}

void AGSFlashlight::calc_x_n(unsigned long x) {
	flashlight_x = x;

	flashlight_n = flashlight_x >> 24;

	if (flashlight_n)
		flashlight_n = (flashlight_n + 1) / 8;

	flashlight_x = ((flashlight_x >> 19) & 0x001F) | ((flashlight_x >> 5) & 0x07E0) | ((flashlight_x << 8) & 0xF800);

	flashlight_x = (flashlight_x | (flashlight_x << 16)) & 0x7E0F81F;
}

inline unsigned long AGSFlashlight::_blender_alpha16_bgr(unsigned long y) {
	unsigned long result;

	y = ((y & 0xFFFF) | (y << 16)) & 0x7E0F81F;

	result = ((flashlight_x - y) * flashlight_n / 32 + y) & 0x7E0F81F;

	return ((result & 0xFFFF) | (result >> 16));
}

inline void AGSFlashlight::setPixel(int x, int y, int color, unsigned int *pixel) {
	if ((x >= g_DarknessDiameter) || (y >= g_DarknessDiameter) || (x < 0) || (y < 0))
		return;

	*(pixel + (y * g_DarknessDiameter) + x) = color;
}

void AGSFlashlight::plotCircle(int xm, int ym, int r, unsigned int color) {
	unsigned int *pixel = *(unsigned int **)_engine->GetRawBitmapSurface(g_LightBitmap);

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

	_engine->ReleaseBitmapSurface(g_LightBitmap);
}

void AGSFlashlight::ClipToRange(int &variable, int min, int max) {
	if (variable < min)
		variable = min;

	if (variable > max)
		variable = max;
}

void AGSFlashlight::AlphaBlendBitmap() {
	unsigned short *destpixel = *(unsigned short **)_engine->GetRawBitmapSurface(_engine->GetVirtualScreen());
	unsigned int *sourcepixel = *(unsigned int **)_engine->GetRawBitmapSurface(g_LightBitmap);

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

	_engine->ReleaseBitmapSurface(_engine->GetVirtualScreen());
	_engine->ReleaseBitmapSurface(g_LightBitmap);
}

void AGSFlashlight::DrawTint() {
	int x, y;
	BITMAP *screen = _engine->GetVirtualScreen();
	unsigned short *destpixel = *(unsigned short **)_engine->GetRawBitmapSurface(screen);

	int red, blue, green, alpha;

	for (y = 0; y < screen_height; y++) {
		for (x = 0; x < screen_width; x++) {
			_engine->GetRawColorComponents(16, *destpixel, &red, &green, &blue, &alpha);

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

			*destpixel = _engine->MakeRawColorPixel(16, red, green, blue, alpha);
			destpixel++;
		}
	}

	_engine->ReleaseBitmapSurface(screen);
}

void AGSFlashlight::DrawDarkness() {
	int x, y;
	unsigned int color = (255 - (int)((float)g_DarknessLightLevel * 2.55f)) << 24;
	BITMAP *screen = _engine->GetVirtualScreen();
	unsigned short *destpixel = *(unsigned short **)_engine->GetRawBitmapSurface(screen);
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

	_engine->ReleaseBitmapSurface(screen);
}

void AGSFlashlight::CreateLightBitmap() {
	if (g_DarknessSize == 0)
		return;

	if (g_LightBitmap)
		_engine->FreeBitmap(g_LightBitmap);

	g_LightBitmap = _engine->CreateBlankBitmap(g_DarknessDiameter, g_DarknessDiameter, 32);

	// Fill with darkness color.
	unsigned int color = (255 - (int)((float)g_DarknessLightLevel * 2.55f)) << 24;
	unsigned int *pixel = *(unsigned int **)_engine->GetRawBitmapSurface(g_LightBitmap);

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

			if ((uint)current_value > targetcolor)
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

	_engine->ReleaseBitmapSurface(g_LightBitmap);
}

void AGSFlashlight::Update() {
	if (g_BitmapMustBeUpdated) {
		CreateLightBitmap();
		g_BitmapMustBeUpdated = false;
	}

	if (g_FlashlightFollowMouse) {
		_engine->GetMousePosition(&g_FlashlightX, &g_FlashlightY);
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

	_engine->MarkRegionDirty(0, 0, screen_width, screen_height);
}

size_t AGSFlashlight::engineFileRead(void *ptr, size_t size, size_t count, long fileHandle) {
	auto totalBytes = _engine->FRead(ptr, size * count, fileHandle);
	return totalBytes / size;
}

size_t AGSFlashlight::engineFileWrite(const void *ptr, size_t size, size_t count, long fileHandle) {
	auto totalBytes = _engine->FWrite(const_cast<void *>(ptr), size * count, fileHandle);
	return totalBytes / size;
}

void AGSFlashlight::RestoreGame(long file) {
	unsigned int SaveVersion = 0;
	engineFileRead(&SaveVersion, sizeof(SaveVersion), 1, file);

	if (SaveVersion != SaveMagic) {
		_engine->AbortGame("agsflashlight: bad save.");
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
		g_FollowCharacter = _engine->GetCharacter(g_FollowCharacterId);

	g_BitmapMustBeUpdated = true;
}

void AGSFlashlight::SaveGame(long file) {
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

void AGSFlashlight::SetFlashlightTint(int RedTint, int GreenTint, int BlueTint) {
	ClipToRange(RedTint, -31, 31);
	ClipToRange(GreenTint, -31, 31);
	ClipToRange(BlueTint, -31, 31);

	if ((RedTint != g_RedTint) || (GreenTint != g_GreenTint) || (BlueTint != g_BlueTint))
		g_BitmapMustBeUpdated = true;

	g_RedTint = RedTint;
	g_GreenTint = GreenTint;
	g_BlueTint = BlueTint;
}

int AGSFlashlight::GetFlashlightTintRed() {
	return g_RedTint;
}

int AGSFlashlight::GetFlashlightTintGreen() {
	return g_GreenTint;
}

int AGSFlashlight::GetFlashlightTintBlue() {
	return g_BlueTint;
}

int AGSFlashlight::GetFlashlightMinLightLevel() {
	return 0;
}

int AGSFlashlight::GetFlashlightMaxLightLevel() {
	return 100;
}

void AGSFlashlight::SetFlashlightDarkness(int LightLevel) {
	ClipToRange(LightLevel, 0, 100);

	if (LightLevel != g_DarknessLightLevel) {
		g_BitmapMustBeUpdated = true;
		g_DarknessLightLevel = LightLevel;

		if (g_DarknessLightLevel > g_BrightnessLightLevel)
			g_BrightnessLightLevel = g_DarknessLightLevel;
	}
}

int AGSFlashlight::GetFlashlightDarkness() {
	return g_DarknessLightLevel;
}

void AGSFlashlight::SetFlashlightDarknessSize(int Size) {
	if (Size != g_DarknessSize) {
		g_BitmapMustBeUpdated = true;
		g_DarknessSize = Size;
		g_DarknessDiameter = g_DarknessSize * 2;
	}
}

int AGSFlashlight::GetFlashlightDarknessSize() {
	return g_DarknessSize;
}


void AGSFlashlight::SetFlashlightBrightness(int LightLevel) {
	ClipToRange(LightLevel, 0, 100);

	if (LightLevel != g_BrightnessLightLevel) {
		g_BitmapMustBeUpdated = true;
		g_BrightnessLightLevel = LightLevel;

		if (g_BrightnessLightLevel < g_DarknessLightLevel)
			g_DarknessLightLevel = g_BrightnessLightLevel;
	}
}

int AGSFlashlight::GetFlashlightBrightness() {
	return g_BrightnessLightLevel;
}

void AGSFlashlight::SetFlashlightBrightnessSize(int Size) {
	if (Size != g_BrightnessSize) {
		g_BitmapMustBeUpdated = true;
		g_BrightnessSize = Size;
	}
}

int AGSFlashlight::GetFlashlightBrightnessSize() {
	return g_BrightnessSize;
}

void AGSFlashlight::SetFlashlightPosition(int X, int Y) {
	g_FlashlightX = X;
	g_FlashlightY = Y;
}

int AGSFlashlight::GetFlashlightPositionX() {
	return g_FlashlightX;
}

int AGSFlashlight::GetFlashlightPositionY() {
	return g_FlashlightY;
}

void AGSFlashlight::SetFlashlightFollowMouse(int OnOff) {
	g_FlashlightFollowMouse = (OnOff != 0);
}

int AGSFlashlight::GetFlashlightFollowMouse() {
	return g_FlashlightFollowMouse ? 1 : 0;
}

void AGSFlashlight::SetFlashlightFollowCharacter(int CharacterId, int dx, int dy, int horz, int vert) {
	g_FollowCharacterId = CharacterId;
	g_FollowCharacterDx = dx;
	g_FollowCharacterDy = dy;
	g_FollowCharacterHorz = horz;
	g_FollowCharacterVert = vert;

	g_FollowCharacter = _engine->GetCharacter(CharacterId);
}

int AGSFlashlight::GetFlashlightFollowCharacter() {
	return g_FollowCharacterId;
}

int AGSFlashlight::GetFlashlightCharacterDX() {
	return g_FollowCharacterDx;
}

int AGSFlashlight::GetFlashlightCharacterDY() {
	return g_FollowCharacterDy;
}

int AGSFlashlight::GetFlashlightCharacterHorz() {
	return g_FollowCharacterHorz;
}

int AGSFlashlight::GetFlashlightCharacterVert() {
	return g_FollowCharacterVert;
}

void AGSFlashlight::SetFlashlightMask(int SpriteSlot) {
	// Not implemented.
}

int AGSFlashlight::GetFlashlightMask() {
	return 0;
}

} // namespace AGSFlashlight
} // namespace Plugins
} // namespace AGS3
