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

#include "common/config-manager.h"
#include "common/file.h"
#include "common/system.h"
#include "common/util.h"
#include "common/rect.h"

#include "audio/mixer.h"

#include "graphics/cursorman.h"
#include "graphics/paletteman.h"

#include "scumm/file.h"
#include "scumm/imuse_digi/dimuse_engine.h"
#include "scumm/scumm.h"
#include "scumm/scumm_v7.h"
#include "scumm/sound.h"
#include "scumm/smush/codec37.h"
#include "scumm/smush/codec47.h"
#include "scumm/smush/smush_font.h"
#include "scumm/smush/smush_player.h"

#include "scumm/insane/insane.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/vorbis.h"

#include "common/compression/deflate.h"

namespace Scumm {

static const int MAX_STRINGS = 200;
static const int ETRS_HEADER_LENGTH = 16;

class StringResource {
private:

	struct {
		int id;
		char *string;
	} _strings[MAX_STRINGS];

	int _nbStrings;
	int _lastId;
	const char *_lastString;

public:

	StringResource() :
		_nbStrings(0),
		_lastId(-1),
		_lastString(nullptr) {
		for (int i = 0; i < MAX_STRINGS; i++) {
			_strings[i].id = 0;
			_strings[i].string = nullptr;
		}
	}
	~StringResource() {
		for (int32 i = 0; i < _nbStrings; i++) {
			delete[] _strings[i].string;
		}
	}

	bool init(char *buffer, int32 length) {
		char *def_start = strchr(buffer, '#');
		while (def_start != nullptr) {
			char *def_end = strchr(def_start, '\n');
			assert(def_end != nullptr);

			char *id_end = def_end;
			while (id_end >= def_start && !Common::isDigit(*(id_end-1))) {
				id_end--;
			}

			assert(id_end > def_start);
			char *id_start = id_end;
			while (Common::isDigit(*(id_start - 1))) {
				id_start--;
			}

			char idstring[32];
			memcpy(idstring, id_start, id_end - id_start);
			idstring[id_end - id_start] = 0;
			int32 id = atoi(idstring);
			char *data_start = def_end;

			while (*data_start == '\n' || *data_start == '\r') {
				data_start++;
			}
			char *data_end = data_start;

			while (1) {
				if (data_end[-2] == '\r' && data_end[-1] == '\n' && data_end[0] == '\r' && data_end[1] == '\n') {
					break;
				}
				// In the Steam Mac version of The Dig, LF-LF is used
				// instead of CR-LF
				if (data_end[-2] == '\n' && data_end[-1] == '\n') {
					break;
				}
				// In Russian Full Throttle strings are finished with
				// just one pair of CR-LF
				if (data_end[-2] == '\r' && data_end[-1] == '\n' && data_end[0] == '#') {
					break;
				}
				data_end++;
				if (data_end >= buffer + length) {
					data_end = buffer + length;
					break;
				}
			}

			data_end -= 2;
			assert(data_end > data_start);
			char *value = new char[data_end - data_start + 1];
			assert(value);
			memcpy(value, data_start, data_end - data_start);
			value[data_end - data_start] = 0;
			char *line_start = value;
			char *line_end;

			while ((line_end = strchr(line_start, '\n'))) {
				line_start = line_end+1;
				if (line_start[0] == '/' && line_start[1] == '/') {
					line_start += 2;
					if	(line_end[-1] == '\r')
						line_end[-1] = ' ';
					else
						*line_end++ = ' ';
					memmove(line_end, line_start, strlen(line_start)+1);
				}
			}
			_strings[_nbStrings].id = id;
			_strings[_nbStrings].string = value;
			_nbStrings ++;
			def_start = strchr(data_end + 2, '#');
		}
		return true;
	}

