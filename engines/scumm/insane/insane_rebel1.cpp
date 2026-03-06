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

#include "common/system.h"
#include "common/events.h"
#include "common/endian.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/mixer.h"
#include "graphics/cursorman.h"
#include "graphics/wincursor.h"
#include "scumm/scumm_v7.h"
#include "scumm/scumm.h"
#include "scumm/smush/smush_player.h"
#include "scumm/insane/insane_rebel1.h"

namespace Scumm {

// RA1 coordinate constants (scaled from RA2's 424x260 → 384x242)
// Original coordinate space: 320x200 game viewport at (0,0) in the 384x242 buffer.
// Ship base position in the original: (0xA0, 100) = (160, 100) = center of 320x200.
// Accumulator range: ±0x82 (~±130), so ship can reach ~(30..290, -30..230).
static const int16 kCenterX = 160;  // _DAT_74B6 init = 0xA0
static const int16 kCenterY = 100;  // _DAT_74B8 init = 100
static const int16 kMinX = 20;
static const int16 kMaxX = 300;
static const int16 kMinY = 20;
static const int16 kMaxY = 180;

// Perspective focal lengths (from original tuning table)
static const int16 kFocalX = 43;    // 0x2b
static const int16 kFocalY = 25;    // 0x19

// Per-difficulty tuning tables from assault_data_3.bin
// Indexed: difficulty * 0x28B + level * 0x1F + offset
// Level 1 values (level index 0):
//                                roll  lift  slide drift wham  shot
static const int16 kTuningLevel1[3][6] = {
	{ 100, 100,  60, 110,  15,   0 },  // Easy
	{ 100, 105,  60, 115,  25,   0 },  // Normal
	{ 105, 110,  65, 120,  30,   0 },  // Hard
};

// Decode BOMP RLE (codec 21) sprite data into a flat pixel buffer.
// Same algorithm as NutRenderer::codec21 but without palette tracking.
static void decodeBomp(byte *dst, const byte *src, int width, int height, int pitch) {
	while (height--) {
		byte *dstNext = dst + pitch;
		const byte *srcNext = src + 2 + READ_LE_UINT16(src);
		src += 2;
		int len = width;
		byte *d = dst;
		do {
			int offs = READ_LE_UINT16(src); src += 2;
			d += offs;
			len -= offs;
			if (len <= 0)
				break;
			int w = READ_LE_UINT16(src) + 1; src += 2;
			len -= w;
			if (len < 0)
				w += len;
			memcpy(d, src, w);
			src += w;
			d += w;
		} while (len > 0);
		dst = dstNext;
		src = srcNext;
	}
}

void InsaneRebel1::loadTuningForLevel(int level) {
	int d = CLIP(_difficulty, 0, 2);
	// Currently only Level 1 tuning is hardcoded; extend for other levels later
	(void)level;
	_tuning.roll  = kTuningLevel1[d][0];
	_tuning.lift  = kTuningLevel1[d][1];
	_tuning.slide = kTuningLevel1[d][2];
	_tuning.drift = kTuningLevel1[d][3];
	_tuning.wham  = kTuningLevel1[d][4];
	_tuning.shot  = kTuningLevel1[d][5];
	debug(1, "RA1: Loaded tuning for difficulty %d: roll=%d lift=%d slide=%d drift=%d wham=%d shot=%d",
		d, _tuning.roll, _tuning.lift, _tuning.slide, _tuning.drift, _tuning.wham, _tuning.shot);
}

InsaneRebel1::InsaneRebel1(ScummEngine_v7 *scumm) : Insane(), _vm(scumm) {
	_screenWidth = 384;
	_screenHeight = 242;

	_shipPosX = kCenterX;
	_shipPosY = kCenterY;
	_shipDirIndex = 17;  // Center of 5x7 grid (2*7 + 3)

	_corridorLeftX = kMinX;
	_corridorTopY = kMinY;
	_corridorRightX = kMaxX;
	_corridorBottomY = kMaxY;

	_rollAccum = 0;
	_liftSmooth = 0;
	_posAccumX = 0;
	_posAccumY = 0;
	_driftParam = 0;

	_difficulty = 0;  // Easy by default
	loadTuningForLevel(0);

	_perspectiveX = 0;
	_perspectiveY = 0;

	_flyControlMode = 0;

	_health = kMaxHealth;
	_lives = 3;
	_score = 0;
	_damageFlags = 0;
	_prevDamageFlags = 0;
	_gameLatch5D = 0;
	_gameLatch5F = 0;
	_damageCooldown = 0;
	_deathTimer = 0;
	_screenFlash = 0;
	_frameCounter = 0;
	_interactiveVideoActive = false;
	_gameCounter = 0;
	_pathBranchEnabled = false;
	_rightPathSelected = false;
	_menuActive = false;
	_menuConfirmed = false;
	_menuSelection = 0;
	_menuFrameCounter = 0;
	_optionsActive = false;
	_optionsSel = 0;
	_turbulenceEnabled = false;
	if (loadRA1Nut("SYS/TALKFONT.NUT", _hudFontBank)) {
		debug(1, "InsaneRebel1: HUD/menu glyph font loaded from SYS/TALKFONT.NUT (%d chars)", _hudFontBank.numSprites);
	} else if (loadRA1Nut("SYS/TECHFONT.NUT", _hudFontBank)) {
		debug(1, "InsaneRebel1: HUD/menu glyph font loaded from SYS/TECHFONT.NUT (%d chars)", _hudFontBank.numSprites);
	} else {
		warning("InsaneRebel1: failed to load RA1 HUD font bank (TECHFONT/TALKFONT)");
	}

	// Audio
	initAudio(11025);

	// Null out Insane base class pointers that the default constructor doesn't initialize
	_smush_roadrashRip = nullptr;
	_smush_roadrsh2Rip = nullptr;
	_smush_roadrsh3Rip = nullptr;
	_smush_goglpaltRip = nullptr;
	_smush_tovista1Flu = nullptr;
	_smush_tovista2Flu = nullptr;
	_smush_toranchFlu = nullptr;
	_smush_minedrivFlu = nullptr;
	_smush_minefiteFlu = nullptr;
	_smush_bensgoggNut = nullptr;
	_smush_bencutNut = nullptr;
	_smush_iconsNut = nullptr;
	_smush_icons2Nut = nullptr;

	_vm->_system->getEventManager()->getEventDispatcher()->registerObserver(this, 1, false);
}

InsaneRebel1::~InsaneRebel1() {
	_vm->_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);
	terminateAudio();
}

bool InsaneRebel1::notifyEvent(const Common::Event &event) {
	if (_menuActive && _optionsActive && event.type == Common::EVENT_KEYDOWN) {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_UP:
		case Common::KEYCODE_w:
			_optionsSel = (_optionsSel + 2) % 3;
			return true;
		case Common::KEYCODE_DOWN:
		case Common::KEYCODE_s:
			_optionsSel = (_optionsSel + 1) % 3;
			return true;
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
		case Common::KEYCODE_SPACE:
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		case Common::KEYCODE_ESCAPE:
			_optionsSel = 2;  // Back
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		default:
			break;
		}
	}

	if (_menuActive && !_optionsActive && event.type == Common::EVENT_KEYDOWN) {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_UP:
		case Common::KEYCODE_w:
			_menuSelection = (_menuSelection + 4) % 5;
			return true;
		case Common::KEYCODE_DOWN:
		case Common::KEYCODE_s:
			_menuSelection = (_menuSelection + 1) % 5;
			return true;
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
		case Common::KEYCODE_SPACE:
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		case Common::KEYCODE_1:
		case Common::KEYCODE_2:
		case Common::KEYCODE_3:
		case Common::KEYCODE_4:
		case Common::KEYCODE_5:
			_menuSelection = event.kbd.keycode - Common::KEYCODE_1;
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		case Common::KEYCODE_ESCAPE:
			_menuSelection = 4;
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		default:
			break;
		}
	}

	if (event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) {
		if (_player) {
			debug("Rebel1: ESC pressed - skipping video");
			_vm->_smushVideoShouldFinish = true;
			return true;
		}
	}

	return false;
}

