/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2003/2004 DJWillis - GP32 Backend
 * Copyright (C) 2005 Won Star - GP32 Backend
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "stdafx.h"
#include "common/scummsys.h"
#include "common/file.h"

#include "base/engine.h"

#include "backends/gp32/gp32std.h"
#include "backends/gp32/gp32std_grap.h"
#include "backends/gp32/gp32std_input.h"

#include "backends/gp32/gfx_splash.h"
#include "backends/gp32/gp32_launcher.h"
#include "backends/gp32/globals.h"

uint16 cpuSpeedTable[15] = {40, 66, 100, 120, 133, 144, 156, 160, 166, 172, 176, 180, 188, 192, 200};
uint16 gammaTable[16] = {5000, 6000, 7000, 8000, 9000, 10000, 11000, 12000, 13000, 14000, 15000, 16000, 17000, 18000, 19000, 20000};
char *oplTable[3] = {"LOW", "MEDIUM", "HIGH"};
uint16 sampleTable[4] = {0, 11025, 22050, 44100};

uint8 maxTable[5] = {15, 16, 3, 3, 2};
uint8 currentSetting[5] = {2, 5, 1, 1, 0};

void writeConfigVars() {
	Common::File file;

	g_vars.cpuSpeed = cpuSpeedTable[currentSetting[0]];
	g_vars.gammaRamp = gammaTable[currentSetting[1]];
	g_vars.fmQuality = currentSetting[2];
	g_vars.sampleRate = sampleTable[currentSetting[3]];

	if (!file.open("gp:\\gpetc\\scummvm.cfg", Common::File::kFileWriteMode)) {
		return;
	}
	file.writeByte(currentSetting[0]);
	file.writeByte(currentSetting[1]);
	file.writeByte(currentSetting[2]);
	file.writeByte(currentSetting[3]);
	file.close();
}

void readConfigVars() {
	Common::File file;
	if (!file.open("gp:\\gpetc\\scummvm.cfg", Common::File::kFileReadMode)) {
		currentSetting[0] = 2;
		currentSetting[1] = 5;
		currentSetting[2] = 1;
		currentSetting[3] = 1;
		writeConfigVars();
		return;
	}
	currentSetting[0] = file.readByte();
	currentSetting[1] = file.readByte();
	currentSetting[2] = file.readByte();
	currentSetting[3] = file.readByte();
	g_vars.cpuSpeed = cpuSpeedTable[currentSetting[0]];
	g_vars.gammaRamp = gammaTable[currentSetting[1]];
	g_vars.fmQuality = currentSetting[2];
	g_vars.sampleRate = sampleTable[currentSetting[3]];
	file.close();
}

void configMenu() {
	uint32 nKeyUD;
	uint16 nKeyP;

	int currentSelect = 0;
	char text[32];

	// OK / CANCEL
	currentSetting[4] = 0;

	while (1) {
		gp_fillRect(frameBuffer2, 0, 0, 320, 240, 0xffff);
		gp_textOut(frameBuffer2, 90, 10, "Configuration Menu", 0);

		gp_textOut(frameBuffer2, 30, 40, "CPU clock speed", 0);
		gp_textOut(frameBuffer2, 30, 80, "Gamma ramp", 0);
		gp_textOut(frameBuffer2, 30, 120, "FMOPL (AdLib) quality", 0);
		gp_textOut(frameBuffer2, 30, 160, "Sampling rate", 0);

		gp_textOut(frameBuffer2, 100, 210, "OK         CANCEL", 0);

		if (currentSelect == 4) {
			gp_textOut(frameBuffer2, 80 + currentSetting[4] * 100, 210, "@", 0);
		} else
			gp_textOut(frameBuffer2, 20, (currentSelect + 1) * 40, "@", 0);

		sprintf(text, "%d MHz", cpuSpeedTable[currentSetting[0]]);
		gp_textOut(frameBuffer2, 220, 40, text, 0);
		sprintf(text, "%.2f", (float)gammaTable[currentSetting[1]] / 10000);
		gp_textOut(frameBuffer2, 220, 80, text, 0);
		gp_textOut(frameBuffer2, 220, 120, oplTable[currentSetting[2]], 0);
		if (sampleTable[currentSetting[3]] == 0) {
			strcpy(text, "NO SOUND");
		} else {
			sprintf(text, "%d Hz", sampleTable[currentSetting[3]]);
		}
		gp_textOut(frameBuffer2, 220, 160, text, 0);

		gp_flipScreen();

		gp_getButtonEvent(&nKeyUD, &nKeyP);

		if (gpd_getButtonDown(nKeyUD, GPC_VK_UP)) {
			if (currentSelect > 0)
				currentSelect--;
		}
		if (gpd_getButtonDown(nKeyUD, GPC_VK_DOWN)) {
			if (currentSelect < 4)
				currentSelect++;
		}
		if (gpd_getButtonDown(nKeyUD, GPC_VK_LEFT)) {
			if (currentSelect <= 4)
				if (currentSetting[currentSelect] > 0)
					currentSetting[currentSelect]--;
		}
		if (gpd_getButtonDown(nKeyUD, GPC_VK_RIGHT)) {
			if (currentSelect <= 4)
				if (currentSetting[currentSelect] < maxTable[currentSelect] - 1)
					currentSetting[currentSelect]++;
		}

		if (gpd_getButtonUp(nKeyUD, GPC_VK_START) ||
			gpd_getButtonUp(nKeyUD, GPC_VK_FA)) {
			if (currentSelect == 4) {
				if (currentSetting[currentSelect] == 0) { // OK
					writeConfigVars();
					return;
				} else { // CANCEL
					return;
				}
			}
		}
	}
}

void splashScreen() {
	uint32 nKeyUD;
	uint16 nKeyP;

	while (1) {
		uint16 *buffer = frameBuffer2;//&frameBuffer1[240 - _screenHeight];
		uint8 *picBuffer = gfx_splash;

		for (int c = 0; c < LCD_WIDTH * LCD_HEIGHT; c++) {
			*buffer++ = gfx_splash_Pal[*picBuffer++];
		}

		gp_flipScreen();

		while (1) {
			gp_getButtonEvent(&nKeyUD, &nKeyP);

			if (gpd_getButtonUp(nKeyUD, GPC_VK_START) ||
				gpd_getButtonUp(nKeyUD, GPC_VK_FA)) {
				gp_fillRect(frameBuffer1, 0, 0, 320, 240, 0xffff);
				gp_fillRect(frameBuffer2, 0, 0, 320, 240, 0xffff);
				return;
			}
			if (gpd_getButtonUp(nKeyUD, GPC_VK_SELECT)) {
				configMenu();
				break;
			}
		}
	}
}
