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

#include "common/fs.h"
#include "awe/pak.h"
#include "awe/resource_nth.h"
#include "awe/script.h"
#include "awe/awe.h"

namespace Awe {

static char *loadTextFile(Common::File &f, const int size) {
	char *buf = (char *)malloc(size + 1);
	if (buf) {
		const int count = f.read(buf, size);
		if (count != size) {
			warning("Failed to read %d bytes (%d expected)", count, size);
			free(buf);
			return nullptr;
		}
		buf[count] = 0;
	}
	return buf;
}

struct Resource15th : ResourceNth {
	Pak _pak;
	char _dataPath[MAXPATHLEN];
	char _menuPath[MAXPATHLEN];
	char *_textBuf;
	const char *_stringsTable[157];
	bool _hasRemasteredMusic;

	Resource15th() {
		Common::strcpy_s(_dataPath, "Music/AW/RmSnd");

		_hasRemasteredMusic = Common::FSNode(_dataPath).isDirectory();
		Common::strcpy_s(_dataPath, "Data");
		Common::strcpy_s(_menuPath, "Menu");
		_textBuf = nullptr;
		memset(_stringsTable, 0, sizeof(_stringsTable));
	}

	virtual ~Resource15th() {
		free(_textBuf);
	}

	virtual bool init() {
		_pak.open(_dataPath);
		_pak.readEntries();
		return _pak._entriesCount != 0;
	}

	virtual uint8 *load(const char *name) {
		uint8 *buf = nullptr;
		const PakEntry *e = _pak.find(name);
		if (e) {
			buf = (uint8 *)malloc(e->size);
			if (buf) {
				uint32 size;
				_pak.loadData(e, buf, &size);
			}
		} else {
			warning("Unable to load '%s'", name);
		}
		return buf;
	}

	virtual uint8 *loadBmp(int num) {
		char name[32];
		if (num >= 3000) {
			snprintf(name, sizeof(name), "e%04d.bmp", num);
		} else {
			snprintf(name, sizeof(name), "file%03d.bmp", num);
		}
		return load(name);
	}

	virtual uint8 *loadDat(int num, uint8 *dst, uint32 *size) {
		char name[32];
		snprintf(name, sizeof(name), "file%03d.dat", num);
		const PakEntry *e = _pak.find(name);
		if (e) {
			_pak.loadData(e, dst, size);
			return dst;
		} else {
			warning("Unable to load '%s'", name);
		}
		return nullptr;
	}

	virtual uint8 *loadWav(int num, uint8 *dst, uint32 *size) {
		char name[32];
		const PakEntry *e = nullptr;
		if (Script::_useRemasteredAudio) {
			snprintf(name, sizeof(name), "rmsnd/file%03d.wav", num);
			e = _pak.find(name);
		}
		if (!e) {
			snprintf(name, sizeof(name), "file%03db.wav", num);
			e = _pak.find(name);
			if (!e) {
				snprintf(name, sizeof(name), "file%03d.wav", num);
				e = _pak.find(name);
			}
		}
		if (e) {
			uint8 *p = (uint8 *)malloc(e->size);
			if (p) {
				_pak.loadData(e, p, size);
				*size = 0;
				return p;
			}
			warning("Failed to allocate %d bytes", e->size);
		} else {
			warning("Unable to load '%s'", name);
		}
		return nullptr;
	}

