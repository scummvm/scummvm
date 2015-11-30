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
#include "lab/diff.h"
#include "lab/labfun.h"
#include "lab/mouse.h"

namespace Lab {

static bool PlayOnce = false;
static bool StopPlayingEnd = false;

static uint32 header, size, WaitSec = 0L, WaitMicros = 0L, DelayMicros = 0L;
static uint16 CurBit = 0, framenumber = 0, samplespeed, numchunks = 1;
static byte *Buffer, temp[5];
static bool donepal = false;
static byte *storagefordifffile, **difffile = &storagefordifffile;
static byte *start;
static uint32 diffwidth, diffheight;
static byte blackbuffer[256 * 3];

bool DoBlack        = false,     /* Black the screen before new picture  */
     nopalchange    = false,     /* Don't change the palette.            */
     IsBM           = false,     /* Just fill in the RawDIFFBM structure */
     stopsound      = false,
     waitForEffect  = false;     /* Wait for each sound effect to finish
                                   before coninuing.                    */

static bool continuous,
	IsPlaying      = false,
	IsAnim         = false,
	IsPal          = false;

uint16 DataBytesPerRow;
DIFFHeader headerdata;
char diffcmap[256 * 3];
BitMap RawDiffBM;

extern BitMap *DispBitMap, *DrawBitMap;
extern byte **startoffile;

#define CONTINUOUS      0xFFFF


/*****************************************************************************/
/* Does the undiffing between the bitmaps.                                   */
/*****************************************************************************/
void unDiff(byte *NewBuf, byte *OldBuf, byte *DiffData, uint16 bytesperrow, bool IsV) {
	byte buftype;

	DiffData++;
	buftype  = *DiffData;
	DiffData++;

	if (IsV)
		VUnDIFFMemory(NewBuf, DiffData, 1, buftype + 1, bytesperrow);
	else
		unDIFFMemory(NewBuf, DiffData, 1, buftype + 1);
}


/*****************************************************************************/
/* Changes the front screen to black.                                        */
/*****************************************************************************/
void blackScreen() {
	memset(blackbuffer, 0, 248 * 3);
	g_lab->writeColorRegs(blackbuffer, 8, 248);

	g_system->delayMillis(32);
}

/*****************************************************************************/
/* Changes the front screen to white.                                        */
/*****************************************************************************/
void whiteScreen() {
	memset(blackbuffer, 255, 248 * 3);
	g_lab->writeColorRegs(blackbuffer, 8, 248);
}

/*****************************************************************************/
/* Changes the entire screen to black.                                       */
/*****************************************************************************/
void blackAllScreen() {
	memset(blackbuffer, 0, 256 * 3);
	g_lab->writeColorRegs(blackbuffer, 0, 256);

	g_system->delayMillis(32);
}

void LabEngine::diffNextFrame() {
	if (header == 65535)  /* Already done. */
		return;

	if (DispBitMap->Flags & BITMAPF_VIDEO) {
		DispBitMap->Planes[0] = getVGABaseAddr();
		DispBitMap->Planes[1] = DispBitMap->Planes[0] + 0x10000;
		DispBitMap->Planes[2] = DispBitMap->Planes[1] + 0x10000;
		DispBitMap->Planes[3] = DispBitMap->Planes[2] + 0x10000;
		DispBitMap->Planes[4] = DispBitMap->Planes[3] + 0x10000;
	}

	_event->mouseHide();

	while (1) {
		if (CurBit >= numchunks) {
			_event->mouseShow();

			if (!IsBM) {
				if (headerdata.fps) {
					waitForTime(WaitSec, WaitMicros);
					addCurTime(0L, DelayMicros, &WaitSec, &WaitMicros);
				}

				if (IsPal && !nopalchange) {
					VGASetPal(diffcmap, 256);
					IsPal = false;
				}

				donepal = true;
			}

			if (IsPal && !nopalchange && !IsBM && !donepal) {
				VGASetPal(diffcmap, 256);
				IsPal = false;
			}

			donepal = false;

			framenumber++;

			if ((framenumber == 1) && (continuous || (!PlayOnce)))
				Buffer = *difffile;

			IsAnim = (framenumber >= 3) && (!PlayOnce);
			CurBit = 0;

			if (DispBitMap->Flags & BITMAPF_VIDEO)
				WSDL_UpdateScreen();

			return; /* done with the next frame. */
		}

		_music->updateMusic();
		readBlock(&header, 4L, difffile);
		swapULong(&header);
		readBlock(&size, 4L, difffile);
		swapULong(&size);

		switch (header) {
		case 8L:
			readBlock(diffcmap, size, difffile);
			IsPal = true;
			break;

		case 10L:
			RawDiffBM.Planes[CurBit] = *difffile;

			if (IsBM)
				(*difffile) += size;
			else {
				readBlock(DrawBitMap->Planes[CurBit], size, difffile);
			}

			CurBit++;
			break;

		case 11L:
			(*difffile) += 4;
			runLengthDecode(DrawBitMap->Planes[CurBit], *difffile);
			CurBit++;
			(*difffile) += size - 4;
			break;

		case 12L:
			(*difffile) += 4;
			VRunLengthDecode(DrawBitMap->Planes[CurBit], *difffile, DrawBitMap->BytesPerRow);
			CurBit++;
			(*difffile) += size - 4;
			break;

		case 20L:
			unDiff(DrawBitMap->Planes[CurBit], DispBitMap->Planes[CurBit], *difffile, DrawBitMap->BytesPerRow, false);
			CurBit++;
			(*difffile) += size;
			break;

		case 21L:
			unDiff(DrawBitMap->Planes[CurBit], DispBitMap->Planes[CurBit], *difffile, DrawBitMap->BytesPerRow, true);
			CurBit++;
			(*difffile) += size;
			break;

		case 25L:
			CurBit++;
			break;

		case 26L:
			CurBit++;
			break;

		case 30L:
		case 31L: {
			if (waitForEffect) {
				while (_music->isSoundEffectActive()) {
					_music->updateMusic();
					waitTOF();
				}
			}

			size -= 8L;


			(*difffile) += 4;
			readBlock(&samplespeed, 2L, difffile);
			swapUShortPtr(&samplespeed, 1);
			(*difffile) += 2;

			byte *music = *difffile;
			uint32 musicsize = size;
			(*difffile) += size;

			_music->playSoundEffect(samplespeed, musicsize, music);
			break;
		}
		case 65535L:
			if ((framenumber == 1) || PlayOnce || StopPlayingEnd) {
				int didTOF = 0;

				if (waitForEffect) {
					while (_music->isSoundEffectActive()) {
						_music->updateMusic();
						waitTOF();

						if (DispBitMap->Flags & BITMAPF_VIDEO)
							didTOF = 1;
					}
				}

				IsPlaying = false;
				_event->mouseShow();

				if (!didTOF)
					WSDL_UpdateScreen();

				return;
			}

			framenumber = 4;  /* Random frame number so it never gets back to 2 */
			*difffile = Buffer;
			break;

		default:
			(*difffile) += size;
			break;
		}
	}
}

/*****************************************************************************/
/* A separate task launched by readDiff.  Plays the DIFF.                    */
/*****************************************************************************/
void playDiff() {
	WaitSec   = 0L;
	WaitMicros = 0L;
	DelayMicros = 0L;
	header      = 0;
	CurBit = 0;
	framenumber = 0;
	numchunks   = 1;
	donepal     = false;
	StopPlayingEnd = false;
	difffile    = &storagefordifffile;

	IsPlaying   = true;

	if (DoBlack) {
		DoBlack = false;
		blackScreen();
	}

	start = *startoffile;            /* Make a copy of the pointer to the start of the file    */
	*difffile = start;               /* Now can modify the file without modifying the original */

	if (start == NULL) {
		IsPlaying = false;
		return;
	}

	continuous = false;
	readBlock(temp, 4L, difffile);
	temp[4] = '\0';

	readBlock(&header, 4L, difffile);
	swapULong(&header);

	if (!((strcmp((char *)temp, "DIFF") == 0) && (header == 1219009121L))) {
		IsPlaying = false;
		return;
	}

	readBlock(&header, 4L, difffile);
	swapULong(&header);

	readBlock(&size, 4L, difffile);
	swapULong(&size);

	if (header == 0) {
		// sizeof(headerdata) != 18, but the padding might be at the end
		readBlock(&headerdata.Version, 2, difffile);
		readBlock(&headerdata.x, 2, difffile);
		readBlock(&headerdata.y, 2, difffile);
		readBlock(&headerdata.depth, 1, difffile);
		readBlock(&headerdata.fps, 1, difffile);
		readBlock(&headerdata.BufferSize, 4, difffile);
		readBlock(&headerdata.Machine, 2, difffile);
		readBlock(&headerdata.Flags, 4, difffile);

		(*difffile) += size - 18;

		swapUShortPtr(&headerdata.Version, 3);
		swapULong(&headerdata.BufferSize);
		swapUShortPtr(&headerdata.Machine, 1);
		swapULong(&headerdata.Flags);

		continuous = CONTINUOUS & headerdata.Flags;
		diffwidth = headerdata.x;
		diffheight = headerdata.y;
		DataBytesPerRow = diffwidth;

		numchunks = (((int32) diffwidth) * diffheight) / 0x10000;

		if ((uint32)(numchunks * 0x10000) < (uint32)(((int32) diffwidth) * diffheight))
			numchunks++;
	} else {
		return;
	}

	for (header = 0; header < 8; header++)
		RawDiffBM.Planes[header] = NULL;

	if (headerdata.fps)
		DelayMicros = ONESECOND / headerdata.fps;

	if (PlayOnce) {
		while (header != 65535)
			g_lab->diffNextFrame();
	} else
		g_lab->diffNextFrame();
}



/*****************************************************************************/
/* Stops an animation from running.                                          */
/*****************************************************************************/
void stopDiff() {
	if (IsPlaying) {
		if (IsAnim)
			blackScreen();
	}
}

/*****************************************************************************/
/* Stops an animation from running.                                          */
/*****************************************************************************/
void stopDiffEnd() {
	if (IsPlaying) {
		StopPlayingEnd = true;
		while (IsPlaying) {
			g_lab->_music->updateMusic();
			g_lab->diffNextFrame();
		}
	}
}


/*****************************************************************************/
/* Stops the continuous sound from playing.                                  */
/*****************************************************************************/
void stopSound() {
	stopsound = true;
}

/*****************************************************************************/
/* Reads in a DIFF file.                                                     */
/*****************************************************************************/
bool readDiff(bool playonce) {
	PlayOnce = playonce;
	playDiff();
	return true;
}


void readSound(bool waitTillFinished, Common::File *file) {
	uint32 magicBytes = file->readUint32LE();
	if (magicBytes != 1219009121L)
		return;

	uint32 soundTag = file->readUint32LE();
	uint32 soundSize = file->readUint32LE();

	if (soundTag == 0)
		file->skip(soundSize);	// skip the header
	else
		return;

	while (soundTag != 65535) {
		g_lab->_music->updateMusic();
		soundTag = file->readUint32LE();
		soundSize = file->readUint32LE() - 8;

		if ((soundTag == 30) || (soundTag == 31)) {
			if (waitTillFinished) {
				while (g_lab->_music->isSoundEffectActive()) {
					g_lab->_music->updateMusic();
					g_lab->waitTOF();
				}
			}

			file->skip(4);

			uint16 sampleRate = file->readUint16LE();
			file->skip(2);
			byte *soundData = (byte *)malloc(soundSize);
			file->read(soundData, soundSize);
			g_lab->_music->playSoundEffect(sampleRate, soundSize, soundData);
		} else if (soundTag == 65535L) {
			if (waitTillFinished) {
				while (g_lab->_music->isSoundEffectActive()) {
					g_lab->_music->updateMusic();
					g_lab->waitTOF();
				}
			}
		} else
			file->skip(soundSize);
	}
}

} // End of namespace Lab
