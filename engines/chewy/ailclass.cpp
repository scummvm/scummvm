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

#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_fseek
#define FORBIDDEN_SYMBOL_EXCEPTION_fread
#define FORBIDDEN_SYMBOL_EXCEPTION_fgetc

#include "engines/chewy/chewy.h"
#include "engines/chewy/ailclass.h"
#include "engines/chewy/ngshext.h"

#define MAX_VOICES 8
#define NORMAL_PLAYBACK 0
#define PATTERN_PLAYBACK 1
#define SEQUENCE_PLAYBACK 2
#define FADE_OUT 1
#define FADE_IN 2

static int16 FreqTable[] = {
	856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
	428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
	214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113,
	107, 101, 95, 90, 85, 80, 76, 71, 67, 64, 60, 57,

	862, 814, 768, 725, 684, 646, 610, 575, 543, 513, 484, 457,
	431, 407, 384, 363, 342, 323, 305, 288, 272, 256, 242, 228,
	216, 203, 192, 181, 171, 161, 152, 144, 136, 128, 121, 114,
	94, 89, 83, 78, 73, 68, 64, 59, 55, 53, 48, 45,

	868, 820, 774, 730, 689, 651, 614, 580, 547, 516, 487, 460,
	434, 410, 387, 365, 345, 325, 307, 290, 274, 258, 244, 230,
	217, 205, 193, 183, 172, 163, 154, 145, 137, 129, 122, 115,
	94, 89, 83, 78, 73, 68, 64, 59, 55, 53, 48, 45,

	875, 826, 779, 736, 694, 655, 619, 584, 551, 520, 491, 463,
	437, 413, 390, 368, 347, 328, 309, 292, 276, 260, 245, 232,
	219, 206, 195, 184, 174, 164, 155, 146, 138, 130, 123, 116,
	94, 89, 83, 78, 73, 68, 64, 59, 55, 53, 48, 45,

	881, 832, 785, 741, 699, 660, 623, 588, 555, 524, 494, 467,
	441, 416, 392, 370, 350, 330, 312, 294, 278, 262, 247, 233,
	220, 208, 196, 185, 175, 165, 156, 147, 139, 131, 123, 117,
	94, 89, 83, 78, 73, 68, 64, 59, 55, 53, 48, 45,

	887, 838, 791, 746, 704, 665, 628, 592, 559, 528, 498, 470,
	444, 419, 395, 373, 352, 332, 314, 296, 280, 264, 249, 235,
	222, 209, 198, 187, 176, 166, 157, 148, 140, 132, 125, 118,
	94, 89, 83, 78, 73, 68, 64, 59, 55, 53, 48, 45,

	894, 844, 796, 752, 709, 670, 632, 597, 563, 532, 502, 474,
	447, 422, 398, 376, 355, 335, 316, 298, 282, 266, 251, 237,
	223, 211, 199, 188, 177, 167, 158, 149, 141, 133, 125, 118,
	94, 89, 83, 78, 73, 68, 64, 59, 55, 53, 48, 45,

	900, 850, 802, 757, 715, 675, 636, 601, 567, 535, 505, 477,
	450, 425, 401, 379, 357, 337, 318, 300, 284, 268, 253, 238,
	225, 212, 200, 189, 179, 169, 159, 150, 142, 134, 126, 119,
	94, 89, 83, 78, 73, 68, 64, 59, 55, 53, 48, 45,

	907, 856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480,
	453, 428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240,
	226, 214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120,
	94, 89, 83, 78, 73, 68, 64, 59, 55, 53, 48, 45,

	814, 768, 725, 684, 646, 610, 575, 543, 513, 484, 457, 431,
	407, 384, 363, 342, 323, 305, 288, 272, 256, 242, 228, 216,
	204, 192, 181, 171, 161, 152, 144, 136, 128, 121, 114, 108,
	94, 89, 83, 78, 73, 68, 64, 59, 55, 53, 48, 45,

	820, 774, 730, 689, 651, 614, 580, 547, 516, 487, 460, 434,
	410, 387, 365, 345, 325, 307, 290, 274, 258, 244, 230, 217,
	205, 193, 183, 172, 163, 154, 145, 137, 129, 122, 115, 109,
	94, 89, 83, 78, 73, 68, 64, 59, 55, 53, 48, 45,

	826, 779, 736, 694, 655, 619, 584, 551, 520, 491, 463, 437,
	413, 390, 368, 347, 328, 309, 292, 276, 260, 245, 232, 219,
	206, 195, 184, 174, 164, 155, 146, 138, 130, 123, 116, 109,
	94, 89, 83, 78, 73, 68, 64, 59, 55, 53, 48, 45,

	832, 785, 741, 699, 660, 623, 588, 555, 524, 495, 467, 441,
	416, 392, 370, 350, 330, 312, 294, 278, 262, 247, 233, 220,
	208, 196, 185, 175, 165, 156, 147, 139, 131, 124, 117, 110,
	94, 89, 83, 78, 73, 68, 64, 59, 55, 53, 48, 45,

	838, 791, 746, 704, 665, 628, 592, 559, 528, 498, 470, 444,
	419, 395, 373, 352, 332, 314, 296, 280, 264, 249, 235, 222,
	209, 198, 187, 176, 166, 157, 148, 140, 132, 125, 118, 111,
	94, 89, 83, 78, 73, 68, 64, 59, 55, 53, 48, 45,

	844, 796, 752, 709, 670, 632, 597, 563, 532, 502, 474, 447,
	422, 398, 376, 355, 335, 316, 298, 282, 266, 251, 237, 224,
	211, 199, 188, 177, 167, 158, 149, 141, 133, 125, 118, 112,
	94, 89, 83, 78, 73, 68, 64, 59, 55, 53, 48, 45,

	850, 802, 757, 715, 674, 637, 601, 567, 535, 505, 477, 450,
	425, 401, 379, 357, 337, 318, 300, 284, 268, 253, 239, 225,
	213, 201, 189, 179, 169, 159, 150, 142, 134, 126, 119, 113,
	100, 95, 89, 84, 79, 74, 70, 65, 61, 59, 54, 51,
};
static uint16 RealFreq[800] = {0};

