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

#include "common/system.h"
#include "common/memstream.h"
#include "chewy/chewy.h"
#include "chewy/events.h"
#include "chewy/file.h"
#include "chewy/flic.h"

namespace Chewy {

void decode_flc(byte *vscr, const byte *dbuf) {
	Common::MemoryReadStream src(dbuf, 0xffffffff);
	int h = src.readUint16LE();
	int val, pair;
	int8 b;

	for (; h > 0; --h, vscr += SCREEN_WIDTH) {
		// Get value, and handle any line skips if needed
		for (val = src.readUint16LE(); val & 0x4000; val = src.readUint16LE()) {
			vscr += (-val * SCREEN_WIDTH) & 0xffff;
		}

		// Iteration for pixels within the line
		byte *dest = vscr;
		for (; val > 0; --val) {
			dest += src.readByte();
			b = src.readSByte();

			if (b < 0) {
				b = -b;
				pair = src.readUint16LE();
				for (; b > 0; --b, dest += 2)
					WRITE_LE_UINT16(dest, pair);

			} else {
				for (; b > 0; --b, dest += 2)
					WRITE_LE_UINT16(dest, src.readUint16LE());
			}
		}
	}
}

void decode_rle(byte *vscr, const byte *dbuf, int br, int h) {
	int x;

	for (; h > 0; --h, vscr += SCREEN_WIDTH) {
		byte *dest = vscr;
		++dbuf;		// Skip number of entries in line

		for (x = 0; x < br; ) {
			int8 len = (int8)*dbuf++;
			if (len < 0) {
				// Copy a number of bytes specified in lower 7 bits
				len = -len;
				if (len) {
					Common::copy(dbuf, dbuf + len, dest);
					dbuf += len;
					dest += len;
					x += len;
				}
			} else {
				// Run length in the lower 7 bits of the next byte
				byte v = *dbuf++;
				if (len) {
					Common::fill(dest, dest + len, v);
					x += len;
					dest += len;
				}
			}
		}

		assert(x == br);
	}
}


flic::flic() {
	Common::fill(&sounds[0], &sounds[50], (byte *)nullptr);
}

flic::~flic() {
}

void flic::play(const char *fname, byte *vscreen, byte *load_p) {
	Stream *lhandle;

	lhandle = File::open(fname);
	if (lhandle) {
		play(lhandle, vscreen, load_p);
		delete lhandle;
	} else {
		modul = DATEI;
		fcode = OPENFEHLER;
	}
}

void flic::play(Common::Stream *handle, byte *vscreen, byte *load_p) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(handle);
	uint16 i;
	size_t tmp_size;
	char key;
	float start, ende;
	bool trace_mode;

	load_puffer = load_p;
	virt_screen = vscreen + 4;
	if (flic_header.load(rs)) {
		if (flic_header.type == FLC) {

			key = 0;
			trace_mode = false;
			fade_flag = false;
			fade_delay = 0;
			cls_flag = false;
			CurrentFrame = 0;
			for (i = 0; (i < flic_header.frames) && (!modul) && (key != 27); i++) {
				if (!frame_header.load(rs)) {
					modul = DATEI;
					fcode = READFEHLER;
				} else {
					if (frame_header.type != PREFIX) {
						tmp_size = ((size_t)frame_header.size) - sizeof(FrameHead);
						start = (float) g_system->getMillis(); // clock()
						start /= 0.05f;
						start += flic_header.speed;
						if (tmp_size) {
							if (rs->read(load_puffer, tmp_size) != tmp_size) {
								modul = DATEI;
								fcode = READFEHLER;
							} else {
								decode_frame();
							}
						}

						do {
							ende = (float)g_system->getMillis(); // clock()
							ende /= 0.05f;
						} while (ende <= start);
						++CurrentFrame;
					} else {
						rs->seek(frame_header.size - (int)FrameHead::SIZE(), SEEK_CUR);
					}
				}
			}
		}
	}
}

