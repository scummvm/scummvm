/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "lab/lab.h"
#include "lab/stddefines.h"
#include "lab/labfun.h"
#include "lab/resource.h"
#include "lab/diff.h"
#include "lab/text.h"
#include "lab/interface.h"

namespace Lab {

static TextFont filler, *msgfont = &filler;
static bool QuitIntro = false, IntroDoBlack;

extern bool nopalchange, DoBlack, IsHiRes;
extern char diffcmap[256 * 3];
extern uint32 VGAScreenWidth, VGAScreenHeight;
extern uint16 *FadePalette;


/******************************************************************************/
/* Goes thru, and responds to all the intuition messages currently in the     */
/* the message port.                                                          */
/******************************************************************************/
void introEatMessages() {
	IntuiMessage *Msg;

	while (1) {
		Msg = getMsg();

		if (g_engine->shouldQuit()) {
			QuitIntro = true;
			return;
		}

		if (Msg == NULL)
			return;
		else {
			if (((Msg->Class == MOUSEBUTTONS) && (IEQUALIFIER_RBUTTON & Msg->Qualifier)) ||
			        ((Msg->Class == RAWKEY) && (Msg->Code == 27))
				)
				QuitIntro = true;
		}
	}
}




/*****************************************************************************/
/* Reads in a picture.                                                       */
/*****************************************************************************/
static void doPictText(const char *Filename, bool isscreen) {
	uint32 lastsecs = 0L, lastmicros = 0L, secs = 0L, micros = 0L;
	IntuiMessage *Msg;
	char filename[50] = "Lab:rooms/Intro/";
	byte *curplace, **tfile;
	bool DrawNextText = true, End = false, Begin = true;

	int32 Class, Code, Drawn;
	int16 Qualifier;
	uint timedelay;

	strcat(filename, Filename);

	if (isscreen) {
		g_music->updateMusic();
		timedelay = 35;
	} else {
		g_music->updateMusic();
		timedelay = 7;
	}

	if (QuitIntro)
		return;

	while (1) {
		if (DrawNextText) {
			if (Begin) {
				Begin = false;

				tfile = g_music->newOpen(filename);

				if (!tfile)
					return;

				curplace = *tfile;
			} else if (isscreen)
				fade(false, 0);

			if (isscreen) {
				setAPen(7L);
				rectFill(VGAScaleX(10), VGAScaleY(10), VGAScaleX(310), VGAScaleY(190));

				Drawn = flowText(msgfont, (!IsHiRes) * -1, 5, 7, false, false, true, true, VGAScaleX(14), VGAScaleY(11), VGAScaleX(306), VGAScaleY(189), (char *)curplace);
				fade(true, 0);
			} else {
				Drawn = longDrawMessage((char *)curplace);
			}

			curplace += Drawn;

			End = (*curplace == 0);

			DrawNextText = false;
			introEatMessages();

			if (QuitIntro) {
				if (isscreen)
					fade(false, 0);

				return;
			}

			g_lab->getTime(&lastsecs, &lastmicros);
		}

		Msg = getMsg();

		if (Msg == NULL) {
			g_music->updateMusic();
			diffNextFrame();

			g_lab->getTime(&secs, &micros);
			g_lab->anyTimeDiff(lastsecs, lastmicros, secs, micros, &secs, &micros);

			if (secs > timedelay) {
				if (End) {
					if (isscreen)
						fade(false, 0);

					return;
				} else {
					DrawNextText = true;
				}
			}

			waitTOF();
		} else {
			Class     = Msg->Class;
			Qualifier = Msg->Qualifier;
			Code      = Msg->Code;

			if (((Class == MOUSEBUTTONS) && (IEQUALIFIER_RBUTTON & Qualifier)) ||
			        ((Class == RAWKEY) && (Code == 27))) {
				QuitIntro = true;

				if (isscreen)
					fade(false, 0);

				return;
			}

			else if (Class == MOUSEBUTTONS) {
				if (IEQUALIFIER_LEFTBUTTON & Qualifier) {
					if (End) {
						if (isscreen)
							fade(false, 0);

						return;
					} else
						DrawNextText = true;
				}

				introEatMessages();

				if (QuitIntro) {
					if (isscreen)
						fade(false, 0);

					return;
				}
			}

			if (End) {
				if (isscreen)
					fade(false, 0);

				return;
			} else
				DrawNextText = true;
		}
	}
}





/*****************************************************************************/
/* Does a one second delay, but checks the music while doing it.             */
/*****************************************************************************/
void musicDelay() {
	int16 counter;

	g_music->updateMusic();

	if (QuitIntro)
		return;

	for (counter = 0; counter < 20; counter++) {
		g_music->updateMusic();
		waitTOF();
		waitTOF();
		waitTOF();
	}
}



static void NReadPict(const char *Filename, bool PlayOnce) {
	Common::String finalFileName = Common::String("P:Intro/") + Filename;

	g_music->updateMusic();
	introEatMessages();

	if (QuitIntro)
		return;

	DoBlack = IntroDoBlack;
	stopDiffEnd();
	readPict(finalFileName.c_str(), PlayOnce);
}


/*****************************************************************************/
/* Does the introduction sequence for Labyrinth.                             */
/*****************************************************************************/
void introSequence() {
	uint16 counter, counter1;

	uint16 Palette[16] = {
		0x0000, 0x0855, 0x0FF9, 0x0EE7, 0x0ED5, 0x0DB4, 0x0CA2, 0x0C91, 0x0B80, 0x0B80, 0x0B91, 0x0CA2, 0x0CB3, 0x0DC4, 0x0DD6, 0x0EE7
	};

	DoBlack = true;

	if (g_lab->getPlatform() != Common::kPlatformWindows) {
		NReadPict("EA0", true);
		NReadPict("EA1", true);
		NReadPict("EA2", true);
		NReadPict("EA3", true);
	} else {
		NReadPict("WYRMKEEP", true);
		// Wait 4 seconds
		for (counter = 0; counter < 4 * 1000 / 10; counter++) {
			introEatMessages();
			if (QuitIntro)
				break;
			g_system->delayMillis(10);
		}
	}

	blackAllScreen();

	g_music->initMusic();

	nopalchange = true;
	if (g_lab->getPlatform() != Common::kPlatformWindows)
		NReadPict("TNDcycle.pic", true);
	else
		NReadPict("TNDcycle2.pic", true);
	nopalchange = false;

	FadePalette = Palette;

	for (counter = 0; counter < 16; counter++) {
		if (QuitIntro)
			break;

		Palette[counter] = ((diffcmap[counter * 3] >> 2) << 8) +
		                   ((diffcmap[counter * 3 + 1] >> 2) << 4) +
		                   (diffcmap[counter * 3 + 2] >> 2);
	}

	g_music->updateMusic();
	fade(true, 0);

	for (int times = 0; times < 150; times++) {
		if (QuitIntro)
			break;

		g_music->updateMusic();
		uint16 temp = Palette[2];

		for (counter = 2; counter < 15; counter++)
			Palette[counter] = Palette[counter + 1];

		Palette[15] = temp;

		setAmigaPal(Palette, 16);
		waitTOF();
	}

	fade(false, 0);
	blackAllScreen();

	g_music->updateMusic();

	NReadPict("Title.A", true);
	NReadPict("AB", true);
	musicDelay();
	NReadPict("BA", true);
	NReadPict("AC", true);
	musicDelay();

	if (g_lab->getPlatform() == Common::kPlatformWindows)
		musicDelay(); // more credits on this page now

	NReadPict("CA", true);
	NReadPict("AD", true);
	musicDelay();

	if (g_lab->getPlatform() == Common::kPlatformWindows)
		musicDelay(); // more credits on this page now

	NReadPict("DA", true);
	musicDelay();

	g_music->newOpen("p:Intro/Intro.1");  /* load the picture into the buffer */

	g_music->updateMusic();
	blackAllScreen();
	g_music->updateMusic();

	msgfont = g_resource->getFont("P:Map.fon");

	nopalchange = true;
	NReadPict("Intro.1", true);
	nopalchange = false;

	for (counter = 0; counter < 16; counter++) {
		Palette[counter] = ((diffcmap[counter * 3] >> 2) << 8) +
		                   ((diffcmap[counter * 3 + 1] >> 2) << 4) +
		                   (diffcmap[counter * 3 + 2] >> 2);
	}

	doPictText("i.1", true);
	doPictText("i.2A", true);
	doPictText("i.2B", true);

	freeAllStolenMem();

	blackAllScreen();
	g_music->updateMusic();

	IntroDoBlack = true;
	NReadPict("Station1", true);
	doPictText("i.3", false);

	NReadPict("Station2", true);
	doPictText("i.4", false);

	NReadPict("Stiles4", true);
	doPictText("i.5", false);

	NReadPict("Stiles3", true);
	doPictText("i.6", false);

	NReadPict("Platform2", true);
	doPictText("i.7", false);

	NReadPict("Subway.1", true);
	doPictText("i.8", false);

	NReadPict("Subway.2", true);

	doPictText("i.9", false);
	doPictText("i.10", false);
	doPictText("i.11", false);

	if (!QuitIntro)
		for (counter = 0; counter < 50; counter++) {
			for (counter1 = (8 * 3); counter1 < (255 * 3); counter1++)
				diffcmap[counter1] = 255 - diffcmap[counter1];

			g_music->updateMusic();
			waitTOF();
			VGASetPal(diffcmap, 256);
			waitTOF();
			waitTOF();
		}

	doPictText("i.12", false);
	doPictText("i.13", false);

	IntroDoBlack = false;
	NReadPict("Daed0", true);
	doPictText("i.14", false);

	NReadPict("Daed1", true);
	doPictText("i.15", false);

	NReadPict("Daed2", true);
	doPictText("i.16", false);
	doPictText("i.17", false);
	doPictText("i.18", false);

	NReadPict("Daed3", true);
	doPictText("i.19", false);
	doPictText("i.20", false);

	NReadPict("Daed4", true);
	doPictText("i.21", false);

	NReadPict("Daed5", true);
	doPictText("i.22", false);
	doPictText("i.23", false);
	doPictText("i.24", false);

	NReadPict("Daed6", true);
	doPictText("i.25", false);
	doPictText("i.26", false);

	NReadPict("Daed7", false);
	doPictText("i.27", false);
	doPictText("i.28", false);
	stopDiffEnd();

	NReadPict("Daed8", true);
	doPictText("i.29", false);
	doPictText("i.30", false);

	NReadPict("Daed9", true);
	doPictText("i.31", false);
	doPictText("i.32", false);
	doPictText("i.33", false);

	NReadPict("Daed9a", true);
	NReadPict("Daed10", true);
	doPictText("i.34", false);
	doPictText("i.35", false);
	doPictText("i.36", false);

	NReadPict("SubX", true);

	if (QuitIntro) {
		setAPen(0);
		rectFill(0, 0, VGAScreenWidth - 1, VGAScreenHeight - 1);
		DoBlack = true;
	}
}

} // End of namespace Lab
