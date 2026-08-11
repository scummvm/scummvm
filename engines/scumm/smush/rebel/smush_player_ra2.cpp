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
#include "common/endian.h"
#include "common/rect.h"
#include "common/system.h"

#include "graphics/blit.h"

#include "scumm/scumm.h"
#include "scumm/scumm_v7.h"
#include "scumm/smush/codec37.h"
#include "scumm/smush/codec47.h"
#include "scumm/smush/smush_font.h"
#include "scumm/smush/rebel/smush_multi_font.h"
#include "scumm/smush/rebel/codec_ra2.h"
#include "scumm/smush/rebel/smush_player_ra2.h"

#include "scumm/insane/insane.h"
#include "scumm/insane/rebel2/rebel.h"

namespace Scumm {

constexpr int kRebel2LoadBufferSize = 400000;
constexpr int kRebel2GameplaySurfaceWidth = 0x1a8;
constexpr int kRebel2GameplaySurfaceHeight = 0x104;
constexpr int kRebel2MusicTrackSize = 800000;

bool isRebel2FullFrameDeltaCodec(int codec) {
	return codec == SMUSH_CODEC_DELTA_BLOCKS || codec == SMUSH_CODEC_DELTA_GLYPHS;
}

bool isRebel2GameplayActive(Insane *insane) {
	if (insane == nullptr)
		return false;

	return static_cast<InsaneRebel2 *>(insane)->getHandler() != 0;
}

bool isRebel2Handler25CorridorMode(Insane *insane) {
	if (insane == nullptr)
		return false;

	InsaneRebel2 *rebel2 = static_cast<InsaneRebel2 *>(insane);
	return rebel2->getHandler() == 25 && rebel2->getHandler25GrdSpriteMode() == 3;
}

static void scaleNativeViewportToHiRes(byte *dst, int dstPitch, int dstWidth, int dstHeight,
		const byte *src, int srcPitch, int srcWidth, int srcHeight, int scrollX, int scrollY) {
	if (!dst || !src || dstPitch <= 0 || dstWidth < 640 || dstHeight < 400 ||
			srcPitch <= 0 || srcWidth <= 0 || srcHeight <= 0)
		return;

	memset(dst, 0, (size_t)dstPitch * dstHeight);

	scrollX = CLIP<int>(scrollX, 0, MAX<int>(0, srcWidth - 1));
	scrollY = CLIP<int>(scrollY, 0, MAX<int>(0, srcHeight - 1));

	const int outWidth = MIN<int>(320, dstWidth / 2);
	const int outHeight = MIN<int>(200, dstHeight / 2);
	const int srcViewWidth = MIN<int>(outWidth, srcWidth - scrollX);
	const int srcViewHeight = MIN<int>(outHeight, srcHeight - scrollY);
	if (srcViewWidth <= 0 || srcViewHeight <= 0)
		return;

	const byte *srcView = src + scrollY * srcPitch + scrollX;
	Graphics::scaleBlit(dst, srcView, dstPitch, srcPitch,
		srcViewWidth * 2, srcViewHeight * 2, srcViewWidth, srcViewHeight,
		Graphics::PixelFormat::createFormatCLUT8());
}

void smushDecodeRA2Uncompressed(byte *dst, const byte *src, int left, int top,
		int width, int height, int dstPitch, int srcPitch, int dataSize,
		int srcSkipX, int srcSkipY) {
	if (dst == nullptr || src == nullptr || width <= 0 || height <= 0 ||
			dstPitch <= 0 || srcPitch <= 0) {
		return;
	}

	const int64 srcOffset = (int64)srcSkipY * srcPitch + srcSkipX;
	if (srcOffset < 0 || (dataSize > 0 && srcOffset >= dataSize))
		return;

	if (dataSize > 0) {
		const int64 bytesAfterOffset = (int64)dataSize - srcOffset;
		if (bytesAfterOffset < width)
			return;

		const int maxRows = (int)((bytesAfterOffset - width) / srcPitch) + 1;
		height = MIN(height, maxRows);
		if (height <= 0)
			return;
	}

	src += srcOffset;
	dst += top * dstPitch + left;
	while (height-- > 0) {
		memcpy(dst, src, width);
		src += srcPitch;
		dst += dstPitch;
	}
}

SmushPlayerRebel2::SmushPlayerRebel2(ScummEngine_v7 *scumm, IMuseDigital *imuseDigital, Insane *insane)
	: SmushPlayer(scumm, imuseDigital, insane) {
	initGamePlayerFields();
	ra2InitAudioTrackSizes();
}

SmushPlayerRebel2::~SmushPlayerRebel2() {
	destroyGamePlayerFields();
}

void SmushPlayerRebel2::initGamePlayerFields() {
	_multiFont = nullptr;
	_storedFobjData = nullptr;
	_storedFobjDataSize = 0;
	_storedFobjCodec = 0;
	_storedFobjParm2 = 0;
	_storedFobjLeft = 0;
	_storedFobjTop = 0;
	_storedFobjWidth = 0;
	_storedFobjHeight = 0;
	_lastFobjData = nullptr;
	_lastFobjDataSize = 0;
	_lastFobjCodec = 0;
	_lastFobjLeft = 0;
	_lastFobjTop = 0;
	_lastFobjWidth = 0;
	_lastFobjHeight = 0;
	_hasFrameFobjForGost = false;
	_skipNext = false;
	_fobjOffsetX = 0;
	_fobjOffsetY = 0;
	_storeFrame = false;
	_loadBuffer = nullptr;
	_loadBufferSize = 0;
	_loadBufferOffset = 0;
	_loadReadOffset = 8;
	_lastLoadChunkIdx = -1;
	_loadStreamId = 0;
	_ra2FrameSourceSkipX = 0;
	_ra2FrameSourceSkipY = 0;
	_ra2FrameObjectOriginalWidth = 0;
	_ra2FrameObjectOriginalHeight = 0;
	_ra2FrameObjectSurfaceWidth = 0;
	_ra2FrameObjectSurfaceHeight = 0;
	_ra2DeltaBlocksWidth = 0;
	_ra2DeltaBlocksHeight = 0;
	_ra2DeltaGlyphsWidth = 0;
	_ra2DeltaGlyphsHeight = 0;
	_ra2LowResVideoBuffer = nullptr;
	_ra2LowResVideoBufferSize = 0;
	_ra2NativeFrameNeedsClear = false;
	_ra2UsingGameplaySurface = false;
	_ra2PendingAnimHeaderPalette = false;
	memset(_ra2Codec45Palette, 0, sizeof(_ra2Codec45Palette));
	memset(_ra2Codec45Lookup, 0, sizeof(_ra2Codec45Lookup));
	memset(_ra2SkipRemapTable, 0, sizeof(_ra2SkipRemapTable));
	_ra2SkipRemapValid = false;
	_scrollX = 0;
	_scrollY = 0;
}

void SmushPlayerRebel2::destroyGamePlayerFields() {
	delete _multiFont;
	_multiFont = nullptr;
	free(_storedFobjData);
	_storedFobjData = nullptr;
	free(_lastFobjData);
	_lastFobjData = nullptr;
	free(_loadBuffer);
	_loadBuffer = nullptr;
	free(_ra2LowResVideoBuffer);
	_ra2LowResVideoBuffer = nullptr;
	_ra2LowResVideoBufferSize = 0;
}

void SmushPlayerRebel2::ra2InitAudioTrackSizes() {
	const int musicTrack = SMUSH_MAX_TRACKS - 1;
	if (_smushNumTracks <= musicTrack || _smushTracks[musicTrack].blockSize >= kRebel2MusicTrackSize)
		return;

	uint8 *blockPtr = (uint8 *)realloc(_smushTracks[musicTrack].blockPtr, kRebel2MusicTrackSize);
	if (!blockPtr) {
		warning("SmushPlayerRebel2::ra2InitAudioTrackSizes: failed to allocate %d-byte music track",
			kRebel2MusicTrackSize);
		return;
	}

	_smushTracks[musicTrack].blockPtr = blockPtr;
	_smushTracks[musicTrack].blockSize = kRebel2MusicTrackSize;
	memset(_smushTracks[musicTrack].blockPtr, 127, _smushTracks[musicTrack].blockSize);
}

void SmushPlayerRebel2::initGameVideoState() {
	_ra2PendingAnimHeaderPalette = false;
	_ra2UsingGameplaySurface = false;
	_smushAudioTable[100] = 0;

	// Some menu videos inherit the previous SMUSH palette.
	setDirtyColors(0, 255);

	_width = _vm->_screenWidth;
	_height = _vm->_screenHeight;

	if (_dst != nullptr) {
		VirtScreen *vs = &_vm->_virtscr[kMainVirtScreen];
		if ((_curVideoFlags & 0x20) == 0) {
			memset(_dst, 0, vs->w * vs->h);
		}
	}
}

void SmushPlayerRebel2::releaseGameVideoState() {
	free(_lastFobjData);
	_lastFobjData = nullptr;
	_lastFobjDataSize = 0;
	_hasFrameFobjForGost = false;
	_ra2NativeFrameNeedsClear = false;
}

bool SmushPlayerRebel2::ra2IsHighResMode() const {
	return _vm->_screenWidth >= 640 && _vm->_screenHeight >= 400;
}

bool SmushPlayerRebel2::ra2EnsureLowResVideoBuffer() {
	const int bufSize = 320 * 200;
	if (_ra2LowResVideoBuffer != nullptr && _ra2LowResVideoBufferSize >= bufSize)
		return true;

	byte *newBuffer = (byte *)realloc(_ra2LowResVideoBuffer, bufSize);
	if (!newBuffer) {
		warning("SmushPlayerRebel2::ra2EnsureLowResVideoBuffer: Failed to allocate %d-byte decode buffer", bufSize);
		free(_ra2LowResVideoBuffer);
		_ra2LowResVideoBuffer = nullptr;
		_ra2LowResVideoBufferSize = 0;
		return false;
	}

	_ra2LowResVideoBuffer = newBuffer;
	_ra2LowResVideoBufferSize = bufSize;
	memset(_ra2LowResVideoBuffer, 0, _ra2LowResVideoBufferSize);
	return true;
}

void SmushPlayerRebel2::ra2ClearCurrentTarget() {
	if (!_dst)
		return;

	// Match the off-screen buffers on identity alone: they can be smaller than the
	// screen (424x260 against 640x400), so a bad _width/_height must not reach the
	// screen-sized clear below.
	int clearSize = 0;
	if (_dst == _specialBuffer) {
		const int64 size64 = (int64)_width * _height;
		if (_width > 0 && _height > 0 && size64 <= INT_MAX && size64 <= _specialBufferSize)
			clearSize = (int)size64;
	} else if (_dst == _ra2LowResVideoBuffer) {
		const int64 size64 = (int64)_width * _height;
		if (_width > 0 && _height > 0 && size64 <= INT_MAX && size64 <= _ra2LowResVideoBufferSize)
			clearSize = (int)size64;
	} else {
		const int64 screen64 = (int64)_vm->_screenWidth * _vm->_screenHeight;
		if (screen64 > 0 && screen64 <= INT_MAX)
			clearSize = (int)screen64;
	}

	if (clearSize > 0)
		memset(_dst, 0, clearSize);
}

bool SmushPlayerRebel2::ra2PromoteCurrentFrameToHiRes(int scrollX, int scrollY) {
	if (!ra2IsHighResMode() || !_dst || _width <= 0 || _height <= 0)
		return false;

	VirtScreen *vs = &_vm->_virtscr[kMainVirtScreen];
	byte *screen = vs->getPixels(0, 0);
	if (_dst == screen && _width == _vm->_screenWidth && _height == _vm->_screenHeight)
		return false;

	const byte *src = _dst;
	const int srcPitch = _width;
	const int srcWidth = _width;
	const int srcHeight = _height;

	scaleNativeViewportToHiRes(screen, _vm->_screenWidth, _vm->_screenWidth, _vm->_screenHeight,
		src, srcPitch, srcWidth, srcHeight, scrollX, scrollY);

	_dst = screen;
	_width = _vm->_screenWidth;
	_height = _vm->_screenHeight;
	setScrollOffset(0, 0);
	return true;
}

bool SmushPlayerRebel2::ra2PromoteHandler7PerspectiveToHiRes(int perspectiveX, int perspectiveY, int viewShift) {
	if (!ra2IsHighResMode() || !_dst || _width <= 0 || _height <= 0)
		return false;

	VirtScreen *vs = &_vm->_virtscr[kMainVirtScreen];
	byte *screen = vs->getPixels(0, 0);
	if (_dst == screen && _width == _vm->_screenWidth && _height == _vm->_screenHeight)
		return false;

	if (!ra2EnsureLowResVideoBuffer())
		return false;

	const byte *src = _dst;
	const int srcPitch = _width;
	const int srcWidth = _width;
	const int srcHeight = _height;

	memset(_ra2LowResVideoBuffer, 0, _ra2LowResVideoBufferSize);
	copyRA2Handler7PerspectiveViewport(_ra2LowResVideoBuffer, 320, 320, 200,
		src, srcPitch, srcWidth, srcHeight, perspectiveX, perspectiveY, viewShift);

	scaleNativeViewportToHiRes(screen, _vm->_screenWidth, _vm->_screenWidth, _vm->_screenHeight,
		_ra2LowResVideoBuffer, 320, 320, 200, 0, 0);

	_dst = screen;
	_width = _vm->_screenWidth;
	_height = _vm->_screenHeight;
	setScrollOffset(0, 0);
	return true;
}

bool SmushPlayerRebel2::handleGameFetch(int32 subSize, Common::SeekableReadStream &b) {
	int16 ftchUnknown = b.readSint16LE();
	int16 ftchX = b.readSint16LE();
	int16 ftchY = b.readSint16LE();

	debugC(DEBUG_SMUSH, "SmushPlayerRebel2::handleGameFetch: frame=%d unknown=%d x=%d y=%d",
		_frame, ftchUnknown, ftchX, ftchY);

	// For Handler 25, skip FTCH because the frame buffer only contains the
	// par4=5 base background without the overlays (par4=4, 6, 7) that were drawn
	// immediately in frame 0. Restoring would erase those overlays.
	if (_insane != nullptr) {
		InsaneRebel2 *rebel2 = static_cast<InsaneRebel2 *>(_insane);
		int handler = rebel2->getHandler();
		if (handler == 25) {
			debugC(DEBUG_SMUSH, "SmushPlayerRebel2::handleGameFetch: Skipping FTCH for Handler 25 - preserving overlays");
			return true;
		}
	}

	// Re-decode stored FOBJ data with current offsets.
	if (_storedFobjData != nullptr) {
		debugC(DEBUG_SMUSH, "SmushPlayerRebel2::handleGameFetch FTCH: Re-decoding stored FOBJ codec=%d pos=(%d,%d) size=%dx%d dataSize=%d",
			_storedFobjCodec, _storedFobjLeft, _storedFobjTop,
			_storedFobjWidth, _storedFobjHeight, _storedFobjDataSize);
		ra2PrepareFrameObjectSurface(_storedFobjLeft, _storedFobjTop,
			_storedFobjWidth, _storedFobjHeight);
		decodeFrameObject(_storedFobjCodec, _storedFobjData,
			_storedFobjLeft, _storedFobjTop,
			_storedFobjWidth, _storedFobjHeight,
			_storedFobjDataSize);
	} else {
		debugC(DEBUG_SMUSH, "SmushPlayerRebel2::handleGameFetch FTCH: No stored FOBJ data! (frame=%d)", _frame);
	}

	return true;
}

bool SmushPlayerRebel2::handleGameTextResource(uint32 subType, int32 subSize, Common::SeekableReadStream &b) {
	if (subType != MKTAG('T','E','X','T') && subType != MKTAG('T','R','E','S'))
		return false;

	if (subSize < 16) {
		b.skip(subSize);
		return true;
	}

	int posX = b.readSint16LE();
	int posY = b.readSint16LE();
	int flags = b.readSint16LE();
	int left = b.readSint16LE();
	int top = b.readSint16LE();
	int width = b.readSint16LE();
	int height = b.readSint16LE();
	b.readUint16LE();

	const char *str = nullptr;
	char *text = nullptr;
	int consumed = 16;

	if (subType == MKTAG('T','E','X','T')) {
		const int textSize = subSize - consumed;
		if (textSize > 0) {
			text = (char *)malloc(textSize + 1);
			if (!text) {
				b.skip(textSize);
				return true;
			}
			b.read(text, textSize);
			text[textSize] = 0;
			consumed += textSize;
			str = text;
		}
	} else if (subSize >= consumed + 2) {
		int stringId = b.readUint16LE();
		consumed += 2;
		debugC(DEBUG_SMUSH, "SmushPlayerRebel2::handleGameTextResource: TRES string_id=%d pos=(%d,%d) flags=0x%x clip=(%d,%d,%d,%d) _strings=%p",
			stringId, posX, posY, flags, left, top, width, height, (void *)_strings);
		if (_strings)
			str = _strings->get(stringId);
	}

	if (consumed < subSize)
		b.skip(subSize - consumed);

	if (!str) {
		free(text);
		return true;
	}

	int color = 1;
	int fontId = 0;

	while (str[0] == '^') {
		if (str[1] == 'f' && Common::isDigit(str[2]) && Common::isDigit(str[3])) {
			fontId = (str[2] - '0') * 10 + str[3] - '0';
			str += 4;
		} else if (str[1] == 'c' && Common::isDigit(str[2]) &&
				Common::isDigit(str[3]) && Common::isDigit(str[4])) {
			color = (str[2] - '0') * 100 + (str[3] - '0') * 10 + str[4] - '0';
			str += 5;
		} else {
			break;
		}
	}

	TextStyleFlags flg = (TextStyleFlags)(flags & 7);
	if (ConfMan.getBool("subtitles"))
		ra2HandleTextResource(str, fontId, color, posX, posY, left, top, width, height, flg);

	free(text);
	return true;
}

bool SmushPlayerRebel2::handleGameTextRendering(const char *str, int fontId, int color,
												int pos_x, int pos_y, int left, int top,
												int width, int height, TextStyleFlags flg) {
	if (ConfMan.getBool("subtitles"))
		ra2HandleTextResource(str, fontId, color, pos_x, pos_y, left, top, width, height, flg);
	return true;
}

SmushFont *SmushPlayerRebel2::getGameFont(int font) {
	const char *ra2FontsLo[] = {
		"SYSTM/TALKFONT.NUT",
		"SYSTM/SMALFONT.NUT",
		"SYSTM/TITLFONT.NUT",
		"SYSTM/POVFONT.NUT"
	};
	const char *ra2FontsHi[] = {
		"SYSTM/TKHIFONT.NUT",
		"SYSTM/SMHIFONT.NUT",
		"SYSTM/TIHIFONT.NUT",
		"SYSTM/POHIFONT.NUT"
	};
	const bool highRes = _vm->_screenWidth >= 640 && _vm->_screenHeight >= 400;
	const char **ra2Fonts = highRes ? ra2FontsHi : ra2FontsLo;
	int numFonts = ARRAYSIZE(ra2FontsLo);
	if (font >= 0 && font < numFonts) {
		_sf[font] = new SmushFont(_vm, ra2Fonts[font], true);
	} else {
		debugC(DEBUG_SMUSH, "SmushPlayerRebel2::getGameFont: RA2 unknown font %d, using TALKFONT", font);
		_sf[font] = new SmushFont(_vm, ra2Fonts[0], true);
	}
	return _sf[font];
}

static const int RA2_MAX_STRINGS = 800;
static const int RA2_ETRS_HEADER_LENGTH = 16;

struct RA2StringEntry {
	int id;
	char *string;
};

class StringResourceRA2 : public StringResource {
	RA2StringEntry _ra2Strings[RA2_MAX_STRINGS];
	int _ra2NbStrings;
	int _ra2LastId;
	const char *_ra2LastString;

public:
	StringResourceRA2() : _ra2NbStrings(0), _ra2LastId(-1), _ra2LastString(nullptr) {
		for (int i = 0; i < RA2_MAX_STRINGS; i++) {
			_ra2Strings[i].id = 0;
			_ra2Strings[i].string = nullptr;
		}
	}