// ---------------------------------------------------------------------------
// Audio
// ---------------------------------------------------------------------------

void InsaneRebel1::initAudio(int sampleRate) {
	_audioSampleRate = sampleRate;
	for (int i = 0; i < kMaxAudioTracks; i++) {
		_audioStreams[i] = nullptr;
		_audioTrackActive[i] = false;
	}
}

void InsaneRebel1::terminateAudio() {
	for (int i = 0; i < kMaxAudioTracks; i++) {
		if (_audioTrackActive[i]) {
			_vm->_mixer->stopHandle(_audioHandles[i]);
			_audioTrackActive[i] = false;
		}
		if (_audioStreams[i]) {
			_audioStreams[i]->finish();
			_audioStreams[i] = nullptr;
		}
	}
}

void InsaneRebel1::queueAudioData(int trackIdx, uint8 *data, int32 size, int volume, int pan) {
	if (trackIdx < 0 || trackIdx >= kMaxAudioTracks || size <= 0 || !data)
		return;

	if (!_audioStreams[trackIdx]) {
		debug(1, "InsaneRebel1: Creating audio stream for track %d at %d Hz", trackIdx, _audioSampleRate);
		_audioStreams[trackIdx] = Audio::makeQueuingAudioStream(_audioSampleRate, false);
		_audioTrackActive[trackIdx] = true;
		_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_audioHandles[trackIdx],
								_audioStreams[trackIdx], -1, Audio::Mixer::kMaxChannelVolume, 0,
								DisposeAfterUse::NO);
	}

	byte *audioCopy = (byte *)malloc(size);
	if (!audioCopy)
		return;
	memcpy(audioCopy, data, size);

	_audioStreams[trackIdx]->queueBuffer(audioCopy, size, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);

	int scaledVolume = (volume * Audio::Mixer::kMaxChannelVolume) / 127;
	int scaledPan = (pan * 127) / 128;
	_vm->_mixer->setChannelVolume(_audioHandles[trackIdx], scaledVolume);
	_vm->_mixer->setChannelBalance(_audioHandles[trackIdx], scaledPan);
}

void InsaneRebel1::processAudioFrame(int16 feedSize) {
	if (!_player)
		return;

	SmushPlayer *sp = _player;

	if (sp->_smushTracksNeedInit) {
		sp->_smushTracksNeedInit = false;
		for (int i = 0; i < SMUSH_MAX_TRACKS; i++) {
			sp->_smushDispatch[i].fadeRemaining = 0;
			sp->_smushDispatch[i].fadeVolume = 0;
			sp->_smushDispatch[i].fadeSampleRate = 0;
			sp->_smushDispatch[i].elapsedAudio = 0;
			sp->_smushDispatch[i].audioLength = 0;
		}
	}

	for (int i = 0; i < sp->_smushNumTracks; i++) {
		SmushPlayer::SmushAudioTrack &track = sp->_smushTracks[i];
		SmushPlayer::SmushAudioDispatch &dispatch = sp->_smushDispatch[i];

		if (track.state == TRK_STATE_INACTIVE || !track.blockPtr)
			continue;

		bool isPlayableTrack = ((track.flags & TRK_TYPE_MASK) == IS_SPEECH && sp->isChanActive(CHN_SPEECH)) ||
							   ((track.flags & TRK_TYPE_MASK) == IS_BKG_MUSIC && sp->isChanActive(CHN_BKGMUS)) ||
							   ((track.flags & TRK_TYPE_MASK) == IS_SFX && sp->isChanActive(CHN_OTHER));

		if (!isPlayableTrack)
			continue;

		int baseVolume;
		switch (track.flags & TRK_TYPE_MASK) {
		case IS_SFX:
			baseVolume = (sp->_smushTrackVols[1] * track.volume) >> 7;
			break;
		case IS_BKG_MUSIC:
			baseVolume = (sp->_smushTrackVols[3] * track.volume) >> 7;
			break;
		case IS_SPEECH:
			baseVolume = (sp->_smushTrackVols[2] * track.volume) >> 7;
			break;
		default:
			baseVolume = track.volume;
			break;
		}
		int mixVolume = baseVolume * sp->_smushTrackVols[0] / 127;

		// Handle FADING -> PLAYING transition
		if (track.state == TRK_STATE_FADING) {
			dispatch.headerPtr = track.dataBuf;
			dispatch.dataBuf = track.subChunkPtr;
			dispatch.dataSize = track.dataSize;
			dispatch.currentOffset = 0;
			dispatch.audioLength = 0;
			track.state = TRK_STATE_PLAYING;
		}

		if (track.state != TRK_STATE_INACTIVE) {
			int32 tmpFeedSize = feedSize;

			while (tmpFeedSize > 0) {
				int32 mixInFrameCount = dispatch.currentOffset;

				if (mixInFrameCount > 0 && dispatch.dataBuf && dispatch.dataSize > 0) {
					if (dispatch.audioRemaining < 0)
						dispatch.audioRemaining = 0;

					int32 offset = dispatch.audioRemaining % dispatch.dataSize;

					if (dispatch.sampleRate > 0 && sp->_smushAudioSampleRate > 0) {
						int32 maxFrames = dispatch.sampleRate * tmpFeedSize / sp->_smushAudioSampleRate;
						if (mixInFrameCount > maxFrames)
							mixInFrameCount = maxFrames;
					}

					if (offset + mixInFrameCount > dispatch.dataSize)
						mixInFrameCount = dispatch.dataSize - offset;

					if (dispatch.audioRemaining + mixInFrameCount > track.availableSize) {
						mixInFrameCount = track.availableSize - dispatch.audioRemaining;
						if (mixInFrameCount <= 0) {
							track.state = TRK_STATE_ENDING;
							break;
						}
					}

					if (mixInFrameCount > 0) {
						if (!dispatch.dataBuf || offset < 0 || offset + mixInFrameCount > dispatch.dataSize)
							break;

						queueAudioData(i, &dispatch.dataBuf[offset], mixInFrameCount, mixVolume, track.pan);

						dispatch.currentOffset -= mixInFrameCount;
						dispatch.audioRemaining += mixInFrameCount;

						if (dispatch.sampleRate > 0) {
							int32 consumedFeed = mixInFrameCount * sp->_smushAudioSampleRate / dispatch.sampleRate;
							tmpFeedSize -= consumedFeed;
						} else {
							tmpFeedSize -= mixInFrameCount;
						}
					}
				}

				if (dispatch.currentOffset <= 0) {
					if (!sp->processAudioCodes(i, tmpFeedSize, mixVolume))
						break;
					if (dispatch.currentOffset <= 0)
						break;
				} else if (tmpFeedSize <= 0) {
					break;
				}
			}
		}

		track.audioRemaining = dispatch.audioRemaining;
		dispatch.state = track.state;
	}
}