void flic::decode_frame() {
	uint16 i;
	byte *tmp_buf;
	bool update_flag;
	ChunkHead chunk_header;

	tmp_buf = load_puffer;
	update_flag = false;
	if (frame_header.chunks != 0) {
		fade_flag = false;
		for (i = 0; i < frame_header.chunks; i++) {
			Common::MemoryReadStream rs(tmp_buf, ChunkHead::SIZE());
			chunk_header.load(&rs);

			tmp_buf += ChunkHead::SIZE();
			chunk_header.size -= ChunkHead::SIZE();

			switch (chunk_header.type) {
			case COLOR_256:
				col256_chunk(tmp_buf);
				break;

			case COLOR_64:
				col64_chunk(tmp_buf);
				break;

			case BYTE_RUN:
				decode_rle(virt_screen, tmp_buf,
				           (int)flic_header.width,
				           (int)flic_header.height);
				update_flag = true;
				break;

			case DELTA_FLC:
				decode_flc(virt_screen, tmp_buf);
				update_flag = true;
				break;

			case DELTA_FLI:
				delta_chunk_byte(tmp_buf);

				update_flag = true;
				break;

			case CLS:
				out->setze_zeiger(virt_screen);
				out->cls();
				out->setze_zeiger(0);

				update_flag = true;
				break;

			case UNPRESSED:
				out->back2back(load_puffer, virt_screen);

				update_flag = true;
				break;

			case PSTAMP:
				break;

			default:
				update_flag = true;
				break;
			}

			tmp_buf += chunk_header.size;
		}
		if (update_flag != false) {
			if (flic_user) {
				out->setze_zeiger(virt_screen);
				flic_user(CurrentFrame);
				out->setze_zeiger(0);
			}
			out->back2screen(virt_screen - 4);
			if (fade_flag != false) {
				out->einblenden(fade_pal, fade_delay);
				fade_flag = false;
			}
		}
	}
}

void flic::col256_chunk(byte *tmp) {
	short int i;
	short int packets;
	short int count;
	char anz, col;
	char r, g, b;
	packets = *(short int *)tmp;
	tmp += 2;
	out->vsync_start();
	if (cls_flag == true)
		out->cls();
	else
		cls_flag = true;
	if (tmp[1] == 0) {
		tmp += 2;
		for (i = 0; i < 768; i++)
			tmp[i] >>= 2;
		if (fade_flag == false)
			out->set_palette(tmp);
		else {
			memset(fade_pal, 0, 768);
			out->set_palette(fade_pal);
			memcpy(fade_pal, tmp, 768);
		}
	} else {
		col = 0;
		for (count = 0; count < packets; count++) {
			col += *tmp++;
			anz = *tmp++;
			for (i = 0; i < anz; i++) {
				r = *tmp++ >> 2;
				g = *tmp++ >> 2;
				b = *tmp++ >> 2;
				out->raster_col(col, r, g, b);
				++col;
			}
		}
	}
}

void flic::col64_chunk(byte *tmp) {
	short int i;
	short int packets;
	short int count;
	char anz, col;
	char r, g, b;
	packets = *((short int *)tmp);
	tmp += 2;
	out->vsync_start();
	if (cls_flag == true)
		out->cls();
	else
		cls_flag = true;
	if (!tmp[1]) {
		if (fade_flag == false)
			out->set_palette(tmp + 2);
		else {
			memset(fade_pal, 0, 768);
			out->set_palette(fade_pal);
			memcpy(fade_pal, tmp + 2, 768);
		}
	} else {
		col = 0;
		for (count = 0; count < packets; count++) {
			col += *tmp++;
			anz = *tmp++;
			for (i = 0; i < anz; i++) {
				r = *tmp++ >> 2;
				g = *tmp++ >> 2;
				b = *tmp++ >> 2;
				out->raster_col(col, r, g, b);
				++col;
			}
		}
	}
}

void flic::delta_chunk_byte(byte *tmp) {
	short int *ipo;
	byte *abl;
	byte *tabl;
	short int i, j;
	short int rest_height;
	short signed int count;
	signed char tmp_count;
	short int data;
	short int mode_word;
	short int pcount;
	byte skip, last_byte = 0;
	bool last_flag;
	abl = virt_screen;
	ipo = (short int *)tmp;
	rest_height = *ipo++;
	tmp += 2;
	for (i = 0; (i < rest_height) && (i < 200); i++) {
		tabl = abl;
		mode_word = *ipo++;
		if (mode_word & 0x4000) {
			mode_word = -mode_word;
			abl += (int16)(mode_word * flic_header.width);
			tabl = abl;
			mode_word = *ipo++;
		}
		if (mode_word & 0x8000) {
			last_byte = (byte)(mode_word & 0xff);
			last_flag = true;
			mode_word = *ipo++;
		} else
			last_flag = false;
		tmp = (byte *)ipo;
		if (mode_word) {
			pcount = 0;
			for (j = 0; (pcount < mode_word) && (j <= flic_header.width); ++pcount) {
				skip = *tmp++;
				abl += skip;
				tmp_count = (signed char) * tmp++;
				count = (short signed int)tmp_count;
				if (count > 0) {
					count <<= 1;
					while ((count) && (j < flic_header.width)) {
						*abl++ = *tmp++;
						++j;
						--count;
					}
				} else {
					count = -count;
					data = *(short int *)tmp;
					tmp += 2;
					while ((count > 0) && (j < flic_header.width)) {
						*((short int *)abl) = data;
						abl += 2;
						j += 2;
						--count;
					}
				}
			}
			if (last_flag)
				*abl++ = last_byte;
		}
		abl = tabl + flic_header.width;
		ipo = (short int *)tmp;
	}
}

