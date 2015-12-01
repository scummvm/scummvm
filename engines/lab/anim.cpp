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

#include "common/endian.h"
#include "lab/lab.h"

namespace Lab {

extern uint16 _dataBytesPerRow;
extern BitMap *DrawBitMap;
extern byte **startoffile;
extern BitMap *DispBitMap;

Anim::Anim(LabEngine *vm) : _vm(vm) {
	header = 0;
	CurBit = 0;
	numchunks = 1;
	IsBM = false;
	headerdata._width = 0;
	headerdata._height = 0;
	headerdata._fps = 0;
	headerdata._flags = 0;
	WaitSec = 0;
	WaitMicros = 0;
	DelayMicros = 0;
	continuous = false;
	IsPlaying = false;
	IsAnim = false;
	IsPal = false;
	nopalchange = false;
	donepal = false;
	framenumber = 0;
	PlayOnce = false;
	Buffer = nullptr;
	storagefordifffile = nullptr;
	difffile = &storagefordifffile;
	size = 0;
	RawDiffBM._bytesPerRow = 0;
	RawDiffBM._flags = 0;
	for (int i = 0; i < 16; i++)
		RawDiffBM._planes[i] = nullptr;
	RawDiffBM._rows = 0;
	waitForEffect = false;
	StopPlayingEnd = false;
	samplespeed = 0;
	DoBlack = false;
	start = nullptr;
	diffwidth = 0;
	diffheight = 0;
	stopsound = false;


	for (int i = 0; i < 3 * 256; i++)
		diffcmap[i] = 0;

}

/*------------------------ unDiff Horizontal Memory -------------------------*/

/*****************************************************************************/
/* Undiffs a piece of memory when header size is a byte, and copy/skip size  */
/* is also a byte.                                                           */
/*****************************************************************************/
static void unDIFFByteByte(byte *dest, byte *diff) {
	uint16 skip, copy;

	while (1) {
		skip = *diff;
		diff++;
		copy = *diff;
		diff++;

		if (skip == 255) {
			if (copy == 0) {
				skip = READ_LE_UINT16(diff);
				diff += 2;
				copy = READ_LE_UINT16(diff);
				diff += 2;
			} else if (copy == 255)
				return;
		}

		dest += skip;
		memcpy(dest, diff, copy);
		dest += copy;
		diff += copy;
	}
}

/*****************************************************************************/
/* Undiffs a piece of memory when header size is a byte, and copy/skip size  */
/* is a word.                                                                */
/*****************************************************************************/
static void unDIFFByteWord(uint16 *dest, uint16 *diff) {
	uint16 skip, copy;

	while (1) {
		skip = ((byte *)diff)[0];
		copy = ((byte *)diff)[1];

		diff++;

		if (skip == 255) {
			if (copy == 0) {
				skip = READ_LE_UINT16(diff);
				diff++;
				copy = READ_LE_UINT16(diff);
				diff++;
			} else if (copy == 255)
				return;
		}

		dest += skip;

		while (copy > 3) {
			*dest = READ_LE_UINT16(diff);
			dest++;
			diff++;

			*dest = READ_LE_UINT16(diff);
			dest++;
			diff++;

			*dest = READ_LE_UINT16(diff);
			dest++;
			diff++;

			*dest = READ_LE_UINT16(diff);
			dest++;
			diff++;

			copy -= 4;
		}

		while (copy) {
			*dest = READ_LE_UINT16(diff);
			dest++;
			diff++;
			copy--;
		}
	}
}

/*****************************************************************************/
/* UnDiffs a coded DIFF string onto an already initialized piece of memory.  */
/*****************************************************************************/
bool Anim::unDIFFMemory(byte *dest, byte *diff, uint16 headerSize, uint16 copySize) {
	if (headerSize == 1) {
		if (copySize == 1)
			unDIFFByteByte(dest, diff);

		else if (copySize == 2)
			unDIFFByteWord((uint16 *)dest, (uint16 *)diff);

		else
			return false;
	} else
		error("unDIFFMemory: HeaderSize is %d", headerSize);

	return true;
}

/*------------------------- unDiff Vertical Memory --------------------------*/

/*****************************************************************************/
/* Undiffs a piece of memory when header size is a byte, and copy/skip size  */
/* is a byte.                                                                */
/*****************************************************************************/
static void VUnDIFFByteByte(byte *Dest, byte *diff, uint16 bytesperrow) {
	byte *CurPtr;
	uint16 skip, copy;
	uint16 counter = 0;


	while (counter < _dataBytesPerRow) {
		CurPtr = Dest + counter;

		for (;;) {
			skip = *diff;
			diff++;
			copy = *diff;
			diff++;

			if (skip == 255) {
				counter += copy;
				break;
			}

			else {
				CurPtr += (skip * bytesperrow);

				while (copy) {
					copy--;
					*CurPtr = *diff;
					CurPtr += bytesperrow;
					diff++;
				}
			}
		}
	}
}

/*****************************************************************************/
/* Undiffs a piece of memory when header size is a byte, and copy/skip size  */
/* is a word.                                                                */
/*****************************************************************************/
static void VUnDIFFByteWord(uint16 *Dest, uint16 *diff, uint16 bytesperrow) {
	uint16 *CurPtr;
	uint16 skip, copy;
	uint16 counter = 0, wordsperrow;


	wordsperrow = bytesperrow / 2;

	while (counter < (_dataBytesPerRow >> 1)) {
		CurPtr = Dest + counter;

		for (;;) {
			skip = ((byte *)diff)[0];
			copy = ((byte *)diff)[1];

			diff++;


			if (skip == 255) {
				counter += copy;
				break;
			}

			else {
				CurPtr += (skip * wordsperrow);

				while (copy) {
					*CurPtr = *diff; //swapUShort(*diff);
					CurPtr += wordsperrow;
					diff++;
					copy--;
				}
			}
		}
	}
}

/*****************************************************************************/
/* Undiffs a piece of memory when header size is a byte, and copy/skip size  */
/* is a long.                                                                */
/*****************************************************************************/
static void VUnDIFFByteLong(uint32 *Dest, uint32 *diff, uint16 bytesperrow) {
	uint32 *CurPtr;
	uint16 skip, copy;
	uint16 counter = 0, longsperrow;
	byte *diff1 = (byte *)diff;


	longsperrow = bytesperrow / 4;

	while (counter < (_dataBytesPerRow >> 2)) {
		CurPtr = Dest + counter;

		for (;;) {
			skip = *diff1;
			diff1++;

			copy = *diff1;
			diff1++;


			if (skip == 255) {
				counter += copy;
				break;
			}

			else {
				CurPtr += (skip * longsperrow);

				while (copy) {
					*CurPtr = *(uint32 *)diff1; //swapULong(*diff);
					CurPtr += longsperrow;
					diff1 += 4;
					copy--;
				}
			}
		}
	}
}

/*****************************************************************************/
/* UnDiffs a coded DIFF string onto an already initialized piece of memory.  */
/*****************************************************************************/
bool Anim::VUnDIFFMemory(byte *Dest, byte *diff, uint16 HeaderSize, uint16 CopySize, uint16 bytesperrow) {
	if (HeaderSize == 1) {
		if (CopySize == 1)
			VUnDIFFByteByte(Dest, diff, bytesperrow);

		else if (CopySize == 2)
			VUnDIFFByteWord((uint16 *)Dest, (uint16 *)diff, bytesperrow);

		else if (CopySize == 4)
			VUnDIFFByteLong((uint32 *)Dest, (uint32 *)diff, bytesperrow);

		else
			return false;
	} else
		return (false);

	return true;
}

/*****************************************************************************/
/* Runlength decodes a chunk of memory.                                      */
/*****************************************************************************/
void Anim::runLengthDecode(byte *Dest, byte *Source) {
	int8 num;
	int16 count;


	while (1) {
		num = (int8)*Source;
		Source++;

		if (num == 127) {
			return;
		} else if (num > '\0') {
			memcpy(Dest, Source, num);
			Source += num;
			Dest   += num;
		} else {
			count = (int16)(-num);
			num   = *Source;
			Source++;

			while (count) {
				*Dest = num;
				Dest++;
				count--;
			}
		}
	}
}

/*****************************************************************************/
/* Does a vertical run length decode.                                        */
/*****************************************************************************/
void Anim::VRunLengthDecode(byte *Dest, byte *Source, uint16 bytesperrow) {
	int8 num;
	int16 count;
	byte *Top = Dest;

	for (uint16 i = 0; i < _dataBytesPerRow; i++) {
		Dest = Top;
		Dest += i;

		num = (int8)*Source;
		Source++;

		while (num != 127) {
			if (num > '\0') {
				while (num) {
					*Dest = *Source;
					Source++;
					Dest += bytesperrow;
					num--;
				}
			} else {
				count = (int16)(-num);
				num   = (int8)*Source;
				Source++;

				while (count) {
					*Dest = num;
					Dest += bytesperrow;
					count--;
				}
			}

			num = *Source;
			Source++;
		}
	}
}

/*****************************************************************************/
/* Does the undiffing between the bitmaps.                                   */
/*****************************************************************************/
void Anim::unDiff(byte *NewBuf, byte *OldBuf, byte *DiffData, uint16 bytesperrow, bool IsV) {
	byte buftype;

	DiffData++;
	buftype  = *DiffData;
	DiffData++;

	if (IsV)
		VUnDIFFMemory(NewBuf, DiffData, 1, buftype + 1, bytesperrow);
	else
		unDIFFMemory(NewBuf, DiffData, 1, buftype + 1);
}

void Anim::diffNextFrame() {
	if (header == 65535)  /* Already done. */
		return;

	if (DispBitMap->_flags & BITMAPF_VIDEO) {
		DispBitMap->_planes[0] = g_lab->getCurrentDrawingBuffer();
		DispBitMap->_planes[1] = DispBitMap->_planes[0] + 0x10000;
		DispBitMap->_planes[2] = DispBitMap->_planes[1] + 0x10000;
		DispBitMap->_planes[3] = DispBitMap->_planes[2] + 0x10000;
		DispBitMap->_planes[4] = DispBitMap->_planes[3] + 0x10000;
	}

	_vm->_event->mouseHide();

	while (1) {
		if (CurBit >= numchunks) {
			_vm->_event->mouseShow();

			if (!IsBM) {
				if (headerdata._fps) {
					_vm->waitForTime(WaitSec, WaitMicros);
					_vm->addCurTime(0L, DelayMicros, &WaitSec, &WaitMicros);
				}

				if (IsPal && !nopalchange) {
					_vm->setPalette(diffcmap, 256);
					IsPal = false;
				}

				donepal = true;
			}

			if (IsPal && !nopalchange && !IsBM && !donepal) {
				_vm->setPalette(diffcmap, 256);
				IsPal = false;
			}

			donepal = false;

			framenumber++;

			if ((framenumber == 1) && (continuous || (!PlayOnce)))
				Buffer = *difffile;

			IsAnim = (framenumber >= 3) && (!PlayOnce);
			CurBit = 0;

			if (DispBitMap->_flags & BITMAPF_VIDEO)
				_vm->screenUpdate();

			return; /* done with the next frame. */
		}

		_vm->_music->updateMusic();
		header = READ_LE_UINT32(*difffile);
		*difffile += 4;

		size = READ_LE_UINT32(*difffile);
		*difffile += 4;

		switch (header) {
		case 8L:
			readBlock(diffcmap, size, difffile);
			IsPal = true;
			break;

		case 10L:
			RawDiffBM._planes[CurBit] = *difffile;

			if (IsBM)
				(*difffile) += size;
			else {
				readBlock(DrawBitMap->_planes[CurBit], size, difffile);
			}

			CurBit++;
			break;

		case 11L:
			(*difffile) += 4;
			runLengthDecode(DrawBitMap->_planes[CurBit], *difffile);
			CurBit++;
			(*difffile) += size - 4;
			break;

		case 12L:
			(*difffile) += 4;
			VRunLengthDecode(DrawBitMap->_planes[CurBit], *difffile, DrawBitMap->_bytesPerRow);
			CurBit++;
			(*difffile) += size - 4;
			break;

		case 20L:
			unDiff(DrawBitMap->_planes[CurBit], DispBitMap->_planes[CurBit], *difffile, DrawBitMap->_bytesPerRow, false);
			CurBit++;
			(*difffile) += size;
			break;

		case 21L:
			unDiff(DrawBitMap->_planes[CurBit], DispBitMap->_planes[CurBit], *difffile, DrawBitMap->_bytesPerRow, true);
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
				while (_vm->_music->isSoundEffectActive()) {
					_vm->_music->updateMusic();
					_vm->waitTOF();
				}
			}

			size -= 8L;


			(*difffile) += 4;
			samplespeed = READ_LE_UINT16(*difffile);
			(*difffile) += 4;

			byte *music = *difffile;
			uint32 musicsize = size;
			(*difffile) += size;

			_vm->_music->playSoundEffect(samplespeed, musicsize, music);
			break;
				  }
		case 65535L:
			if ((framenumber == 1) || PlayOnce || StopPlayingEnd) {
				int didTOF = 0;

				if (waitForEffect) {
					while (_vm->_music->isSoundEffectActive()) {
						_vm->_music->updateMusic();
						_vm->waitTOF();

						if (DispBitMap->_flags & BITMAPF_VIDEO)
							didTOF = 1;
					}
				}

				IsPlaying = false;
				_vm->_event->mouseShow();

				if (!didTOF)
					_vm->screenUpdate();

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
void Anim::playDiff() {
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
	uint32 signature = READ_BE_UINT32(*difffile);
	(*difffile) += 4;

	header = READ_LE_UINT32(*difffile);
	(*difffile) += 4;

	if ((signature != MKTAG('D', 'I', 'F', 'F')) || (header != 1219009121L)) {
		IsPlaying = false;
		return;
	}

	header = READ_LE_UINT32(*difffile);
	(*difffile) += 4;

	size = READ_LE_UINT32(*difffile);
	(*difffile) += 4;

	if (header == 0) {
		// sizeof(headerdata) != 18, but the padding might be at the end
		headerdata._version = READ_LE_UINT16(*difffile);
		(*difffile) += 2;
		headerdata._width = READ_LE_UINT16(*difffile);
		(*difffile) += 2;
		headerdata._height = READ_LE_UINT16(*difffile);
		(*difffile) += 2;
		headerdata._depth = *difffile[0];
		(*difffile)++;
		headerdata._fps = *difffile[0];
		(*difffile)++;
		headerdata._bufferSize = READ_LE_UINT32(*difffile);
		(*difffile) += 4;
		headerdata._machine = READ_LE_UINT16(*difffile);
		(*difffile) += 2;
		headerdata._flags = READ_LE_UINT32(*difffile);
		(*difffile) += 4;

		(*difffile) += size - 18;

		continuous = CONTINUOUS & headerdata._flags;
		diffwidth = headerdata._width;
		diffheight = headerdata._height;
		_dataBytesPerRow = diffwidth;

		numchunks = (((int32) diffwidth) * diffheight) / 0x10000;

		if ((uint32)(numchunks * 0x10000) < (uint32)(((int32) diffwidth) * diffheight))
			numchunks++;
	} else {
		return;
	}

	for (header = 0; header < 8; header++)
		RawDiffBM._planes[header] = NULL;

	if (headerdata._fps)
		DelayMicros = ONESECOND / headerdata._fps;

	if (PlayOnce) {
		while (header != 65535)
			diffNextFrame();
	} else
		diffNextFrame();
}

/*****************************************************************************/
/* Stops an animation from running.                                          */
/*****************************************************************************/
void Anim::stopDiff() {
	if (IsPlaying) {
		if (IsAnim)
			blackScreen();
	}
}

/*****************************************************************************/
/* Stops an animation from running.                                          */
/*****************************************************************************/
void Anim::stopDiffEnd() {
	if (IsPlaying) {
		StopPlayingEnd = true;
		while (IsPlaying) {
			g_lab->_music->updateMusic();
			diffNextFrame();
		}
	}
}

/*****************************************************************************/
/* Stops the continuous sound from playing.                                  */
/*****************************************************************************/
void Anim::stopSound() {
	stopsound = true;
}

/*****************************************************************************/
/* Reads in a DIFF file.                                                     */
/*****************************************************************************/
bool Anim::readDiff(bool playonce) {
	PlayOnce = playonce;
	playDiff();
	return true;
}

} // End of namespace Lab