	const char *get(int id) {
		if (id == _lastId) {
			return _lastString;
		}
		debugC(DEBUG_SMUSH, "StringResource::get(%d)", id);
		for (int i = 0; i < _nbStrings; i++) {
			if (_strings[i].id == id) {
				_lastId = id;
				_lastString = _strings[i].string;
				return _strings[i].string;
			}
		}
		warning("invalid string id : %d", id);
		_lastId = -1;
		_lastString = "unknown string";
		return _lastString;
	}
};

static StringResource *getStrings(ScummEngine *vm, const char *file, bool is_encoded) {
	debugC(DEBUG_SMUSH, "trying to read text resources from %s", file);
	ScummFile theFile(vm);

	vm->openFile(theFile, file);
	if (!theFile.isOpen()) {
		return 0;
	}
	int32 length = theFile.size();
	char *filebuffer = new char [length + 1];
	assert(filebuffer);
	theFile.read(filebuffer, length);
	filebuffer[length] = 0;

	if (is_encoded && READ_BE_UINT32(filebuffer) == MKTAG('E','T','R','S')) {
		assert(length > ETRS_HEADER_LENGTH);
		length -= ETRS_HEADER_LENGTH;
		for (int i = 0; i < length; ++i) {
			filebuffer[i] = filebuffer[i + ETRS_HEADER_LENGTH] ^ 0xCC;
		}
		filebuffer[length] = '\0';
	}
	StringResource *sr = new StringResource;
	assert(sr);
	sr->init(filebuffer, length);
	delete[] filebuffer;
	return sr;
}

void SmushPlayer::timerCallback() {
	parseNextFrame();
}

SmushPlayer::SmushPlayer(ScummEngine_v7 *scumm, IMuseDigital *imuseDigital, Insane *insane) {
	_vm = scumm;
	_imuseDigital = imuseDigital;
	_insane = insane;
	_nbframes = 0;
	_deltaBlocksCodec = 0;
	_deltaGlyphsCodec = 0;
	_strings = nullptr;
	_sf[0] = nullptr;
	_sf[1] = nullptr;
	_sf[2] = nullptr;
	_sf[3] = nullptr;
	_sf[4] = nullptr;
	_base = nullptr;
	_frameBuffer = nullptr;
	_specialBuffer = nullptr;

	_seekPos = -1;

	_skipNext = false;
	_dst = nullptr;
	_storeFrame = false;
	_compressedFileMode = false;
	_width = 0;
	_height = 0;
	_IACTpos = 0;
	_speed = -1;
	_insanity = false;
	_middleAudio = false;
	_skipPalette = false;
	_IACTstream = nullptr;
	_paused = false;
	_pauseStartTime = 0;
	_pauseTime = 0;

	memset(_pal, 0, sizeof(_pal));
	memset(_deltaPal, 0, sizeof(_deltaPal));
	memset(_shiftedDeltaPal, 0, sizeof(_shiftedDeltaPal));

	for (int i = 0; i < 4; i++)
		_iactTable[i] = 0;

	_IACTchannel = new Audio::SoundHandle();
	_compressedFileSoundHandle = new Audio::SoundHandle();

	_smushNumTracks = 0;
	_gainReductionLowerBound = 64;
	_gainReductionFactor = 256;
	_gainReductionMultiplier = 256;
	_smushTracksNeedInit = true;
	_smushAudioInitialized = false;
	_smushAudioCallbackEnabled = false;

	initAudio(_imuseDigital->getSampleRate(), 200000);
}

SmushPlayer::~SmushPlayer() {
	delete _IACTchannel;
	delete _compressedFileSoundHandle;
	terminateAudio();
}

void SmushPlayer::init(int32 speed) {
	VirtScreen *vs = &_vm->_virtscr[kMainVirtScreen];

	_frame = 0;
	_speed = speed;
	_endOfFile = false;

	_vm->_smushVideoShouldFinish = false;
	_vm->_smushActive = true;

	_vm->setDirtyColors(0, 255);
	_dst = vs->getPixels(0, 0);

	// HACK HACK HACK: This is an *evil* trick, beware!
	// We do this to fix bug #1792. A proper solution would change all the
	// drawing code to use the pitch value specified by the virtual screen.
	// However, since a lot of the SMUSH code currently assumes the screen
	// width and pitch to be equal, this will require lots of changes. So
	// we resort to this hackish solution for now.
	_origPitch = vs->pitch;
	_origNumStrips = _vm->_gdi->_numStrips;
	vs->pitch = vs->w;
	_vm->_gdi->_numStrips = vs->w / 8;

	_vm->_mixer->stopHandle(*_compressedFileSoundHandle);
	_vm->_mixer->stopHandle(*_IACTchannel);
	_IACTpos = 0;
}

void SmushPlayer::release() {
	_vm->_smushVideoShouldFinish = true;

	for (int i = 0; i < 5; i++) {
		delete _sf[i];
		_sf[i] = nullptr;
	}

	delete _strings;
	_strings = nullptr;

	delete _base;
	_base = nullptr;

	free(_specialBuffer);
	_specialBuffer = nullptr;

	free(_frameBuffer);
	_frameBuffer = nullptr;

	_IACTstream = nullptr;

	_vm->_smushActive = false;
	_vm->_fullRedraw = true;

	// HACK HACK HACK: This is an *evil* trick, beware! See above for
	// some explanation.
	_vm->_virtscr[kMainVirtScreen].pitch = _origPitch;
	_vm->_gdi->_numStrips = _origNumStrips;

	delete _deltaBlocksCodec;
	_deltaBlocksCodec = 0;
	delete _deltaGlyphsCodec;
	_deltaGlyphsCodec = 0;
}

void SmushPlayer::handleStore(int32 subSize, Common::SeekableReadStream &b) {
	debugC(DEBUG_SMUSH, "SmushPlayer::handleStore()");
	assert(subSize >= 4);
	_storeFrame = true;
}

void SmushPlayer::handleFetch(int32 subSize, Common::SeekableReadStream &b) {
	debugC(DEBUG_SMUSH, "SmushPlayer::handleFetch()");
	assert(subSize >= 6);

	if (_frameBuffer != nullptr) {
		memcpy(_dst, _frameBuffer, _width * _height);
	}
}

void SmushPlayer::handleIACT(int32 subSize, Common::SeekableReadStream &b) {
	debugC(DEBUG_SMUSH, "SmushPlayer::IACT()");
	assert(subSize >= 8);

	int code = b.readUint16LE();
	int flags = b.readUint16LE();
	int unknown = b.readSint16LE();
	int userId = b.readUint16LE();

	if ((code != 8) && (flags != 46)) {
		_vm->_insane->procIACT(_dst, 0, 0, 0, b, 0, 0, code, flags, unknown, userId);
		return;
	}

	if (_compressedFileMode) {
		return;
	}

	assert(flags == 46 && unknown == 0);
	/*int trkId =*/ b.readUint16LE();
	int index = b.readUint16LE();
	int nbframes = b.readUint16LE();
	/*int32 size =*/ b.readUint32LE();
	int32 bsize = subSize - 18;

	if (_vm->_game.id == GID_CMI) {
		// TODO: Move this code into another SmushChannel subclass?
		byte *src = (byte *)malloc(bsize);
		b.read(src, bsize);
		byte *d_src = src;
		byte value;

		while (bsize > 0) {
			if (_IACTpos >= 2) {
				int32 len = READ_BE_UINT16(_IACToutput) + 2;
				len -= _IACTpos;
				if (len > bsize) {
					memcpy(_IACToutput + _IACTpos, d_src, bsize);
					_IACTpos += bsize;
					bsize = 0;
				} else {
					byte *output_data = (byte *)malloc(4096);

					memcpy(_IACToutput + _IACTpos, d_src, len);
					byte *dst = output_data;
					byte *d_src2 = _IACToutput;
					d_src2 += 2;
					int32 count = 1024;
					byte variable1 = *d_src2++;
					byte variable2 = variable1 / 16;
					variable1 &= 0x0f;
					do {
						value = *(d_src2++);
						if (value == 0x80) {
							*dst++ = *d_src2++;
							*dst++ = *d_src2++;
						} else {
							int16 val = (int8)value << variable2;
							*dst++ = val >> 8;
							*dst++ = (byte)(val);
						}
						value = *(d_src2++);
						if (value == 0x80) {
							*dst++ = *d_src2++;
							*dst++ = *d_src2++;
						} else {
							int16 val = (int8)value << variable1;
							*dst++ = val >> 8;
							*dst++ = (byte)(val);
						}
					} while (--count);

					if (!_IACTstream) {
						_IACTstream = Audio::makeQueuingAudioStream(22050, true);
						_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, _IACTchannel, _IACTstream);
					}
					_IACTstream->queueBuffer(output_data, 0x1000, DisposeAfterUse::YES, Audio::FLAG_STEREO | Audio::FLAG_16BITS);

					bsize -= len;
					d_src += len;
					_IACTpos = 0;
				}
			} else {
				if (bsize > 1 && _IACTpos == 0) {
					*(_IACToutput + 0) = *d_src++;
					_IACTpos = 1;
					bsize--;
				}
				*(_IACToutput + _IACTpos) = *d_src++;
				_IACTpos++;
				bsize--;
			}
		}

		free(src);
	} else if ((_vm->_game.id == GID_DIG) && !(_vm->_game.features & GF_DEMO)) {
		int bufId, volume, paused, curSoundId;

		byte *dataBuffer = (byte *)malloc(bsize);
		b.read(dataBuffer, bsize);

		switch (userId) {
		case TRK_USERID_SPEECH:
			bufId = DIMUSE_BUFFER_SPEECH;
			volume = 127;
			break;
		case TRK_USERID_MUSIC:
			bufId = DIMUSE_BUFFER_MUSIC;
			volume = 127;
			break;
		case TRK_USERID_SFX:
			bufId = DIMUSE_BUFFER_SFX;
			volume = 127;
			break;
		default:
			if (userId >= 100 && userId <= 163) {
				bufId = DIMUSE_BUFFER_SPEECH;
				volume = 2 * userId - 200;
			} else if (userId >= 200 && userId <= 263) {
				bufId = DIMUSE_BUFFER_MUSIC;
				volume = 2 * userId - 400;
			} else if (userId >= 300 && userId <= 363) {
				bufId = DIMUSE_BUFFER_SFX;
				volume = 2 * userId - 600;
			} else {
				free(dataBuffer);
				error("SmushPlayer::handleIACT(): ERROR: got invalid userID (%d)", userId);
			}
			break;
		}

		paused = nbframes - index == 1;

		// Apparently this is expected to happen (e.g.: Brink's death video)
		if (index && _iactTable[bufId] - index != -1) {
			free(dataBuffer);
			debugC(DEBUG_SMUSH, "SmushPlayer::handleIACT(): WARNING: got out of order block");
			return;
		}

		_iactTable[bufId] = index;

		if (index) {
			if (_imuseDigital->diMUSEGetParam(bufId + DIMUSE_SMUSH_SOUNDID, DIMUSE_P_SND_TRACK_NUM)) {
				_imuseDigital->diMUSEFeedStream(bufId + DIMUSE_SMUSH_SOUNDID, dataBuffer, subSize - 18, paused);
				free(dataBuffer);
				return;
			}
			free(dataBuffer);
			error("SmushPlayer::handleIACT(): ERROR: got unexpected non-zero IACT block, bufID %d", bufId);
		} else {
			if (READ_BE_UINT32(dataBuffer) != MKTAG('i', 'M', 'U', 'S')) {
				free(dataBuffer);
				error("SmushPlayer::handleIACT(): ERROR: got non-IMUS IACT block");
			}

			curSoundId = 0;
			do {
				curSoundId = _imuseDigital->diMUSEGetNextSound(curSoundId);
				if (!curSoundId)
					break;
			} while (_imuseDigital->diMUSEGetParam(curSoundId, DIMUSE_P_SND_HAS_STREAM) != 1 || _imuseDigital->diMUSEGetParam(curSoundId, DIMUSE_P_STREAM_BUFID) != bufId);

			if (!curSoundId) {
				// There isn't any previous sound running: start a new stream
				if (_imuseDigital->diMUSEStartStream(bufId + DIMUSE_SMUSH_SOUNDID, 126, bufId)) {
					free(dataBuffer);
					error("SmushPlayer::handleIACT(): ERROR: couldn't start stream");
				}
			} else {
				// There's an old sound running: switch the stream from the old one to the new one
				_imuseDigital->diMUSESwitchStream(curSoundId, bufId + DIMUSE_SMUSH_SOUNDID, bufId == DIMUSE_BUFFER_MUSIC ? 1000 : 150, 0, 0);
			}

			_imuseDigital->diMUSESetParam(bufId + DIMUSE_SMUSH_SOUNDID, DIMUSE_P_VOLUME, volume);

			if (bufId == DIMUSE_BUFFER_SPEECH) {
				_imuseDigital->diMUSESetParam(bufId + DIMUSE_SMUSH_SOUNDID, DIMUSE_P_GROUP, DIMUSE_GROUP_SPEECH);
			} else if (bufId == DIMUSE_BUFFER_MUSIC) {
				_imuseDigital->diMUSESetParam(bufId + DIMUSE_SMUSH_SOUNDID, DIMUSE_P_GROUP, DIMUSE_GROUP_MUSIC);
			} else {
				_imuseDigital->diMUSESetParam(bufId + DIMUSE_SMUSH_SOUNDID, DIMUSE_P_GROUP, DIMUSE_GROUP_SFX);
			}

			_imuseDigital->diMUSEFeedStream(bufId + DIMUSE_SMUSH_SOUNDID, dataBuffer, subSize - 18, paused);
			free(dataBuffer);
			return;
		}
	}
}