// Load an RA1 NUT sprite file (ANIM v1).
// RA1 NUTs can have odd-size FOBJ chunks padded to 2-byte alignment within
// FRME containers. This loader handles that padding properly, unlike the
// shared NutRenderer::loadFont which assumes even-size chunks.
bool InsaneRebel1::loadRA1Nut(const char *filename, RA1SpriteBank &bank) {
	ScummFile *file = _vm->instantiateScummFile();
	_vm->openFile(*file, filename);
	if (!file->isOpen()) {
		warning("InsaneRebel1::loadRA1Nut: can't open %s", filename);
		delete file;
		return false;
	}

	uint32 tag = file->readUint32BE();
	if (tag != MKTAG('A','N','I','M')) {
		warning("InsaneRebel1::loadRA1Nut: no ANIM tag in %s", filename);
		delete file;
		return false;
	}
	uint32 animSize = file->readUint32BE();
	byte *data = (byte *)malloc(animSize);
	file->read(data, animSize);
	file->close();
	delete file;

	// data[0..3] = AHDR tag, data[4..7] = AHDR size
	if (READ_BE_UINT32(data) != MKTAG('A','H','D','R')) {
		warning("InsaneRebel1::loadRA1Nut: no AHDR in %s", filename);
		free(data);
		return false;
	}

	const uint16 expectedSprites = READ_LE_UINT16(data + 10);
	bank.numSprites = expectedSprites;
	bank.sprites = new RA1Sprite[bank.numSprites];
	memset(bank.sprites, 0, sizeof(RA1Sprite) * bank.numSprites);

	uint32 *fobjOffsets = (uint32 *)calloc(expectedSprites, sizeof(uint32));
	if (!fobjOffsets) {
		free(data);
		return false;
	}

	// Pass 1: Parse ANIM chunks properly and collect FRME->FOBJ offsets in-order.
	uint32 decodedSize = 0;
	uint16 foundSprites = 0;
	uint32 chunkOffset = 0;
	while (chunkOffset + 8 <= animSize && foundSprites < expectedSprites) {
		uint32 chunkTag = READ_BE_UINT32(data + chunkOffset);
		uint32 chunkSize = READ_BE_UINT32(data + chunkOffset + 4);
		uint32 chunkDataOffset = chunkOffset + 8;
		uint32 chunkEnd = chunkDataOffset + chunkSize;
		if (chunkEnd > animSize)
			break;

		if (chunkTag == MKTAG('F','R','M','E')) {
			bool foundFobj = false;
			uint32 subOffset = chunkDataOffset;
			while (subOffset + 8 <= chunkEnd) {
				uint32 subTag = READ_BE_UINT32(data + subOffset);
				uint32 subSize = READ_BE_UINT32(data + subOffset + 4);
				uint32 subDataOffset = subOffset + 8;
				uint32 subEnd = subDataOffset + subSize;
				if (subEnd > chunkEnd)
					break;

				if (subTag == MKTAG('F','O','B','J') && subOffset + 22 <= animSize) {
					uint16 w = READ_LE_UINT16(data + subOffset + 14);
					uint16 h = READ_LE_UINT16(data + subOffset + 16);
					decodedSize += (uint32)w * (uint32)h;
					fobjOffsets[foundSprites] = subOffset;
					foundFobj = true;
					break;
				}

				subOffset = subEnd;
				if (subSize & 1)
					subOffset++;
			}
			// Always increment for every FRME to preserve char-to-glyph alignment.
			// Empty FRMEs (no FOBJ) keep fobjOffsets[i] = 0, decoded as blank sprites.
			foundSprites++;
		}

		chunkOffset = chunkEnd;
		if (chunkSize & 1)
			chunkOffset++;
	}

	bank.decodedData = (byte *)calloc(decodedSize ? decodedSize : 1, 1);
	bank.decodedSize = decodedSize;
	byte *decPtr = bank.decodedData;

	// Pass 2: Decode collected FOBJ entries.
	for (uint16 i = 0; i < foundSprites; i++) {
		uint32 fobjOffset = fobjOffsets[i];
		if (fobjOffset == 0) {
			// Empty FRME (no FOBJ) — leave sprite as blank (zeroed by memset).
			continue;
		}

		int codec = READ_LE_UINT16(data + fobjOffset + 8);
		bank.sprites[i].xoffs = READ_LE_INT16(data + fobjOffset + 10);
		bank.sprites[i].yoffs = READ_LE_INT16(data + fobjOffset + 12);
		bank.sprites[i].width = READ_LE_UINT16(data + fobjOffset + 14);
		bank.sprites[i].height = READ_LE_UINT16(data + fobjOffset + 16);

		int pixelCount = bank.sprites[i].width * bank.sprites[i].height;
		const byte *fobjData = data + fobjOffset + 22;

		if (codec == 21) {
			bank.sprites[i].data = decPtr;
			decodeBomp(decPtr, fobjData, bank.sprites[i].width,
					   bank.sprites[i].height, bank.sprites[i].width);
		} else {
			bank.sprites[i].width = 0;
			bank.sprites[i].height = 0;
			bank.sprites[i].data = nullptr;
			warning("InsaneRebel1::loadRA1Nut: unsupported codec %d in sprite %d", codec, i);
		}

		decPtr += pixelCount;
	}

	free(fobjOffsets);

	free(data);
	debug(1, "InsaneRebel1::loadRA1Nut('%s'): expected=%d found=%d decoded=%d bytes",
		  filename, expectedSprites, foundSprites, decodedSize);
	return true;
}

void InsaneRebel1::loadLevelSprites(int level) {
	Common::String filename = Common::String::format("LVL%d/L%dBANK1.NUT", level, level);
	loadRA1Nut(filename.c_str(), _shipBank);
	loadRA1Nut("SYS/DISPLAY.NUT", _displayBank);

	Common::String bangFile = Common::String::format("LVL%d/L%dBANG.NUT", level, level);
	loadRA1Nut(bangFile.c_str(), _bangBank);
}

void InsaneRebel1::procPreRendering(byte *renderBitmap) {
}

void InsaneRebel1::procPostRendering(byte *renderBitmap, int32 codecparam, int32 setupsan12,
	int32 setupsan13, int32 curFrame, int32 maxFrame) {
	if (_menuActive && renderBitmap) {
		int width = _player ? _player->_width : 0;
		int height = _player ? _player->_height : 0;
		if (width == 0) width = _screenWidth;
		if (height == 0) height = _screenHeight;
		int pitch = width;
		renderMainMenuOverlay(renderBitmap, pitch, width, height);
	}

	if (!_interactiveVideoActive || _shipBank.numSprites == 0 || !renderBitmap)
		return;

	int width = _player->_width;
	int height = _player->_height;
	if (width == 0) width = _screenWidth;
	if (height == 0) height = _screenHeight;
	int pitch = width;

	updateShipPhysics();
	renderShip(renderBitmap, pitch, width, height);
	renderExplosions(renderBitmap, pitch, width, height);
	renderHUD(renderBitmap, pitch, width, height);
}

void InsaneRebel1::drawFontBankString(byte *dst, int pitch, int width, int height, int x, int y, const char *text) {
	if (!dst || !text || _hudFontBank.numSprites <= 0)
		return;

	for (int i = 0; text[i] != '\0'; i++) {
		const byte ch = (byte)text[i];

		if (ch == ' ') {
			x += 6;
			continue;
		}

		// RA1 font renderer indexes printable characters from '!' (0x21), not raw ASCII.
		if (ch < 0x21) {
			x += 4;
			continue;
		}
		const int fontIdx = (int)ch - 0x21;
		if (fontIdx < 0 || fontIdx >= _hudFontBank.numSprites) {
			x += 4;
			continue;
		}

		const RA1Sprite &glyph = _hudFontBank.sprites[fontIdx];
		const int gw = glyph.width;
		const int gh = glyph.height;
		const int gx = x + glyph.xoffs;
		const int gy = y + glyph.yoffs;
		const uint64 glyphPixels = (uint64)gw * (uint64)gh;
		if (!glyph.data || gw <= 0 || gh <= 0 || glyphPixels == 0 || glyphPixels > 0x10000) {
			x += 4;
			continue;
		}
		if (!(_hudFontBank.decodedData && _hudFontBank.decodedSize > 0)) {
			x += 4;
			continue;
		}
		const byte *bankStart = _hudFontBank.decodedData;
		const byte *bankEnd = _hudFontBank.decodedData + _hudFontBank.decodedSize;
		if (glyph.data < bankStart || glyph.data >= bankEnd || glyph.data + glyphPixels > bankEnd) {
			x += 4;
			continue;
		}

		for (int py = 0; py < gh; py++) {
			const int sy = gy + py;
			if (sy < 0 || sy >= height)
				continue;
			for (int px = 0; px < gw; px++) {
				const int sx = gx + px;
				if (sx < 0 || sx >= width)
					continue;
				const byte pixel = glyph.data[py * gw + px];
				if (pixel != 0)
					dst[sy * pitch + sx] = pixel;
			}
		}

		x += gw > 0 ? gw : 4;
	}
}