	void loadStrings(Language lang) {
		if (!_textBuf) {
			const char *name = nullptr;
			switch (lang) {
			case Common::FR_FRA:
				name = "Francais.Txt";
				break;
			case Common::EN_ANY:
				name = "English.Txt";
				break;
			case Common::DE_DEU:
				name = "German.Txt";
				break;
			case Common::ES_ESP:
				name = "Espanol.txt";
				break;
			case Common::IT_ITA:
				name = "Italian.Txt";
				break;
			default:
				return;
			}
			char path[MAXPATHLEN];
			snprintf(path, sizeof(path), "%s/lang_%s", _menuPath, name);
			Common::File f;
			if (f.open(path)) {
				const int size = f.size();
				_textBuf = loadTextFile(f, size);
				if (_textBuf) {
					char *p = _textBuf;
					while (true) {
						char *end = strchr(p, '\r');
						if (!end) {
							break;
						}
						*end++ = 0;
						if (*end == '\n') {
							*end++ = 0;
						}
						const int len = end - p;
						int strNum = -1;
						if (len > 3 && sscanf(p, "%03d", &strNum) == 1) {
							p += 3;
							while (*p == ' ' || *p == '\t') {
								++p;
							}
							if (strNum > 0 && (uint32)strNum < ARRAYSIZE(_stringsTable)) {
								_stringsTable[strNum] = p;
							}
						}
						p = end;
					}
				}
			}
		}
	}

	virtual const char *getString(Language lang, int num) {
		loadStrings(lang);
		if ((uint32)num < ARRAYSIZE(_stringsTable)) {
			return _stringsTable[num];
		}
		return nullptr;
	}

	virtual const char *getMusicName(int num) {
		const char *path = nullptr;
		switch (num) {
		case 7:
			if (_hasRemasteredMusic && Script::_useRemasteredAudio) {
				path = "Music/AW/RmSnd/Intro2004.wav";
			} else {
				path = "Music/AW/Intro2004.wav";
			}
			break;
		case 138:
			if (_hasRemasteredMusic && Script::_useRemasteredAudio) {
				path = "Music/AW/RmSnd/End2004.wav";
			} else {
				path = "Music/AW/End2004.wav";
			}
			break;

		default:
			break;
		}
		return path;
	}

	virtual void getBitmapSize(int *w, int *h) {
		*w = 1280;
		*h = 800;
	}
};

static uint8 *inflateGzip(const char *filepath) {
	Common::File f;
	if (!f.open(filepath)) {
		warning("Unable to open '%s'", filepath);
		return nullptr;
	}
	const uint16 sig = f.readUint16LE();
	if (sig != 0x8B1F) {
		warning("Unexpected file signature 0x%x for '%s'", sig, filepath);
		return nullptr;
	}
	f.seek(-4, SEEK_END);
	const uint32 dataSize = f.readUint32LE();
	uint8 *out = (uint8 *)malloc(dataSize);
	if (!out) {
		warning("Failed to allocate %d bytes", dataSize);
		return nullptr;
	}
	f.seek(0);

#ifdef TODO
	z_stream str;
	memset(&str, 0, sizeof(str));
	int err = inflateInit2(&str, MAX_WBITS + 16);
	if (err == Z_OK) {
		Bytef buf[1 << MAX_WBITS];
		str.next_in = buf;
		str.avail_in = 0;
		str.next_out = out;
		str.avail_out = dataSize;
		while (err == Z_OK && str.avail_out != 0) {
			if (str.avail_in == 0 && !f.err()) {
				str.next_in = buf;
				str.avail_in = f.read(buf, sizeof(buf));
			}
			err = inflate(&str, Z_NO_FLUSH);
		}
		inflateEnd(&str);
		if (err == Z_STREAM_END) {
			return out;
		}
	}
	free(out);
#else
	error("TODO: inflateGzip");
#endif
	return nullptr;
}

struct Resource20th : ResourceNth {
	char *_textBuf;
	const char *_stringsTable[192];
	char _musicName[64];
	uint8 _musicType;
	char _datName[32];
	const char *_bitmapSize;

	Resource20th() : _textBuf(nullptr), _bitmapSize(nullptr) {
		memset(_stringsTable, 0, sizeof(_stringsTable));
		_musicType = 0;
		_datName[0] = 0;
		_musicName[0] = 0;
	}

	virtual ~Resource20th() {
		free(_textBuf);
	}

