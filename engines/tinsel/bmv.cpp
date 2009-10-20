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
 * The movie player.
 */

#include "common/file.h"
#include "sound/mixer.h"
#include "sound/audiostream.h"
#include "tinsel/tinsel.h"
#include "tinsel/background.h"
#include "tinsel/cliprect.h"
#include "tinsel/coroutine.h"
#include "tinsel/config.h"
#include "tinsel/dw.h"
#include "tinsel/events.h"
#include "tinsel/font.h"
#include "tinsel/graphics.h"
#include "tinsel/handle.h"
#include "tinsel/heapmem.h"
#include "tinsel/multiobj.h"
#include "tinsel/object.h"
#include "tinsel/palette.h"
#include "tinsel/sched.h"
#include "tinsel/strres.h"
#include "tinsel/text.h"
#include "tinsel/timers.h"
#include "tinsel/tinlib.h"
#include "tinsel/tinsel.h"

namespace Tinsel {

//----------------- GLOBAL GLOBAL DATA ------------------------

bool bOldAudio;

//----------------- LOCAL GLOBAL DATA ------------------------

//static READREQ rr;

//----------------- LOCAL DEFINES ----------------------------

#define SZ_C_BLOB	65
#define SZ_U_BLOB	128

#define BLANK_SOUND	0x0	// for 16 bit silence

#define PT_A	20	// Number of times PT_B may be reached
#define PT_B	6


#define SLOT_SIZE	(25*1024)
//#define NUM_SLOTS	168
#define NUM_SLOTS	122		// -> ~ 3MB


#define PREFETCH	(NUM_SLOTS/2)	// For initial test

#ifndef _Windows
//#define ADVANCE_SOUND		12	// 1 second
#define ADVANCE_SOUND		18	// 1 1/2 second
//#define MAX_ADVANCE_SOUND	36	// 3 seconds
#else
#define ADVANCE_SOUND		18	// 1 1/2 seconds
#endif
#define SUBSEQUENT_SOUND	6	// 1/2 second



// PACKET TYPE IDs & FLAGS

#define CD_SLOT_NOP	0x00	// Skip to next slot
#define CD_LE_FIN	0x01	// End of movie
#define CD_PDELTA	0x02	// Image compressed to previous one
#define CD_SDELTA	0x03	// Image self-compressed

#define BIT0		0x01

#define CD_XSCR		0x04	// Screen has a scroll offset
#define CD_CMAP	0x08	// Colour map is included
#define CD_CMND	0x10	// Command is included
#define CD_AUDIO	0x20	// Audio data is included
#define CD_EXTEND	0x40	// Extended modes "A"-"z"
#define CD_PRINT	0x80	// goes in conjunction with CD_CMD

// Data field sizes
#define sz_XSCR_pkt		2
#define sz_CMAP_pkt		0x300
#define sz_CMD_TALK_pkt		10
#define sz_CMD_PRINT_pkt	8
#define sz_AUDIO_pkt		3675


typedef struct {

	short	x;
	short	y;
	short	stringId;
	unsigned char	duration;
	char	r;			// may be b!
	char	g;
	char	b;			// may be r!

} TALK_CMD, *PTALK_CMD;

typedef struct {

	int16	x;
	int16	y;
	int16	stringId;
	unsigned char	duration;
	unsigned char	fontId;

} PRINT_CMD, *PPRINT_CMD;


//----------------- LOCAL GLOBAL DATA ------------------------

// Set when a movie is on
static bool bMovieOn;

// Set to kill one off
static bool bAbort;

// For escaping out of movies
static int bmvEscape;

// Movie file pointer
static Common::File stream;

// Movie file name
static char szMovieFile[14];

// Pointers to buffers
static byte *bigBuffer; //, *screenBuffer;

// Next data to use to extract a frame
static int nextUseOffset;

// Next data to use to extract sound data
static int nextSoundOffset;

// When above offset gets to what this is set at, rewind
static int wrapUseOffset;

// The offset of the most future packet
static int mostFutureOffset;

// The current frame
static int currentFrame;
static int currentSoundFrame;

// Number of packets currently in RAM
static int numAdvancePackets;

// Next slot that will be read from disc
static int nextReadSlot;

// Set when the whole file has been read
static bool bFileEnd;

// Palette
static COLORREF moviePal[256];

static int blobsInBuffer;

static struct {