void SmushPlayer::handleTextResource(uint32 subType, int32 subSize, Common::SeekableReadStream &b) {
	int pos_x = b.readSint16LE();
	int pos_y = b.readSint16LE();
	int flags = b.readSint16LE();
	int left = b.readSint16LE();
	int top = b.readSint16LE();
	int width = b.readSint16LE();
	int height = b.readSint16LE();
	/*int32 unk2 =*/ b.readUint16LE();

	const char *str;
	char *string = nullptr, *string2 = nullptr;
	if (subType == MKTAG('T','E','X','T')) {
		string = (char *)malloc(subSize - 16);
		str = string;
		b.read(string, subSize - 16);
	} else {
		int string_id = b.readUint16LE();
		if (!_strings)
			return;
		str = _strings->get(string_id);
	}

	// if subtitles disabled and bit 3 is set, then do not draw
	//
	// Query ConfMan here. However it may be slower, but
	// player may want to switch the subtitles on or off during the
	// playback. This fixes bug #2812
	if ((!ConfMan.getBool("subtitles")) && ((flags & 8) == 8))
		return;

	bool isCJKComi = (_vm->_game.id == GID_CMI && _vm->_useCJKMode);
	int color = 15;
	int fontId = isCJKComi ? 1 : 0;

	while (*str == '/') {
		str++; // For Full Throttle text resources
	}

	byte transBuf[512];
	if (_vm->_game.id == GID_CMI) {
		_vm->translateText((const byte *)str - 1, transBuf, sizeof(transBuf));
		while (*str++ != '/')
			;
		string2 = (char *)transBuf;

		// If string2 contains formatting information there probably
		// wasn't any translation for it in the language.tab file. In
		// that case, pretend there is no string2.
		if (string2[0] == '^')
			string2[0] = 0;
	}

	while (str[0] == '^') {
		switch (str[1]) {
		case 'f':
			fontId = str[3] - '0';
			str += 4;
			break;
		case 'c':
			color = str[4] - '0' + 10 *(str[3] - '0');
			str += 5;
		break;
		default:
			error("invalid escape code in text string");
		}
	}

	if (_vm->_game.id == GID_CMI && string2[0] != 0)
		str = string2;

	// This is a hack from the original COMI CJK interpreter. Its purpose is to avoid
	// ugly combinations of two byte characters (rendered with the respective special
	// font) and standard one byte (NUT font) characters (see bug #11947).
	if (isCJKComi && !(fontId == 0 && color == 1)) {
		fontId = 1;
		color = 255;
	}

	SmushFont *sf = getFont(fontId);
	assert(sf != nullptr);

	// The hack that used to be here to prevent bug #2220 is no longer necessary and
	// has been removed. The font renderer can handle all ^codes it encounters (font
	// changes on the fly will be ignored for Smush texts, since our code design does
	// not permit it and the feature isn't used anyway).

	if (_vm->_language == Common::HE_ISR && !(flags & kStyleAlignCenter)) {
		flags |= kStyleAlignRight;
		pos_x = _width - 1 - pos_x;
	}

	TextStyleFlags flg = (TextStyleFlags)(flags & 7);
	// flags:
	// bit 0 - center                  0x01
	// bit 1 - not used (align right)  0x02
	// bit 2 - word wrap               0x04
	// bit 3 - switchable              0x08
	// bit 4 - fill background         0x10
	// bit 5 - outline/shadow          0x20        (apparently only set by the text renderer itself, not from the smush data)
	// bit 6 - vertical fix (COMI)     0x40        (COMI handles this in the printing method, but I haven't seen a case where it is used)
	// bit 7 - skip ^ codes (COMI)     0x80        (should be irrelevant for Smush, we strip these commands anyway)
	// bit 8 - no vertical fix (COMI)  0x100       (COMI handles this in the printing method, but I haven't seen a case where it is used)

	if (flg & kStyleWordWrap) {
		// COMI has to do it all a bit different, of course. SCUMM7 games immediately render the text from here and actually use the clipping data
		// provided by the text resource. COMI does not render directly, but enqueues a blast string (which is then drawn through the usual main
		// loop routines). During that process the rect data will get dumped and replaced with the following default values. It's hard to tell
		// whether this is on purpose or not (the text looks not necessarily better or worse, just different), so we follow the original...
		if (_vm->_game.id == GID_CMI) {
			left = top = 10;
			width = _width - 20;
			height = _height - 20;
		}
		Common::Rect clipRect(MAX<int>(0, left), MAX<int>(0, top), MIN<int>(left + width, _width), MIN<int>(top + height, _height));
		sf->drawStringWrap(str, _dst, clipRect, pos_x, pos_y, color, flg);
	} else {
		// Similiar to the wrapped text, COMI will pass on rect coords here, which will later be lost. Unlike with the wrapped text, it will
		// finally use the full screen dimenstions. SCUMM7 renders directly from here (see comment above), but also with the full screen.
		Common::Rect clipRect(0, 0, _width, _height);
		sf->drawString(str, _dst, clipRect, pos_x, pos_y, color, flg);
	}

	free(string);
}

const char *SmushPlayer::getString(int id) {
	if (_strings != nullptr) {
		return _strings->get(id);
	} else {
		warning("Couldn't load string with id {%d}, are you maybe missing a TRS subtitle file?", id);
		return nullptr;
	}
}

bool SmushPlayer::readString(const char *file) {
	const char *i = strrchr(file, '.');
	if (i == nullptr) {
		error("invalid filename : %s", file);
	}
	char fname[260];
	memcpy(fname, file, MIN<int>(sizeof(fname), i - file));
	Common::strlcpy(fname + (i - file), ".trs", sizeof(fname) - (i - file));
	if ((_strings = getStrings(_vm, fname, false)) != 0) {
		return true;
	}

	if (_vm->_game.id == GID_DIG && (_strings = getStrings(_vm, "digtxt.trs", true)) != 0) {
		return true;
	}
	return false;
}

void SmushPlayer::readPalette(byte *out, Common::SeekableReadStream &in) {
	in.read(out, 0x300);
}

void SmushPlayer::handleDeltaPalette(int32 subSize, Common::SeekableReadStream &b) {
	debugC(DEBUG_SMUSH, "SmushPlayer::handleDeltaPalette()");

	b.readUint16LE();
	uint16 xpalCommand = b.readUint16LE();

	if (xpalCommand == 256) {
		b.readUint16LE();
		for (int i = 0; i < 768; ++i) {
			_shiftedDeltaPal[i] += _deltaPal[i];
			
			_pal[i] = CLIP<int32>(_shiftedDeltaPal[i] >> 7, 0, 255);
		}

		setDirtyColors(0, 255);
	} else {
		for (int j = 0; j < 768; ++j) {
			_shiftedDeltaPal[j] = _pal[j] << 7;
			_deltaPal[j] = b.readUint16LE();
		}

		if (xpalCommand == 512)
			readPalette(_pal, b);

		setDirtyColors(0, 255);
	}
}

void SmushPlayer::handleNewPalette(int32 subSize, Common::SeekableReadStream &b) {
	debugC(DEBUG_SMUSH, "SmushPlayer::handleNewPalette()");
	assert(subSize >= 0x300);

	if (_skipPalette)
		return;

	readPalette(_pal, b);
	setDirtyColors(0, 255);
}

byte *SmushPlayer::getVideoPalette() {
	return _pal;
}

void smushDecodeRLE(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);
void smushDecodeUncompressed(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);

void SmushPlayer::decodeFrameObject(int codec, const uint8 *src, int left, int top, int width, int height) {
	if ((height == 242) && (width == 384)) {
		if (_specialBuffer == 0)
			_specialBuffer = (byte *)malloc(242 * 384);
		_dst = _specialBuffer;
	} else if ((height > _vm->_screenHeight) || (width > _vm->_screenWidth))
		return;
	// FT Insane uses smaller frames to draw overlays with moving objects
	// Other .san files do have them as well but their purpose in unknown
	// and often it causes memory overdraw. So just skip those frames
	else if (!_insanity && ((height != _vm->_screenHeight) || (width != _vm->_screenWidth)))
		return;

	if ((height == 242) && (width == 384)) {
		_width = width;
		_height = height;
	} else {
		_width = _vm->_screenWidth;
		_height = _vm->_screenHeight;
	}

	switch (codec) {
	case SMUSH_CODEC_RLE:
	case SMUSH_CODEC_RLE_ALT:
		smushDecodeRLE(_dst, src, left, top, width, height, _vm->_screenWidth);
		break;
	case SMUSH_CODEC_DELTA_BLOCKS:
		if (!_deltaBlocksCodec)
			_deltaBlocksCodec = new SmushDeltaBlocksDecoder(width, height);
		if (_deltaBlocksCodec)
			_deltaBlocksCodec->decode(_dst, src);
		break;
	case SMUSH_CODEC_DELTA_GLYPHS:
		if (!_deltaGlyphsCodec)
			_deltaGlyphsCodec = new SmushDeltaGlyphsDecoder(width, height);
		if (_deltaGlyphsCodec)
			_deltaGlyphsCodec->decode(_dst, src);
		break;
	case SMUSH_CODEC_UNCOMPRESSED:
		// Used by Full Throttle Classic (from Remastered)
		smushDecodeUncompressed(_dst, src, left, top, width, height, _vm->_screenWidth);
		break;
	default:
		error("Invalid codec for frame object : %d", codec);
	}

	if (_storeFrame) {
		if (_frameBuffer == nullptr) {
			_frameBuffer = (byte *)malloc(_width * _height);
		}
		memcpy(_frameBuffer, _dst, _width * _height);
		_storeFrame = false;
	}
}

void SmushPlayer::handleZlibFrameObject(int32 subSize, Common::SeekableReadStream &b) {
	if (_skipNext) {
		_skipNext = false;
		return;
	}

	int32 chunkSize = subSize;
	byte *chunkBuffer = (byte *)malloc(chunkSize);
	assert(chunkBuffer);
	b.read(chunkBuffer, chunkSize);

	unsigned long decompressedSize = READ_BE_UINT32(chunkBuffer);
	byte *fobjBuffer = (byte *)malloc(decompressedSize);
	if (!Common::inflateZlib(fobjBuffer, &decompressedSize, chunkBuffer + 4, chunkSize - 4))
		error("SmushPlayer::handleZlibFrameObject() Zlib uncompress error");
	free(chunkBuffer);

	byte *ptr = fobjBuffer;
	int codec = READ_LE_UINT16(ptr); ptr += 2;
	int left = READ_LE_UINT16(ptr); ptr += 2;
	int top = READ_LE_UINT16(ptr); ptr += 2;
	int width = READ_LE_UINT16(ptr); ptr += 2;
	int height = READ_LE_UINT16(ptr); ptr += 2;

	decodeFrameObject(codec, fobjBuffer + 14, left, top, width, height);

	free(fobjBuffer);
}

void SmushPlayer::handleFrameObject(int32 subSize, Common::SeekableReadStream &b) {
	assert(subSize >= 14);
	if (_skipNext) {
		_skipNext = false;
		return;
	}

	int codec = b.readUint16LE();
	int left = b.readUint16LE();
	int top = b.readUint16LE();
	int width = b.readUint16LE();
	int height = b.readUint16LE();

	b.readUint16LE();
	b.readUint16LE();

	int32 chunk_size = subSize - 14;
	byte *chunk_buffer = (byte *)malloc(chunk_size);
	assert(chunk_buffer);
	b.read(chunk_buffer, chunk_size);

	decodeFrameObject(codec, chunk_buffer, left, top, width, height);

	free(chunk_buffer);
}

