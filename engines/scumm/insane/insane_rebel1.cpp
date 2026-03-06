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
// RA2 center: (212, 130), RA1 center: (192, 121)
// RA2 bounds: [20, 404]x[20, 240], RA1 bounds: [18, 366]x[18, 224]
static const int16 kCenterX = 192;
static const int16 kCenterY = 121;
static const int16 kMinX = 18;
static const int16 kMaxX = 366;
static const int16 kMinY = 18;
static const int16 kMaxY = 224;

// Perspective focal lengths (scaled from RA2: focalX=0x2b, focalY=0x19)
static const int16 kFocalX = 39;   // 0x2b * 384/424 ≈ 39
static const int16 kFocalY = 23;   // 0x19 * 242/260 ≈ 23

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

	_smoothedVelocity = 0;
	_verticalInput = 0;
	memset(_velocityHistory, 0, sizeof(_velocityHistory));
	_driftParam = 0;
	_driftAccum = 0;

	_perspectiveX = 0;
	_perspectiveY = 0;
	_viewShift = 0;

	_flyControlMode = 0;

	_health = kMaxHealth;
	_lives = 3;
	_score = 0;
	_damageFlags = 0;
	_gameLatch5D = 0;
	_gameLatch5F = 0;
	_damageCooldown = 0;
	_deathTimer = 0;
	_screenFlash = 0;
	_frameCounter = 0;
	_interactiveVideoActive = false;
	_menuActive = false;
	_menuConfirmed = false;
	_menuSelection = 0;
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
	if (_menuActive && event.type == Common::EVENT_KEYDOWN) {
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

void InsaneRebel1::renderMainMenuOverlay(byte *dst, int pitch, int width, int height) {
	static const char *kMenuItems[5] = {
		"START NEW GAME",
		"GAME OPTIONS",
		"ENTER PASSCODE",
		"CONTINUE DEMO",
		"EXIT"
	};

	const int menuX = 92;
	const int menuY = 60;
	const int rowH = 16;
	const int boxW = 190;

	for (int i = 0; i < 5; i++) {
		const int y = menuY + i * rowH;
		if (i == _menuSelection) {
			for (int yy = 0; yy < 12; yy++) {
				const int sy = y + yy;
				if (sy < 0 || sy >= height)
					continue;
				for (int xx = 0; xx < boxW; xx++) {
					const int sx = menuX + xx;
					if (sx < 0 || sx >= width)
						continue;
					if (xx < 2 || yy < 2 || xx >= boxW - 2 || yy >= 10)
						dst[sy * pitch + sx] = 0xFF;
				}
			}
		}

		drawFontBankString(dst, pitch, width, height, menuX + 10, y + 1, kMenuItems[i]);
	}

	drawFontBankString(dst, pitch, width, height, 118, 36, "MAIN MENU");
}

// Velocity-based ship physics adapted from RA2 Handler 7 (FUN_40C3CC case 4).
// Mouse input → velocity history averaging → position delta → corridor collision → perspective.
void InsaneRebel1::updateShipPhysics() {
	_frameCounter++;

	// Decrement cooldown
	if (_damageCooldown > 0)
		_damageCooldown--;

	// --- Step 1: Mouse input as offset from screen center ---
	// Use _vm->_mouse (0-319, 0-199 virtual screen coords), same as RA2.
	// Center = (160, 100) in virtual screen space.
	int16 inputX = (int16)(_vm->_mouse.x - 160);
	int16 inputY = (int16)(_vm->_mouse.y - 100);

	// Clamp: [-160, 160] horizontal, [-127, 127] vertical (same as RA2)
	inputX = CLIP<int16>(inputX, -160, 160);
	inputY = CLIP<int16>(inputY, -127, 127);

	// --- Step 2: Scale to [-127, 127] (same as RA2: scaledInputX = inputX * 127 / 160) ---
	int16 scaledInputX = (int16)((inputX * 127) / 160);
	int16 scaledInputY = inputY;

	// --- Step 3: Velocity history + smoothed average ---
	for (int i = 24; i > 0; i--)
		_velocityHistory[i] = _velocityHistory[i - 1];
	_velocityHistory[0] = scaledInputX;

	const int smoothWindow = 5;
	int velSum = 0;
	for (int i = 0; i < smoothWindow; i++)
		velSum += _velocityHistory[i];
	_smoothedVelocity = (int16)(velSum / smoothWindow);

	// --- Step 4: Position delta ---
	const int16 levelSpeed = 32;
	const int16 levelYSpeed = 48;
	int16 absSmoothVel = ABS(_smoothedVelocity);
	int16 positionDeltaX;

	if (_flyControlMode == 1) {
		// Mode 1: Full cross-axis coupling
		if (scaledInputX < 1)
			positionDeltaX = (int16)((levelSpeed * _smoothedVelocity - absSmoothVel * scaledInputY) >> 9);
		else
			positionDeltaX = (int16)((levelSpeed * _smoothedVelocity + absSmoothVel * scaledInputY) >> 9);
	} else {
		// Mode 0/2/3: Reduced cross-axis coupling
		if (scaledInputX < 1)
			positionDeltaX = (int16)((levelSpeed * _smoothedVelocity - (absSmoothVel * scaledInputY >> 2)) >> 9);
		else
			positionDeltaX = (int16)((levelSpeed * _smoothedVelocity + (absSmoothVel * scaledInputY >> 2)) >> 9);
	}

	// Original asm drift pipeline (0x1e3fc-0x1e5cb):
	//   xDelta = (random(200) - 100 - driftTuning * _driftParam) / 2
	//   accumX += xDelta  (32-bit accumulator)
	//   shipDeltaX = accumX >> 8  (divide by 256 for pixel position)
	// So drift contributes ~±0.2 px/frame. We approximate by accumulating and shifting.
	const int16 kDriftTuning = 3; // TODO: load from per-difficulty/level tuning table
	int16 driftBias = (int16)(_vm->_rnd.getRandomNumber(199) - 100 - kDriftTuning * _driftParam);
	_driftAccum += driftBias >> 1;
	_driftAccum = CLIP<int32>(_driftAccum, -0x8200, 0x8200);
	positionDeltaX += (int16)(_driftAccum >> 8);

	// Clamp X delta to ±12 per frame
	positionDeltaX = CLIP<int16>(positionDeltaX, -12, 12);
	_shipPosX += positionDeltaX;

	// Y delta (no drift in original assembly — field4 is unused)
	int16 positionDeltaY;
	if (_flyControlMode == 1) {
		positionDeltaY = (int16)((levelYSpeed * scaledInputY) >> 10);
		positionDeltaY = CLIP<int16>(positionDeltaY, -12, 12);
	} else {
		positionDeltaY = (int16)((levelYSpeed * scaledInputY) >> 10);
	}
	_shipPosY -= positionDeltaY;

	_verticalInput = scaledInputY;

	// --- Step 6: Position clamping ---
	_shipPosX = CLIP<int16>(_shipPosX, kMinX, kMaxX);
	_shipPosY = CLIP<int16>(_shipPosY, kMinY, kMaxY);

	// --- Step 7: Corridor collision (modes 0 and 2) ---
	// From FUN_1C54D: wall contact sets damageFlags bits 0-3 (directional)
	// and pushes velocity history to bounce away from the wall.
	if (_flyControlMode == 0 || _flyControlMode == 2) {
		if (_corridorRightX < _shipPosX) {
			_shipPosX = _corridorRightX;
			_damageFlags |= 0x02;  // Right wall
			if (_damageCooldown < 5) {
				for (int i = 0; i < 25; i++)
					_velocityHistory[i] = -127;
			}
		}
		if (_shipPosX < _corridorLeftX) {
			_shipPosX = _corridorLeftX;
			_damageFlags |= 0x04;  // Left wall
			if (_damageCooldown < 5) {
				for (int i = 0; i < 25; i++)
					_velocityHistory[i] = 127;
			}
		}
		if (_corridorBottomY < _shipPosY) {
			_shipPosY = _corridorBottomY;
			_damageFlags |= 0x01;  // Bottom wall
		}
		if (_shipPosY < _corridorTopY) {
			_shipPosY = _corridorTopY;
			_damageFlags |= 0x08;  // Top wall
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

	_viewShift = CLIP<int16>(_smoothedVelocity, -127, 127);

	// --- Step 9: Direction sprite (5x7 grid with hysteresis) ---
	// vDir from vertical input: (0xa0 - verticalInput) >> 6
	int16 vDir = (int16)(((int)(0xa0 - _verticalInput) + ((0xa0 - _verticalInput) < 0 ? 63 : 0)) >> 6);
	vDir = CLIP<int16>(vDir, 0, 4);

	// hDir from smoothed velocity: (0x95 - smoothedVelocity) / 0x2b
	int16 hDir = (int16)((0x95 - _smoothedVelocity) / 0x2b);
	hDir = CLIP<int16>(hDir, 0, 6);

	// Hysteresis at center positions
	if (hDir == 3 && ABS(_smoothedVelocity) > 10)
		hDir = (_smoothedVelocity < 1) ? 4 : 2;
	if (vDir == 2 && ABS(_verticalInput) > 15)
		vDir = (_verticalInput < 1) ? 3 : 1;

	_shipDirIndex = CLIP<int16>(vDir * 7 + hDir, 0, _shipBank.numSprites - 1);

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
			_health -= kHeavyDamage;
		// Wall collision (bits 1,2,4 = 0x16)
		if (_damageFlags & 0x16)
			_health -= kLightDamage;

		if (_health < 0)
			_deathTimer = kDeathTimerInit;

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

	debug(7, "RA1 ship: pos=(%d,%d) vel=%d vIn=%d dx=%d dir=%d health=%d corridor=[%d,%d]-[%d,%d]",
		_shipPosX, _shipPosY, _smoothedVelocity, _verticalInput,
		positionDeltaX, _shipDirIndex, _health,
		_corridorLeftX, _corridorTopY, _corridorRightX, _corridorBottomY);
}

void InsaneRebel1::renderShip(byte *dst, int pitch, int width, int height) {
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
		if (_health > kHeavyDamage * 2)
			barColor = 0xA0;  // Green — low damage
		else if (_health > kLightDamage * 2)
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
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_gameLatch5D = 0;
		_gameLatch5F = 0;
		_driftParam = 0;
		_driftAccum = 0;
		_smoothedVelocity = 0;
		_verticalInput = 0;
		memset(_velocityHistory, 0, sizeof(_velocityHistory));

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
		// Per-frame corridor data: f1=frame index, f2=constant(788), f3=drift bias, f4=unused
		// Original asm: drift bias * tuning "drift" param, combined with random turbulence
		// f4 is never referenced in the original handler function
		if (subSize >= 20) {
			b.readUint32BE(); // f2 (constant 788, unused in physics)
			_driftParam = (int16)(int32)b.readUint32BE();
			b.readUint32BE(); // f4 (unused in original assembly)
			debug(7, "RA1 GAME 0x07: frame=%d driftParam=%d", param1, _driftParam);
		}
		break;

	case 0x0D:
		// Corridor boundaries: per-frame flight corridor
		// Raw: 0x0D, left, top, right, bottom (all 32-bit BE)
		if (subSize >= 20) {
			_corridorLeftX = (int16)param1;
			_corridorTopY = (int16)b.readUint32BE();
			_corridorRightX = (int16)b.readUint32BE();
			_corridorBottomY = (int16)b.readUint32BE();
			debug(5, "RA1 GAME 0x0D: corridor left=%d top=%d right=%d bottom=%d",
				_corridorLeftX, _corridorTopY, _corridorRightX, _corridorBottomY);
		}
		break;

	case 0x0E:
		// Secondary collision zone (FUN_1C6E9): AABB test, sets damageFlags bit 4 (0x10)
		if (subSize >= 20) {
			int16 zoneLeft = (int16)param1;
			int16 zoneTop = (int16)b.readUint32BE();
			int16 zoneRight = (int16)b.readUint32BE();
			int16 zoneBottom = (int16)b.readUint32BE();
			if (_shipPosX >= zoneLeft && _shipPosX <= zoneRight &&
				_shipPosY >= zoneTop && _shipPosY <= zoneBottom) {
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
void InsaneRebel1::playCinematic(const char *filename) {
	debug(1, "InsaneRebel1::playCinematic('%s')", filename);
	SmushPlayer *splayer = _vm->_splayer;
	_player = splayer;
	_interactiveVideoActive = false;
	_vm->_smushVideoShouldFinish = false;
	splayer->setCurVideoFlags(0x28);  // Cinematic mode + buffer preserve
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
		playCinematic("OPEN/O1OPTION.ANM");
		_menuActive = false;

		if (_vm->shouldQuit())
			return 5;

		if (_menuConfirmed)
			return _menuSelection + 1;
	}

	return 5;
}

// Level 1 flow (0x16100-0x16737):
//   1. Load NUTs (L1BANK1, L1BANK2, L1EXPLD, L1BANG, L1LASER)
//   2. L1HANGAR.ANM — Hangar departure cutscene
//   3. "Chapter 1" text
//   4. L1CU1.ANM — Pre-flight cutscene
//   5. L1PLAY1L.ANM — Stage 1 gameplay (left path) — INTERACTIVE
//   6. L1PLAY1R.ANM — Stage 1 gameplay (right path) — INTERACTIVE
//   7. L1CU2.ANM — Mid-level cutscene
//   8. L1PLAY2.ANM — Stage 2 turret — INTERACTIVE
//   9. L1END.ANM — Level end cutscene
//   Death: L1CRASHA/B.ANM → L1DEATH.ANM → L1RETRY.ANM → retry from L1NEW
bool InsaneRebel1::runLevel1() {
	debug(1, "InsaneRebel1: Running level 1");

	// Load level sprites (original: pushes L1BANK1..L1BANG NUT filenames)
	loadLevelSprites(1);

	// L1HANGAR.ANM — Hangar departure intro (original: 0x5918, flags 0x0420)
	playCinematic("LVL1/L1HANGAR.ANM");
	if (_vm->shouldQuit())
		return false;

	// L1CU1.ANM — Pre-flight cutscene (original: 0x5944, flags 0x0400)
	playCinematic("LVL1/L1CU1.ANM");
	if (_vm->shouldQuit())
		return false;

	// Retry loop
	while (!_vm->shouldQuit()) {
		// Reset health for this attempt
		_health = kMaxHealth;
		_damageFlags = 0;
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_frameCounter = 0;

		// L1PLAY1L.ANM — Stage 1 gameplay (left path, original: 0x5953)
		playInteractiveVideo("LVL1/L1PLAY1L.ANM");
		if (_vm->shouldQuit())
			return false;

		if (_health >= 0) {
			// Survived stage 1 — continue to right path
			// L1PLAY1R.ANM (original: 0x5965, flags with seekframe=0x187)
			playInteractiveVideo("LVL1/L1PLAY1R.ANM");
			if (_vm->shouldQuit())
				return false;

			if (_health >= 0) {
				// L1CU2.ANM — Mid-level cutscene (original: 0x5977)
				playCinematic("LVL1/L1CU2.ANM");
				if (_vm->shouldQuit())
					return false;

				// L1PLAY2.ANM — Stage 2 turret (original: 0x5986)
				playInteractiveVideo("LVL1/L1PLAY2.ANM");
				if (_vm->shouldQuit())
					return false;

				// L1END.ANM — Level complete! (original: 0x59a3)
				playCinematic("LVL1/L1END.ANM");
				return true;
			}
		}

		// Death sequence (original: 0x165e8-0x16737)
		// Random crash variant A or B
		if (_vm->_rnd.getRandomNumber(1) == 0)
			playCinematic("LVL1/L1CRASHA.ANM");
		else
			playCinematic("LVL1/L1CRASHB.ANM");
		if (_vm->shouldQuit())
			return false;

		// L1DEATH.ANM (original: 0x5a4b)
		playCinematic("LVL1/L1DEATH.ANM");
		if (_vm->shouldQuit())
			return false;

		_lives--;
		if (_lives <= 0) {
			// Game over — no more retries
			debug(1, "InsaneRebel1: Game over (no lives left)");
			return false;
		}

		// L1RETRY.ANM — Retry prompt (original: 0x5a5c)
		// After retry, original jumps back to L1NEW→L1PLAY1L (0x16214→0x16680)
		playCinematic("LVL1/L1RETRY.ANM");
		if (_vm->shouldQuit())
			return false;

		// L1NEW.ANM — Briefing before retry (original: 0x5a3c at retry path 0x16680)
		playCinematic("LVL1/L1NEW.ANM");
		if (_vm->shouldQuit())
			return false;

		// Loop back to gameplay
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
		case 5:
			// Exit
			return;
		default:
			// Options, Passcode, Demo — not yet implemented, return to menu
			break;
		}
	}
}

// Play interactive gameplay video (with ship physics + HUD).
void InsaneRebel1::playInteractiveVideo(const char *filename) {
	debug(1, "InsaneRebel1::playInteractiveVideo('%s')", filename);

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