void flic::custom_play(CustomInfo *ci) {
	uint16 i;
	char key;
	uint32 start, ende;
	bool trace_mode;

	Cinfo = ci;
	load_puffer = ci->TempArea;
	virt_screen = ci->VirtScreen + 4;
	Music = ci->MusicSlot;
	Sound = ci->SoundSlot;

	if (ci->Fname != 0) {
		ci->Handle = File::open(ci->Fname);
	}

	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(ci->Handle);
	if (rs) {
		if (custom_header.load(rs)) {
			if (!scumm_strnicmp(custom_header.id, "CFO\0", 4)) {
				key = 0;
				trace_mode = false;

				cls_flag = false;
				fade_flag = false;
				fade_delay = 0;
				CurrentFrame = 0;

				for (i = 0; (i < custom_header.frames) && (!modul) && (key != 27); i++) {
					if (!custom_frame.load(rs)) {
						modul = DATEI;
						fcode = READFEHLER;
					} else {
						if ((custom_frame.type != PREFIX) && (custom_frame.type != CUSTOM)) {
							start = g_system->getMillis() + custom_header.speed;
							if (custom_frame.size) {
								if (rs->read(load_puffer, custom_frame.size) != custom_frame.size) {
									modul = DATEI;
									fcode = READFEHLER;
								} else
									decode_cframe();
							}

							// Show the next frame
							g_screen->update();

							// Loop until the frame time expires
							do {
								ende = g_system->getMillis();
								g_events->update();
							} while (ende <= start);
							++CurrentFrame;

						} else if (custom_frame.type == CUSTOM) {
							decode_custom_frame(
								dynamic_cast<Common::SeekableReadStream *>(ci->Handle));

						} else {
							out->raster_col(255, 63, 63, 63);
							out->printxy(0, 0, 255, 0, 0, "Unknown Frame Type");
							taste;
						}
					}
				}
			}
		}
		if (ci->Fname != 0)
			chewy_fclose(ci->Handle);
	} else {
		modul = DATEI;
		fcode = OPENFEHLER;
	}
}