void SmushPlayer::handleFrame(int32 frameSize, Common::SeekableReadStream &b) {
	debugC(DEBUG_SMUSH, "SmushPlayer::handleFrame(%d)", _frame);
	uint8 *audioChunk = nullptr;
	_skipNext = false;

	if (_insanity) {
		_vm->_insane->procPreRendering();
	}

	while (frameSize > 0) {
		const uint32 subType = b.readUint32BE();
		const int32 subSize = b.readUint32BE();
		const int32 subOffset = b.pos();
		switch (subType) {
		case MKTAG('N','P','A','L'):
			handleNewPalette(subSize, b);
			break;
		case MKTAG('F','O','B','J'):
			handleFrameObject(subSize, b);
			break;
		case MKTAG('Z','F','O','B'):
			handleZlibFrameObject(subSize, b);
			break;
		case MKTAG('P','S','A','D'):
			if (!_compressedFileMode) {
				audioChunk = (uint8 *)malloc(subSize + 8);
				b.seek(-8, SEEK_CUR);
				b.read(audioChunk, subSize + 8);
				feedAudio(audioChunk, 0, 127, 0, 0);
				free(audioChunk);
				audioChunk = nullptr;
			}

			break;
		case MKTAG('T','R','E','S'):
			handleTextResource(subType, subSize, b);
			break;
		case MKTAG('X','P','A','L'):
			handleDeltaPalette(subSize, b);
			break;
		case MKTAG('I','A','C','T'):
			handleIACT(subSize, b);
			break;
		case MKTAG('S','T','O','R'):
			handleStore(subSize, b);
			break;
		case MKTAG('F','T','C','H'):
			handleFetch(subSize, b);
			break;
		case MKTAG('S','K','I','P'):
			_vm->_insane->procSKIP(subSize, b);
			break;
		case MKTAG('T','E','X','T'):
			handleTextResource(subType, subSize, b);
			break;
		default:
			error("Unknown frame subChunk found : %s, %d", tag2str(subType), subSize);
		}

		frameSize -= subSize + 8;
		b.seek(subOffset + subSize, SEEK_SET);
		if (subSize & 1) {
			b.skip(1);
			frameSize--;
		}
	}

	if (_insanity) {
		_vm->_insane->procPostRendering(_dst, 0, 0, 0, _frame, _nbframes-1);
	}

	if (_width != 0 && _height != 0) {
		updateScreen();
	}

	_frame++;
}

void SmushPlayer::handleAnimHeader(int32 subSize, Common::SeekableReadStream &b) {
	debugC(DEBUG_SMUSH, "SmushPlayer::handleAnimHeader()");
	assert(subSize >= 0x300 + 6);
	byte *headerContent = (byte *)malloc(subSize * sizeof(byte));

	if (headerContent) {
		// Fill out the header
		b.read(headerContent, subSize);

		byte headerMajorVersion = headerContent[0];
		byte headerMinorVersion = headerContent[1];

		_nbframes = READ_LE_UINT16(&headerContent[2]);

		// Video files might contain framerate overrides
		if (headerMajorVersion > 1) {
			uint16 speed = READ_LE_UINT16(&headerContent[6 + 0x300]);
			if ((_curVideoFlags & 8) == 0 && speed != 0) {
				debug(5, "SmushPlayer::handleAnimHeader(): header version %d.%d, video speed override %d fps (cur speed %d)",
						headerMajorVersion,
						headerMinorVersion,
						speed,
						_speed);

				_speed = speed;
			}
		}

		if (!_skipPalette) {
			byte *palettePtr = &headerContent[6];
			memcpy(_pal, palettePtr, sizeof(_pal));
			setDirtyColors(0, 255);
		}

		free(headerContent);
	}
}

void SmushPlayer::setupAnim(const char *file) {
	if (_insanity) {
		if (!((_vm->_game.features & GF_DEMO) && (_vm->_game.platform == Common::kPlatformDOS)))
			readString("mineroad.trs");
	} else
		readString(file);
}

void SmushPlayer::setCurVideoFlags(int16 flags) {
	_curVideoFlags = flags;
}

SmushFont *SmushPlayer::getFont(int font) {
	char file_font[11];

	if (_sf[font])
		return _sf[font];

	if (_vm->_game.id == GID_FT) {
		if (!((_vm->_game.features & GF_DEMO) && (_vm->_game.platform == Common::kPlatformDOS))) {
			const char *ft_fonts[] = {
				"scummfnt.nut",
				"techfnt.nut",
				"titlfnt.nut",
				"specfnt.nut"
			};

			assert(font >= 0 && font < ARRAYSIZE(ft_fonts));

			_sf[font] = new SmushFont(_vm, ft_fonts[font], true);
		}
	} else {
		int numFonts = (_vm->_game.id == GID_CMI && !(_vm->_game.features & GF_DEMO)) ? 5 : 4;
		assert(font >= 0 && font < numFonts);
		Common::sprintf_s(file_font, "font%d.nut", font);
		_sf[font] = new SmushFont(_vm, file_font, _vm->_game.id == GID_DIG && font != 0);
	}

	assert(_sf[font]);
	return _sf[font];
}

void SmushPlayer::parseNextFrame() {

	if (_seekPos >= 0) {
		if (_seekFile.size() > 0) {
			delete _base;

			ScummFile *tmp = new ScummFile(_vm);
			if (!g_scumm->openFile(*tmp, Common::Path(_seekFile)))
				error("SmushPlayer: Unable to open file %s", _seekFile.c_str());
			_base = tmp;
			_base->readUint32BE();
			_baseSize = _base->readUint32BE();

			if (_seekPos > 0) {
				assert(_seekPos > 8);
				// In this case we need to get palette and number of frames
				const uint32 subType = _base->readUint32BE();
				const int32 subSize = _base->readUint32BE();
				const int32 subOffset = _base->pos();
				assert(subType == MKTAG('A','H','D','R'));
				handleAnimHeader(subSize, *_base);
				_base->seek(subOffset + subSize, SEEK_SET);

				_middleAudio = true;
				_seekPos -= 8;
			} else {
				// We need this in Full Throttle when entering/leaving
				// the old mine road.
				tryCmpFile(_seekFile.c_str());
			}
			_skipPalette = false;
		} else {
			_skipPalette = true;
		}

		_base->seek(_seekPos + 8, SEEK_SET);
		_frame = _seekFrame;
		_startFrame = _frame;
		_startTime = _vm->_system->getMillis();

		_seekPos = -1;
	}

	assert(_base);

	const uint32 subType = _base->readUint32BE();
	const int32 subSize = _base->readUint32BE();
	const int32 subOffset = _base->pos();

	if (_base->pos() >= (int32)_baseSize) {
		_vm->_smushVideoShouldFinish = true;
		_endOfFile = true;
		return;
	}

	debug(3, "Chunk: %s at %x", tag2str(subType), subOffset);

	switch (subType) {
	case MKTAG('A','H','D','R'): // FT INSANE may seek file to the beginning
		handleAnimHeader(subSize, *_base);
		break;
	case MKTAG('F','R','M','E'):
		handleFrame(subSize, *_base);
		break;
	default:
		error("Unknown Chunk found at %x: %s, %d", subOffset, tag2str(subType), subSize);
	}

	_base->seek(subOffset + subSize, SEEK_SET);

	if (_insanity)
		_vm->_sound->processSound();

	_vm->_imuseDigital->flushTracks();
}

void SmushPlayer::setPalette(const byte *palette) {
	memcpy(_pal, palette, 0x300);
	setDirtyColors(0, 255);
}

void SmushPlayer::setPaletteValue(int n, byte r, byte g, byte b) {
	_pal[n * 3 + 0] = r;
	_pal[n * 3 + 1] = g;
	_pal[n * 3 + 2] = b;
	setDirtyColors(n, n);
}

void SmushPlayer::setDirtyColors(int min, int max) {
	if (_palDirtyMin > min)
		_palDirtyMin = min;
	if (_palDirtyMax < max)
		_palDirtyMax = max;
}

void SmushPlayer::warpMouse(int x, int y, int buttons) {
	_warpNeeded = true;
	_warpX = x;
	_warpY = y;
	_warpButtons = buttons;
}

void SmushPlayer::updateScreen() {
	uint32 end_time, start_time = _vm->_system->getMillis();
	_updateNeeded = true;
	end_time = _vm->_system->getMillis();
	debugC(DEBUG_SMUSH, "Smush stats: updateScreen( %03d )", end_time - start_time);
}

void SmushPlayer::insanity(bool flag) {
	_insanity = flag;
}

void SmushPlayer::seekSan(const char *file, int32 pos, int32 contFrame) {
	_seekFile = file ? file : "";
	_seekPos = pos;
	_seekFrame = contFrame;
	_pauseTime = 0;
}

void SmushPlayer::tryCmpFile(const char *filename) {
	_vm->_mixer->stopHandle(*_compressedFileSoundHandle);

	_compressedFileMode = false;
	const char *i = strrchr(filename, '.');
	if (i == NULL) {
		error("invalid filename : %s", filename);
	}
#if defined(USE_MAD) || defined(USE_VORBIS)
	char fname[260];
#endif
	Common::File *file = new Common::File();

	// FIXME: How about using AudioStream::openStreamFile instead of the code below?

#ifdef USE_VORBIS
	memcpy(fname, filename, MIN<int>(i - filename, sizeof(fname)));
	Common::strlcpy(fname + (i - filename), ".ogg", sizeof(fname) - (i - filename));
	if (file->open(fname)) {
		_compressedFileMode = true;
		_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, _compressedFileSoundHandle, Audio::makeVorbisStream(file, DisposeAfterUse::YES));
		return;
	}