// getFontBankStringWidth -- Measure pixel width of a string using the HUD font bank.
// Matches the pre-pass width calculation in the original drawString (FUN_221B7).
int InsaneRebel1::getFontBankStringWidth(const char *text) {
	if (!text || _hudFontBank.numSprites <= 0)
		return 0;

	int w = 0;
	for (int i = 0; text[i] != '\0'; i++) {
		const byte ch = (byte)text[i];
		if (ch == ' ') {
			w += 6;
			continue;
		}
		if (ch < 0x21) {
			w += 4;
			continue;
		}
		const int fontIdx = (int)ch - 0x21;
		if (fontIdx < 0 || fontIdx >= _hudFontBank.numSprites) {
			w += 4;
			continue;
		}
		const RA1Sprite &glyph = _hudFontBank.sprites[fontIdx];
		w += glyph.width > 0 ? glyph.width : 4;
	}
	return w;
}

// renderMainMenuOverlay -- Draw menu text and selection highlight box.
// Original menu strings from assault_data_3.bin at 0x5822.
// Highlight uses RA2-style flashing border box (FUN_004292d0 pattern).
void InsaneRebel1::renderMainMenuOverlay(byte *dst, int pitch, int width, int height) {
	_menuFrameCounter++;

	if (_optionsActive) {
		// --- Options submenu ---
		static const char *kDiffNames[3] = { "EASY", "NORMAL", "HARD" };

		const int titleW = getFontBankStringWidth("GAME OPTIONS");
		drawFontBankString(dst, pitch, width, height, (width - titleW) / 2, 36, "GAME OPTIONS");

		// Build dynamic option strings
		char diffLine[64];
		snprintf(diffLine, sizeof(diffLine), "DIFFICULTY: %s", kDiffNames[CLIP(_difficulty, 0, 2)]);
		const char *turbLine = _turbulenceEnabled ? "TURBULENCE: ON" : "TURBULENCE: OFF";
		const char *kOptionsItems[3] = { diffLine, turbLine, "BACK" };

		const int menuY = 60;
		const int rowH = 16;

		for (int i = 0; i < 3; i++) {
			const int textW = getFontBankStringWidth(kOptionsItems[i]);
			const int textX = (width - textW) / 2;
			const int y = menuY + i * rowH;
			drawFontBankString(dst, pitch, width, height, textX, y + 1, kOptionsItems[i]);

			if (i == _optionsSel) {
				byte highlightColor = ((_menuFrameCounter / 8) & 1) ? 248 : 240;
				int bracketWidth = textW + 12;
				int leftX = CLIP(textX - 6, 0, width - 1);
				int rightX = CLIP(leftX + bracketWidth, 0, width - 1);
				int topY = CLIP(y - 1, 0, height - 1);
				int bottomY = CLIP(y + rowH - 2, 0, height - 1);
				for (int x = leftX; x <= rightX; x++) {
					dst[topY * pitch + x] = highlightColor;
					dst[bottomY * pitch + x] = highlightColor;
				}
				for (int py = topY; py <= bottomY; py++) {
					dst[py * pitch + leftX] = highlightColor;
					dst[py * pitch + rightX] = highlightColor;
				}
			}
		}
		return;
	}

	// --- Main menu ---
	static const char *kMenuItems[5] = {
		"START NEW GAME",
		"GAME OPTIONS",
		"ENTER PASSCODE",
		"CONTINUE DEMO",
		"EXIT TO DOS"
	};

	// Center title
	const int titleW = getFontBankStringWidth("MAIN MENU");
	const int titleX = (width - titleW) / 2;
	drawFontBankString(dst, pitch, width, height, titleX, 36, "MAIN MENU");

	// Draw menu items centered horizontally
	const int menuY = 60;
	const int rowH = 16;

	for (int i = 0; i < 5; i++) {
		const int textW = getFontBankStringWidth(kMenuItems[i]);
		const int textX = (width - textW) / 2;
		const int y = menuY + i * rowH;

		drawFontBankString(dst, pitch, width, height, textX, y + 1, kMenuItems[i]);

		// Selection highlight box — flashing border (FUN_004292d0 pattern from RA2)
		if (i == _menuSelection) {
			// Flash between two palette colors every 8 frames
			byte highlightColor = ((_menuFrameCounter / 8) & 1) ? 248 : 240;

			int bracketWidth = textW + 12;
			int bracketHeight = rowH;
			int leftX = textX - 6;
			int rightX = leftX + bracketWidth;
			int topY = y - 1;
			int bottomY = y + bracketHeight - 2;

			// Clamp
			if (leftX < 0) leftX = 0;
			if (rightX >= width) rightX = width - 1;
			if (topY < 0) topY = 0;
			if (bottomY >= height) bottomY = height - 1;

			// Draw rectangle border (4 lines)
			for (int x = leftX; x <= rightX && x < width; x++) {
				if (topY >= 0 && topY < height)
					dst[topY * pitch + x] = highlightColor;
				if (bottomY >= 0 && bottomY < height)
					dst[bottomY * pitch + x] = highlightColor;
			}
			for (int py = topY; py <= bottomY && py < height; py++) {
				if (leftX >= 0 && leftX < width)
					dst[py * pitch + leftX] = highlightColor;
				if (rightX >= 0 && rightX < width)
					dst[py * pitch + rightX] = highlightColor;
			}
		}
	}
}

