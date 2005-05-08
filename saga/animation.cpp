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
	int i;

	_anim_limit = MAX_ANIMATIONS;
	_anim_count = 0;

	for (i = 0; i < MAX_ANIMATIONS; i++)
		_anim_tbl[i] = NULL;

	_initialized = true;
}

Anim::~Anim(void) {
	uint16 i;

	for (i = 0; i < MAX_ANIMATIONS; i++)
		free(_anim_tbl[i]);

	_initialized = false;
}

int Anim::load(const byte *anim_resdata, size_t anim_resdata_len, uint16 *anim_id_p) {
	ANIMATION *new_anim;
	ANIMATION_HEADER ah;

	uint16 anim_id = 0;
	uint16 i;

	if (!_initialized) {
		warning("Anim::load not initialised");
		return FAILURE;
	}

	// Find an unused animation slot
	for (i = 0; i < MAX_ANIMATIONS; i++) {
		if (_anim_tbl[i] == NULL) {
			anim_id = i;
			break;
		}
	}

	if (i == MAX_ANIMATIONS) {
		warning("Anim::load could not find unused animation slot");
		return FAILURE;
	}

	new_anim = (ANIMATION *)malloc(sizeof(*new_anim));
	if (new_anim == NULL) {
		warning("Anim::load Allocation failure");
		return MEM;
	}

	new_anim->resdata = anim_resdata;
	new_anim->resdata_len = anim_resdata_len;

	MemoryReadStreamEndian headerReadS(anim_resdata, anim_resdata_len, IS_BIG_ENDIAN);

	readAnimHeader(headerReadS, ah);
	new_anim->maxframe = ah.maxframe;
	new_anim->loopframe = ah.loopframe;

	if (_vm->getGameType() == GType_ITE) {
		// Cache frame offsets
		new_anim->frame_offsets = (size_t *)malloc((new_anim->maxframe + 1) * sizeof(*new_anim->frame_offsets));
		if (new_anim->frame_offsets == NULL) {
			warning("Anim::load Allocation failure");
			return MEM;
		}

		for (i = 0; i <= new_anim->maxframe; i++) {
			getFrameOffset(anim_resdata, anim_resdata_len, i, &new_anim->frame_offsets[i]);
		}
	} else {
		new_anim->cur_frame_p = anim_resdata + SAGA_FRAME_HEADER_LEN; // ? len - may vary
		new_anim->cur_frame_len = anim_resdata_len - SAGA_FRAME_HEADER_LEN;
	}

	// Set animation data
	new_anim->current_frame = 0;
	new_anim->completed = 0;
	new_anim->cycles = new_anim->maxframe;

	new_anim->frame_time = DEFAULT_FRAME_TIME;
	new_anim->flags = 0;
	new_anim->link_id = -1;
	new_anim->state = ANIM_PAUSE;

	_anim_tbl[anim_id] = new_anim;

	*anim_id_p = anim_id;

	_anim_count++;

	return SUCCESS;
}

int Anim::link(int16 anim_id1, int16 anim_id2) {
	ANIMATION *anim1;
	ANIMATION *anim2;

	if ((anim_id1 >= _anim_count) || (anim_id2 >= _anim_count)) {
		return FAILURE;
	}

	anim1 = _anim_tbl[anim_id1];

	anim1->link_id = anim_id2;

	if (anim_id2 == -1)
		return SUCCESS;

	anim2 = _anim_tbl[anim_id2];

	if ((anim1 == NULL) || (anim2 == NULL)) {
		return FAILURE;
	}

	anim2->frame_time = anim1->frame_time;

	return SUCCESS;
}

void Anim::setCycles(uint animId, int cycles) {
	if (animId >= _anim_count) {
		warning("Anim::setCycles(): wrong animation number (%d)", animId);
		return;
	}
	
	_anim_tbl[animId]->cycles = cycles;
}