#endif
#ifdef USE_MAD
	memcpy(fname, filename, MIN<int>(i - filename, sizeof(fname)));
	Common::strlcpy(fname + (i - filename), ".mp3", sizeof(fname) - (i - filename));
	if (file->open(fname)) {
		_compressedFileMode = true;
		_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, _compressedFileSoundHandle, Audio::makeMP3Stream(file, DisposeAfterUse::YES));
		return;
	}
#endif
	delete file;
}

void SmushPlayer::pause() {
	if (!_paused) {
		_paused = true;
		_pauseStartTime = _vm->_system->getMillis();
	}
}

void SmushPlayer::unpause() {
	if (_paused) {
		_paused = false;
		_pauseTime += (_vm->_system->getMillis() - _pauseStartTime);
		_pauseStartTime = 0;
	}
}

void SmushPlayer::play(const char *filename, int32 speed, int32 offset, int32 startFrame) {
	// Verify the specified file exists
	ScummFile f(_vm);
	_vm->openFile(f, filename);
	if (!f.isOpen()) {
		warning("SmushPlayer::play() File not found %s", filename);
		return;
	}
	f.close();

	_updateNeeded = false;
	_warpNeeded = false;
	_palDirtyMin = 256;
	_palDirtyMax = -1;

	// Hide mouse
	bool oldMouseState = CursorMan.showMouse(false);

	// Load the video
	_seekFile = filename;
	_seekPos = offset;
	_seekFrame = startFrame;
	_base = 0;

	setupAnim(filename);
	init(speed);

	_startTime = _vm->_system->getMillis();
	_startFrame = startFrame;
	_frame = startFrame;

	_pauseTime = 0;

	// This piece of code is used to ensure there are
	// no audio hiccups while loading the SMUSH video;
	// Each version of the engine does it in its own way.
	if (_imuseDigital->isFTSoundEngine()) {
		_imuseDigital->fillStreamsWhileMusicCritical(20);
	} else {
		_imuseDigital->floodMusicBuffer();
	}

	int skipped = 0;

	for (;;) {
		uint32 now, elapsed;
		bool skipFrame = false;

		if (_insanity) {
			// Seeking makes a mess of trying to sync the audio to
			// the sound. Synt to time instead.
			now = _vm->_system->getMillis() - _pauseTime;
			elapsed = now - _startTime;
		} else if (_vm->_mixer->isSoundHandleActive(*_compressedFileSoundHandle)) {
			// Compressed SMUSH files.
			elapsed = _vm->_mixer->getSoundElapsedTime(*_compressedFileSoundHandle);
		} else if (_vm->_mixer->isSoundHandleActive(*_IACTchannel)) {
			// Curse of Monkey Island SMUSH files.
			elapsed = _vm->_mixer->getSoundElapsedTime(*_IACTchannel);
		} else {
			// For other SMUSH files, we don't necessarily have any
			// one channel to sync against, so we have to use
			// elapsed real time.
			now = _vm->_system->getMillis() - _pauseTime;
			elapsed = now - _startTime;
		}

		if (elapsed >= ((_frame - _startFrame) * 1000) / _speed) {
			if (elapsed >= ((_frame + 1) * 1000) / _speed)
				skipFrame = true;
			else
				skipFrame = false;
			timerCallback();
		}

		_vm->scummLoop_handleSound();

		if (_warpNeeded) {
			_vm->_system->warpMouse(_vm->_macScreen ? _warpX * 2 : _warpX, _vm->_macScreen ? (_warpY * 2 + 2 * _vm->_macScreenDrawOffset) : _warpY);
			_warpNeeded = false;
		}
		_vm->parseEvents();
		_vm->processInput();
		if (_palDirtyMax >= _palDirtyMin) {
			// Apply gamma correction for Mac versions
			if (_vm->_macScreen) {
				byte palette[768];
				memcpy(palette, _pal, 768);
				for (int i = 0; i < ARRAYSIZE(palette); i++) {
					palette[i] = _vm->_macGammaCorrectionLookUp[_pal[i]];
				}

				_vm->_system->getPaletteManager()->setPalette(palette + _palDirtyMin * 3, _palDirtyMin, _palDirtyMax - _palDirtyMin + 1);
			} else {
				_vm->_system->getPaletteManager()->setPalette(_pal + _palDirtyMin * 3, _palDirtyMin, _palDirtyMax - _palDirtyMin + 1);
			}

			_palDirtyMax = -1;
			_palDirtyMin = 256;
			skipFrame = false;
		}
		if (skipFrame) {
			if (++skipped > 10) {
				skipFrame = false;
				skipped = 0;
			}
		} else
			skipped = 0;
		if (_updateNeeded) {
			if (!skipFrame) {
				// WORKAROUND for bug #2415: "FT DEMO: assertion triggered
				// when playing movie". Some frames there are 384 x 224
				int frameWidth = MIN(_width, _vm->_screenWidth);
				int frameHeight = MIN(_height, _vm->_screenHeight);

				if (_vm->_macScreen) {
					_vm->mac_drawBufferToScreen(_dst, frameWidth, 0, 0, frameWidth, frameHeight);
				} else {
					_vm->_system->copyRectToScreen(_dst, _width, 0, 0, frameWidth, frameHeight);
				}

				_vm->_system->updateScreen();
				_updateNeeded = false;
			}
		}
		if (_endOfFile)
			break;
		if (_vm->shouldQuit() || _vm->_saveLoadFlag || _vm->_smushVideoShouldFinish) {
			_vm->_mixer->stopHandle(*_compressedFileSoundHandle);
			_vm->_mixer->stopHandle(*_IACTchannel);
			_IACTpos = 0;

			resetAudioTracks(); // For DIG demo
			_imuseDigital->stopSMUSHAudio(); // For DIG & COMI
			break;
		}
		_vm->_system->delayMillis(10);
	}

	release();

	// Reset mouse state
	CursorMan.showMouse(oldMouseState);
}

void SmushPlayer::initAudio(int samplerate, int32 maxChunkSize) {
	int32 maxSizes[SMUSH_MAX_TRACKS] = {100000, 100000, 100000, 400000};

	_imuseDigital->setSmushPlayer(this);

	// DIG demo uses this audio system but doesn't use INSANE
	if (_insane)
		_insane->setSmushPlayer(this);

	setGainReductionParams(114, 2048);

	memset(_smushAudioTable, 0, sizeof(_smushAudioTable));

	for (int i = 0; i < SMUSH_MAX_TRACKS; i++) {
		_smushTrackVols[i] = 127;
		_smushTrackFlags[i] = 1;
		addAudioTrack(maxSizes[i], maxChunkSize);
	}

	_smushAudioSampleRate = samplerate;
	_smushAudioInitialized = true;
	_smushAudioCallbackEnabled = true;

	resetAudioTracks();
}

void SmushPlayer::terminateAudio() {
	if (_smushAudioInitialized) {
		_smushAudioInitialized = false;
		_smushAudioCallbackEnabled = false;
	}

	for (int i = 0; i < _smushNumTracks; i++) {
		free(_smushTracks[i].blockPtr);
		free(_smushTracks[i].fadeBuf);
	}
	_smushNumTracks = 0;
}

int SmushPlayer::isChanActive(int flagId) {
	return _smushTrackFlags[flagId];
}

int SmushPlayer::setChanFlag(int id, int flagVal) {
	if (id >= 0 && id <= _smushNumTracks)
		_smushTrackFlags[id] = flagVal;

	return id;
}

int SmushPlayer::addAudioTrack(int32 trackBlockSize, int32 maxBlockSize) {
	int id = _smushNumTracks;
	_smushTracks[id].state = TRK_STATE_INACTIVE;
	_smushTracks[id].audioRemaining = 0;
	_smushTracks[id].flags = 0;
	_smushTracks[id].groupId = GRP_MASTER;
	_smushTracks[id].blockSize = trackBlockSize;
	_smushTracks[id].parsedChunks = 0;

	_smushTracks[id].fadeBuf = (uint8 *)malloc(SMUSH_FADE_SIZE);
	if (!_smushTracks[id].fadeBuf)
		return -1;

	_smushTracks[id].blockPtr = (uint8 *)malloc(_smushTracks[id].blockSize);
	if (!_smushTracks[id].blockPtr)
		return -1;

	memset(_smushTracks[id].blockPtr, 127, _smushTracks[id].blockSize);
	// Track the effective number of tracks, so that if only one fails,
	// the others can carry on as they should
	_smushNumTracks++;

	return 0;
}

void SmushPlayer::resetAudioTracks() {
	for (int i = 0; i < _smushNumTracks; i++) {
		_smushTracks[i].state = TRK_STATE_INACTIVE;
		_smushTracks[i].groupId = GRP_MASTER;
	}
}

void SmushPlayer::setGainReductionParams(int16 gainReductionLowerBound, int16 gainReductionMultiplier) {
	if (gainReductionLowerBound)
		_gainReductionLowerBound = gainReductionLowerBound;
	if (gainReductionMultiplier)
		_gainReductionFactor = gainReductionMultiplier;
}

void SmushPlayer::setGroupVolume(int groupId, int volValue) {
	switch (groupId) {
	case GRP_MASTER:
		_smushTrackVols[0] = volValue; // Master
		break;
	case GRP_SFX:
		_smushTrackVols[1] = volValue; // Sfx
		break;
	case GRP_BKGMUS:
		_smushTrackVols[3] = volValue; // Music
		break;
	case GRP_SPEECH:
		_smushTrackVols[2] = volValue; // Voice
		break;
	default: // Internal groups
		for (int i = 0; i < SMUSH_MAX_TRACKS; i++) {
			if (_smushTracks[i].groupId == groupId)
				_smushTracks[i].volume = volValue;
		}
	}
}