// Ship physics matching FUN_1DEB5 (accumulator-based position system).
// Roll accumulator (_74CA) driven by input, position accumulators (_74C2/_74C6)
// driven by roll + drift + cross-coupling. Ship position = base + accum >> 8.
void InsaneRebel1::updateShipPhysics() {
	_frameCounter++;

	// Decrement cooldown
	if (_damageCooldown > 0)
		_damageCooldown--;

	// --- Step 1: Mouse input as offset from screen center ---
	// Original: _DAT_756C (horizontal), _DAT_756E (vertical)
	int16 inputX = (int16)(_vm->_mouse.x - 160);
	int16 inputY = (int16)(_vm->_mouse.y - 100);
	inputX = CLIP<int16>(inputX, -127, 127);
	inputY = CLIP<int16>(inputY, -127, 127);

	// --- Step 2: Roll accumulator (_74CA) ---
	// Normal mode: accumulate; mode 0x10: snap to input
	_rollAccum += (_tuning.roll * (int32)inputX) >> 5;
	_rollAccum = CLIP<int32>(_rollAccum, -0x47F, 0x47F);

	// --- Step 3: Vertical smoothing (_74CE) ---
	// Exponential decay toward -inputY
	_liftSmooth += (-_liftSmooth - (int32)inputY) >> 1;
	_liftSmooth = CLIP<int32>(_liftSmooth, -0x20, 0x20);

	// --- Step 4: Position accumulator deltas ---
	// X delta: drift + slide coupling - cross-coupling
	int32 rng = _turbulenceEnabled ? (int32)_vm->_rnd.getRandomNumber(199) : 100;  // 0-199, centered at 100
	int32 crossTermX;
	if (_liftSmooth < 0)
		crossTermX = ((int32)_tuning.lift * _liftSmooth * _rollAccum) >> 11;
	else
		crossTermX = ((int32)_tuning.lift * _liftSmooth * _rollAccum) >> 12;

	int32 deltaX = (((rng - 100) - (int32)_tuning.drift * _driftParam) >> 1)
	             + (((int32)_tuning.slide * _rollAccum) >> 7)
	             - crossTermX;

	// Y delta: roll magnitude + lift cross-coupling
	int32 absRoll = ABS(_rollAccum);
	int32 crossTermY;
	if (_liftSmooth < 0)
		crossTermY = ((int32)_tuning.lift * (0x7DE - absRoll) * _liftSmooth) >> 12;
	else
		crossTermY = ((int32)_tuning.lift * (0x7DE - absRoll) * _liftSmooth) >> 13;

	int32 deltaY = (absRoll >> 1) + crossTermY;

	// --- Step 5: Update position accumulators ---
	_posAccumX += deltaX;
	_posAccumX = CLIP<int32>(_posAccumX, -0x8200, 0x8200);
	_posAccumY += deltaY;
	_posAccumY = CLIP<int32>(_posAccumY, -0x3200, 0x4600);

	// --- Step 6: Derive pixel position from accumulators ---
	// Original: _74BA = _74C2 >> 8, _74BC = _74C6 >> 8
	// Ship position = base + offset
	_shipPosX = kCenterX + (int16)(_posAccumX >> 8);
	_shipPosY = kCenterY + (int16)(_posAccumY >> 8);

	// Clamp to screen bounds
	_shipPosX = CLIP<int16>(_shipPosX, kMinX, kMaxX);
	_shipPosY = CLIP<int16>(_shipPosY, kMinY, kMaxY);

	// --- Step 7: Corridor collision (FUN_1C54D) ---
	// Wall contact forces position accumulators to corridor edge and sets
	// damage flags. Flag bit 0x10 (zone hit) suppresses damage bits only.
	{
		bool hasZoneHit = (_damageFlags & 0x10) != 0;

		if (_shipPosX > _corridorRightX) {
			_posAccumX = (_corridorRightX - kCenterX) << 8;
			_shipPosX = _corridorRightX;
			if (!hasZoneHit) {
				if (_rollAccum > -0x100)
					_rollAccum = -0x100;  // Push left
				_damageFlags |= 0x02;  // Right wall
			}
		}
		if (_shipPosX < _corridorLeftX) {
			_posAccumX = (_corridorLeftX - kCenterX) << 8;
			_shipPosX = _corridorLeftX;
			if (!hasZoneHit) {
				if (_rollAccum < 0x100)
					_rollAccum = 0x100;   // Push right
				_damageFlags |= 0x04;  // Left wall
			}
		}
		if (_shipPosY < _corridorTopY) {
			_posAccumY = ((_corridorTopY - kCenterY) << 8) + 0x100;
			_shipPosY = _corridorTopY;
			if (!hasZoneHit)
				_damageFlags |= 0x01;
		}
		if (_shipPosY > _corridorBottomY) {
			_posAccumY = ((_corridorBottomY - kCenterY) << 8) - 0x100;
			_shipPosY = _corridorBottomY;
			if (!hasZoneHit)
				_damageFlags |= 0x08;
		}
	}

	// --- Step 8: Perspective offsets ---
	{
		int absOffX = ABS(_shipPosX - kCenterX);
		if (absOffX > 0)
			_perspectiveX = (int16)((kFocalX * kCenterX * absOffX) /
				((kCenterX - kFocalX) * absOffX + kFocalX * kCenterX));
		else
			_perspectiveX = 0;
		if (_shipPosX < kCenterX + 1)
			_perspectiveX = -_perspectiveX;

		int absOffY = ABS(_shipPosY - kCenterY);
		if (absOffY > 0)
			_perspectiveY = (int16)((kFocalY * kCenterY * absOffY) /
				((kCenterY - kFocalY) * absOffY + kFocalY * kCenterY));
		else
			_perspectiveY = 0;
		if (_shipPosY < kCenterY + 1)
			_perspectiveY = -_perspectiveY;
	}

	// --- Step 9: Direction sprite index (FUN_1DEB5 LAB_1e23e) ---
	// Horizontal component from _74CA (rollAccum):
	//   |rollAccum| <= 0x80: center (0)
	//   rollAccum > 0x80:  ((rollAccum - 0x80) >> 8) * 5 + 5   (right: 5,10,15,20)
	//   rollAccum < -0x80: ((abs(rollAccum) - 0x80) >> 8) * 5 + 25 (left: 25,30,35,40)
	int hComponent;
	if (_rollAccum > 0x80) {
		hComponent = ((_rollAccum - 0x80) >> 8) * 5 + 5;
	} else if (_rollAccum < -0x80) {
		hComponent = ((-_rollAccum - 0x80) >> 8) * 5 + 25;
	} else {
		hComponent = 0;
	}

	// Vertical component from _74CE (liftSmooth):
	//   (_74CE + 0x20) * 5 / 0x41  → 0..4  (5 rows)
	int vComponent = (_liftSmooth + 0x20) * 5 / 0x41;

	_shipDirIndex = CLIP<int16>((int16)(vComponent + hComponent), 0, _shipBank.numSprites - 1);

	// --- Step 10: Damage/event bit synthesis + damage processing ---
	// RA1 FUN_1B297-style latches from GAME opcodes:
	//   0x5D latch 0xFFFF -> bit 0x40 (obstacle/contact)
	//   0x5F non-zero + RNG -> bit 0x80 (projectile-like hit)
	if (_gameLatch5D == 0xFFFF)
		_damageFlags |= 0x40;
	if (_gameLatch5F != 0 && _vm->_rnd.getRandomNumber((uint16)(_gameLatch5F - 1)) == 0)
		_damageFlags |= 0x80;

	// Damage guard/mask from FUN_1DEB5: (_damageFlags & 0x96) != 0
	// damageFlags & 0x96 = bits 1,2,4,7 = wall collisions (0x16) + projectile hit (0x80)
	if ((_damageFlags & 0x96) != 0 && _damageCooldown == 0 &&
		_health >= 0 && _deathTimer <= 0) {
		// Projectile hit (bit 7 = 0x80)
		if (_damageFlags & 0x80)
			_health -= _tuning.shot;
		// Wall collision (bits 1,2,4 = 0x16)
		if (_damageFlags & 0x16)
			_health -= _tuning.wham;

		if (_health < 0)
			_deathTimer = kDeathTimerInit;

		_prevDamageFlags = _damageFlags;
		_damageCooldown = kDamageCooldownInit;
		_screenFlash = 3;
	}

	// Latches are per-frame event inputs in the original pipeline.
	_gameLatch5D = 0;
	_gameLatch5F = 0;

	// Death animation countdown
	if (_health < 0 && _deathTimer > 0)
		_deathTimer--;

	// Health regeneration: +1 every 32 frames (from original asm)
	if ((_frameCounter & 0x1F) == 0) {
		if (_health >= 0 && _health < kMaxHealth)
			_health++;
		if (_health >= 0)
			_score += 1;
	}

	// Screen flash decay
	if (_screenFlash > 0)
		_screenFlash--;

	// Clear per-frame damage flags
	_damageFlags = 0;

	// --- Path branching detection ---
	// Original (FUN_1B297): at GAME counter 394 (0x18A), sets nextSceneA=0x67/nextSceneB=0x69.
	// After this point, drift goes strongly negative (pushing ship left for the hard path).
	// If ship is right of center, player chose the right/easy path → switch to L1PLAY1R.
	// The check fires once when the game counter first reaches the branch point.
	if (_pathBranchEnabled && !_rightPathSelected && _gameCounter >= kPathBranchCounter) {
		if (_shipPosX > kCenterX) {
			_rightPathSelected = true;
			_vm->_smushVideoShouldFinish = true;
			debug(1, "RA1: Right path selected (counter=%d, shipX=%d)", _gameCounter, _shipPosX);
		}
	}

	debug(7, "RA1 ship: pos=(%d,%d) roll=%d lift=%d accX=%d accY=%d dir=%d health=%d corridor=[%d,%d]-[%d,%d]",
		_shipPosX, _shipPosY, _rollAccum, _liftSmooth,
		_posAccumX, _posAccumY, _shipDirIndex, _health,
		_corridorLeftX, _corridorTopY, _corridorRightX, _corridorBottomY);
}