void *dig; // FIXME: DIG_DRIVER
int smp[MAX_VOICES]; // FIXME: HSAMPLE
int TimerHandle; // FIXME: HTIMER
int16 TimerEnabled = false;
int16 RealVoices;

int16 SoundEnable = 0;
int16 SoundCard = 0;
int16 SoundMasterVol = 120;
int16 MusicMasterVol = 120;
int16 MusicSwitch = false;
int16 SoundSwitch = false;
int16 LoopEnable = OFF;
int16 PlaybackMode = NORMAL_PLAYBACK;
int16 StartPos = 0;
int16 EndPos = 0;
int16 MusicFade = OFF;
uint16 FadeStart = 0;
uint16 FadeCounter = 0;
uint16 FadeVol = 0;
uint16 GlobalFrequency = 22050;

int16 MusicStatus = OFF;
int16 InInterrupt = 0;
int16 PatternCount = 6;
int16 CurrentTempo = 6;
int16 FineSpeedV;
int16 FineSpeedN;
int16 FineSpeedS;
tmf_inst Instrument[4] = {{ 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0 }};
uint16 InsFreq [4] = {0};
char *Sample [4] = {0};

tmf_header *ActiveSong;
int16 SeqPointer;
int16 PatPointer;
int16 PatLine;
char *Patterns[128];
char *CurrentLine;
int16 StereoPos [8] = {63};

char *Dbuffer [8][2] = { {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} };
uint32 DbufferLen [8] = {0};
uint32 DbSampleLen [8] = {0};
FILE *DbufferHandles [8] = {0};