void flic::decode_custom_frame(Common::SeekableReadStream *handle) {
	uint16 para[10];
	ChunkHead chead;
	uint16 i, j;
	tmf_header *th;
	byte *tmp;
	musik_info mi;
	th = (tmf_header *)Music;

	for (i = 0; (i < custom_frame.chunks) && (!modul); i++) {
		if (!chead.load(handle)) {
			modul = DATEI;
			fcode = READFEHLER;
		}

		switch (chead.type) {
		case FADE_IN:
			if (!File::readArray(handle, &para[0], chead.size / 2)) {
				modul = DATEI;
				fcode = READFEHLER;
			} else {
				fade_flag = true;
				fade_delay = para[0];
			}

			break;

		case FADE_OUT:
			if (!File::readArray(handle, &para[0], chead.size / 2)) {
				modul = DATEI;
				fcode = READFEHLER;
			} else
				out->ausblenden(para[0]);
			break;

		case LOAD_MUSIC:
			if (handle->read(Music, chead.size) != chead.size) {
				modul = DATEI;
				fcode = READFEHLER;
			} else {
				tmp = Music;
				tmp += sizeof(tmf_header);
				tmp += ((uint32)th->pattern_anz) * 1024l;
				for (j = 0; j < 31; j++) {
					if (th->instrument[j].laenge) {
						th->ipos[j] = tmp;
						tmp += th->instrument[j].laenge;
					}
				}
			}

			break;

		case LOAD_RAW:
			if (!File::readArray(handle, &para[0], 1) ||
			        handle->read(Sound, chead.size - 2) != (chead.size - 2)) {
				modul = DATEI;
				fcode = READFEHLER;
			} else {
				sounds[para[0]] = Sound;
				Ssize[para[0]] = chead.size;
				Sound += chead.size - 2;
			}
			break;

		case LOAD_VOC:
			if (!File::readArray(handle, &para[0], 1) ||
				handle->read(Sound, chead.size - 2) != (chead.size - 2)) {
				modul = DATEI;
				fcode = READFEHLER;
			} else {
				sounds[para[0]] = Sound;
				Ssize[para[0]] = chead.size - 2;
				Sound += chead.size;
			}
			break;

		case PLAY_MUSIC:
			if (!strncmp(th->id, "TMF\0", 4))
#ifndef AIL
				snd->play_mod(th);
#else
				ailsnd->play_mod(th);
#endif

			break;

		case PLAY_SEQ:
			if (!File::readArray(handle, &para[0], chead.size / 2)) {
				modul = DATEI;
				fcode = READFEHLER;
			} else {
				if (!strncmp(th->id, "TMF\0", 4)) {
#ifndef AIL
					snd->play_mod(th);
					snd->stop_mod();
					snd->play_sequence(para[0], para[1]);
#else
					ailsnd->play_mod(th);
					ailsnd->stop_mod();
					ailsnd->play_sequence(para[0], para[1]);
#endif
				}
			}
			break;

		case PLAY_PATTERN:
			if (!File::readArray(handle, &para[0], chead.size / 2)) {
				modul = DATEI;
				fcode = READFEHLER;
			} else {
				if (!strncmp(th->id, "TMF\0", 4)) {
#ifndef AIL
					snd->play_mod(th);
					snd->stop_mod();
					snd->play_pattern(para[0]);
#else
					ailsnd->play_mod(th);
					ailsnd->stop_mod();
					ailsnd->play_pattern(para[0]);
#endif
				}
			}
			break;

		case STOP_MUSIC:
#ifndef AIL
			snd->stop_mod();
#else
			ailsnd->stop_mod();
#endif
			break;

		case WAIT_MSTOP:
			do {
#ifndef AIL
				snd->get_musik_info(&mi);
#else
				ailsnd->get_musik_info(&mi);
#endif
			} while (mi.musik_playing != 0);
			break;

		case SET_MVOL:
			if (!File::readArray(handle, &para[0], chead.size / 2)) {
				modul = DATEI;
				fcode = READFEHLER;
			} else
#ifndef AIL
				snd->set_music_mastervol(para[0]);
#else
				ailsnd->set_music_mastervol(para[0]);
#endif

			break;

		case SET_LOOPMODE:
			if (!File::readArray(handle, &para[0], chead.size / 2)) {
				modul = DATEI;
				fcode = READFEHLER;
			} else
#ifndef AIL
				snd->set_loopmode(para[0]);
#else
				ailsnd->set_loopmode(para[0]);
#endif
			break;

		case PLAY_RAW:
			break;

		case PLAY_VOC:
			if (!File::readArray(handle, &para[0], chead.size / 2)) {
				modul = DATEI;
				fcode = READFEHLER;
			} else
#ifndef AIL
				snd->play_voc(sounds[para[0]], para[1], para[2], para[3]);
#else
				if (para[3] == 255)
					para[3] = 0;
				else
					para[3] += 1;
			ailsnd->play_voc(sounds[para[0]], para[1], para[2], para[3]);
#endif

			break;

		case SET_SVOL:
			if (!File::readArray(handle, &para[0], chead.size / 2)) {
				modul = DATEI;
				fcode = READFEHLER;
			} else
#ifndef AIL
				snd->set_sound_mastervol(para[0]);
#else
				ailsnd->set_sound_mastervol(para[0]);
#endif
			break;

		case SET_CVOL:
			if (!File::readArray(handle, &para[0], chead.size / 2)) {
				modul = DATEI;
				fcode = READFEHLER;
			} else
#ifndef AIL
				snd->set_channelvol((byte)para[0], (byte)para[1]);
#else
				ailsnd->set_channelvol((byte)para[0], (byte)para[1]);
#endif
			break;

		case FREE_EFFECT:
			if (!File::readArray(handle, &para[0], chead.size / 2)) {
				modul = DATEI;
				fcode = READFEHLER;
			} else
				free_sound(para[0]);
			break;

		case MFADE_IN:
			if (!File::readArray(handle, &para[0], chead.size / 2)) {
				modul = DATEI;
				fcode = READFEHLER;
			} else
#ifndef AIL
				snd->fade_in(para[0]);
#else
				ailsnd->fade_in(para[0]);
#endif
			break;

		case MFADE_OUT:
			if (!File::readArray(handle, &para[0], chead.size / 2)) {
				modul = DATEI;
				fcode = READFEHLER;
			} else
#ifndef AIL
				snd->fade_out(para[0]);
#else
				ailsnd->fade_out(para[0]);
#endif
			break;

		case SET_STEREO:
			if (!File::readArray(handle, &para[0], chead.size / 2)) {
				modul = DATEI;
				fcode = READFEHLER;
			} else
#ifdef AIL
				ailsnd->set_stereo_pos(para[0], para[1]);
#endif
			break;

		case SET_SPEED:
			custom_header.speed = handle->readUint32LE();
			break;

		case CLEAR_SCREEN:
			out->setze_zeiger(virt_screen);
			out->cls();
			out->setze_zeiger(0);
			out->cls();
			break;

		default:
			out->printxy(0, 10, 255, 0, 0, "Unknown Chunk %d ", chead.type);
			break;
		}
	}
}