void SmushPlayer::handleSAUDChunk(uint8 *srcBuf, uint32 size, int groupId, int vol, int pan, int16 flags, int trkId, int index, int maxFrames) {
	int targetTrk, diff1, diff2;
	uint32 dataSize, mod;
	int32 maxBlockSize, saudSize;
	uint16 flagsAccumulator[SMUSH_MAX_TRACKS];

	if (!index) {
		saudSize = READ_BE_UINT32(&srcBuf[4]);

		// Find the maximum block sizes between the current ones
		maxBlockSize = 0;
		for (int i = 0; i < _smushNumTracks; i++) {
			if (_smushTracks[i].blockSize > maxBlockSize) {
				maxBlockSize = _smushTracks[i].blockSize;
			}
		}

		diff2 = 10000000;
		if ((flags & TRK_TYPE_MASK) != IS_BKG_MUSIC) {
			for (int i = 0; i < _smushNumTracks; i++) {
				if (_smushTracks[i].blockSize - saudSize >= 0) {
					diff1 = _smushTracks[i].blockSize - saudSize;
				} else {
					diff1 = saudSize - _smushTracks[i].blockSize;
				}

				if (diff1 < diff2 && saudSize < 3 * _smushTracks[i].blockSize / 2) {
					if (_smushTracks[i].blockSize - saudSize >= 0) {
						diff2 = _smushTracks[i].blockSize - saudSize;
					} else {
						diff2 = saudSize - _smushTracks[i].blockSize;
					}

					maxBlockSize = _smushTracks[i].blockSize;
				}
			}
		}

		for (int i = 0; i < _smushNumTracks; i++) {
			flagsAccumulator[i] = 0;
			if (_smushTracks[i].blockSize == maxBlockSize) {
				if (_smushTracks[i].state == TRK_STATE_INACTIVE ||
					_smushTracks[i].state == TRK_STATE_ENDING   ||
					_smushTracks[i].flags <= flags) {
					if (_smushTracks[i].state == TRK_STATE_INACTIVE)
						flagsAccumulator[i] = 0x1000;

					if (_smushTracks[i].state == TRK_STATE_ENDING)
						flagsAccumulator[i] += 0x200;

					if (_smushTracks[i].flags == flags)
						flagsAccumulator[i] += 0x400;

					if (_smushTracks[i].flags < flags)
						flagsAccumulator[i] += 0x800;

					flagsAccumulator[i] += _smushTracks[i].parsedChunks + 1;
				}
			}
		}
		fillAudioTrackInfo(srcBuf, flagsAccumulator, size, groupId, vol, pan, flags, trkId, index, maxFrames);
		return;
	}

	targetTrk = -1;
	for (int i = 0; i < _smushNumTracks; i++) {
		if (_smushTracks[i].state != TRK_STATE_INACTIVE &&
			trkId == _smushTrackIds[i] &&
			index == (_smushTrackIdxs[i] + 1) &&
			maxFrames == _smushMaxFrames[i]) {
			targetTrk = i;
			break;
		}
	}

	if (targetTrk != -1) {
		_smushTrackIdxs[targetTrk]++;
		dataSize = _smushTracks[targetTrk].dataSize;
		mod = _smushTracks[targetTrk].availableSize % dataSize;

		if (mod + size <= dataSize) {
			memcpy(&_smushTracks[targetTrk].subChunkPtr[mod], srcBuf, size);
		} else {
			memcpy(&_smushTracks[targetTrk].subChunkPtr[mod], srcBuf, dataSize - mod);
			memcpy(
				_smushTracks[targetTrk].subChunkPtr,
				&srcBuf[_smushTracks[targetTrk].dataSize - mod],
				size + mod - _smushTracks[targetTrk].dataSize);
		}

		if (vol >= 0 && vol < 128)
			_smushTracks[targetTrk].volume = vol;

		if (pan > -128 && pan < 128)
			_smushTracks[targetTrk].pan = pan;

		_smushTracks[targetTrk].availableSize += size;
	}
}

void SmushPlayer::fillAudioTrackInfo(uint8 *srcBuf, uint16 *flagsAccumulator, uint32 size, int groupId, int vol, int pan, int16 flags, int trkId, int index, int maxFrames) {
	uint32 sdatSize, subChunkOffset, chunkSize;

	int maxFlagAcc = -1;
	int maxAccId = -1;

	for (int i = 0; i < _smushNumTracks; i++) {
		if (flagsAccumulator[i] && flagsAccumulator[i] > maxFlagAcc) {
			maxFlagAcc = flagsAccumulator[i];
			maxAccId = i;
		}
	}

	if (maxAccId != -1) {
		for (int i = 0; i < _smushNumTracks; i++) {
			if (_smushTracks[i].parsedChunks < 255) {
				_smushTracks[i].parsedChunks++;
			}
		}

		_smushTracks[maxAccId].parsedChunks = 0;
		_smushTracks[maxAccId].state = TRK_STATE_INACTIVE;
		_smushTrackIds[maxAccId] = trkId;
		_smushTrackIdxs[maxAccId] = 0;
		_smushMaxFrames[maxAccId] = maxFrames;
		subChunkOffset = READ_BE_UINT32(&srcBuf[12]);
		sdatSize = READ_BE_UINT32(&srcBuf[subChunkOffset + 20]);

		chunkSize = _smushTracks[maxAccId].blockSize;

		if (size < chunkSize)
			chunkSize = size;

		memset(_smushTracks[maxAccId].blockPtr, 127, _smushTracks[maxAccId].blockSize);
		memcpy(_smushTracks[maxAccId].blockPtr, srcBuf, chunkSize);
		_smushTracks[maxAccId].dataBuf = _smushTracks[maxAccId].blockPtr + 16;
		_smushTracks[maxAccId].dataSize = _smushTracks[maxAccId].blockSize - subChunkOffset - 24;
		_smushTracks[maxAccId].subChunkPtr = &_smushTracks[maxAccId].dataBuf[subChunkOffset + 8];
		_smushTracks[maxAccId].availableSize = size - subChunkOffset - 24;
		_smushTracks[maxAccId].sdatSize = sdatSize;
		_smushTracks[maxAccId].groupId = groupId;
		_smushTracks[maxAccId].volume = 127;
		_smushTracks[maxAccId].pan = 0;

		if (vol >= 0 && vol < 128)
			_smushTracks[maxAccId].volume = vol;

		if (pan > -128 && pan < 128)
			_smushTracks[maxAccId].pan = pan;

		_smushTracks[maxAccId].flags = flags;
		_smushTracks[maxAccId].audioRemaining = 0;
		_smushTracks[maxAccId].state = TRK_STATE_FADING;
	}

	return;
}

