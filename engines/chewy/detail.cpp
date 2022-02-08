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
 */

#include "chewy/detail.h"
#include "chewy/file.h"
#include "chewy/global.h"
#include "chewy/sound.h"
#include "chewy/resource.h"

namespace Chewy {

bool RaumInfo::load(Common::SeekableReadStream *src) {
	RoomNr = src->readByte();
	BildNr = src->readByte();
	AutoMovAnz = src->readByte();
	TafLoad = src->readByte();
	src->read(TafName, 14);
	ZoomFak = src->readByte();
	Dummy = src->readByte();

	return true;
}

bool RaumAutoMov::load(Common::SeekableReadStream *src) {
	X = src->readSint16LE();
	Y = src->readSint16LE();
	SprNr = src->readByte();
	dummy = src->readByte();

	return true;
}

bool sound_def_blk::load(Common::SeekableReadStream *src) {
	int i;

	for (i = 0; i < MAX_SOUNDS; ++i)
		sound_enable[i] = src->readSint16LE();
	for (i = 0; i < MAX_SOUNDS; ++i)
		sound_index[i] = src->readSint16LE();
	for (i = 0; i < MAX_SOUNDS; ++i)
		sound_start[i] = src->readSint16LE();
	for (i = 0; i < MAX_SOUNDS; ++i)
		kanal[i] = src->readSint16LE();
	for (i = 0; i < MAX_SOUNDS; ++i)
		volume[i] = src->readSint16LE();
	for (i = 0; i < MAX_SOUNDS; ++i)
		repeats[i] = src->readSint16LE();
	for (i = 0; i < MAX_SOUNDS; ++i)
		stereo[i] = src->readSint16LE();

	return true;
}

bool ani_detail_info::load(Common::SeekableReadStream *src) {
	x = src->readSint16LE();
	y = src->readSint16LE();
	start_flag = src->readByte();
	repeat = src->readByte();
	start_ani = src->readSint16LE();
	end_ani = src->readSint16LE();
	ani_count = src->readSint16LE();
	delay = src->readSint16LE();
	delay_count = src->readSint16LE();
	reverse = src->readSint16LE();
	timer_start = src->readSint16LE();
	z_ebene = src->readSint16LE();
	load_flag = src->readByte();
	zoom = src->readByte();
	sfx.load(src);
	show_1_phase = src->readSint16LE();
	phase_nr = src->readSint16LE();

	return true;
}

bool static_detail_info::load(Common::SeekableReadStream *src) {
	x = src->readSint16LE();
	y = src->readSint16LE();
	SprNr = src->readSint16LE();
	z_ebene = src->readSint16LE();
	Hide = src->readByte();
	Dummy = src->readByte();

	return true;
}

bool room_detail_info::load(Common::SeekableReadStream *src) {
	int i;

	StaticDetailAnz = src->readSint16LE();
	AniDetailAnz = src->readSint16LE();
	src->skip(4);	// dptr
	for (i = 0; i < MAXDETAILS; ++i)
		Ainfo[i].load(src);
	for (i = 0; i < MAXDETAILS; ++i)
		Sinfo[i].load(src);
	for (i = 0; i < MAX_M_ITEMS * 4; ++i)
		mvect[i] = src->readSint16LE();
	for (i = 0; i < MAX_M_ITEMS; ++i)
		mtxt[i] = src->readSint16LE();
	Ri.load(src);
	for (i = 0; i < MAX_AUTO_MOV; ++i)
		AutoMov[i].load(src);
	for (i = 0; i < MAXDETAILS * MAX_SOUNDS; ++i)
		tvp_index[i] = src->readSint16LE();
	src->skip(4 * MAXDETAILS * MAX_SOUNDS); // sample

	return true;
}

bool RdiDateiHeader::load(Common::SeekableReadStream *src) {
	src->read(Id, 4);
	Anz = src->readSint16LE();

	return true;
}


detail::detail() {
	int16 size = sizeof(room_detail_info);
	char *tptr = (char *)&rdi;

	for (int16 i = 0; i < size; i++)
		*tptr++ = 0;

	for (int16 i = 0; i < MAXDETAILS; i++) {
		rdi.Sinfo[i].SprNr = -1;
		rdi.Ainfo[i].start_ani = -1;
	}

	for (int16 i = 0; i < (MAX_M_ITEMS - 1) << 2; i += 4)
		rdi.mvect[i] = -1;

	global_delay = 0;
	ani_freezeflag = false;
	fulltaf = false;
	rdi.dptr = nullptr;
	tafname[0] = 0;
	for (int16 i = 0; i < (MAXDETAILS * MAX_SOUNDS); i++) {
		rdi.sample[i] = nullptr;
		rdi.tvp_index[i] = -1;
	}
	direct_taf_ani = OFF;
}

detail::~detail() {
	direct_taf_ani = OFF;
}

void detail::load_rdi(const char *fname_, int16 room_nr) {
	Common::File f;
	taf_info *tmprdi = rdi.dptr;

	if (fname_ && f.open(fname_)) {
		if (rdi_datei_header.load(&f)) {
			if (!scumm_strnicmp(rdi_datei_header.Id, "RDI", 3)) {
				f.seek(room_nr * room_detail_info::SIZE(), SEEK_CUR);

				if (!rdi.load(&f)) {
					error("load_rdi error");
				}
			} else {
				error("load_rdi error");
			}
		} else {
			error("load_rdi error");
		}

		f.close();
	} else {
		error("load_rdi error");
	}

	rdi.dptr = tmprdi;
}

void detail::load_rdi_taf(const char *fname_, int16 load_flag) {
	if (strcmp(tafname, fname_)) {
		if (rdi.dptr) {
			if (fulltaf) {
				free(rdi.dptr);
				rdi.dptr = nullptr;
				fulltaf = false;
			} else {
				del_taf_tbl(rdi.dptr);
			}
		}
		strcpy(tafname, fname_);
		if (!load_flag) {
			rdi.dptr = init_taf_tbl(fname_);
			if (!modul)
				load_taf_tbl(rdi.dptr);
		} else {
			rdi.dptr = mem->taf_adr(fname_);
			fulltaf = true;
		}
	} else {
		if (!fulltaf)
			load_taf_tbl(rdi.dptr);
	}
}

void detail::del_dptr() {
	if (rdi.dptr) {
		if (fulltaf)
			free((char *)rdi.dptr);
		else {
			del_taf_tbl(rdi.dptr);
			rdi.dptr = nullptr;
			tafname[0] = 0;
		}
		rdi.dptr = nullptr;
	}
}

void detail::load_taf_tbl(taf_info *fti) {
	if (!fti) {
		fti = rdi.dptr;
	}

	if (fti) {
		for (int16 i = 0; i < MAXDETAILS; i++) {
			if (rdi.Sinfo[i].SprNr != -1)
				load_taf_seq(rdi.Sinfo[i].SprNr, 1, fti);
			if (rdi.Ainfo[i].start_ani != -1 &&
					rdi.Ainfo[i].end_ani != -1 && !rdi.Ainfo[i].load_flag)
				load_taf_seq(rdi.Ainfo[i].start_ani, (rdi.Ainfo[i].end_ani - rdi.Ainfo[i].start_ani) + 1, fti);
		}
	}
}

taf_info *detail::init_taf_tbl(const char *fname_) {
	taf_info *Tt = 0;
	SpriteResource *res = new SpriteResource(tafname);
	int16 anz = res->getChunkCount();
	byte *tmp = (byte *)MALLOC((int32)anz * sizeof(byte *) + sizeof(taf_info));

	Tt = (taf_info *)tmp;
	Tt->anzahl = anz;
	Tt->korrektur = (int16 *)MALLOC((int32)Tt->anzahl * 2 * sizeof(int16));
	Tt->image = (byte **)(tmp + sizeof(taf_info));
	memcpy(Tt->korrektur, (byte *)res->getSpriteCorrectionsTable(), Tt->anzahl * 2 * sizeof(int16));
	Tt->palette = 0;

	delete res;

	return Tt;
}

void detail::del_taf_tbl(taf_info *Tt) {
	if (!Tt) {
		Tt = rdi.dptr;
	}
	for (int16 i = 0; i < Tt->anzahl; i++) {
		free(Tt->image[i]);
	}
	free((char *) Tt->korrektur);
	free((char *) Tt);
}

void detail::del_taf_tbl(int16 start, int16 anz, taf_info *Tt) {
	if (!Tt)
		Tt = rdi.dptr;
	for (int16 i = start; i < start + anz && i < Tt->anzahl; i++) {
		free(Tt->image[i]);
		Tt->image[i] = 0;
	}
}

void detail::load_taf_seq(int16 spr_nr, int16 spr_anz, taf_info *Tt) {
	if (!Tt)
		Tt = rdi.dptr;

	SpriteResource *res = new SpriteResource(tafname);

	for (int16 i = 0; i < spr_anz; i++) {
		if (!Tt->image[spr_nr + i]) {
			res->getSpriteData(spr_nr + i, &Tt->image[spr_nr + i], true);
		}
	}

	delete res;
}

void detail::set_static_spr(int16 nr, int16 spr_nr) {
	rdi.Sinfo[nr].SprNr = spr_nr;
}

void detail::hide_static_spr(int16 nr) {
	if (nr >= 0 && nr < MAXDETAILS)
		rdi.Sinfo[nr].Hide = true;
}

void detail::show_static_spr(int16 nr) {
	if (nr >= 0 && nr < MAXDETAILS)
		rdi.Sinfo[nr].Hide = false;
}

byte *detail::get_static_image(int16 det_nr) {
	byte *ret;
	int16 index = rdi.Sinfo[det_nr].SprNr;
	if (index == -1)
		ret = nullptr;
	else
		ret = rdi.dptr->image[index];
	return ret;
}

void detail::set_static_pos(int16 det_nr, int16 x, int16 y, bool hide, bool korr_flag) {
	if (korr_flag) {
		int16 *Cxy = rdi.dptr->korrektur + (rdi.Sinfo[det_nr].SprNr << 1);
		x += Cxy[0];
		y += Cxy[1];
	}
	rdi.Sinfo[det_nr].x = x;
	rdi.Sinfo[det_nr].y = y;
	rdi.Sinfo[det_nr].Hide = hide;
}

void detail::set_detail_pos(int16 det_nr, int16 x, int16 y) {
	rdi.Ainfo[det_nr].x = x;
	rdi.Ainfo[det_nr].y = y;
}

void detail::get_ani_werte(int16 ani_nr, int16 *start, int16 *end) {
	*start = rdi.Ainfo[ani_nr].start_ani;
	*end = rdi.Ainfo[ani_nr].end_ani;
}

void detail::set_ani(int16 ani_nr, int16 start, int16 end) {
	if (start > end) {
		int16 tmp = start;
		start = end;
		end = tmp;
	}
	rdi.Ainfo[ani_nr].start_ani = start;
	rdi.Ainfo[ani_nr].end_ani = end;
}

byte *detail::get_image(int16 spr_nr) {
	byte *ret = rdi.dptr->image[spr_nr];
	return ret;
}

ani_detail_info *detail::get_ani_detail(int16 ani_nr) {
	ani_detail_info *ret = &rdi.Ainfo[ani_nr];
	return ret;
}

int16 *detail::get_korrektur_tbl() {
	int16 *ret;
	ret = rdi.dptr->korrektur;
	return ret;
}

void detail::init_taf(taf_info *dptr) {
	rdi.dptr = dptr;
}

taf_info *detail::get_taf_info() {
	taf_info *ret;
	ret = rdi.dptr;
	return ret;
}

room_detail_info *detail::get_room_detail_info() {
	return &rdi;
}

void detail::freeze_ani() {
	ani_freezeflag = true;
}

void detail::unfreeze_ani() {
	ani_freezeflag = false;
}

void detail::set_static_ani(int16 ani_nr, int16 static_nr) {
	rdi.Ainfo[ani_nr].show_1_phase = true;
	if (static_nr != -1)
		rdi.Ainfo[ani_nr].phase_nr = static_nr;
}

void detail::del_static_ani(int16 ani_nr) {
	rdi.Ainfo[ani_nr].show_1_phase = false;
}

void detail::plot_ani_details(int16 scrx, int16 scry, int16 start, int16 end, int16 zoomx, int16 zoomy) {
	if (start > end)
		SWAP(start, end);

	if (start > MAXDETAILS)
		start = 0;

	if (end > MAXDETAILS)
		end = MAXDETAILS - 1;

	for (int16 i = start; (i <= end) && (!modul); i++) {
		ani_detail_info *adiptr = &rdi.Ainfo[i];
		if ((adiptr->start_flag) && (adiptr->start_ani != -1) && (adiptr->end_ani != -1)) {
			int16 sprnr = adiptr->ani_count;
			int16 *Cxy = rdi.dptr->korrektur + (sprnr << 1);
			int16 kx = Cxy[0];
			int16 ky = Cxy[1];
			if (zoomx != 0 || zoomy != 0)
				calc_zoom_kor(&kx, &ky, zoomx, zoomy);
			int16 x = adiptr->x + kx - scrx;
			int16 y = adiptr->y + ky - scry;
			if (adiptr->load_flag == 1) {
				load_taf_ani_sprite(sprnr);
				if (!modul)
					out->scale_set(taf_load_buffer, x, y, zoomx, zoomy, 0);
			} else
				out->scale_set(rdi.dptr->image[sprnr], x, y, zoomx, zoomy, 0);

			Sound *sound = g_engine->_sound;

			for (int16 k = 0; k < MAX_SOUNDS; k++) {
				int16 soundEffect = adiptr->sfx.sound_index[k];
				if ((adiptr->sfx.sound_enable[k]) && (soundEffect != -1) &&
				        (rdi.sample[soundEffect])) {
					if ((adiptr->sfx.sound_start[k] == adiptr->ani_count) &&
					        (!adiptr->delay_count)) {
						const uint channel = adiptr->sfx.kanal[k] & 7;
						sound->setSoundChannelBalance(channel, adiptr->sfx.stereo[k]);
						sound->setSoundChannelVolume(channel, adiptr->sfx.volume[k]);
						sound->playSound(soundEffect, channel,	adiptr->sfx.repeats[k]);
					}
				}
			}

			if (!ani_freezeflag) {
				if (adiptr->reverse) {
					if (adiptr->delay_count > 0)
						--adiptr->delay_count;
					else {
						adiptr->delay_count = adiptr->delay + global_delay;
						if (adiptr->ani_count > adiptr->start_ani)
							--adiptr->ani_count;
						else {
							adiptr->ani_count = adiptr->end_ani;
							if ((adiptr->start_flag != 255) && (adiptr->start_flag > 0)) {
								--adiptr->start_flag;

							}
						}
					}
				} else {
					if (adiptr->delay_count > 0)
						--adiptr->delay_count;
					else {
						adiptr->delay_count = adiptr->delay + global_delay;
						if (adiptr->ani_count < adiptr->end_ani)
							++adiptr->ani_count;
						else {
							adiptr->ani_count = adiptr->start_ani;
							if ((adiptr->start_flag != 255) && (adiptr->start_flag > 0)) {
								--adiptr->start_flag;
							}
						}
					}
				}
			}
		} else {
			adiptr->start_flag = 0;
			if (adiptr->show_1_phase) {
				rdi.Sinfo[adiptr->phase_nr].Hide = false;
				plot_static_details(scrx, scry, adiptr->phase_nr, adiptr->phase_nr);
				rdi.Sinfo[adiptr->phase_nr].Hide = true;
			}
		}
	}
}

void detail::plot_static_details(int16 scrx, int16 scry, int16 start, int16 end) {
	if (start > end)
		SWAP(start, end);

	if (start > MAXDETAILS)
		start = 0;
	
	if (end > MAXDETAILS)
		end = MAXDETAILS - 1;

	for (int16 i = start; i <= end; i++) {
		if (rdi.Sinfo[i].SprNr != -1 && !rdi.Sinfo[i].Hide) {
			int16 x = rdi.Sinfo[i].x - scrx;
			int16 y = rdi.Sinfo[i].y - scry;
			byte *simage = rdi.dptr->image[rdi.Sinfo[i].SprNr];
			out->sprite_set(simage, x, y, 0);
		}
	}
}

void detail::start_detail(int16 nr, int16 rep, int16 reverse) {
	if (nr != -1) {
		ani_detail_info *adiptr;
		adiptr = &rdi.Ainfo[nr];
		if (rep)
			adiptr->start_flag = rep;
		else
			adiptr->start_flag = adiptr->repeat;

		adiptr->reverse = reverse;
		if (reverse)
			adiptr->ani_count = adiptr->end_ani;
		else
			adiptr->ani_count = adiptr->start_ani;
	}
}

void detail::stop_detail(int16 nr) {
	if (nr >= 0 && nr < 32) {
		ani_detail_info *adiptr = &rdi.Ainfo[nr];
		adiptr->start_flag = 0;
	}
}

void detail::set_ani_delay(int16 nr, int16 del) {
	ani_detail_info *adiptr = &rdi.Ainfo[nr];
	adiptr->delay = del;
}

void detail::init_list(int16 *mv) {
	for (int16 i = 0; i < ((MAX_M_ITEMS - 1) << 2); i++)
		rdi.mvect[i] = mv[i];
}

void detail::get_list(int16 *mv) {
	for (int16 i = 0; i < ((MAX_M_ITEMS - 1) << 2); i++)
		mv[i] = rdi.mvect[i];
}

int16 detail::maus_vector(int16 x, int16 y) {
	int16 i, j;
	i = -1;
	for (j = 0; (j < (MAX_M_ITEMS - 1) << 2) && (i == -1); j += 4) {
		if (rdi.mvect[j] != -1) {
			if ((x >= rdi.mvect[j]) && (x <= rdi.mvect[j + 2]) &&
			        (y >= rdi.mvect[j + 1]) && (y <= rdi.mvect[j + 3]))
				i = j / 4;
		}
	}

	return i;
}

int16 detail::get_ani_status(int16 det_nr) {
	int16 ret;
	if (rdi.Ainfo[det_nr].start_flag > 0)
		ret = 1;
	else
		ret = 0;
	return ret;
}

SprInfo detail::plot_detail_sprite(int16 scrx, int16 scry, int16 det_nr, int16 spr_nr, int16 mode) {
	if (det_nr > MAXDETAILS)
		det_nr = MAXDETAILS - 1;
	ani_detail_info *adiptr = &rdi.Ainfo[det_nr];
	if (spr_nr < adiptr->start_ani)
		spr_nr = adiptr->start_ani;
	if (spr_nr > adiptr->end_ani)
		spr_nr = adiptr->end_ani - 1;
	int16 *Cxy = rdi.dptr->korrektur + (spr_nr << 1);
	int16 *Xy = (int16 *)rdi.dptr->image[spr_nr];
	spr_info.Image = rdi.dptr->image[spr_nr];
	spr_info.X = adiptr->x + Cxy[0] - scrx;
	spr_info.Y = adiptr->y + Cxy[1] - scry;
	spr_info.X1 = spr_info.X + Xy[0];
	spr_info.Y1 = spr_info.Y + Xy[1];
	if (mode)
		out->sprite_set(spr_info.Image, spr_info.X, spr_info.Y, 0);

	Sound *sound = g_engine->_sound;

	for (int16 k = 0; k < MAX_SOUNDS; k++) {
		int16 soundEffect = adiptr->sfx.sound_index[k];
		if ((adiptr->sfx.sound_enable[k]) && (soundEffect != -1) &&
		        (rdi.sample[soundEffect])) {
			if (adiptr->sfx.sound_start[k] == spr_nr) {
				const uint channel = adiptr->sfx.kanal[k] & 7;
				sound->setSoundChannelBalance(channel, adiptr->sfx.stereo[k]);
				sound->setSoundChannelVolume(channel, adiptr->sfx.volume[k]);
				sound->playSound(soundEffect, channel, adiptr->sfx.repeats[k]);
			}
		}
	}

	return spr_info;
}

void detail::set_global_delay(int16 delay) {
	global_delay = delay;
	global_delay -= 2;
}

void detail::calc_zoom_kor(int16 *kx, int16 *ky, int16 xzoom, int16 yzoom) {
	float tmpx;
	float tmpy;
	float tmpx1;
	float tmpy1;
	tmpx = (float)(((float) * kx / 100.0) * ((float)xzoom));
	tmpy = (float)(((float) * ky / 100.0) * ((float)yzoom));

	tmpx1 = tmpx - (int16)tmpx;
	if (fabs(tmpx1) > 0.5)
		++tmpx;
	tmpy1 = tmpy - (int16)tmpy;
	if (fabs(tmpy1) > 0.5)
		++tmpy;
	*kx += (int16)tmpx;
	*ky += (int16)tmpy;
}

void detail::disable_detail_sound(int16 nr) {
	sound_def_blk *sdb = &rdi.Ainfo[nr].sfx;
	for (int16 i = 0; i < MAX_SOUNDS; i++)
		sdb->sound_enable[i] = false;
}

void detail::enable_detail_sound(int16 nr) {
	sound_def_blk *sdb = &rdi.Ainfo[nr].sfx;
	for (int16 i = 0; i < MAX_SOUNDS; i++)
		if (sdb->sound_index[i] != -1)
			sdb->sound_enable[i] = true;
}

void detail::play_detail_sound(int16 nr) {
	sound_def_blk *sdb = &rdi.Ainfo[nr].sfx;
	Sound *sound = g_engine->_sound;
	for (int16 k = 0; k < MAX_SOUNDS; k++) {
		if ((sdb->sound_enable[k]) && (sdb->sound_index[k] != -1) &&
		        (rdi.sample[sdb->sound_index[k]])) {
			const uint channel = sdb->kanal[k] & 7;
			sound->setSoundChannelBalance(channel, sdb->stereo[k]);
			sound->setSoundChannelVolume(channel, sdb->volume[k]);
			sound->playSound(sdb->sound_index[k], channel, sdb->repeats[k]);
		}
	}
}

void detail::clear_detail_sound(int16 nr) {
	sound_def_blk *sdb = &rdi.Ainfo[nr].sfx;
	for (short i = 0; i < MAX_SOUNDS; i++) {
		sdb->sound_enable[i] = false;
		sdb->sound_index[i] = -1;
		sdb->sound_start[i] = 0;
		sdb->kanal[i] = 0;
		sdb->volume[i] = 0;
		sdb->repeats[i] = 0;
		sdb->stereo[i] = 0;
	}
	remove_unused_samples();
}

void detail::disable_room_sound() {
	for (int16 i = 0; i < MAXDETAILS; i++)
		disable_detail_sound(i);
	ailsnd->endSound();
}

void detail::enable_room_sound() {
	for (int16 i = 0; i < MAXDETAILS; i++)
		enable_detail_sound(i);
}

void detail::clear_room_sound() {
	for (int16 j = 0; j < MAXDETAILS; j++) {
		sound_def_blk *sdb = &rdi.Ainfo[j].sfx;
		for (int16 i = 0; i < MAX_SOUNDS; i++) {
			sdb->sound_enable[i] = false;
			sdb->sound_index[i] = -1;
			sdb->sound_start[i] = 0;
			sdb->kanal[i] = 0;
			sdb->volume[i] = 0;
			sdb->repeats[i] = 0;
			sdb->stereo[i] = 0;
		}
	}
	for (int16 i = 0; i < MAXDETAILS * MAX_SOUNDS; i++) {
		rdi.tvp_index[i] = -1;
		rdi.sample[i] = nullptr;
	}
}

void detail::remove_unused_samples() {
	bool found = false;

	for (int16 k = 0; k < MAXDETAILS * MAX_SOUNDS; k++) {
		if (rdi.tvp_index[k] != -1) {
			found = false;
			for (int16 i = 0; (i < MAXDETAILS) && (found == false); i++) {
				for (int16 j = 0; (j < MAX_SOUNDS) && (found == false); j++)
					if (rdi.Ainfo[i].sfx.sound_index[j] == k)
						found = true;
			}
			if (!found) {
				rdi.tvp_index[k] = -1;
				rdi.sample[k] = nullptr;
			}
		}
	}
}

int16 detail::mouse_on_detail(int16 mouse_x, int16 mouse_y, int16 scrx, int16 scry) {
	int16 ok = false;
	int16 detail_nr = -1;
	for (int16 i = 0; i < MAXDETAILS && !ok; i++) {
		ani_detail_info *adiptr = &rdi.Ainfo[i];
		if ((adiptr->start_flag) && (adiptr->start_ani != -1) && (adiptr->end_ani != -1)) {
			int16 sprnr = adiptr->ani_count;
			int16 *Cxy = rdi.dptr->korrektur + (sprnr << 1);
			int16 x = adiptr->x + Cxy[0] - scrx;
			int16 y = adiptr->y + Cxy[1] - scry;
			int16 *Xy;
			if (adiptr->load_flag == 1) {
				Xy = (int16 *)taf_load_buffer;
			} else {
				Xy = (int16 *)rdi.dptr->image[sprnr];
			}
			if (mouse_x >= x && mouse_x <= x + Xy[0] &&
			        mouse_y >= y && mouse_y <= y + Xy[1]) {
				ok = true;
				detail_nr = i;
			}
		}
	}

	return detail_nr;
}

void detail::set_taf_ani_mem(byte *load_area) {
	taf_load_buffer = load_area;
	direct_taf_ani = ON;
}

void detail::load_taf_ani_sprite(int16 nr) {
	SpriteResource *res = new SpriteResource(tafname);
	res->getSpriteData(nr, &taf_load_buffer, false);
	delete res;
}

} // namespace Chewy
