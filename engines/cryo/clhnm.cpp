#include "common/debug.h"

#include "cryo/cryolib.h"

namespace Cryo {

static short safe_palette = 0;
static short pred_r = 0, pred_l = 0;
static short use_adpcm = 0;
static float hnm_rate = 0.0;
static float next_frame_time = 0.0;
static float expected_frame_time = 0.0;
static float time_drift = 0.0;
static short use_mono = 0;
static short use_sound = 0;
static short use_sound_sync = 0;
static short pending_sounds = 0;
static short sound_started = 0;
static short preserve_color0 = 0;
static soundchannel_t *soundChannel_adpcm = 0;
static soundgroup_t *soundGroup_adpcm = 0;
static soundchannel_t *soundChannel = 0;
static soundgroup_t *soundGroup = 0;
static void (*custom_chunk_handler)(unsigned char *buffer, int size, short id, char h6, char h7) = 0;
static short use_preload = 0;
static short decomp_table[256];

void CLHNM_Desentrelace320(unsigned char *frame_buffer, unsigned char *final_buffer, unsigned short height);

void CLHNM_DecompLempelZiv(unsigned char *buffer, unsigned char *output) {
	unsigned char *inp = buffer;
	unsigned char *out = output;

	unsigned int queue = 0;
	int qpos = -1;

	//TODO: fix for BE
#define GetBit() ( 1 & ( (qpos >= 0) ? (queue >> qpos--) : (queue = *(unsigned int*)((inp += 4) - 4)) >> ((qpos = 30) + 1) ) )

	for (;;) {
		if (GetBit()) {
			*out++ = *inp++;
		} else {
			int l, o;
			if (GetBit()) {
				l = *inp & 7;
				o = *(unsigned short *)inp >> 3;
				inp += 2;
				o -= 8192;
				if (!l)
					l = *inp++;
				if (!l)
					break;
			} else {
				l = GetBit() * 2 + GetBit();
				o = *(inp++) - 256;
			}
			l += 2;
			while (l--) {
				*out = *(out + o);
				out++;
			}
		}
	}

#undef GetBit

	return;
}

void CLHNM_DecompUBA(unsigned char *output, unsigned char *curr_buffer, unsigned char *prev_buffer,
                     unsigned char *input, int width, char flags) {
	unsigned int code;
	char mode, count, color;
	unsigned short offs;
	unsigned char *ref;
	unsigned char *out_start = output;
	unsigned char swap;
	int shft1, shft2;
//	return;
	if ((flags & 1) == 0) {
		//HNM4 classic
		int twolinesabove = -(width * 2);
		for (;;) {
			code = PLE32(input) & 0xFFFFFF; //input++;
			count = code & 0x1F;
			if (count) {
				input += 3;
				mode = (code >> 5) & 0xF;
				offs = code >> 9;
				//
				swap = mode >> 3;
				ref = ((mode & 1) ? prev_buffer : curr_buffer) + (output - out_start) + (offs * 2) - 32768;
				if (mode & 2) {
					//                  ref += twolinesabove;
					shft1 = twolinesabove + 1;
					shft2 = 0;
					//swap ^= 1;
				} else {
					shft1 = 0;
					shft2 = 1;
				}
				while (count--) {
					unsigned char b0 = ref[shft1];
					unsigned char b1 = ref[shft2];
					output[swap] = b0;
					output[swap ^ 1] = b1;
					output += 2;
					ref += (mode & 4) ? -2 : 2;
				}
			} else {
				input++;
				mode = code & 0xFF; // bits 0..4 are zero
				switch (mode) {
				case 0:
					*(output++) = *(input++);
					*(output++) = *(input++);
					break;
				case 0x20:
					output += 2 * *(input++);
					break;
				case 0x40:
					output += 2 * (code >> 8);
					input += 2;
					break;
				case 0x60:
					count = *(input++);
					color = *(input++);
					while (count--) {
						*(output++) = color;
						*(output++) = color;
					}
					break;
				default:
					return;
				}
			}
		}
	} else {
		assert(0);
		//HNM4 hires
		for (;;) {
			code = PLE32(input) & 0xFFFFFF;
			input++;
			count = code & 0x3F;
			if (count) {
				mode = (code >> 5) & 0xF;
				offs = code >> 9;
				//
			} else {
				mode = code & 0xFF; // bits 0..5 are zero
				switch (mode) {
				case 0x00:
					output += *input++;
					break;
				case 0x40:
					*output++ = *input++;
					*(output++ + width) = *input++;
					break;
				case 0x80:
					output += width;
					break;
				default:
					return;
				}
			}
		}
	}
}

void CLHNM_Init() {
	use_preload = 0;
	custom_chunk_handler = 0;
	preserve_color0 = 0;
	CLNoError;
}

void CLHNM_Done() {
	CLNoError;
}

void CLHNM_SetupTimer(float rate) {
	hnm_rate = 100.0 / rate;
	CLNoError;
}

void CLHNM_WaitLoop(hnm_t *hnm) {
	expected_frame_time += hnm_rate;
	next_frame_time = expected_frame_time - time_drift;
	if (use_sound_sync && TimerTicks > 1000.0 + next_frame_time)
		use_sound = 0;
	while (TimerTicks < next_frame_time) ;  // waste time
	time_drift = TimerTicks - next_frame_time;
}

void CLHNM_SetupSound(short numSounds, short arg4, short sampleSize, float rate, short mode) {
	soundChannel = CLSoundChannel_New(mode);
	soundGroup = CLSoundGroup_New(numSounds, arg4, sampleSize, rate, mode);
	if (sampleSize == 16)
		CLSoundGroup_Reverse16All(soundGroup);
}

void CLHNM_SetupSoundADPCM(short numSounds, short arg4, short sampleSize, float rate, short mode) {
	soundChannel_adpcm = CLSoundChannel_New(mode);
	soundGroup_adpcm = CLSoundGroup_New(numSounds, arg4, sampleSize, rate, mode);
}

void CLHNM_CloseSound() {
	if (soundChannel) {
		CLSoundChannel_Stop(soundChannel);
		CLSoundChannel_Free(soundChannel);
		soundChannel = 0;
	}
	if (soundGroup) {
		CLSoundGroup_Free(soundGroup);
		soundGroup = 0;
	}
	if (soundChannel_adpcm) {
		CLSoundChannel_Stop(soundChannel_adpcm);
		CLSoundChannel_Free(soundChannel_adpcm);
		soundChannel = 0;
	}
	if (soundGroup_adpcm) {
		CLSoundGroup_Free(soundGroup_adpcm);
		soundGroup = 0;
	}
}

void CLHNM_SetForceZero2Black(short forceblack) {
	preserve_color0 = forceblack;
}

hnm_t *CLHNM_New(int preload_size) {
	hnm_t *hnm;
	short i;

	preload_size = 0;   //TODO: let's ignore it for now

	CLBeginCheck;
	hnm = (hnm_t *)CLMemory_Alloc(sizeof(*hnm));
	CLCheckError();
	if (hnm) {
		if (preload_size) {
			use_preload = 1;

		}
		if (!__libError) {
			hnm->frame = 0;
			hnm->ff_4 = 0;
			hnm->file = 0;
			hnm->work_buffer[0] = 0;
			hnm->work_buffer[1] = 0;
			hnm->final_buffer = 0;
			hnm->read_buffer = 0;
			hnm->ff_896 = 0;
			hnm->total_read = 0;
			for (i = 0; i < 256; i++) {
				hnm->palette[i].a = 0;
				hnm->palette[i].r = 0;
				hnm->palette[i].g = 0;
				hnm->palette[i].b = 0;
			}
		} else
			CLCheckError();
	}
	CLEndCheck;
	return hnm;
}

void CLHNM_Dispose(hnm_t *hnm) {
	CLBeginCheck;

	if (use_preload) {

	}

	CLMemory_Free(hnm);
	CLCheckError();

	CLEndCheck;
}


void CLHNM_SetFile(hnm_t *hnm, file_t *file) {
	hnm->file = file;
	CLNoError;
}

void CLHNM_SetFinalBuffer(hnm_t *hnm, unsigned char *buffer) {
	hnm->final_buffer = buffer;
	CLNoError;
}

void CLHNM_AllocMemory(hnm_t *hnm) {
	CLBeginCheck;

	hnm->work_buffer[0] = (unsigned char *)CLMemory_Alloc(hnm->header.buffersize + 2);
	CLCheckError();

	if (!hnm->work_buffer[0])
		goto fin;

	hnm->work_buffer[1] = (unsigned char *)CLMemory_Alloc(hnm->header.buffersize + 2);
	CLCheckError();

	if (!hnm->work_buffer[1]) {
		CLMemory_Free(hnm->work_buffer[0]);
		CLCheckError();
		hnm->work_buffer[0] = 0;
		goto fin;
	}

	if (!use_preload) {
		hnm->read_buffer = (unsigned char *)CLMemory_Alloc(hnm->header.buffersize + 2);
//		CLCheckError();
		if (!hnm->read_buffer) {
			CLMemory_Free(hnm->work_buffer[0]);
			CLCheckError();
			hnm->work_buffer[0] = 0;
			CLMemory_Free(hnm->work_buffer[1]);
			CLCheckError();
			hnm->work_buffer[1] = 0;
		}
	}
fin:
	;
	CLEndCheck;
}

void CLHNM_DeallocMemory(hnm_t *hnm) {
	CLBeginCheck;
	if (hnm->work_buffer[0]) {
		CLMemory_Free(hnm->work_buffer[0]);
		CLCheckError();
		hnm->work_buffer[0] = 0;
	}
	if (hnm->work_buffer[1]) {
		CLMemory_Free(hnm->work_buffer[1]);
		CLCheckError();
		hnm->work_buffer[1] = 0;
	}

	if (!use_preload) {
		if (hnm->read_buffer) {
			CLMemory_Free(hnm->read_buffer);
			CLCheckError();
			hnm->read_buffer = 0;
		}
	}

	CLEndCheck;
}

void CLHNM_Read(hnm_t *hnm, int size) {
	long _size = size;
	if (!use_preload) {
		CLFile_Read(*hnm->file, hnm->read_buffer, &_size);
	} else {
	}
}

void CLHNM_GiveTime(hnm_t *hnm) {
	if (use_preload) {
		//stuff preload_buffer from disk
	}
}

void CLHNM_CanLoop(hnm_t *hnm, short can_loop) {
	hnm->can_loop = can_loop;
}

void CLHNM_SelectBuffers(hnm_t *hnm) {
	if (hnm->frame % 2) {
		hnm->new_frame_buffer = hnm->work_buffer[1];
		hnm->old_frame_buffer = hnm->work_buffer[0];
	} else {
		hnm->new_frame_buffer = hnm->work_buffer[0];
		hnm->old_frame_buffer = hnm->work_buffer[1];
	}
}

void CLHNM_ChangePalette(hnm_t *hnm) {
	short mincolor, maxcolor;
	unsigned short fst, cnt;
	unsigned char *pal;
	color_t *color;
	CLPalette_GetLastPalette(hnm->palette);
	pal = hnm->data_ptr;
	if (*(unsigned short *)pal == 0xFFFF)
		return;
	mincolor = 255;
	maxcolor = 0;
	do {
		fst = *pal++;
		cnt = *pal++;
		if (cnt == 0)
			cnt = 256;
		debug("hnm: setting palette, fst = %d, cnt = %d, last = %d", fst, cnt, fst + cnt - 1);
		assert(fst + cnt <= 256);
		if (mincolor > fst)
			mincolor = fst;
		if (maxcolor < fst + cnt)
			maxcolor = fst + cnt;
		color = hnm->palette + fst;
		if (safe_palette) {
			while (cnt--) {
				unsigned char r = *pal++;
				unsigned char g = *pal++;
				unsigned char b = *pal++;
				short rr = r << 10;
				short gg = g << 10;
				short bb = b << 10;
				if (color->r != rr || color->g != gg || color->b != bb)
					CLBlitter_OneBlackFlash();
				color->r = rr;
				color->g = gg;
				color->b = bb;
				color++;
			}
		} else {
			while (cnt--) {
				unsigned char r = *pal++;
				unsigned char g = *pal++;
				unsigned char b = *pal++;
				color->r = r << 10;
				color->g = g << 10;
				color->b = b << 10;
				color++;
			}
		}

	} while (*(unsigned short *)pal != 0xFFFF);
#if 0
	if (preserve_color0) {
		hnm->palette[0].r = 0;
		hnm->palette[0].g = 0;
		hnm->palette[0].b = 0;
	}
#endif
//	CLBlitter_Send2ScreenNextCopy(hnm->palette, mincolor, maxcolor - mincolor);
	CLBlitter_Send2ScreenNextCopy(hnm->palette, 0, 256);
}

void CLHNM_Desentrelace(hnm_t *hnm) {
	switch (hnm->header.width) {
	case 320:
		CLHNM_Desentrelace320(hnm->new_frame_buffer, hnm->final_buffer, hnm->header.height);
		CLNoError;
		break;
//	case 480:
//		CLHNM_Desentrelace480(hnm->new_frame_buffer, hnm->final_buffer, hnm->header.height);
//		CLNoError;
//		break;
	default:
		__libError = -5;
		__osError = 0;
		CLCheckError();
	}
}

void CLHNM_FlushPreloadBuffer(hnm_t *hnm) {
	if (use_preload) {
	}
}

soundchannel_t *CLHNM_GetSoundChannel() {
	return soundChannel;
}


void CLHNM_TryRead(hnm_t *hnm, int size) {
	short err;
	do {
		CLHNM_Read(hnm, size);
		err = __libError == -6;
		if (err)
			CLHNM_GiveTime(hnm);
	} while (err);
}

void CLHNM_ResetInternalTimer() {
	time_drift = 0.0;
	next_frame_time = expected_frame_time = TimerTicks;
}

void CLHNM_Reset(hnm_t *hnm) {
	hnm->frame = 0;
	hnm->ff_4 = 0;
	hnm->total_read = 0;
	sound_started = 0;
	pending_sounds = 0;
	CLHNM_ResetInternalTimer();
	CLNoError;
}

short CLHNM_LoadFrame(hnm_t *hnm) {
	int chunk;
	CLBeginCheck;
	CLHNM_TryRead(hnm, 4);
	CLEndCheck;
	chunk = *(int *)hnm->read_buffer;
	chunk = LE32(chunk);
	chunk &= 0xFFFFFF;  // upper bit - keyframe mark?
	if (!chunk)
		return 0;

	if (use_preload) {
	} else {
		if (chunk - 4 > hnm->header.buffersize) {
			__libError = -3;
			__osError = 0;
			CLCheckError();
		} else if (use_preload && chunk - 4 > 102400) {
			__libError = -3;
			__osError = 0;
			CLCheckError();
		}
	}
	CLBeginCheck;
	CLHNM_TryRead(hnm, chunk - 4);
	CLEndCheck;
	hnm->data_ptr = hnm->read_buffer;
	hnm->total_read += chunk;
	return 1;
}

void CLHNM_WantsSound(short sound) {
	use_sound = sound;
}

void CLHNM_LoadDecompTable(short *buffer) {
	short i;
	short e;
	for (i = 0; i < 256; i++) {
		e = *buffer++;
		decomp_table[i] = LE16(e);
	}
}

void CLHNM_DecompADPCM(unsigned char *buffer, short *output, int size) {
	short l = pred_l, r = pred_r;
	size &= ~1;
	while (size--) {
		*output++ = l += decomp_table[*buffer++];
		*output++ = r += decomp_table[*buffer++];
		if (l > 512 || r > 512)
			DebugStr(" coucou");
	}
	pred_l = l;
	pred_r = r;
}

void CLHNM_SoundInADPCM(short is_adpcm) {
	use_adpcm = is_adpcm;
}

void CLHNM_SoundMono(short is_mono) {
	use_mono = is_mono;
}

short CLHNM_NextElement(hnm_t *hnm) {
	int sz;
	short id;
	char h6, h7;
	short i;
	if (hnm->frame == 0) {
		CLHNM_ResetInternalTimer();
		pred_l = pred_r = 0;
	}
	if (hnm->frame == hnm->header.nframe)
		return 0;
	if (!CLHNM_LoadFrame(hnm))
		return 0;
	for (;;) {
		sz = PLE32(hnm->data_ptr) & 0xFFFFFF;
		hnm->data_ptr += 4;
		id = *(short *)hnm->data_ptr;
		hnm->data_ptr += 2;
		h6 = *hnm->data_ptr;
		hnm->data_ptr += 1;
		h7 = *hnm->data_ptr;
		hnm->data_ptr += 1;
		hnm->chunk_id = id;
		switch (id) {
		case BE16('PL'):
			CLHNM_ChangePalette(hnm);
			hnm->data_ptr += sz - 8;
			break;
		case BE16('IZ'):
			hnm->frame++;
			CLHNM_SelectBuffers(hnm);
			CLHNM_DecompLempelZiv(hnm->data_ptr + 4, hnm->new_frame_buffer);
			switch (hnm->header.width) {
//			case 320: CLBlitter_RawCopy320ASM(hnm->new_frame_buffer, hnm->old_frame_buffer, hnm->header.height); break;
//			case 480: CLBlitter_RawCopy480ASM(hnm->new_frame_buffer, hnm->old_frame_buffer, hnm->header.height); break;
//			case 640: CLBlitter_RawCopy640ASM(hnm->new_frame_buffer, hnm->old_frame_buffer, hnm->header.height); break;
//			default: memcpy(hnm->old_frame_buffer, hnm->new_frame_buffer, hnm->header.width * hnm->header.height);
			default:
				memcpy(hnm->old_frame_buffer, hnm->new_frame_buffer, hnm->header.buffersize);  //TODO strange buffer size here
			}
			if (!(h6 & 1))
				CLHNM_Desentrelace(hnm);
			else {
//				if(hnm->header.width == 640)
//					CLBlitter_RawCopy640(hnm->new_frame_buffer, hnm->final_buffer, hnm->header.height);
//				else
				memcpy(hnm->final_buffer, hnm->new_frame_buffer, hnm->header.height);   //TODO: wrong size?
			}
			if (use_adpcm) {
				if (!sound_started) {
					for (i = 0; i < pending_sounds; i++)
						CLSoundGroup_PlayNextSample(soundGroup_adpcm, soundChannel);
					sound_started = 1;
				}
			} else {
				if (!sound_started) {
					for (i = 0; i < pending_sounds; i++)
						CLSoundGroup_PlayNextSample(soundGroup, soundChannel);
					sound_started = 1;
				}
			}
			goto end_frame;
		case BE16('IU'):
			hnm->frame++;
			CLHNM_SelectBuffers(hnm);
			CLHNM_DecompUBA(hnm->new_frame_buffer, hnm->new_frame_buffer, hnm->old_frame_buffer, hnm->data_ptr, hnm->header.width, h6);
			if (!(h6 & 1))
				CLHNM_Desentrelace(hnm);
			else {
//				if(hnm->header.width == 640)
//					CLBlitter_RawCopy640(hnm->new_frame_buffer, hnm->final_buffer, hnm->header.height);
//				else
				memcpy(hnm->final_buffer, hnm->new_frame_buffer, hnm->header.width * hnm->header.height);
			}
			goto end_frame;
		case BE16('sd'):
		case BE16('SD'):
			if (use_sound) {
				if (!h6) {
					int sound_size = sz - 8;
					if (!use_adpcm) {
						CLSoundGroup_SetDatas(soundGroup, hnm->data_ptr, sound_size - 2, 0);
						if (sound_started)
							CLSoundGroup_PlayNextSample(soundGroup, soundChannel);
						else
							pending_sounds++;
					} else {
						short *sound_buffer = (short *)CLSoundGroup_GetNextBuffer(soundGroup_adpcm);
						if (!pending_sounds) {
							const int kDecompTableSize = 256 * sizeof(short);
							CLHNM_LoadDecompTable((short *)hnm->data_ptr);
							CLHNM_DecompADPCM(hnm->data_ptr + kDecompTableSize, sound_buffer, sound_size - kDecompTableSize);
							CLSoundGroup_AssignDatas(soundGroup_adpcm, sound_buffer, (sound_size - kDecompTableSize) * 2, 0);
						} else {
							CLHNM_DecompADPCM(hnm->data_ptr, sound_buffer, sound_size);
							CLSoundGroup_AssignDatas(soundGroup_adpcm, sound_buffer, sound_size * 2, 0);
						}
						pending_sounds++;
						if (sound_started)
							CLSoundGroup_PlayNextSample(soundGroup_adpcm, soundChannel);
					}
				} else {
					__libError = -3;
					__osError = 0;
					CLCheckError();
				}
			}
			hnm->data_ptr += sz - 8;
			break;
		default:
			if (custom_chunk_handler)
				custom_chunk_handler(hnm->data_ptr, sz - 8, id, h6, h7);
			hnm->data_ptr += sz - 8;
		}
	}
end_frame:
	;
	if (use_preload) {
	}
	return 1;
}

void CLHNM_ReadHeader(hnm_t *hnm) {
	CLBeginCheck;
	if (!use_preload) {
		long size = sizeof(hnm->header);
		CLFile_Read(*hnm->file, &hnm->header, &size);
	} else
		;
	CLCheckError();
	CLEndCheck;
	hnm->header.width = LE16(hnm->header.width);
	hnm->header.height = LE16(hnm->header.height);
	hnm->header.filesize = LE32(hnm->header.filesize);
	hnm->header.nframe = LE32(hnm->header.nframe);
	hnm->header.table_offset = LE32(hnm->header.table_offset);
	hnm->header.speed = LE16(hnm->header.speed);
	hnm->header.maxbuffer = LE16(hnm->header.maxbuffer);
	hnm->header.buffersize = LE32(hnm->header.buffersize);
	hnm->header.ff_20 = LE16(hnm->header.ff_20);
	hnm->header.buffersize += 4096; //TODO: checkme
}

short CLHNM_GetVersion(hnm_t *hnm) {
	CLNoError;
	if (hnm->header.id == BE32('HNM4'))
		return 4;
	return -1;
}

int CLHNM_GetFrameNum(hnm_t *hnm) {
	return hnm->frame;
}

void CLHNM_DeactivatePreloadBuffer() {
	use_preload = 0;
}

void CLHNM_Prepare2Read(hnm_t *hnm, int mode) {
	if (use_preload) {
	}
}

void CLHNM_SetPosIntoFile(hnm_t *hnm, long pos) {
	CLFile_SetPosition(*hnm->file, 1, pos);
}

void CLHNM_Desentrelace320(unsigned char *frame_buffer, unsigned char *final_buffer, unsigned short height) {
	unsigned int *input = (unsigned int *)frame_buffer;
	unsigned int *line0 = (unsigned int *)final_buffer;
	unsigned int *line1 = (unsigned int *)(final_buffer + 320);
	int count = (height) / 2;
	while (count--) {
		short i;
		for (i = 0; i < 320 / 4; i++) {
			unsigned int p0 = *input++;
			unsigned int p4 = *input++;
#if 0
			*line0++ = ((p4 & 0xFF00) >> 8) | ((p4 & 0xFF000000) >> 16) | ((p0 & 0xFF00) << 8) | (p0 & 0xFF000000);
//			*line0++ = (p0 & 0xFF000000) | ((p0 & 0xFF00) << 8) | ((p4 & 0xFF000000) >> 16) | ((p4 & 0xFF00) >> 8);
			*line1++ = ((p0 & 0xFF0000) << 8) | ((p0 & 0xFF) << 16) | ((p4 & 0xFF0000) >> 8) | (p4 & 0xFF);
#else
			*line0++ = (p0 & 0xFF) | ((p0 & 0xFF0000) >> 8) | ((p4 & 0xFF) << 16) | ((p4 & 0xFF0000) << 8);
			*line1++ = ((p0 & 0xFF00) >> 8) | ((p0 & 0xFF000000) >> 16) | ((p4 & 0xFF00) << 8) | (p4 & 0xFF000000);
#endif
		}
		line0 += 320 / 4;
		line1 += 320 / 4;
	}
}

} // End of namespace Cryo
