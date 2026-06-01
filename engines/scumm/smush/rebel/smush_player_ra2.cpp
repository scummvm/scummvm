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

// SmushPlayerRebel2 — RA2-specific SmushPlayer subclass
//
// Overrides the virtual hooks defined in SmushPlayer to provide
// Rebel Assault 2 specific video, font, text and codec handling.

#include "common/endian.h"
#include "common/rect.h"
#include "common/system.h"

#include "scumm/scumm.h"
#include "scumm/scumm_v7.h"
#include "scumm/smush/smush_font.h"
#include "scumm/smush/rebel/smush_multi_font.h"
#include "scumm/smush/rebel/codec_ra2.h"
#include "scumm/smush/rebel/smush_player_ra2.h"

#include "scumm/insane/insane.h"
#include "scumm/insane/rebel2/rebel.h"

namespace Scumm {

// ---------------------------------------------------------------------------
// SmushPlayerRebel2 — construction / destruction
// ---------------------------------------------------------------------------

SmushPlayerRebel2::SmushPlayerRebel2(ScummEngine_v7 *scumm, IMuseDigital *imuseDigital, Insane *insane)
	: SmushPlayer(scumm, imuseDigital, insane) {
	initGamePlayerFields();
}

SmushPlayerRebel2::~SmushPlayerRebel2() {
	destroyGamePlayerFields();
}

// ---------------------------------------------------------------------------
// Virtual hook overrides
// ---------------------------------------------------------------------------

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
	_loadReadOffset = 8;  // Original starts reading at offset 8 (skips header)
	_lastLoadChunkIdx = -1;
	_totalLoadChunks = 0;
	_ra2FrameSourceSkipY = 0;
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
}

/**
 * RA2-specific initialization in SmushPlayer::init().
 * Re-pushes the SMUSH palette (videos without NPAL inherit from previous),
 * and handles background preservation between cinematic and gameplay videos.
 */
void SmushPlayerRebel2::initGameVideoState() {
	// Re-push the SMUSH palette to the system. Videos like O_LEVEL.SAN
	// have no NPAL chunk and inherit the palette from the previous video.
	// Since play() resets _palDirtyMin/Max, the palette would never be pushed otherwise.
	setDirtyColors(0, 255);

	// Handle background preservation between videos:
	// - Cinematic videos (flags 0x20) clear the buffer for a fresh start
	// - Gameplay videos (flags 0x28) preserve the existing screen content
	if (_dst != nullptr) {
		VirtScreen *vs = &_vm->_virtscr[kMainVirtScreen];
		if ((_curVideoFlags & 0x08) == 0) {
			// Cinematic mode (flags 0x20) - clear buffer for fresh video
			memset(_dst, 0, vs->w * vs->h);
		}
		// Gameplay mode (flags 0x28): no-op, the existing screen content is preserved.
	}
}

/**
 * RA2-specific cleanup in SmushPlayer::release().
 * Frees stored FOBJ data but preserves _frameBuffer across videos.
 */
void SmushPlayerRebel2::releaseGameVideoState() {
	free(_storedFobjData);
	_storedFobjData = nullptr;
	_storedFobjDataSize = 0;
	_storedFobjParm2 = 0;
	free(_lastFobjData);
	_lastFobjData = nullptr;
	_lastFobjDataSize = 0;
	_hasFrameFobjForGost = false;
	// Preserve _frameBuffer across videos so that gameplay videos (which have no
	// background FOBJ) can use the stored background from the previous BEG video.
}

/**
 * RA2-specific FTCH handling.
 * For Handler 25, skips FTCH to preserve overlays.
 * For other handlers, re-decodes stored FOBJ with current offsets.
 */