	~StringResourceRA2() override {
		for (int i = 0; i < _ra2NbStrings; i++)
			delete[] _ra2Strings[i].string;
	}

	bool init(char *buffer, int32 length) {
		char *def_start = strchr(buffer, '#');
		while (def_start != nullptr) {
			char *def_end = strchr(def_start, '\n');
			if (!def_end) break;

			char *id_end = def_end;
			while (id_end >= def_start && !Common::isDigit(*(id_end-1)))
				id_end--;
			if (id_end <= def_start) { def_start = strchr(def_end + 1, '#'); continue; }

			char *id_start = id_end;
			while (Common::isDigit(*(id_start - 1)))
				id_start--;

			char idstring[32];
			memcpy(idstring, id_start, id_end - id_start);
			idstring[id_end - id_start] = 0;
			int32 id = atoi(idstring);

			char *data_start = def_end;
			while (*data_start == '\n' || *data_start == '\r')
				data_start++;

			char *data_end = data_start;
			while (1) {
				if (data_end[-2] == '\r' && data_end[-1] == '\n' && data_end[0] == '\r' && data_end[1] == '\n') break;
				if (data_end[-2] == '\n' && data_end[-1] == '\n') break;
				if (data_end[-2] == '\r' && data_end[-1] == '\n' && data_end[0] == '#') break;
				data_end++;
				if (data_end >= buffer + length) { data_end = buffer + length; break; }
			}
			data_end -= 2;

			if (data_end <= data_start) { def_start = strchr(def_end + 1, '#'); continue; }

			if (data_start[0] == '/' && data_start[1] == '/')
				data_start += 2;
			if (data_end <= data_start) { def_start = strchr(def_end + 1, '#'); continue; }

			char *value = new char[data_end - data_start + 1];
			memcpy(value, data_start, data_end - data_start);
			value[data_end - data_start] = 0;

			char *line_start = value;
			char *line_end;
			while ((line_end = strchr(line_start, '\n'))) {
				line_start = line_end + 1;
				if (line_start[0] == '/' && line_start[1] == '/') {
					line_start += 2;
					if (line_end[-1] == '\r')
						line_end[-1] = '\n';
					memmove(line_end + 1, line_start, strlen(line_start) + 1);
				}
			}

			if (_ra2NbStrings < RA2_MAX_STRINGS) {
				_ra2Strings[_ra2NbStrings].id = id;
				_ra2Strings[_ra2NbStrings].string = value;
				_ra2NbStrings++;
			} else {
				delete[] value;
			}
			def_start = strchr(data_end + 2, '#');
		}
		return true;
	}