	POBJECT	pText;
	int	dieFrame;

} texts[2];

static COLORREF talkColour;

static int bigProblemCount;

static bool bIsText;

static int movieTick;
static int startTick;
static uint32 nextMovieTime = 0;

static uint16 Au_Prev1 = 0;
static uint16 Au_Prev2 = 0;
static byte *ScreenBeg;
static byte *screenBuffer;

static bool audioStarted;

static Audio::AppendableAudioStream *audioStream = 0;
static Audio::SoundHandle audioHandle;

const uint16 Au_DecTable[16] = {16512, 8256, 4128, 2064, 1032, 516, 258, 192,
		129, 88, 64, 56, 48, 40, 36, 32};

//---------------- DECOMPRESSOR FUNCTIONS --------------------

#define SCREEN_WIDE 640
#define SCREEN_HIGH 429
#define SAM_P_BLOB (32 * 2)

#define ROR(x,v) x = ((x >> (v%32)) | (x << (32 - (v%32))));
#define ROL(x,v) x = ((x << (v%32)) | (x >> (32 - (v%32))));
#define NEXT_BYTE(v) v = forwardDirection ? v + 1 : v - 1;

static void PrepBMV(const byte *sourceData, int length, short deltaFetchDisp) {
	uint8 NibbleHi = 0;
	const byte *saved_esi;
	uint32 eax = 0;
	uint32 edx = length;
	int32 ebx = deltaFetchDisp;
	uint32 ecx = 0;
	const byte *esi;
	byte *edi, *ebp;

	bool forwardDirection = (deltaFetchDisp <= -SCREEN_WIDE) || (deltaFetchDisp >= 0);
	if (forwardDirection) {
		// Forward decompression
		esi = sourceData;
		edi = ScreenBeg;
		ebp = ScreenBeg + SCREEN_WIDE * SCREEN_HIGH;
	} else {
		esi = sourceData + length - 1;
		edi = ScreenBeg + SCREEN_WIDE * SCREEN_HIGH - 1;
		ebp = ScreenBeg - 1;
	}

	bool firstLoop, flag;

	int loopCtr = 0;
	for (;;) {
		flag = false;

		if ((loopCtr == 0) || (edx == 4)) {
			// Get the next hi,lo nibble
			eax = (eax & 0xffffff00) | *esi;
			firstLoop = true;
		} else {
			// Get the high nibble
			eax = (eax & 0xffffff00) | (NibbleHi >> 4);
			firstLoop = false;
		}

		// Is lo nibble '00xx'?
		if ((eax & 0xC) == 0) {
			for (;;) {
//@_rDN_Lp_1:
				// Only execute this bit first the first time into the loop
				if (!firstLoop) {
					ROR(eax, 2);
					ecx += 2;
					eax = (eax & 0xffffff00) | *esi;

					if ((eax & 0xC) != 0)
						break;
				}
				firstLoop = false;

//@_rD2nd_1:
				ROR(eax, 2);		// Save bi-bit into hi 2 bits
				ecx += 2;			//   and increase bit-shifter
				// Shift another 2 bits to get hi nibble
				eax = (eax & 0xffffff00) | ((eax & 0xff) >> 2);
				NEXT_BYTE(esi);

				if ((eax & 0xC) != 0) {
					flag = true;
					ROL(eax, ecx);
					break;
				}
			}
		} else if (loopCtr != 0) {
			flag = edx != 4;
		}

		if (flag) {
//@_rdNum__1:
			edx = 4;			// offset rDNum_Lo ; Next nibble is a 'lo'
		} else {
// @_rDNum_1
			NibbleHi = (uint8)eax;
			edx = 0;			// offset rDNum_Hi ; Next nibble is a 'hi' (reserved)
			eax &= 0xffffff0f;
			NEXT_BYTE(esi);
			ROL(eax, ecx);
		}
//rDN_1:
//@_rD_or_R:
		bool actionFlag = (eax & 1) != 0;
		eax >>= 1;
		ecx = eax - 1;

		// Move to next loop index
		if (++loopCtr == 4) loopCtr = 1;

		if (actionFlag) {
			// Adjust loopCtr to fall into the correct processing case
			loopCtr = loopCtr % 3 + 1;
		}

		switch (loopCtr) {
		case 1:
			// @_rDelta:
			saved_esi = esi;			// Save the source pointer
			esi = edi + ebx;			// Point it to existing data

			while (ecx > 0) {
				*edi = *esi;
				NEXT_BYTE(esi);
				NEXT_BYTE(edi);
				--ecx;
			}

			esi = saved_esi;
			break;

		case 2:
			// @_rRaw
			// Copy data from source to dest
			while (ecx > 0) {
				*edi = *esi;
				NEXT_BYTE(esi);
				NEXT_BYTE(edi);
				--ecx;
			}
			break;

		case 3:
			// @_rRun
			// Repeating run of data
			eax = forwardDirection ? *(edi - 1) : *(edi + 1);

			while (ecx > 0) {
				*edi = (uint8)eax;
				NEXT_BYTE(edi);
				--ecx;
			}
			break;
		default:
			break;
		}

		if (edi == ebp)
			break;		// Exit if complete

		eax = 0;
	}
}

static void InitBMV(byte *memoryBuffer) {
	// Clear the two extra 'off-screen' rows
	memset(memoryBuffer, 0, SCREEN_WIDE);
	memset(memoryBuffer + SCREEN_WIDE * (SCREEN_HIGH + 1), 0, SCREEN_WIDE);

	if (audioStream) {
		_vm->_mixer->stopHandle(audioHandle);

		delete audioStream;
		audioStream = 0;
	}

	// Set the screen beginning to the second line (ie. past the off-screen line)
	ScreenBeg = memoryBuffer + SCREEN_WIDTH;
	Au_Prev1 = Au_Prev2 = 0;
}

void PrepAudio(const byte *sourceData, int blobCount, byte *destPtr) {
	uint16 dx1 = Au_Prev1;
	uint16 dx2 = Au_Prev2;

	uint16 *destP = (uint16 *)destPtr;
	const int8 *srcP = (const int8 *)sourceData;

	// Blob Loop
	while (blobCount-- > 0) {
		uint32 ebx = (uint8) *srcP++;
		uint32 ebp = ebx & 0x1E;

		int blobSize = SAM_P_BLOB / 2;

		ebx = (((ebx & 0x0F) << 4) | ((ebx & 0xF0) >> 4)) & 0x1E;

		ebp = Au_DecTable[ebp >> 1];
		ebx = Au_DecTable[ebx >> 1];

		// Inner loop
		while (blobSize-- > 0) {
			uint32 s1 = (((int32) *srcP++) * ((int32) ebp)) >> 5;
			uint32 s2 = (((int32) *srcP++) * ((int32) ebx)) >> 5;

			dx1 += s1 & 0xFFFF;
			dx2 += s2 & 0xFFFF;

			*destP++ = TO_BE_16(dx1);
			*destP++ = TO_BE_16(dx2);
		}
	}

	Au_Prev1 = dx1;
	Au_Prev2 = dx2;
}

//----------------- BMV FUNCTIONS ----------------------------

static bool MaintainBuffer(void);


/**
 * Called when a packet contains a palette field.
 * Build a COLORREF array and queue it to the DAC.
 */
static void MoviePalette(int paletteOffset) {
	int	i;
	byte *r;

	r = bigBuffer + paletteOffset;

	for (i = 0; i < 256; i++, r += 3)	{
		moviePal[i] = TINSEL_RGB(*r, *(r + 1), *(r + 2));
	}

	UpdateDACqueue(1, 255, &moviePal[1]);

	// Don't clobber talk
	if (talkColour != 0)
		SetTextPal(talkColour);
}

static void InitialiseMovieSound() {
	audioStream =
		Audio::makeAppendableAudioStream(22050,
				Audio::Mixer::FLAG_16BITS | Audio::Mixer::FLAG_STEREO);
	audioStarted = false;
}

static void StartMovieSound() {
}

static void FinishMovieSound() {
	if (audioStream) {
		_vm->_mixer->stopHandle(audioHandle);

		delete audioStream;
		audioStream = 0;
	}
}

/**
 * Called when a packet contains an audio field.
 */
static void MovieAudio(int audioOffset, int blobs) {
	if (audioOffset == 0 && blobs == 0)
		blobs = 57;

	byte *data = new byte[blobs * 128];

	if (audioOffset != 0)
		PrepAudio(bigBuffer+audioOffset, blobs, data);
	else
		memset(data, 0, blobs * 128);

	audioStream->queueBuffer(data, blobs * 128);

	if (currentSoundFrame == ADVANCE_SOUND) {
		if (!audioStarted) {
			_vm->_mixer->playInputStream(Audio::Mixer::kSFXSoundType,
					&audioHandle, audioStream, -1, Audio::Mixer::kMaxChannelVolume, 0, false);
			audioStarted = true;
		}
	}
}

/*-----------------------------------------------------*\
|-------------------------------------------------------|
\*-----------------------------------------------------*/

static void FettleMovieText(void) {
	int i;

	bIsText = false;

	for (i = 0; i < 2; i++) {
		if (texts[i].pText) {
			if (currentFrame > texts[i].dieFrame) {
				MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), texts[i].pText);
				texts[i].pText = NULL;
			} else {
				MultiForceRedraw(texts[i].pText);
				bIsText = true;
			}
		}
	}
}