bool SmushPlayerRebel2::handleGameFetch(int32 subSize, Common::SeekableReadStream &b) {
	int16 ftchUnknown = b.readSint16LE();
	int16 ftchX = b.readSint16LE();
	int16 ftchY = b.readSint16LE();

	debugC(DEBUG_SMUSH, "SmushPlayer::handleFetch: frame=%d unknown=%d x=%d y=%d",
		_frame, ftchUnknown, ftchX, ftchY);

	// For Handler 25, skip FTCH because the frame buffer only contains the
	// par4=5 base background without the overlays (par4=4, 6, 7) that were drawn
	// immediately in frame 0. Restoring would erase those overlays.
	if (_insane != nullptr) {
		InsaneRebel2 *rebel2 = static_cast<InsaneRebel2 *>(_insane);
		int handler = rebel2->getHandler();
		if (handler == 25) {
			debugC(DEBUG_SMUSH, "SmushPlayer::handleFetch: Skipping FTCH for Handler 25 - preserving overlays");
			return true;
		}
	}

	// Re-decode stored FOBJ data with current offsets (matching original FUN_004246d0).
	if (_storedFobjData != nullptr) {
		debugC(DEBUG_SMUSH, "SmushPlayer FTCH: Re-decoding stored FOBJ codec=%d pos=(%d,%d) size=%dx%d dataSize=%d",
			_storedFobjCodec, _storedFobjLeft, _storedFobjTop,
			_storedFobjWidth, _storedFobjHeight, _storedFobjDataSize);
		decodeFrameObject(_storedFobjCodec, _storedFobjData,
			_storedFobjLeft, _storedFobjTop,
			_storedFobjWidth, _storedFobjHeight,
			_storedFobjDataSize);
	} else {
		debugC(DEBUG_SMUSH, "SmushPlayer FTCH: No stored FOBJ data! (frame=%d)", _frame);
	}

	return true;
}

/**
 * RA2-specific text rendering using SmushMultiFont for inline font switching.
 */
bool SmushPlayerRebel2::handleGameTextRendering(const char *str, int fontId, int color,
												int pos_x, int pos_y, int left, int top,
												int width, int height, TextStyleFlags flg) {
	ra2HandleTextResource(str, fontId, color, pos_x, pos_y, left, top, width, height, flg);
	return true;
}

SmushFont *SmushPlayerRebel2::getGameFont(int font) {
	// Font table for low-res mode (320x200). Original exe uses pointer
	// arithmetic to select hi/lo font pairs (e.g. TKHIFONT+0x14=TALKFONT).
	// Font 0: TALKFONT (TKHIFONT hi-res)
	// Font 1: SMALFONT (SMHIFONT hi-res)
	// Font 2: TITLFONT (TIHIFONT hi-res)
	// Font 3: POVFONT  (POHIFONT hi-res)
	const char *ra2_fonts[] = {
		"SYSTM/TALKFONT.NUT",
		"SYSTM/SMALFONT.NUT",
		"SYSTM/TITLFONT.NUT",
		"SYSTM/POVFONT.NUT"
	};
	int numFonts = ARRAYSIZE(ra2_fonts);
	if (font >= 0 && font < numFonts) {
		_sf[font] = new SmushFont(_vm, ra2_fonts[font], true);
	} else {
		debugC(DEBUG_SMUSH, "SmushPlayer::getFont: RA2 unknown font %d, using TALKFONT", font);
		_sf[font] = new SmushFont(_vm, ra2_fonts[0], true);
	}
	return _sf[font];
}

// ---------------------------------------------------------------------------
// RA2 string resource loading — separate from shared StringResource
// ---------------------------------------------------------------------------

// RA2 TRS format differences from standard SCUMM:
//   - Up to ~658 entries (standard only supports 200)
//   - Entries can be empty (no content between header and next #)
//   - Content lines prefixed with //
//   - Multi-line entries preserve newlines (credits, cast lists)

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

			// Skip empty entries
			if (data_end <= data_start) { def_start = strchr(def_end + 1, '#'); continue; }

			// Strip leading // prefix
			if (data_start[0] == '/' && data_start[1] == '/')
				data_start += 2;
			if (data_end <= data_start) { def_start = strchr(def_end + 1, '#'); continue; }

			char *value = new char[data_end - data_start + 1];
			memcpy(value, data_start, data_end - data_start);
			value[data_end - data_start] = 0;

			// Preserve newlines for multi-line TRES text (credits, cast lists)
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

/**
 * Reset XPAL delta palette from the current base palette.
 * Prevents stale delta values from a previous video corrupting the palette.
 */
void SmushPlayerRebel2::adjustGamePalette() {
	for (int j = 0; j < 768; ++j) {
		_shiftedDeltaPal[j] = _pal[j] << 7;
	}
	memset(_deltaPal, 0, sizeof(_deltaPal));
}

/**
 * RA2-specific handleAnimHeader fixup: when AHDR reports 0x0 dimensions,
 * use screen dimensions instead.
 */
