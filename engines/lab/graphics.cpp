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

#include "lab/stddefines.h"
#include "lab/diff.h"
#include "lab/parsetypes.h"
#include "lab/labfun.h"
#include "lab/parsefun.h"
#include "lab/mouse.h"
#include "lab/vga.h"
#include "lab/text.h"

namespace Lab {

struct BitMap bit1, bit2, *DispBitMap = &bit1, *DrawBitMap = &bit1;


extern struct BitMap RawDiffBM;
extern char diffcmap[256 * 3], lastcmap[256 * 3];
extern bool IsBM, NoFlip, nopalchange, ContMusic;

extern int32 ReadSoFar;
extern bool ReadIsDone, ReadIsError;
extern bool DoBlack, EffectPlaying, stopsound;
extern bool IsHiRes;
extern struct TextFont *MsgFont;
extern const char *CurFileName;





/*---------------------------------------------------------------------------*/
/*------ From readPict.c.  Reads in pictures and animations from disk. ------*/
/*---------------------------------------------------------------------------*/


extern uint32 VGAScreenWidth, VGAScreenHeight, VGAPages, VGABytesPerPage;

/*****************************************************************************/
/* Reads in a picture into the dest bitmap.                                  */
/*****************************************************************************/
bool readPict(const char *filename, bool PlayOnce) {
	byte **file = NULL;
#if defined(DOSCODE)
	uint16 counter;
#endif

	stopDiff();

	ReadSoFar  = 0L;
	ReadIsDone = false;
	ReadIsError = false;

	file = g_music->newOpen(filename);

	if (file == NULL) {
		if ((filename[0] == 'p') || (filename[0] == 'P'))
			blackScreen();

		return false;
	}

	DispBitMap->BytesPerRow = VGAScreenWidth;
	DispBitMap->Rows        = VGAScreenHeight;
#if defined(DOSCODE)
	DispBitMap->Flags       = 0;
#else
	DispBitMap->Flags       = BITMAPF_VIDEO;
#endif
	DispBitMap->Depth       = VGAPages;
#if defined(DOSCODE)

	for (counter = 0; counter < VGAPages; counter++)
		DispBitMap->Planes[counter] = getVGABaseAddr();

#else
	// playDiff deals with resetting planes for the "video" display.
#endif

	readDiff(PlayOnce);

#if !defined(DOSCODE)
	ungetVGABaseAddr();
#endif

	return true;
}




/*****************************************************************************/
/* Reads in a music file.  Ignores any graphics.                             */
/*****************************************************************************/
bool readMusic(const char *filename) {
	byte **file = NULL;

	file = g_music->newOpen(filename);

	if (file == NULL) {
		return false;
	}

	DoBlack = false;
	readSound();

	return true;
}




/*****************************************************************************/
/* Reads in a picture into buffer memory.                                    */
/*****************************************************************************/
byte *readPictToMem(const char *filename, uint16 x, uint16 y) {
	byte **file = NULL;
	byte *Mem, *CurMem;
#if defined(DOSCODE)
	uint16 counter;
#endif

	stopDiff();

	ReadSoFar  = 0L;
	ReadIsDone = false;
	ReadIsError = false;

	allocFile((void **)&Mem, (int32) x * (int32) y, "Bitmap");
	CurMem = Mem;

	file = g_music->newOpen(filename);

	if (file == NULL)
		return NULL;

	DispBitMap->BytesPerRow = x;
	DispBitMap->Rows        = y;
	DispBitMap->Flags       = 0;
	DispBitMap->Depth       = VGAPages;
#if defined(DOSCODE)

	for (counter = 0; counter < VGAPages; counter++) {
		DispBitMap->Planes[counter] = CurMem;
		CurMem += VGABytesPerPage;
	}

#else
	DispBitMap->Planes[0] = CurMem;
	DispBitMap->Planes[1] = DispBitMap->Planes[0] + 0x10000;
	DispBitMap->Planes[2] = DispBitMap->Planes[1] + 0x10000;
	DispBitMap->Planes[3] = DispBitMap->Planes[2] + 0x10000;
	DispBitMap->Planes[4] = DispBitMap->Planes[3] + 0x10000;
#endif

	readDiff(true);

	return Mem;
}




/*---------------------------------------------------------------------------*/
/*------------ Does all the text rendering to the message boxes. ------------*/
/*---------------------------------------------------------------------------*/


bool DoNotDrawMessage = false;

extern bool LongWinInFront, Alternate;


/*----- The flowText routines -----*/




/******************************************************************************/
/* Extracts the first word from a string.                                     */
/******************************************************************************/
static void getWord(char *WordBuffer, const char *MainBuffer, uint16 *WordWidth) {
	uint16 width = 0;

	while ((MainBuffer[width] != ' ') && MainBuffer[width] &&
	        (MainBuffer[width] != '\n')) {
		WordBuffer[width] = MainBuffer[width];
		width++;
	}

	WordBuffer[width] = 0;

	*WordWidth = width;
}





/******************************************************************************/
/* Gets a line of text for flowText; makes sure that its length is less than  */
/* or equal to the maximum width.                                             */
/******************************************************************************/
static void getLine(struct TextFont *tf, char *LineBuffer, const char **MainBuffer, uint16 LineWidth) {
	uint16 CurWidth = 0, WordWidth;
	char WordBuffer[100];
	bool doit = true;

	LineWidth += textLength(tf, " ", 1);

	LineBuffer[0] = 0;

	while ((*MainBuffer)[0] && doit) {
		getWord(WordBuffer, *MainBuffer, &WordWidth);
		strcat(WordBuffer, " ");

		if ((CurWidth + textLength(tf, WordBuffer, WordWidth + 1)) <= LineWidth) {
			strcat(LineBuffer, WordBuffer);
			(*MainBuffer) += WordWidth;

			if ((*MainBuffer)[0] == '\n')
				doit = false;

			if ((*MainBuffer)[0])
				(*MainBuffer)++;

			CurWidth = textLength(tf, LineBuffer, strlen(LineBuffer));
		} else
			doit = false;
	}

	/* NYI: Would add code here to break up words in case they were longer than a line */
}




/******************************************************************************/
/* Dumps a chunk of text to an arbitrary box; flows it within that box and    */
/* optionally centers it. Returns the number of characters that were          */
/* processed.                                                                 */
/*                                                                            */
/* Note: Every individual word MUST be int16 enough to fit on a line, and     */
/* each line less than 255 characters.                                        */
/******************************************************************************/
uint32 flowText(void *font,      /* the TextAttr pointer */
                uint16 spacing,          /* How much vertical spacing between the lines */
                uint16 pencolor,         /* pen number to use for text */
                uint16 backpen,          /* the background color */
                bool fillback,                /* Whether to fill the background */
                bool centerh,                 /* Whether to center the text horizontally */
                bool centerv,                 /* Whether to center the text vertically */
                bool output,                  /* Whether to output any text */
                uint16 x1,               /* Cords */
                uint16 y1, uint16 x2, uint16 y2, const char *str) { /* The text itself */
	struct TextFont *msgfont = (TextFont *)font;
	char linebuffer[256];
	const char *temp;
	uint16 numlines, actlines, fontheight, width;
	uint16 x, y;

	if (fillback) {
		setAPen(backpen);
		rectFill(x1, y1, x2, y2);
	}

	if (str == NULL)
		return 0L;

	setAPen(pencolor);

	fontheight = textHeight(msgfont) + spacing;
	numlines   = (y2 - y1 + 1) / fontheight;
	width      = x2 - x1 + 1;
	y          = y1;

	if (centerv && output) {
		temp = str;
		actlines = 0;

		while (temp[0]) {
			getLine(msgfont, linebuffer, &temp, width);
			actlines++;
		}

		if (actlines <= numlines)
			y += ((y2 - y1 + 1) - (actlines * fontheight)) / 2;
	}

	temp = str;

	while (numlines && str[0]) {
		getLine(msgfont, linebuffer, &str, width);

		x = x1;

		if (centerh)
			x += (width - textLength(msgfont, linebuffer, strlen(linebuffer))) / 2;

		if (output)
			text(msgfont, x, y, pencolor, linebuffer, strlen(linebuffer));

		numlines--;
		y += fontheight;
	}

	return (str - temp);
}


extern uint32 VGABytesPerPage;
extern byte *VGABASEADDRESS;


/******************************************************************************/
/* Calls flowText, but flows it to memory.  Same restrictions as flowText.    */
/******************************************************************************/
uint32 flowTextToMem(struct Image *DestIm, void *font,     /* the TextAttr pointer */
                     uint16 spacing,          /* How much vertical spacing between the lines */
                     uint16 pencolor,         /* pen number to use for text */
                     uint16 backpen,          /* the background color */
                     bool fillback,                /* Whether to fill the background */
                     bool centerh,                 /* Whether to center the text horizontally */
                     bool centerv,                 /* Whether to center the text vertically */
                     bool output,                  /* Whether to output any text */
                     uint16 x1,               /* Cords */
                     uint16 y1, uint16 x2, uint16 y2, const char *str) { /* The text itself */
	uint32 res, vgabyte = VGABytesPerPage;
	byte *tmp = VGABASEADDRESS;

	VGABASEADDRESS = DestIm->ImageData;
	VGABytesPerPage = (uint32) DestIm->Width * (int32) DestIm->Height;

	res = flowText(font, spacing, pencolor, backpen, fillback, centerh, centerv, output, x1, y1, x2, y2, str);

	VGABytesPerPage = vgabyte;
	VGABASEADDRESS = tmp;

	return res;
}




/*----- The control panel stuff -----*/



void createBox(uint16 y2) {
	setAPen(7);                 /* Message box area */
	rectFill(VGAScaleX(4), VGAScaleY(154), VGAScaleX(315), VGAScaleY(y2 - 2));

	setAPen(0);                 /* Box around message area */
	drawHLine(VGAScaleX(2), VGAScaleY(152), VGAScaleX(317));
	drawVLine(VGAScaleX(317), VGAScaleY(152), VGAScaleY(y2));
	drawHLine(VGAScaleX(2), VGAScaleY(y2), VGAScaleX(317));
	drawVLine(VGAScaleX(2), VGAScaleY(152), VGAScaleY(y2));
}




int32 longcharsdrawn = 0L;
bool LastMessageLong = false;

void longDrawMessage(const char *str) {
	char NewText[512];

	if (str == NULL)
		return;

	attachGadgetList(NULL);
	mouseHide();
	strcpy(NewText, str);

	if (!LongWinInFront) {
		LongWinInFront = true;
		setAPen(3);                 /* Clear Area */
		rectFill(0, VGAScaleY(149) + SVGACord(2), VGAScaleX(319), VGAScaleY(199));
	}

	createBox(198);
	longcharsdrawn = flowText(MsgFont, 0, 1, 7, false, true, true, true, VGAScaleX(6), VGAScaleY(155), VGAScaleX(313), VGAScaleY(195), str);
	mouseShow();
}




/******************************************************************************/
/* Draws a message to the message box.                                        */
/******************************************************************************/
void drawMessage(const char *str) {
	if (DoNotDrawMessage) {
		DoNotDrawMessage = false;
		return;
	}

	if (str) {
		if ((textLength(MsgFont, str, strlen(str)) > VGAScaleX(306))) {
			longDrawMessage(str);
			LastMessageLong = true;
		} else {
			if (LongWinInFront) {
				LongWinInFront = false;
				drawPanel();
			}

			mouseHide();
			createBox(168);
			text(MsgFont, VGAScaleX(7), VGAScaleY(155) + SVGACord(2), 1, str, strlen(str));
			mouseShow();
			LastMessageLong = false;
		}
	}
}


/*---------------------------------------------------------------------------*/
/*--------------------------- All the wipe stuff. ---------------------------*/
/*---------------------------------------------------------------------------*/



#define TRANSWIPE      1
#define SCROLLWIPE     2
#define SCROLLBLACK    3
#define SCROLLBOUNCE   4
#define TRANSPORTER    5
#define READFIRSTFRAME 6
#define READNEXTFRAME  7




/*****************************************************************************/
/* Copies memory from one location to another 64 bytes at a time.            */
/*****************************************************************************/
void copyLong64(uint32 *Dest, uint32 *Source, uint32 Many64) {
	while (Many64) {
		*Dest = *Source;
		Dest++;
		Source++;
		*Dest = *Source;
		Dest++;
		Source++;
		*Dest = *Source;
		Dest++;
		Source++;
		*Dest = *Source;
		Dest++;
		Source++;
		*Dest = *Source;
		Dest++;
		Source++;
		*Dest = *Source;
		Dest++;
		Source++;
		*Dest = *Source;
		Dest++;
		Source++;
		*Dest = *Source;
		Dest++;
		Source++;
		*Dest = *Source;
		Dest++;
		Source++;
		*Dest = *Source;
		Dest++;
		Source++;
		*Dest = *Source;
		Dest++;
		Source++;
		*Dest = *Source;
		Dest++;
		Source++;
		*Dest = *Source;
		Dest++;
		Source++;
		*Dest = *Source;
		Dest++;
		Source++;
		*Dest = *Source;
		Dest++;
		Source++;
		*Dest = *Source;
		Dest++;
		Source++;

		Many64--;
	}
}




/*****************************************************************************/
/* Scrolls the display to black.                                             */
/*****************************************************************************/
static void doScrollBlack(void) {
	byte *mem, *tempmem;
	struct Image Im;
	uint16 width, height, by, nheight, CurPage;
	uint32 size, copysize;
	uint32 *BaseAddr;

	mouseHide();
	width = VGAScaleX(320);
	height = VGAScaleY(149) + SVGACord(2);

	/*
	   while (EffectPlaying)
	   {
	    g_music->updateMusic();
	    waitTOF();
	   }
	 */

	allocFile((void **) &mem, (int32) width * (int32) height, "Temp Mem");
	/*
	   mem = stealBufMem((int32) width * (int32) height);
	 */
	Im.Width = width;
	Im.Height = height;
	Im.ImageData = mem;
	g_music->fillUpMusic(true);
	readScreenImage(&Im, 0, 0);
	g_music->fillUpMusic(true);

	BaseAddr = (uint32 *) getVGABaseAddr();

	by      = VGAScaleX(4);
	nheight = height;

	while (nheight) {
		g_music->newCheckMusic();

		if (!IsHiRes)
			waitTOF();

#if !defined(DOSCODE)
		BaseAddr = (uint32 *) getVGABaseAddr();
#endif

		if (by > nheight)
			by = nheight;

		mem += by * width;
		nheight -= by;
		size = (int32) nheight * (int32) width;
		CurPage = 0;
		tempmem = mem;

		while (size) {
			if (size > VGABytesPerPage)
				copysize = VGABytesPerPage;
			else
				copysize = size;

			size -= copysize;

			setPage(CurPage);
			copyLong64(BaseAddr, (uint32 *) tempmem, copysize >> 6);
			tempmem += copysize;
			CurPage++;
		}

		setAPen(0);
		rectFill(0, nheight, width - 1, nheight + by - 1);

		WSDL_UpdateScreen();

		if (!IsHiRes) {
			if (nheight <= (height / 8))
				by = 1;
			else if (nheight <= (height / 4))
				by = 2;
			else if (nheight <= (height / 2))
				by = 3;
		}
	}

	freeAllStolenMem();
	mouseShow();

#if !defined(DOSCODE)
	ungetVGABaseAddr();
#endif
}




extern struct BitMap RawDiffBM;
extern DIFFHeader headerdata;




static void copyPage(uint16 width, uint16 height, uint16 nheight, uint16 startline, byte *mem) {
	uint32 size, OffSet, copysize;
	uint16 CurPage;
	uint32 *BaseAddr;

	BaseAddr = (uint32 *) getVGABaseAddr();

	size = (int32)(height - nheight) * (int32) width;
	mem += startline * width;
	CurPage = ((int32) nheight * (int32) width) / VGABytesPerPage;
	OffSet = ((int32) nheight * (int32) width) - (CurPage * VGABytesPerPage);

	while (size) {
		if (size > (VGABytesPerPage - OffSet))
			copysize = VGABytesPerPage - OffSet;
		else
			copysize = size;

		size -= copysize;

		setPage(CurPage);
		copyLong64(BaseAddr + (OffSet >> 2), (uint32 *) mem, copysize >> 6);
		mem += copysize;
		CurPage++;
		OffSet = 0;
	}

#if !defined(DOSCODE)
	ungetVGABaseAddr();
#endif
}


/*****************************************************************************/
/* Scrolls the display to a new picture from a black screen.                 */
/*****************************************************************************/
static void doScrollWipe(char *filename) {
	byte *mem;
	uint16 width, height, by, nheight, startline = 0, onrow = 0;

	mouseHide();
	width = VGAScaleX(320);
	height = VGAScaleY(149) + SVGACord(2);

	while (EffectPlaying) {
		g_music->newCheckMusic();
		waitTOF();
	}

	IsBM = true;
	readPict(filename, true);
	VGASetPal(diffcmap, 256);
	IsBM = false;
	mem = RawDiffBM.Planes[0];

	g_music->fillUpMusic(true);
	by      = VGAScaleX(3);
	nheight = height;

	while (onrow < headerdata.y) {
		g_music->newCheckMusic();

		if ((by > nheight) && nheight)
			by = nheight;

		if ((startline + by) > (headerdata.y - height - 1))
			break;

		if (nheight)
			nheight -= by;

		copyPage(width, height, nheight, startline, mem);

		WSDL_UpdateScreen();

		if (!nheight)
			startline += by;

		onrow += by;

		if (nheight <= (height / 4))
			by = VGAScaleX(5);
		else if (nheight <= (height / 3))
			by = VGAScaleX(4);
		else if (nheight <= (height / 2))
			by = VGAScaleX(3);
	}

	mouseShow();
}




/*****************************************************************************/
/* Does the scroll bounce.  Assumes bitmap already in memory.                */
/*****************************************************************************/
static void doScrollBounce(void) {
	byte *mem;
	uint16 width, height, startline, counter,
#if defined(DOSCODE)
	       newby[5] = {5, 4, 3, 2, 1}, newby1[8] = {3, 3, 2, 2, 2, 1, 1, 1};
#else
	       newby[5] = {10, 8, 6, 4, 2}, newby1[8] = {6, 6, 4, 4, 4, 2, 2, 2};
#endif

	mouseHide();
	width = VGAScaleX(320);
	height = VGAScaleY(149) + SVGACord(2);
	mem = RawDiffBM.Planes[0];

	g_music->fillUpMusic(true);
	startline = headerdata.y - height - 1;

	for (counter = 0; counter < 5; counter++) {
		g_music->newCheckMusic();
		startline -= newby[counter];
		copyPage(width, height, 0, startline, mem);

		WSDL_UpdateScreen();
		waitTOF();
	}

	for (counter = 8; counter > 0; counter--) {
		g_music->newCheckMusic();
		startline += newby1[counter - 1];
		copyPage(width, height, 0, startline, mem);

		WSDL_UpdateScreen();
		waitTOF();

	}

	mouseShow();
}



/*****************************************************************************/
/* Does the transporter wipe.                                                */
/*****************************************************************************/
static void doTransWipe(CloseDataPtr *CPtr, char *filename) {
	uint16 LastY, CurY, counter, linesdone = 0, lineslast;
	struct Image ImSource, ImDest;

	if (IsHiRes) {
		lineslast = 3;
		LastY = 358;
	} else {
		lineslast = 1;
		LastY = 148;
	}

	for (counter = 0; counter < 2; counter++) {
		CurY = counter * 2;

		while (CurY < LastY) {
			if (linesdone >= lineslast) {
				g_music->updateMusic();
				waitTOF();
				linesdone = 0;
			}

			ghoastRect(0, 0, CurY, VGAScreenWidth - 1, CurY + 1);
			CurY += 4;
			linesdone++;
		}
	}

	setAPen(0);

	for (counter = 0; counter < 2; counter++) {
		CurY = counter * 2;

		while (CurY <= LastY) {
			if (linesdone >= lineslast) {
				g_music->updateMusic();
				waitTOF();
				linesdone = 0;
			}

			rectFill(0, CurY, VGAScreenWidth - 1, CurY + 1);
			CurY += 4;
			linesdone++;
		}
	}

	if (filename == NULL)
		CurFileName = getPictName(CPtr);
	else if (filename[0] > ' ')
		CurFileName = filename;
	else
		CurFileName = getPictName(CPtr);

	byte *BitMapMem = readPictToMem(CurFileName, VGAScreenWidth, LastY + 5);
	VGASetPal(diffcmap, 256);

	if (BitMapMem) {
		ImSource.Width = VGAScreenWidth;
		ImSource.Height = LastY;
		ImSource.ImageData = BitMapMem;

		ImDest.Width = VGAScreenWidth;
		ImDest.Height = VGAScreenHeight;
		ImDest.ImageData = getVGABaseAddr();

		for (counter = 0; counter < 2; counter++) {
			CurY = counter * 2;

			while (CurY < LastY) {
				if (linesdone >= lineslast) {
					g_music->updateMusic();
					waitTOF();
					linesdone = 0;
				}

#if !defined(DOSCODE)
				ImDest.ImageData = getVGABaseAddr();
#endif

				bltBitMap(&ImSource, 0, CurY, &ImDest, 0, CurY, VGAScreenWidth, 2);
				ghoastRect(0, 0, CurY, VGAScreenWidth - 1, CurY + 1);
				CurY += 4;
				linesdone++;
			}
		}

		for (counter = 0; counter < 2; counter++) {
			CurY = counter * 2;

			while (CurY <= LastY) {
				if (linesdone >= lineslast) {
					g_music->updateMusic();
					waitTOF();
					linesdone = 0;
				}

#if !defined(DOSCODE)
				ImDest.ImageData = getVGABaseAddr();
#endif

				if (CurY == LastY)
					bltBitMap(&ImSource, 0, CurY, &ImDest, 0, CurY, VGAScreenWidth, 1);
				else
					bltBitMap(&ImSource, 0, CurY, &ImDest, 0, CurY, VGAScreenWidth, 2);

				CurY += 4;
				linesdone++;
			}
		}
	}

#if !defined(DOSCODE)
	ungetVGABaseAddr();
#endif
}



/*****************************************************************************/
/* Does a certain number of pre-programmed wipes.                            */
/*****************************************************************************/
void doWipe(uint16 WipeType, CloseDataPtr *CPtr, char *filename) {
	if ((WipeType == TRANSWIPE) || (WipeType == TRANSPORTER))
		doTransWipe(CPtr, filename);
	else if (WipeType == SCROLLWIPE)
		doScrollWipe(filename);
	else if (WipeType == SCROLLBLACK)
		doScrollBlack();
	else if (WipeType == SCROLLBOUNCE)
		doScrollBounce();
	else if (WipeType == READFIRSTFRAME)
		readPict(filename, false);
	else if (WipeType == READNEXTFRAME)
		diffNextFrame();
}

} // End of namespace Lab