void InsaneRebel1::renderShip(byte *dst, int pitch, int width, int height) {
	// From FUN_1DEB5 LAB_1e2b2: ship drawn when health >= 0 OR deathTimer > 20
	// Hidden during last 20 frames of death sequence (deathTimer 20→0)
	if (_health < 0 && _deathTimer <= 20)
		return;

	if (_shipDirIndex < 0 || _shipDirIndex >= _shipBank.numSprites)
		return;

	const RA1Sprite &spr = _shipBank.sprites[_shipDirIndex];

	// Position: game coords → screen coords via perspective transform
	// Adapted from RA2's renderHandler7Ship:
	//   shipCenterX = (shipX - center) + perspX + screenCenterX
	int drawX = (_shipPosX - kCenterX) + _perspectiveX + kCenterX - spr.width / 2;
	int drawY = (_shipPosY - kCenterY) + _perspectiveY + kCenterY - spr.height / 2;

	renderSprite(dst, pitch, width, height, drawX, drawY, spr);
}

// Render explosion sprites during damage cooldown and death sequence.
// From FUN_1DEB5 at LAB_1e185 (damage hit) and LAB_1e0e3 (death shake).
void InsaneRebel1::renderExplosions(byte *dst, int pitch, int width, int height) {
	if (_bangBank.numSprites <= 0)
		return;

	// Ship screen center position (matches assembly: DAT_74b6+DAT_74ba, DAT_74b8+DAT_74bc)
	int shipScreenX = (_shipPosX - kCenterX) + _perspectiveX + kCenterX;
	int shipScreenY = (_shipPosY - kCenterY) + _perspectiveY + kCenterY;

	// --- Death shake explosions (FUN_1DEB5 LAB_1e0e3) ---
	// When dead and deathTimer > 10: random explosion sprites scatter around ship
	if (_health < 0 && _deathTimer > 10) {
		int intensity = _deathTimer - 10;  // 20→1 as timer goes 30→11
		if (intensity > 10)
			intensity = 20 - intensity;     // Triangle: 0→10→0

		// di = intensity * 4 + 1 (vertical scatter range)
		// si = -20 + intensity * 4 (horizontal scatter range, DAT_75d8 is 0)
		int rangeY = intensity * 4 + 1;
		int rangeX = -20 + intensity * 4;
		if (rangeX < 1) rangeX = 1;

		for (int i = 0; i < intensity; i++) {
			// Random sprite from bang bank (FUN_21db0(10))
			int sprIdx = _vm->_rnd.getRandomNumber(_bangBank.numSprites - 1);

			// Random position around ship (matching assembly random scatter)
			int randX = (int)_vm->_rnd.getRandomNumber(rangeX * 2) - rangeX;
			int randY = (int)_vm->_rnd.getRandomNumber(rangeY * 2) - rangeY;

			int drawX = shipScreenX + randX;
			int drawY = shipScreenY + randY;

			const RA1Sprite &spr = _bangBank.sprites[sprIdx];
			renderSprite(dst, pitch, width, height,
				drawX - spr.width / 2, drawY - spr.height / 2, spr);
		}
		return;
	}

	// --- Damage hit explosion (FUN_1DEB5 LAB_1e185) ---
	// When alive, in cooldown, and bang bank loaded
	if (_health >= 0 && _damageCooldown > 0) {
		// Sprite index = 10 - damageCooldown (frames 0→9 as cooldown 10→1)
		int sprIdx = _bangBank.numSprites - _damageCooldown;
		if (sprIdx < 0 || sprIdx >= _bangBank.numSprites)
			return;

		// Position at ship center (DAT_75d8 is always 0 in RA1)
		int drawX = shipScreenX;
		int drawY = shipScreenY;

		const RA1Sprite &spr = _bangBank.sprites[sprIdx];
		renderSprite(dst, pitch, width, height,
			drawX - spr.width / 2, drawY - spr.height / 2, spr);
	}
}

// Render bottom status bar from DISPLAY.NUT with dynamic damage bar and score.
// Original layout (320-wide): DAMAGE [green bar] | PILOTS [3 icons] | SCORE [number]
void InsaneRebel1::renderHUD(byte *dst, int pitch, int width, int height) {
	if (_displayBank.numSprites == 0)
		return;

	const RA1Sprite &bar = _displayBank.sprites[0];

	// DISPLAY.NUT sprite is 320×19 at xoffs=0, yoffs=176 in the original game.
	// Video FOBJs fill the full 384×242 buffer from (0,0), so use sprite offsets directly.
	int hudX = bar.xoffs;
	int hudY = bar.yoffs;

	// Draw the status bar background with transparency (pixel 0 = transparent)
	if (bar.data && bar.width > 0 && bar.height > 0) {
		int drawX = hudX, drawY = hudY, drawW = bar.width, drawH = bar.height;
		int srcOffX = 0, srcOffY = 0;
		if (drawX < 0) { srcOffX = -drawX; drawW += drawX; drawX = 0; }
		if (drawY < 0) { srcOffY = -drawY; drawH += drawY; drawY = 0; }
		if (drawX + drawW > width) drawW = width - drawX;
		if (drawY + drawH > height) drawH = height - drawY;

		for (int iy = 0; iy < drawH; iy++) {
			const byte *s = bar.data + (srcOffY + iy) * bar.width + srcOffX;
			byte *d = dst + (drawY + iy) * pitch + drawX;
			for (int ix = 0; ix < drawW; ix++) {
				byte px = s[ix];
				if (px != 0)
					d[ix] = px;
			}
		}

		debug(5, "RA1 HUD: drawn at (%d,%d) size=%dx%d",
			hudX, hudY, bar.width, bar.height);
	}

	// Draw health bar from FUN_1BBCB behavior.
	// Original logic uses current health as fill width and computes x as (0x92 - health),
	// so the bar is right-anchored and shrinks from left to right as damage increases.
	{
		int barMaxW = kMaxHealth;
		int barH = 5;
		int healthWidth = CLIP<int16>(_health, 0, kMaxHealth);
		int barX = hudX + (0x92 - healthWidth);
		int barY = hudY + 8;
		int fillW = CLIP(healthWidth, 0, barMaxW);

		// Color based on damage level (matching original thresholds from FUN_1BBCB)
		byte barColor;
		if (_health > _tuning.shot * 2)
			barColor = 0xA0;  // Green — low damage
		else if (_health > _tuning.wham * 2)
			barColor = 0x2C;  // Yellow — moderate damage
		else
			barColor = 0x30;  // Red — critical

		// Flash effect on damage
		if (_screenFlash > 0)
			barColor = 0xFF;  // White flash

		for (int iy = 0; iy < barH && barY + iy < height; iy++) {
			byte *d = dst + (barY + iy) * pitch + barX;
			for (int ix = 0; ix < fillW && barX + ix < width; ix++) {
				d[ix] = barColor;
			}
		}
	}

	// Draw lives and score from DISPLAY.NUT glyphs.
	if (_hudFontBank.numSprites > 0) {
		char livesStr[8];
		Common::sprintf_s(livesStr, "%d", MAX<int>(_lives, 0));
		drawFontBankString(dst, pitch, width, height, hudX + 180, hudY + 6, livesStr);

		char scoreStr[16];
		Common::sprintf_s(scoreStr, "%07d", MAX<int>(_score, 0));
		drawFontBankString(dst, pitch, width, height, hudX + 257, hudY + 4, scoreStr);
	}

}

