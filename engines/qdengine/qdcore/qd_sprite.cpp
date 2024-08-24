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

#include "common/debug.h"
#include "common/file.h"
#include "common/savefile.h"

#include "engines/metaengine.h"
#include "graphics/surface.h"

#include "qdengine/qdengine.h"
#include "qdengine/qd_fwd.h"
#include "qdengine/system/graphics/gr_dispatcher.h"
#include "qdengine/system/graphics/rle_compress.h"
#include "qdengine/qdcore/qd_setup.h"
#include "qdengine/qdcore/qd_sprite.h"
#include "qdengine/qdcore/qd_file_manager.h"
#include "qdengine/qdcore/util/2PassScale.h"
#include "qdengine/qdcore/util/Filters.h"


namespace QDEngine {

bool operator == (const qdSprite &sp1, const qdSprite &sp2) {
	if (sp1._size == sp2._size && sp1._picture_offset == sp2._picture_offset && sp1._picture_size == sp2._picture_size) {
		if (sp1.is_compressed()) {
			if (!sp2.is_compressed()) return false;

			if (sp1._rle_data && sp2._rle_data) {
				if (*sp1._rle_data == *sp2._rle_data)
					return true;
			} else
				return false;
		} else {
			if (sp1._data && sp2._data) {
				const byte *d1 = sp1._data;
				const byte *d2 = sp2._data;
				int sz = sp1._picture_size.x * sp1._picture_size.y;

				if (sp1.check_flag(qdSprite::ALPHA_FLAG) && !sp2.check_flag(qdSprite::ALPHA_FLAG)) return false;
				if (sp2.check_flag(qdSprite::ALPHA_FLAG) && !sp1.check_flag(qdSprite::ALPHA_FLAG)) return false;

				switch (sp1._format) {
				case GR_RGB565:
				case GR_ARGB1555:
					sz *= 2;
					if (sp1.check_flag(qdSprite::ALPHA_FLAG)) sz *= 2;
					break;
				case GR_RGB888:
					sz *= 3;
					break;
				case GR_ARGB8888:
					sz *= 4;
					break;
				}

				for (int i = 0; i < sz; i++)
					if (*d1++ != *d2++) return false;

				return true;
			} else
				return false;
		}
	}
	return false;
}

qdSprite::qdSprite() : _data(0),
	_rle_data(0),
	_flags(0) {
	_size = _picture_size = _picture_offset = Vect2i(0, 0);

	_format = 0;
}

qdSprite::qdSprite(int wid, int hei, int format):
	_rle_data(0),
	_flags(0) {
	_size = _picture_size = Vect2i(wid, hei);
	_picture_offset = Vect2i(0, 0);

	_format = format;

	int bytes_per_pix;
	switch (format) {
	case GR_RGB565:
		bytes_per_pix = 2;
		break;
	case GR_ARGB1555:
		bytes_per_pix = 2;
		set_flag(ALPHA_FLAG);
		break;
	case GR_RGB888:
		bytes_per_pix = 3;
		break;
	case GR_ARGB8888:
		bytes_per_pix = 4;
		set_flag(ALPHA_FLAG);
		break;
	default:
		bytes_per_pix = 1;
		break;
	};

	_data = new byte[wid * hei * bytes_per_pix];
}

qdSprite::qdSprite(const qdSprite &spr) : _data(0),
	_rle_data(0),
	_flags(0) {
	*this = spr;
}

qdSprite::~qdSprite() {
	free();
}

qdSprite &qdSprite::operator = (const qdSprite &spr) {
	if (this == &spr) return *this;

	_format = spr._format;
	_flags = spr._flags;
	_size = spr._size;
	_picture_size = spr._picture_size;
	_picture_offset = spr._picture_offset;

	delete [] _data;
	if (spr._data) {
		int ssx;
		switch (_format) {
		case GR_RGB565:
		case GR_ARGB1555:
			if (check_flag(ALPHA_FLAG))
				ssx = 4;
			else
				ssx = 2;
			break;
		case GR_RGB888:
			ssx = 3;
			break;
		case GR_ARGB8888:
			ssx = 4;
			break;
		default:
			ssx = 2;
			break;
		}

		_data = new byte[_picture_size.x * _picture_size.y * ssx];
		memcpy(_data, spr._data, _picture_size.x * _picture_size.y * ssx);
	} else
		_data = NULL;

	delete _rle_data;
	if (spr._rle_data)
		_rle_data = new RLEBuffer(*spr._rle_data);
	else
		_rle_data = NULL;

	_file = spr._file;

	return *this;
}

void qdSprite::free() {
	delete [] _data;
	delete _rle_data;

	_size = _picture_size = _picture_offset = Vect2i(0, 0);

	_format = 0;
	_data = 0;

	_rle_data = 0;

	drop_flag(ALPHA_FLAG);
}

bool qdSprite::load(const Common::Path fpath) {
	set_file(fpath);

	return load();
}

bool qdSprite::load() {
	free();

	debugC(3, kDebugLoad, "qdSprite::load(%s)", transCyrillic(_file.toString()));

	int sx, sy, flags, ssx, colors;
	byte header[18];

	Common::SeekableReadStream *fh;

	if (_file.isRelativeTo("scummvm")) {
		Common::InSaveFile *saveFile = g_engine->getSaveFileManager()->openForLoading(_file.baseName());

		ExtendedSavegameHeader saveHeader;
		if (MetaEngine::readSavegameHeader(saveFile, &saveHeader, false)) {
			_size = _picture_size = Vect2i(g_engine->_thumbSizeX, g_engine->_thumbSizeY);
			_picture_offset = Vect2i(0, 0);

			_format = GR_RGB565;

			_data = new byte[_size.x * _size.y * 2];

			// Scale image down
			float rx = static_cast<float>(g_engine->_screenW) / g_engine->_thumbSizeX;
			float ry = static_cast<float>(g_engine->_screenH) / g_engine->_thumbSizeY;

			for (int i = 0; i < _size.y; i++) {
				uint16 *dst = (uint16 *)&_data[2 * _size.x * i];
				for (int j = 0; j < _size.x; j++) {
					*dst++ = *(uint16 *)saveHeader.thumbnail->getBasePtr(rx * j, ry * i);
				}
			}
		}

		delete saveFile;

		return true;
	}

	if (!qdFileManager::instance().open_file(&fh, _file)) {
		return false;
	}

	fh->read(header, 18);

	if (header[0]) { // Length of Image ID field
		fh->seek(header[0], SEEK_CUR);
	}

	// ColorMapType. 0 - цветовой таблицы нет. 1 - есть. Остальное не соотв. стандарту.
	// Изображения с цветовой таблицей не обрабатываем.
	if (header[1]) {
		warning("qdSprite::load(): Bad file format: '%s'", transCyrillic(_file.toString()));
		return false;
	}

	// ImageType. 2 - truecolor без сжатия, 10 - truecolor со сжатием (RLE).
	if ((header[2] != 2) && (header[2] != 10)) {
		warning("qdSprite::load(): Bad file format: '%s'", transCyrillic(_file.toString()));
		return false;
	}

	sx = _picture_size.x = header[12] + (header[13] << 8);
	sy = _picture_size.y = header[14] + (header[15] << 8);

	_size = _picture_size;

	colors = header[16];
	flags = header[17];

	ssx = sx * colors / 8;

	switch (colors / 8) {
	//! Режим 16 бит не реализован
	//case 2:
	//  format_ = GR_ARGB1555;
	//  break;
	case 3:
		_format = GR_RGB888;
		break;
	case 4:
		_format = GR_ARGB8888;
		break;
	// Иначе неверный формат файла
	default: {
		warning("qdSprite::load(): Bad file format: '%s'", transCyrillic(_file.toString()));
		return false;
	}
	}

	_data = new byte[ssx * sy];

	// RLE
	if (10 == header[2]) {
		int cur = 0; // В какую ячейку считываем сейчас
		int i, j;    // Для циклов далее (теор. ускорение)
		byte info, fl, len;
		byte pixel[4];
		byte col_bytes = colors / 8;
		while (cur < ssx * sy) {
			info = fh->readByte();
			fl = (info >> 7) & 0x01;
			len = (info & 0x7F) + 1;
			// Пакет со сжатием
			if (1 == fl) {
				fh->read(&pixel, col_bytes);
				for (i = 0; i < len; i++)
					for (j = 0; j < col_bytes; j++) {
						_data[cur] = pixel[j];
						cur++;
					}
			}
			// Пакет без сжатия
			else
				for (i = 0; i < len; i++) {
					fh->read(&pixel, col_bytes);
					for (j = 0; j < col_bytes; j++) {
						_data[cur] = pixel[j];
						cur++;
					}
				}

		} // while
	}
	// Загрузка изображения без сжатия
	else
		fh->read(_data, ssx * sy);

	// Если 3 и 4 биты ImageDescriptor (fl) нули, то начало изображения - левый нижний угол
	// экрана и изображение нужно инвертировать. Иначе предполагаем, что изображение корректно.
	// Xотя не факт, что это так, но иное маловероятно + другие значения не документированы...
	if (!(flags & 0x20)) {
		int y;

		byte *str_buf = new byte[ssx];
		byte *str0, *str1;

		str0 = _data;
		str1 = _data + ssx * (sy - 1);

		for (y = 0; y < sy / 2; y++) {
			memcpy(str_buf, str0, ssx);
			memcpy(str0, str1, ssx);
			memcpy(str1, str_buf, ssx);

			str0 += ssx;
			str1 -= ssx;
		}

		delete [] str_buf;
	}

	delete fh;

	if (_format == GR_ARGB8888) {
		set_flag(ALPHA_FLAG);
		for (int i = 0; i < _picture_size.x * _picture_size.y; i++) {
			uint16 r, g, b, a;
			const uint32 min_color = 8;

			b = _data[i * 4 + 0];
			g = _data[i * 4 + 1];
			r = _data[i * 4 + 2];
			a = _data[i * 4 + 3];

			if (a >= 250 && r < min_color && g < min_color && b < min_color) {
				r = g = b = min_color;
			}

			_data[i * 4 + 0] = b * a >> 8;
			_data[i * 4 + 1] = g * a >> 8;
			_data[i * 4 + 2] = r * a >> 8;
			_data[i * 4 + 3] = 255 - a;
		}
	} else {
		for (int i = 0; i < _picture_size.x * _picture_size.y; i++) {
			const uint32 min_color = 8;

			uint32 b = _data[i * 3 + 0];
			uint32 g = _data[i * 3 + 1];
			uint32 r = _data[i * 3 + 2];

			if ((r || g || b) && (r < min_color && g < min_color && b < min_color))
				_data[i * 3 + 0] = _data[i * 3 + 1] = _data[i * 3 + 2] = min_color;
		}
	}

	return true;
}

void qdSprite::save(const Common::Path fname) {
	if (_format != GR_RGB888 && _format != GR_ARGB8888) return;

	const Common::Path out_file = !fname.empty() ? fname : _file;

	static byte header[18];

	Common::DumpFile fh;

	fh.open(out_file);

	memset(header, 0, 18);
	header[2] = 2;

	header[13] = (_picture_size.x >> 8) & 0xFF;
	header[12] = _picture_size.x & 0xFF;

	header[15] = (_picture_size.y >> 8) & 0xFF;
	header[14] = _picture_size.y & 0xFF;

	header[16] = (_format == GR_ARGB8888) ? 32 : 24;
	header[17] = 0x20;

	fh.write(header, 18);

	if (_format == GR_ARGB8888) {
		byte *buf = new byte[_picture_size.x * _picture_size.y * 4];
		byte *p = buf;
		byte *dp = _data;

		for (int i = 0; i < _picture_size.x * _picture_size.y; i++) {
			uint16 r, g, b, a;

			r = dp[0];
			g = dp[1];
			b = dp[2];
			a = 255 - dp[3];

			if (a) {
				p[0] = (r << 8) / a;
				p[1] = (g << 8) / a;
				p[2] = (b << 8) / a;
			} else
				p[0] = p[1] = p[2] = 0;

			p[3] = a;

			p += 4;
			dp += 4;
		}

		fh.write(buf, _picture_size.x * _picture_size.y * 4);
		delete [] buf;
	} else
		fh.write(_data, _picture_size.x * _picture_size.y * 3);

	fh.close();
}

bool qdSprite::compress() {
	if (is_compressed()) return false;

	switch (_format) {
	case GR_RGB565:
	case GR_ARGB1555:
		if (_data) {
			_rle_data = new RLEBuffer;

			if (!check_flag(ALPHA_FLAG)) {
				byte *p = new byte[_picture_size.x * _picture_size.y * 4];
				uint16 *dp = reinterpret_cast<uint16 *>(p);
				uint16 *sp = reinterpret_cast<uint16 *>(_data);
				for (int i = 0; i < _picture_size.x * _picture_size.y; i++) {
					*dp++ = *sp++;
					*dp++ = 0;
				}
				_rle_data->encode(_picture_size.x, _picture_size.y, p);
				delete [] p;
			} else
				_rle_data->encode(_picture_size.x, _picture_size.y, _data);

			delete [] _data;
			_data = 0;

			return true;
		}
		return false;
	case GR_RGB888:
		if (_data) {
			byte *p = new byte[_picture_size.x * _picture_size.y * 4];
			byte *ptr = p;
			byte *data_ptr = _data;
			for (int i = 0; i < _picture_size.x * _picture_size.y; i++) {
				ptr[0] = data_ptr[0];
				ptr[1] = data_ptr[1];
				ptr[2] = data_ptr[2];
				ptr[3] = 0;

				ptr += 4;
				data_ptr += 3;
			}

			_rle_data = new RLEBuffer;
			_rle_data->encode(_picture_size.x, _picture_size.y, p);

			delete [] p;
			delete [] _data;
			_data = 0;

			return true;
		}
		return false;
	case GR_ARGB8888:
		if (_data) {
			_rle_data = new RLEBuffer;
			_rle_data->encode(_picture_size.x, _picture_size.y, _data);
			set_flag(ALPHA_FLAG);

			delete [] _data;
			_data = 0;
			return true;
		}
		return false;
	}
	return false;
}

bool qdSprite::uncompress() {
	if (!is_compressed()) return false;

	switch (_format) {
	case GR_RGB565:
	case GR_ARGB1555:
		if (check_flag(ALPHA_FLAG)) {
			_data = new byte[_picture_size.x * _picture_size.y * 4];
			byte *p = _data;
			for (int i = 0; i < _picture_size.y; i++) {
				_rle_data->decode_line(i, p);
				p += _picture_size.x * sizeof(uint32);
			}
		} else {
			_data = new byte[_picture_size.x * _picture_size.y * 2];
			uint16 *p = reinterpret_cast<uint16 *>(_data);
			for (int i = 0; i < _picture_size.y; i++) {
				const uint16 *rle_p = reinterpret_cast<const uint16 *>(RLEBuffer::get_buffer(0));
				_rle_data->decode_line(i);

				for (int j = 0; j < _picture_size.x; j++) {
					*p++ = *rle_p++;
					rle_p++;
				}
			}
		}
		break;
	case GR_RGB888:
		if (!check_flag(ALPHA_FLAG)) {
			_data = new byte[_picture_size.x * _picture_size.y * 3];
			byte *p = _data;
			for (int i = 0; i < _picture_size.y; i++) {
				const byte *rle_p = reinterpret_cast<const byte *>(RLEBuffer::get_buffer(0));
				_rle_data->decode_line(i);

				for (int j = 0; j < _picture_size.x; j++) {
					p[0] = rle_p[0];
					p[1] = rle_p[1];
					p[2] = rle_p[2];
					p += 3;
					rle_p += 4;
				}
			}
		} else {
			_data = new byte[_picture_size.x * _picture_size.y * 4];
			byte *p = _data;

			for (int i = 0; i < _picture_size.y; i++) {
				_rle_data->decode_line(i, p);
				p += _picture_size.x * 4;
			}

			_format = GR_ARGB8888;
		}
		break;
	case GR_ARGB8888:
		if (!check_flag(ALPHA_FLAG)) {
			_data = new byte[_picture_size.x * _picture_size.y * 3];
			byte *p = _data;
			for (int i = 0; i < _picture_size.y; i++) {
				const byte *rle_p = reinterpret_cast<const byte *>(RLEBuffer::get_buffer(0));
				_rle_data->decode_line(i);

				for (int j = 0; j < _picture_size.x; j++) {
					p[0] = rle_p[0];
					p[1] = rle_p[1];
					p[2] = rle_p[2];
					p += 3;
					rle_p += 4;
				}
			}
			_format = GR_RGB888;
		} else {
			_data = new byte[_picture_size.x * _picture_size.y * 4];
			byte *p = _data;

			for (int i = 0; i < _picture_size.y; i++) {
				_rle_data->decode_line(i, p);
				p += _picture_size.x * 4;
			}
		}
		break;
	}

	delete _rle_data;
	_rle_data = 0;

	return true;
}

void qdSprite::redraw(int x, int y, int z, int mode) const {
	debugC(2, kDebugGraphics, "qdSprite::redraw(%d, %d, z=%d, mode=%d)", x, y, z, mode);

	int xx = x - size_x() / 2;
	int yy = y - size_y() / 2;

	if (mode & GR_FLIP_HORIZONTAL)
		xx += _size.x - _picture_offset.x - _picture_size.x;
	else
		xx += _picture_offset.x;

	if (mode & GR_FLIP_VERTICAL)
		yy += _size.y - _picture_offset.y - _picture_size.y;
	else
		yy += _picture_offset.y;

#ifdef _GR_ENABLE_ZBUFFER
	if (!is_compressed()) {
		if (!_data) return;
		if (check_flag(ALPHA_FLAG))
			grDispatcher::instance()->putSpr_a_z(xx, yy, z, _picture_size.x, _picture_size.y, _data, mode);
		else
			grDispatcher::instance()->putSpr_z(xx, yy, z, _picture_size.x, _picture_size.y, _data, mode);
	} else
		grDispatcher::instance()->putSpr_rle_z(xx, yy, z, _picture_size.x, _picture_size.y, _rle_data, mode, check_flag(ALPHA_FLAG));
#else
	if (!is_compressed()) {
		if (!_data) return;
		if (check_flag(ALPHA_FLAG))
			grDispatcher::instance()->putSpr_a(xx, yy, _picture_size.x, _picture_size.y, _data, mode);
		else
			grDispatcher::instance()->putSpr(xx, yy, _picture_size.x, _picture_size.y, _data, mode, _format);
	} else
		grDispatcher::instance()->putSpr_rle(xx, yy, _picture_size.x, _picture_size.y, _rle_data, mode, check_flag(ALPHA_FLAG));
#endif

	if (debugChannelSet(1, kDebugGraphics))
		grDispatcher::instance()->rectangle(xx, yy, _picture_size.x, _picture_size.y, 0, 0, GR_OUTLINED);
}

void qdSprite::redraw_rot(int x, int y, int z, float angle, int mode) const {
	int xx = x;
	int yy = y;

	Vect2i delta = _picture_offset + _picture_size / 2 - _size / 2;

	if (mode & GR_FLIP_HORIZONTAL)
		delta.x = -delta.x;
	if (mode & GR_FLIP_VERTICAL)
		delta.y = -delta.y;

	if (delta.x || delta.y) {
		xx += round(float(delta.x) * cosf(angle) - float(delta.y) * sinf(angle));
		yy += round(float(delta.x) * sinf(angle) + float(delta.y) * cosf(angle));
	}

	xx -= _picture_size.x / 2;
	yy -= _picture_size.y / 2;

	if (!is_compressed()) {
		if (!_data) return;
		grDispatcher::instance()->putSpr_rot(Vect2i(xx, yy), _picture_size, _data, check_flag(ALPHA_FLAG), mode, angle);
	} else
		grDispatcher::instance()->putSpr_rle_rot(Vect2i(xx, yy), _picture_size, _rle_data, check_flag(ALPHA_FLAG), mode, angle);
}

void qdSprite::redraw_rot(int x, int y, int z, float angle, const Vect2f &scale, int mode) const {
	int xx = x;
	int yy = y;

	Vect2i delta = _picture_offset + _picture_size / 2 - _size / 2;

	if (mode & GR_FLIP_HORIZONTAL)
		delta.x = -delta.x;
	if (mode & GR_FLIP_VERTICAL)
		delta.y = -delta.y;

	delta.x = round(float(delta.x) * scale.x);
	delta.y = round(float(delta.y) * scale.y);

	if (delta.x || delta.y) {
		xx += round(float(delta.x) * cosf(angle) - float(delta.y) * sinf(angle));
		yy += round(float(delta.x) * sinf(angle) + float(delta.y) * cosf(angle));
	}

	xx -= round(float(_picture_size.x / 2) * scale.x);
	yy -= round(float(_picture_size.y / 2) * scale.y);

	if (!is_compressed()) {
		if (!_data) return;
		grDispatcher::instance()->putSpr_rot(Vect2i(xx, yy), _picture_size, _data, check_flag(ALPHA_FLAG), mode, angle, scale);
	} else
		grDispatcher::instance()->putSpr_rle_rot(Vect2i(xx, yy), _picture_size, _rle_data, check_flag(ALPHA_FLAG), mode, angle, scale);
}

void qdSprite::redraw(int x, int y, int z, float scale, int mode) const {
	int xx = x - round(float(size_x()) * scale) / 2;
	int yy = y - round(float(size_y()) * scale) / 2;

	if (mode & GR_FLIP_HORIZONTAL)
		xx += round(float(_size.x - _picture_offset.x - _picture_size.x) * scale);
	else
		xx += round(float(_picture_offset.x) * scale);

	if (mode & GR_FLIP_VERTICAL)
		yy += round(float(_size.y - _picture_offset.y - _picture_size.y) * scale);
	else
		yy += round(float(_picture_offset.y) * scale);

#ifdef _GR_ENABLE_ZBUFFER
	if (!is_compressed()) {
		if (!_data) return;
		if (check_flag(ALPHA_FLAG))
			grDispatcher::instance()->putSpr_a_z(xx, yy, z, _picture_size.x, _picture_size.y, _data, mode, scale);
		else
			grDispatcher::instance()->putSpr_z(xx, yy, z, _picture_size.x, _picture_size.y, _data, mode, scale);
	} else
		grDispatcher::instance()->putSpr_rle_z(xx, yy, z, _picture_size.x, _picture_size.y, _rle_data, mode, scale, check_flag(ALPHA_FLAG));
#else
	if (!is_compressed()) {
		if (!_data) return;
		if (check_flag(ALPHA_FLAG))
			grDispatcher::instance()->putSpr_a(xx, yy, _picture_size.x, _picture_size.y, _data, mode, scale);
		else
			grDispatcher::instance()->putSpr(xx, yy, _picture_size.x, _picture_size.y, _data, mode, scale);
	} else
		grDispatcher::instance()->putSpr_rle(xx, yy, _picture_size.x, _picture_size.y, _rle_data, mode, scale, check_flag(ALPHA_FLAG));
#endif
}

void qdSprite::draw_mask(int x, int y, int z, uint32 mask_color, int mask_alpha, int mode) const {
	int xx = x - size_x() / 2;
	int yy = y - size_y() / 2;

	if (mode & GR_FLIP_HORIZONTAL)
		xx += _size.x - _picture_offset.x - _picture_size.x;
	else
		xx += _picture_offset.x;

	if (mode & GR_FLIP_VERTICAL)
		yy += _size.y - _picture_offset.y - _picture_size.y;
	else
		yy += _picture_offset.y;

	if (!is_compressed()) {
		if (!_data) return;
		if (check_flag(ALPHA_FLAG))
			grDispatcher::instance()->putSprMask_a(xx, yy, _picture_size.x, _picture_size.y, _data, mask_color, mask_alpha, mode);
		else
			grDispatcher::instance()->putSprMask(xx, yy, _picture_size.x, _picture_size.y, _data, mask_color, mask_alpha, mode);
	} else
		grDispatcher::instance()->putSprMask_rle(xx, yy, _picture_size.x, _picture_size.y, _rle_data, mask_color, mask_alpha, mode, check_flag(ALPHA_FLAG));
}

void qdSprite::draw_mask(int x, int y, int z, uint32 mask_color, int mask_alpha, float scale, int mode) const {
	int xx = x - round(float(size_x()) * scale) / 2;
	int yy = y - round(float(size_y()) * scale) / 2;

	if (mode & GR_FLIP_HORIZONTAL)
		xx += round(float(_size.x - _picture_offset.x - _picture_size.x) * scale);
	else
		xx += round(float(_picture_offset.x) * scale);

	if (mode & GR_FLIP_VERTICAL)
		yy += round(float(_size.y - _picture_offset.y - _picture_size.y) * scale);
	else
		yy += round(float(_picture_offset.y) * scale);

	if (!is_compressed()) {
		if (!_data) return;
		if (check_flag(ALPHA_FLAG))
			grDispatcher::instance()->putSprMask_a(xx, yy, _picture_size.x, _picture_size.y, _data, mask_color, mask_alpha, mode, scale);
		else
			grDispatcher::instance()->putSprMask(xx, yy, _picture_size.x, _picture_size.y, _data, mask_color, mask_alpha, mode, scale);
	} else
		grDispatcher::instance()->putSprMask_rle(xx, yy, _picture_size.x, _picture_size.y, _rle_data, mask_color, mask_alpha, mode, scale, check_flag(ALPHA_FLAG));
}

void qdSprite::draw_mask_rot(int x, int y, int z, float angle, uint32 mask_color, int mask_alpha, int mode) const {
	int xx = x;
	int yy = y;

	Vect2i delta = _picture_offset + _picture_size / 2 - _size / 2;

	if (mode & GR_FLIP_HORIZONTAL)
		delta.x = -delta.x;
	if (mode & GR_FLIP_VERTICAL)
		delta.y = -delta.y;

	if (delta.x || delta.y) {
		xx += round(float(delta.x) * cosf(angle) - float(delta.y) * sinf(angle));
		yy += round(float(delta.x) * sinf(angle) + float(delta.y) * cosf(angle));
	}

	xx -= _picture_size.x / 2;
	yy -= _picture_size.y / 2;

	if (!is_compressed()) {
		if (!_data) return;
		grDispatcher::instance()->putSprMask_rot(Vect2i(xx, yy), _picture_size, _data, check_flag(ALPHA_FLAG), mask_color, mask_alpha, mode, angle);
	} else
		grDispatcher::instance()->putSprMask_rle_rot(Vect2i(xx, yy), _picture_size, _rle_data, check_flag(ALPHA_FLAG), mask_color, mask_alpha, mode, angle);
}

void qdSprite::draw_mask_rot(int x, int y, int z, float angle, uint32 mask_color, int mask_alpha, const Vect2f &scale, int mode) const {
	int xx = x;
	int yy = y;

	Vect2i delta = _picture_offset + _picture_size / 2 - _size / 2;

	if (mode & GR_FLIP_HORIZONTAL)
		delta.x = -delta.x;
	if (mode & GR_FLIP_VERTICAL)
		delta.y = -delta.y;

	delta.x = round(float(delta.x) * scale.x);
	delta.y = round(float(delta.y) * scale.y);

	if (delta.x || delta.y) {
		xx += round(float(delta.x) * cosf(angle) - float(delta.y) * sinf(angle));
		yy += round(float(delta.x) * sinf(angle) + float(delta.y) * cosf(angle));
	}

	xx -= round(float(_picture_size.x / 2) * scale.x);
	yy -= round(float(_picture_size.y / 2) * scale.y);

	if (!is_compressed()) {
		if (!_data) return;
		grDispatcher::instance()->putSprMask_rot(Vect2i(xx, yy), _picture_size, _data, check_flag(ALPHA_FLAG), mask_color, mask_alpha, mode, angle, scale);
	} else
		grDispatcher::instance()->putSprMask_rle_rot(Vect2i(xx, yy), _picture_size, _rle_data, check_flag(ALPHA_FLAG), mask_color, mask_alpha, mode, angle, scale);
}

void qdSprite::draw_contour(int x, int y, uint32 color, int mode) const {
	int xx = x - size_x() / 2;
	int yy = y - size_y() / 2;

	if (mode & GR_FLIP_HORIZONTAL)
		xx += _size.x - _picture_offset.x - _picture_size.x;
	else
		xx += _picture_offset.x;

	if (mode & GR_FLIP_VERTICAL)
		yy += _size.y - _picture_offset.y - _picture_size.y;
	else
		yy += _picture_offset.y;

	if (is_compressed()) {
		grDispatcher::instance()->drawSprContour(xx, yy, _picture_size.x, _picture_size.y, _rle_data, color, mode, check_flag(ALPHA_FLAG));
	} else {
		if (!_data) return;
		if (check_flag(ALPHA_FLAG))
			grDispatcher::instance()->drawSprContour_a(xx, yy, _picture_size.x, _picture_size.y, _data, color, mode);
		else
			grDispatcher::instance()->drawSprContour(xx, yy, _picture_size.x, _picture_size.y, _data, color, mode);
	}
}

void qdSprite::draw_contour(int x, int y, uint32 color, float scale, int mode) const {
	int xx = x - round(float(size_x()) * scale) / 2;
	int yy = y - round(float(size_y()) * scale) / 2;

	if (mode & GR_FLIP_HORIZONTAL)
		xx += round(float(_size.x - _picture_offset.x - _picture_size.x) * scale);
	else
		xx += round(float(_picture_offset.x) * scale);

	if (mode & GR_FLIP_VERTICAL)
		yy += round(float(_size.y - _picture_offset.y - _picture_size.y) * scale);
	else
		yy += round(float(_picture_offset.y) * scale);

	if (!is_compressed()) {
		if (check_flag(ALPHA_FLAG))
			grDispatcher::instance()->drawSprContour_a(xx, yy, _picture_size.x, _picture_size.y, _data, color, mode, scale);
		else
			grDispatcher::instance()->drawSprContour(xx, yy, _picture_size.x, _picture_size.y, _data, color, mode, scale);
	} else
		grDispatcher::instance()->drawSprContour(xx, yy, _picture_size.x, _picture_size.y, _rle_data, color, mode, scale, check_flag(ALPHA_FLAG));
}

bool qdSprite::hit(int x, int y) const {
	x += _size.x / 2;
	y += _size.y / 2;

	if (x < 0 || y < 0 || x >= _size.x || y >= _size.y) return false;

	if (x >= _picture_offset.x && x < _picture_offset.x + _picture_size.x && y >= _picture_offset.y && y < _picture_offset.y + _picture_size.y) {
		x -= _picture_offset.x;
		y -= _picture_offset.y;

		if (!is_compressed()) {
			if (!_data) return false;

			switch (_format) {
			case GR_RGB565:
			case GR_ARGB1555:
				if (check_flag(ALPHA_FLAG)) {
					if (reinterpret_cast<uint16 * >(_data)[(x + y * _picture_size.x) * 2 + 1] < 240) return true;
				} else {
					if (reinterpret_cast<uint16 * >(_data)[x + y * _picture_size.x]) return true;
				}
				break;
			case GR_RGB888:
				if (_data[(x + y * _picture_size.x) * 3] || _data[(x + y * _picture_size.x) * 3 + 1] || _data[(x + y * _picture_size.x) * 3 + 2]) return true;
				break;
			case GR_ARGB8888:
				if (_data[(x + y * _picture_size.x) * 4 + 3] < 240) return true;
				break;
			}
		} else {
			uint32 pixel;
			_rle_data->decode_pixel(x, y, pixel);
			if (check_flag(ALPHA_FLAG)) {
				switch (_format) {
				case GR_RGB565:
				case GR_ARGB1555:
					if (reinterpret_cast<uint16 * >(&pixel)[1] < 240) return true;
					return false;
				case GR_RGB888:
				case GR_ARGB8888:
					if (reinterpret_cast<byte * >(&pixel)[3] < 240) return true;
					return false;
				}
			} else {
				if (pixel)
					return true;
				else
					return false;
			}
		}
	}

	return false;
}

bool qdSprite::hit(int x, int y, float scale) const {
	x = round(float(x) / scale);
	y = round(float(y) / scale);

	return hit(x, y);
}

bool qdSprite::put_pixel(int x, int y, byte r, byte g, byte b) {
	x -= _picture_offset.x;
	y -= _picture_offset.y;

	if ((x < 0) || (x >= _size.x) || (y < 0) || (y >= _size.y))
		return false;

	int bytes_per_pix;
	uint16 word;

	switch (_format) {
	case GR_RGB565:
		bytes_per_pix = 2;
		word = grDispatcher::make_rgb565u(r, g, b);
		_data[bytes_per_pix * (y * _size.x + x) + 0] = static_cast<byte>(word & 0x00FF);
		_data[bytes_per_pix * (y * _size.x + x) + 1] = static_cast<byte>(word >> 8 & 0x00FF);
		break;
	case GR_RGB888:
		bytes_per_pix = 3;
		_data[bytes_per_pix * (y * _size.x + x) + 0] = b;
		_data[bytes_per_pix * (y * _size.x + x) + 1] = g;
		_data[bytes_per_pix * (y * _size.x + x) + 2] = r;
		break;
	default:
		return false;
		break;
	}

	return true;
}

void qdSprite::qda_load(Common::SeekableReadStream *fh, int version) {
	free();

	static char str[256];

	_size.x = fh->readSint32LE();
	_size.y = fh->readSint32LE();
	_picture_size.x = fh->readSint32LE();
	_picture_size.y = fh->readSint32LE();
	_picture_offset.x = fh->readSint32LE();
	_picture_offset.y = fh->readSint32LE();
	_format = fh->readSint32LE();
	int32 len = fh->readSint32LE();

	int32 al_flag, compress_flag;

	str[len] = 0;
	fh->read(str, len);
	set_file(str);

	if (version >= 101) {
		_flags = fh->readSint32LE();
		al_flag = fh->readSint32LE();
		compress_flag = fh->readSint32LE();
	} else {
		_flags = 0;
		compress_flag = 0;
		al_flag = fh->readSint32LE();
	}

	if (!compress_flag) {
		if (version < 102) {
			switch (_format) {
			case GR_RGB565:
			case GR_ARGB1555:
				if (!al_flag) {
					_data = new byte[_picture_size.x * _picture_size.y * 2];
				} else {
					warning("qdSprite::qda_load(): al_flag is set, check the sprite picture"); // TODO)
					_data = new byte[_picture_size.x * _picture_size.y * 4];
				}

				fh->read(_data, _picture_size.x * _picture_size.y * 2);
				break;
			case GR_RGB888:
				if (!al_flag) {
					_data = new byte[_picture_size.x * _picture_size.y * 3];
				} else {
					warning("qdSprite::qda_load(): al_flag is set, check the sprite picture"); // TODO
					_data = new byte[_picture_size.x * _picture_size.y * 4];
				}

				fh->read(_data, _picture_size.x * _picture_size.y * 3);
				break;
			case GR_ARGB8888:
				_data = new byte[_picture_size.x * _picture_size.y * 4];
				fh->read(_data, _picture_size.x * _picture_size.y * 4);
				break;
			}
			if (al_flag) {
				byte *alpha_data = new byte[_picture_size.x * _picture_size.y];
				fh->read(alpha_data, _picture_size.x * _picture_size.y);

				switch (_format) {
				case GR_RGB565:
				case GR_ARGB1555: {
					byte *dp = _data + _picture_size.x * _picture_size.y * 4 - 4;
					byte *sp = _data + _picture_size.x * _picture_size.y * 2 - 2;
					byte *ap = alpha_data + _picture_size.x * _picture_size.y - 1;

					for (int i = 0; i < _picture_size.x * _picture_size.y; i++) {
						dp[0] = sp[0];
						dp[1] = sp[1];
						dp[2] = 0;
						dp[3] = *ap--;

						dp -= 4;
						sp -= 2;
					}
				}
				break;
				case GR_RGB888: {
					byte *dp = _data + _picture_size.x * _picture_size.y * 4 - 4;
					byte *sp = _data + _picture_size.x * _picture_size.y * 3 - 3;
					byte *ap = alpha_data + _picture_size.x * _picture_size.y - 1;

					for (int i = 0; i < _picture_size.x * _picture_size.y; i++) {
						dp[0] = sp[0];
						dp[1] = sp[1];
						dp[2] = sp[2];
						dp[3] = *ap--;

						dp -= 4;
						sp -= 3;
					}

					_format = GR_ARGB8888;
				}
				break;
				}

				set_flag(ALPHA_FLAG);
				delete [] alpha_data;
			}
		} else {
			switch (_format) {
			case GR_RGB565:
			case GR_ARGB1555:
				if (check_flag(ALPHA_FLAG)) {
					_data = new byte[_picture_size.x * _picture_size.y * 4];
					fh->read(_data, _picture_size.x * _picture_size.y * 4);
				} else {
					_data = new byte[_picture_size.x * _picture_size.y * 2];
					fh->read(_data, _picture_size.x * _picture_size.y * 2);
				}
				break;
			case GR_RGB888:
				_data = new byte[_picture_size.x * _picture_size.y * 3];
				fh->read(_data, _picture_size.x * _picture_size.y * 3);
				break;
			case GR_ARGB8888:
				_data = new byte[_picture_size.x * _picture_size.y * 4];
				fh->read(_data, _picture_size.x * _picture_size.y * 4);
				break;
			}
		}
	} else {
		_rle_data = new RLEBuffer;
		_rle_data->load(fh);
	}
}

bool qdSprite::crop() {
	int left, top, right, bottom;
	if (!get_edges_width(left, top, right, bottom)) return false;
	return crop(left, top, right, bottom);
}

bool qdSprite::crop(int left, int top, int right, int bottom, bool store_offsets) {
	int sx = _picture_size.x - left - right;
	int sy = _picture_size.y - top - bottom;

	if (sx == _picture_size.x && sy == _picture_size.y) return true;

	int psz = 1;
	switch (_format) {
	case GR_RGB565:
	case GR_ARGB1555:
		psz = (check_flag(ALPHA_FLAG)) ? 4 : 2;
		break;
	case GR_RGB888:
		psz = 3;
		break;
	case GR_ARGB8888:
		psz = 4;
		break;
	}

	int idx1 = 0;
	int idx = left * psz + top * _picture_size.x * psz;

	byte *data_new = new byte[sx * sy * psz];
	for (int y = 0; y < sy; y++) {
		memcpy(data_new + idx1, _data + idx, sx * psz);
		idx += _picture_size.x * psz;
		idx1 += sx * psz;
	}
	delete [] _data;
	_data = data_new;

	if (store_offsets) {
		_picture_offset.x += left;
		_picture_offset.y += top;
	} else {
		_size.x = _picture_offset.x + sx;
		_size.y = _picture_offset.x + sy;
	}

	_picture_size.x = sx;
	_picture_size.y = sy;

	return true;
}

bool qdSprite::undo_crop() {
	if (!_data) return false;

	if (_picture_size == _size) return false;

	int psx = 1;
	if (_format == GR_RGB565 || _format == GR_ARGB1555)
		psx = (check_flag(ALPHA_FLAG)) ? 4 : 2;
	if (_format == GR_RGB888) {
		psx = 3;
		drop_flag(ALPHA_FLAG);
	}
	if (_format == GR_ARGB8888)
		psx = 4;

	byte *new_data = new byte[_size.x * _size.y * psx];
	memset(new_data, 0, _size.x * _size.y * psx);

	if (check_flag(ALPHA_FLAG)) {
		byte *p = (_format == GR_ARGB8888) ? new_data + 3 : new_data + 2;
		for (int i = 0; i < _size.x * _size.y; i++) {
			*p = 255;
			p += 4;
		}
	}

	byte *dp = _data;
	byte *p = new_data + (_picture_offset.x + _picture_offset.y * _size.x) * psx;

	for (int i = 0; i < _picture_size.y; i++) {
		memcpy(p, dp, _picture_size.x * psx);

		p += _size.x * psx;
		dp += _picture_size.x * psx;
	}

	delete [] _data;
	_data = new_data;

	_picture_size = _size;
	_picture_offset = Vect2i(0, 0);

	return true;
}

bool qdSprite::get_edges_width(int &left, int &top, int &right, int &bottom) {
	if (!_data) return false;

	left = _picture_size.x - 1;
	top = _picture_size.y - 1;

	right = left;
	bottom = top;

	if (_format == GR_ARGB1555 || _format == GR_RGB565) {
		if (check_flag(ALPHA_FLAG)) {
			int idx = 0;
			uint16 *data_ptr = reinterpret_cast<uint16 *>(_data);
			for (int y = 0; y < _picture_size.y; y++) {
				int x = 0;
				while (x < _picture_size.x && data_ptr[(idx + x) * 2 + 1] == 255) x++;
				if (x < left) left = x;
				idx += _picture_size.x - 1;

				x = 0;
				while (x < _picture_size.x && data_ptr[(idx - x) * 2 + 1] == 255) x++;
				if (x < right) right = x;
				idx++;
			}

			idx = 0;
			for (int x = 0; x < _picture_size.x; x++) {
				int y = 0;
				while (y < _picture_size.y && data_ptr[(idx + y * _picture_size.x) * 2 + 1] == 255) y++;
				if (y < top) top = y;

				y = 0;
				while (y < _picture_size.y && data_ptr[(idx - y * _picture_size.x + (_picture_size.y - 1) * _picture_size.x) * 2 + 1] == 255) y++;
				if (y < bottom) bottom = y;
				idx++;
			}
		} else {
			int idx = 0;
			uint16 *data_ptr = reinterpret_cast<uint16 *>(_data);
			for (int y = 0; y < _picture_size.y; y++) {
				int x = 0;
				while (x < _picture_size.x && !data_ptr[idx + x]) x++;
				if (x < left) left = x;
				idx += _picture_size.x - 1;

				x = 0;
				while (x < _picture_size.x && !data_ptr[idx - x]) x++;
				if (x < right) right = x;
				idx++;
			}

			idx = 0;
			for (int x = 0; x < _picture_size.x; x++) {
				int y = 0;
				while (y < _picture_size.y && !data_ptr[idx + y * _picture_size.x]) y++;
				if (y < top) top = y;

				y = 0;
				while (y < _picture_size.y && !data_ptr[idx - y * _picture_size.x + (_picture_size.y - 1) * _picture_size.x]) y++;
				if (y < bottom) bottom = y;
				idx++;
			}
		}
	}
	if (_format == GR_RGB888) {
		int idx = 0;
		for (int y = 0; y < _picture_size.y; y++) {
			int x = 0;
			while (x < _picture_size.x && !(_data[idx + x * 3 + 0] + _data[idx + x * 3 + 1] + _data[idx + x * 3 + 2])) x++;
			if (x < left) left = x;
			idx += (_picture_size.x - 1) * 3;

			x = 0;
			while (x < _picture_size.x && !(_data[idx - x * 3 + 0] + _data[idx - x * 3 + 1] + _data[idx - x * 3 + 2])) x++;
			if (x < right) right = x;
			idx += 3;
		}

		idx = 0;
		for (int x = 0; x < _picture_size.x; x++) {
			int y = 0;
			while (y < _picture_size.y && !(_data[idx + y * _picture_size.x * 3 + 0] + _data[idx + y * _picture_size.x * 3 + 1] + _data[idx + y * _picture_size.x * 3 + 2])) y++;
			if (y < top) top = y;

			y = 0;
			while (y < _picture_size.y && !(_data[idx - y * _picture_size.x * 3 + (_picture_size.y - 1) * _picture_size.x * 3 + 0] + _data[idx - y * _picture_size.x * 3 + (_picture_size.y - 1) * _picture_size.x * 3 + 1] + _data[idx - y * _picture_size.x * 3 + (_picture_size.y - 1) * _picture_size.x * 3 + 2])) y++;
			if (y < bottom) bottom = y;
			idx += 3;
		}
	}
	if (_format == GR_ARGB8888) {
		int idx = 0;
		for (int y = 0; y < _picture_size.y; y++) {
			int x = 0;
			while (x < _picture_size.x && _data[idx + x * 4 + 3] == 255) x++;
			if (x < left) left = x;
			idx += (_picture_size.x - 1) * 4;

			x = 0;
			while (x < _picture_size.x && _data[idx - x * 4 + 3] == 255) x++;
			if (x < right) right = x;
			idx += 4;
		}

		idx = 0;
		for (int x = 0; x < _picture_size.x; x++) {
			int y = 0;
			while (y < _picture_size.y && _data[idx + y * _picture_size.x * 4 + 3] == 255) y++;
			if (y < top) top = y;

			y = 0;
			while (y < _picture_size.y && _data[idx - y * _picture_size.x * 4 + (_picture_size.y - 1) * _picture_size.x * 4 + 3] == 255) y++;
			if (y < bottom) bottom = y;
			idx += 4;
		}
	}

	if (left + right >= _size.x) {
		left = 0;
		right = _size.x - 1;
	}
	if (top + bottom >= _size.y) {
		top = 0;
		bottom = _size.y - 1;
	}

	return true;
}

uint32 qdSprite::data_size() const {
	if (!is_compressed()) {
		uint32 sz = _picture_size.x * _picture_size.y;

		switch (_format) {
		case GR_RGB565:
		case GR_ARGB1555:
			sz *= 2;
			if (check_flag(ALPHA_FLAG)) sz *= 2;
			break;
		case GR_RGB888:
			sz *= 3;
			break;
		case GR_ARGB8888:
			sz *= 4;
			break;
		}
		return sz;
	} else
		return _rle_data->size();
}

bool qdSprite::scale(float coeff_x, float coeff_y) {
	static scl::C2PassScale<scl::CBilinearFilter> scale_engine;
	static Std::vector<byte> temp_buffer(300 * 400 * 4, 0);

	bool compress_flag = false;

	if (is_compressed()) {
		uncompress();
		compress_flag = true;
	}

	undo_crop();

	int sx = round(float(_picture_size.x) * coeff_x);
	int sy = round(float(_picture_size.y) * coeff_y);

	byte *src_data = _data;

	if (_format == GR_RGB888) {
		if ((int)temp_buffer.size() < _picture_size.x * _picture_size.y * 4)
			temp_buffer.resize(_picture_size.x * _picture_size.y * 4);

		src_data = &*temp_buffer.begin();
		memset(src_data, 0, _picture_size.x * _picture_size.y * 4);

		byte *p = src_data;
		byte *dp = _data;

		for (int i = 0; i < _picture_size.x * _picture_size.y; i++) {
			p[0] = dp[0];
			p[1] = dp[1];
			p[2] = dp[2];
			p[3] = 0;

			p += 4;
			dp += 3;
		}
	}

	byte *dest_data = new byte[sx * sy * 4];

	scale_engine.scale(reinterpret_cast<uint32 *>(src_data), _picture_size.x, _picture_size.y, reinterpret_cast<uint32 *>(dest_data), sx, sy);

	delete [] _data;

	if (_format == GR_RGB888) {
		_data = new byte[sx * sy * 3];

		byte *p = dest_data;
		byte *dp = _data;

		for (int i = 0; i < sx * sy; i++) {
			dp[0] = p[0];
			dp[1] = p[1];
			dp[2] = p[2];

			p += 4;
			dp += 3;
		}

		delete [] dest_data;
	} else {
		_data = dest_data;
	}

	_picture_size.x = sx;
	_picture_size.y = sy;

	_size.x = round(float(_size.x) * coeff_x);
	_size.y = round(float(_size.y) * coeff_y);

	_picture_offset.x = round(float(_picture_offset.x) * coeff_x);
	_picture_offset.y = round(float(_picture_offset.y) * coeff_y);

	crop();

	if (compress_flag)
		if (!compress()) return false;

	return true;
}

Vect2i qdSprite::remove_edges() {
	int left, top, right, bottom;
	get_edges_width(left, top, right, bottom);
	crop(left, top, right, bottom, false);

	return Vect2i(left, top);
}

grScreenRegion qdSprite::screen_region(int mode, float scale) const {
	int x, y;

	if (mode & GR_FLIP_HORIZONTAL)
		x = round(float(_size.x / 2 - _picture_offset.x - _picture_size.x / 2) * scale);
	else
		x = round(float(_picture_offset.x + _picture_size.x / 2 - _size.x / 2) * scale);

	if (mode & GR_FLIP_VERTICAL)
		y = round(float(_size.y / 2 - _picture_offset.y - _picture_size.y / 2) * scale);
	else
		y = round(float(_picture_offset.y + _picture_size.y / 2 - _size.y / 2) * scale);

	int sx = round(float(_picture_size.x) * scale) + 4;
	int sy = round(float(_picture_size.y) * scale) + 4;

	return grScreenRegion(x, y, sx, sy);
}
} // namespace QDEngine