int Anim::play(uint16 anim_id, int vector_time, bool playing) {
	EVENT event;
	ANIMATION *anim;
	ANIMATION *link_anim;
	uint16 link_anim_id;

	BUFFER_INFO buf_info;

	byte *display_buf;

	const byte *nextf_p;
	size_t nextf_len;

	uint16 frame;
	int frame_time;
	int result;

	if (anim_id >= _anim_count) {
		return FAILURE;
	}

	_vm->_render->getBufferInfo(&buf_info);
	display_buf = buf_info.bg_buf;

	anim = _anim_tbl[anim_id];
	if (anim == NULL) {
		return FAILURE;
	}

	if (playing)
		anim->state = ANIM_PLAYING;

	if (anim->state == ANIM_PAUSE)
		return SUCCESS;

	if (anim->completed < anim->cycles) {
		frame = anim->current_frame;
		if (_vm->getGameType() == GType_ITE) {
			// FIXME: if start > 0, then this works incorrectly
			result = ITE_DecodeFrame(anim->resdata, anim->resdata_len, anim->frame_offsets[frame], display_buf,
									_vm->getDisplayWidth() * _vm->getDisplayHeight());
			if (result != SUCCESS) {
				warning("Anim::play: Error decoding frame %u", anim->current_frame);
				anim->state = ANIM_PAUSE;
				return FAILURE;
			}
		} else {
			if (anim->cur_frame_p == NULL) {
				warning("Anim::play: Frames exhausted");
				return FAILURE;
			}

			result = IHNM_DecodeFrame(display_buf,  _vm->getDisplayWidth() * _vm->getDisplayHeight(),
									anim->cur_frame_p, anim->cur_frame_len, &nextf_p, &nextf_len);
			if (result != SUCCESS) {
				warning("Anim::play: Error decoding frame %u", anim->current_frame);
				anim->state = ANIM_PAUSE;
				return FAILURE;
			}

			anim->cur_frame_p = nextf_p;
			anim->cur_frame_len = nextf_len;
		}
		anim->current_frame++;
		anim->completed++;

		if (anim->current_frame > anim->maxframe) {
			anim->current_frame = anim->loopframe;
			
			// FIXME: HACK. probably needs more testing for IHNM
			anim->cur_frame_p = anim->resdata + SAGA_FRAME_HEADER_LEN;
			anim->cur_frame_len = anim->resdata_len - SAGA_FRAME_HEADER_LEN;

			if (anim->flags & ANIM_STOPPING || anim->current_frame == -1)
				anim->state = ANIM_PAUSE;
		}

	} else {
		// Animation done playing
		if (anim->link_id != -1) {
			// If this animation has a link, follow it
			anim->current_frame = 0;
			anim->state = ANIM_PAUSE;

			link_anim_id = anim->link_id;
			link_anim = _anim_tbl[link_anim_id];

			if (link_anim != NULL) {
				link_anim->current_frame = 0;
				link_anim->state = ANIM_PLAYING;
			}

			anim_id = link_anim_id;
		} else {
			// No link, stop playing
			anim->current_frame = anim->maxframe;
			anim->state = ANIM_PAUSE;

			if (anim->flags & ANIM_ENDSCENE) {
				// This animation ends the scene
				event.type = ONESHOT_EVENT;
				event.code = SCENE_EVENT;
				event.op = EVENT_END;
				event.time = anim->frame_time + vector_time;
				_vm->_events->queue(&event);
			}
			return SUCCESS;
		}
	}

	if (anim->state == ANIM_PAUSE && anim->link_id != -1) {
		// If this animation has a link, follow it
		link_anim_id = anim->link_id;
		link_anim = _anim_tbl[link_anim_id];

		if (link_anim != NULL) {
			debug(5, "Animation ended going to %d", link_anim_id);
			link_anim->current_frame = 0;
			link_anim->state = ANIM_PLAYING;
		}
		anim_id = link_anim_id;
		frame_time = 0;
	} else {
		frame_time = anim->frame_time + vector_time;
	}

	event.type = ONESHOT_EVENT;
	event.code = ANIM_EVENT;
	event.op = EVENT_FRAME;
	event.param = anim_id;
	event.time = frame_time;

	_vm->_events->queue(&event);

	return SUCCESS;
}

