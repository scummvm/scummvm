/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Background animation management module
#include "saga/saga.h"
#include "saga/gfx.h"

#include "saga/console.h"
#include "saga/events.h"
#include "saga/render.h"

#include "saga/animation.h"

namespace Saga {

Anim::Anim(SagaEngine *vm) : _vm(vm) {
	uint16 i;

	for (i = 0; i < MAX_ANIMATIONS; i++)
		_animations[i] = NULL;
}

Anim::~Anim(void) {
	reset();
}

void Anim::load(uint16 animId, const byte *animResourceData, size_t animResourceLength) {
	AnimationData *anim;

	if (animId >= MAX_ANIMATIONS) {
		error("Anim::load could not find unused animation slot");
	}

	anim = _animations[animId] = new AnimationData(animResourceData, animResourceLength);

	MemoryReadStreamEndian headerReadS(anim->resourceData, anim->resourceLength, IS_BIG_ENDIAN);
	anim->magic = headerReadS.readUint16LE(); // cause ALWAYS LE
	anim->screenWidth = headerReadS.readUint16();
	anim->screenHeight = headerReadS.readUint16();

	anim->unknown06 = headerReadS.readByte();
	anim->unknown07 = headerReadS.readByte();
	anim->maxFrame = headerReadS.readByte() - 1;
	anim->loopFrame = headerReadS.readByte() - 1;
	anim->start = headerReadS.readUint16BE();

	anim->start += headerReadS.pos();


	if (_vm->getGameType() == GType_ITE) {
		// Cache frame offsets
		anim->frameOffsets = (size_t *)malloc((anim->maxFrame + 1) * sizeof(*anim->frameOffsets));
		if (anim->frameOffsets == NULL) {
			memoryError("Anim::load");
		}

		fillFrameOffsets(anim);
	} else {
		anim->cur_frame_p = anim->resourceData + SAGA_FRAME_HEADER_LEN; // ? len - may vary
		anim->cur_frame_len = anim->resourceLength - SAGA_FRAME_HEADER_LEN;
	}

	// Set animation data
	anim->currentFrame = 0;
	anim->completed = 0;
	anim->cycles = anim->maxFrame;

	anim->frameTime = DEFAULT_FRAME_TIME;
	anim->flags = 0;
	anim->linkId = -1;
	anim->state = ANIM_PAUSE;	
}

void Anim::link(int16 animId1, int16 animId2) {
	AnimationData *anim1;
	AnimationData *anim2;

	anim1 = getAnimation(animId1);

	anim1->linkId = animId2;

	if (animId2 == -1) {
		return;
	}

	anim2 = getAnimation(animId2);
	anim2->frameTime = anim1->frameTime;
}

void Anim::setCycles(uint16 animId, int cycles) {
	AnimationData *anim;

	anim = getAnimation(animId);
	
	anim->cycles = cycles;
}

void Anim::play(uint16 animId, int vectorTime, bool playing) {
	EVENT event;
	BUFFER_INFO buf_info;

	byte *displayBuffer;

	const byte *nextf_p;
	size_t nextf_len;

	uint16 frame;
	int frameTime;
	int result;

	AnimationData *anim;
	AnimationData *linkAnim;

	anim = getAnimation(animId);

	_vm->_render->getBufferInfo(&buf_info);
	displayBuffer = buf_info.bg_buf;


	if (playing) {
		anim->state = ANIM_PLAYING;
	}

	if (anim->state == ANIM_PAUSE) {
		return;
	}

	if (anim->completed < anim->cycles) {
		frame = anim->currentFrame;
		if (_vm->getGameType() == GType_ITE) {
			// FIXME: if start > 0, then this works incorrectly
			ITE_DecodeFrame(anim, anim->frameOffsets[frame], displayBuffer,
							_vm->getDisplayWidth() * _vm->getDisplayHeight());
		} else {
			if (anim->cur_frame_p == NULL) {
				warning("Anim::play: Frames exhausted");
				return;
			}

			result = IHNM_DecodeFrame(displayBuffer,  _vm->getDisplayWidth() * _vm->getDisplayHeight(),
									anim->cur_frame_p, anim->cur_frame_len, &nextf_p, &nextf_len);
			if (result != SUCCESS) {
				warning("Anim::play: Error decoding frame %u", anim->currentFrame);
				anim->state = ANIM_PAUSE;
				return;
			}

			anim->cur_frame_p = nextf_p;
			anim->cur_frame_len = nextf_len;
		}

		anim->currentFrame++;
		if (anim->completed != 65535) {
			anim->completed++;
		}

		if (anim->currentFrame > anim->maxFrame) {
			anim->currentFrame = anim->loopFrame;
			
			if (_vm->getGameType() == GType_IHNM) {
				// FIXME: HACK. probably needs more testing for IHNM
				anim->cur_frame_p = anim->resourceData + SAGA_FRAME_HEADER_LEN;
				anim->cur_frame_len = anim->resourceLength - SAGA_FRAME_HEADER_LEN;
			}
			
			if (anim->flags & ANIM_STOPPING || anim->currentFrame == -1) {
				anim->state = ANIM_PAUSE;
			}
		}

	} else {
		// Animation done playing
		if (anim->linkId != -1) {
			// If this animation has a link, follow it
			anim->currentFrame = 0;
			anim->completed = 0;
			anim->state = ANIM_PAUSE;

		} else {
			// No link, stop playing
			anim->currentFrame = anim->maxFrame;
			anim->state = ANIM_PAUSE;

			if (anim->flags & ANIM_ENDSCENE) {
				// This animation ends the scene
				event.type = ONESHOT_EVENT;
				event.code = SCENE_EVENT;
				event.op = EVENT_END;
				event.time = anim->frameTime + vectorTime;
				_vm->_events->queue(&event);
			}
			return;
		}
	}

	if (anim->state == ANIM_PAUSE && anim->linkId != -1) {
		// If this animation has a link, follow it
		linkAnim = getAnimation(anim->linkId);

		debug(5, "Animation ended going to %d", anim->linkId);
		linkAnim->currentFrame = 0;
		linkAnim->completed = 0;
		linkAnim->state = ANIM_PLAYING;
		animId = anim->linkId;
		frameTime = 0;
	} else {
		frameTime = anim->frameTime + vectorTime;
	}

	event.type = ONESHOT_EVENT;
	event.code = ANIM_EVENT;
	event.op = EVENT_FRAME;
	event.param = animId;
	event.time = frameTime;

	_vm->_events->queue(&event);

}

void Anim::stop(uint16 animId) {
	AnimationData *anim;

	anim = getAnimation(animId);

	anim->state = ANIM_PAUSE;
}

void Anim::finish(uint16 animId) {
	AnimationData *anim;

	anim = getAnimation(animId);

	anim->state = ANIM_STOPPING;
}

void Anim::resume(uint16 animId, int cycles) {
	AnimationData *anim;

	anim = getAnimation(animId);
	
	anim->cycles += cycles;
	play(animId, 0, true);
}

void Anim::reset() {
	uint16 i;

	for (i = 0; i < MAX_ANIMATIONS; i++) {
		if (_animations[i] != NULL) {
			delete _animations[i];
			_animations[i] = NULL;
		}
	}
}

void Anim::setFlag(uint16 animId, uint16 flag) {
	AnimationData *anim;

	anim = getAnimation(animId);

	anim->flags |= flag;
}

void Anim::clearFlag(uint16 animId, uint16 flag) {
	AnimationData *anim;

	anim = getAnimation(animId);

	anim->flags &= ~flag;
}

void Anim::setFrameTime(uint16 animId, int time) {
	AnimationData *anim;

	anim = getAnimation(animId);

	anim->frameTime = time;
}

int16 Anim::getCurrentFrame(uint16 animId) {
	AnimationData *anim;

	anim = getAnimation(animId);
	
	return anim->currentFrame;
}

void Anim::ITE_DecodeFrame(AnimationData *anim, size_t frameOffset, byte *buf, size_t bufLength) {
	FRAME_HEADER fh;

	byte *write_p;

	uint16 magic;

	uint16 xStart;
	uint16 yStart;
	uint32 screenWidth;
	uint32 screenHeight;

	int mark_byte;
	byte data_byte;
	int new_row;

	uint16 control_ch;
	uint16 param_ch;

	uint16 runcount;
	int x_vector;

	uint16 i;

	screenWidth = anim->screenWidth;
	screenHeight = anim->screenHeight;

	if ((screenWidth * screenHeight) > bufLength) {
		// Buffer argument is too small to hold decoded frame, abort.
		error("ITE_DecodeFrame: Buffer size inadequate");
	}

	MemoryReadStream readS(anim->resourceData + frameOffset, anim->resourceLength - frameOffset);

	// Check for frame magic byte
	magic = readS.readByte();
	if (magic == SAGA_FRAME_END) {
		return;
	}

	if (magic != SAGA_FRAME_START) {
		error("ITE_DecodeFrame: Invalid frame offset %x", frameOffset);
	}


	fh.xStart = readS.readUint16BE();
	if (_vm->getFeatures() & GF_BIG_ENDIAN_DATA)
		fh.yStart = readS.readUint16BE();
	else
		fh.yStart = readS.readByte();
	readS.readByte();		/* Skip pad byte */
	fh.xPos = readS.readUint16BE();
	fh.yPos = readS.readUint16BE();
	fh.width = readS.readUint16BE();
	fh.height = readS.readUint16BE();

	xStart = fh.xStart;
	yStart = fh.yStart;

#if 1
#define VALIDATE_WRITE_POINTER \
	if ((write_p < buf) || (write_p >= (buf + screenWidth * screenHeight))) { \
		error("VALIDATE_WRITE_POINTER: write_p=%x buf=%x", write_p, buf); \
	}
#else
#define VALIDATE_WRITE_POINTER 
#endif

	// Setup write pointer to the draw origin
	write_p = (buf + (yStart * screenWidth) + xStart);
	VALIDATE_WRITE_POINTER;

	// Begin RLE decompression to output buffer
	do {
		mark_byte = readS.readByte();
		switch (mark_byte) {
		case SAGA_FRAME_LONG_UNCOMPRESSED_RUN: // Long Unencoded Run
			runcount = readS.readSint16BE();
			for (i = 0; i < runcount; i++) {
				data_byte = readS.readByte();
				if (data_byte != 0) {
					*write_p = data_byte;
				}
				write_p++;
				VALIDATE_WRITE_POINTER;
			}
			continue;
			break;
		case SAGA_FRAME_LONG_COMPRESSED_RUN: // Long encoded run
			runcount = readS.readSint16BE();
			data_byte = readS.readByte();
			for (i = 0; i < runcount; i++) {
				*write_p++ = data_byte;
				VALIDATE_WRITE_POINTER;
			}
			continue;
			break;
		case SAGA_FRAME_ROW_END: // End of row
			x_vector = readS.readSint16BE();
			
			if (_vm->getFeatures() & GF_BIG_ENDIAN_DATA)
				new_row = readS.readSint16BE();
			else
				new_row = readS.readByte();

			// Set write pointer to the new draw origin
			write_p = buf + ((yStart + new_row) * screenWidth) + xStart + x_vector;
			VALIDATE_WRITE_POINTER;
			continue;
			break;
		case SAGA_FRAME_REPOSITION: // Reposition command
			x_vector = readS.readSint16BE();
			write_p += x_vector;
			VALIDATE_WRITE_POINTER;
			continue;
			break;
		case SAGA_FRAME_END: // End of frame marker
			return;
			break;
		default:
			break;
		}

		// Mask all but two high order control bits
		control_ch = mark_byte & 0xC0U;
		param_ch = mark_byte & 0x3FU;
		switch (control_ch) {
		case SAGA_FRAME_EMPTY_RUN: // 1100 0000
			// Run of empty pixels
			runcount = param_ch + 1;
			write_p += runcount;
			VALIDATE_WRITE_POINTER;
			continue;
			break;
		case SAGA_FRAME_COMPRESSED_RUN: // 1000 0000
			// Run of compressed data
			runcount = param_ch + 1;
			data_byte = readS.readByte();
			for (i = 0; i < runcount; i++) {
				*write_p++ = data_byte;
				VALIDATE_WRITE_POINTER;
			}
			continue;
			break;
		case SAGA_FRAME_UNCOMPRESSED_RUN: // 0100 0000
			// Uncompressed run
			runcount = param_ch + 1;
			for (i = 0; i < runcount; i++) {
				data_byte = readS.readByte();
				if (data_byte != 0) {
					*write_p = data_byte;
				}
				write_p++;
				VALIDATE_WRITE_POINTER;
			}
			continue;
			break;
		default:
			// Unknown marker found - abort
			error("ITE_DecodeFrame: Invalid RLE marker encountered");
			break;
		}
	} while (mark_byte != 63); // end of frame marker

}

int Anim::IHNM_DecodeFrame(byte *decode_buf, size_t decode_buf_len, const byte *thisf_p,
					size_t thisf_len, const byte **nextf_p, size_t *nextf_len) {
	int in_ch;
	int decoded_data = 0;
	int cont_flag = 1;
	int control_ch;
	int param_ch;
	byte data_pixel;
	int x_origin = 0;
	int y_origin = 0;
	int x_vector;
	int new_row;

	uint16 runcount;
	uint16 c;

	size_t in_ch_offset;

	MemoryReadStreamEndian readS(thisf_p, thisf_len, !IS_BIG_ENDIAN); // RLE has inversion BE<>LE

	byte *outbuf_p = decode_buf;
	byte *outbuf_endp = (decode_buf + decode_buf_len) - 1;
	size_t outbuf_remain = decode_buf_len;


	*nextf_p = NULL;

	for (; cont_flag; decoded_data = 1) {
		in_ch_offset = readS.pos();
		in_ch = readS.readByte();
		switch (in_ch) {
		case 0x0F: // 15: Frame header
			{
				int param1;
				int param2;
				int param3;
				int param4;
				int param5;
				int param6;

				if (thisf_len - readS.pos() < 13) {
					warning("0x%02X: Input buffer underrun", in_ch);
					return FAILURE;
				}

				param1 = readS.readUint16();
				param2 = readS.readUint16();
				readS.readByte(); // skip 1?
				param3 = readS.readUint16();
				param4 = readS.readUint16();
				param5 = readS.readUint16();
				param6 = readS.readUint16();

				x_origin = param1;
				y_origin = param2;

				outbuf_p = decode_buf + x_origin + (y_origin * _vm->getDisplayWidth());

				if (outbuf_p > outbuf_endp) {
					warning("0x%02X: (0x%X) Invalid output position. (x: %d, y: %d)",
							in_ch, in_ch_offset, x_origin, y_origin);
					return FAILURE;
				}

				outbuf_remain = (outbuf_endp - outbuf_p) + 1;
				continue;
			}
			break;
		case SAGA_FRAME_LONG_UNCOMPRESSED_RUN: // Long Unencoded Run
			runcount = readS.readSint16();
			if (thisf_len - readS.pos() < runcount) {
				warning("0x%02X: Input buffer underrun", in_ch);
				return FAILURE;
			}
			if (outbuf_remain < runcount) {
				warning("0x%02X: Output buffer overrun", in_ch);
				return FAILURE;
			}

			for (c = 0; c < runcount; c++) {
				data_pixel = readS.readByte();
				if (data_pixel != 0) {
					*outbuf_p = data_pixel;
				}
				outbuf_p++;
			}

			outbuf_remain -= runcount;
			continue;
			break;
		case 0x1F: // 31: Unusued?
			if (thisf_len - readS.pos() < 3) {
				warning("0x%02X: Input buffer underrun", in_ch);
				return FAILURE;
			}

			readS.readByte();
			readS.readByte();
			readS.readByte();
			continue;
			break;
		case SAGA_FRAME_LONG_COMPRESSED_RUN: // Long compressed run
			if (thisf_len - readS.pos() <= 3) {
				warning("0x%02X: Input buffer underrun", in_ch);
				return FAILURE;
			}

			runcount = readS.readSint16();
			data_pixel = readS.readByte();

			for (c = 0; c < runcount; c++) {
				*outbuf_p++ = data_pixel;
			}

			outbuf_remain -= runcount;
			continue;
			break;

		case SAGA_FRAME_ROW_END: // End of row
			if (thisf_len - readS.pos() <= 4) {
				return FAILURE;
			}

			x_vector = readS.readSint16();
			new_row = readS.readSint16();

			outbuf_p = decode_buf + ((y_origin + new_row) * _vm->getDisplayWidth()) + x_origin + x_vector;
			outbuf_remain = (outbuf_endp - outbuf_p) + 1;
			continue;
			break;
		case SAGA_FRAME_REPOSITION: // Reposition command
			if (thisf_len - readS.pos() < 2) {
				return FAILURE;
			}

			x_vector = readS.readSint16();

			if (((x_vector > 0) && ((size_t) x_vector > outbuf_remain)) || (-x_vector > outbuf_p - decode_buf)) {
				warning("SAGA_FRAME_REPOSITION: Invalid x_vector");
				return FAILURE;
			}

			outbuf_p += x_vector;
			outbuf_remain -= x_vector;
			continue;
			break;

		case SAGA_FRAME_END:	// Frame end marker
			debug(1, "SAGA_FRAME_END: Frame end marker");
			if (decoded_data && (thisf_len - readS.pos() > 0)) {
				*nextf_p = thisf_p + readS.pos();
				*nextf_len = thisf_len - readS.pos();
			} else {
				*nextf_p = NULL;
				*nextf_len = 0;
			}

			cont_flag = 0;
			continue;
			break;

		default:
			break;
		}

		control_ch = in_ch & 0xC0;
		param_ch = in_ch & 0x3f;
		switch (control_ch) {

		case SAGA_FRAME_EMPTY_RUN: // Run of empty pixels
			runcount = param_ch + 1;
			if (outbuf_remain < runcount) {
				return FAILURE;
			}

			outbuf_p += runcount;
			outbuf_remain -= runcount;
			continue;
			break;
		case SAGA_FRAME_COMPRESSED_RUN: // Run of compressed data
			runcount = param_ch + 1;
			if ((outbuf_remain < runcount) || (thisf_len - readS.pos() <= 1)) {
				return FAILURE;
			}

			data_pixel = readS.readByte();

			for (c = 0; c < runcount; c++) {
				*outbuf_p++ = data_pixel;
			}

			outbuf_remain -= runcount;
			continue;
			break;
		case SAGA_FRAME_UNCOMPRESSED_RUN: // Uncompressed run
			runcount = param_ch + 1;
			if ((outbuf_remain < runcount) || (thisf_len - readS.pos() < runcount)) {
				return FAILURE;
			}

			for (c = 0; c < runcount; c++) {
				data_pixel = readS.readByte();
				if (data_pixel != 0) {
					*outbuf_p = data_pixel;
				}
				outbuf_p++;
			}

			outbuf_remain -= runcount;

			continue;
			break;

		default:
			break;
		}
	}

	return SUCCESS;
}

void Anim::fillFrameOffsets(AnimationData *anim) {
	uint16 currentFrame;

	byte mark_byte;
	uint16 control;
	uint16 runcount;
	uint16 magic;

	int i;

	MemoryReadStreamEndian readS(anim->resourceData, anim->resourceLength, IS_BIG_ENDIAN); 

	readS.seek(12);
	

	readS._bigEndian = !IS_BIG_ENDIAN; // RLE has inversion BE<>LE

	for (currentFrame = 0; currentFrame <= anim->maxFrame; currentFrame++) {
		anim->frameOffsets[currentFrame] = readS.pos();
		magic = readS.readByte();
		if (magic == SAGA_FRAME_END) {
			if (currentFrame != anim->maxFrame) {
				error("currentFrame != anim->maxFrame");
			}
			break;
		}
		if (magic != SAGA_FRAME_START) {
			error("Frame sync failure. Magic Number not found");
		}

		// skip header
		readS.seek(SAGA_FRAME_HEADER_LEN, SEEK_CUR);

		// For some strange reason, the animation header is in little
		// endian format, but the actual RLE encoded frame data, 
		// including the frame header, is in big endian format. */
		do {
			mark_byte = readS.readByte();
//			debug(7, "_pos=%x mark_byte=%x", readS.pos(), mark_byte);

			switch (mark_byte) {
			case SAGA_FRAME_END: // End of frame marker
				continue;
				break;
			case SAGA_FRAME_REPOSITION: // Reposition command
				readS.readSint16BE();
				continue;
				break;
			case SAGA_FRAME_ROW_END: // End of row marker
				readS.readSint16BE();
				if (_vm->getFeatures() & GF_BIG_ENDIAN_DATA)
					readS.readSint16BE();
				else
					readS.readByte();
				continue;
				break;
			case SAGA_FRAME_LONG_COMPRESSED_RUN: // Long compressed run marker
				readS.readSint16BE();
				readS.readByte();
				continue;
				break;
			case SAGA_FRAME_LONG_UNCOMPRESSED_RUN: // (16) 0001 0000
				// Long Uncompressed Run
				runcount = readS.readSint16BE();
				for (i = 0; i < runcount; i++)
					readS.readByte();
				continue;
				break;
			default:
				break;
			}

			// Mask all but two high order (control) bits
			control = mark_byte & 0xC0;
			switch (control) {
			case SAGA_FRAME_EMPTY_RUN:
				// Run of empty pixels
				continue;
				break;
			case SAGA_FRAME_COMPRESSED_RUN:
				// Run of compressed data
				readS.readByte(); // Skip data byte
				continue;
				break;
			case SAGA_FRAME_UNCOMPRESSED_RUN:
				// Uncompressed run
				runcount = (mark_byte & 0x3f) + 1;
				for (i = 0; i < runcount; i++)
					readS.readByte();
				continue;
				break;
			default:
				error("Encountered unknown RLE marker");
				break;
			}
		} while (mark_byte != SAGA_FRAME_END);
	}
}

void Anim::animInfo() {
	uint16 animCount;
	uint16 i;

	animCount = getAnimationCount();

	_vm->_console->DebugPrintf("There are %d animations loaded:\n", animCount);

	for (i = 0; i < MAX_ANIMATIONS; i++) {
		if (_animations[i] == NULL) {
			continue;
		}

		_vm->_console->DebugPrintf("%02d: Frames: %u Flags: %u\n", i, _animations[i]->maxFrame, _animations[i]->flags);
	}
}

} // End of namespace Saga