ailclass::ailclass() {
#if 0
	int16 i;
	char *enstr;
	char a[] = {116, 109, 121, 83, 120, 107, 124, 107, 103, 114, 0}; // "NGS-REVEAL"
	char
	b[] = {121, 117, 123, 116, 106, 133, 121, 127, 121, 122, 107, 115, 133, 78, 105, 79,
		133, 116, 107, 125, 133, 109, 107, 116, 107, 120, 103, 122, 111, 117, 116, 133,
		121, 117, 108, 122, 125, 103, 120, 107, 0 }; // "SOUND_SYSTEM_(C)_NEW_GENERATION_SOFTWARE"
	for (i = 0; a[i] != 0; i++)
		a[i] -= 38;
	for (i = 0; b[i] != 0; i++)
		b[i] -= 38;
	enstr = (char *)getenv(a);
	if (enstr) {
		printf(b);
		printf("\n");
		delay(800);
	}
#endif
	SoundEnable = false;
	MusicFade = false;
}

ailclass::~ailclass() {
	warning("STUB: ailclass::~ailclass()");

#if 0
	if (SoundCard != NONE)
		AIL_shutdown();
#endif
}

int16 ailclass::init(uint16 freq) {
	warning("STUB: ailclass::init()");

#if 0
	int16 port, irq, dma;
	int16 i;
	GlobalFrequency = freq;
	AIL_startup();
	AIL_set_preference(DIG_SERVICE_RATE, 300);
	AIL_set_preference(DIG_LATENCY, 30);
	AIL_set_preference(DIG_USE_STEREO, YES);
	AIL_set_preference(DIG_HARDWARE_SAMPLE_RATE, freq);
	AIL_set_preference(DIG_USE_16_BITS, NO);
	dig = AIL_install_DIG_INI();
	if (dig == NULL) {
		AIL_shutdown();
		SoundCard = 0;
	} else {
		SoundEnable = TRUE;
		for (i = 0; i < MAX_VOICES; i++) {
			smp[i] = AIL_allocate_sample_handle(dig);
			if (smp[i] == NULL) {
				break;
			}
		}
		RealVoices = i;
		SoundCard = SOUNDBLASTER;
	}
	return (SoundCard);
#endif
	return SOUNDBLASTER;
}

int16 ailclass::init(char *midi_drv_name) {
	return (SoundCard);
}

void ailclass::exit1() {
	warning("STUB: ailclass::exit1()");
	// AIL_shutdown();
	SoundCard = NONE;
}

void ailclass::set_music_mastervol(int16 vol) {
	vol <<= 1;
	if (vol > 120)
		vol = 120;
	MusicMasterVol = vol;
}

void ailclass::set_sound_mastervol(int16 vol) {
	vol <<= 1;
	if (vol > 120)
		vol = 120;
	SoundMasterVol = vol;
}

void ailclass::set_channelvol(uint8 channel, uint8 vol) {
	channel &= 3;
	channel += 4;

	warning("STUB: ailclass::set_channelvol()");
#if 0
	if (SoundEnable)
		AIL_set_sample_volume(smp[channel], vol);
#endif
}

void ailclass::set_stereo_pos(int16 channel, int16 pos) {
	channel &= 7;
	channel += 4;
	pos &= 127;

	warning("STUB: ailclass::set_stereo_pos()");
#if 0
	if (SoundEnable) {
		StereoPos[channel] = pos;
		AIL_set_sample_pan(smp[channel], (byte)StereoPos[channel]);
	}
#endif
}

void ailclass::disable_sound() {
	SoundEnable = false;
}

void ailclass::init_mix_mode() {
	warning("STUB: ailclass::init_mix_mode()");

	if (SoundEnable) {
		init_note_table(GlobalFrequency);
#if 0
		TimerHandle = AIL_register_timer(mod_irq);
		if (TimerHandle != -1) {
			AIL_set_timer_frequency(TimerHandle, 50);
			AIL_start_timer(TimerHandle);
			TimerEnabled = true;
		} else
#endif
			TimerEnabled = false;
		StereoPos[0] = 7;
		StereoPos[1] = 120;
		StereoPos[2] = 30;
		StereoPos[3] = 90;
	}
}