	virtual bool init() {
		static const char *dirs[] = { "BGZ", "DAT", "WGZ", nullptr };
		for (int i = 0; dirs[i]; ++i) {
			char path[MAXPATHLEN];
			snprintf(path, sizeof(path), "game/%s", dirs[i]);
			Common::FSNode f(path);

			if (f.exists() && !f.isDirectory()) {
				warning("'%s' is not a directory", path);
				return false;
			}
		}
		static const char *bmps[] = {
			"1728x1080",
			"1280x800",
			"1152x720",
			"960x600",
			"864x540",
			"768x480",
			"480x300",
			"320x200",
			nullptr
		};
		_bitmapSize = nullptr;
		for (int i = 0; bmps[i]; ++i) {
			char path[MAXPATHLEN];
			snprintf(path, sizeof(path), "game/BGZ/data%s", bmps[i]);
			Common::FSNode f(path);

			if (f.isDirectory()) {
				_bitmapSize = bmps[i];
				break;
			}
		}
		return true;
	}

	virtual uint8 *load(const char *name) {
		if (strcmp(name, "font.bmp") == 0) {
			return inflateGzip("game/BGZ/Font.bgz");
		} else if (strcmp(name, "heads.bmp") == 0) {
			return inflateGzip("game/BGZ/Heads.bgz");
		}

		return nullptr;
	}

	virtual uint8 *loadBmp(int num) {
		char path[MAXPATHLEN];
		if (num >= 3000 && _bitmapSize) {
			snprintf(path, sizeof(path), "game/BGZ/data%s/%s_e%04d.bgz", _bitmapSize, _bitmapSize, num);
		} else {
			snprintf(path, sizeof(path), "game/BGZ/file%03d.bgz", num);
		}

		return inflateGzip(path);
	}

	void preloadDat(int part, int type, int num) {
		static const char *names[] = {
			"INTRO", "EAU", "PRI", "CITE", "arene", "LUXE", "FINAL", nullptr
		};
		static const char *exts[] = {
			"pal", "mac", "mat", nullptr
		};
		if (part > 0 && part < 8) {
			if (type == 3) {
				assert(num == 0x11);
				Common::strcpy_s(_datName, "BANK2.MAT");
			} else {
				snprintf(_datName, sizeof(_datName), "%s2011.%s", names[part - 1], exts[type]);
			}
			debugC(kDebugResource, "Loading '%s'", _datName);
		} else {
			_datName[0] = 0;
		}
	}

	virtual uint8 *loadDat(int num, uint8 *dst, uint32 *size) {
		bool datOpen = false;
		char path[MAXPATHLEN];
		Common::strcpy_s(path, "game/DAT");

		Common::File f;
		if (_datName[0]) {
			datOpen = f.open(_datName);
		}
		if (!datOpen) {
			snprintf(_datName, sizeof(_datName), "FILE%03d.DAT", num);
			datOpen = f.open(_datName);
		}
		if (datOpen) {
			const uint32 dataSize = f.size();
			const uint32 count = f.read(dst, dataSize);
			if (count != dataSize) {
				warning("Failed to read %d bytes (expected %d)", dataSize, count);
			}
			*size = dataSize;
		} else {
			warning("Unable to open '%s/%s'", path, _datName);
			dst = nullptr;
		}
		_datName[0] = 0;
		return dst;
	}

