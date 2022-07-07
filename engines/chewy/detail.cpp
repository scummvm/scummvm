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
#include "chewy/globals.h"
#include "chewy/mcga_graphics.h"
#include "chewy/memory.h"
#include "chewy/sound.h"
#include "chewy/resource.h"

namespace Chewy {

#define ON 1
#define OFF 0

bool RoomInfo::load(Common::SeekableReadStream *src) {
	_roomNr = src->readByte();
	_imageNr = src->readByte();
	_autoMovNr = src->readByte();
	_tafLoad = src->readByte();
	src->read(_tafName, 14);
	_zoomFactor = src->readByte();
	_dummy = src->readByte();

	return true;
}

bool RoomAutoMov::load(Common::SeekableReadStream *src) {
	_x = src->readSint16LE();
	_y = src->readSint16LE();
	_sprNr = src->readByte();
	_dummy = src->readByte();

	return true;
}

bool SoundDefBlk::load(Common::SeekableReadStream *src) {
	int i;

	src->skip(MAX_SOUNDS * 2);	// sound_enable flags
	for (i = 0; i < MAX_SOUNDS; ++i)
		sound_index[i] = src->readSint16LE();
	for (i = 0; i < MAX_SOUNDS; ++i)
		sound_start[i] = src->readSint16LE();
	for (i = 0; i < MAX_SOUNDS; ++i)
		channel[i] = src->readSint16LE();
	for (i = 0; i < MAX_SOUNDS; ++i)
		volume[i] = src->readSint16LE();
	for (i = 0; i < MAX_SOUNDS; ++i)
		repeats[i] = src->readSint16LE();
	for (i = 0; i < MAX_SOUNDS; ++i)
		stereo[i] = src->readSint16LE();

	return true;
}

bool AniDetailInfo::load(Common::SeekableReadStream *src) {
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

bool StaticDetailInfo::load(Common::SeekableReadStream *src) {
	x = src->readSint16LE();
	y = src->readSint16LE();
	SprNr = src->readSint16LE();
	z_ebene = src->readSint16LE();
	hide = src->readByte();
	Dummy = src->readByte();

	return true;
}

bool RoomDetailInfo::load(Common::SeekableReadStream *src) {
	int i;

	_staticDetailNr = src->readSint16LE();
	_aniDetailNr = src->readSint16LE();
	src->skip(4);	// dptr
	for (i = 0; i < MAXDETAILS; ++i)
		Ainfo[i].load(src);
	for (i = 0; i < MAXDETAILS; ++i)
		staticSprite[i].load(src);
	for (i = 0; i < MAX_M_ITEMS * 4; ++i)
		mvect[i] = src->readSint16LE();
	for (i = 0; i < MAX_M_ITEMS; ++i)
		mtxt[i] = src->readSint16LE();
	Ri.load(src);
	for (i = 0; i < MAX_AUTO_MOV; ++i)
		autoMove[i].load(src);
	for (i = 0; i < MAXDETAILS * MAX_SOUNDS; ++i)
		detailSfxIndex[i] = src->readSint16LE();
	src->skip(4 * MAXDETAILS * MAX_SOUNDS); // sample offsets

	return true;
}

bool RdiDataHeader::load(Common::SeekableReadStream *src) {
	src->read(_id, 4);
	_nr = src->readSint16LE();

	return true;
}


Detail::Detail() {
	int16 size = sizeof(RoomDetailInfo);
	char *tptr = (char *)&_rdi;

	for (int16 i = 0; i < size; i++)
		*tptr++ = 0;

	for (int16 i = 0; i < MAXDETAILS; i++) {
		_rdi.staticSprite[i].SprNr = -1;
		_rdi.Ainfo[i].start_ani = -1;
	}

	for (int16 i = 0; i < (MAX_M_ITEMS - 1) << 2; i += 4)
		_rdi.mvect[i] = -1;

	_globalDelay = 0;
	_aniFreezeflag = false;
	_fullTaf = false;
	_rdi.dptr = nullptr;
	_tafName = "";
	for (int16 i = 0; i < (MAXDETAILS * MAX_SOUNDS); i++) {
		_rdi.detailSfxIndex[i] = -1;
	}
	_directTafAni = OFF;
}

Detail::~Detail() {
	_directTafAni = OFF;
}

void Detail::load_rdi(const char *fname_, int16 room_nr) {
	Common::File f;
	TafInfo *tmprdi = _rdi.dptr;

	if (fname_ && f.open(fname_)) {
		if (_rdiDataHeader.load(&f)) {
			if (!scumm_strnicmp(_rdiDataHeader._id, "RDI", 3)) {
				f.seek(room_nr * RoomDetailInfo::SIZE(), SEEK_CUR);

				if (!_rdi.load(&f)) {
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

	_rdi.dptr = tmprdi;
}

void Detail::load_rdi_taf(const char *filename, int16 load_flag) {
	if (_tafName.compareToIgnoreCase(filename)) {
		if (_rdi.dptr) {
			if (_fullTaf) {
				free(_rdi.dptr);
				_rdi.dptr = nullptr;
				_fullTaf = false;
			} else {
				del_taf_tbl(_rdi.dptr);
			}
		}
		_tafName = filename;
		if (!load_flag) {
			_rdi.dptr = init_taf_tbl(filename);
			load_taf_tbl(_rdi.dptr);
		} else {
			_rdi.dptr = _G(mem)->taf_adr(filename);
			_fullTaf = true;
		}
	} else if (!_fullTaf)
		load_taf_tbl(_rdi.dptr);
}

void Detail::del_dptr() {
	if (_rdi.dptr) {
		if (_fullTaf)
			free((char *)_rdi.dptr);
		else {
			del_taf_tbl(_rdi.dptr);
			_rdi.dptr = nullptr;
			_tafName = "";
		}
		_rdi.dptr = nullptr;
	}
}

void Detail::load_taf_tbl(TafInfo *fti) {
	if (!fti) {
		fti = _rdi.dptr;
	}

	if (fti) {
		for (int16 i = 0; i < MAXDETAILS; i++) {
			if (_rdi.staticSprite[i].SprNr != -1)
				load_taf_seq(_rdi.staticSprite[i].SprNr, 1, fti);
			if (_rdi.Ainfo[i].start_ani != -1 &&
					_rdi.Ainfo[i].end_ani != -1 && !_rdi.Ainfo[i].load_flag)
				load_taf_seq(_rdi.Ainfo[i].start_ani, (_rdi.Ainfo[i].end_ani - _rdi.Ainfo[i].start_ani) + 1, fti);
		}
	}
}

TafInfo *Detail::init_taf_tbl(const char *fname_) {
	TafInfo *Tt = nullptr;
	SpriteResource *res = new SpriteResource(_tafName);
	int16 nr = res->getChunkCount();
	byte *tmp = (byte *)MALLOC((int32)nr * sizeof(byte *) + sizeof(TafInfo));

	Tt = (TafInfo *)tmp;
	Tt->count = nr;
	Tt->correction = (int16 *)MALLOC((int32)Tt->count * 2 * sizeof(int16));
	Tt->image = (byte **)(tmp + sizeof(TafInfo));
	memcpy(Tt->correction, (byte *)res->getSpriteCorrectionsTable(), Tt->count * 2 * sizeof(int16));
	Tt->palette = nullptr;

	delete res;

	return Tt;
}

void Detail::del_taf_tbl(TafInfo *Tt) {
	if (!Tt) {
		Tt = _rdi.dptr;
	}
	for (int16 i = 0; i < Tt->count; i++) {
		free(Tt->image[i]);
	}
	free(Tt->correction);
	free(Tt);
}

void Detail::del_taf_tbl(int16 start, int16 nr, TafInfo *Tt) {
	if (!Tt)
		Tt = _rdi.dptr;
	for (int16 i = start; i < start + nr && i < Tt->count; i++) {
		free(Tt->image[i]);
		Tt->image[i] = nullptr;
	}
}

void Detail::load_taf_seq(int16 sprNr, int16 sprCount, TafInfo *Tt) {
	if (!Tt)
		Tt = _rdi.dptr;

	SpriteResource *res = new SpriteResource(_tafName);

	for (int16 i = 0; i < sprCount; i++) {
		if (!Tt->image[sprNr + i]) {
			res->getSpriteData(sprNr + i, &Tt->image[sprNr + i], true);
		}
	}

	delete res;
}

void Detail::hideStaticSpr(int16 nr) {
	if (nr >= 0 && nr < MAXDETAILS)
		_rdi.staticSprite[nr].hide = true;
}

void Detail::showStaticSpr(int16 nr) {
	if (nr >= 0 && nr < MAXDETAILS)
		_rdi.staticSprite[nr].hide = false;
}

void Detail::setStaticPos(int16 detNr, int16 x, int16 y, bool hideFl, bool correctionFlag) {
	if (correctionFlag) {
		int16 *Cxy = &_rdi.dptr->correction[_rdi.staticSprite[detNr].SprNr * 2];
		x += Cxy[0];
		y += Cxy[1];
	}
	_rdi.staticSprite[detNr].x = x;
	_rdi.staticSprite[detNr].y = y;
	_rdi.staticSprite[detNr].hide = hideFl;
}

void Detail::setDetailPos(int16 detNr, int16 x, int16 y) {
	_rdi.Ainfo[detNr].x = x;
	_rdi.Ainfo[detNr].y = y;
}

void Detail::getAniValues(int16 aniNr, int16 *start, int16 *end) {
	*start = _rdi.Ainfo[aniNr].start_ani;
	*end = _rdi.Ainfo[aniNr].end_ani;
}

AniDetailInfo *Detail::getAniDetail(int16 aniNr) {
	return &_rdi.Ainfo[aniNr];
}

void Detail::init_taf(TafInfo *dptr) {
	_rdi.dptr = dptr;
}

TafInfo *Detail::get_taf_info() {
	return _rdi.dptr;
}

RoomDetailInfo *Detail::getRoomDetailInfo() {
	return &_rdi;
}

void Detail::freezeAni() {
	_aniFreezeflag = true;
}

void Detail::unfreezeAni() {
	_aniFreezeflag = false;
}

void Detail::set_static_ani(int16 ani_nr, int16 static_nr) {
	_rdi.Ainfo[ani_nr].show_1_phase = true;
	if (static_nr != -1)
		_rdi.Ainfo[ani_nr].phase_nr = static_nr;
}

void Detail::del_static_ani(int16 ani_nr) {
	_rdi.Ainfo[ani_nr].show_1_phase = false;
}

void Detail::plot_ani_details(int16 scrx, int16 scry, int16 start, int16 end, int16 zoomx, int16 zoomy) {
	if (start > end)
		SWAP(start, end);

	if (start >= MAXDETAILS)
		start = 0;

	if (end >= MAXDETAILS)
		end = MAXDETAILS - 1;

	for (int16 i = start; (i <= end); i++) {
		AniDetailInfo *adiptr = &_rdi.Ainfo[i];
		if ((adiptr->start_flag) && (adiptr->start_ani != -1) && (adiptr->end_ani != -1)) {
			int16 sprnr = adiptr->ani_count;
			int16 *Cxy = _rdi.dptr->correction + sprnr * 2;
			int16 kx = Cxy[0];
			int16 ky = Cxy[1];
			if (zoomx != 0 || zoomy != 0)
				calc_zoom_kor(&kx, &ky, zoomx, zoomy);
			int16 x = adiptr->x + kx - scrx;
			int16 y = adiptr->y + ky - scry;
			if (adiptr->load_flag == 1) {
				load_taf_ani_sprite(sprnr);
				_G(out)->scale_set(_tafLoadBuffer, x, y, zoomx, zoomy, 0);
			} else
				_G(out)->scale_set(_rdi.dptr->image[sprnr], x, y, zoomx, zoomy, 0);

			Sound *sound = g_engine->_sound;

			for (int16 k = 0; k < MAX_SOUNDS; k++) {
				const int16 sfxSlot = adiptr->sfx.sound_index[k];
				if (sfxSlot != -1) {
					const int16 sfxIndex = _rdi.detailSfxIndex[sfxSlot];
					if (adiptr->sfx.sound_start[k] == adiptr->ani_count &&
					        !adiptr->delay_count) {
						const uint channel = adiptr->sfx.channel[k] & 7;
						sound->playSound(sfxIndex, channel, false /*adiptr->sfx.repeats[k]*/, adiptr->sfx.volume[k], adiptr->sfx.stereo[k]);
					}
				}
			}

			if (!_aniFreezeflag) {
				if (adiptr->reverse) {
					if (adiptr->delay_count > 0)
						--adiptr->delay_count;
					else {
						adiptr->delay_count = adiptr->delay + _globalDelay;
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
						adiptr->delay_count = adiptr->delay + _globalDelay;
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
				_rdi.staticSprite[adiptr->phase_nr].hide = false;
				plot_static_details(scrx, scry, adiptr->phase_nr, adiptr->phase_nr);
				_rdi.staticSprite[adiptr->phase_nr].hide = true;
			}
		}
	}
}

void Detail::plot_static_details(int16 scrx, int16 scry, int16 start, int16 end) {
	if (start > end)
		SWAP(start, end);

	if (start >= MAXDETAILS)
		start = 0;
	
	if (end >= MAXDETAILS)
		end = MAXDETAILS - 1;

	for (int16 i = start; i <= end; i++) {
		if (_rdi.staticSprite[i].SprNr != -1 && !_rdi.staticSprite[i].hide) {
			int16 x = _rdi.staticSprite[i].x - scrx;
			int16 y = _rdi.staticSprite[i].y - scry;
			byte *spriteImage = _rdi.dptr->image[_rdi.staticSprite[i].SprNr];
			_G(out)->spriteSet(spriteImage, x, y, 0);
		}
	}
}

void Detail::startDetail(int16 nr, int16 rep, int16 reverse) {
	if (nr != -1) {
		AniDetailInfo *adiptr = &_rdi.Ainfo[nr];
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

void Detail::stop_detail(int16 nr) {
	if (nr >= 0 && nr < 32) {
		_rdi.Ainfo[nr].start_flag = 0;
	}
}

void Detail::playSound(int16 nr, int16 slot) {
	const int16 sfxSlot = _rdi.Ainfo[nr].sfx.sound_index[slot];
	if (sfxSlot != -1) {
		const int16 sfxIndex = _rdi.detailSfxIndex[sfxSlot];
		g_engine->_sound->playSound(sfxIndex, slot);
	}
}

void Detail::stopSound(int16 slot) {
	g_engine->_sound->stopSound(slot);
}

int16 Detail::maus_vector(int16 x, int16 y) {
	int16 i = -1;
	for (int16 j = 0; (j < (MAX_M_ITEMS - 1) << 2) && (i == -1); j += 4) {
		if (_rdi.mvect[j] != -1) {
			if ((x >= _rdi.mvect[j]) && (x <= _rdi.mvect[j + 2]) &&
			        (y >= _rdi.mvect[j + 1]) && (y <= _rdi.mvect[j + 3]))
				i = j / 4;
		}
	}

	return i;
}

int16 Detail::get_ani_status(int16 det_nr) {
	return (_rdi.Ainfo[det_nr].start_flag > 0) ? 1 : 0;
}

SprInfo Detail::plot_detail_sprite(int16 scrx, int16 scry, int16 det_nr, int16 spr_nr, int16 mode) {
	if (det_nr >= MAXDETAILS)
		det_nr = MAXDETAILS - 1;
	AniDetailInfo *adiptr = &_rdi.Ainfo[det_nr];
	if (spr_nr < adiptr->start_ani)
		spr_nr = adiptr->start_ani;
	if (spr_nr > adiptr->end_ani)
		spr_nr = adiptr->end_ani - 1;
	int16 *Cxy = &_rdi.dptr->correction[spr_nr << 1];
	int16 *Xy = (int16 *)_rdi.dptr->image[spr_nr];
	_sprInfo._image = _rdi.dptr->image[spr_nr];
	_sprInfo._x = adiptr->x + Cxy[0] - scrx;
	_sprInfo._y = adiptr->y + Cxy[1] - scry;
	_sprInfo.X1 = _sprInfo._x + Xy[0];
	_sprInfo.Y1 = _sprInfo._y + Xy[1];
	if (mode)
		_G(out)->spriteSet(_sprInfo._image, _sprInfo._x, _sprInfo._y, 0);

	Sound *sound = g_engine->_sound;

	for (int16 k = 0; k < MAX_SOUNDS; k++) {
		int16 soundEffect = adiptr->sfx.sound_index[k];
		if (soundEffect != -1) {
			if (adiptr->sfx.sound_start[k] == spr_nr) {
				const uint channel = adiptr->sfx.channel[k] & 7;
				sound->playSound(soundEffect, channel, adiptr->sfx.repeats[k], adiptr->sfx.volume[k], adiptr->sfx.stereo[k]);
			}
		}
	}

	return _sprInfo;
}

void Detail::set_global_delay(int16 delay) {
	_globalDelay = delay - 2;
}

void Detail::calc_zoom_kor(int16 *kx, int16 *ky, int16 xzoom, int16 yzoom) {
	float tmpx = (float)(((float)*kx / 100.0) * ((float)xzoom));
	float tmpy = (float)(((float)*ky / 100.0) * ((float)yzoom));

	float tmpx1 = tmpx - (int16)tmpx;
	if (fabs(tmpx1) > 0.5)
		++tmpx;
	float tmpy1 = tmpy - (int16)tmpy;
	if (fabs(tmpy1) > 0.5)
		++tmpy;
	*kx += (int16)tmpx;
	*ky += (int16)tmpy;
}

int16 Detail::mouse_on_detail(int16 mouse_x, int16 mouse_y, int16 scrx, int16 scry) {
	int16 ok = false;
	int16 detail_nr = -1;
	for (int16 i = 0; i < MAXDETAILS && !ok; i++) {
		AniDetailInfo *adiptr = &_rdi.Ainfo[i];
		if ((adiptr->start_flag) && (adiptr->start_ani != -1) && (adiptr->end_ani != -1)) {
			int16 sprnr = adiptr->ani_count;
			int16 *Cxy = _rdi.dptr->correction + (sprnr << 1);
			int16 x = adiptr->x + Cxy[0] - scrx;
			int16 y = adiptr->y + Cxy[1] - scry;
			int16 *Xy;
			if (adiptr->load_flag == 1) {
				Xy = (int16 *)_tafLoadBuffer;
			} else {
				Xy = (int16 *)_rdi.dptr->image[sprnr];
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

void Detail::set_taf_ani_mem(byte *load_area) {
	_tafLoadBuffer = load_area;
	_directTafAni = ON;
}

void Detail::load_taf_ani_sprite(int16 nr) {
	SpriteResource *res = new SpriteResource(_tafName);
	res->getSpriteData(nr, &_tafLoadBuffer, false);
	delete res;
}

} // namespace Chewy