void InsaneRebel1::renderSprite(byte *dst, int pitch, int width, int height,
								int x, int y, const RA1Sprite &spr) {
	if (!spr.data || spr.width <= 0 || spr.height <= 0)
		return;

	int drawX = x, drawY = y, drawW = spr.width, drawH = spr.height;
	int srcOffsetX = 0, srcOffsetY = 0;

	if (drawX < 0) { srcOffsetX = -drawX; drawW += drawX; drawX = 0; }
	if (drawY < 0) { srcOffsetY = -drawY; drawH += drawY; drawY = 0; }
	if (drawX + drawW > width) drawW = width - drawX;
	if (drawY + drawH > height) drawH = height - drawY;
	if (drawW <= 0 || drawH <= 0)
		return;

	for (int iy = 0; iy < drawH; iy++) {
		const byte *s = spr.data + (srcOffsetY + iy) * spr.width + srcOffsetX;
		byte *d = dst + (drawY + iy) * pitch + drawX;
		for (int ix = 0; ix < drawW; ix++) {
			byte px = s[ix];
			if (px != 0)
				d[ix] = px;
		}
	}
}

void InsaneRebel1::procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
	int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
	int16 par1, int16 par2, int16 par3, int16 par4) {
}

void InsaneRebel1::procSKIP(int32 subSize, Common::SeekableReadStream &b) {
}

// Parse RA1 GAME chunks.
void InsaneRebel1::handleGameChunk(int32 subSize, Common::SeekableReadStream &b) {
	if (subSize < 8)
		return;

	uint32 opcode = b.readUint32BE();
	uint32 param1 = b.readUint32BE();

	switch (opcode) {
	case 0x5E:
		// RA1 dispatcher inline reset/init path (FUN_1BE1B case 0x5E).
		// This is not a pure control-mode assignment.
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_gameLatch5D = 0;
		_gameLatch5F = 0;
		_driftParam = 0;
		_rollAccum = 0;
		_liftSmooth = 0;
		_posAccumX = 0;
		_posAccumY = 0;

		// Field1 == 0 corresponds to baseline recenter behavior in the original.
		if ((int32)param1 == 0) {
			_shipPosX = kCenterX;
			_shipPosY = kCenterY;
		}

		// Keep a conservative default mode after reset.
		_flyControlMode = 0;
		debug(5, "RA1 GAME 0x5E: reset state field1=%d", (int32)param1);
		break;

	case 0x5D:
		_gameLatch5D = (uint16)param1;
		debug(5, "RA1 GAME 0x5D (link/event latch) param=%u", _gameLatch5D);
		break;

	case 0x5F:
		_gameLatch5F = (uint16)param1;
		debug(5, "RA1 GAME 0x5F (random-hit latch) param=%u", _gameLatch5F);
		break;

	case 0x07:
		// Per-frame corridor data: f1=frame counter, f2=max frames, f3=drift bias, f4=unused
		// f1 is the original's _DAT_7740 (game frame counter)
		// f3 is the drift/wind parameter combined with tuning table
		_gameCounter = param1;
		if (subSize >= 20) {
			b.readUint32BE(); // f2 (max frames, unused in physics)
			_driftParam = (int16)(int32)b.readUint32BE();
			b.readUint32BE(); // f4 (unused in original assembly)
			debug(7, "RA1 GAME 0x07: counter=%d driftParam=%d", _gameCounter, _driftParam);
		}
		break;

	case 0x0D:
		// Corridor boundaries: per-frame flight corridor
		// Original params: left, top, WIDTH, HEIGHT (not right/bottom!)
		// FUN_1C54D computes center = (left+width/2, top+height/2), transforms, then checks edges.
		if (subSize >= 20) {
			_corridorLeftX = (int16)param1;
			_corridorTopY = (int16)b.readUint32BE();
			int16 corridorWidth = (int16)b.readUint32BE();
			int16 corridorHeight = (int16)b.readUint32BE();
			_corridorRightX = _corridorLeftX + corridorWidth;
			_corridorBottomY = _corridorTopY + corridorHeight;
			debug(5, "RA1 GAME 0x0D: corridor left=%d top=%d right=%d bottom=%d (w=%d h=%d)",
				_corridorLeftX, _corridorTopY, _corridorRightX, _corridorBottomY,
				corridorWidth, corridorHeight);
		}
		break;

	case 0x0E:
		// Secondary collision zone (FUN_1C6E9): AABB test, sets damageFlags bit 4 (0x10)
		// Original params: left, top, WIDTH, HEIGHT (same as 0x0D)
		if (subSize >= 20) {
			int16 zoneLeft = (int16)param1;
			int16 zoneTop = (int16)b.readUint32BE();
			int16 zoneWidth = (int16)b.readUint32BE();
			int16 zoneHeight = (int16)b.readUint32BE();
			int16 zoneRight = zoneLeft + zoneWidth;
			int16 zoneBottom = zoneTop + zoneHeight;
			if (_shipPosX > zoneLeft && _shipPosX < zoneRight &&
				_shipPosY > zoneTop && _shipPosY < zoneBottom) {
				_damageFlags |= 0x10;
			}
			debug(7, "RA1 GAME 0x0E: zone=[%d,%d]-[%d,%d] flags=0x%02x",
				zoneLeft, zoneTop, zoneRight, zoneBottom, _damageFlags);
		}
		break;

	case 0x08: case 0x09: case 0x0A: case 0x0B:
	case 0x19: case 0x1A:
		if (subSize >= 20) {
			uint32 param2 = b.readUint32BE();
			uint32 param3 = b.readUint32BE();
			uint32 param4 = b.readUint32BE();
			debug(7, "RA1 GAME 0x%02x: params=(%d,%d,%d,%d)", opcode, param1, param2, param3, param4);
		}
		break;

	default:
		debug(7, "RA1 GAME unknown 0x%02x size=%d", opcode, subSize);
		break;
	}
}

// ---------------------------------------------------------------------------
// Game flow (matching original at 0x15597)
// ---------------------------------------------------------------------------

// Play a passive cinematic (no game callback, skippable).
// Reuses RA2's pattern: reset handler, set cinematic flags, play video.
// startFrame > 0: fast-forward (decode without display/audio) to that frame.
void InsaneRebel1::playCinematic(const char *filename, int32 startFrame) {
	debug(1, "InsaneRebel1::playCinematic('%s', startFrame=%d)", filename, startFrame);
	SmushPlayer *splayer = _vm->_splayer;
	_player = splayer;
	_interactiveVideoActive = false;
	_vm->_smushVideoShouldFinish = false;
	splayer->setCurVideoFlags(0x28);  // Cinematic mode + buffer preserve
	if (startFrame > 0)
		splayer->setFastForwardToFrame(startFrame);
	splayer->play(filename, 12);
}

void InsaneRebel1::clearVideoBuffer() {
	if (_vm->_screenWidth <= 0 || _vm->_screenHeight <= 0)
		return;

	const int pixelCount = _vm->_screenWidth * _vm->_screenHeight;
	byte *clearBuffer = (byte *)calloc(pixelCount, 1);
	if (!clearBuffer)
		return;

	if (_vm->_macScreen) {
		_vm->mac_drawBufferToScreen(clearBuffer, _vm->_screenWidth, 0, 0, _vm->_screenWidth, _vm->_screenHeight);
	} else {
		_vm->_system->copyRectToScreen(clearBuffer, _vm->_screenWidth, 0, 0, _vm->_screenWidth, _vm->_screenHeight);
	}
	_vm->_system->updateScreen();

	free(clearBuffer);
}

// Intro sequence (0x155ef-0x158f8):
//   1. O1LOGO.ANM — LucasArts logo
//   2. O1OPEN.ANM — Star Wars opening crawl
void InsaneRebel1::playIntroSequence() {
	debug(1, "InsaneRebel1: Playing intro sequence");

	// LucasArts logo (original: PUSH 0x57cc, CALL FUN_1BA32 with flags 0x0420)
	playCinematic("OPEN/O1LOGO.ANM");
	if (_vm->shouldQuit())
		return;
	clearVideoBuffer();

	// Star Wars opening crawl (original: PUSH 0x5800, CALL FUN_1BA32)
	playCinematic("OPEN/O1OPEN.ANM");
}

