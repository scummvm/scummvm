/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * The movie player.
 */

#include "tinsel/tinsel.h"
#include "tinsel/background.h"
#include "tinsel/bmv.h"
#include "tinsel/cliprect.h"
#include "tinsel/config.h"
#include "tinsel/dw.h"
#include "tinsel/events.h"
#include "tinsel/font.h"
#include "tinsel/graphics.h"
#include "tinsel/handle.h"
#include "tinsel/multiobj.h"
#include "tinsel/sched.h"
#include "tinsel/strres.h"
#include "tinsel/text.h"
#include "tinsel/timers.h"
#include "tinsel/tinlib.h"
#include "tinsel/tinsel.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#include "common/textconsole.h"

namespace Tinsel {

//----------------- LOCAL DEFINES ----------------------------

#define BMOVIE_EXTENSION	".bmv"

#define SZ_C_BLOB	65

#define PT_A	20	// Number of times PT_B may be reached
#define PT_B	6

#define ADVANCE_SOUND		18	// 1 1/2 seconds
#define SUBSEQUENT_SOUND	6	// 1/2 second

// PACKET TYPE IDs & FLAGS

#define CD_SLOT_NOP	0x00	// Skip to next slot
#define CD_LE_FIN	0x01	// End of movie

#define BIT0		0x01

#define CD_XSCR		0x04	// Screen has a scroll offset
#define CD_CMAP	0x08	// Color map is included
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


struct TALK_CMD {
	short	x;
	short	y;
	short	stringId;
	unsigned char	duration;
	char	r;			// may be b!
	char	g;
	char	b;			// may be r!
};

struct PRINT_CMD {
	int16	x;
	int16	y;
	int16	stringId;
	unsigned char	duration;
	unsigned char	fontId;
};


//----------------- LOCAL GLOBAL DATA ------------------------

static const uint16 Au_DecTable[16] = {16512, 8256, 4128, 2064, 1032, 516, 258, 192,
		129, 88, 64, 56, 48, 40, 36, 32};

//---------------- DECOMPRESSOR FUNCTIONS --------------------

#define SCREEN_WIDE 640
#define SCREEN_HIGH ((TinselVersion == 3) ? 432 : 429)
#define SAM_P_BLOB (32 * 2)

#define ROR(x,v) x = ((x >> (v%32)) | (x << (32 - (v%32))))
#define ROL(x,v) x = ((x << (v%32)) | (x >> (32 - (v%32))))
#define NEXT_BYTE(v) v = (forwardDirection ? v + 1 : v - 1)

void BMVPlayer::PrepBMV(const byte *sourceData, int length, short deltaFetchDisp) {
	uint8 NibbleHi = 0;
	uint32 edx = length;
	int32 ebx = deltaFetchDisp;
	const byte *src;
	byte *dst, *endDst;

	const bool forwardDirection = (deltaFetchDisp <= -SCREEN_WIDE) || (deltaFetchDisp >= 0);
	if (forwardDirection) {
		// Forward decompression
		src = sourceData;
		dst = ScreenBeg;
		endDst = ScreenBeg + SCREEN_WIDE * SCREEN_HIGH;
	} else {
		src = sourceData + length - 1;
		dst = ScreenBeg + SCREEN_WIDE * SCREEN_HIGH - 1;
		endDst = ScreenBeg - 1;
	}

	bool firstLoop, flag;

	int loopCtr = 0;
	do {
		uint32 eax = 0;
		uint32 bitshift = 0;
		flag = false;

		if ((loopCtr == 0) || (edx == 4)) {
			// Get the next hi,lo nibble
			eax = (eax & 0xffffff00) | *src;
			firstLoop = true;
		} else {
			// Get the high nibble
			eax = (eax & 0xffffff00) | NibbleHi;
			firstLoop = false;
		}

		// Is lo nibble '00xx'?
		if ((eax & 0xC) == 0) {
			for (;;) {
//@_rDN_Lp_1:
				// Only execute this bit first the first time into the loop
				if (!firstLoop) {
					ROR(eax, 2);
					bitshift += 2;
					eax = (eax & 0xffffff00) | *src;

					if ((eax & 0xC) != 0)
						break;
				}
				firstLoop = false;

//@_rD2nd_1:
				ROR(eax, 2);		// Save bi-bit into hi 2 bits
				bitshift += 2;			//   and increase bit-shifter
				// Shift another 2 bits to get hi nibble
				eax = (eax & 0xffffff00) | ((eax & 0xff) >> 2);
				NEXT_BYTE(src);

				if ((eax & 0xC) != 0) {
					flag = true;
					ROL(eax, bitshift);
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
			NibbleHi = ((uint8)eax) >> 4;
			edx = 0;			// offset rDNum_Hi ; Next nibble is a 'hi' (reserved)
			eax &= 0xffffff0f;
			NEXT_BYTE(src);
			ROL(eax, bitshift);
		}
//rDN_1:
//@_rD_or_R:
		bool actionFlag = (eax & 1) != 0;
		eax >>= 1;
		int byteLen = eax - 1;

		// Move to next loop index
		++loopCtr;
		if (loopCtr == 4)
			loopCtr = 1;

		if (actionFlag) {
			// Adjust loopCtr to fall into the correct processing case
			loopCtr = loopCtr % 3 + 1;
		}

		switch (loopCtr) {
		case 1: {
			// @_rDelta:
			const byte *saved_src = src;			// Save the source pointer
			src = dst + ebx;			// Point it to existing data

			while (byteLen > 0) {
				*dst = *src;
				NEXT_BYTE(src);
				NEXT_BYTE(dst);
				--byteLen;
			}

			src = saved_src;
			break;
			}

		case 2:
			// @_rRaw
			// Copy data from source to dest
			while (byteLen > 0) {
				*dst = *src;
				NEXT_BYTE(src);
				NEXT_BYTE(dst);
				--byteLen;
			}
			break;

		case 3:
			// @_rRun
			// Repeating run of data
			eax = forwardDirection ? *(dst - 1) : *(dst + 1);

			while (byteLen > 0) {
				*dst = (uint8)eax;
				NEXT_BYTE(dst);
				--byteLen;
			}
			break;
		default:
			break;
		}
	} while (dst != endDst);
}


static uint32 DecodeVLE(const byte **src, uint32* hasHi, uint32 *nibbleHi) {
	uint32 length = 0;
	byte bitshift = 0;

	*hasHi = 0;
	*nibbleHi = 0;

	// number is stored as series of 2 bits in a nibble (4b), and the coding ends when any of two upper bits are set to 1
	while (true) {
		uint32 byte = **src;
		++(*src);

		length |= (byte & 0x0F) << bitshift;
		bitshift += 2;

		if (byte & 0x0C) { // end if any of 0b1100 bits are set
			*nibbleHi = byte >> 4;
			*hasHi = 1;
			break;
		}

		byte >>= 4;

		length |= (byte & 0x0F) << bitshift;
		bitshift += 2;

		if (byte & 0x0C) { // end if any of 0b1100 bits are set
			break;
		}
	}
	return length;
}

void BMVPlayer::t3DoOperation(BMV_OP op, uint32 len, const byte **src, byte **dst, int32 deltaOffset) {
	// note that there must be no padding in DST stride.
	// data in dst & moviePal has already in correct platform endianess
	switch(op) {
		case BMV_OP_DELTA: {
			for (uint32 i = 0; i < len; ++i) {
				*(uint16*)(*dst) = *(uint16*)((*dst) + deltaOffset);
				*dst += 2;
			}
			break;
		}
		case BMV_OP_RAW: {
			for (uint32 i = 0; i < len; ++i) {
				byte byte = **src;
				*src += 1;

				byte += 1;                                 // everything is shifted by one for some reason
				byte = ((byte & 0x03) << 6) | (byte >> 2); // bits are swizzled 12345678 -> 34567812; ROL(x, 2)

				uint16 color = 0;
				if (byte < 7) {
					// first 256 entries are organized differently, see the bit swizzling above
					color = moviePal[(byte + 1) * 256 + **src];
					*src += 1;
				} else if (byte == 7) {
					// note that there is no palette conversion, a bug?, also this might not be aligned properly in memory on some architectures
					color = READ_LE_UINT16(*src);
					*src += 2;
				} else {
					color = moviePal[byte];
				}

				*(uint16*)(*dst) = color;
				*dst += 2;
			}
			break;
		}
		case BMV_OP_RUN: {
			uint16 val = *(uint16*)((*dst) - 2);
			for (uint32 i = 0; i < len; ++i) {
				*(uint16*)(*dst) = val;
				*dst += 2;
			}
			break;
		}
		case BMV_OP_COUNT:
			break;
	}
}

void BMVPlayer::t3PrepBMV(const byte *src, uint32 len, int32 deltaOffset) {
	if (len == 0) {
		return;
	}

	/*
	BMV v3 video packet contains:
	- update of palette
	- encoded video frame

	Palette can be updated in parts, encoding contains offset and count if entries to copy from the stream to the palette.

	Encoded frame can contain 3 operations:
	- delta - copy previously written N pixels, either from previous line, previous frame, or an offset within previous frame
	- raw - copy N values from source stream
	- run - repeat last pixel value N times

	Encoding is convoluted and relies on nibbles - lo nibble = 4 bottom bits of a byte, hi nibble = 4 upper bits of a byte.
	Operations keep rotating, after delta operation there can follow RAW or RUN operation, after RAW only RUN or DELTA and after RUN only DELTA or RAW can follow.
	Which operation will follow depends on lowest bit of a current nibble.

	Length of each operation can be encoded either directly or via special variable length encoding. From a nibble two bottom bits are preprended to final value, then next nibble is processed, if a new nibble is needed next byte is read from the stream. Process ends if any of two upper bits are set, in that case two upper bits are prepended as well.

	Which length is going to be used depends on a quite convoluted encoding and there are two ways how they are encoded (note that operations still keep switching).
	In outer loop, if:
	- hi nibble has any of 2 upper bits set then only byte is read in this iteration and if:
		- lo nibble has any of 2 upper bits set then length is between 7-30
		- lo nibble has none of 2 upper bits set then there are 2 operations encoded and each nibble represents value 1-6
	- hi nibble has none of 2 upper bits set then multiple bytes can be read in this iterations and if:
		- lo nibble has any of 2 upper bits set then length is 1-6
		- lo nibble has none of 2 upper bits set then length is using variable length encoding.
			- if there hi nibble left then there is another operation with length 1-6
		- start of other encoding - inner loop
	In inner loop, if:
	- lo nibble has any of 2 upper bits set then length is between 7-30
	- lo nibble has none of 2 upper bits set then length is using variable length encoding
	- hi nibble has any of 2 upper bits set then length is between 1-6 and inner loop ends
	- hi nibble has none of 2 upper bits set then the rest used to select next operation and partly encode next length of next operation

	Frame image data is always written from left to right, from top to bottom.
	*/

	const byte* srcEnd = src + len;
	byte* dst = ScreenBeg;

	// update the palette
	uint32 i = 8; // first 8 entries are not used and are special instructions, 0-6 select entries above 256, 7 direct value
	while (true) {
		uint32 count = 0;
		uint32 skip = 0;

		byte byte = *src;
		++src;

		if (byte < 0x90) {
			uint16 word = READ_LE_UINT16(src); // this might not be aligned properly in memory on some architectures
			src += 2;

			count =                 word & 0x0FFF;         // 0 - 4095 (0xFFF)
			skip  = (byte << 4) | ((word & 0xF000) >> 12); // 0 - 2207 (0x89F)
		} else {
			count = (byte & 0x7) + 1;   // 1 - 8
			skip  = (byte >> 3) - 0x11; // 1 - 14
		}

		for (uint32 j = 0; j < count; ++j) {
			uint16 color = READ_LE_UINT16(src); // this might not be aligned properly in memory on some architectures
			src += 2;

			moviePal[i] = color;
			++i;
		}

		i += skip;
		if (skip == 0) {
			break;
		}
	}

	// decode the frame
#define BMV_NEXT_OP(op, nibble) { op = (BMV_OP)((op + 1 + (nibble & 1)) % BMV_OP_COUNT); }
	BMV_OP op = BMV_OP_RUN; // first operation can be DELTA or RAW

	while (src < srcEnd) { // outer loop
		byte byte = *src;
		++src;

		uint32 loNibble = byte & 0x0F;
		uint32 hiNibble = byte >> 4;

		uint32 length = 0;

		if ((hiNibble & 0x0C) == 0) { // length might use more than one byte and there might be an inner loop
			if ((loNibble & 0x0C) == 0) { // variable length encoding
				uint32 hasHi = 0;

				BMV_NEXT_OP(op, loNibble);
				length  = (loNibble >> 1); // note that upper two bit are always 0
				length += (hiNibble << 1); // note that upper two bit are always 0
				length += DecodeVLE(&src, &hasHi, &hiNibble) << 3; // hiNibble is overriden
				length -= 1;
				t3DoOperation(op, length, &src, &dst, deltaOffset);

				if (!hasHi) {
					continue; // outer loop
				} else if ((hiNibble & 0x0C) != 0) { // process remaining nibble
					BMV_NEXT_OP(op, hiNibble);
					length = (hiNibble >> 1) - 1;
					t3DoOperation(op, length, &src, &dst, deltaOffset);
					continue; // outer loop
				}
			} else { // length is encoded directly (1-6)
				BMV_NEXT_OP(op, loNibble);
				length = (loNibble >> 1) - 1;
				t3DoOperation(op, length, &src, &dst, deltaOffset);
			}

			while (src < srcEnd) { // inner loop
				// there is always hiNibble, either from previous operation or previous iteration
				BMV_NEXT_OP(op, hiNibble);
				length = (hiNibble >> 1); // note that upper two bit are always 0

				byte = *src;
				++src;

				loNibble = byte & 0x0F;
				hiNibble = byte >> 4;

				if ((loNibble & 0x0C) == 0) {
					uint32 hasHi = 0;
					length += (loNibble << 1); // process lo nibble
					length += (hiNibble << 3); // process hi nibble
					if ((hiNibble & 0x0C) == 0) { // continue if there is more
						length += DecodeVLE(&src, &hasHi, &hiNibble) << 5; // hiNibble is overriden
					}
					length -= 1;
					t3DoOperation(op, length, &src, &dst, deltaOffset);

					if (!hasHi) {
						break; // break inner loop and continue with outer loop
					}
				} else {
					length += (loNibble << 1) - 1;
					t3DoOperation(op, length, &src, &dst, deltaOffset);
				}

				if ((hiNibble & 0x0C) != 0) {
					BMV_NEXT_OP(op, hiNibble);
					length = (hiNibble >> 1) - 1;
					t3DoOperation(op, length, &src, &dst, deltaOffset);
					break; // finish inner loop
				}
			}
		} else if ((loNibble & 0x0C) == 0) { // one operation with length 7-30
			BMV_NEXT_OP(op, loNibble);
			length  = (loNibble >> 1); // note that upper two bit are always 0
			length += (hiNibble << 1) - 1;
			t3DoOperation(op, length, &src, &dst, deltaOffset);
		} else { // each nibble contain one operation and length is 1-6
			BMV_NEXT_OP(op, loNibble);
			length = (loNibble >> 1) - 1;
			t3DoOperation(op, length, &src, &dst, deltaOffset);

			BMV_NEXT_OP(op, hiNibble);
			length = (hiNibble >> 1) - 1;
			t3DoOperation(op, length, &src, &dst, deltaOffset);
		}
	}
#undef BMV_NEXT_OP
}

void BMVPlayer::ReadHeader() {
	stream.readUint32LE(); // magic
	stream.readUint32LE(); // payload
	slotSize = stream.readUint32LE();
	frames = stream.readUint32LE();
	prefetchSlots = stream.readUint16LE();
	numSlots = stream.readUint16LE();
	frameRate = stream.readUint16LE();
	audioMaxSize = stream.readUint16LE();
	audioBlobSize = stream.readUint16LE();
	stream.readByte(); // audioId
	stream.readByte(); // videoId
	width = stream.readUint16LE();
	height = stream.readUint16LE();
	bpp = 2;

	assert((frameRate & 0xff) == 0);
	frameTime = frameRate >> 8;
	assert((24 % frameTime) == 0);
	frameTime = 24 / frameTime;

	// skip 8 bytes
	stream.readUint32LE();
	stream.readUint32LE();
}

void BMVPlayer::InitBMV(byte *memoryBuffer) {
	if (TinselVersion == 3) {
		// Clear the whole buffer
		memset(memoryBuffer, 0, SCREEN_WIDE * (SCREEN_HIGH + 2) * bpp);
		// Reset the palette as it might be partially updated
		memset(moviePal, 0, sizeof(moviePal));
	} else {
		// Clear the two extra 'off-screen' rows
		memset(memoryBuffer, 0, SCREEN_WIDE * bpp);
		memset(memoryBuffer + SCREEN_WIDE * (SCREEN_HIGH + 1) * bpp, 0, SCREEN_WIDE * bpp);
	}

	if (_audioStream) {
		_vm->_mixer->stopHandle(_audioHandle);

		delete _audioStream;
		_audioStream = 0;
	}

	// Set the screen beginning to the second line (ie. past the off-screen line)
	ScreenBeg = memoryBuffer + SCREEN_WIDTH * bpp;
	Au_Prev1 = Au_Prev2 = 0;
}

void BMVPlayer::PrepAudio(const byte *sourceData, int blobCount, byte *destPtr) {
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

BMVPlayer::BMVPlayer() {
	bOldAudio = 0;
	bMovieOn = 0;
	bAbort = 0;
	bmvEscape = 0;

	memset(szMovieFile, 0, sizeof(szMovieFile));

	bigBuffer = 0;
	nextUseOffset = 0;
	nextSoundOffset = 0;
	wrapUseOffset = 0;
	mostFutureOffset = 0;
	currentFrame = 0;
	currentSoundFrame = 0;
	numAdvancePackets = 0;
	nextReadSlot = 0;
	bFileEnd = 0;

	memset(moviePal, 0, sizeof(moviePal));

	blobsInBuffer = 0;

	memset(texts, 0, sizeof(texts));

	talkColor = 0;
	bigProblemCount = 0;
	bIsText = 0;
	movieTick = 0;
	startTick = 0;
	nextMovieTime = 0;
	Au_Prev1 = 0;
	Au_Prev2 = 0;
	ScreenBeg = 0;
	screenBuffer = 0;
	audioStarted = 0;
	_audioStream = 0;
	nextMaintain = 0;
}

/**
 * Called when a packet contains a palette field.
 * Build a COLORREF array and queue it to the DAC.
 */
void BMVPlayer::MoviePalette(int paletteOffset) {
	int	i;
	byte *r;

	r = bigBuffer + paletteOffset;

	for (i = 0; i < 256; i++, r += 3)	{
		moviePal[i] = TINSEL_RGB(*r, *(r + 1), *(r + 2));
	}

	UpdateDACqueue(1, 255, &moviePal[1]);

	// Don't clobber talk
	if (talkColor != 0)
		SetTextPal(talkColor);
}

void BMVPlayer::InitializeMovieSound() {
	_audioStream = Audio::makeQueuingAudioStream(22050, true);
	audioStarted = false;
}

void BMVPlayer::StartMovieSound() {
}

void BMVPlayer::FinishMovieSound() {
	if (_audioStream) {
		_vm->_mixer->stopHandle(_audioHandle);

		delete _audioStream;
		_audioStream = 0;
	}
}

/**
 * Called when a packet contains an audio field.
 */
void BMVPlayer::MovieAudio(int audioOffset, int blobs) {
	if (audioOffset == 0 && blobs == 0)
		blobs = 57;

	byte *data = (byte *)malloc(blobs * 128);

	if (audioOffset != 0)
		PrepAudio(bigBuffer+audioOffset, blobs, data);
	else
		memset(data, 0, blobs * 128);

	_audioStream->queueBuffer(data, blobs * 128, DisposeAfterUse::YES, Audio::FLAG_16BITS | Audio::FLAG_STEREO);

	if (currentSoundFrame == ADVANCE_SOUND) {
		if (!audioStarted) {
			_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType,
					&_audioHandle, _audioStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
			audioStarted = true;
		}
	}
}

/*-----------------------------------------------------*\
|-------------------------------------------------------|
\*-----------------------------------------------------*/

void BMVPlayer::FettleMovieText() {
	int i;

	bIsText = false;

	for (i = 0; i < 2; i++) {
		if (texts[i].pText) {
			if (currentFrame > texts[i].dieFrame) {
				MultiDeleteObjectIfExists(FIELD_STATUS, &texts[i].pText);
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

void BMVPlayer::BmvDrawText(bool bDraw) {
	int	w, h, x, y;

	for (int i = 0; i < 2; i++) {
		if (texts[i].pText) {
			Common::Rect bounds = MultiBounds(texts[i].pText);
			x = bounds.left;
			y = bounds.top;
			w = MIN(bounds.right + 1, (int)SCREEN_WIDTH) - x;
			h = MIN(bounds.bottom + 1, SCREEN_HIGH) - y;

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
				UpdateClipRect(_vm->_bg->GetPlayfieldList(FIELD_STATUS), &ptWin,	&rcPlayClip);
			}
		}
	}
}

/*-----------------------------------------------------*\
|-------------------------------------------------------|
\*-----------------------------------------------------*/

void BMVPlayer::MovieText(CORO_PARAM, int stringId, int x, int y, int fontId, COLORREF *pTalkColor, int duration) {
	SCNHANDLE hFont;
	int	index;

	if (fontId == 1) {
		// It's a 'print'

		hFont = _vm->_font->GetTagFontHandle();
		index = 0;
	} else {
		// It's a 'talk'

		if (pTalkColor != NULL)
			SetTextPal(*pTalkColor);
		hFont = _vm->_font->GetTalkFontHandle();
		index = 1;
	}

	MultiDeleteObjectIfExists(FIELD_STATUS, &texts[index].pText);

	LoadSubString(stringId, 0, _vm->_font->TextBufferAddr(), TBUFSZ);

	texts[index].dieFrame = currentFrame + duration;
	texts[index].pText = ObjectTextOut(_vm->_bg->GetPlayfieldList(FIELD_STATUS),
						_vm->_font->TextBufferAddr(),
						0,
						x, y,
						hFont,
						TXT_CENTER, 0);
	KeepOnScreen(texts[index].pText, &x, &y);
}

/**
 * Called when a packet contains a command field.
 */
int BMVPlayer::MovieCommand(char cmd, int commandOffset) {
	if (cmd & CD_PRINT) {
		PRINT_CMD *pCmd = (PRINT_CMD *)(bigBuffer + commandOffset);

		MovieText(Common::nullContext, (int16)READ_16(&pCmd->stringId),
				(int16)READ_16(&pCmd->x),
				(int16)READ_16(&pCmd->y),
				pCmd->fontId,
				NULL,
				pCmd->duration);

		return sz_CMD_PRINT_pkt;
	} else {
		if (_vm->_config->_useSubtitles) {
			TALK_CMD *pCmd = (TALK_CMD *)(bigBuffer + commandOffset);
			talkColor = TINSEL_RGB(pCmd->r, pCmd->g, pCmd->b);

			MovieText(Common::nullContext, (int16)READ_16(&pCmd->stringId),
					(int16)READ_16(&pCmd->x),
					(int16)READ_16(&pCmd->y),
					0,
					&talkColor,
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
void BMVPlayer::PlayBMV(CORO_PARAM, SCNHANDLE hFileStem, int myEscape) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	assert(!bMovieOn);

	Common::strlcpy(szMovieFile, (char *)_vm->_handle->LockMem(hFileStem), 14);
	Common::strlcat(szMovieFile, BMOVIE_EXTENSION, 14);

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
int BMVPlayer::FollowingPacket(int thisPacket, bool bReallyImportant) {
	unsigned char *data;
	int	nextSlot, length;

	// Set pointer to thisPacket's data
	data = bigBuffer + thisPacket;

	switch (*data) {
	case CD_SLOT_NOP:
		nextSlot = thisPacket/slotSize;
		if (thisPacket%slotSize)
			nextSlot++;

		return nextSlot * slotSize;

	case CD_LE_FIN:
		return -1;

	default:
		// Following 3 bytes are the length
		if (bReallyImportant) {
			// wrapped round or at least 3 bytes
			assert(((nextReadSlot * slotSize) < thisPacket) ||
				((thisPacket + 3) < (nextReadSlot * slotSize)));

			if ((nextReadSlot * slotSize >= thisPacket) &&
				((thisPacket + 3) >= nextReadSlot*slotSize)) {
				// MaintainBuffer calls this back, but with false
				MaintainBuffer();
			}
		} else {
			// not wrapped and not 3 bytes
			if (nextReadSlot*slotSize >= thisPacket && thisPacket+3 >= nextReadSlot*slotSize)
				return thisPacket + 3;
		}
		length = (int32)READ_32(bigBuffer + thisPacket + 1);
		length &= 0x00ffffff;
		return thisPacket + length + 4;
	}
}

/**
 * Called from the foreground when starting playback of a movie.
 */
void BMVPlayer::LoadSlots(int number) {
	int nextOffset;

	assert(number + nextReadSlot < numSlots);

	if (stream.read(bigBuffer + nextReadSlot*slotSize, number * slotSize) !=
			(uint32)(number * slotSize)) {
		int possibleSlots;

		// May be a short file
		possibleSlots = stream.size() / slotSize;
		if ((number + nextReadSlot) > possibleSlots) {
			bFileEnd = true;
			nextReadSlot = possibleSlots;
		} else
			error(FILE_IS_CORRUPT, szMovieFile);
	}

	nextReadSlot += number;

	nextOffset = FollowingPacket(nextUseOffset, true);
	while (nextOffset < nextReadSlot*slotSize
			&& nextOffset != -1) {
		numAdvancePackets++;
		mostFutureOffset = nextOffset;
		nextOffset = FollowingPacket(mostFutureOffset, false);
	}
}

/**
 * Called from the foreground when starting playback of a movie.
 */
void BMVPlayer::InitializeBMV() {
	if (!stream.open(szMovieFile))
		error(CANNOT_FIND_FILE, szMovieFile);

	if (TinselVersion == 3) {
		ReadHeader();
	} else {
		bpp = 1;
		frameTime = 2;
		slotSize = (25*1024);
		//numSlots = 168;
		numSlots = 122;	// -> ~ 3MB
		prefetchSlots = (numSlots/2); // For initial test
	}


	// Grab the data buffer
	bigBuffer = (byte *)malloc(numSlots * slotSize);
	if (bigBuffer == NULL)
		error(NO_MEM, "FMV data buffer");

	// Screen buffer (2 lines more than screen
	screenBuffer = (byte *)malloc(SCREEN_WIDTH * (SCREEN_HIGH + 2) * bpp);
	if (screenBuffer == NULL)
		error(NO_MEM, "FMV screen buffer");

	// Pass the sceen buffer to the decompresser
	InitBMV(screenBuffer);

	// Initialize some stuff
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
	talkColor = 0;
	bigProblemCount = 0;

	movieTick = 0;

	bIsText = false;

	// Prefetch data
	LoadSlots(prefetchSlots);

	if (TinselVersion != 3) {
		while (numAdvancePackets < ADVANCE_SOUND) {
			LoadSlots(1);
		}
	}

	// Initialize the sound channel
	InitializeMovieSound();
}

/**
 * Called from the foreground when ending playback of a movie.
 */
void BMVPlayer::FinishBMV() {
	int	i;

	// Notify the sound channel
	FinishMovieSound();

	// Close the file stream
	if (stream.isOpen())
		stream.close();

	// Release the data buffer
	free(bigBuffer);
	bigBuffer = nullptr;

	// Release the screen buffer
	free(screenBuffer);
	screenBuffer = nullptr;

	// Ditch any text objects
	for (i = 0; i < 2; i++) {
		MultiDeleteObjectIfExists(FIELD_STATUS, &texts[i].pText);
	}
	bMovieOn = false;

	nextMovieTime = 0;

	// Test for 'twixt-movie glitch
	ClearScreen();
}

/**
 * MaintainBuffer()
 */
bool BMVPlayer::MaintainBuffer() {
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
	} else if (nextOffset > numSlots * slotSize) {
		// The current unfinished packet will not fit
		// Copy this slot to slot 0

		// Not if we're still using it!!!
		// Or, indeed, if the player is still lagging
		if (nextUseOffset < slotSize || nextUseOffset > mostFutureOffset) {
			// Slot 0 is still in use, buffer is full!
			return false;
		}

		// Tell data player where to make the jump
		wrapUseOffset = mostFutureOffset;

		// mostFuture Offset is now in slot 0
		mostFutureOffset %= slotSize;

		// Copy the data we already have for unfinished packet
		memcpy(bigBuffer + mostFutureOffset,
			bigBuffer + wrapUseOffset,
			slotSize - mostFutureOffset);

		// Next read is into slot 1
		nextReadSlot = 1;
	}

	if (nextReadSlot == numSlots) {
		// Want to go to slot zero, wait if still in use
		if (nextUseOffset < slotSize) {
			// Slot 0 is still in use, buffer is full!
			return false;
		}

		// nextOffset must be the buffer size
		assert(nextOffset == numSlots*slotSize);

		// wrapUseOffset must not be set
		assert(wrapUseOffset == -1);
		wrapUseOffset = nextOffset;

		nextReadSlot = 0;
		mostFutureOffset = 0;
	}

	// Don't overwrite unused data
	if (nextUseOffset / slotSize == nextReadSlot) {
		// Buffer is full!
		return false;
	}

	if ((int)stream.read(bigBuffer + nextReadSlot * slotSize, slotSize) != slotSize) {
		bFileEnd = true;
	}

	// Read next slot next time
	nextReadSlot++;

	// Find new mostFutureOffset
	nextOffset = FollowingPacket(mostFutureOffset, false);
	while (nextOffset < nextReadSlot*slotSize
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
bool BMVPlayer::DoBMVFrame() {
	unsigned char *data;
	int	graphOffset, length;
	signed short	xscr;

	if (nextUseOffset == wrapUseOffset) {
		nextUseOffset %= slotSize;
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
				nextUseOffset %= slotSize;
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
			nextUseOffset %= slotSize;
			wrapUseOffset = -1;
		}
		numAdvancePackets--;
		return false;

	case CD_LE_FIN:
		FinishBMV();
		numAdvancePackets--;
		return true;

	default:
		length = (int32)READ_32(data + 1);
		length &= 0x00ffffff;

		graphOffset = nextUseOffset + 4;	// Skip command byte and length

		if (*data & CD_AUDIO) {
			if (TinselVersion == 3) {
				int audioSize = audioMaxSize;
				if (*data & CD_EXTEND) {
					audioSize -= audioBlobSize;
				}

				//MovieAudio(graphOffset, audioSize);
				graphOffset += audioSize;
				length -= audioSize;
			} else if (bOldAudio) {
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
			if (TinselVersion != 3) { // TinselV3 has palette embeded in the video frame
				MoviePalette(graphOffset);
			}
			graphOffset += sz_CMAP_pkt;	// Skip palette data
			length -= sz_CMAP_pkt;
		}

		if (*data & CD_XSCR) {
			xscr = (int16)READ_16(bigBuffer + graphOffset);
			graphOffset += sz_XSCR_pkt;	// Skip scroll offset
			length -= sz_XSCR_pkt;
		} else if (*data & BIT0)
			xscr = -640 * bpp;
		else
			xscr = 0;

		if (TinselVersion == 3) {
			if (length > 0) {
				t3PrepBMV(bigBuffer + graphOffset, length, xscr);
				currentFrame++;
			}
		} else {
			PrepBMV(bigBuffer + graphOffset, length, xscr);
			currentFrame++;
		}

		numAdvancePackets--;

		nextUseOffset = FollowingPacket(nextUseOffset, true);
		if (nextUseOffset == wrapUseOffset) {
			nextUseOffset %= slotSize;
			wrapUseOffset = -1;
		}
		return true;
	}
}

/**
 * DoSoundFrame
 */
bool BMVPlayer::DoSoundFrame() {
	unsigned char *data;
	int	graphOffset;

	if (nextSoundOffset == wrapUseOffset) {
		nextSoundOffset %= slotSize;
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
				nextSoundOffset %= slotSize;
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
			nextSoundOffset %= slotSize;
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
			nextSoundOffset %= slotSize;
		}
		currentSoundFrame++;
		return true;
	}
}

/**
 * CopyMovieToScreen
 */
void BMVPlayer::CopyMovieToScreen() {
	// Not if not up and running yet!
	if (!screenBuffer || (currentFrame == 0)) {
		_vm->_bg->DrawBackgnd();
		return;
	}

	if (TinselVersion == 3) {
		// Videos in Tinsel V3 are using 432 lines
		memcpy(_vm->screen().getPixels(), ScreenBeg, SCREEN_WIDTH * SCREEN_HIGH * bpp);
	} else {
		// The movie surface is slightly less high than the output screen (429 rows versus 432).
		// Because of this, there's some extra line clearing above and below the displayed area
		int yStart = (SCREEN_HEIGHT - SCREEN_HIGH) / 2;
		memset(_vm->screen().getPixels(), 0, yStart * SCREEN_WIDTH * bpp);
		memcpy(_vm->screen().getBasePtr(0, yStart), ScreenBeg, SCREEN_WIDTH * SCREEN_HIGH * bpp);
		memset(_vm->screen().getBasePtr(0, yStart + SCREEN_HIGH), 0, (SCREEN_HEIGHT - SCREEN_HIGH - yStart) * SCREEN_WIDTH * bpp);

		PalettesToVideoDAC(); // Keep palette up-to-date
	}

	BmvDrawText(true);
	UpdateScreenRect(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	g_system->updateScreen();
	BmvDrawText(false);
}

/**
 * Handles playback of any active movie. Called from the foreground 24 times a second.
 */
void BMVPlayer::FettleBMV() {

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

	if (!stream.isOpen()) {
		int i;

		// First time in with this movie

		InitializeBMV();

		if (TinselVersion == 3) {
			startTick = -1;
		} else {
			for (i = 0; i < ADVANCE_SOUND;) {
				if (DoSoundFrame()) {
					i++;
				}
			}
			startTick = -ONE_SECOND / 4;	// 1/4 second
		}
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

	if ((TinselVersion != 3) && (bigProblemCount < PT_A)) {
		refFrame = currentSoundFrame;

		while (currentSoundFrame < ((tick+1-startTick)/frameTime + ADVANCE_SOUND) && bMovieOn) {
			if (currentSoundFrame == refFrame+PT_B)
				break;

			DoSoundFrame();
		}
	}

	// Time to process a frame (or maybe more)
	if ((TinselVersion != 3) && (bigProblemCount < PT_A)) {
		refFrame = currentFrame;

		while ((currentFrame < (tick-startTick)/frameTime) && bMovieOn) {
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
		if ((currentFrame == refFrame) || (currentFrame <= refFrame+3)) {
			bigProblemCount = 0;
		}
	} else {
		while (currentFrame < (tick-startTick)/frameTime && bMovieOn) {
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
bool BMVPlayer::MoviePlaying() {
	return bMovieOn;
}

/**
 * Returns the audio lag in ms
 */
int32 BMVPlayer::MovieAudioLag() {
	if (!bMovieOn || !_audioStream || (TinselVersion == 3))
		return 0;

	// Calculate lag
	int32 playLength = (movieTick - startTick - 1) * ((((uint32) 1000) << 10) / 24);
	return (playLength - (((int32) _vm->_mixer->getSoundElapsedTime(_audioHandle)) << 10)) >> 10;
}

uint32 BMVPlayer::NextMovieTime() {
	return nextMovieTime;
}

void BMVPlayer::AbortMovie() {
	bAbort = true;
}

} // End of namespace Tinsel