void SmushPlayer::processDispatches(int16 feedSize) {
	int32 fadeStartOffset, cpySize, fadeRemaining, fadeFeedSize, mixFeedSize, mixInFrameCount;
	int32 offset, tmpFeedSize, maxFadeChunkSize;
	int16 fadeInFrameCount, flags, fadeMixStartingPoint;

	int fadePan, fadeVolume, mixPan, mixVolume, baseVolume, mixStartingPoint;

	bool isPlayableTrack;
	bool speechIsPlaying = false;

	int engineBaseFeedSize = _imuseDigital->getFeedSize();

	if (!_paused) {
		if (_smushTracksNeedInit) {
			_smushTracksNeedInit = false;
			for (int i = 0; i < SMUSH_MAX_TRACKS; i++) {
				_smushDispatch[i].fadeRemaining = 0;
				_smushDispatch[i].fadeVolume = 0;
				_smushDispatch[i].fadeSampleRate = 0;
				_smushDispatch[i].elapsedAudio = 0;
				_smushDispatch[i].audioLength = 0;
			}
		}

		for (int i = 0; i < _smushNumTracks; i++) {
			isPlayableTrack = ((_smushTracks[i].flags & TRK_TYPE_MASK) == IS_SPEECH    && isChanActive(CHN_SPEECH)) ||
							  ((_smushTracks[i].flags & TRK_TYPE_MASK) == IS_BKG_MUSIC && isChanActive(CHN_BKGMUS)) ||
							  ((_smushTracks[i].flags & TRK_TYPE_MASK) == IS_SFX       && isChanActive(CHN_OTHER));
			flags = _smushTracks[i].flags;

			switch (flags & TRK_TYPE_MASK) {
			case IS_SFX:
				baseVolume = (_smushTrackVols[1] * _smushTracks[i].volume) >> 7;
				break;
			case IS_BKG_MUSIC:
				baseVolume = (_smushTrackVols[3] * _smushTracks[i].volume) >> 7;
				break;
			case IS_SPEECH:
				baseVolume = (_smushTrackVols[2] * _smushTracks[i].volume) >> 7;
				break;
			default:
				error("SmushPlayer::processDispatches(): unrecognized flag %d", _smushTracks[i].flags & TRK_TYPE_MASK);
			}

			mixVolume = baseVolume * _smushTrackVols[0] / 127;
			if ((flags & TRK_TYPE_MASK) == IS_BKG_MUSIC && isChanActive(CHN_SPEECH))
				mixVolume = ((baseVolume * _smushTrackVols[0] / 127) * _gainReductionMultiplier) >> 8;

			// Check if there's the need to allocate a crossfade
			if (_smushTracks[i].state == TRK_STATE_FADING && _smushDispatch[i].state == TRK_STATE_PLAYING) {
				fadeStartOffset = _smushDispatch[i].audioRemaining % _smushDispatch[i].dataSize;
				_smushDispatch[i].fadeRemaining = SMUSH_FADE_SIZE;
				_smushDispatch[i].fadeVolume = _smushTracks[i].volume;
				_smushDispatch[i].fadeSampleRate = _smushDispatch[i].sampleRate;

				memset(_smushTracks[i].fadeBuf, 127, _smushDispatch[i].fadeRemaining);
				cpySize = _smushDispatch[i].dataSize - fadeStartOffset;

				if (cpySize > _smushDispatch[i].fadeRemaining)
					cpySize = _smushDispatch[i].fadeRemaining;
				memcpy(_smushTracks[i].fadeBuf, &_smushDispatch[i].dataBuf[fadeStartOffset], cpySize);

				_smushDispatch[i].volumeStep = 0;
			} else if (_smushTracks[i].state == TRK_STATE_PLAYING) {
				if (_smushDispatch[i].audioRemaining < _smushTracks[i].availableSize - _smushTracks[i].dataSize + 15000) {
					if (_smushTracks[i].availableSize < _smushTracks[i].sdatSize) {
						_smushDispatch[i].volumeStep = 0;
						mixInFrameCount = _smushTracks[i].availableSize - _smushDispatch[i].audioRemaining - 15000;

						if (mixInFrameCount > _smushDispatch[i].currentOffset)
							mixInFrameCount = _smushDispatch[i].currentOffset;

						if (_smushDispatch[i].audioRemaining + mixInFrameCount > _smushTracks[i].sdatSize - _smushDispatch[i].dataSize)
							mixInFrameCount = _smushTracks[i].sdatSize - _smushDispatch[i].dataSize - _smushDispatch[i].audioRemaining;

						if (mixInFrameCount > 0) {
							_smushDispatch[i].fadeRemaining = SMUSH_FADE_SIZE;
							_smushDispatch[i].fadeSampleRate = _smushDispatch[i].sampleRate;

							memcpy(
								_smushTracks[i].fadeBuf,
								&_smushDispatch[i].dataBuf[_smushDispatch[i].audioRemaining % _smushDispatch[i].dataSize],
								_smushDispatch[i].fadeRemaining);

							_smushDispatch[i].audioRemaining += mixInFrameCount;
							_smushDispatch[i].currentOffset -= mixInFrameCount;
						}
					}
				}
			}

			// If the fade has been allocated, flush it in the mixer
			if (_smushDispatch[i].fadeRemaining) {
				maxFadeChunkSize = _smushDispatch[i].fadeSampleRate * feedSize / _smushAudioSampleRate;

				if (_smushDispatch[i].fadeRemaining > maxFadeChunkSize) {
					fadeRemaining = maxFadeChunkSize;
				} else {
					fadeRemaining = _smushDispatch[i].fadeRemaining;
				}

				fadeMixStartingPoint = 0;
				while (fadeRemaining) {
					fadeInFrameCount = (fadeRemaining < engineBaseFeedSize / 4) ? fadeRemaining : engineBaseFeedSize / 4;

					if (fadeInFrameCount == maxFadeChunkSize) {
						fadeFeedSize = feedSize;
					} else {
						fadeFeedSize = _smushAudioSampleRate * fadeInFrameCount / _smushDispatch[i].fadeSampleRate;
					}

					if (isPlayableTrack) {
						fadeVolume = _smushDispatch[i].fadeRemaining * _smushDispatch[i].fadeVolume * _smushTrackVols[0] / (SMUSH_FADE_SIZE * 127);
						fadePan = _smushTracks[i].pan;

						debug(5, "SmushPlayer::processDispatches(): fading dispatch %d, volume %d", i, fadeVolume);

						sendAudioToDiMUSE(
							&_smushTracks[i].fadeBuf[SMUSH_FADE_SIZE - _smushDispatch[i].fadeRemaining],
							fadeMixStartingPoint,
							fadeFeedSize,
							fadeInFrameCount,
							fadeVolume,
							fadePan);
					}

					fadeMixStartingPoint += fadeFeedSize;
					fadeRemaining -= fadeInFrameCount;
					_smushDispatch[i].fadeRemaining -= fadeInFrameCount;
				}
			}

			if (_smushTracks[i].state == TRK_STATE_FADING) {
				if (_smushDispatch[i].state != TRK_STATE_PLAYING)
					_smushDispatch[i].volumeStep = 16;

				_smushDispatch[i].headerPtr = _smushTracks[i].dataBuf;
				_smushDispatch[i].dataBuf = _smushTracks[i].subChunkPtr;
				_smushDispatch[i].dataSize = _smushTracks[i].dataSize;
				_smushDispatch[i].currentOffset = 0;
				_smushDispatch[i].audioLength = 0;
				_smushTracks[i].state = TRK_STATE_PLAYING;
			}

			if (_smushTracks[i].state != TRK_STATE_INACTIVE) {
				tmpFeedSize = feedSize;
				mixStartingPoint = 0;
				if (feedSize > 0) {
					while (1) {
						mixInFrameCount = _smushDispatch[i].currentOffset;
						if (mixInFrameCount > 0) {
							offset = _smushDispatch[i].audioRemaining % _smushDispatch[i].dataSize;

							if (mixInFrameCount > _smushDispatch[i].sampleRate * tmpFeedSize / _smushAudioSampleRate)
								mixInFrameCount = _smushDispatch[i].sampleRate * tmpFeedSize / _smushAudioSampleRate;

							if (offset + mixInFrameCount > _smushDispatch[i].dataSize)
								mixInFrameCount = _smushDispatch[i].dataSize - offset;

							if (mixInFrameCount + _smushDispatch[i].audioRemaining <= _smushTracks[i].availableSize) {
								// Fade-in until full volume is reached
								if (_smushDispatch[i].volumeStep < 16) {
									_smushDispatch[i].volumeStep++;
									debug(5, "SmushPlayer::processDispatches(): fading track %d, volume step %d", i, _smushDispatch[i].volumeStep);
								}

								if (mixInFrameCount > engineBaseFeedSize / 4)
									mixInFrameCount = engineBaseFeedSize / 4;

								_smushTracks[i].state = TRK_STATE_PLAYING;

								// This flag is toggled one time per chunk: if it's yields "true" for
								// even one track, it stays like that for the whole chunk
								speechIsPlaying = !speechIsPlaying ? (_smushTracks[i].flags & TRK_TYPE_MASK) == IS_SPEECH : true;
							} else {
								// Fade-out until silent
								if (_smushDispatch[i].volumeStep) {
									_smushDispatch[i].volumeStep--;
									debug(5, "SmushPlayer::processDispatches(): fading track %d, volume step %d", i, _smushDispatch[i].volumeStep);
								}

								_smushTracks[i].state = TRK_STATE_ENDING;

								if (mixInFrameCount > engineBaseFeedSize / 4)
									mixInFrameCount = engineBaseFeedSize / 4;

								_smushDispatch[i].audioRemaining -= mixInFrameCount;
								_smushDispatch[i].currentOffset += mixInFrameCount;
								offset = _smushDispatch[i].audioRemaining % _smushDispatch[i].dataSize;
							}

							if (mixInFrameCount == _smushDispatch[i].sampleRate * tmpFeedSize / _smushAudioSampleRate) {
								mixFeedSize = tmpFeedSize;
							} else {
								mixFeedSize = mixInFrameCount * _smushAudioSampleRate / _smushDispatch[i].sampleRate;
							}

							if (isPlayableTrack) {
								mixPan = _smushTracks[i].pan;
								sendAudioToDiMUSE(
									&_smushDispatch[i].dataBuf[offset],
									mixStartingPoint,
									mixFeedSize,
									mixInFrameCount,
									(mixVolume * _smushDispatch[i].volumeStep) >> 4,
									mixPan);
							}

							_smushDispatch[i].currentOffset -= mixInFrameCount;
							_smushDispatch[i].audioRemaining += mixInFrameCount;
							tmpFeedSize -= mixFeedSize;
							mixStartingPoint += mixFeedSize;
						}

						if (_smushDispatch[i].currentOffset <= 0) {
							if (processAudioCodes(i, tmpFeedSize, mixVolume) && tmpFeedSize <= 0) {
								break;
							}
						} else if (tmpFeedSize <= 0) {
							break;
						}
					}
				}
			}

			_smushTracks[i].audioRemaining = _smushDispatch[i].audioRemaining;
			_smushDispatch[i].state = _smushTracks[i].state;
		}

		if (speechIsPlaying) {
			if (_gainReductionMultiplier > _gainReductionLowerBound) {
				_gainReductionMultiplier -= (feedSize * 2 * _gainReductionFactor) >> 13;
				if (_gainReductionMultiplier < _gainReductionLowerBound)
					_gainReductionMultiplier = _gainReductionLowerBound;
			}
		} else {
			if (_gainReductionMultiplier < 256) {
				_gainReductionMultiplier += (feedSize * 2 * _gainReductionFactor) >> 15;
				if (_gainReductionMultiplier > 256)
					_gainReductionMultiplier = 256;
			}
		}
	}
}