void Anim::stop(uint16 animId) {
	if (animId >= _anim_count) {
		warning("Anim::stop(): wrong animation number (%d)", animId);
		return;
	}
	
	_anim_tbl[animId]->state = ANIM_PAUSE;
}

void Anim::finish(uint16 animId) {
	if (animId >= _anim_count) {
		warning("Anim::finish(): wrong animation number (%d)", animId);
		return;
	}
	
	_anim_tbl[animId]->state = ANIM_STOPPING;
}

void Anim::resume(uint16 animId, int cycles) {
	if (animId >= _anim_count) {
		warning("Anim::resume(): wrong animation number (%d)", animId);
		return;
	}
	
	_anim_tbl[animId]->cycles += cycles;
	play(animId, 0, true);
}

int Anim::reset() {
	uint16 i;

	for (i = 0; i < MAX_ANIMATIONS; i++) {

		freeId(i);
	}

	_anim_count = 0;

	return SUCCESS;
}

int Anim::setFlag(uint16 anim_id, uint16 flag) {
	ANIMATION *anim;

	if (anim_id > _anim_count) {
		return FAILURE;
	}

	anim = _anim_tbl[anim_id];
	if (anim == NULL) {
		return FAILURE;
	}

	anim->flags |= flag;

	return SUCCESS;
}

int Anim::clearFlag(uint16 anim_id, uint16 flag) {
	ANIMATION *anim;

	if (anim_id > _anim_count) {
		return FAILURE;
	}

	anim = _anim_tbl[anim_id];
	if (anim == NULL) {
		return FAILURE;
	}

	anim->flags &= ~flag;

	return SUCCESS;
}

int Anim::setFrameTime(uint16 anim_id, int time) {
	ANIMATION *anim;

	if (anim_id > _anim_count) {
		return FAILURE;
	}

	anim = _anim_tbl[anim_id];
	if (anim == NULL) {
		return FAILURE;
	}

	anim->frame_time = time;

	return SUCCESS;
}

int16 Anim::getCurrentFrame(uint16 animId) {
	if (animId >= _anim_count) {
		warning("Anim::stop(): wrong animation number (%d)", animId);
		return 0;
	}
	
	return _anim_tbl[animId]->current_frame;
}

int Anim::freeId(uint16 anim_id) {
	ANIMATION *anim;

	if (anim_id > _anim_count) {
		return FAILURE;
	}

	anim = _anim_tbl[anim_id];
	if (anim == NULL) {
		return FAILURE;
	}

	if (_vm->getGameType() == GType_ITE) {
		free(anim->frame_offsets);
		anim->frame_offsets = NULL;
	}

	free(anim);
	_anim_tbl[anim_id] = NULL;
	_anim_count--;

	return SUCCESS;
}

void Anim::readAnimHeader(MemoryReadStreamEndian &readS, ANIMATION_HEADER &ah) {
	ah.magic = readS.readUint16LE(); // cause ALWAYS LE
	ah.screen_w = readS.readUint16();
	ah.screen_h = readS.readUint16();

	ah.unknown06 = readS.readByte();
	ah.unknown07 = readS.readByte();
	ah.maxframe = readS.readByte() - 1;
	ah.loopframe = readS.readByte() - 1;
	ah.start = readS.readUint16BE();

	if (ah.start != 65535 && ah.start != 0)
		warning("Anim::readAnimHeader(): found different start: %d. Fix Anim::play()", ah.start);
	ah.start += readS.pos();
}

