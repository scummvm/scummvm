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

#include "chewy/detail.h"
#include "chewy/file.h"
#include "chewy/global.h"

namespace Chewy {

detail::detail() {
	char *tptr;
	int16 i, size;
	size = sizeof(room_detail_info);
	tptr = (char *)&rdi;
	for (i = 0; i < size; i++)
		*tptr++ = 0;
	for (i = 0; i < MAXDETAILS; i++) {
		rdi.Sinfo[i].SprNr = -1;
		rdi.Ainfo[i].start_ani = -1;
	}
	for (i = 0; i < (MAX_M_ITEMS - 1) << 2; i += 4)
		rdi.mvect[i] = -1;
#ifdef KONVERT
	for (i = 0; i < MAXDETAILS; i++) {
		rdi_new.Sinfo[i].SprNr = -1;
		rdi_new.Ainfo[i].start_ani = -1;
	}

#endif
	global_delay = false;
	ani_freezeflag = false;
	fulltaf = false;
	rdi.dptr = 0;
	tafname[0] = 0;
	for (i = 0; i < (MAXDETAILS * MAX_SOUNDS); i++) {
		rdi.sample[i] = NULL;
		rdi.tvp_index[i] = -1;
	}
	SoundBuffer = 0;
	direct_taf_ani = OFF;
	CurrentTaf = 0;
}

detail::~detail() {
	if (CurrentTaf)
		chewy_fclose(CurrentTaf);
	direct_taf_ani = OFF;
}

void detail::load_rdi(const char *fname_, int16 room_nr) {
	Stream *handle;
	taf_info *tmprdi;
	tmprdi = rdi.dptr;
	handle = chewy_fopen(fname_, "rb");
	if (handle) {
		if (chewy_fread(&rdi_datei_header, sizeof(RdiDateiHeader), 1, handle)) {
			if (!scumm_strnicmp(rdi_datei_header.Id, "RDI", 3)) {
				chewy_fseek(handle, (long)room_nr * (long)sizeof(room_detail_info), SEEK_CUR);
				if (!chewy_fread(&rdi, sizeof(room_detail_info), 1, handle)) {
					modul = DATEI;
					fcode = READFEHLER;
				}
			} else {
				modul = DATEI;
				fcode = READFEHLER;
			}
		} else {
			modul = DATEI;
			fcode = READFEHLER;
		}
		chewy_fclose(handle);
	} else {
		modul = DATEI;
		fcode = OPENFEHLER;
	}
	rdi.dptr = tmprdi;
}

void detail::load_rdi_taf(const char *fname_, int16 load_flag) {
	if (strcmp(tafname, fname_)) {
		if (rdi.dptr) {
			if (fulltaf) {
				free((char *)rdi.dptr);
				rdi.dptr = 0;
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
			rdi.dptr = 0;
			tafname[0] = 0;
		}
		rdi.dptr = 0;
	}

}

void detail::load_taf_tbl(taf_info *fti) {

	int16 i;
	if (!fti) {
		fti = rdi.dptr;
	}
	if (!modul) {
		if (fti) {

			if (CurrentTaf) {
				for (i = 0; i < MAXDETAILS; i++) {
					if (rdi.Sinfo[i].SprNr != -1)
						load_taf_seq(CurrentTaf, rdi.Sinfo[i].SprNr, 1, fti);
					if (rdi.Ainfo[i].start_ani != -1 &&
					        rdi.Ainfo[i].end_ani != -1 && !rdi.Ainfo[i].load_flag)
						load_taf_seq(CurrentTaf, rdi.Ainfo[i].start_ani, (rdi.Ainfo[i].end_ani - rdi.Ainfo[i].start_ani) + 1, fti);
				}

			} else {
				modul = DATEI;
				fcode = OPENFEHLER;
			}
		}
	} else {
		ERROR
	}
}

taf_info *detail::init_taf_tbl(const char *fname_) {
	taf_dateiheader *tafheader;
	taf_info *Tt;
	char *tmp;
	int16 anz;
	int16 i;

	Tt = 0;

	mem->file->get_tafinfo(fname_, &tafheader);
	if (!modul) {
		anz = tafheader->count;
		tmp = (char *)calloc((int32)anz * 4l + sizeof(taf_info), 1);
		if (!modul) {
			Tt = (taf_info *)tmp;
			Tt->anzahl = anz;
			Tt->korrektur = (int16 *)calloc((int32)Tt->anzahl * 4l, 1);
			if (!modul) {
				mem->file->load_korrektur(fname_, (byte *)Tt->korrektur);
				Tt->palette = 0;
				CurrentTaf = chewy_fopen(fname_, "rb");
				if (CurrentTaf) {
					load_sprite_pointer(CurrentTaf);
				} else {
					modul = DATEI;
					fcode = OPENFEHLER;
				}
				for (i = 0; i < Tt->anzahl; i++)
					Tt->image[i] = 0;
			}
		}
	}
	return(Tt);
}

void detail::del_taf_tbl(taf_info *Tt) {
	int16 i;
	if (!Tt) {
		Tt = rdi.dptr;
	}
	for (i = 0; i < Tt->anzahl; i++) {
		free(Tt->image[i]);
	}
	free((char *) Tt->korrektur);
	free((char *) Tt);

	if (CurrentTaf) {
		chewy_fclose(CurrentTaf);
		CurrentTaf = 0;
	}
}

void detail::del_taf_tbl(int16 start, int16 anz, taf_info *Tt) {
	int16 i;
	if (!Tt)
		Tt = rdi.dptr;
	for (i = start; i < start + anz && i < Tt->anzahl; i++) {
		free(Tt->image[i]);
		Tt->image[i] = 0;
	}
}

void detail::load_taf_seq(int16 spr_nr, int16 spr_anz, taf_info *Tt) {

	if (!Tt)
		Tt = rdi.dptr;

	if (CurrentTaf) {
		load_taf_seq(CurrentTaf, spr_nr, spr_anz, Tt);

	}
}

void detail::load_taf_seq(void *h, int16 spr_nr, int16 spr_anz, taf_info *Tt) {
	Stream *handle = (Stream *)h;
	uint32 size;
	taf_imageheader iheader;
	int16 i;

	chewy_fseek(handle, SpritePos[spr_nr], SEEK_SET);
	for (i = 0; i < spr_anz && !modul; i++) {
		if (chewy_fread(&iheader, sizeof(taf_imageheader), 1, handle)) {
			if (!Tt->image[spr_nr + i]) {
				size = iheader.width * iheader.height ;
				Tt->image[spr_nr + i] = (byte *)calloc(size + 4l, 1);
				((int16 *)Tt->image[spr_nr + i])[0] = iheader.width;
				((int16 *)Tt->image[spr_nr + i])[1] = iheader.height;
				chewy_fseek(handle, iheader.image, SEEK_SET);
				mem->file->load_tafmcga(handle, iheader.komp, size, Tt->image[spr_nr + i] + 4l);
			}
			chewy_fseek(handle, iheader.next, SEEK_SET);
		} else {
			fcode = READFEHLER;
			modul = DATEI;
		}
	}
}

#ifdef ICM

void detail::save_detail(char *fname, int16 room_nr) {
	Stream *new_handle;
	Stream *old_handle;
	taf_info *tmprdi;
	char tmp_name[MAXPATH + 1];
	char *str;
#ifdef KONVERT
	room_detail_info_new tmp_rdi_new;
#endif
	room_detail_info tmp_rdi;
	int16 i;
	char *tmp;
	modul = 0;
	fcode = 0;
	tmprdi = rdi.dptr;
	tmp = (char *)&tmp_rdi;
	for (i = 0; i < sizeof(room_detail_info); i++)
		tmp[i] = 0;

	strcpy(tmp_name, fname);
	str = strchr(tmp_name, '.');
	strcpy(str + 1, "BAK");
	mem->file->fcopy(tmp_name, fname);
	remove("rdi.old\0");
	mem->file->fcopy("rdi.old", fname);
	old_handle = chewy_fopen("rdi.old", "rb");
	if (old_handle) {
		new_handle = chewy_fopen(fname, "wb");
		if (new_handle) {
			strcpy(rdi_datei_header.Id, "RDI\0");
			rdi_datei_header.Anz = MAX_RDI_ROOM;
			if (chewy_fwrite(&rdi_datei_header, sizeof(RdiDateiHeader), 1, new_handle)) {

				chewy_fseek(old_handle, sizeof(RdiDateiHeader), SEEK_SET);
				for (i = 0; i < room_nr && !modul ; i++) {
					if (chewy_fread(&tmp_rdi, sizeof(room_detail_info), 1, old_handle)) {
#ifdef KONVERT
						konvert_format(&tmp_rdi, &tmp_rdi_new);
						if (!chewy_fwrite(&tmp_rdi_new, sizeof(room_detail_info_new), 1, new_handle))
#else
						if (!chewy_fwrite(&tmp_rdi, sizeof(room_detail_info), 1, new_handle))
#endif
						{
							modul = DATEI;
							fcode = WRITEFEHLER;
						}
					} else {
						modul = DATEI;
						fcode = READFEHLER;
					}
				}

#ifdef KONVERT
				konvert_format(&rdi, &rdi_new);
				if (chewy_fwrite(&rdi_new, sizeof(room_detail_info_new), 1, new_handle))
#else
				if (chewy_fwrite(&rdi, sizeof(room_detail_info), 1, new_handle))
#endif
				{

					chewy_fseek(old_handle, sizeof(room_detail_info), SEEK_CUR);

					for (i = 0; i < MAX_RDI_ROOM - (room_nr + 1) && !modul ; i++) {
						if (chewy_fread(&tmp_rdi, sizeof(room_detail_info), 1, old_handle)) {
#ifdef KONVERT
							konvert_format(&tmp_rdi, &tmp_rdi_new);
							if (!chewy_fwrite(&tmp_rdi_new, sizeof(room_detail_info_new), 1, new_handle))
#else
							if (!chewy_fwrite(&tmp_rdi, sizeof(room_detail_info), 1, new_handle))
#endif
							{
								modul = DATEI;
								fcode = WRITEFEHLER;
							}
						} else {
							modul = DATEI;
							fcode = READFEHLER;
						}
					}
				} else {
					modul = DATEI;
					fcode = WRITEFEHLER;
				}
			} else {
				modul = DATEI;
				fcode = WRITEFEHLER;
			}
			chewy_fclose(new_handle);
		} else {
			modul = DATEI;
			fcode = OPENFEHLER;
		}
		chewy_fclose(old_handle);
	} else {
		modul = DATEI;
		fcode = OPENFEHLER;
	}
	remove("rdi.old\0");
	rdi.dptr = tmprdi;
}

#endif

#ifdef KONVERT
void detail::konvert_format(room_detail_info *rdi_a,
                            room_detail_info_new *rdi_n) {

	char *tmp;
	char *tmp1;
	int16 j;
	int16 i;

	rdi_n->StaticDetailAnz = rdi_a->StaticDetailAnz;
	rdi_n->AniDetailAnz = rdi_a->AniDetailAnz;
	rdi_n->dptr = rdi_a->dptr;

	for (j = 0; j < MAXDETAILS; j++) {
		rdi_n->Ainfo[j].x = rdi_a->Ainfo[j].x;
		rdi_n->Ainfo[j].y = rdi_a->Ainfo[j].y;
		rdi_n->Ainfo[j].start_flag = rdi_a->Ainfo[j].start_flag;
		rdi_n->Ainfo[j].repeat = rdi_a->Ainfo[j].repeat;
		rdi_n->Ainfo[j].start_ani = rdi_a->Ainfo[j].start_ani;
		rdi_n->Ainfo[j].end_ani = rdi_a->Ainfo[j].end_ani;
		rdi_n->Ainfo[j].ani_count = rdi_a->Ainfo[j].ani_count;
		rdi_n->Ainfo[j].delay = rdi_a->Ainfo[j].delay;
		rdi_n->Ainfo[j].delay_count = rdi_a->Ainfo[j].delay_count;
		rdi_n->Ainfo[j].reverse = rdi_a->Ainfo[j].reverse;
		rdi_n->Ainfo[j].timer_start = rdi_a->Ainfo[j].timer_start;
		rdi_n->Ainfo[j].z_ebene = rdi_a->Ainfo[j].z_ebene;
		rdi_n->Ainfo[j].load_flag = rdi_a->Ainfo[j].load_flag;
		rdi_n->Ainfo[j].zoom = rdi_a->Ainfo[j].zoom;
		rdi_n->Ainfo[j].show_1_phase = rdi_a->Ainfo[j].show_1_phase;
		rdi_n->Ainfo[j].phase_nr = rdi_a->Ainfo[j].phase_nr;
	}

	for (i = 0; i < MAXDETAILS * MAX_SOUNDS; i++) {
		rdi_n->tvp_index[i] = rdi_a->tvp_index[i];
		rdi_n->sample [i] = rdi_a->sample [i];
		for (j = 0; j < MAX_SOUNDS; j++) {
			rdi_n->Ainfo[i / MAX_SOUNDS].sfx.sound_enable[j] =
			    rdi_a->Ainfo[i / 3].sfx.sound_enable[j];
			rdi_n->Ainfo[i / MAX_SOUNDS].sfx.sound_index [j] =
			    rdi_a->Ainfo[i / 3].sfx.sound_index [j];
			rdi_n->Ainfo[i / MAX_SOUNDS].sfx.sound_start [j] =
			    rdi_a->Ainfo[i / 3].sfx.sound_start [j];
			rdi_n->Ainfo[i / MAX_SOUNDS].sfx.kanal [j] =
			    rdi_a->Ainfo[i / 3].sfx.kanal [j];
			rdi_n->Ainfo[i / MAX_SOUNDS].sfx.volume [j] =
			    rdi_a->Ainfo[i / 3].sfx.volume [j];
			rdi_n->Ainfo[i / MAX_SOUNDS].sfx.repeats [j] =
			    rdi_a->Ainfo[i / 3].sfx.repeats [j];
			rdi_n->Ainfo[i / MAX_SOUNDS].sfx.stereo [j] =
			    rdi_a->Ainfo[i / 3].sfx.stereo [j];
		}
	}

	for (j = 0; j < MAXDETAILS; j++) {
		rdi_n->Sinfo[j].x = rdi_a->Sinfo[j].x;
		rdi_n->Sinfo[j].y = rdi_a->Sinfo[j].y;
		rdi_n->Sinfo[j].SprNr = rdi_a->Sinfo[j].SprNr;
		rdi_n->Sinfo[j].z_ebene = rdi_a->Sinfo[j].z_ebene;
		rdi_n->Sinfo[j].Hide = rdi_a->Sinfo[j].Hide;
		rdi_n->Sinfo[j].Dummy = rdi_a->Sinfo[j].Dummy;
	}

	for (i = 0; i < MAX_M_ITEMS * 4; i++)
		rdi_n->mvect[i] = rdi_a->mvect[i];

	for (i = 0; i < MAX_M_ITEMS; i++)
		rdi_n->mtxt[i] = rdi_a->mtxt[i];

	rdi_n->Ri.RoomNr = rdi_a->Ri.RoomNr;
	rdi_n->Ri.BildNr = rdi_a->Ri.BildNr;
	rdi_n->Ri.AutoMovAnz = rdi_a->Ri.AutoMovAnz;
	rdi_n->Ri.TafLoad = rdi_a->Ri.TafLoad;
	strncpy(rdi_n->Ri.TafName, rdi_a->Ri.TafName, 14);
	rdi_n->Ri.ZoomFak = rdi_a->Ri.ZoomFak;
	rdi_n->Ri.Dummy = rdi_a->Ri.Dummy;

	for (j = 0; j < MAX_AUTO_MOV; j++) {
		rdi_n->AutoMov[j].X = rdi_a->AutoMov[j].X;
		rdi_n->AutoMov[j].Y = rdi_a->AutoMov[j].Y;
		rdi_n->AutoMov[j].SprNr = rdi_a->AutoMov[j].SprNr;
		rdi_n->AutoMov[j].dummy = rdi_a->AutoMov[j].dummy;
	}
}
#endif

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
	int16 index;
	index = rdi.Sinfo[det_nr].SprNr;
	if (index == -1)
		ret = 0;
	else
		ret = rdi.dptr->image[index];
	return(ret);
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
	int16 tmp;
	if (start > end) {
		tmp = start;
		start = end;
		end = tmp;
	}
	rdi.Ainfo[ani_nr].start_ani = start;
	rdi.Ainfo[ani_nr].end_ani = end;
}

byte *detail::get_image(int16 spr_nr) {
	byte *ret;
	ret = rdi.dptr->image[spr_nr];
	return(ret);
}

ani_detail_info *detail::get_ani_detail(int16 ani_nr) {
	ani_detail_info *ret;
	ret = &rdi.Ainfo[ani_nr];
	return(ret);
}

int16 *detail::get_korrektur_tbl() {
	int16 *ret;
	ret = rdi.dptr->korrektur;
	return(ret);
}

void detail::init_taf(taf_info *dptr) {
	rdi.dptr = dptr;
}

taf_info *detail::get_taf_info() {
	taf_info *ret;
	ret = rdi.dptr;
	return(ret);
}

room_detail_info *detail::get_room_detail_info() {
	return(&rdi);
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

void detail::plot_ani_details(int16 scrx, int16 scry, int16 start, int16 end,
                              int16 zoomx, int16 zoomy) {
	int16 i, k;
	int16 x, y, sprnr;
	int16 *Cxy;
	int16 kx, ky;
	ani_detail_info *adiptr;
	int16 sound_effect;
	if (start > end) {
		i = start;
		start = end;
		end = i;
	}
	if (start > MAXDETAILS)
		start = 0;
	if (end > MAXDETAILS)
		end = MAXDETAILS - 1;
	for (i = start; (i <= end) && (!modul); i++) {
		adiptr = &rdi.Ainfo[i];
		if ((adiptr->start_flag) && (adiptr->start_ani != -1) && (adiptr->end_ani != -1)) {
			sprnr = adiptr->ani_count;
			Cxy = rdi.dptr->korrektur + (sprnr << 1);
			kx = Cxy[0];
			ky = Cxy[1];
			if (zoomx != 0 || zoomy != 0)
				calc_zoom_kor(&kx, &ky, zoomx, zoomy);
			x = adiptr->x + kx - scrx;
			y = adiptr->y + ky - scry;
			if (adiptr->load_flag == 1) {
				load_taf_ani_sprite(sprnr);
				if (!modul)
					out->scale_set(taf_load_buffer, x, y, zoomx, zoomy, 0);
			} else
				out->scale_set(rdi.dptr->image[sprnr], x, y, zoomx, zoomy, 0);

			for (k = 0; k < MAX_SOUNDS; k++) {
				sound_effect = adiptr->sfx.sound_index[k];
				if ((adiptr->sfx.sound_enable[k]) && (sound_effect != -1) &&
				        (rdi.sample[sound_effect])) {
					if ((adiptr->sfx.sound_start[k] == adiptr->ani_count) &&
					        (!adiptr->delay_count)) {
#ifdef AIL
						ailsnd->set_stereo_pos(adiptr->sfx.kanal[k],
						                       adiptr->sfx.stereo[k]);
						ailsnd->play_voc(rdi.sample[sound_effect], adiptr->sfx.kanal[k],
						                 adiptr->sfx.volume[k], adiptr->sfx.repeats[k]);
#else
#if 0
						snd->play_voc(rdi.sample[sound_effect], adiptr->sfx.kanal[k],
						              adiptr->sfx.volume[k], adiptr->sfx.repeats[k]);
#endif
						warning("STUB: detail::plot_ani_details()");
#endif
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
	int16 x, y;
	int16 i;
	byte *simage;
	if (start > end) {
		i = start;
		start = end;
		end = i;
	}
	if (start > MAXDETAILS)
		start = 0;
	if (end > MAXDETAILS)
		end = MAXDETAILS - 1;
	for (i = start; i <= end; i++) {
		if (rdi.Sinfo[i].SprNr != -1 && !rdi.Sinfo[i].Hide) {
			x = rdi.Sinfo[i].x - scrx;
			y = rdi.Sinfo[i].y - scry;
			simage = rdi.dptr->image[rdi.Sinfo[i].SprNr];
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
	ani_detail_info *adiptr;
	adiptr = &rdi.Ainfo[nr];
	adiptr->start_flag = 0;

}

void detail::set_ani_delay(int16 nr, int16 del) {
	ani_detail_info *adiptr;
	adiptr = &rdi.Ainfo[nr];
	adiptr->delay = del;
}

void detail::init_list(int16 *mv) {
	int16 i;
	for (i = 0; i < ((MAX_M_ITEMS - 1) << 2); i++)
		rdi.mvect[i] = mv[i];
}

void detail::get_list(int16 *mv) {
	int16 i;
	for (i = 0; i < ((MAX_M_ITEMS - 1) << 2); i++)
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
	return (i);
}

int16 detail::get_ani_status(int16 det_nr) {
	int16 ret;
	if (rdi.Ainfo[det_nr].start_flag > 0)
		ret = 1;
	else
		ret = 0;
	return(ret);
}

SprInfo detail::plot_detail_sprite(int16 scrx, int16 scry, int16 det_nr,
                                   int16 spr_nr, int16 mode) {
	int16 *Cxy;
	int16 *Xy;
	ani_detail_info *adiptr;
	int16 k, sound_effect;
	if (det_nr > MAXDETAILS)
		det_nr = MAXDETAILS - 1;
	adiptr = &rdi.Ainfo[det_nr];
	if (spr_nr < adiptr->start_ani)
		spr_nr = adiptr->start_ani;
	if (spr_nr > adiptr->end_ani)
		spr_nr = adiptr->end_ani - 1;
	Cxy = rdi.dptr->korrektur + (spr_nr << 1);
	Xy = (int16 *) rdi.dptr->image[spr_nr];
	spr_info.Image = rdi.dptr->image[spr_nr];
	spr_info.X = adiptr->x + Cxy[0] - scrx;
	spr_info.Y = adiptr->y + Cxy[1] - scry;
	spr_info.X1 = spr_info.X + Xy[0];
	spr_info.Y1 = spr_info.Y + Xy[1];
	if (mode)
		out->sprite_set(spr_info.Image, spr_info.X, spr_info.Y, 0);

	for (k = 0; k < MAX_SOUNDS; k++) {
		sound_effect = adiptr->sfx.sound_index[k];
		if ((adiptr->sfx.sound_enable[k] != false) && (sound_effect != -1) &&
		        (rdi.sample[sound_effect])) {
			if (adiptr->sfx.sound_start[k] == spr_nr) {
#ifdef AIL
				ailsnd->set_stereo_pos(adiptr->sfx.kanal[k],
				                       adiptr->sfx.stereo[k]);
				ailsnd->play_voc(rdi.sample[sound_effect], adiptr->sfx.kanal[k],
				                 adiptr->sfx.volume[k], adiptr->sfx.repeats[k]);
#else
				warning("STUB: detail::plot_detail_sprite()");
#if 0
				snd->play_voc(rdi.sample[sound_effect], adiptr->sfx.kanal[k],
				              adiptr->sfx.volume[k], adiptr->sfx.repeats[k]);
#endif
#endif
			}
		}
	}

	return (spr_info);
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

void detail::set_sound_area(byte *buffer, uint32 size) {
	SoundBuffer = buffer;
	SoundBufferSize = size;
}

void detail::load_room_sounds(Stream *tvp_handle) {
	int16 i, break_flag;
	int16 index;
	byte *workbuf;
	uint32 size;
	uint32 allsize;
	if (tvp_handle) {
		workbuf = SoundBuffer;
		allsize = 0;
		size = 0;
		break_flag = false;
		if (workbuf) {

			for (i = 0; i < MAXDETAILS * MAX_SOUNDS; i++)
				rdi.sample[i] = NULL;

			for (i = 0; (i < MAXDETAILS * MAX_SOUNDS) && (break_flag == false); i++) {
				index = rdi.tvp_index[i];
				if ((index != -1) && (rdi.sample[i] == NULL)) {
					mem->file->select_pool_item(tvp_handle, index);
					size = mem->file->load_voc(tvp_handle, workbuf);
					if (!modul) {

						rdi.sample[i] = workbuf;
						workbuf += size;
						allsize += size;
					} else
						break_flag = true;
					if (allsize > SoundBufferSize) {
						modul = SPEICHER;
						fcode = NOSPEICHER;
						err->set_user_msg("SFX-SIZE TOO LARGE");
						break_flag = true;
					}
				}
			}
		}
	} else {
		modul = DATEI;
		fcode = OPENFEHLER;
	}
}

void detail::set_sound_para(int16 nr, sound_def_blk *sdb_src) {
	sound_def_blk *sdb_dest;
	sdb_dest = &rdi.Ainfo[nr].sfx;
	memcpy(sdb_dest, sdb_src, sizeof(sound_def_blk));
}

void detail::get_sound_para(int16 nr, sound_def_blk *sdb_dest) {
	sound_def_blk *sdb_src;
	sdb_src = &rdi.Ainfo[nr].sfx;
	memcpy(sdb_dest, sdb_src, sizeof(sound_def_blk));
}

void detail::map_tvp2sound(int16 nr, int16 sslot, int16 tvp_index) {
	sound_def_blk *sdb;
	int16 i, j;
	sdb = &rdi.Ainfo[nr].sfx;
	remove_unused_samples();

	bool found = false;
	if (sdb->sound_index[sslot] != -1) {
		for (i = 0; (i < MAXDETAILS) && (!found); i++) {
			for (j = 0; (j < MAX_SOUNDS) && (!found); j++)
				if ((rdi.Ainfo[i].sfx.sound_index[j] == sdb->sound_index[sslot]) &&
				        (i != nr) && (j != sslot))
					found = true;
		}

		if (!found) {
			rdi.tvp_index[sdb->sound_index[sslot]] = -1;
			sdb->sound_index[sslot] = -1;
		}
	}

	found = false;
	for (i = 0; (i < MAXDETAILS * MAX_SOUNDS) && (!found); i++) {
		if (rdi.tvp_index[i] == tvp_index) {
			found = true;
			sdb->sound_index[sslot] = i;
		}
	}

	for (i = 0; (i < MAXDETAILS * MAX_SOUNDS) && (!found); i++) {
		if (rdi.tvp_index[i] == -1) {
			found = true;
			sdb->sound_index[sslot] = i;
			rdi.tvp_index[i] = tvp_index;
		}
	}
}

void detail::disable_sound(int16 nr, int16 sslot) {
	sound_def_blk *sdb;
	sdb = &rdi.Ainfo[nr].sfx;
	sdb->sound_enable[sslot] = 0;
	ailsnd->end_sample(sdb->kanal[sslot]);
}

void detail::enable_sound(int16 nr, int16 sslot) {
	sound_def_blk *sdb;
	sdb = &rdi.Ainfo[nr].sfx;
	sdb->sound_enable[sslot] = 1;
}

void detail::play_sound(int16 nr, int16 sslot) {
	warning("STUB: detail::play_sound()");
#if 0
	sound_def_blk *sdb;
	sdb = &rdi.Ainfo[nr].sfx;
	if ((sdb->sound_enable[sslot] != FALSE) && (sdb->sound_index[sslot] != -1) &&
	        (rdi.sample[sdb->sound_index[sslot]])) {
#ifdef AIL
		ailsnd->set_stereo_pos(sdb->kanal[sslot], sdb->stereo[sslot]);
		ailsnd->play_voc(rdi.sample[sdb->sound_index[sslot]], sdb->kanal[sslot],
		                 sdb->volume[sslot], sdb->repeats[sslot]);
#else
		snd->play_voc(rdi.sample[sdb->sound_index[sslot]], sdb->kanal[sslot],
		              sdb->volume[sslot], sdb->repeats[sslot]);
#endif
	}
#endif
}

void detail::disable_detail_sound(int16 nr) {
	sound_def_blk *sdb;
	int16 i;
	sdb = &rdi.Ainfo[nr].sfx;
	for (i = 0; i < MAX_SOUNDS; i++)
		sdb->sound_enable[i] = 0;
}

void detail::enable_detail_sound(int16 nr) {
	sound_def_blk *sdb;
	int16 i;
	sdb = &rdi.Ainfo[nr].sfx;
	for (i = 0; i < MAX_SOUNDS; i++)
		if (sdb->sound_index[i] != -1)
			sdb->sound_enable[i] = 1;
}

void detail::play_detail_sound(int16 nr) {
	warning("STUB: detail::play_detail_sound()");
#if 0
	sound_def_blk *sdb;
	int16 k;
	sdb = &rdi.Ainfo[nr].sfx;
	for (k = 0; k < MAX_SOUNDS; k++) {
		if ((sdb->sound_enable[k] != FALSE) && (sdb->sound_index[k] != -1) &&
		        (rdi.sample[sdb->sound_index[k]])) {
#ifdef AIL
			ailsnd->set_stereo_pos(sdb->kanal[k], sdb->stereo[k]);
			ailsnd->play_voc(rdi.sample[sdb->sound_index[k]], sdb->kanal[k],
			                 sdb->volume[k], sdb->repeats[k]);
#else
			snd->play_voc(rdi.sample[sound_effect], sdb->kanal[k],
			              sdb->volume[k], sdb->repeats[k]);
#endif
		}
	}
#endif
}

void detail::clear_detail_sound(int16 nr) {
	sound_def_blk *sdb;
	short i;
	sdb = &rdi.Ainfo[nr].sfx;
	for (i = 0; i < MAX_SOUNDS; i++) {
		sdb->sound_enable[i] = 0;
		sdb->sound_index [i] = -1;
		sdb->sound_start [i] = 0;
		sdb->kanal [i] = 0;
		sdb->volume [i] = 0;
		sdb->repeats [i] = 0;
		sdb->stereo [i] = 0;
	}
	remove_unused_samples();
}

void detail::disable_room_sound() {
	int16 i;
	for (i = 0; i < MAXDETAILS; i++)
		disable_detail_sound(i);
	ailsnd->end_sound();
}

void detail::enable_room_sound() {
	int16 i;
	for (i = 0; i < MAXDETAILS; i++)
		enable_detail_sound(i);
}

void detail::clear_room_sound() {
	sound_def_blk *sdb;
	int16 i, j;
	for (j = 0; j < MAXDETAILS; j++) {
		sdb = &rdi.Ainfo[j].sfx;
		for (i = 0; i < MAX_SOUNDS; i++) {
			sdb->sound_enable[i] = 0;
			sdb->sound_index [i] = -1;
			sdb->sound_start [i] = 0;
			sdb->kanal [i] = 0;
			sdb->volume [i] = 0;
			sdb->repeats [i] = 0;
			sdb->stereo [i] = 0;
		}
	}
	for (i = 0; i < MAXDETAILS * MAX_SOUNDS; i++) {
		rdi.tvp_index[i] = -1;
		rdi.sample [i] = NULL;
	}
}
#ifdef DETEDIT

void detail::shadow_room(int16 mode) {
	if (mode == SAVE_ROOM)
		memcpy(&rdi_shadow, &rdi, sizeof(room_detail_info));
	else
		memcpy(&rdi, &rdi_shadow, sizeof(room_detail_info));
}
#endif

void detail::remove_unused_samples() {
	int16 i, j, k, found;
	found = false;

	for (k = 0; k < MAXDETAILS * MAX_SOUNDS; k++) {
		if (rdi.tvp_index[k] != -1) {
			found = false;
			for (i = 0; (i < MAXDETAILS) && (found == false); i++) {
				for (j = 0; (j < MAX_SOUNDS) && (found == false); j++)
					if (rdi.Ainfo[i].sfx.sound_index[j] == k)
						found = true;
			}
			if (found == false) {
				rdi.tvp_index[k] = -1;
				rdi.sample[k] = NULL;
			}
		}
	}
}

int16 detail::mouse_on_detail(int16 mouse_x, int16 mouse_y,
                              int16 scrx, int16 scry) {
	int16 i;
	int16 ok;
	int16 detail_nr;
	int16 *Cxy;
	int16 *Xy;
	int16 sprnr;
	int16 x, y;
	ani_detail_info *adiptr;
	ok = false;
	detail_nr = -1;
	for (i = 0; i < MAXDETAILS && !ok; i++) {
		adiptr = &rdi.Ainfo[i];
		if ((adiptr->start_flag) && (adiptr->start_ani != -1) && (adiptr->end_ani != -1)) {
			sprnr = adiptr->ani_count;
			Cxy = rdi.dptr->korrektur + (sprnr << 1);
			x = adiptr->x + Cxy[0] - scrx;
			y = adiptr->y + Cxy[1] - scry;
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
	return (detail_nr);
}

void detail::set_taf_ani_mem(byte *load_area) {
	taf_load_buffer = load_area;
	direct_taf_ani = ON;
}

void detail::load_taf_ani_sprite(int16 nr) {
	int32 size;
	taf_imageheader iheader;
	if (CurrentTaf) {
		chewy_fseek(CurrentTaf, SpritePos[nr], SEEK_SET);

		if (chewy_fread(&iheader, sizeof(taf_imageheader), 1, CurrentTaf)) {
			size = (int32)iheader.width * (int32)iheader.height;
			((int16 *)taf_load_buffer)[0] = iheader.width;
			((int16 *)taf_load_buffer)[1] = iheader.height;
			chewy_fseek(CurrentTaf, iheader.image, SEEK_SET);
			if (taf_load_buffer)
				mem->file->load_tafmcga(CurrentTaf, iheader.komp, size, taf_load_buffer + 4l);
			else {
				modul = SPEICHER;
				fcode = NOSPEICHER;
				err->set_user_msg("Taf-Load-Puffer nicht initialisiert");
			}
		} else {
			fcode = READFEHLER;
			modul = DATEI;
		}
	} else {
		fcode = OPENFEHLER;
		modul = DATEI;
		err->set_user_msg("Arbeitstaf nicht geöffnet");
	}
}

void detail::load_sprite_pointer(void *h) {
	Stream *handle = (Stream *)h;
	uint16 anzahl;
	taf_dateiheader header;
	taf_imageheader iheader;
	int16 i;
	if (handle) {
		chewy_fseek(handle, 0, SEEK_SET);
		if (chewy_fread(&header, sizeof(taf_dateiheader), 1, handle)) {
			anzahl = header.count;
			chewy_fseek(handle, header.next, SEEK_SET);
			SpritePos[0] = header.next;
			for (i = 1; i < anzahl && !modul; i++) {
				if (!chewy_fread(&iheader, sizeof(taf_imageheader), 1, handle)) {
					fcode = READFEHLER;
					modul = DATEI;
				}
				SpritePos[i] = iheader.next;
				chewy_fseek(handle, iheader.next, SEEK_SET);
			}
		} else {
			modul = DATEI;
			fcode = READFEHLER;
		}
	}
}

} // namespace Chewy
