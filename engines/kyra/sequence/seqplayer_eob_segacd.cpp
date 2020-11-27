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

#ifdef ENABLE_EOB

#include "kyra/engine/eob.h"
#include "kyra/graphics/screen_eob.h"
#include "kyra/graphics/screen_eob_segacd.h"
#include "kyra/resource/resource.h"
#include "kyra/resource/resource_segacd.h"
#include "kyra/sequence/seqplayer_eob_segacd.h"
#include "common/system.h"

namespace Kyra {

SegaSequencePlayer::SegaSequencePlayer(EoBEngine *vm, Screen_EoB *screen, SegaCDResource *res) : _vm(vm), _screen(screen), _res(res), _tileSets(0), _debugResyncCnt(0), _speechAnimType(0),
	_playingID(1), _waterdeepScene(0), _playSpeechAnimation(0), _frameTimer(0), _waterdeepSceneTimer(0), _speechAnimTimer(0), _speechAnimNo(0), _speechAnimFrame(0),
	_newTrack(-1), _pauseStart(0), _fastForward(false), _renderer(_screen->sega_getRenderer()), _animator(_screen->sega_getAnimator()) {
#define SQOPC(x) _opcodes.push_back(new SQOpcode(this, &SegaSequencePlayer::x, #x))
	SQOPC(s_initDrawObject);
	SQOPC(s_drawTileSet);
	SQOPC(s_loadTileDataSingle);
	SQOPC(s_drawTileSetCustom);
	SQOPC(s_drawTileSetCustomTopToBottom);
	SQOPC(s_fillRect);
	SQOPC(s_void);
	SQOPC(s_initSprite);
	SQOPC(s_removeSprite);
	SQOPC(s_displayTextJp);
	SQOPC(s_fadeToNeutral);
	SQOPC(s_fadeToBlack);
	SQOPC(s_fadeToNeutral2);
	SQOPC(s_fadeToWhite);
	SQOPC(s_setPalette);
	SQOPC(s_vScroll);
	SQOPC(s_hScroll);
	SQOPC(s_paletteOps);
	SQOPC(s_initSpriteCustomCoords);
	SQOPC(s_fillRectWithPattern);
	SQOPC(s_loadTileDataSeries);
	SQOPC(s_drawTileSetSeries);
	SQOPC(s_initSpriteSeries);
	SQOPC(s_initSpriteCustom);
	SQOPC(s_drawTileSetCustomCoords);
	SQOPC(s_waitForPaletteFade);
	SQOPC(s_clearSprites);
	SQOPC(s_moveSprites2);
	SQOPC(s_moveSprites);
	SQOPC(s_moveMorphSprite);
	SQOPC(s_unpauseCD);
	SQOPC(s_toggleWaterDeepAnimations);
	SQOPC(s_assignSpeechAnimGraphics);
	SQOPC(s_toggleSpeechAnimation);
	SQOPC(s_orbZoomEffect);
	SQOPC(s_stopCD);
	SQOPC(s_playCD);
	SQOPC(s_displayTextEn);
	SQOPC(s_loadCustomPalettes);
	SQOPC(s_playSoundEffect);
#undef SQOPC

	_scrollManager = new ScrollManager(_renderer);
	assert(_scrollManager);
	_tileSets = new TileSet[100];
	assert(_tileSets);
	memset(_tileSets, 0, 100 * sizeof(TileSet));
	_drawObjects = new DrawObject[100];
	assert(_drawObjects);
	memset(_drawObjects, 0, 100 * sizeof(DrawObject));

	memset(_speechAnimDrawOps, 0, sizeof(_speechAnimDrawOps));

	_scaleSrcBuffer = new uint8[0x5800];
	assert(_scaleSrcBuffer);
	memset(_scaleSrcBuffer, 0, 0x5800 * sizeof(uint8));
	_scaleOutBuffer = new uint8[0x5800];
	assert(_scaleOutBuffer);
	memset(_scaleOutBuffer, 0, 0x5800 * sizeof(uint8));
	_scaleStampMap = new uint16[0x100];
	assert(_scaleStampMap);
	memset(_scaleStampMap, 0, 0x100 * sizeof(uint16));
	_scaleTraceVectors = new uint16[0x580];
	assert(_scaleTraceVectors);
	memset(_scaleTraceVectors, 0, 0x580 * sizeof(uint16));

	int temp;
	_wdDsX = _vm->staticres()->loadRawDataBe16(kEoB1IntroWdDsX, temp);
	_wdDsY = _vm->staticres()->loadRawData(kEoB1IntroWdDsY, temp);
	_wdAnimSprites = _vm->staticres()->loadRawData(kEoB1WdAnimSprites, temp);
	_speechAnimData = _vm->staticres()->loadRawData(kEoB1SpeechAnimData, temp);
	_cdaTracks = _vm->staticres()->loadRawData(kEoB1SequenceTrackMap, temp);
	for (int i = 0; i < 6; ++i)
		_patternTables[i] = _vm->staticres()->loadRawDataBe16(kEoB1PatternTable0 + i, temp);
}

SegaSequencePlayer::~SegaSequencePlayer() {
	delete[] _drawObjects;
	delete[] _tileSets;
	delete[] _scaleSrcBuffer;
	delete[] _scaleOutBuffer;
	delete[] _scaleStampMap;
	delete[] _scaleTraceVectors;

	for (Common::Array<SQOpcode*>::iterator i = _opcodes.begin(); i != _opcodes.end(); ++i)
		delete (*i);

	delete _scrollManager;
}

bool SegaSequencePlayer::play(int id) {
	_renderer->render(0);
	_screen->sega_fadeToBlack(2);
	_screen->clearPage(0);

	_animator->clearSprites();
	_scrollManager->setVScrollTimers(0, 1, 0, 0, 1, 0);
	_scrollManager->setHScrollTimers(0, 1, 0, 0, 1, 0);
	_vm->_txt->clearDim(2);

	_renderer->fillRectWithTiles(2, 0, 0, 40, 28, 0xE6C2);
	_renderer->fillRectWithTiles(0, 0, 0, 64, 42, 0);
	_renderer->fillRectWithTiles(1, 0, 0, 64, 28, 0);
	_renderer->fillRectWithTiles(2, 1, (id == 53 || id == 54) ? 22 : 20, 38, 6, 0xE51C, true);

	_debugResyncCnt = 0;
	_playingID = id;
	_newTrack = -1;

	_vm->_allowSkip = true;
	_vm->resetSkipFlag();

	for (bool runLoop = true; runLoop && !(_vm->shouldQuit() || _vm->skipFlag()); ) {
		uint32 offset = (id - 1) * 0x3C000;
		if (id >= 54)
			offset -= 0x8000;
		if (id >= 55)
			offset -= 0x18000;

		uint32 size = (id == 53) ? 0x34000 : ((id == 54) ? 0x24000 : 0x3C000);

		if (!_res->loadContainer("VISUAL", offset, size))
			return false;

		Common::SeekableReadStreamEndian *in = _res->resStreamEndian(0);
		if (!in)
			return false;
		_screen->sega_loadCustomPaletteData(in);
		delete in;

		_screen->sega_selectPalette(31, 0, false);
		_screen->sega_selectPalette(32, 1, false);
		_screen->sega_selectPalette(30, 3, false);

		in = _res->resStreamEndian(2);
		if (!in)
			return false;
		uint32 len = in->size();
		uint8 *tileData = new uint8[len];
		in->read(tileData, len);
		delete in;

		in = _res->resStreamEndian(1);
		if (!in)
			return false;
		memset(_tileSets, 0, 100 * sizeof(TileSet));
		for (TileSet *t = _tileSets; !in->eos(); ++t) {
			uint32 off = in->readUint32();
			if ((off & 0xFFFF) == 0xFFFF)
				break;
			t->data = (const uint16*)(tileData + off);
			t->width = in->readUint16();
			t->height = in->readUint16();
		}
		delete in;

		in = _res->resStreamEndian(3);
		if (!in)
			return false;
		len = in->size();
		uint8 *frames = new uint8[len];
		in->read(frames, len);
		delete in;

		if (id == 53)
			_vm->delay(2000);
		else if (id == 55 || id == 56)
			_vm->snd_playSong(2);

		run(frames);

		delete[] frames;
		delete[] tileData;
		if (++id != 54)
			runLoop = false;
	}

	debugC(3, kDebugLevelSequence, "Total millis out of sync: %d", _debugResyncCnt);

	if (_vm->shouldQuit() || _vm->skipFlag()) {
		if (!(_playingID == 55 || _playingID == 56))
			_vm->snd_stopSound();
	}

	_vm->_allowSkip = false;
	_vm->resetSkipFlag();

	_playingID = 1;
	return true;
}

void SegaSequencePlayer::pause(bool pause) {
	if (pause)
		_pauseStart = _vm->_system->getMillis();
	else
		_frameTimer += (_vm->_system->getMillis() - _pauseStart);
}

void SegaSequencePlayer::run(const uint8 *data) {
	_waterdeepScene = _playSpeechAnimation = false;
	_frameTimer = _vm->_system->getMillis();
	_fastForward = false;

	for (bool runLoop = true; runLoop; ) {
		// In case of a skip or shouldQuit event we fastforward through the sequence instead of just aborting. This way
		// we ensure that we have the same palettes and scroll offsets as if the sequence had been played normally.
		_fastForward = _vm->shouldQuit() || _vm->skipFlag();
		uint16 frameSize = READ_BE_UINT16(data);
		if (!frameSize)
			return;

		uint32 frameStart = _vm->_system->getMillis();
		uint16 timeStamp = READ_BE_UINT16(data + 2);
		uint32 nextFrame = _frameTimer + (timeStamp * 16667) / 1000;
		bool insertDelay = false;

		if (_vm->_system->getMillis() >= nextFrame || _fastForward) {
			debugC(5, kDebugLevelSequence, "SeqPlayer: Timestamp %08d", timeStamp);
			for (uint16 timeStamp2 = timeStamp; timeStamp2 == timeStamp; ) {
				uint16 op = READ_BE_UINT16(data + 4);
				_opcodes[op]->run(data + 6);
				_screen->clearPage(0);

				frameSize = READ_BE_UINT16(data);
				data += (frameSize & ~1);

				timeStamp2 = READ_BE_UINT16(data + 2);
			}
		} else {
			insertDelay = true;
		}

		if (_waterdeepScene)
			animateWaterdeepScene();

		if (_playSpeechAnimation)
			updateSpeechAnimations();

		_animator->update();

		if (_fastForward) {
			_scrollManager->fastForward();
		} else {
			_scrollManager->updateScrollTimers();
			_renderer->render(0);
			_screen->sega_updatePaletteFaders(-1);
			_screen->updateScreen();
		}

		if (insertDelay) {
			int diff = _vm->_system->getMillis() - (frameStart + 16);
			if (diff < 0)
				_vm->delay((uint32)-diff);
		}
	}
}

void SegaSequencePlayer::animateWaterdeepScene() {
	if (--_waterdeepSceneTimer > 0)
		return;
	_waterdeepSceneTimer = 5;

	for (int i = 0; i < 5; ++i) {
		int rnd = _vm->_rnd.getRandomNumber(30);
		DrawObject *d = &_drawObjects[10 + rnd];
		_animator->initSprite(_wdAnimSprites[rnd] + 3, _wdDsX[_wdAnimSprites[rnd]] - 80, _wdDsY[_wdAnimSprites[rnd]] + 32, d->nTblVal, d->addr);
	}
}

void SegaSequencePlayer::updateSpeechAnimations() {
	if (--_speechAnimTimer > 0)
		return;

	int animDrawOp = -1;

	for (bool runLoop = true; runLoop; ) {
		if (_speechAnimTimer == 0) {
			const uint8 *pos = &_speechAnimData[_speechAnimNo * 12 + _speechAnimFrame];
			_speechAnimTimer = pos[1];
			if (pos[0] != 0xFF) {
				animDrawOp = pos[0];
				runLoop = false;
			}
			_speechAnimFrame += 2;
		} else {
			_speechAnimTimer = 0;
		}

		if (animDrawOp == -1) {
			_speechAnimNo = (_speechAnimType == 2) ? _vm->_rnd.getRandomNumberRng(4, 6) : _vm->_rnd.getRandomNumberRng(0, 3);
			_speechAnimFrame = 0;
		}
	}

	updateSpeechAnimGraphics(animDrawOp);
}

void SegaSequencePlayer::updateSpeechAnimGraphics(int animDrawOp) {
	assert(animDrawOp < 6);
	DrawObject *d = &_drawObjects[_speechAnimDrawOps[animDrawOp * 2]];
	if (_speechAnimDrawOps[animDrawOp * 2 + 1])
		_renderer->loadToVRAM(d->tileData, (d->width * d->height) << 5, (d->nTblVal & 0x7FF) << 5);
	else
		_renderer->fillRectWithTiles(d->addr, d->x, d->y, d->width, d->height, d->nTblVal, true);

}

#define ARG(x) READ_BE_UINT16(pos + x)
#define S_ARG(x) (int16)ARG(x)

void SegaSequencePlayer::s_initDrawObject(const uint8 *pos) {
	int idx = ARG(0);
	DrawObject *w = &_drawObjects[idx];
	TileSet *t = &_tileSets[idx];
	w->agg = ARG(2);
	w->tileData = t->data;
	w->width = t->width;
	w->height = t->height;
	w->nTblVal = ARG(4) == 0xFFFF ? _drawObjects[idx - 1].width * _drawObjects[idx - 1].height + _drawObjects[idx - 1].nTblVal : (ARG(4) == 0xFFFE ? _drawObjects[idx - 1].nTblVal : ARG(4));
	w->x = ARG(6);
	w->y = ARG(8);
	w->addr = ARG(10);
}

void SegaSequencePlayer::s_drawTileSet(const uint8 *pos) {
	DrawObject *w = &_drawObjects[ARG(0)];
	_renderer->fillRectWithTiles(w->addr, w->x, w->y, w->width, w->height, w->nTblVal, true);
}

void SegaSequencePlayer::s_loadTileDataSingle(const uint8 *pos) {
	DrawObject *w = &_drawObjects[ARG(0)];
	_renderer->loadToVRAM(w->tileData, (w->width * w-> height) << 5, (w->nTblVal & 0x7FF) << 5);
}

void SegaSequencePlayer::s_drawTileSetCustom(const uint8 *pos) {
	_renderer->fillRectWithTiles(ARG(8), ARG(0), ARG(2), ARG(4), ARG(6), ARG(10), true);
}

void SegaSequencePlayer::s_drawTileSetCustomTopToBottom(const uint8 *pos) {
	_renderer->fillRectWithTiles(ARG(8), ARG(0), ARG(2), ARG(4), ARG(6), ARG(10), true, true);
}

void SegaSequencePlayer::s_fillRect(const uint8 *pos) {
	_renderer->fillRectWithTiles(ARG(8), ARG(0), ARG(2), ARG(4), ARG(6), ARG(10));
}

void SegaSequencePlayer::s_initSprite(const uint8 *pos) {
	DrawObject *d = &_drawObjects[ARG(2)];
	_animator->initSprite(ARG(0), d->x << 3, d->y << 3, d->nTblVal, d->addr);
}

void SegaSequencePlayer::s_removeSprite(const uint8 *pos) {
	_animator->initSprite(ARG(0), 0x4000, 0, 0, 0);
}

void SegaSequencePlayer::s_displayTextJp(const uint8 *pos) {
	if (_vm->gameFlags().lang != Common::JA_JPN)
		return;

	const char *str = (const char*)pos;
	_vm->_txt->clearDim(2);

	int w = _screen->getTextWidth(str);
	int x = 0;
	int y = 0;

	if (w < 288) {
		x = 152 - (w >> 1);
		y = 16;
	}

	_vm->_txt->printShadedText(str, x, y, -1, 0xEE);
}

void SegaSequencePlayer::s_fadeToNeutral(const uint8 *pos) {
	_screen->sega_fadeToNeutral(ARG(0));
}

void SegaSequencePlayer::s_fadeToBlack(const uint8 *pos) {
	_screen->sega_fadeToBlack(ARG(0));
}

void SegaSequencePlayer::s_fadeToNeutral2(const uint8 *pos) {
	_screen->sega_fadeToNeutral(ARG(0));
}

void SegaSequencePlayer::s_fadeToWhite(const uint8 *pos) {
	_screen->sega_fadeToWhite(ARG(0));
}

void SegaSequencePlayer::s_setPalette(const uint8 *pos) {
	_screen->sega_selectPalette(ARG(2) + 31, ARG(0), false);
}

void SegaSequencePlayer::s_vScroll(const uint8 *pos) {
	_scrollManager->setVScrollTimers(ARG(0), S_ARG(2), ARG(4), ARG(6), S_ARG(8), ARG(10));
}

void SegaSequencePlayer::s_hScroll(const uint8 *pos) {
	_scrollManager->setHScrollTimers(ARG(0), S_ARG(2), ARG(4), ARG(6), S_ARG(8), ARG(10));
}

void SegaSequencePlayer::s_paletteOps(const uint8 *pos) {
	_screen->sega_paletteOps(S_ARG(0), S_ARG(2), S_ARG(4));
}

void SegaSequencePlayer::s_initSpriteCustomCoords(const uint8 *pos) {
	DrawObject *d = &_drawObjects[ARG(2)];
	_animator->initSprite(ARG(0), S_ARG(4), S_ARG(6), d->nTblVal, d->addr);
}

void SegaSequencePlayer::s_fillRectWithPattern(const uint8 *pos) {
	assert(ARG(12) < 6);
	_renderer->fillRectWithTiles(ARG(8), ARG(0), ARG(2), ARG(4), ARG(6), ARG(10), false, false, _patternTables[ARG(12)]);
}

void SegaSequencePlayer::s_loadTileDataSeries(const uint8 *pos) {
	for (DrawObject *d = &_drawObjects[ARG(0)]; d != &_drawObjects[ARG(0) + ARG(2)]; ++d)
		_renderer->loadToVRAM(d->tileData, (d->width * d->height) << 5, (d->nTblVal & 0x7FF) << 5);
}

void SegaSequencePlayer::s_drawTileSetSeries(const uint8 *pos) {
	for (DrawObject *d = &_drawObjects[ARG(0)]; d != &_drawObjects[ARG(0) + ARG(2)]; ++d)
		_renderer->fillRectWithTiles(d->addr, d->x, d->y, d->width, d->height, d->nTblVal, true);
}

void SegaSequencePlayer::s_initSpriteSeries(const uint8 *pos) {
	int id = ARG(0);
	for (DrawObject *d = &_drawObjects[ARG(2)]; d != &_drawObjects[ARG(2) + ARG(4)]; ++d)
		_animator->initSprite(id++, d->x << 3, d->y << 3, d->nTblVal, d->addr);
}

void SegaSequencePlayer::s_initSpriteCustom(const uint8 *pos) {
	_animator->initSprite(ARG(0), S_ARG(4), S_ARG(6), ARG(2), ARG(8));
}

void SegaSequencePlayer::s_drawTileSetCustomCoords(const uint8 *pos) {
	DrawObject *w = &_drawObjects[ARG(0)];
	_renderer->fillRectWithTiles(w->addr, ARG(2), ARG(4), w->width, w->height, w->nTblVal, true);
}

void SegaSequencePlayer::s_waitForPaletteFade(const uint8*) {
	_screen->sega_fadePalette(0, 0, -1, true, true);
}

void SegaSequencePlayer::s_clearSprites(const uint8*) {
	_animator->clearSprites();
}

void SegaSequencePlayer::s_moveSprites2(const uint8 *pos) {
	_animator->moveSprites2(ARG(0), ARG(2), S_ARG(4), S_ARG(6));
}

void SegaSequencePlayer::s_moveSprites(const uint8 *pos) {
	_animator->moveSprites(ARG(0), ARG(2), S_ARG(4), S_ARG(6));
}

void SegaSequencePlayer::s_moveMorphSprite(const uint8 *pos) {
	_animator->moveMorphSprite(ARG(0), ARG(2), S_ARG(4), S_ARG(6));
}

void SegaSequencePlayer::s_unpauseCD(const uint8 *pos) {
	// We don't support this in our AudioCD API. The original will use s_playCD() to seek to a track,
	// wait for the seek to finish and then pause. It then uses this opcode to actually start the playback.
	// Since s_playCD() and s_unpauseCD() are not always called on the same frame I emulate the original
	// behavior like this.
	if (_newTrack != -1)
		_vm->snd_playSong(_newTrack, false);
	_newTrack = -1;
}

void SegaSequencePlayer::s_toggleWaterDeepAnimations(const uint8 *pos) {
	_waterdeepScene = ARG(0);
	_waterdeepSceneTimer = 0;
}

void SegaSequencePlayer::s_assignSpeechAnimGraphics(const uint8 *pos) {
	if (ARG(0) == 100) {
		_speechAnimType = ARG(2);
	} else {
		assert(ARG(0) < 6);
		_speechAnimDrawOps[ARG(0) * 2] = ARG(2);
		_speechAnimDrawOps[ARG(0) * 2 + 1] = ARG(4);
	}
}

void SegaSequencePlayer::s_toggleSpeechAnimation(const uint8 *pos) {
	_playSpeechAnimation = ARG(0);
	_speechAnimTimer = 0;
	if (_playSpeechAnimation)
		updateSpeechAnimGraphics(0);
}

void SegaSequencePlayer::s_orbZoomEffect(const uint8*) {
	_renderer->memsetVRAM(0x2AA0, 0, 0x5800);
	DrawObject *d = &_drawObjects[16];
	memset(_scaleSrcBuffer, 0, 0x5800);
	memcpy(_scaleSrcBuffer + 128, d->tileData, (d->width * d->height) << 5);
	_renderer->fillRectWithTiles(0, 4, 0, 32, 22, 0x2155, true, true);

	memset(_scaleStampMap, 0, 0x100 * sizeof(uint16));
	uint16 *dst2 = &_scaleStampMap[(7 << 4) + 6];
	uint16 t = 1;
	for (int h = 0; h < 9; ++h) {
		uint16 *dst = dst2;
		for (int w = 0; w < 10; ++w)
			*dst++ = t++;
		dst2 += 16;
	}

	int step = 512;
	for (int i = 0; i < 90; ++i) {
		uint32 nextFrame = _vm->_system->getMillis() + 64;
		uint16 *dst = _scaleTraceVectors;
		uint32 xtr = 0x58000 - step * 128;
		uint32 ytr = 0x59000 - step * 88;
		for (int ii = 0; ii < 176; ++ii) {
			*dst++ = xtr >> 8;
			*dst++ = ytr >> 8;
			*dst++ = step;
			*dst++ = 0;
			ytr += step;
		}
		memset(_scaleOutBuffer, 0, 0x5800);
		_screen->sega_gfxScale(_scaleOutBuffer, 256, 176, 21, _scaleSrcBuffer, _scaleStampMap, _scaleTraceVectors);
		_renderer->loadToVRAM(_scaleOutBuffer, 0x5800, 0x2AA0);

		if (!_fastForward) {
			_renderer->render(0);
			_screen->updateScreen();
			_vm->delayUntil(nextFrame);
		}

		step += 16;
	}
}

void SegaSequencePlayer::s_stopCD(const uint8*) {
	_vm->snd_stopSound();
}

void SegaSequencePlayer::s_playCD(const uint8 *pos) {
	int track = _cdaTracks[ARG(0)];

	// The original seeks to the requested CD track here and pauses it.
	// The actual playback is then triggered via s_unpauseCD().
	if (track)
		_newTrack = track;
	_vm->snd_stopSound();
}

void SegaSequencePlayer::s_displayTextEn(const uint8 *pos) {
	if (_vm->gameFlags().lang == Common::JA_JPN)
		return;

	const char *str = (const char*)pos;
	_vm->_txt->clearDim(2);

	if (_playingID >= 55) {
		int cs = _screen->setFontStyles(_screen->_currentFont, Font::kStyleFullWidth);
		_vm->_txt->printShadedText(str, 0, 0, -1, 0xEE);
		_screen->setFontStyles(_screen->_currentFont, cs);
	} else {
		int x = 0;
		int y = 0;

		if (_playingID >= 53) {
			x = 152 - (_screen->getTextWidth(str) >> 1);
			y = 16;
		}

		_vm->_txt->printShadedText(str, x, y, -1, 0xEE);
	}
}

void SegaSequencePlayer::s_loadCustomPalettes(const uint8 *pos) {
	Common::SeekableReadStreamEndian *in = _res->resStreamEndian(0);
	in->seek(ARG(0) << 5);
	_screen->sega_loadCustomPaletteData(in);
	delete in;
}

void SegaSequencePlayer::s_playSoundEffect(const uint8 *pos) {
	if (!_fastForward)
		_vm->snd_playSoundEffect(ARG(0));
}

#undef S_ARG
#undef ARG

} // End of namespace Kyra

#endif // ENABLE_EOB