void ailclass::exit_mix_mode() {
	warning("STUB: ailclass::exit_mix_mode()");

#if 0
	if (SoundEnable) {
		if (TimerEnabled) {
			AIL_stop_timer(TimerHandle);
			AIL_release_timer_handle(TimerHandle);
		}
	}
#endif
}

void ailclass::play_mod(tmf_header *th) {
	char *tmp;
	int16 i;
	ActiveSong = th;
	if (SoundEnable) {
		tmp = (char *)ActiveSong;
		tmp += sizeof(tmf_header);
		for (i = 0; i < 128; i++) {
			Patterns[i] = tmp;
			tmp += 1024;
		}
		SeqPointer = 0;
		PatLine = 0;
		PatPointer = ActiveSong->sequenz[SeqPointer];
		CurrentLine = Patterns[PatPointer];
		CurrentTempo = 6;
		PatternCount = 0;
		FineSpeedV = 1;
		FineSpeedN = 0;
		FineSpeedS = 0;

		MusicStatus = ON;
	}
}

void ailclass::stop_mod() {
	warning("STUB: ailclass::stop_mod()");

#if 0
	int16 i;
	if (SoundEnable) {
		for (i = 0; i < 4; i++) {
			AIL_end_sample(smp[i]);
			Instrument[i].replen = 0;
		}
		MusicStatus = OFF;
	}
#endif
}

void ailclass::continue_music() {
	if (SoundEnable)
		if (!strncmp(ActiveSong->id, "TMF", 3))
			MusicStatus = ON;
}

void ailclass::play_sequence(int16 startpos, int16 endpos) {
	if (SoundEnable) {
		if (!strncmp(ActiveSong->id, "TMF", 3)) {
			StartPos = startpos;
			EndPos = endpos;
			SeqPointer = StartPos;
			PatLine = 0;
			PatPointer = ActiveSong->sequenz[SeqPointer];
			CurrentLine = Patterns[PatPointer];
			MusicStatus = ON;
		}
	}
}

void ailclass::play_pattern(int16 pattnr) {
	if (SoundEnable) {
		if (!strncmp(ActiveSong->id, "TMF", 3)) {
			StartPos = 0;
			EndPos = 0;
			SeqPointer = 0;
			PatLine = 0;
			PatPointer = pattnr;
			CurrentLine = Patterns[PatPointer];
			MusicStatus = ON;
		}
	}
}

void ailclass::set_loopmode(int16 mode) {
	mode &= 1;
	if (SoundEnable)
		LoopEnable = mode;
}

void ailclass::fade_in(uint16 delay) {
	FadeVol = MusicMasterVol;
	MusicMasterVol = 0;
	FadeStart = delay;
	FadeCounter = 0;
	MusicFade = FADE_IN;
}

void ailclass::fade_out(uint16 delay) {
	FadeStart = delay;
	FadeCounter = 0;
	MusicFade = FADE_OUT;
	FadeVol = MusicMasterVol;
}

void ailclass::get_musik_info(musik_info *mi) {
	mi->musik_playing = MusicStatus;
	mi->play_mode = 0;
	mi->pattern_line = PatLine;
	mi->sequence_pos = SeqPointer;
	mi->cur_pattnr = PatPointer;
}

void ailclass::get_channel_info(channel_info *ch, int16 kanal) {
	warning("STUB: ailclass::get_channel_info()");

#if 0
	ch->finetune = Instrument[kanal].finetune;
	ch->volume = Instrument[kanal].insvol;
	ch->repstart = Instrument[kanal].repstart;
	ch->replen = Instrument[kanal].replen;
	ch->len = Instrument[kanal].laenge;
	ch->pointer = (dword)Sample[kanal];
	ch->pos = AIL_sample_position(smp[kanal]);
#endif
}