// Main menu on O1OPTION.ANM background (0x15968).
// Original renders text overlay with 5 menu items via FUN_21F7A.
// For now, we play the menu video as a passive cinematic (non-interactive)
// and return "Start New Game" immediately.
// TODO: Implement interactive menu with keyboard/mouse selection.
int InsaneRebel1::runMainMenu() {
	debug(1, "InsaneRebel1: Main menu");

	_menuSelection = 0;
	while (!_vm->shouldQuit()) {
		_menuActive = true;
		_menuConfirmed = false;
		_menuFrameCounter = 0;
		clearVideoBuffer();
		playCinematic("OPEN/O1OPTION.ANM");
		_menuActive = false;

		if (_vm->shouldQuit())
			return 5;

		if (_menuConfirmed)
			return _menuSelection + 1;
	}

	return 5;
}

void InsaneRebel1::runOptionsMenu() {
	_optionsSel = 0;
	_optionsActive = true;

	while (!_vm->shouldQuit()) {
		_menuActive = true;
		_menuConfirmed = false;
		_menuFrameCounter = 0;
		clearVideoBuffer();
		playCinematic("OPEN/O1OPTION.ANM");
		_menuActive = false;

		if (_vm->shouldQuit())
			break;

		if (_menuConfirmed) {
			switch (_optionsSel) {
			case 0:
				// Cycle difficulty
				_difficulty = (_difficulty + 1) % 3;
				loadTuningForLevel(0);
				break;
			case 1:
				// Toggle turbulence
				_turbulenceEnabled = !_turbulenceEnabled;
				break;
			case 2:
				// Back to main menu
				_optionsActive = false;
				return;
			}
		}
	}
	_optionsActive = false;
}

// Level 1 flow (0x16100-0x167A2, from disassembly):
//   1. Load NUTs (L1BANK1, L1BANK2, L1EXPLD, L1BANG, L1LASER)
//   2. L1HANGAR.ANM — Full hangar departure cutscene (782 frames, flags 0x0420)
//   3. L1CU1.ANM — Pre-flight cutscene (flags 0x0400)
//   4. L1PLAY1L.ANM — Stage 1 flight, hard/left path (788 frames)
//      At frame 394, if player steers right → L1PLAY1R (easy path, 396 frames)
//   5. L1CU2.ANM — Mid-level cutscene
//   6. L1PLAY2.ANM — Stage 2 turret
//      If score < 5 (0x75D0): L1RETRY → retry Stage 2
//   7. L1END.ANM — Level complete
//   Death (health<0): L1CRASHA/B → lives check:
//     lives>0: L1NEW → jump back to Stage 1 (skip L1HANGAR/L1CU1)
//     lives==0: L1DEATH → return to menu

bool InsaneRebel1::runLevel1() {
	debug(1, "InsaneRebel1: Running level 1");

	// Load level sprites (original: pushes L1BANK1..L1BANG NUT filenames)
	loadLevelSprites(1);

	// L1HANGAR.ANM — Hangar departure (original: 0x5918, flags 0x0420)
	// Plays once at level start, never replayed on retry.
	playCinematic("LVL1/L1HANGAR.ANM");
	if (_vm->shouldQuit())
		return false;

	// L1CU1.ANM — Pre-flight cutscene (original: 0x5944, flags 0x0400)
	// Plays once at level start, never replayed on retry.
	playCinematic("LVL1/L1CU1.ANM");
	if (_vm->shouldQuit())
		return false;

	// Retry loop — on death with lives, L1NEW plays then jumps back here
	while (!_vm->shouldQuit()) {
		// Reset health for this attempt (original: MOV WORD [0x7560], 98 at 0x16214)
		_health = kMaxHealth;
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_frameCounter = 0;
		_gameCounter = 0;
		_pathBranchEnabled = true;
		_rightPathSelected = false;

		// Stage 1 flight — L1PLAY1L (hard/left path)
		// The first 394 frames are the common section. At counter 394, if
		// ship is right of center, we switch to L1PLAY1R (easy path).
		playInteractiveVideo("LVL1/L1PLAY1L.ANM");
		if (_vm->shouldQuit())
			return false;

		if (_rightPathSelected && _health >= 0) {
			debug(1, "InsaneRebel1: Switching to right path (L1PLAY1R)");
			_pathBranchEnabled = false;
			playInteractiveVideo("LVL1/L1PLAY1R.ANM");
			if (_vm->shouldQuit())
				return false;
		}
		_pathBranchEnabled = false;

		if (_health >= 0) {
			// L1CU2.ANM — Mid-level cutscene (original: 0x5977)
			playCinematic("LVL1/L1CU2.ANM");
			if (_vm->shouldQuit())
				return false;

			// L1PLAY2.ANM — Stage 2 turret (original: 0x5986)
			playInteractiveVideo("LVL1/L1PLAY2.ANM");
			if (_vm->shouldQuit())
				return false;

			// TODO: Check score threshold (original: CMP WORD [0x75D0], 5)
			// If score < 5: L1RETRY → retry Stage 2

			// L1END.ANM — Level complete! (original: 0x59a3)
			playCinematic("LVL1/L1END.ANM");
			return true;
		}

		// Death sequence (original: 0x165dd-0x166bb)
		// Random crash variant A or B
		if (_vm->_rnd.getRandomNumber(1) == 0)
			playCinematic("LVL1/L1CRASHA.ANM");
		else
			playCinematic("LVL1/L1CRASHB.ANM");
		if (_vm->shouldQuit())
			return false;

		// Check lives (original: CMP WORD [0x7562], 0 at 0x1666B)
		_lives--;
		if (_lives <= 0) {
			// Game over — L1DEATH then return (original: 0x166C0)
			playCinematic("LVL1/L1DEATH.ANM");
			debug(1, "InsaneRebel1: Game over (no lives left)");
			return false;
		}

		// Lives remaining — L1NEW briefing then retry (original: 0x16675)
		playCinematic("LVL1/L1NEW.ANM");
		if (_vm->shouldQuit())
			return false;

		// Loop back to gameplay (original: JMP 0x16214 — health reset + Stage 1)
	}

	return false;
}

// Main game entry point — called from ScummEngine::go().
// Matches original flow at 0x15597: intro → menu → level.
void InsaneRebel1::runGame() {
	// Play intro sequence (logo + opening)
	playIntroSequence();
	if (_vm->shouldQuit())
		return;

	// Main menu → gameplay loop
	while (!_vm->shouldQuit()) {
		int menuResult = runMainMenu();
		if (_vm->shouldQuit())
			return;

		switch (menuResult) {
		case 1: {
			// Start New Game — play L1NEW briefing then level 1
			playCinematic("LVL1/L1NEW.ANM");
			if (_vm->shouldQuit())
				return;

			bool completed = runLevel1();
			if (completed) {
				debug(1, "InsaneRebel1: Level 1 completed!");
				// TODO: Continue to level 2
			}
			// Return to menu after level ends
			break;
		}
		case 2:
			// Game Options
			runOptionsMenu();
			break;
		case 5:
			// Exit
			return;
		default:
			// Passcode, Demo — not yet implemented, return to menu
			break;
		}
	}
}

// Play interactive gameplay video (with ship physics + HUD).
void InsaneRebel1::playInteractiveVideo(const char *filename) {
	debug(1, "InsaneRebel1::playInteractiveVideo('%s')", filename);

	// Stop any leftover audio from previous video
	terminateAudio();
	initAudio(_audioSampleRate);

	SmushPlayer *splayer = _vm->_splayer;
	_player = splayer;
	clearBit(0);
	_interactiveVideoActive = true;
	_vm->_smushVideoShouldFinish = false;
	splayer->setCurVideoFlags(0x28);

	// Center mouse, hide cursor, and lock mouse to window (like RA2 flight)
	smush_warpMouse(160, 100, -1);
	CursorMan.showMouse(false);
	g_system->lockMouse(true);

	splayer->play(filename, 12);
	_interactiveVideoActive = false;

	g_system->lockMouse(false);
}

} // End of namespace Scumm