int Anim::ITE_DecodeFrame(const byte *resdata, size_t resdata_len, size_t frame_offset, byte *buf, size_t buf_len) {
	ANIMATION_HEADER ah;
	FRAME_HEADER fh;

	byte *write_p;

	uint16 magic;

	uint16 x_start;
	uint16 y_start;
	uint32 screen_w;
	uint32 screen_h;

	int mark_byte;
	byte data_byte;
	int new_row;

	uint16 control_ch;
	uint16 param_ch;

	uint16 runcount;
	int x_vector;

	uint16 i;

	if (!_initialized) {
		return FAILURE;
	}

	MemoryReadStreamEndian headerReadS(resdata, resdata_len, IS_BIG_ENDIAN);

	readAnimHeader(headerReadS, ah);

	screen_w = ah.screen_w;
	screen_h = ah.screen_h;

	if ((screen_w * screen_h) > buf_len) {
		// Buffer argument is too small to hold decoded frame, abort.
		warning("ITE_DecodeFrame: Buffer size inadequate");
		return FAILURE;
	}

	MemoryReadStream readS(resdata + frame_offset, resdata_len - frame_offset);

	// Check for frame magic byte
	magic = readS.readByte();
	if (magic != SAGA_FRAME_START) {
		warning("ITE_DecodeFrame: Invalid frame offset %x", frame_offset);
		return FAILURE;
	}


	fh.x_start = readS.readUint16BE();
	if (_vm->getFeatures() & GF_BIG_ENDIAN_DATA)
		fh.y_start = readS.readUint16BE();
	else
		fh.y_start = readS.readByte();
	readS.readByte();		/* Skip pad byte */
	fh.x_pos = readS.readUint16BE();
	fh.y_pos = readS.readUint16BE();
	fh.width = readS.readUint16BE();
	fh.height = readS.readUint16BE();

	x_start = fh.x_start;
	y_start = fh.y_start;

#if 1
#define VALIDATE_WRITE_POINTER \
	if ((write_p < buf) || (write_p >= (buf + screen_w * screen_h))) { \
		warning("VALIDATE_WRITE_POINTER: write_p=%x buf=%x", write_p, buf); \
		return FAILURE; \
	}
#else
#define VALIDATE_WRITE_POINTER 
#endif

	// Setup write pointer to the draw origin
	write_p = (buf + (y_start * screen_w) + x_start);
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
			write_p = buf + ((y_start + new_row) * screen_w) + x_start + x_vector;
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
			return SUCCESS;
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
			warning("ITE_DecodeFrame: Invalid RLE marker encountered");
			return FAILURE;
			break;
		}
	} while (mark_byte != 63); // end of frame marker

	return SUCCESS;
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

int Anim::getFrameOffset(const byte *resdata, size_t resdata_len, uint16 find_frame, size_t *frame_offset_p) {
	ANIMATION_HEADER ah;

	uint16 current_frame;

	byte mark_byte;
	uint16 control;
	uint16 runcount;
	uint16 magic;

	int i;

	MemoryReadStreamEndian readS(resdata, resdata_len, IS_BIG_ENDIAN); 

	readAnimHeader(readS, ah);

	if (find_frame > ah.maxframe) {
		return FAILURE;
	}

	readS._bigEndian = !IS_BIG_ENDIAN; // RLE has inversion BE<>LE

	for (current_frame = 0; current_frame < find_frame; current_frame++) {
		magic = readS.readByte();
		if (magic != SAGA_FRAME_START) {
			// Frame sync failure. Magic Number not found
			return FAILURE;
		}

		// skip header
		for (i = 0; i < SAGA_FRAME_HEADER_LEN; i++)
			readS.readByte();


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
				// Encountered unknown RLE marker, abort
				return FAILURE;
				break;
			}
		} while (mark_byte != SAGA_FRAME_END);
	}

	*frame_offset_p = readS.pos();
	return SUCCESS;
}

void Anim::animInfo() {
	uint16 anim_ct;
	uint16 i;
	uint16 idx;

	anim_ct = _anim_count;

	_vm->_console->DebugPrintf("There are %d animations loaded:\n", anim_ct);

	for (idx = 0, i = 0; i < anim_ct; idx++, i++) {
		while (_anim_tbl[idx] == NULL) {
			idx++;
		}

		_vm->_console->DebugPrintf("%02d: Frames: %u Flags: %u\n", i, _anim_tbl[idx]->maxframe, _anim_tbl[idx]->flags);
	}
}

} // End of namespace Saga