	const char *get(int id) override {
		if (id == _ra2LastId)
			return _ra2LastString;
		for (int i = 0; i < _ra2NbStrings; i++) {
			if (_ra2Strings[i].id == id) {
				_ra2LastId = id;
				_ra2LastString = _ra2Strings[i].string;
				return _ra2LastString;
			}
		}
		warning("StringResourceRA2: invalid string id : %d", id);
		_ra2LastId = -1;
		_ra2LastString = "unknown string";
		return _ra2LastString;
	}
};

bool SmushPlayerRebel2::handleGameSetupStrings() {
	ScummFile *theFile = _vm->instantiateScummFile();
	_vm->openFile(*theFile, "SYSTM/GAME.TRS");
	if (!theFile->isOpen()) {
		delete theFile;
		return true; // handled (no strings available)
	}
	int32 length = theFile->size();
	char *filebuffer = new char[length + 1];
	theFile->read(filebuffer, length);
	filebuffer[length] = 0;
	theFile->close();
	delete theFile;

	if (READ_BE_UINT32(filebuffer) == MKTAG('E','T','R','S')) {
		assert(length > RA2_ETRS_HEADER_LENGTH);
		length -= RA2_ETRS_HEADER_LENGTH;
		for (int i = 0; i < length; ++i)
			filebuffer[i] = filebuffer[i + RA2_ETRS_HEADER_LENGTH] ^ 0xCC;
		filebuffer[length] = '\0';
	}

	StringResourceRA2 *sr = new StringResourceRA2;
	sr->init(filebuffer, length);
	delete[] filebuffer;
	_strings = sr;
	return true;
}

void SmushPlayerRebel2::adjustGamePalette() {
	for (int j = 0; j < 768; ++j) {
		_shiftedDeltaPal[j] = _pal[j] << 7;
	}
	memset(_deltaPal, 0, sizeof(_deltaPal));
}

bool SmushPlayerRebel2::shouldLoadAnimHeaderPalette() const {
	return false;
}

void SmushPlayerRebel2::ra2HandleDeltaPalette(int32 subSize, Common::SeekableReadStream &b) {
	if (subSize < 4) {
		b.skip(subSize);
		return;
	}

	b.readUint16LE();
	const uint16 xpalCommand = b.readUint16LE();

	if (xpalCommand != 0 && xpalCommand != 512) {
		if (subSize > 4)
			b.skip(subSize - 4);

		for (int i = 0; i < 768; ++i) {
			_shiftedDeltaPal[i] += _deltaPal[i];
			_pal[i] = CLIP<int32>(_shiftedDeltaPal[i] >> 7, 0, 255);
		}
		setDirtyColors(0, 255);
		return;
	}

	const int32 deltaBytes = 0x300 * 2;
	if (subSize < 4 + deltaBytes) {
		b.skip(subSize - 4);
		return;
	}

	int16 deltaPal[0x300];
	for (int i = 0; i < 768; ++i)
		deltaPal[i] = b.readSint16LE();

	if (xpalCommand == 512 && subSize >= 4 + deltaBytes + 0x300) {
		b.read(_pal, 0x300);
	} else if (subSize > 4 + deltaBytes) {
		b.skip(subSize - 4 - deltaBytes);
	}

	for (int i = 0; i < 768; ++i) {
		_shiftedDeltaPal[i] = _pal[i] << 7;
		_deltaPal[i] = deltaPal[i];
	}

	setDirtyColors(0, 255);
}

bool SmushPlayerRebel2::handleGameAnimHeader(byte *headerContent) {
	if (!_skipPalette && (_curVideoFlags & 0x400) == 0) {
		memcpy(_pal, headerContent + 6, sizeof(_pal));
		adjustGamePalette();

		// AHDR replaces _pal before the first FRME, so delay the dirty range
		// until frame start.
		_palDirtyMin = 256;
		_palDirtyMax = -1;
		_ra2PendingAnimHeaderPalette = true;
	}

	int width = READ_LE_UINT16(&headerContent[4]);
	int height = READ_LE_UINT16(&headerContent[6]);

	if (width == 0 && height == 0) {
		_width = _vm->_screenWidth;
		_height = _vm->_screenHeight;
		debugC(DEBUG_SMUSH, "SmushPlayerRebel2::handleGameAnimHeader: RA2 AHDR has 0x0 dims - using screen size %dx%d", _width, _height);
	} else if (width != _vm->_screenWidth || height != _vm->_screenHeight) {
		// Do not let AHDR alone change pitch while _dst still points at the
		// virtual screen; FOBJ selection allocates larger surfaces when needed.
		_width = _vm->_screenWidth;
		_height = _vm->_screenHeight;
		debugC(DEBUG_SMUSH, "SmushPlayerRebel2::handleGameAnimHeader: RA2 AHDR %dx%d - using screen size until FOBJ selects a surface",
			width, height);
	} else {
		_width = width;
		_height = height;
	}

	return true;
}

void SmushPlayerRebel2::handleGameLoad(int32 subSize, Common::SeekableReadStream &b) {
	handleLoad(subSize, b);
}

void SmushPlayerRebel2::handleLoad(int32 subSize, Common::SeekableReadStream &b) {
	debugC(DEBUG_SMUSH, "SmushPlayerRebel2::handleLoad()");

	if (subSize < 10) {
		warning("SmushPlayerRebel2::handleLoad: chunk too small (%d bytes)", subSize);
		return;
	}

	int16 streamId = b.readUint16LE();
	int16 chunkIndex = b.readUint16LE();
	b.skip(6);  // Unknown/padding

	int32 dataSize = subSize - 10;

	debugC(DEBUG_SMUSH, "SmushPlayerRebel2::handleLoad: stream=%d chunk=%d dataSize=%d bufferOffset=%d",
		streamId, chunkIndex, dataSize, _loadBufferOffset);

	if (chunkIndex == 0) {
		_loadBufferOffset = 0;
		_loadReadOffset = 8;
		_lastLoadChunkIdx = -1;
		_loadStreamId = streamId;

		if (_loadBuffer == nullptr || _loadBufferSize < kRebel2LoadBufferSize) {
			free(_loadBuffer);
			_loadBufferSize = kRebel2LoadBufferSize;
			_loadBuffer = (byte *)malloc(_loadBufferSize);
			if (_loadBuffer == nullptr) {
				warning("SmushPlayerRebel2::handleLoad: Failed to allocate %d bytes for LOAD buffer",
					_loadBufferSize);
				_loadBufferSize = 0;
				return;
			}
			debugC(DEBUG_SMUSH, "SmushPlayerRebel2::handleLoad: Allocated %d bytes for LOAD buffer",
				_loadBufferSize);
		}
	}

	if (_lastLoadChunkIdx + 1 != chunkIndex) {
		debugC(DEBUG_SMUSH, "SmushPlayerRebel2::handleLoad: Non-sequential chunk %d (expected %d), skipping",
			chunkIndex, _lastLoadChunkIdx + 1);
		return;
	}

	if (_loadBuffer == nullptr || _loadBufferOffset + subSize >= _loadBufferSize) {
		warning("SmushPlayerRebel2::handleLoad: Buffer overflow - offset=%d size=%d limit=%d",
			_loadBufferOffset, subSize, _loadBufferSize);
		return;
	}

	b.read(_loadBuffer + _loadBufferOffset, dataSize);
	_loadBufferOffset += dataSize;
	_lastLoadChunkIdx = chunkIndex;

	debugC(DEBUG_SMUSH, "SmushPlayerRebel2::handleLoad: Accumulated %d bytes total", _loadBufferOffset);
}

void SmushPlayerRebel2::ra2HandleTextResource(const char *str, int fontId, int color,
										int pos_x, int pos_y, int left, int top,
										int width, int height, TextStyleFlags flg) {
	// Promote native frames before drawing 2x-scaled high-res text.
	const bool hiRes = ra2IsHighResMode() && !isRebel2GameplayActive(_insane);
	if (hiRes)
		ra2PromoteCurrentFrameToHiRes(0, 0);

	ensureMultiFont();
	_multiFont->setDefaultFont(fontId);
	const int scale = hiRes ? 2 : 1;
	pos_x *= scale;
	pos_y *= scale;
	left *= scale;
	top *= scale;
	width *= scale;
	height *= scale;

	debugC(DEBUG_SMUSH, "SmushPlayerRebel2::ra2HandleTextResource: RA2 TRES frame=%d fontId=%d color=%d flags=0x%x pos=(%d,%d) clip=(%d,%d,%d,%d) str=\"%.40s\"",
		  _frame, fontId, color, (int)flg, pos_x, pos_y, left, top, width, height, str);

	if (flg & kStyleWordWrap) {
		Common::Rect clipRect(MAX<int>(0, left), MAX<int>(0, top), MIN<int>(left + width, _width), MIN<int>(top + height, _height));
		_multiFont->drawStringWrap(str, _dst, clipRect, pos_x, pos_y, _width, color, flg);
	} else {
		Common::Rect clipRect(0, 0, _width, _height);
		_multiFont->drawString(str, _dst, clipRect, pos_x, pos_y, _width, color, flg);
	}
}

void SmushPlayerRebel2::ra2PrepareFrameObjectSurface(int left, int top, int width, int height) {
	_ra2FrameObjectOriginalWidth = width;
	_ra2FrameObjectOriginalHeight = height;
	_ra2FrameObjectSurfaceWidth = width;
	_ra2FrameObjectSurfaceHeight = height;

	const int64 right = (int64)left + width;
	const int64 bottom = (int64)top + height;
	if (right > _ra2FrameObjectSurfaceWidth && right <= INT_MAX)
		_ra2FrameObjectSurfaceWidth = (int)right;
	if (bottom > _ra2FrameObjectSurfaceHeight && bottom <= INT_MAX)
		_ra2FrameObjectSurfaceHeight = (int)bottom;
}

bool SmushPlayerRebel2::ra2SelectFrameBuffer(int codec, int width, int height) {
	if (codec == SMUSH_CODEC_RA2_BOMP) {
		const bool highRes = ra2IsHighResMode();
		const bool gameplayActive = isRebel2GameplayActive(_insane);
		const int64 currentSurfaceSize64 = (int64)_width * _height;
		if (highRes && gameplayActive &&
				_dst == _specialBuffer && _specialBuffer != nullptr &&
				_width > 320 && _height > 200 &&
				currentSurfaceSize64 > 0 && currentSurfaceSize64 <= _specialBufferSize) {
			if (_ra2NativeFrameNeedsClear) {
				ra2ClearCurrentTarget();
				_ra2NativeFrameNeedsClear = false;
			}
			debugC(DEBUG_SMUSH, "SmushPlayerRebel2::ra2SelectFrameBuffer: Using current _specialBuffer %dx%d for high-res codec 45 mask",
				_width, _height);
		} else if (highRes && (!gameplayActive || !_ra2UsingGameplaySurface || _specialBuffer == nullptr)) {
			if (!ra2EnsureLowResVideoBuffer())
				return false;
			_dst = _ra2LowResVideoBuffer;
			_width = 320;
			_height = 200;
			if (_ra2NativeFrameNeedsClear) {
				ra2ClearCurrentTarget();
				_ra2NativeFrameNeedsClear = false;
			}
			debugC(DEBUG_SMUSH, "SmushPlayerRebel2::ra2SelectFrameBuffer: Using low-res decode buffer for high-res codec 45 mask");
		} else if (_specialBuffer != nullptr) {
			_dst = _specialBuffer;
			if (_ra2NativeFrameNeedsClear) {
				ra2ClearCurrentTarget();
				_ra2NativeFrameNeedsClear = false;
			}
			debugC(DEBUG_SMUSH, "SmushPlayerRebel2::ra2SelectFrameBuffer: Using _specialBuffer for codec 45 mask");
		} else {
			VirtScreen *vs = &_vm->_virtscr[kMainVirtScreen];
			_dst = vs->getPixels(0, 0);
			debugC(DEBUG_SMUSH, "SmushPlayerRebel2::ra2SelectFrameBuffer: Using virtual screen for codec 45 mask");
		}
		return true;
	}

	// Low-res gameplay uses a 424x260 target, then promotes the selected
	// viewport to the 640x400 screen in high-res mode.
	const int screenSize = _vm->_screenWidth * _vm->_screenHeight;
	const int nativeScreenSize = 320 * 200;
	const int64 fobjSize64 = (int64)width * height;
	int surfaceWidth = width;
	int surfaceHeight = height;

	const bool highRes = ra2IsHighResMode();
	const bool gameplayActive = isRebel2GameplayActive(_insane);
	const bool fullFrameDelta = isRebel2FullFrameDeltaCodec(codec);
	const int64 currentSurfaceSize64 = (int64)_width * _height;

	if (!gameplayActive && fobjSize64 <= nativeScreenSize) {
		if (highRes) {
			if (!ra2EnsureLowResVideoBuffer())
				return false;
			_dst = _ra2LowResVideoBuffer;
			_width = 320;
			_height = 200;
		} else {
			VirtScreen *vs = &_vm->_virtscr[kMainVirtScreen];
			_dst = vs->getPixels(0, 0);
			_width = _vm->_screenWidth;
			_height = _vm->_screenHeight;
		}

		if (_ra2NativeFrameNeedsClear) {
			ra2ClearCurrentTarget();
			_ra2NativeFrameNeedsClear = false;
		}
		debugC(DEBUG_SMUSH, "SmushPlayerRebel2::ra2SelectFrameBuffer: Using native screen target for menu/cinematic FOBJ %dx%d",
			width, height);
		return true;
	}

	if (gameplayActive && !fullFrameDelta &&
			isRebel2Handler25CorridorMode(_insane) &&
			_dst == _specialBuffer && _specialBuffer != nullptr &&
			_width == 350 && _height == 200 &&
			_ra2FrameObjectSurfaceWidth <= _width &&
			_ra2FrameObjectSurfaceHeight <= _height &&
			currentSurfaceSize64 > 0 && currentSurfaceSize64 <= _specialBufferSize) {
		if (_ra2NativeFrameNeedsClear) {
			ra2ClearCurrentTarget();
			_ra2NativeFrameNeedsClear = false;
		}
		debugC(DEBUG_SMUSH, "SmushPlayerRebel2::ra2SelectFrameBuffer: Using current Handler25 corridor _specialBuffer %dx%d for FOBJ %dx%d",
			_width, _height, width, height);
		return true;
	}

	const bool oversizedNative = fobjSize64 > nativeScreenSize ||
		width > 320 || height > 200 ||
		_ra2FrameObjectSurfaceWidth > 320 || _ra2FrameObjectSurfaceHeight > 200;
	const bool useGameplaySurface = gameplayActive && !fullFrameDelta &&
		(oversizedNative || _ra2UsingGameplaySurface);

	if (useGameplaySurface) {
		surfaceWidth = kRebel2GameplaySurfaceWidth;
		surfaceHeight = kRebel2GameplaySurfaceHeight;
		_ra2UsingGameplaySurface = true;
	} else if (!highRes && fobjSize64 > screenSize && !fullFrameDelta) {
		surfaceWidth = MAX(surfaceWidth, _ra2FrameObjectSurfaceWidth);
		surfaceHeight = MAX(surfaceHeight, _ra2FrameObjectSurfaceHeight);
	}

	if (highRes && gameplayActive && !useGameplaySurface && !oversizedNative &&
			width > 0 && height > 0 &&
			_dst == _specialBuffer && _specialBuffer != nullptr &&
			_width > 320 && _height > 200 &&
			currentSurfaceSize64 > 0 && currentSurfaceSize64 <= _specialBufferSize) {
		if (_ra2NativeFrameNeedsClear) {
			ra2ClearCurrentTarget();
			_ra2NativeFrameNeedsClear = false;
		}
		debugC(DEBUG_SMUSH, "SmushPlayerRebel2::ra2SelectFrameBuffer: Using current _specialBuffer %dx%d for high-res gameplay FOBJ %dx%d",
			_width, _height, width, height);
		return true;
	}

	if (highRes && !useGameplaySurface && !oversizedNative) {
		if (width <= 0 || height <= 0) {
			debugC(DEBUG_SMUSH, "SmushPlayerRebel2::ra2SelectFrameBuffer: Skipping invalid FOBJ dimensions %dx%d", width, height);
			return false;
		}
		if (!ra2EnsureLowResVideoBuffer())
			return false;
		_dst = _ra2LowResVideoBuffer;
		_width = 320;
		_height = 200;
		if (_ra2NativeFrameNeedsClear) {
			ra2ClearCurrentTarget();
			_ra2NativeFrameNeedsClear = false;
		}
		debugC(DEBUG_SMUSH, "SmushPlayerRebel2::ra2SelectFrameBuffer: Using low-res decode buffer for high-res FOBJ %dx%d",
			width, height);
		return true;
	}

	const int64 bufSize64 = (int64)surfaceWidth * surfaceHeight;
	if (width <= 0 || height <= 0 || surfaceWidth <= 0 || surfaceHeight <= 0 || bufSize64 > INT_MAX) {
		debugC(DEBUG_SMUSH, "SmushPlayerRebel2::ra2SelectFrameBuffer: Skipping invalid FOBJ dimensions %dx%d", width, height);
		return false;
	}

	const int bufSize = (int)bufSize64;
	const bool needsSpecialBuffer = useGameplaySurface || oversizedNative || bufSize > screenSize;
	if (needsSpecialBuffer) {
		if (_specialBuffer == nullptr || bufSize > _specialBufferSize) {
			byte *oldDst = _dst;
			const int oldWidth = _width;
			const int oldHeight = _height;
			int oldPitch = oldWidth;
			if (oldDst != _specialBuffer && oldDst != _ra2LowResVideoBuffer)
				oldPitch = _vm->_screenWidth;
			const bool oldTargetIsNative = oldDst == _specialBuffer || oldDst == _ra2LowResVideoBuffer ||
				(oldWidth <= 320 && oldHeight <= 200);

			byte *newSpecialBuffer = (byte *)malloc(bufSize);
			if (newSpecialBuffer == nullptr) {
				warning("SmushPlayerRebel2::ra2SelectFrameBuffer: Failed to allocate %d bytes for FOBJ %dx%d",
					bufSize, width, height);
				return false;
			}
			memset(newSpecialBuffer, 0, bufSize);

			if (oldTargetIsNative && oldDst != nullptr && oldWidth > 0 && oldHeight > 0 && oldPitch > 0) {
				const int copyWidth = MIN<int>(surfaceWidth, MIN<int>(oldWidth, oldPitch));
				const int copyHeight = MIN<int>(surfaceHeight, oldHeight);
				for (int y = 0; y < copyHeight; y++) {
					memcpy(newSpecialBuffer + y * surfaceWidth,
						   oldDst + y * oldPitch,
						   copyWidth);
				}
			}

			free(_specialBuffer);
			_specialBuffer = newSpecialBuffer;
			_specialBufferSize = bufSize;
			debugC(DEBUG_SMUSH, "SmushPlayerRebel2::ra2SelectFrameBuffer: Allocated new _specialBuffer %dx%d for FOBJ %dx%d (%d bytes)",
				surfaceWidth, surfaceHeight, width, height, bufSize);
		}
		_width = surfaceWidth;
		_height = surfaceHeight;
	}

	if (needsSpecialBuffer &&
			_specialBuffer != nullptr && _specialBufferSize >= bufSize) {
		_dst = _specialBuffer;
		if (_ra2NativeFrameNeedsClear) {
			ra2ClearCurrentTarget();
			_ra2NativeFrameNeedsClear = false;
		}
		debugC(DEBUG_SMUSH, "SmushPlayerRebel2::ra2SelectFrameBuffer: Using _specialBuffer %dx%d for oversized FOBJ %dx%d",
			_width, _height, width, height);
	} else {
		if (_specialBuffer == nullptr) {
			VirtScreen *vs = &_vm->_virtscr[kMainVirtScreen];
			_dst = vs->getPixels(0, 0);
			debugC(DEBUG_SMUSH, "SmushPlayerRebel2::ra2SelectFrameBuffer: Reset _dst to virtual screen for FOBJ %dx%d at (%d,%d)",
				width, height, 0, 0);
		} else {
			_dst = _specialBuffer;
			debugC(DEBUG_SMUSH, "SmushPlayerRebel2::ra2SelectFrameBuffer: Using _specialBuffer for small FOBJ %dx%d (compositing with large frame)",
				width, height);
		}
	}

	return true;
}

bool SmushPlayerRebel2::ra2DecodePlacedDeltaCodec(int codec, const uint8 *src, int left, int top,
									 int width, int height, int pitch, int dataSize) {
	if (!src || !_dst || width <= 0 || height <= 0 || pitch <= 0 || dataSize <= 0)
		return true;

	const int visibleWidth = (_ra2FrameObjectOriginalWidth > 0) ? _ra2FrameObjectOriginalWidth : width;
	const int visibleHeight = (_ra2FrameObjectOriginalHeight > 0) ? _ra2FrameObjectOriginalHeight : height;
	if (visibleWidth <= 0 || visibleHeight <= 0)
		return true;

	const int blockSize = (codec == SMUSH_CODEC_DELTA_BLOCKS) ? 4 : 8;
	const int decodeWidth = (visibleWidth + blockSize - 1) & ~(blockSize - 1);
	const int decodeHeight = (visibleHeight + blockSize - 1) & ~(blockSize - 1);
	const int64 decodeSize64 = (int64)decodeWidth * decodeHeight;
	if (decodeSize64 <= 0 || decodeSize64 > INT_MAX) {
		warning("SmushPlayerRebel2::ra2DecodePlacedDeltaCodec: invalid codec %d frame size %dx%d",
			codec, decodeWidth, decodeHeight);
		return true;
	}

	byte *deltaFrame = (byte *)malloc((size_t)decodeSize64);
	if (deltaFrame == nullptr) {
		warning("SmushPlayerRebel2::ra2DecodePlacedDeltaCodec: failed to allocate %d-byte scratch frame",
			(int)decodeSize64);
		return true;
	}

	if (codec == SMUSH_CODEC_DELTA_BLOCKS) {
		if (_deltaBlocksCodec != nullptr &&
				(_ra2DeltaBlocksWidth != decodeWidth || _ra2DeltaBlocksHeight != decodeHeight)) {
			delete _deltaBlocksCodec;
			_deltaBlocksCodec = nullptr;
			_ra2DeltaBlocksWidth = 0;
			_ra2DeltaBlocksHeight = 0;
		}
		if (_deltaBlocksCodec == nullptr) {
			_deltaBlocksCodec = new SmushDeltaBlocksDecoder(decodeWidth, decodeHeight, true);
			_ra2DeltaBlocksWidth = decodeWidth;
			_ra2DeltaBlocksHeight = decodeHeight;
		}
		_deltaBlocksCodec->decode(deltaFrame, src);
	} else {
		if (_deltaGlyphsCodec != nullptr &&
				(_ra2DeltaGlyphsWidth != decodeWidth || _ra2DeltaGlyphsHeight != decodeHeight)) {
			delete _deltaGlyphsCodec;
			_deltaGlyphsCodec = nullptr;
			_ra2DeltaGlyphsWidth = 0;
			_ra2DeltaGlyphsHeight = 0;
		}
		if (_deltaGlyphsCodec == nullptr) {
			_deltaGlyphsCodec = new SmushDeltaGlyphsDecoder(decodeWidth, decodeHeight);
			_ra2DeltaGlyphsWidth = decodeWidth;
			_ra2DeltaGlyphsHeight = decodeHeight;
		}
		_deltaGlyphsCodec->decode(deltaFrame, src);
	}

	int srcX = CLIP<int>(_ra2FrameSourceSkipX, 0, visibleWidth);
	int srcY = CLIP<int>(_ra2FrameSourceSkipY, 0, visibleHeight);
	int copyWidth = MIN<int>(width, visibleWidth - srcX);
	int copyHeight = MIN<int>(height, visibleHeight - srcY);
	int dstX = left;
	int dstY = top;

	if (dstX < 0) {
		const int skip = -dstX;
		srcX += skip;
		copyWidth -= skip;
		dstX = 0;
	}
	if (dstY < 0) {
		const int skip = -dstY;
		srcY += skip;
		copyHeight -= skip;
		dstY = 0;
	}

	const int dstHeight = (_height > 0) ? _height : _vm->_screenHeight;
	if (dstX + copyWidth > pitch)
		copyWidth = pitch - dstX;
	if (dstY + copyHeight > dstHeight)
		copyHeight = dstHeight - dstY;

	const bool transparentZero = codec == SMUSH_CODEC_DELTA_BLOCKS &&
		visibleWidth == 350 && visibleHeight == 200 &&
		isRebel2Handler25CorridorMode(_insane);

	if (copyWidth > 0 && copyHeight > 0) {
		for (int y = 0; y < copyHeight; y++) {
			byte *dstLine = _dst + (dstY + y) * pitch + dstX;
			const byte *srcLine = deltaFrame + (srcY + y) * decodeWidth + srcX;

			if (transparentZero) {
				for (int x = 0; x < copyWidth; x++) {
					if (srcLine[x] != 0)
						dstLine[x] = srcLine[x];
				}
			} else {
				memcpy(dstLine, srcLine, copyWidth);
			}
		}
	}

	debugC(DEBUG_SMUSH, "SmushPlayerRebel2::ra2DecodePlacedDeltaCodec: codec=%d pos=(%d,%d) visible=%dx%d decode=%dx%d copy=%dx%d skip=(%d,%d) transparent=%d",
		codec, left, top, visibleWidth, visibleHeight, decodeWidth, decodeHeight,
		MAX<int>(0, copyWidth), MAX<int>(0, copyHeight), _ra2FrameSourceSkipX, _ra2FrameSourceSkipY,
		transparentZero ? 1 : 0);

	free(deltaFrame);
	return true;
}

bool SmushPlayerRebel2::ra2DecodeCodec(int codec, const uint8 *src, int left, int top,
									 int width, int height, int pitch, int dataSize) {
	switch (codec) {
	case SMUSH_CODEC_RLE_ALT:
		smushDecodeRLEOpaque(_dst, src, left, top, width, height, pitch, dataSize);
		return true;
	case SMUSH_CODEC_RLE:
		if ((_curVideoFlags & 0x100) != 0) {
			smushDecodeRLEOpaque(_dst, src, left, top, width, height, pitch, dataSize);
			return true;
		}
		return false;
	case SMUSH_CODEC_UNCOMPRESSED: {
		const int sourcePitch = (_ra2FrameObjectOriginalWidth > 0) ? _ra2FrameObjectOriginalWidth : width;
		smushDecodeRA2Uncompressed(_dst, src, left, top, width, height, pitch, sourcePitch,
			dataSize, _ra2FrameSourceSkipX, _ra2FrameSourceSkipY);
		return true;
	}
	case SMUSH_CODEC_LINE_UPDATE:
	case SMUSH_CODEC_LINE_UPDATE2: {
		const uint8 *adjustedSrc = smushSkipRLELines(src, dataSize, _ra2FrameSourceSkipY);
		smushDecodeLineUpdate(_dst, adjustedSrc, left, top, width, height, pitch, dataSize);
		return true;
	}
	case SMUSH_CODEC_SKIP_RLE: {
		const uint8 *adjustedSrc = smushSkipRLELines(src, dataSize, _ra2FrameSourceSkipY);
		smushDecodeSkipRLE(_dst, adjustedSrc, left, top, width, height, pitch, dataSize);
		return true;
	}
	case SMUSH_CODEC_RA2_BOMP: {
		smushDecodeRA2Blur(_dst, src, left, top, pitch, _height, pitch, dataSize,
			_ra2Codec45Palette, _ra2Codec45Lookup);
		return true;
	}
	default:
		return false;
	}
}

void SmushPlayerRebel2::ra2StoreFobjData(int codec, const byte *data, int32 dataSize,
								   int left, int top, int width, int height) {
	free(_storedFobjData);
	_storedFobjData = (byte *)malloc(dataSize);
	memcpy(_storedFobjData, data, dataSize);
	_storedFobjDataSize = dataSize;
	_storedFobjCodec = codec;
	_storedFobjParm2 = 0;
	_storedFobjLeft = left;
	_storedFobjTop = top;
	_storedFobjWidth = width;
	_storedFobjHeight = height;
	_storeFrame = false;
}

void SmushPlayerRebel2::ra2HandleGost(int32 subSize, Common::SeekableReadStream &b) {
	if (subSize < 6) {
		warning("SmushPlayerRebel2::ra2HandleGost: chunk too small (%d bytes)", subSize);
		return;
	}

	int16 ghostType = b.readSint16LE();
	int16 ghostX = b.readSint16LE();
	int16 ghostY = b.readSint16LE();

	if (!_hasFrameFobjForGost || _lastFobjData == nullptr || _lastFobjDataSize <= 0) {
		debugC(DEBUG_SMUSH, "SmushPlayerRebel2::ra2HandleGost GOST: frame=%d ignored (no current-frame FOBJ cached)", _frame);
		return;
	}

	uint16 priorityFlags = 0;
	if (ghostType == 0) {
		priorityFlags = 0x2000;
	} else if (ghostType == 1) {
		priorityFlags = 0x4000;
	} else if (ghostType == 2) {
		priorityFlags = 0x6000;
	}

	int left = _lastFobjLeft + ghostX;
	int top = _lastFobjTop + ghostY;

	debugC(DEBUG_SMUSH, "SmushPlayerRebel2::ra2HandleGost GOST: frame=%d type=%d flags=0x%04x gostPos=(%d,%d) basePos=(%d,%d) finalPos=(%d,%d) size=%dx%d codec=%d",
		_frame, ghostType, priorityFlags, ghostX, ghostY,
		_lastFobjLeft, _lastFobjTop, left, top,
		_lastFobjWidth, _lastFobjHeight, _lastFobjCodec);

	ra2PrepareFrameObjectSurface(left, top, _lastFobjWidth, _lastFobjHeight);
	decodeFrameObject(_lastFobjCodec, _lastFobjData, left, top,
		_lastFobjWidth, _lastFobjHeight, _lastFobjDataSize);
}

void SmushPlayerRebel2::handleGameParseNextFrame() {
	processDispatches(_smushAudioSampleRate / 12);
}

bool SmushPlayerRebel2::handleGameFrameBufferSelect(int codec, int width, int height) {
	if ((height != _vm->_screenHeight) || (width != _vm->_screenWidth)) {
		return ra2SelectFrameBuffer(codec, width, height);
	}
	return false;
}

bool SmushPlayerRebel2::handleGameDimensionOverride(int codec, int width, int height) {
	if ((height != _vm->_screenHeight) || (width != _vm->_screenWidth)) {
		if (_dst == _ra2LowResVideoBuffer && _ra2LowResVideoBuffer != nullptr) {
			return true;
		}

		if (_insane != nullptr) {
			InsaneRebel2 *rebel2 = static_cast<InsaneRebel2 *>(_insane);
			if (rebel2->getHandler() != 0) {
				return true;
			}
		}

		if (_dst == _specialBuffer && _specialBuffer != nullptr) {
			return true;
		}

		return false;
	}

	return false;
}

int SmushPlayerRebel2::handleGameFrameObjectPitch(int pitch) {
	if (_dst == _ra2LowResVideoBuffer && _ra2LowResVideoBuffer != nullptr && _width > 0)
		return _width;

	return pitch;
}

bool SmushPlayerRebel2::handleGameAdjustCoords(int codec, int &left, int &top, int &width, int &height, int pitch, int *srcSkipY) {
	int sourceSkipY = 0;
	const int adjustedLeft = left + _fobjOffsetX;
	_ra2FrameSourceSkipX = (adjustedLeft < 0) ? -adjustedLeft : 0;
	_ra2FrameSourceSkipY = 0;

	if (codec == SMUSH_CODEC_RA2_BOMP) {
		left = adjustedLeft;
		top += _fobjOffsetY;
		if (srcSkipY)
			*srcSkipY = 0;
		return true;
	}

	if (_dst == _ra2LowResVideoBuffer && _ra2LowResVideoBuffer != nullptr) {
		left += _fobjOffsetX;
		top += _fobjOffsetY;

		if (top < 0) {
			sourceSkipY = -top;
			height += top;
			top = 0;
		}
		if (left < 0) {
			width += left;
			left = 0;
		}
		if (top + height > _height)
			height = _height - top;
		if (left + width > pitch)
			width = pitch - left;
	} else {
		adjustFrameCoords(left, top, width, height, pitch, &sourceSkipY);
	}

	if (codec == SMUSH_CODEC_LINE_UPDATE || codec == SMUSH_CODEC_LINE_UPDATE2 ||
			codec == SMUSH_CODEC_SKIP_RLE || codec == SMUSH_CODEC_UNCOMPRESSED) {
		_ra2FrameSourceSkipY = sourceSkipY;
		if (srcSkipY)
			*srcSkipY = 0;
	} else if (isRebel2FullFrameDeltaCodec(codec)) {
		_ra2FrameSourceSkipY = sourceSkipY;
		if (srcSkipY)
			*srcSkipY = 0;
	} else if (srcSkipY) {
		*srcSkipY = sourceSkipY;
	}
	return true;
}

bool SmushPlayerRebel2::handleGameCodecDecode(int codec, const uint8 *src, int left, int top, int width, int height, int pitch, int dataSize, uint8 param, uint16 parm2) {
	if (isRebel2FullFrameDeltaCodec(codec))
		return ra2DecodePlacedDeltaCodec(codec, src, left, top, width, height, pitch, dataSize);

	if (codec == SMUSH_CODEC_SKIP_RLE && parm2 >= 0x100) {
		if (parm2 == 0x100 && dataSize >= 256) {
			memcpy(_ra2SkipRemapTable, src, 256);
			_ra2SkipRemapValid = true;
			src += 256;
			dataSize -= 256;
		}
		const byte *remap = _ra2SkipRemapValid ? _ra2SkipRemapTable : nullptr;
		smushDecodeRA2SkipRemap(_dst, src, left, top, width, height, pitch, dataSize, remap, (byte)parm2);
		return true;
	}

	return ra2DecodeCodec(codec, src, left, top, width, height, pitch, dataSize);
}

void SmushPlayerRebel2::handleFrameObject(int32 subSize, Common::SeekableReadStream &b) {
	assert(subSize >= 14);
	if (_skipNext) {
		_skipNext = false;
		return;
	}

	int codec = b.readUint16LE();
	int left = (int)b.readSint16LE();
	int top = (int)b.readSint16LE();
	int width = b.readUint16LE();
	int height = b.readUint16LE();
	b.readUint16LE();                     // objectId
	uint16 parm2 = b.readUint16LE();

	const int32 chunkSize = subSize - 14;
	handleGameFrameObjectPre(codec, left, top, width, height, chunkSize);

	byte *chunkBuffer = (byte *)malloc(chunkSize);
	assert(chunkBuffer);
	b.read(chunkBuffer, chunkSize);

	handleGameFrameObjectPost(codec, chunkBuffer, chunkSize, left, top, width, height);
	decodeFrameObject(codec, chunkBuffer, left, top, width, height, chunkSize, 0, parm2);
	free(chunkBuffer);
}

bool SmushPlayerRebel2::handleGameStoreFrame() {
	return true;
}

void SmushPlayerRebel2::handleGameFrameObjectPre(int codec, int left, int top, int width, int height, int dataSize) {
	ra2PrepareFrameObjectSurface(left, top, width, height);

	debugC(DEBUG_SMUSH, "SmushPlayerRebel2::handleGameFrameObjectPre FOBJ: frame=%d codec=%d pos=(%d,%d) size=%dx%d dataSize=%d storeFrame=%d _width=%d _height=%d",
		_frame, codec, left, top, width, height, dataSize, _storeFrame, _width, _height);
}

void SmushPlayerRebel2::handleGameFrameObjectPost(int codec, const byte *data, int32 dataSize, int left, int top, int width, int height) {
	rememberLastFobj(codec, data, dataSize, left, top, width, height);

	if (_storeFrame) {
		ra2StoreFobjData(codec, data, dataSize, left, top, width, height);
	}
}

void SmushPlayerRebel2::handleGameFrameStart() {
	_hasFrameFobjForGost = false;
	_ra2NativeFrameNeedsClear = ((_curVideoFlags & 0x20) == 0);

	if (_ra2PendingAnimHeaderPalette) {
		setDirtyColors(0, 255);
		_ra2PendingAnimHeaderPalette = false;
	}

	if (ra2IsHighResMode()) {
		if (isRebel2GameplayActive(_insane)) {
			if (_ra2UsingGameplaySurface && _specialBuffer != nullptr &&
					_specialBufferSize >= kRebel2GameplaySurfaceWidth * kRebel2GameplaySurfaceHeight) {
				_dst = _specialBuffer;
				_width = kRebel2GameplaySurfaceWidth;
				_height = kRebel2GameplaySurfaceHeight;
			} else if (ra2EnsureLowResVideoBuffer()) {
				_dst = _ra2LowResVideoBuffer;
				_width = 320;
				_height = 200;
			}
		} else if (ra2EnsureLowResVideoBuffer()) {
			_dst = _ra2LowResVideoBuffer;
			_width = 320;
			_height = 200;
		}
	}

	if ((_curVideoFlags & 0x20) == 0 && _dst != nullptr) {
		ra2ClearCurrentTarget();
		if (!ra2IsHighResMode() || isRebel2GameplayActive(_insane) || _dst != _vm->_virtscr[kMainVirtScreen].getPixels(0, 0))
			_ra2NativeFrameNeedsClear = false;
	}
}

bool SmushPlayerRebel2::handleGameSkipChunk(uint32 subType, int32 subSize, Common::SeekableReadStream &b) {
	if (_skipNext) {
		_skipNext = false;
		debugC(DEBUG_SMUSH, "SmushPlayerRebel2::handleGameSkipChunk: SKIP consumed chunk %s frame=%d", tag2str(subType), _frame);
		return true;
	}

	if (subType == MKTAG('P','S','A','D')) {
		if (!_compressedFileMode && !isFastForwardingCurrentFrame())
			ra2HandleFrameAudioChunk(subType, subSize, b);
		return true;
	}

	if (subType == MKTAG('X','P','A','L')) {
		ra2HandleDeltaPalette(subSize, b);
		return true;
	}

	return false;
}

void SmushPlayerRebel2::ra2HandleFrameAudioChunk(uint32 subType, int32 subSize, Common::SeekableReadStream &b) {
	if (subSize <= 0)
		return;

	uint8 *audioChunk = (uint8 *)malloc(subSize + 8);
	if (!audioChunk) {
		b.skip(subSize);
		return;
	}

	WRITE_BE_UINT32(audioChunk, subType);
	WRITE_BE_UINT32(audioChunk + 4, subSize);
	b.read(audioChunk + 8, subSize);
	ra2FeedAudio(audioChunk, 0, 127, 0, 0);
	free(audioChunk);
}

void SmushPlayerRebel2::ra2FeedAudio(uint8 *srcBuf, int groupId, int volume, int pan, int16 flags) {
	int32 maxFrames;
	uint16 trkId, index;

	if (!_smushAudioInitialized)
		return;

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

		const int panDelta = (int8)srcBuf[17];
		const int effPan = (panDelta == -128) ? 128 : pan + panDelta;

		handleSAUDChunk(
			srcBuf + 18,
			READ_BE_UINT32(&srcBuf[4]) - 10,
			groupId,
			volume,
			effPan,
			flags,
			trkId,
			index,
			maxFrames);
	}
}

void SmushPlayerRebel2::handleGameGost(int32 subSize, Common::SeekableReadStream &b) {
	ra2HandleGost(subSize, b);
}

void SmushPlayerRebel2::handleGameProcessAudio(int16 feedSize) {
	if (_insane) {
		InsaneRebel2 *rebel2 = static_cast<InsaneRebel2 *>(_insane);
		rebel2->processAudioFrame(feedSize);
	}
}

} // End of namespace Scumm