/*-----------------------------------------------------*\
|-------------------------------------------------------|
\*-----------------------------------------------------*/

static void BmvDrawText(bool bDraw) {
	int	w, h, x, y;

	for (int i = 0; i < 2; i++) {
		if (texts[i].pText) {
			x = MultiLeftmost(texts[i].pText);
			y = MultiHighest(texts[i].pText);
			w = MIN(MultiRightmost(texts[i].pText) + 1, (int)SCREEN_WIDTH) - x;
			h = MIN(MultiLowest(texts[i].pText) + 1, SCREEN_HIGH) - y;

			const byte *src = ScreenBeg + (y * SCREEN_WIDTH) + x;
			byte *dest = (byte *)_vm->screen().getBasePtr(x, y);

			for (int j = 0; j < h; j++, dest += SCREEN_WIDTH, src += SCREEN_WIDTH) {
				memcpy(dest, src, w);
			}

			if (bDraw) {
				Common::Point ptWin;
				Common::Rect rcPlayClip;

				ptWin.x = ptWin.y = 0;
				rcPlayClip.left = x;
				rcPlayClip.top = y;
				rcPlayClip.right = x+w;
				rcPlayClip.bottom = y+h;
				UpdateClipRect(GetPlayfieldList(FIELD_STATUS), &ptWin,	&rcPlayClip);
			}
		}
	}
}