bool SmushPlayerRebel2::handleGameAnimHeader(byte *headerContent) {
	int width = READ_LE_UINT16(&headerContent[4]);
	int height = READ_LE_UINT16(&headerContent[6]);

	if (width == 0 && height == 0) {
		_width = _vm->_screenWidth;
		_height = _vm->_screenHeight;
		debugC(DEBUG_SMUSH, "SmushPlayer::handleAnimHeader: RA2 AHDR has 0x0 dims - using screen size %dx%d", _width, _height);
	} else {
		_width = width;
		_height = height;
	}
	return true;
}

// ---------------------------------------------------------------------------
// RA2 helper methods used by the base SmushPlayer pipeline.
// ---------------------------------------------------------------------------

void SmushPlayerRebel2::handleGameLoad(int32 subSize, Common::SeekableReadStream &b) {
	handleLoad(subSize, b);
}

/**
 * Handle LOAD chunk for Rebel Assault 2.
 *
 * LOAD chunks stream embedded resource data across multiple frames.
 * The data is accumulated in a buffer and consumed by the audio system.
 */
void SmushPlayerRebel2::handleLoad(int32 subSize, Common::SeekableReadStream &b) {
	debugC(DEBUG_SMUSH, "SmushPlayer::handleLoad()");

	if (subSize < 10) {
		warning("SmushPlayer::handleLoad: chunk too small (%d bytes)", subSize);
		return;
	}

	int16 totalChunks = b.readUint16LE();
	int16 chunkIndex = b.readUint16LE();
	b.skip(6);  // Unknown/padding

	int32 dataSize = subSize - 10;

	debugC(DEBUG_SMUSH, "SmushPlayer::handleLoad: chunk %d/%d, dataSize=%d, bufferOffset=%d",
		chunkIndex, totalChunks, dataSize, _loadBufferOffset);

	// First chunk in sequence - reset buffer state
	if (chunkIndex == 0) {
		_loadBufferOffset = 0;
		_loadReadOffset = 8;
		_lastLoadChunkIdx = -1;
		_totalLoadChunks = totalChunks;

		int32 estimatedSize = totalChunks * 600;
		if (_loadBuffer == nullptr || _loadBufferSize < estimatedSize) {
			free(_loadBuffer);
			_loadBufferSize = estimatedSize;
			_loadBuffer = (byte *)malloc(_loadBufferSize);
			if (_loadBuffer == nullptr) {
				warning("SmushPlayer::handleLoad: Failed to allocate %d bytes for LOAD buffer",
					_loadBufferSize);
				_loadBufferSize = 0;
				return;
			}
			debugC(DEBUG_SMUSH, "SmushPlayer::handleLoad: Allocated %d bytes for LOAD buffer",
				_loadBufferSize);
		}
	}

	// Check sequential order
	if (_lastLoadChunkIdx + 1 != chunkIndex) {
		debugC(DEBUG_SMUSH, "SmushPlayer::handleLoad: Non-sequential chunk %d (expected %d), skipping",
			chunkIndex, _lastLoadChunkIdx + 1);
		return;
	}

	// Check buffer capacity
	if (_loadBuffer == nullptr || _loadBufferOffset + dataSize >= _loadBufferSize) {
		warning("SmushPlayer::handleLoad: Buffer overflow - offset=%d size=%d limit=%d",
			_loadBufferOffset, dataSize, _loadBufferSize);
		return;
	}

	// Copy data to buffer
	b.read(_loadBuffer + _loadBufferOffset, dataSize);
	_loadBufferOffset += dataSize;
	_lastLoadChunkIdx = chunkIndex;

	debugC(DEBUG_SMUSH, "SmushPlayer::handleLoad: Accumulated %d bytes total", _loadBufferOffset);

	if (chunkIndex == totalChunks - 1) {
		debugC(DEBUG_SMUSH, "SmushPlayer::handleLoad: Sequence complete - %d chunks, %d bytes total",
			totalChunks, _loadBufferOffset);
	}
}

/**
 * RA2-specific text rendering using SmushMultiFont for inline font switching.
 */