void flic::decode_cframe() {
	ChunkHead chunk_header;
	uint16 i;
	byte *tmp_buf;
	int16 update_flag;
	tmp_buf = load_puffer;
	update_flag = false;

	if (custom_frame.chunks != 0) {
		for (i = 0; i < custom_frame.chunks; i++) {
			Common::MemoryReadStream rs(tmp_buf, ChunkHead::SIZE());
			chunk_header.load(&rs);

			tmp_buf += ChunkHead::SIZE();
			chunk_header.size -= ChunkHead::SIZE();

			switch (chunk_header.type) {
			case COLOR_256:
				col256_chunk(tmp_buf);
				break;

			case COLOR_64:
				break;

			case BYTE_RUN:
				decode_rle(virt_screen, tmp_buf,
				    custom_header.width, custom_header.height);
				update_flag = true;
				break;

			case DELTA_FLC:
				decode_flc(virt_screen, tmp_buf);
				update_flag = true;

				break;
			case DELTA_FLI:
				break;

			case CLS:
				out->setze_zeiger(virt_screen);
				out->cls();
				out->setze_zeiger(0);

				update_flag = true;
				break;

			case UNPRESSED:
				out->back2back(load_puffer, virt_screen);

				update_flag = true;
				break;

			case PSTAMP:
				break;

			default:
				out->raster_col(255, 63, 63, 63);
				out->printxy(0, 0, 255, 0, 0, "Unknown CHUNK");

				update_flag = true;
				break;
			}

			tmp_buf += chunk_header.size;
		}

		if (update_flag != false) {
			if (custom_user) {
				out->back2back(virt_screen, load_puffer);
				out->setze_zeiger(virt_screen);
				custom_user(CurrentFrame);
				out->setze_zeiger(0);
				out->back2screen(virt_screen - 4);
				out->back2back(load_puffer, virt_screen);
			} else
				out->back2screen(virt_screen - 4);
			if (fade_flag != false) {
				out->einblenden(fade_pal, fade_delay);
				fade_flag = false;
			}
		}
	}
}

void flic::free_sound(int16 nr) {
	byte *fsound;
	long fsize;
	long copysize;
	int16 i;
	fsound = sounds[nr];
	fsize = Ssize[nr];
	if ((fsound != 0) && (fsize != 0)) {
		copysize = Cinfo->MaxSoundSize;
		copysize -= (long)(fsound - Cinfo->SoundSlot);
		memmove(fsound, fsound + fsize, copysize);
		for (i = 0; i < 50; i++) {
			if (sounds[i] == fsound) {
				sounds[i] = 0;
				Ssize[i] = 0;
			} else if (sounds[i] > fsound)
				sounds[i] -= fsize;
		}
		Sound -= fsize;
	}
}

void flic::set_custom_user_function(int16(*user_funktion)(int16 frame)) {
	custom_user = user_funktion;
}

void flic::remove_custom_user_function() {
	custom_user = 0;
}

void flic::set_flic_user_function(int16(*user_funktion)(int16 frame)) {
	flic_user = user_funktion;
}

void flic::remove_flic_user_function() {
	flic_user = 0;
}

} // namespace Chewy