	virtual uint8 *loadWav(int num, uint8 *dst, uint32 *size) {
		char path[MAXPATHLEN];
		if (!Script::_useRemasteredAudio) {
			snprintf(path, sizeof(path), "game/WGZ/original/file%03d.wgz", num);

			if (!Common::File::exists(path)) {
				snprintf(path, sizeof(path), "game/WGZ/original/file%03dB.wgz", num);
			}

			*size = 0;
			return inflateGzip(path);
		}

		switch (num) {
		case 81:
			{
			const int r = g_engine->getRandomNumber(1, 3);
			snprintf(path, sizeof(path), "game/WGZ/file081-EX-%d.wgz", r);
			}
			break;

		case 85:
			{
			const int r = g_engine->getRandomNumber(1, 2);
			const char *snd = "IN";
			if (_musicType == 1) {
				snd = "EX";
			}
			snprintf(path, sizeof(path), "game/WGZ/file085-%s-%d.wgz", snd, r);
			}
			break;
		case 96:
			{
			const int r = g_engine->getRandomNumber(1, 3);
			const char *snd = "GR";
			if (_musicType == 1) {
				snd = "EX";
			} else if (_musicType == 2) {
				snd = "IN";
			}
			snprintf(path, sizeof(path), "game/WGZ/file096-%s-%d.wgz", snd, r);
			}
			break;
		case 163:
			{
			const char *snd = "GR";
			if (_musicType == 1) {
				snd = "EX";
			} else if (_musicType == 2) {
				snd = "IN";
			}
			snprintf(path, sizeof(path), "game/WGZ/file163-%s-1.wgz", snd);
			}
			break;
		default:
			snprintf(path, sizeof(path), "game/WGZ/file%03d.wgz", num);
			if (!Common::File::exists(path)) {
				snprintf(path, sizeof(path), "game/WGZ/file%03dB.wgz", num);
			}
			break;
		}
		*size = 0;
		return inflateGzip(path);
	}

	void loadStrings(Language lang) {
		if (!_textBuf) {
			const char *name = nullptr;
			switch (lang) {
			case Common::FR_FRA:
				name = "FR";
				break;
			case Common::EN_ANY:
				name = "EN";
				break;
			case Common::DE_DEU:
				name = "DE";
				break;
			case Common::ES_ESP:
				name = "ES";
				break;
			case Common::IT_ITA:
				name = "IT";
				break;
			default:
				return;
			}
			char path[MAXPATHLEN];
			static const char *fmt[] = {
				"game/TXT/%s.txt",
				"game/TXT/Linux/%s.txt",
				nullptr
			};
			bool isOpen = false;
			Common::File f;
			for (int i = 0; fmt[i] && !isOpen; ++i) {
				snprintf(path, sizeof(path), fmt[i], name);
				isOpen = f.open(path);
			}
			if (isOpen) {
				const int size = f.size();
				_textBuf = loadTextFile(f, size);
				if (_textBuf) {
					int count = 0;
					for (char *p = _textBuf; count < (int)ARRAYSIZE(_stringsTable); ) {
						_stringsTable[count++] = p;
						char *end = strchr(p, '\n');
						if (!end) {
							break;
						}
						*end++ = 0;
						p = end;
					}
				}
			}
		}
	}
	virtual const char *getString(Language lang, int num) {
		loadStrings(lang);
		if ((uint32)num < ARRAYSIZE(_stringsTable)) {
			return _stringsTable[num];
		}
		return nullptr;
	}

	virtual const char *getMusicName(int num) {
		if (num >= 5000 && Script::_useRemasteredAudio) {
			snprintf(_musicName, sizeof(_musicName), "game/OGG/amb%d.ogg", num);
			switch (num) {
			case 5005:
				_musicType = 1;
				break;
			case 5006:
				_musicType = 3;
				break;
			default:
				_musicType = 2;
				break;
			}
		} else {
			switch (num) {
			case 7:
				if (Script::_useRemasteredAudio) {
					Common::strcpy_s(_musicName, "game/OGG/Intro_20th.ogg");
				} else {
					Common::strcpy_s(_musicName, "game/OGG/original/intro.ogg");
				}
				break;
			case 138:
				if (!Script::_useRemasteredAudio) {
					Common::strcpy_s(_musicName, "game/OGG/original/ending.ogg");
					break;
				}
				/* fall-through */
			default:
				return nullptr;
			}
		}
		return _musicName;
	}

	virtual void getBitmapSize(int *w, int *h) {
		if (_bitmapSize && sscanf(_bitmapSize, "%dx%d", w, h) == 2) {
			return;
		}
		*w = 0;
		*h = 0;
	}
};

ResourceNth *ResourceNth::create(int edition) {
	switch (edition) {
	case 15:
		return new Resource15th();
	case 20:
		return new Resource20th();
	default:
		break;
	}
	return nullptr;
}

} // namespace Awe