/*-----------------------------------------------------*\
|-------------------------------------------------------|
\*-----------------------------------------------------*/

static void MovieText(CORO_PARAM, int stringId, int x, int y, int fontId, COLORREF *pTalkColour, int duration) {
	SCNHANDLE hFont;
	int	index;

	if (fontId == 1) {
		// It's a 'print'

		hFont = GetTagFontHandle();
		index = 0;
	} else {
		// It's a 'talk'

		if (pTalkColour != NULL)
			SetTextPal(*pTalkColour);
		hFont = GetTalkFontHandle();
		index = 1;
	}

	if (texts[index].pText)
		MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), texts[index].pText);

	LoadSubString(stringId, 0, TextBufferAddr(), TBUFSZ);

	texts[index].dieFrame = currentFrame + duration;
	texts[index].pText = ObjectTextOut(coroParam, GetPlayfieldList(FIELD_STATUS),
						TextBufferAddr(),
						0,
						x, y,
						hFont,
						TXT_CENTRE, 0);
	KeepOnScreen(texts[index].pText, &x, &y);
}

/**
 * Called when a packet contains a command field.
 */
static int MovieCommand(char cmd, int commandOffset) {
	if (cmd & CD_PRINT) {
		PPRINT_CMD pCmd;

		pCmd = (PPRINT_CMD)(bigBuffer + commandOffset);

		MovieText(nullContext, (int16)READ_LE_UINT16(&pCmd->stringId),
				(int16)READ_LE_UINT16(&pCmd->x),
				(int16)READ_LE_UINT16(&pCmd->y),
				pCmd->fontId,
				NULL,
				pCmd->duration);

		return sz_CMD_PRINT_pkt;
	} else {
		if (bSubtitles) {
			PTALK_CMD pCmd;

			pCmd = (PTALK_CMD)(bigBuffer + commandOffset);
			talkColour = TINSEL_RGB(pCmd->r, pCmd->g, pCmd->b);

			MovieText(nullContext, (int16)READ_LE_UINT16(&pCmd->stringId),
					(int16)READ_LE_UINT16(&pCmd->x),
					(int16)READ_LE_UINT16(&pCmd->y),
					0,
					&talkColour,
					pCmd->duration);
		}
		return sz_CMD_TALK_pkt;
	}
}

/**
 * Called from PlayMovie() in tinlib.cpp
 * Kicks off the playback of a movie, and waits around
 * until it's finished.
 */
