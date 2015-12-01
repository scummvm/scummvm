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
#include "lab/intro.h"
#include "lab/labfun.h"
#include "lab/resource.h"
#include "lab/anim.h"
#include "lab/text.h"
#include "lab/interface.h"

namespace Lab {
extern uint16 *FadePalette;

Intro::Intro() {
	_quitIntro = false;
}

/******************************************************************************/
/* Goes thru, and responds to all the intuition messages currently in the     */
/* the message port.                                                          */
/******************************************************************************/
void Intro::introEatMessages() {
	while (1) {
		IntuiMessage *msg = g_lab->getMsg();

		if (g_engine->shouldQuit()) {
			_quitIntro = true;
			return;
		}

		if (msg == NULL)
			return;
		else {
			if (((msg->msgClass == MOUSEBUTTONS) && (IEQUALIFIER_RBUTTON & msg->qualifier)) ||
			        ((msg->msgClass == RAWKEY) && (msg->code == 27))
				)
				_quitIntro = true;
		}
	}
}




/*****************************************************************************/
/* Reads in a picture.                                                       */
/*****************************************************************************/
void Intro::doPictText(const char *filename, TextFont *msgFont, bool isscreen) {
	uint32 lastsecs = 0L, lastmicros = 0L, secs = 0L, micros = 0L;
	IntuiMessage *msg;
	char path[50] = "Lab:rooms/Intro/";
	byte *curplace, **tfile;
	bool drawNextText = true, end = false, begin = true;

	int32 cls, code, Drawn;
	int16 qualifier;
	uint timedelay;

	strcat(path, filename);

	if (isscreen) {
		g_lab->_music->updateMusic();
		timedelay = 35;
	} else {
		g_lab->_music->updateMusic();
		timedelay = 7;
	}

	if (_quitIntro)
		return;

	while (1) {
		if (drawNextText) {
			if (begin) {
				begin = false;

				tfile = g_lab->_music->newOpen(path);

				if (!tfile)
					return;

				curplace = *tfile;
			} else if (isscreen)
				fade(false, 0);

			if (isscreen) {
				g_lab->setAPen(7);
				g_lab->rectFill(VGAScaleX(10), VGAScaleY(10), VGAScaleX(310), VGAScaleY(190));

				Drawn = flowText(msgFont, (!g_lab->_isHiRes) * -1, 5, 7, false, false, true, true, VGAScaleX(14), VGAScaleY(11), VGAScaleX(306), VGAScaleY(189), (char *)curplace);
				fade(true, 0);
			} else {
				Drawn = g_lab->longDrawMessage((char *)curplace);
			}

			curplace += Drawn;

			end = (*curplace == 0);

			drawNextText = false;
			introEatMessages();

			if (_quitIntro) {
				if (isscreen)
					fade(false, 0);

				return;
			}

			g_lab->getTime(&lastsecs, &lastmicros);
		}

		msg = g_lab->getMsg();

		if (msg == NULL) {
			g_lab->_music->updateMusic();
			g_lab->_anim->diffNextFrame();
			g_lab->getTime(&secs, &micros);
			g_lab->anyTimeDiff(lastsecs, lastmicros, secs, micros, &secs, &micros);

			if (secs > timedelay) {
				if (end) {
					if (isscreen)
						fade(false, 0);

					return;
				} else {
					drawNextText = true;
				}
			}

			g_lab->waitTOF();
		} else {
			cls       = msg->msgClass;
			qualifier = msg->qualifier;
			code      = msg->code;

			if (((cls == MOUSEBUTTONS) && (IEQUALIFIER_RBUTTON & qualifier)) ||
			        ((cls == RAWKEY) && (code == 27))) {
				_quitIntro = true;

				if (isscreen)
					fade(false, 0);

				return;
			}

			else if (cls == MOUSEBUTTONS) {
				if (IEQUALIFIER_LEFTBUTTON & qualifier) {
					if (end) {
						if (isscreen)
							fade(false, 0);

						return;
					} else
						drawNextText = true;
				}

				introEatMessages();

				if (_quitIntro) {
					if (isscreen)
						fade(false, 0);

					return;
				}
			}

			if (end) {
				if (isscreen)
					fade(false, 0);

				return;
			} else
				drawNextText = true;
		}
	}
}





/*****************************************************************************/
/* Does a one second delay, but checks the music while doing it.             */
/*****************************************************************************/
void Intro::musicDelay() {
	g_lab->_music->updateMusic();

	if (_quitIntro)
		return;

	for (uint16 i = 0; i < 20; i++) {
		g_lab->_music->updateMusic();
		g_lab->waitTOF();
		g_lab->waitTOF();
		g_lab->waitTOF();
	}
}



void Intro::nReadPict(const char *filename, bool playOnce) {
	Common::String finalFileName = Common::String("P:Intro/") + filename;

	g_lab->_music->updateMusic();
	introEatMessages();

	if (_quitIntro)
		return;

	g_lab->_anim->_doBlack = _introDoBlack;
	g_lab->_anim->stopDiffEnd();
	readPict(finalFileName.c_str(), playOnce);
}


/*****************************************************************************/
/* Does the introduction sequence for Labyrinth.                             */
/*****************************************************************************/
void Intro::introSequence() {
	uint16 palette[16] = {
		0x0000, 0x0855, 0x0FF9, 0x0EE7,
		0x0ED5, 0x0DB4, 0x0CA2, 0x0C91,
		0x0B80, 0x0B80, 0x0B91, 0x0CA2,
		0x0CB3, 0x0DC4, 0x0DD6, 0x0EE7
	};

	g_lab->_anim->_doBlack = true;

	if (g_lab->getPlatform() != Common::kPlatformWindows) {
		nReadPict("EA0", true);
		nReadPict("EA1", true);
		nReadPict("EA2", true);
		nReadPict("EA3", true);
	} else {
		nReadPict("WYRMKEEP", true);
		// Wait 4 seconds
		for (uint16 i = 0; i < 4 * 1000 / 10; i++) {
			introEatMessages();
			if (_quitIntro)
				break;
			g_system->delayMillis(10);
		}
	}

	blackAllScreen();

	g_lab->_music->initMusic();

	g_lab->_anim->_noPalChange = true;
	if (g_lab->getPlatform() != Common::kPlatformWindows)
		nReadPict("TNDcycle.pic", true);
	else
		nReadPict("TNDcycle2.pic", true);
	g_lab->_anim->_noPalChange = false;

	FadePalette = palette;

	for (uint16 i = 0; i < 16; i++) {
		if (_quitIntro)
			break;

		palette[i] = ((g_lab->_anim->_diffPalette[i * 3] >> 2) << 8) +
		             ((g_lab->_anim->_diffPalette[i * 3 + 1] >> 2) << 4) +
		              (g_lab->_anim->_diffPalette[i * 3 + 2] >> 2);
	}

	g_lab->_music->updateMusic();
	fade(true, 0);

	for (int times = 0; times < 150; times++) {
		if (_quitIntro)
			break;

		g_lab->_music->updateMusic();
		uint16 temp = palette[2];

		for (uint16 i = 2; i < 15; i++)
			palette[i] = palette[i + 1];

		palette[15] = temp;

		setAmigaPal(palette, 16);
		g_lab->waitTOF();
	}

	fade(false, 0);
	blackAllScreen();

	g_lab->_music->updateMusic();

	nReadPict("Title.A", true);
	nReadPict("AB", true);
	musicDelay();
	nReadPict("BA", true);
	nReadPict("AC", true);
	musicDelay();

	if (g_lab->getPlatform() == Common::kPlatformWindows)
		musicDelay(); // more credits on this page now

	nReadPict("CA", true);
	nReadPict("AD", true);
	musicDelay();

	if (g_lab->getPlatform() == Common::kPlatformWindows)
		musicDelay(); // more credits on this page now

	nReadPict("DA", true);
	musicDelay();

	g_lab->_music->newOpen("p:Intro/Intro.1");  /* load the picture into the buffer */

	g_lab->_music->updateMusic();
	blackAllScreen();
	g_lab->_music->updateMusic();

	TextFont *msgFont = g_lab->_resource->getFont("P:Map.fon");

	g_lab->_anim->_noPalChange = true;
	nReadPict("Intro.1", true);
	g_lab->_anim->_noPalChange = false;

	for (uint16 i = 0; i < 16; i++) {
		palette[i] = ((g_lab->_anim->_diffPalette[i * 3] >> 2) << 8) +
		             ((g_lab->_anim->_diffPalette[i * 3 + 1] >> 2) << 4) +
		              (g_lab->_anim->_diffPalette[i * 3 + 2] >> 2);
	}

	doPictText("i.1", msgFont, true);
	doPictText("i.2A", msgFont, true);
	doPictText("i.2B", msgFont, true);

	freeAllStolenMem();

	blackAllScreen();
	g_lab->_music->updateMusic();

	_introDoBlack = true;
	nReadPict("Station1", true);
	doPictText("i.3", msgFont, false);

	nReadPict("Station2", true);
	doPictText("i.4", msgFont, false);

	nReadPict("Stiles4", true);
	doPictText("i.5", msgFont, false);

	nReadPict("Stiles3", true);
	doPictText("i.6", msgFont, false);

	nReadPict("Platform2", true);
	doPictText("i.7", msgFont, false);

	nReadPict("Subway.1", true);
	doPictText("i.8", msgFont, false);

	nReadPict("Subway.2", true);

	doPictText("i.9", msgFont, false);
	doPictText("i.10", msgFont, false);
	doPictText("i.11", msgFont, false);

	if (!_quitIntro)
		for (uint16 i = 0; i < 50; i++) {
			for (uint16 idx = (8 * 3); idx < (255 * 3); idx++)
				g_lab->_anim->_diffPalette[idx] = 255 - g_lab->_anim->_diffPalette[idx];

			g_lab->_music->updateMusic();
			g_lab->waitTOF();
			g_lab->setPalette(g_lab->_anim->_diffPalette, 256);
			g_lab->waitTOF();
			g_lab->waitTOF();
		}

	doPictText("i.12", msgFont, false);
	doPictText("i.13", msgFont, false);

	_introDoBlack = false;
	nReadPict("Daed0", true);
	doPictText("i.14", msgFont, false);

	nReadPict("Daed1", true);
	doPictText("i.15", msgFont, false);

	nReadPict("Daed2", true);
	doPictText("i.16", msgFont, false);
	doPictText("i.17", msgFont, false);
	doPictText("i.18", msgFont, false);

	nReadPict("Daed3", true);
	doPictText("i.19", msgFont, false);
	doPictText("i.20", msgFont, false);

	nReadPict("Daed4", true);
	doPictText("i.21", msgFont, false);

	nReadPict("Daed5", true);
	doPictText("i.22", msgFont, false);
	doPictText("i.23", msgFont, false);
	doPictText("i.24", msgFont, false);

	nReadPict("Daed6", true);
	doPictText("i.25", msgFont, false);
	doPictText("i.26", msgFont, false);

	nReadPict("Daed7", false);
	doPictText("i.27", msgFont, false);
	doPictText("i.28", msgFont, false);
	g_lab->_anim->stopDiffEnd();

	nReadPict("Daed8", true);
	doPictText("i.29", msgFont, false);
	doPictText("i.30", msgFont, false);

	nReadPict("Daed9", true);
	doPictText("i.31", msgFont, false);
	doPictText("i.32", msgFont, false);
	doPictText("i.33", msgFont, false);

	nReadPict("Daed9a", true);
	nReadPict("Daed10", true);
	doPictText("i.34", msgFont, false);
	doPictText("i.35", msgFont, false);
	doPictText("i.36", msgFont, false);

	nReadPict("SubX", true);

	if (_quitIntro) {
		g_lab->setAPen(0);
		g_lab->rectFill(0, 0, g_lab->_screenWidth - 1, g_lab->_screenHeight - 1);
		g_lab->_anim->_doBlack = true;
	}

	closeFont(msgFont);
}

} // End of namespace Lab