void SmushPlayerRebel2::ra2HandleTextResource(const char *str, int fontId, int color,
										int pos_x, int pos_y, int left, int top,
										int width, int height, TextStyleFlags flg) {
	ensureMultiFont();
	_multiFont->setDefaultFont(fontId);

	debugC(DEBUG_SMUSH, "SmushPlayer::handleTextResource: RA2 TRES frame=%d fontId=%d color=%d flags=0x%x flg=%d pos=(%d,%d) clip=(%d,%d,%d,%d) str=\"%.40s\"",
		  _frame, fontId, color, (int)flg, (int)flg, pos_x, pos_y, left, top, width, height, str);

	if (flg & kStyleWordWrap) {
		Common::Rect clipRect(MAX<int>(0, left), MAX<int>(0, top), MIN<int>(left + width, _width), MIN<int>(top + height, _height));
		_multiFont->drawStringWrap(str, _dst, clipRect, pos_x, pos_y, color, flg);
	} else {
		Common::Rect clipRect(0, 0, _width, _height);
		_multiFont->drawString(str, _dst, clipRect, pos_x, pos_y, color, flg);
	}
}

/**
 * RA2-specific buffer selection for non-standard FOBJ dimensions.
 * Returns the destination buffer to use and updates _dst, _width, _height.
 */
void SmushPlayerRebel2::ra2SelectFrameBuffer(int width, int height) {
	// Rebel2 uses a special buffer for all non-matching frames.
	// Level 1: First frame is 424x260 (background), small sprites reuse same buffer
	// Level 2: Uses virtual screen directly (handled below when _specialBuffer stays null)
	int bufSize = width * height;
	if (bufSize > _vm->_screenWidth * _vm->_screenHeight) {
		// Frame is larger than screen - need special buffer
		if (_specialBuffer == nullptr || bufSize > _specialBufferSize) {
			free(_specialBuffer);
			_specialBuffer = (byte *)malloc(bufSize);
			_specialBufferSize = bufSize;
			_width = width;
			_height = height;
			// Zero-fill the new buffer to avoid garbage in areas not written by FOBJ codec
			memset(_specialBuffer, 0, bufSize);
			debugC(DEBUG_SMUSH, "SmushPlayer: Allocated new _specialBuffer %dx%d (%d bytes)", width, height, bufSize);
		}
	}

	if (bufSize > _vm->_screenWidth * _vm->_screenHeight &&
	    _specialBuffer != nullptr && _specialBufferSize >= bufSize) {
		_dst = _specialBuffer;
		debugC(DEBUG_SMUSH, "SmushPlayer: Using _specialBuffer for oversized FOBJ %dx%d", width, height);
	} else {
		if (_specialBuffer == nullptr) {
			VirtScreen *vs = &_vm->_virtscr[kMainVirtScreen];
			_dst = vs->getPixels(0, 0);
			debugC(DEBUG_SMUSH, "SmushPlayer: Reset _dst to virtual screen for FOBJ %dx%d at (%d,%d) _dst=%p",
				width, height, 0, 0, (void*)_dst);
		} else {
			// Large frame was in this video, use _specialBuffer for compositing
			_dst = _specialBuffer;
			debugC(DEBUG_SMUSH, "SmushPlayer: Using _specialBuffer for small FOBJ %dx%d (compositing with large frame)",
				width, height);
		}
	}
}

/**
 * Dispatch to RA2-specific codec functions.
 * Returns true if the codec was handled, false for standard codecs.
 */
bool SmushPlayerRebel2::ra2DecodeCodec(int codec, const uint8 *src, int left, int top,
								 int width, int height, int pitch, int dataSize) {
	switch (codec) {
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
		const uint8 *adjustedSrc = smushSkipRLELines(src, dataSize, _ra2FrameSourceSkipY);
		smushDecodeRA2Bomp(_dst, adjustedSrc, left, top, width, height, pitch, dataSize);
		return true;
	}
	default:
		return false;
	}
}

/**
 * Save raw FOBJ data when STOR is pending (for later re-decoding by FTCH).
 */
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

/**
 * RA2 GOST chunk handler.
 * Re-renders the most recent frame FOBJ at the supplied ghost position.
 */