void PlayBMV(CORO_PARAM, SCNHANDLE hFileStem, int myEscape) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	assert(!bMovieOn);

	strcpy(szMovieFile, (char *)LockMem(hFileStem));
	strcat(szMovieFile, BMOVIE_EXTENSION);

	assert(strlen(szMovieFile) <= 12);

	bMovieOn = true;
	bAbort = false;
	bmvEscape = myEscape;

	do {
		CORO_SLEEP(1);
	} while (bMovieOn);

	CORO_END_CODE;
}

/**
 * Given a packet offset, calculates the offset of the
 * next packet. The packet may not yet exist, and the
 *return value may be off the end of bigBuffer.
 */
static int FollowingPacket(int thisPacket, bool bReallyImportant) {
	unsigned char *data;
	int	nextSlot, length;

	// Set pointer to thisPacket's data
	data = bigBuffer + thisPacket;

	switch (*data) {
	case CD_SLOT_NOP:
		nextSlot = thisPacket/SLOT_SIZE;
		if (thisPacket%SLOT_SIZE)
			nextSlot++;

		return nextSlot * SLOT_SIZE;

	case CD_LE_FIN:
		return -1;

	default:
		// Following 3 bytes are the length
		if (bReallyImportant) {
			// wrapped round or at least 3 bytes
			assert(((nextReadSlot * SLOT_SIZE) < thisPacket) ||
				((thisPacket + 3) < (nextReadSlot * SLOT_SIZE)));

			if ((nextReadSlot * SLOT_SIZE >= thisPacket) &&
				((thisPacket + 3) >= nextReadSlot*SLOT_SIZE)) {
				// MaintainBuffer calls this back, but with false
				MaintainBuffer();
			}
		} else {
			// not wrapped and not 3 bytes
			if (nextReadSlot*SLOT_SIZE >= thisPacket && thisPacket+3 >= nextReadSlot*SLOT_SIZE)
				return thisPacket + 3;
		}
		length = (int32)READ_LE_UINT32(bigBuffer + thisPacket + 1);
		length &= 0x00ffffff;
		return thisPacket + length + 4;
	}
}

/**
 * Called from the foreground when starting playback of a movie.
 */
static void LoadSlots(int number) {
	int nextOffset;

	assert(number + nextReadSlot < NUM_SLOTS);

	if (stream.read(bigBuffer + nextReadSlot*SLOT_SIZE, number * SLOT_SIZE) !=
			(uint32)(number * SLOT_SIZE)) {
		int possibleSlots;

		// May be a short file
		possibleSlots = stream.size() / SLOT_SIZE;
		if ((number + nextReadSlot) > possibleSlots) {
			bFileEnd = true;
			nextReadSlot = possibleSlots;
		} else
			error(FILE_IS_CORRUPT, szMovieFile);
	}

	nextReadSlot += number;

	nextOffset = FollowingPacket(nextUseOffset, true);
	while (nextOffset < nextReadSlot*SLOT_SIZE
			&& nextOffset != -1) {
		numAdvancePackets++;
		mostFutureOffset = nextOffset;
		nextOffset = FollowingPacket(mostFutureOffset, false);
	}
}

/**
 * Called from the foreground when starting playback of a movie.
 */
static void InitialiseBMV(void) {
	if (!stream.open(szMovieFile))
		error(CANNOT_FIND_FILE, szMovieFile);

	// Grab the data buffer
	bigBuffer = (byte *)malloc(NUM_SLOTS * SLOT_SIZE);
	if (bigBuffer == NULL)
		error(NO_MEM, "FMV data buffer");

	// Screen buffer (2 lines more than screen
	screenBuffer = (byte *)malloc(SCREEN_WIDTH * (SCREEN_HIGH + 2));
	if (screenBuffer == NULL)
		error(NO_MEM, "FMV screen buffer");

	// Pass the sceen buffer to the decompresser
	InitBMV(screenBuffer);

	// Initialise some stuff
	nextUseOffset = 0;
	nextSoundOffset = 0;
	wrapUseOffset = -1;
	mostFutureOffset = 0;
	currentFrame = 0;
	currentSoundFrame = 0;
	numAdvancePackets = 0;
	nextReadSlot = 0;
	bFileEnd = false;
	blobsInBuffer = 0;
	memset(texts, 0, sizeof(texts));
	talkColour = 0;
	bigProblemCount = 0;

	movieTick = 0;

	bIsText = false;

//	memset(&rr, 0, sizeof(rr));

	// Prefetch data
	LoadSlots(PREFETCH);

	while (numAdvancePackets < ADVANCE_SOUND)
		LoadSlots(1);

	// Initialise the sound channel
	InitialiseMovieSound();
}