int16 ailclass::music_playing() {
	return (MusicStatus);
}

int16 ailclass::get_sample_status(int16 kanal) {
	warning("STUB: ailclass::get_sample_status()");

#if 0
	int16 ret;
	kanal &= 3;
	kanal += 4;
	ret = AIL_sample_status(smp[kanal]);
	return (ret);
#endif
	return 0;
}

void ailclass::init_note_table(uint16 sfreq) {
	float TimerFreq;
	TimerFreq = 7093789.2f;
	for (int j = 0; j < 16; j++) {
		for (int i = 0; i < 48; i++)
			RealFreq[(j * 48) + i] = (uint16)(TimerFreq / (float)(FreqTable[(j * 48) + i] << 1));

	}
}

void ailclass::play_voc(char *anf_adr, int16 kanal, int16 vol, int16 rep) {
	warning("STUB: ailclass::play_voc()");

#if 0
	byte *vptr = 0;
	byte blockt;
	byte freq = 0;
	dword blocklen;
	uint16 RealFrq;
	if (SoundEnable) {
		vptr = (byte *)anf_adr;
		vol = (vol << 1) & 127;
		kanal &= 3;
		kanal += 4;
		while (*vptr != 0) {
			blockt = *vptr++;
			if (blockt > 7)
				blockt = 8;
			blocklen = (uint32) * vptr++;
			blocklen += ((uint32) * vptr++) << 8;
			blocklen += ((uint32) * vptr++) << 16;
			if (blockt == 1) {
				freq = vptr[0];
				RealFrq = 1000000 / (256 - freq);
				AIL_init_sample(smp[kanal]);
				AIL_set_sample_type(smp[kanal], DIG_F_MONO_8, 0);
				AIL_set_sample_address(smp[kanal], vptr + 2, blocklen);
				AIL_set_sample_playback_rate(smp[kanal], RealFrq);
				AIL_set_sample_volume(smp[kanal], vol);
				AIL_set_sample_loop_count(smp[kanal], rep);
				AIL_set_sample_pan(smp[kanal], (byte)StereoPos[kanal]);
				AIL_start_sample(smp[kanal]);
			}
			vptr += blocklen;
		}
	}
#endif
}

void ailclass::play_raw(int16 kanal, char *sp, uint32 len,
                        uint16 freqwert, int16 volume, int16 rep) {
	warning("STUB: ailclass::play_raw()");

#if 0
	kanal &= 3;
	kanal += 4;
	if (SoundEnable) {
		AIL_init_sample(smp[kanal]);
		AIL_set_sample_type(smp[kanal], DIG_F_MONO_8, 0);
		AIL_set_sample_address(smp[kanal], sp, len);
		AIL_set_sample_playback_rate(smp[kanal], freqwert);
		AIL_set_sample_volume(smp[kanal], volume);
		AIL_set_sample_loop_count(smp[kanal], rep);
		AIL_set_sample_pan(smp[kanal], (byte)StereoPos[kanal]);
		AIL_start_sample(smp[kanal]);
	}
#endif
}

void ailclass::stop_sound() {
	warning("STUB: ailclass::stop_sound()");

#if 0
	int16 i;
	for (i = 4; i < 8; i++)
		AIL_stop_sample(smp[i]);
#endif
}

void ailclass::continue_sound() {
	warning("STUB: ailclass::continue_sound()");

#if 0
	int16 i;
	for (i = 4; i < 8; i++)
		AIL_resume_sample(smp[i]);
#endif
}

void ailclass::end_sound() {
	warning("STUB: ailclass::end_sound()");

#if 0
	int16 i;
	for (i = 4; i < 8; i++)
		AIL_end_sample(smp[i]);
#endif
}

