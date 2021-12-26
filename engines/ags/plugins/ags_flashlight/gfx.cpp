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

#include "ags/plugins/ags_flashlight/ags_flashlight.h"

namespace AGS3 {
namespace Plugins {
namespace AGSFlashlight {

void AGSFlashlight::calc_x_n(uint32 _x) {
	flashlight_x = _x;

	flashlight_n = flashlight_x >> 24;

	if (flashlight_n)
		flashlight_n = (flashlight_n + 1) / 8;

	flashlight_x = ((flashlight_x >> 19) & 0x001F) | ((flashlight_x >> 5) & 0x07E0) | ((flashlight_x << 8) & 0xF800);

	flashlight_x = (flashlight_x | (flashlight_x << 16)) & 0x7E0F81F;
}


uint32 AGSFlashlight::_blender_alpha16_bgr(uint32 y) {
	uint32 result;

	y = ((y & 0xFFFF) | (y << 16)) & 0x7E0F81F;

	result = ((flashlight_x - y) * flashlight_n / 32 + y) & 0x7E0F81F;

	return ((result & 0xFFFF) | (result >> 16));
}



void AGSFlashlight::setPixel(int x, int y, uint32 color, uint32 *pixel) {
	if ((x >= g_DarknessDiameter) || (y >= g_DarknessDiameter) || (x < 0) || (y < 0))
		return;

	*(pixel + (y * g_DarknessDiameter) + x) = color;
}


void AGSFlashlight::plotCircle(int xm, int ym, int r, uint32 color) {
	uint32 *pixel = (uint32 *)_engine->GetRawBitmapSurface(g_LightBitmap);

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
			err += ++x * 2 + 1;

		if (r <= y)
			err += ++y * 2 + 1;
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
	uint16 *destpixel = (uint16 *)_engine->GetRawBitmapSurface(_engine->GetVirtualScreen());
	uint32 *sourcepixel = (uint32 *)_engine->GetRawBitmapSurface(g_LightBitmap);

	uint16 *currentdestpixel = destpixel;
	uint32 *currentsourcepixel = sourcepixel;

	int x, y;

	int targetX = (g_FlashlightDrawAtX > -1) ? g_FlashlightDrawAtX : 0;
	int targetY = (g_FlashlightDrawAtY > -1) ? g_FlashlightDrawAtY : 0;

	int startX = (g_FlashlightDrawAtX < 0) ? -1 * g_FlashlightDrawAtX : 0;
	int endX = (g_FlashlightDrawAtX + g_DarknessDiameter < screen_width) ? g_DarknessDiameter : g_DarknessDiameter - ((g_FlashlightDrawAtX + g_DarknessDiameter) - screen_width);

	int startY = (g_FlashlightDrawAtY < 0) ? -1 * g_FlashlightDrawAtY : 0;
	int endY = (g_FlashlightDrawAtY + g_DarknessDiameter < screen_height) ? g_DarknessDiameter : g_DarknessDiameter - ((g_FlashlightDrawAtY + g_DarknessDiameter) - screen_height);

	for (y = 0; y < endY - startY; y++) {
		currentdestpixel = destpixel + (y + targetY) * screen_width + targetX;
		currentsourcepixel = sourcepixel + (y + startY) * g_DarknessDiameter + startX;

		for (x = 0; x < endX - startX; x++) {
			calc_x_n(*currentsourcepixel);
			*currentdestpixel = (uint16)_blender_alpha16_bgr(*currentdestpixel);

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
	uint16 *destpixel = (uint16 *)_engine->GetRawBitmapSurface(screen);

	int32 red, blue, green, alpha;

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
	uint32 color = (255 - (int)((float)g_DarknessLightLevel * 2.55f)) << 24;
	BITMAP *screen = _engine->GetVirtualScreen();
	assert(screen->format.bytesPerPixel == 2);
	uint16 *destpixel = (uint16 *)_engine->GetRawBitmapSurface(screen);
	uint16 *currentpixel;

	calc_x_n(color);

	if (g_DarknessSize == 0) {
		// Whole screen.
		for (x = 0; x < screen_width * screen_height; x++) {
			*destpixel = (uint16)_blender_alpha16_bgr(*destpixel);
			destpixel++;
		}
	} else {
		// Top.
		if (g_FlashlightDrawAtY > -1) {
			currentpixel = destpixel;
			for (y = 0; y < g_FlashlightDrawAtY; y++) {
				for (x = 0; x < screen_width; x++) {
					*currentpixel = (uint16)_blender_alpha16_bgr(*currentpixel);
					currentpixel++;
				}
			}
		}

		// Bottom.
		if (g_FlashlightDrawAtY + g_DarknessDiameter < screen_height) {
			currentpixel = destpixel + (g_FlashlightDrawAtY + g_DarknessDiameter) * screen_width;
			for (y = g_FlashlightDrawAtY + g_DarknessDiameter; y < screen_height; y++) {
				for (x = 0; x < screen_width; x++) {
					*currentpixel = (uint16)_blender_alpha16_bgr(*currentpixel);
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
					*currentpixel = (uint16)_blender_alpha16_bgr(*currentpixel);
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
					*currentpixel = (uint16)_blender_alpha16_bgr(*currentpixel);
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
	uint32 color = (255 - (int)((float)g_DarknessLightLevel * 2.55f)) << 24;
	uint32 *pixel = (uint32 *)_engine->GetRawBitmapSurface(g_LightBitmap);

	int i;
	for (i = 0; i < g_DarknessDiameter * g_DarknessDiameter; i++)
		*pixel++ = (uint32)color;

	// Draw light circle if wanted.
	if (g_DarknessSize > 0 && g_DarknessLightLevel != g_BrightnessLightLevel) {
		uint32 current_value = 0;
		color = (255 - (int)((float)g_BrightnessLightLevel * 2.55f));
		uint32 targetcolor = ((255 - (int)((float)g_DarknessLightLevel * 2.55f)));

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

} // namespace AGSFlashlight
} // namespace Plugins
} // namespace AGS3