/**
 * Called from the foreground when ending playback of a movie.
 */
void FinishBMV(void) {
	int	i;

	// Notify the sound channel
	FinishMovieSound();

	// Close the file stream
	if (stream.isOpen())
		stream.close();

	// Release the data buffer
	if (bigBuffer != NULL) {
		free(bigBuffer);
		bigBuffer = NULL;
	}

	// Release the screen buffer
	if (screenBuffer != NULL) {
		free(screenBuffer);
		screenBuffer = NULL;
	}

	// Ditch any text objects
	for (i = 0; i < 2; i++) {
		if (texts[i].pText) {
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), texts[i].pText);
			texts[i].pText = NULL;
		}
	}
	bMovieOn = false;

	nextMovieTime = 0;

	// Test for 'twixt-movie glitch
	ClearScreen();
}

/**
 * MaintainBuffer()
 */
static bool MaintainBuffer(void) {
	int nextOffset;

	// No action if the file is all read
	if (bFileEnd == true)
		return false;

	// See if next complete packet exists
	// and if so, if it will fit in the top of the buffer
	nextOffset = FollowingPacket(mostFutureOffset, false);
	if (nextOffset == -1) {
		// No following packets
		return false;
	} else if (nextOffset > NUM_SLOTS * SLOT_SIZE) {
		// The current unfinished packet will not fit
		// Copy this slot to slot 0

		// Not if we're still using it!!!
		// Or, indeed, if the player is still lagging
		if (nextUseOffset < SLOT_SIZE || nextUseOffset > mostFutureOffset) {
			// Slot 0 is still in use, buffer is full!
			return false;
		}

		// Tell data player where to make the jump
		wrapUseOffset = mostFutureOffset;

		// mostFuture Offset is now in slot 0
		mostFutureOffset %= SLOT_SIZE;

		// Copy the data we already have for unfinished packet
		memcpy(bigBuffer + mostFutureOffset,
			bigBuffer + wrapUseOffset,
			SLOT_SIZE - mostFutureOffset);

		// Next read is into slot 1
		nextReadSlot = 1;
	}

	if (nextReadSlot == NUM_SLOTS) {
		// Want to go to slot zero, wait if still in use
		if (nextUseOffset < SLOT_SIZE) {
			// Slot 0 is still in use, buffer is full!
			return false;
		}

		// nextOffset must be the buffer size
		assert(nextOffset == NUM_SLOTS*SLOT_SIZE);

		// wrapUseOffset must not be set
		assert(wrapUseOffset == -1);
		wrapUseOffset = nextOffset;

		nextReadSlot = 0;
		mostFutureOffset = 0;
	}

	// Don't overwrite unused data
	if (nextUseOffset / SLOT_SIZE == nextReadSlot) {
		// Buffer is full!
		return false;
	}

	if (stream.read(bigBuffer + nextReadSlot * SLOT_SIZE, SLOT_SIZE) != SLOT_SIZE) {
		bFileEnd = true;
	}

	// Read next slot next time
	nextReadSlot++;

	// Find new mostFutureOffset
	nextOffset = FollowingPacket(mostFutureOffset, false);
	while (nextOffset < nextReadSlot*SLOT_SIZE
			&& nextOffset != -1) {
		numAdvancePackets++;
		mostFutureOffset = nextOffset;
		nextOffset = FollowingPacket(mostFutureOffset, false);
	}

	// New test feature for e.g. short files
	if (bFileEnd && *(bigBuffer+mostFutureOffset) != CD_LE_FIN)
		bAbort = true;

	return true;
}

/**
 * DoBMVFrame
 */