bool SmushPlayer::processAudioCodes(int idx, int32 &tmpFeedSize, int &mixVolume) {
	uint8 *code, *buf, subcode, value;
	int chunk;

	while (tmpFeedSize) {
		code = _smushDispatch[idx].headerPtr;

		switch (code[0]) {
		case SAUD_OP_INIT:
			_smushDispatch[idx].audioLength = 0;
			buf = _smushDispatch[idx].headerPtr;
			_smushDispatch[idx].audioRemaining = READ_BE_UINT32(buf + 2);
			_smushDispatch[idx].currentOffset = READ_BE_UINT32(buf + 6);
			_smushDispatch[idx].sampleRate = _smushAudioSampleRate;
			_smushDispatch[idx].headerPtr += _smushDispatch[idx].headerPtr[1] + 2;
			if (_smushDispatch[idx].audioRemaining < _smushTracks[idx].availableSize + (_smushTracks[idx].availableSize >= _smushTracks[idx].sdatSize ? 0 : 15000) - _smushTracks[idx].dataSize) {
				chunk = _smushTracks[idx].availableSize - _smushTracks[idx].dataSize - _smushDispatch[idx].audioRemaining + 15000;
				if (chunk > _smushDispatch[idx].currentOffset) {
					_smushTracks[idx].state = TRK_STATE_INACTIVE;
					_smushTracks[idx].groupId = GRP_MASTER;
					tmpFeedSize = 0;
					break;
				}

				_smushDispatch[idx].audioRemaining += chunk;
				_smushDispatch[idx].currentOffset -= chunk;
			}
			break;

		case SAUD_OP_UPDATE_HEADER:
		case SAUD_OP_COMPARE_GT:
		case SAUD_OP_COMPARE_LT:
		case SAUD_OP_COMPARE_EQ:
		case SAUD_OP_COMPARE_NE:
			subcode = code[4];
			switch (subcode) {
			case SAUD_VALUEID_ALL_VOLS:
				value = _smushTrackVols[0];
				break;
			case SAUD_VALUEID_TRK_VOL:
				value = _smushTracks[idx].volume;
				break;
			case SAUD_VALUEID_TRK_PAN:
				value = _smushTracks[idx].pan;
				break;
			default:
				value = _smushAudioTable[subcode];
				break;
			}

			switch (code[0]) {
			case SAUD_OP_UPDATE_HEADER:
				if (value || (subcode == 0)) {
					_smushDispatch[idx].headerPtr = &code[READ_BE_UINT16(&code[2])];
				}
				break;
			case SAUD_OP_COMPARE_GT:
				value = value > code[5];
				break;
			case SAUD_OP_COMPARE_LT:
				value = value < code[5];
				break;
			case SAUD_OP_COMPARE_EQ:
				value = value == code[5];
				break;
			case SAUD_OP_COMPARE_NE:
				value = value != code[5];
				break;
			default:
				break;
			}

			if (!value) {
				_smushDispatch[idx].headerPtr = &code[code[1] + 2];
			} else {
				_smushDispatch[idx].headerPtr = &code[READ_BE_UINT16(&code[2])];
			}
			break;

		case SAUD_OP_SET_PARAM:
			switch (code[2]) {
			case SAUD_VALUEID_ALL_VOLS:
				_smushTrackVols[0] = code[3];
				break;
			case SAUD_VALUEID_TRK_VOL:
				_smushTracks[idx].volume = code[3];
				mixVolume = (_smushTrackVols[0] * _smushTracks[idx].volume) / 127;

				// Set a lower mix volume of the background music if speech is active
				if ((_smushTracks[idx].flags & TRK_TYPE_MASK) == IS_BKG_MUSIC && isChanActive(CHN_SPEECH))
					mixVolume = (mixVolume * _gainReductionMultiplier) >> 8;
				break;
			case SAUD_VALUEID_TRK_PAN:
				_smushTracks[idx].pan = code[3];
				break;
			default:
				_smushAudioTable[code[2]] = code[3];
				break;
			}
			_smushDispatch[idx].headerPtr = &code[code[1] + 2];
			break;

		case SAUD_OP_INCR_PARAM:
			switch (code[2]) {
			case SAUD_VALUEID_ALL_VOLS:
				_smushTrackVols[0] += code[3];
				break;
			case SAUD_VALUEID_TRK_VOL:
				_smushTracks[idx].volume += code[3];
				break;
			case SAUD_VALUEID_TRK_PAN:
				_smushTracks[idx].pan += code[3];
				break;
			default:
				_smushAudioTable[code[2]] += code[3];
				break;
			}
			_smushDispatch[idx].headerPtr = &code[code[1] + 2];
			break;

		case SAUD_OP_SET_OFFSET:
			_smushDispatch[idx].audioLength = 0;
			buf = _smushDispatch[idx].headerPtr;
			_smushDispatch[idx].audioRemaining = READ_BE_UINT32(buf + 2);
			_smushDispatch[idx].currentOffset = READ_BE_UINT32(buf + 6);
			_smushDispatch[idx].sampleRate = _smushAudioSampleRate;

			_smushDispatch[idx].headerPtr += _smushDispatch[idx].headerPtr[1] + 2;
			if (_smushDispatch[idx].audioRemaining < _smushTracks[idx].availableSize + (_smushTracks[idx].availableSize >= _smushTracks[idx].sdatSize ? 0 : 15000) - _smushTracks[idx].dataSize) {
				chunk = _smushTracks[idx].availableSize - _smushTracks[idx].dataSize - _smushDispatch[idx].audioRemaining + 15000;
				if (chunk > _smushDispatch[idx].currentOffset) {
					_smushTracks[idx].state = TRK_STATE_INACTIVE;
					_smushTracks[idx].groupId = GRP_MASTER;
					tmpFeedSize = 0;
					break;
				}

				_smushDispatch[idx].audioRemaining += chunk;
				_smushDispatch[idx].currentOffset -= chunk;
			}
			break;

		case SAUD_OP_SET_LENGTH:
			if (!_smushDispatch[idx].audioLength) {
				_smushDispatch[idx].audioLength = READ_BE_UINT32(&code[6]);
				_smushDispatch[idx].elapsedAudio = 0;
			}

			buf = _smushDispatch[idx].headerPtr;
			_smushDispatch[idx].audioRemaining = _smushDispatch[idx].elapsedAudio + READ_BE_UINT32(buf + 2);

			_smushDispatch[idx].currentOffset = READ_BE_UINT32(buf + 14);
			if (_smushDispatch[idx].currentOffset > _smushDispatch[idx].audioLength)
				_smushDispatch[idx].currentOffset = _smushDispatch[idx].audioLength;

			_smushDispatch[idx].sampleRate = _smushAudioSampleRate;

			_smushDispatch[idx].audioLength -= _smushDispatch[idx].currentOffset;
			_smushDispatch[idx].elapsedAudio += _smushDispatch[idx].currentOffset;

			if (_smushDispatch[idx].audioLength) {
				_smushDispatch[idx].headerPtr = &code[code[1] + 2];
			} else {
				_smushDispatch[idx].headerPtr = &code[READ_BE_UINT16(&code[18])];
			}

			if (_smushDispatch[idx].audioRemaining >= _smushTracks[idx].availableSize + (_smushTracks[idx].availableSize >= _smushTracks[idx].sdatSize ? 0 : 15000) - _smushTracks[idx].dataSize) {
				chunk = _smushTracks[idx].availableSize - _smushTracks[idx].dataSize - _smushDispatch[idx].audioRemaining + 15000;
				if (chunk > _smushDispatch[idx].currentOffset) {
					_smushTracks[idx].state = TRK_STATE_INACTIVE;
					_smushTracks[idx].groupId = GRP_MASTER;
					tmpFeedSize = 0;
				} else {
					_smushDispatch[idx].audioRemaining += chunk;
					_smushDispatch[idx].currentOffset -= chunk;
				}
			}
			break;

		default:
			_smushTracks[idx].state = TRK_STATE_INACTIVE;
			_smushTracks[idx].groupId = GRP_MASTER;
			tmpFeedSize = 0;
		}

		if (_smushDispatch[idx].currentOffset > 0) {
			return false;
		}
	}

	return true;
}

void SmushPlayer::sendAudioToDiMUSE(uint8 *mixBuf, int32 mixStartingPoint, int32 mixFeedSize, int32 mixInFrameCount, int volume, int pan) {
	int clampedVol, clampedPan;
	bool is11025Hz = false;

	if (mixFeedSize == 2 * mixInFrameCount) {
		is11025Hz = true;
	} else if (mixFeedSize != mixInFrameCount) {
		return;
	}

	clampedPan = CLIP<int>((pan / 2) + 64, 0, 127);
	clampedVol = CLIP<int>(volume, 0, 127);
	_imuseDigital->receiveAudioFromSMUSH(mixBuf, mixInFrameCount, mixFeedSize, mixStartingPoint, clampedVol, clampedPan, is11025Hz);
}

void SmushPlayer::feedAudio(uint8 *srcBuf, int groupId, int volume, int pan, int16 flags) {
	int panDelta, effPan;
	int32 maxFrames;
	uint16 trkId, index;

	if (_smushAudioInitialized) {
		// Check file encoding
		if (srcBuf[8] == 0 && srcBuf[9] == 0 && srcBuf[12] == 0 && srcBuf[13] == 0 && srcBuf[16] == 0 && srcBuf[17] == 0) {
			trkId = READ_BE_INT16(&srcBuf[10]);
			index = READ_BE_INT16(&srcBuf[14]);
			maxFrames = READ_BE_INT16(&srcBuf[18]);

			handleSAUDChunk(
				srcBuf + 20,
				READ_BE_UINT32(&srcBuf[4]) - 12,
				groupId,
				volume,
				pan,
				flags,
				trkId,
				index,
				maxFrames);
		} else {
			trkId = READ_LE_INT16(&srcBuf[8]);
			index = READ_LE_INT16(&srcBuf[10]);
			maxFrames = READ_LE_INT16(&srcBuf[12]);
			flags |= READ_LE_INT16(&srcBuf[14]);
			volume = (volume * srcBuf[16]) >> 7;
			panDelta = srcBuf[17];

			if (panDelta == 128) {
				effPan = 128;
			} else {
				effPan = pan + panDelta;
			}

			handleSAUDChunk(
				srcBuf + 18,
				READ_BE_UINT32(&srcBuf[4]) - 10,
				groupId,
				(volume * srcBuf[16]) >> 7,
				effPan,
				flags,
				trkId,
				index,
				maxFrames);
		}
	}
}

bool SmushPlayer::isAudioCallbackEnabled() {
	return _smushAudioCallbackEnabled;
}

} // End of namespace Scumm
