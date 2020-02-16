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

#include "kyra/engine/eob.h"
#include "kyra/graphics/screen_eob.h"
#include "kyra/graphics/screen_eob_segacd.h"
#include "kyra/resource/resource.h"
#include "kyra/resource/resource_segacd.h"
#include "kyra/sequence/seqplayer_eob_segacd.h"
#include "common/system.h"

namespace Kyra {

SegaSequencePlayer::SegaSequencePlayer(EoBEngine *vm, Screen_EoB *screen, SegaCDResource *res) : _vm(vm), _screen(screen), _res(res), _tileSets(0), _debugResyncCnt(0), _varUnkX1(0),
	_var1(false), _waterdeepScene(0), _update2(0), _waitFlag(false), _waterdeepSceneTimer(0), _unkSEQ2(0), _renderer(_screen->sega_getRenderer()), _animator(_screen->sega_getAnimator()) {
#define SQOPC(x) _opcodes.push_back(new SQOpcode(this, &SegaSequencePlayer::x, #x))
	SQOPC(s_initDrawObject);
	SQOPC(s_drawTileSet);
	SQOPC(s_loadTileDataSingle);
	SQOPC(s_3);
	SQOPC(s_4);
	SQOPC(s_fillRect);
	SQOPC(s_6);
	SQOPC(s_7);
	SQOPC(s_8);
	SQOPC(s_9_dispText);
	SQOPC(s_fadeToNeutral);
	SQOPC(s_fadeToBlack);
	SQOPC(s_fadeToNeutral2);
	SQOPC(s_fadeToWhite);
	SQOPC(s_setPalette);
	SQOPC(s_vScroll);
	SQOPC(s_hScroll);
	SQOPC(s_paletteOps);
	SQOPC(s_initSprite);
	SQOPC(s_fillRectWithPattern);
	SQOPC(s_loadTileDataMult);
	SQOPC(s_21);
	SQOPC(s_22);
	SQOPC(s_initSprite2);
	SQOPC(s_drawTileSetCustom);
	SQOPC(s_waitForPaletteFade);
	SQOPC(s_clearSprites);
	SQOPC(s_27);
	SQOPC(s_moveSprites);
	SQOPC(s_moveMorphSprite);
	SQOPC(s_unpauseCD);
	SQOPC(s_enableWaterDeepAnimations);
	SQOPC(s_32);
	SQOPC(s_setUpdate2);
	SQOPC(s_orbEffect);
	SQOPC(s_stopCD);
	SQOPC(s_playCD);
	SQOPC(s_displayText);
	SQOPC(s_loadCustomPalettes);
	SQOPC(s_playSoundEffect);
#undef SQOPC

	_vScrollTimers = new ScrollTimer[2];
	assert(_vScrollTimers);
	_hScrollTimers = new ScrollTimer[2];
	assert(_hScrollTimers);
	_tileSets = new TileSet[100];
	assert(_tileSets);
	memset(_tileSets, 0, 100 * sizeof(TileSet));
	_drawObjects = new DrawObject[100];
	assert(_drawObjects);
	memset(_drawObjects, 0, 100 * sizeof(DrawObject));
	_tempBuffer = new uint8[65000];
	assert(_tempBuffer);
	memset(_tempBuffer, 0, 65000 * sizeof(uint8));
	_varUnkX1;
	memset(_varUnkX2, 0, sizeof(_varUnkX2));

	int temp;
	_wdDsX = _vm->staticres()->loadRawDataBe16(kEoB1IntroWdDsX, temp);
	_wdDsY = _vm->staticres()->loadRawData(kEoB1IntroWdDsY, temp);
}

SegaSequencePlayer::~SegaSequencePlayer() {
	delete[] _drawObjects;
	delete[] _tileSets;
	delete[] _vScrollTimers;
	delete[] _hScrollTimers;
	delete[] _tempBuffer;

	for (Common::Array<SQOpcode*>::iterator i = _opcodes.begin(); i != _opcodes.end(); ++i)
		delete (*i);
}

bool SegaSequencePlayer::play(int id) {
	//uint8 shadowColor = 0xEE;

	_screen->sega_fadeToBlack(2);

	//gfx37(0);
	_animator->clearSprites();
	setVScrollTimers(0, 1, 0, 0, 1, 0);
	setHScrollTimers(0, 1, 0, 0, 1, 0);
	_vm->_txt->clearDim(2);

	_renderer->fillRectWithTiles(2, 0, 0, 40, 28, 0xE6C2);
	_renderer->fillRectWithTiles(0, 0, 0, 64, 42, 0);
	_renderer->fillRectWithTiles(1, 0, 0, 64, 28, 0);
	_renderer->fillRectWithTiles(2, 1, (id == 53 || id == 54) ? 22 : 20, 38, 6, 0xE51C, true);

	_debugResyncCnt = 0;

	for (bool runLoop = true; runLoop && !(_vm->shouldQuit() || _vm->skipFlag()); ) {
		uint32 offset = (id - 1) * 0x3C000;
		if (id >= 54)
			offset -= 0x8000;
		if (id >= 55)
			offset -= 0x18000;

		uint32 size = (id == 53) ? 0x34000 : ((id == 54) ? 0x24000 : 0x3C000);

		if (!_res->loadContainer("VISUAL", offset, size))
			return false;

		Common::SeekableReadStreamEndian *in = _res->getEndianAwareResourceStream(0);
		if (!in)
			return false;
		_screen->sega_loadCustomPaletteData(in);
		delete in;

		_screen->sega_selectPalette(0, 31, false);
		_screen->sega_selectPalette(1, 32, false);
		_screen->sega_selectPalette(3, 30, false);

		in = _res->getEndianAwareResourceStream(2);
		if (!in)
			return false;
		uint32 len = in->size();
		uint8 *tileData = new uint8[len];
		in->read(tileData, len);
		delete in;

		in = _res->getEndianAwareResourceStream(1);
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

		in = _res->getEndianAwareResourceStream(3);
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

	return true;
}

void SegaSequencePlayer::setWaitFlag(bool enable) {
	_waitFlag = enable;
}

void SegaSequencePlayer::run(const uint8 *data) {
	_var1 = _waterdeepScene = _update2 = false;
	uint32 frameCounter = 0;
	uint32 nextFrame = 0;

	for (bool runLoop = true; runLoop && !(_vm->shouldQuit() || _vm->skipFlag()); ) {
		uint16 frameSize = READ_BE_UINT16(data);
		if (!frameSize)
			return;

		uint32 frameStart = _vm->_system->getMillis();
		uint16 timeStamp = READ_BE_UINT16(data + 2);
		uint32 lastFrame = nextFrame;
		nextFrame = timeStamp * 16;
		if (nextFrame < lastFrame)
			frameCounter = 0;

		if (frameCounter >= nextFrame) {
			debugC(5, kDebugLevelSequence, "SeqPlayer: Timestamp %08d", timeStamp);
			for (uint16 timeStamp2 = timeStamp; timeStamp2 == timeStamp; ) {
				uint16 op = READ_BE_UINT16(data + 4);
				_opcodes[op]->run(data + 6);

				frameSize = READ_BE_UINT16(data);
				data += (frameSize & ~1);

				timeStamp2 = READ_BE_UINT16(data + 2);
			}
		}

		if (_waterdeepScene)
			animateWaterdeepScene();

		if (_update2)
			update2();

		updateScrollTimers();
		_animator->update();
		_renderer->render(0);
		_screen->sega_updatePaletteFaders(-1);
		_screen->updateScreen();

		uint32 now = _vm->_system->getMillis();
		int diff = now - (frameStart + 16);
		if (diff < 0)
			_vm->delay(frameStart + 16 - now);
		else if (diff) {
			frameCounter += diff;
			// This will be triggered with higher values whenever there is a palette fading and the code waits for it
			// to finish (the code will wait for s_fadeToBlack() and s_fadeToNeutral() but not for s_paletteOps(), unless
			// followed by s_waitForPaletteFade()). This doesn't cause issues, since it is the intended original behavior,
			// but it needs to be compensated in the timing. The original does that automatically, since the frameCounter
			// comes from the vblank interrupt vector. I don't use _system->getMillis directly for the frame timer, since
			// it will be messed up when activating the GMM during sequence playback.
			debugC(4, kDebugLevelSequence, "    Out of Sync. Catching up millis: %d", diff);
			_debugResyncCnt += diff;
		}
		frameCounter += 16;

		/*if (_finFlag & 0x80) {
			if (_waitFlag || _curVis == 55 || _curVis == 56)
				return true;
		}*/
	}
}

void SegaSequencePlayer::setVScrollTimers(uint16 destA, int incrA, int delayA, uint16 destB, int incrB, int delayB) {
	_vScrollTimers[0]._offsDest = destA;
	_vScrollTimers[0]._incr = incrA;
	_vScrollTimers[0]._timer = _vScrollTimers[0]._delay = delayA;
	_vScrollTimers[1]._offsDest = destB;
	_vScrollTimers[1]._incr = incrB;
	_vScrollTimers[1]._timer = _vScrollTimers[1]._delay = delayB;
}

void SegaSequencePlayer::setHScrollTimers(uint16 destA, int incrA, int delayA, uint16 destB, int incrB, int delayB) {
	_hScrollTimers[0]._offsDest = destA;
	_hScrollTimers[0]._incr = incrA;
	_hScrollTimers[0]._timer = _hScrollTimers[0]._delay = delayA;
	_hScrollTimers[1]._offsDest = destB;
	_hScrollTimers[1]._incr = incrB;
	_hScrollTimers[1]._timer = _hScrollTimers[1]._delay = delayB;
}

void SegaSequencePlayer::updateScrollTimers() {
	for (int i = 0; i <= 4; ++i) {
		ScrollTimer &t = i < 2 ? _vScrollTimers[i] : _hScrollTimers[i - 2];
		if (t._delay == 0 && t._offsCur != t._offsDest)
			t._offsCur = t._offsDest;
		if (t._offsCur == t._offsDest)
			continue;
		if (--t._timer)
			continue;

		t._offsCur += t._incr;
		t._timer = t._delay;
	}

	_renderer->writeVSRAMValue(0, _vScrollTimers[0]._offsCur);
	_renderer->writeVSRAMValue(2, _vScrollTimers[1]._offsCur);
	uint16 hscr[2] = { _hScrollTimers[0]._offsCur, _hScrollTimers[1]._offsCur };
	_renderer->loadToVRAM(hscr, 4, 0xD800);
}

void SegaSequencePlayer::animateWaterdeepScene() {
	static uint8 spr[31] = {
		0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03,	0x03, 0x04, 0x04, 0x05, 0x05, 0x05, 0x06, 0x06,
		0x07, 0x07, 0x07, 0x08, 0x08, 0x09, 0x09, 0x0a,	0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0d, 0x0d
	};

	if (--_waterdeepSceneTimer > 0)
		return;
	_waterdeepSceneTimer = 5;

	for (int i = 0; i < 5; ++i) {
		int rnd = _vm->_rnd.getRandomNumber(30);
		DrawObject *d = &_drawObjects[10 + rnd];
		_animator->initSprite(spr[rnd] + 3, _wdDsX[spr[rnd]] - 80, _wdDsY[spr[rnd]] + 32, d->nTblVal, d->addr);
	}
}

void SegaSequencePlayer::update2() {

}

void SegaSequencePlayer::update3() {

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

void SegaSequencePlayer::s_3(const uint8 *pos) {

}

void SegaSequencePlayer::s_4(const uint8 *pos) {

}

void SegaSequencePlayer::s_fillRect(const uint8 *pos) {
	_renderer->fillRectWithTiles(ARG(8), ARG(0), ARG(2), ARG(4), ARG(6), ARG(10));
}

void SegaSequencePlayer::s_6(const uint8 *pos) {

}

void SegaSequencePlayer::s_7(const uint8 *pos) {

}

void SegaSequencePlayer::s_8(const uint8 *pos) {

}

void SegaSequencePlayer::s_9_dispText(const uint8 *pos) {

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
	setVScrollTimers(ARG(0), S_ARG(2), ARG(4), ARG(6), S_ARG(8), ARG(10));
}

void SegaSequencePlayer::s_hScroll(const uint8 *pos) {
	setHScrollTimers(ARG(0), S_ARG(2), ARG(4), ARG(6), S_ARG(8), ARG(10));
}

void SegaSequencePlayer::s_paletteOps(const uint8 *pos) {
	_screen->sega_paletteOps(S_ARG(0), S_ARG(2), S_ARG(4));
}

void SegaSequencePlayer::s_initSprite(const uint8 *pos) {
	DrawObject *d = &_drawObjects[ARG(2)];
	_animator->initSprite(ARG(0), S_ARG(4), S_ARG(6), d->nTblVal, d->addr);
}

void SegaSequencePlayer::s_fillRectWithPattern(const uint8 *pos) {
	_renderer->fillRectWithTiles(ARG(8), ARG(0), ARG(2), ARG(4), ARG(6), ARG(10), false, false, ARG(12));
}

void SegaSequencePlayer::s_loadTileDataMult(const uint8 *pos) {
	for (DrawObject *w = &_drawObjects[ARG(0)]; w != &_drawObjects[ARG(0) + ARG(2)]; ++w)
		_renderer->loadToVRAM(w->tileData, (w->width * w->height) << 5, (w->nTblVal & 0x7FF) << 5);
}

void SegaSequencePlayer::s_21(const uint8 *pos) {

}

void SegaSequencePlayer::s_22(const uint8 *pos) {

}

void SegaSequencePlayer::s_initSprite2(const uint8 *pos) {
	_animator->initSprite(ARG(0), S_ARG(4), S_ARG(6), ARG(2), ARG(8));
}

void SegaSequencePlayer::s_drawTileSetCustom(const uint8 *pos) {
	DrawObject *w = &_drawObjects[ARG(0)];
	_renderer->fillRectWithTiles(w->addr, ARG(2), ARG(4), w->width, w->height, w->nTblVal, true);
}

void SegaSequencePlayer::s_waitForPaletteFade(const uint8*) {
	_screen->sega_fadePalette(0, 0, -1, true, true);
}

void SegaSequencePlayer::s_clearSprites(const uint8*) {
	_animator->clearSprites();
}

void SegaSequencePlayer::s_27(const uint8 *pos) {

}

void SegaSequencePlayer::s_moveSprites(const uint8 *pos) {
	_animator->moveSprites(ARG(0), ARG(2), S_ARG(4), S_ARG(6));
}

void SegaSequencePlayer::s_moveMorphSprite(const uint8 *pos) {
	_animator->moveMorphSprite(ARG(0), ARG(2), S_ARG(4), S_ARG(6));
}

void SegaSequencePlayer::s_unpauseCD(const uint8 *pos) {
	// Do nothing. We don't support this in our AudioCD API. The original will use s_playCD() to seek to a track,
	// wait for the seek to finish and then pause. It then used this opcode to actually start the playback. Since
	// s_playCD() and s_unpauseCD() always seem to be called on the same frame we can just start the playback
	// normally. The only difference to the playback of songs is that there is no looping.
}

void SegaSequencePlayer::s_enableWaterDeepAnimations(const uint8 *pos) {
	_waterdeepScene = ARG(0);
	_waterdeepSceneTimer = 0;
}

void SegaSequencePlayer::s_32(const uint8 *pos) {
	if (ARG(0) == 100) {
		_varUnkX1 = ARG(2);
	} else {
		assert(ARG(0) < 6);
		_varUnkX2[ARG(0) * 2] = ARG(2);
		_varUnkX2[ARG(0) * 2 + 1] = ARG(4);
	}
}

void SegaSequencePlayer::s_setUpdate2(const uint8 *pos) {
	_update2 = ARG(0);
	_unkSEQ2 = 0;
	if (_update2)
		update3();
}

void SegaSequencePlayer::s_orbEffect(const uint8*) {
	_renderer->memsetVRAM(0x2AA0, 0, 22528);
	DrawObject *d = &_drawObjects[16];
	memset(_tempBuffer, 0, 22528);
	memcpy(_tempBuffer + 128, d->tileData, (d->width * d->height) << 5);
	_renderer->fillRectWithTiles(0, 4, 0, 32, 22, 0x2155, true, true);

	memset(&_tempBuffer[0xF600], 0, 512 * sizeof(uint8));
	uint8 *dst2 = &_tempBuffer[0xF600 + (7 << 4) + 6];
	uint16 t = 1;
	for (int h = 0; h < 9; ++h) {
		uint8 *dst = dst2;
		for (int w = 0; w < 10; ++w) {
			*dst++ = t & 0xFF;
			*dst++ = t++ >> 4;;
		}
		dst2 += 32;
	}
	int z = 512;
	for (int i = 0; i < 90; ++i) {
		uint16 *dst = (uint16*)(&_tempBuffer[0x6A00]);
		uint16 a = 0x58000 - z * 128;
		uint16 b = 0x59000 - z * 88;
		for (int ii = 0; ii < 90; ++ii) {
			*dst++ = a >> 8;
			*dst++ = b >> 8;
			*dst++ = z;
			*dst++ = 0;
			b += z;
		}
		_renderer->loadToVRAM(&_tempBuffer[0x6A00], 22528, 0x2AA0);
		z += 16;
	}
}

void SegaSequencePlayer::s_stopCD(const uint8*) {
	_vm->snd_stopSound();
}

void SegaSequencePlayer::s_playCD(const uint8 *pos) {
	int track = _cdaTracks[ARG(0)];
	if (track)
		_vm->snd_playSong(track, false);

	if (_waitFlag) {
		while (!(_vm->shouldQuit() || _vm->skipFlag()))
			_vm->delay(20);
	}
}

void SegaSequencePlayer::s_displayText(const uint8 *pos) {

}

void SegaSequencePlayer::s_loadCustomPalettes(const uint8 *pos) {
	Common::SeekableReadStreamEndian *in = _res->getEndianAwareResourceStream(0);
	in->seek(ARG(0) << 5);
	_screen->sega_loadCustomPaletteData(in);
	delete in;
}

void SegaSequencePlayer::s_playSoundEffect(const uint8 *pos) {
	_vm->snd_playSoundEffect(ARG(0));
}

#undef S_ARG
#undef ARG

const uint8 SegaSequencePlayer::_cdaTracks[60] = {
	0x00, 0x0d, 0x0f, 0x11, 0x12, 0x13, 0x14, 0x15,
	0x00, 0x1d, 0x1e, 0x00, 0x0e, 0x16, 0x37, 0x38,
	0x22, 0x00, 0x23, 0x24, 0x25, 0x1c, 0x18, 0x10,
	0x1f, 0x17, 0x00, 0x1b, 0x21, 0x00, 0x00, 0x00,
	0x30, 0x31, 0x26, 0x27, 0x28, 0x29, 0x35, 0x36,
	0x33, 0x34, 0x2c, 0x2b, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};

} // End of namespace Kyra