void SmushPlayerRebel2::ra2HandleGost(int32 subSize, Common::SeekableReadStream &b) {
	if (subSize < 6) {
		warning("SmushPlayerRebel2::ra2HandleGost: chunk too small (%d bytes)", subSize);
		return;
	}

	int16 ghostType = b.readSint16LE();
	int16 ghostX = b.readSint16LE();
	int16 ghostY = b.readSint16LE();

	if (!_hasFrameFobjForGost || _lastFobjData == nullptr || _lastFobjDataSize <= 0) {
		debugC(DEBUG_SMUSH, "SmushPlayer GOST: frame=%d ignored (no current-frame FOBJ cached)", _frame);
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

	// Match FUN_0042cba0 default behavior (flags bit 0 clear): GOST coordinates
	// are relative to the cached FOBJ header position.
	int left = _lastFobjLeft + ghostX;
	int top = _lastFobjTop + ghostY;

	debugC(DEBUG_SMUSH, "SmushPlayer GOST: frame=%d type=%d flags=0x%04x gostPos=(%d,%d) basePos=(%d,%d) finalPos=(%d,%d) size=%dx%d codec=%d",
		_frame, ghostType, priorityFlags, ghostX, ghostY,
		_lastFobjLeft, _lastFobjTop, left, top,
		_lastFobjWidth, _lastFobjHeight, _lastFobjCodec);

	// Priority bits (0x2000/0x4000/0x6000) are currently not modeled in
	// ScummVM's SMUSH decoders. Coordinate-correct re-decode restores expected
	// RA2 chapter preview behavior.
	decodeFrameObject(_lastFobjCodec, _lastFobjData, left, top,
		_lastFobjWidth, _lastFobjHeight, _lastFobjDataSize);
}

/**
 * RA2 per-frame audio processing.
 */
void SmushPlayerRebel2::handleGameParseNextFrame() {
	// Call processDispatches directly since RA2 has no iMUSE
	// 11025 Hz / 12 fps = ~918 samples per frame
	processDispatches(_smushAudioSampleRate / 12);
}

// ---------------------------------------------------------------------------
// Frame decode pipeline overrides
// ---------------------------------------------------------------------------

bool SmushPlayerRebel2::handleGameFrameBufferSelect(int codec, int width, int height) {
	if ((height != _vm->_screenHeight) || (width != _vm->_screenWidth)) {
		ra2SelectFrameBuffer(width, height);
		return true;
	}
	return false;
}

bool SmushPlayerRebel2::handleGameDimensionOverride(int codec, int width, int height) {
	if ((height != _vm->_screenHeight) || (width != _vm->_screenWidth)) {
		// RA2: preserve _width/_height set during buffer allocation
		return true;
	}
	return false;
}

bool SmushPlayerRebel2::handleGameAdjustCoords(int codec, int &left, int &top, int &width, int &height, int pitch, int *srcSkipY) {
	int sourceSkipY = 0;
	_ra2FrameSourceSkipY = 0;
	adjustFrameCoords(left, top, width, height, pitch, &sourceSkipY);
	if (codec == SMUSH_CODEC_LINE_UPDATE || codec == SMUSH_CODEC_LINE_UPDATE2 ||
			codec == SMUSH_CODEC_SKIP_RLE || codec == SMUSH_CODEC_RA2_BOMP) {
		_ra2FrameSourceSkipY = sourceSkipY;
		if (srcSkipY)
			*srcSkipY = 0;
	} else if (srcSkipY) {
		*srcSkipY = sourceSkipY;
	}
	return true;
}

bool SmushPlayerRebel2::handleGameCodecDecode(int codec, const uint8 *src, int left, int top, int width, int height, int pitch, int dataSize, uint8 param, uint16 parm2) {
	// Handle RA2-specific codecs (21, 23, 44, 45); return false for standard
	// codecs (RLE, uncompressed, codec 37/47) so the base class decodes them.
	return ra2DecodeCodec(codec, src, left, top, width, height, pitch, dataSize);
}

bool SmushPlayerRebel2::handleGameStoreFrame() {
	// RA2 handles STOR via ra2StoreFobjData in handleGameFrameObjectPost
	return true;
}

void SmushPlayerRebel2::handleGameFrameObjectPre(int codec, int left, int top, int width, int height, int dataSize) {
	debugC(DEBUG_SMUSH, "SmushPlayer FOBJ: frame=%d codec=%d pos=(%d,%d) size=%dx%d dataSize=%d storeFrame=%d _width=%d _height=%d",
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
}

bool SmushPlayerRebel2::handleGameSkipChunk(uint32 subType, int32 subSize, Common::SeekableReadStream &b) {
	if (_skipNext) {
		_skipNext = false;
		debugC(DEBUG_SMUSH, "SmushPlayer::handleFrame: SKIP consumed chunk %s frame=%d", tag2str(subType), _frame);
		return true;
	}
	return false;
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