void ailclass::stop_sample(int16 kanal) {
	warning("STUB: ailclass::stop_sample()");

#if 0
	kanal &= 3;
	kanal += 4;
	AIL_stop_sample(smp[kanal]);
#endif
}

void ailclass::continue_sample(int16 kanal) {
	warning("STUB: ailclass::continue_sample()");

#if 0
	kanal &= 3;
	kanal += 4;
	AIL_resume_sample(smp[kanal]);
#endif
}

void ailclass::end_sample(int16 kanal) {
	warning("STUB: ailclass::end_sample()");

#if 0
	kanal &= 3;
	kanal += 4;
	AIL_end_sample(smp[kanal]);
#endif
}

void ailclass::init_double_buffer(char *b1, char *b2, uint32 len, int16 kanal) {
	kanal &= 3;
	kanal += 4;
	Dbuffer [kanal][0] = b1;
	Dbuffer [kanal][1] = b2;
	DbufferLen [kanal] = len;
}

void ailclass::start_db_voc(void *v, int16 kanal, int16 vol) {
	warning("STUB: ailclass::start_db_voc()");

#if 0
	byte *vptr = 0;
	byte blockt;
	byte freq = 0;
	dword blocklen;
	uint16 RealFrq;
	kanal &= 3;
	kanal += 4;
	vol = (vol << 1) & 127;
	if ((SoundEnable) && (DbufferLen[kanal] != 0) && (voc != 0) &&
	        (Dbuffer[kanal][0] != 0) && (Dbuffer[kanal][1] != 0)) {
		memset(Dbuffer[kanal][0], 0, DbufferLen[kanal]);
		memset(Dbuffer[kanal][1], 0, DbufferLen[kanal]);
		do {
			blockt = fgetc(voc);
			if (blockt > 7)
				blockt = 8;
			blocklen = (uint32) fgetc(voc);
			blocklen += ((uint32)fgetc(voc)) << 8;
			blocklen += ((uint32)fgetc(voc)) << 16;
			if (blockt != 1)
				fseek(voc, blocklen, SEEK_CUR);
		} while ((blockt != 1) && (blockt != 0));
		if ((blockt == 1) && (!modul)) {
			freq = fgetc(voc);
			RealFrq = 1000000 / (256 - freq);
			if (blocklen > DbufferLen[kanal]) {
				DbufferHandles[kanal] = voc;
				DbSampleLen[kanal] = blocklen;
				AIL_init_sample(smp[kanal]);
				AIL_set_sample_type(smp[kanal], DIG_F_MONO_8, 0);
				AIL_set_sample_playback_rate(smp[kanal], RealFrq);
				AIL_set_sample_volume(smp[kanal], vol);
				AIL_set_sample_pan(smp[kanal], (byte)StereoPos[kanal]);
				AIL_load_sample_buffer(smp[kanal], 0, Dbuffer[kanal][0],
				                       DbufferLen[kanal]);
			} else {
				DbSampleLen[kanal] = 0;
				if (!fread(Dbuffer[kanal][0], blocklen, 1, voc)) {
					modul = DATEI;
					fcode = READFEHLER;
				}
				if (!modul) {
					AIL_init_sample(smp[kanal]);
					AIL_set_sample_type(smp[kanal], DIG_F_MONO_8, 0);
					AIL_set_sample_address(smp[kanal], Dbuffer[kanal][0], blocklen);
					AIL_set_sample_playback_rate(smp[kanal], RealFrq);
					AIL_set_sample_volume(smp[kanal], vol);
					AIL_set_sample_pan(smp[kanal], (byte)StereoPos[kanal]);
					AIL_start_sample(smp[kanal]);
				}
			}
		}
	}
#endif
}

