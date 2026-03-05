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
	_damageCooldown = 0;
	_deathTimer = 0;
	_screenFlash = 0;
	_frameCounter = 0;

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
}

InsaneRebel1::~InsaneRebel1() {
	terminateAudio();
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

	bank.numSprites = READ_LE_UINT16(data + 10);
	bank.sprites = new RA1Sprite[bank.numSprites];

	// Pass 1: Walk chunks with alignment to compute total decoded size.
	uint32 offset = 0;
	uint32 decodedSize = 0;
	for (int i = 0; i < bank.numSprites; i++) {
		// Skip current chunk (AHDR or previous FRME)
		uint32 chunkSize = READ_BE_UINT32(data + offset + 4);
		offset += chunkSize + 8;
		if (chunkSize & 1) offset++;  // Word-align

		// Now at FRME; skip its header to reach FOBJ
		offset += 8;
		if (offset + 22 > animSize) break;

		uint16 w = READ_LE_UINT16(data + offset + 14);
		uint16 h = READ_LE_UINT16(data + offset + 16);
		decodedSize += w * h;
	}

	bank.decodedData = (byte *)calloc(decodedSize, 1);
	byte *decPtr = bank.decodedData;

	// Pass 2: Decode sprites.
	offset = 0;
	for (int i = 0; i < bank.numSprites; i++) {
		uint32 chunkSize = READ_BE_UINT32(data + offset + 4);
		offset += chunkSize + 8;
		if (chunkSize & 1) offset++;

		offset += 8;  // Skip FRME header → now at FOBJ
		if (offset + 22 > animSize) break;

		int codec = READ_LE_UINT16(data + offset + 8);
		bank.sprites[i].xoffs = READ_LE_INT16(data + offset + 10);
		bank.sprites[i].yoffs = READ_LE_INT16(data + offset + 12);
		bank.sprites[i].width = READ_LE_UINT16(data + offset + 14);
		bank.sprites[i].height = READ_LE_UINT16(data + offset + 16);
		bank.sprites[i].data = decPtr;

		int pixelCount = bank.sprites[i].width * bank.sprites[i].height;
		const byte *fobjData = data + offset + 22;

		if (codec == 21) {
			decodeBomp(decPtr, fobjData, bank.sprites[i].width,
					   bank.sprites[i].height, bank.sprites[i].width);
		} else {
			warning("InsaneRebel1::loadRA1Nut: unsupported codec %d in sprite %d", codec, i);
		}

		decPtr += pixelCount;
	}

	free(data);
	debug(1, "InsaneRebel1::loadRA1Nut('%s'): %d sprites, %d bytes decoded",
		  filename, bank.numSprites, decodedSize);
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

	if (_shipBank.numSprites == 0 || !renderBitmap)
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

	// Dead zone: ignore small offsets from center to prevent drift
	const int16 kDeadZone = 8;
	if (ABS(inputX) < kDeadZone) inputX = 0;
	if (ABS(inputY) < kDeadZone) inputY = 0;

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

	// --- Step 10: Damage processing (from FUN_1DEB5 decompilation) ---
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

	// Death animation countdown
	if (_health < 0 && _deathTimer > 0)
		_deathTimer--;

	// Health regeneration: +1 every 32 frames (from original asm)
	if (_health >= 0 && _health < kMaxHealth && (_frameCounter & 0x1F) == 0)
		_health++;

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

	// Draw health bar (from FUN_1BBCB decompilation).
	// Bar starts at x=56, y=8 within HUD, max width ~76px, height ~5px.
	// Color thresholds: green (>50%), yellow (25-50%), red (<25%).
	{
		int barMaxW = 76;
		int barH = 5;
		int barX = hudX + 56;
		int barY = hudY + 8;
		int damage = kMaxHealth - CLIP<int16>(_health, 0, kMaxHealth);
		int fillW = barMaxW * damage / kMaxHealth;
		fillW = CLIP(fillW, 0, barMaxW);

		// Color based on damage level (matching original thresholds from FUN_1BBCB)
		byte barColor;
		if (_health > kMaxHealth / 2)
			barColor = 0xA0;  // Green — low damage
		else if (_health > kMaxHealth / 4)
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

	// Draw score as decimal digits.
	// From screenshot: score area starts around x=265 within HUD, using palette text color.
	// For now, just draw simple 4×7 digit bitmaps.
	{
		char scoreStr[8];
		snprintf(scoreStr, sizeof(scoreStr), "%06d", _score);
		int digitX = hudX + 265;
		int digitY = hudY + 7;
		byte textColor = 0xFF;  // White

		for (int c = 0; c < 6 && scoreStr[c]; c++) {
			int digit = scoreStr[c] - '0';
			// Simple 3×5 digit rendering
			static const uint16 digitPatterns[10] = {
				0x7B6F, // 0: 111 011 011 011 111
				0x2492, // 1: 010 010 010 010 010
				0x73E7, // 2: 111 001 111 100 111
				0x73CF, // 3: 111 001 111 001 111
				0x5BC9, // 4: 101 101 111 001 001
				0x7E3F, // 5: 111 110 111 001 111
				0x7E7F, // 6: 111 110 111 101 111
				0x7249, // 7: 111 001 001 001 001
				0x7FFF, // 8: 111 111 111 111 111  (simplified)
				0x7FCF, // 9: 111 111 111 001 111
			};
			if (digit < 0 || digit > 9) digit = 0;
			uint16 pat = digitPatterns[digit];
			for (int py = 0; py < 5; py++) {
				for (int px = 0; px < 3; px++) {
					int bit = 14 - (py * 3 + px);
					if (pat & (1 << bit)) {
						int sx = digitX + c * 5 + px;
						int sy = digitY + py;
						if (sx >= 0 && sx < width && sy >= 0 && sy < height)
							dst[sy * pitch + sx] = textColor;
					}
				}
			}
		}
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
		// Mode control
		_flyControlMode = (int16)param1;
		debug(5, "RA1 GAME 0x5E: flyControlMode=%d", _flyControlMode);
		break;

	case 0x5D:
		debug(5, "RA1 GAME 0x5D (link) param=%d", param1);
		break;

	case 0x5F:
		debug(5, "RA1 GAME 0x5F (event) param=%d", param1);
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

void InsaneRebel1::playLevel(int level) {
	loadLevelSprites(level);

	Common::String filename = Common::String::format("LVL%d/L%dPLAY1L.ANM", level, level);
	debug(1, "InsaneRebel1::playLevel(%d): playing %s", level, filename.c_str());

	SmushPlayer *splayer = _vm->_splayer;
	_player = splayer;

	// Center mouse, hide cursor, and lock mouse to window (like RA2 flight)
	smush_warpMouse(160, 100, -1);
	CursorMan.showMouse(false);
	g_system->lockMouse(true);

	splayer->play(filename.c_str(), 12);

	g_system->lockMouse(false);
}

} // End of namespace Scumm