static bool DoBMVFrame(void) {
	unsigned char *data;
	int	graphOffset, length;
	signed short	xscr;

	if (nextUseOffset == wrapUseOffset) {
		nextUseOffset %= SLOT_SIZE;
	}

	while (nextUseOffset == mostFutureOffset) {
		data = bigBuffer + nextUseOffset;
		if (*data != CD_LE_FIN) {
			// Don't get stuck in an infinite loop
			if (!MaintainBuffer()) {
				FinishBMV();
				return false;
			}

			if (nextUseOffset == wrapUseOffset) {
				nextUseOffset %= SLOT_SIZE;
			}
		} else
			break;
	}

	// Set pointer to data
	data = bigBuffer + nextUseOffset;

	// If still at most Future, it must be last
	if (nextUseOffset == mostFutureOffset) {
		assert(*data == CD_LE_FIN);
	}

	switch (*data) {
	case CD_SLOT_NOP:
		nextUseOffset = FollowingPacket(nextUseOffset, true);
		if (nextUseOffset == wrapUseOffset) {
			nextUseOffset %= SLOT_SIZE;
			wrapUseOffset = -1;
		}
		numAdvancePackets--;
		return false;

	case CD_LE_FIN:
		FinishBMV();
		numAdvancePackets--;
		return true;

	default:
		length = (int32)READ_LE_UINT32(data + 1);
		length &= 0x00ffffff;

		graphOffset = nextUseOffset + 4;	// Skip command byte and length

		if (*data & CD_AUDIO) {
			if (bOldAudio) {
				graphOffset += sz_AUDIO_pkt;	// Skip audio data
				length -= sz_AUDIO_pkt;
			} else {
				int blobs;

				blobs = *(bigBuffer + graphOffset);
				blobs *= SZ_C_BLOB;
				graphOffset += (blobs + 1);
				length -= (blobs + 1);
			}
		}

		if (*data & CD_CMND) {
			int cmdLen;

			// Process command and skip data
			cmdLen = MovieCommand(*data, graphOffset);

			graphOffset += cmdLen;
			length -= cmdLen;
		}

		if (*data & CD_CMAP) {
			MoviePalette(graphOffset);
			graphOffset += sz_CMAP_pkt;	// Skip palette data
			length -= sz_CMAP_pkt;
		}

		if (*data & CD_XSCR) {
			xscr = (int16)READ_LE_UINT16(bigBuffer + graphOffset);
			graphOffset += sz_XSCR_pkt;	// Skip scroll offset
			length -= sz_XSCR_pkt;
		} else if (*data & BIT0)
			xscr = -640;
		else
			xscr = 0;

		PrepBMV(bigBuffer + graphOffset, length, xscr);

		currentFrame++;
		numAdvancePackets--;

		nextUseOffset = FollowingPacket(nextUseOffset, true);
		if (nextUseOffset == wrapUseOffset) {
			nextUseOffset %= SLOT_SIZE;
			wrapUseOffset = -1;
		}
		return true;
	}
}

/**
 * DoSoundFrame
 */
static bool DoSoundFrame(void) {
	unsigned char *data;
	int	graphOffset;

	if (nextSoundOffset == wrapUseOffset) {
		nextSoundOffset %= SLOT_SIZE;
	}

	// Make sure the full slot is here
	while (nextSoundOffset == mostFutureOffset) {
		data = bigBuffer + nextSoundOffset;
		if (*data != CD_LE_FIN) {
			// Don't get stuck in an infinite loop
			if (!MaintainBuffer()) {
				if (!bOldAudio)
					MovieAudio(0, 0);
				currentSoundFrame++;
				return false;
			}

			if (nextSoundOffset == wrapUseOffset) {
				nextSoundOffset %= SLOT_SIZE;
			}
		} else
			break;
	}

	// Set pointer to data
	data = bigBuffer + nextSoundOffset;

	// If still at most Future, it must be last
	if (nextSoundOffset == mostFutureOffset) {
		assert(*data == CD_LE_FIN);
	}

	switch (*data) {
	case CD_SLOT_NOP:
		nextSoundOffset = FollowingPacket(nextSoundOffset, true);
		if (nextSoundOffset == wrapUseOffset) {
			nextSoundOffset %= SLOT_SIZE;
		}
		return false;

	case CD_LE_FIN:
		if (!bOldAudio)
			MovieAudio(0, 0);
		currentSoundFrame++;
		return true;

	default:
		if (*data & CD_AUDIO) {
			graphOffset = nextSoundOffset + 4;	// Skip command byte and length

			if (!bOldAudio) {
				int blobs = *(bigBuffer + graphOffset);
				MovieAudio(graphOffset+1, blobs);
			}
		} else {
			if (!bOldAudio)
				MovieAudio(0, 0);
		}

		nextSoundOffset = FollowingPacket(nextSoundOffset, false);
		if (nextSoundOffset == wrapUseOffset) {
			nextSoundOffset %= SLOT_SIZE;
		}
		currentSoundFrame++;
		return true;
	}
}