void ailclass::serve_db_samples() {
	warning("STUB: ailclass::serve_db_samples()");

#if 0
	int16 i;
	int16 BufNr;
	dword len;
	for (i = 4; (i < 8) && (!modul); i++) {
		if (DbSampleLen[i] != 0) {
			BufNr = AIL_sample_buffer_ready(smp[i]);
			if (BufNr != -1) {
				if (DbSampleLen[i] > DbufferLen[i]) {
					len = DbufferLen[i];
					DbSampleLen[i] -= DbufferLen[i];
				} else {
					len = DbSampleLen[i];
					DbSampleLen[i] = 0;
				}
				if (!fread(Dbuffer[i][BufNr], len, 1, DbufferHandles[i])) {
					modul = DATEI;
					fcode = READFEHLER;
				} else
					AIL_load_sample_buffer(smp[i], BufNr, Dbuffer[i][BufNr], len);
			}
		} else if (DbufferHandles[i]) {
			BufNr = AIL_sample_buffer_ready(smp[i]);
			if (BufNr != -1) {
				AIL_load_sample_buffer(smp[i], BufNr, Dbuffer[i][BufNr], 0);
				DbufferHandles[i] = 0;
			}
		}
	}
#endif
}

void mod_irq() {
	if (!InInterrupt) {
		++InInterrupt;
		if (MusicSwitch && (MusicStatus == ON)) {
			check_sample_end();
			if (PatternCount <= 0) {
				PatternCount = CurrentTempo;
				DecodePatternLine();
			}
			if (MusicFade == FADE_IN) {
				if (!FadeCounter) {
					FadeCounter = FadeStart;
					if ((MusicMasterVol + 2) <= FadeVol)
						MusicMasterVol += 2;
					else {
						MusicFade = OFF;
						MusicMasterVol = FadeVol;
					}
				} else
					--FadeCounter;
			} else if (MusicFade == FADE_OUT) {
				if (!FadeCounter) {
					FadeCounter = FadeStart;
					if ((MusicMasterVol - 2) > 0)
						MusicMasterVol -= 2;
					else {
						MusicFade = OFF;
						MusicStatus = OFF;
						MusicMasterVol = FadeVol;
					}
				} else
					--FadeCounter;
			}
		}
		PatternCount -= FineSpeedV;
		FineSpeedS += FineSpeedN;
		while (FineSpeedS > 1000) {
			FineSpeedS -= 1000;
			--PatternCount;
		}
	}
	--InInterrupt;
}

void check_sample_end() {
	warning("STUB: ailclass::check_sample_end()");

#if 0
	int16 i;
	for (i = 0; i < 4; i++) {
		if (AIL_sample_status(smp[i]) != SMP_PLAYING) {
			if (Instrument[i].replen >= 10) {
				AIL_init_sample(smp[i]);
				AIL_set_sample_type(smp[i], DIG_F_MONO_8, 0);
				AIL_set_sample_address(smp[i], Sample[i] +
				                       Instrument[i].repstart,
				                       Instrument[i].replen);
				AIL_set_sample_playback_rate(smp[i], InsFreq[i]);
				if ((byte)Instrument[i].insvol > (byte)(MusicMasterVol))
					Instrument[i].insvol = (byte)MusicMasterVol;
				AIL_set_sample_volume(smp[i], Instrument[i].insvol);
				AIL_set_sample_pan(smp[i], StereoPos[i]);
				AIL_start_sample(smp[i]);
			}
		}
	}
#endif
}

void DecodePatternLine() {
	DecodeChannel(0);
	DecodeChannel(1);
	DecodeChannel(2);
	DecodeChannel(3);
	++PatLine;
	if (PatLine > 63) {
		PatLine = 0;
		++SeqPointer;
		switch (PlaybackMode) {
		case NORMAL_PLAYBACK:
			if (SeqPointer < ActiveSong->lied_len) {
				PatPointer = ActiveSong->sequenz[SeqPointer];
				CurrentLine = Patterns[PatPointer];
			} else {
				if (LoopEnable == OFF)
					MusicStatus = OFF;
				else {
					PatLine = 0;
					SeqPointer = 0;
					PatPointer = ActiveSong->sequenz[SeqPointer];
					CurrentLine = Patterns[PatPointer];
				}
			}
			break;

		case SEQUENCE_PLAYBACK:
			if (SeqPointer != EndPos) {
				PatPointer = ActiveSong->sequenz[SeqPointer];
				CurrentLine = Patterns[PatPointer];
			} else {
				if (LoopEnable == OFF)
					MusicStatus = OFF;
				else {
					PatLine = 0;
					SeqPointer = StartPos;
					PatPointer = ActiveSong->sequenz[SeqPointer];
					CurrentLine = Patterns[PatPointer];
				}
			}
			break;

		case PATTERN_PLAYBACK:
			if (LoopEnable == OFF)
				MusicStatus = OFF;
			else {
				PatLine = 0;
				SeqPointer = 0;
				PatPointer = StartPos;
				CurrentLine = Patterns[PatPointer];
			}
			break;
		}
	}
}

void DecodeChannel(int16 ch) {
	warning("STUB: ailclass::decode_channel()");

#if 0
	byte note, effect, inst, effpar;
	tmf_inst *ti;
	int16 tmp1;
	dword tmp2;
	ch &= 3;
	ti = &Instrument[ch];
	note = *CurrentLine;
	++CurrentLine;
	inst = *CurrentLine;
	++CurrentLine;
	effect = *CurrentLine;
	++CurrentLine;
	effpar = *CurrentLine;
	++CurrentLine;
	if (note <= 48) {
		switch (effect) {
		case 0x9:
			break;

		case 0xa:
			if (effpar & 0xf0) {
				effpar >>= 4;
				ti->insvol += effpar;
				if ((byte)ti->insvol > (byte)MusicMasterVol)
					ti->insvol = (byte)MusicMasterVol;
			} else {
				effpar &= 0x0f;
				if (ti->insvol > effpar)
					ti->insvol -= effpar;
				else
					ti->insvol = 0;
			}
			AIL_set_sample_volume(smp[ch], ti->insvol);
			break;

		case 0xb:
			PatLine = 63;
			SeqPointer = effpar;
			break;

		case 0xc:
			ti->insvol = effpar;
			if (ti->insvol > (byte)MusicMasterVol)
				ti->insvol = (byte)MusicMasterVol;
			AIL_set_sample_volume(smp[ch], ti->insvol);
			break;

		case 0xd:
			PatLine = 63;
			break;

		case 0xe:
			break;

		case 0xf:
			if (effpar >= 0x20) {
				FineSpeedV = effpar / 125;
				FineSpeedN = ((effpar % 125) * 1000) / 125;
			} else {
				CurrentTempo = (int16) effpar;
				FineSpeedV = 1;
				FineSpeedN = 0;
				FineSpeedS = 0;
			}
			break;
		}
		if (note < 48) {
			InsFreq[ch] = RealFreq[note];
			if (inst > 0) {
				--inst;
				memcpy(ti, &ActiveSong->instrument[inst], sizeof(tmf_inst));
				Sample[ch] = ActiveSong->ipos[inst];
			}

			if (ti->insvol > (byte)MusicMasterVol)
				ti->insvol = (byte)MusicMasterVol;
			AIL_end_sample(smp[ch]);
			while (AIL_sample_status(smp[ch]) == SMP_PLAYING);
			AIL_init_sample(smp[ch]);
			AIL_set_sample_type(smp[ch], DIG_F_MONO_8, 0);
			AIL_set_sample_address(smp[ch], Sample[ch], ti->laenge);
			AIL_set_sample_playback_rate(smp[ch], InsFreq[ch]);
			AIL_set_sample_volume(smp[ch], ti->insvol);
			AIL_set_sample_pan(smp[ch], StereoPos[ch]);
			AIL_start_sample(smp[ch]);
		}
	}
#endif
}