/**
 * CopyMovieToScreen
 */
void CopyMovieToScreen(void) {
	// Not if not up and running yet!
	if (!screenBuffer || (currentFrame == 0)) {
		ForceEntireRedraw();
		DrawBackgnd();
		return;
	}

	// The movie surface is slightly less high than the output screen (429 rows versus 432).
	// Because of this, there's some extra line clearing above and below the displayed area
	int yStart = (SCREEN_HEIGHT - SCREEN_HIGH) / 2;
	memset(_vm->screen().getBasePtr(0, 0), 0, yStart * SCREEN_WIDTH);
	memcpy(_vm->screen().getBasePtr(0, yStart), ScreenBeg, SCREEN_WIDTH * SCREEN_HIGH);
	memset(_vm->screen().getBasePtr(0, yStart + SCREEN_HIGH), 0,
		(SCREEN_HEIGHT - SCREEN_HIGH - yStart) * SCREEN_WIDTH);

	BmvDrawText(true);
	PalettesToVideoDAC();			// Keep palette up-to-date
	UpdateScreenRect(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	g_system->updateScreen();
	BmvDrawText(false);
}

/**
 * LookAtBuffers
 */
static void LookAtBuffers(void) {
	static int junk;
	int i;

	if (bigBuffer) {
		for (i = 0; i < NUM_SLOTS; i++)
			junk += bigBuffer[i*SLOT_SIZE];
	}
}

/**
 * Handles playback of any active movie. Called from the foreground 24 times a second.
 */
void FettleBMV(void) {
	static int nextMaintain = 0;

	int refFrame;
	// Tick counter needs to be incrementing at a 24Hz rate
	int tick = movieTick++;

	if (!bMovieOn)
		return;

	// Escape the rest if appropriate
	if (bAbort || (bmvEscape && bmvEscape != GetEscEvents())) {
		FinishBMV();
		return;
	}

	LookAtBuffers();

	if (!stream.isOpen()) {
		int i;

		// First time in with this movie

		InitialiseBMV();

		for (i = 0; i < ADVANCE_SOUND;) {
			if (DoSoundFrame())
				i++;
		}
		startTick = -ONE_SECOND / 4;	// 1/4 second
		return;
	}

	if (startTick < 0) {
		startTick++;
		return;
	}
	if (startTick == 0) {
		startTick = tick;
		nextMaintain = startTick + 1;
		StartMovieSound();
	}

	nextMovieTime = g_system->getMillis() + 41;

	FettleMovieText();

	if (bigProblemCount < PT_A) {
		refFrame = currentSoundFrame;

		while (currentSoundFrame < ((tick+1-startTick)/2 + ADVANCE_SOUND) && bMovieOn) {
			if (currentSoundFrame == refFrame+PT_B)
				break;

			DoSoundFrame();
		}
	}

	// Time to process a frame (or maybe more)
	if (bigProblemCount < PT_A) {
		refFrame = currentFrame;

		while ((currentFrame < (tick-startTick)/2) && bMovieOn) {
			DoBMVFrame();

			if (currentFrame == refFrame+PT_B) {
				bigProblemCount++;

				if (bigProblemCount == PT_A) {
					startTick = tick-(2*currentFrame);
					bigProblemCount = 0;
				}
				break;
			}
		}
		if (currentFrame == refFrame || currentFrame <= refFrame+3) {
			bigProblemCount = 0;
		}
	} else {
		while (currentFrame < (tick-startTick)/2 && bMovieOn) {
			DoBMVFrame();
		}
	}

	if (tick >= nextMaintain || numAdvancePackets < SUBSEQUENT_SOUND) {
		MaintainBuffer();
		nextMaintain = tick + 2;
	}
}

/**
 * Returns true if a movie is playing.
 */
bool MoviePlaying(void) {
	return bMovieOn;
}

/**
 * Returns the audio lag in ms
 */
int32 MovieAudioLag(void) {
	if (!bMovieOn || !audioStream)
		return 0;

	// Calculate lag
	int32 playLength = (movieTick - startTick - 1) * ((((uint32) 1000) << 10) / 24);
	return (playLength - (((int32) _vm->_mixer->getSoundElapsedTime(audioHandle)) << 10)) >> 10;
}

uint32 NextMovieTime(void) {
	return nextMovieTime;
}

void AbortMovie(void) {
	bAbort = true;
}

} // End of namespace Tinsel
