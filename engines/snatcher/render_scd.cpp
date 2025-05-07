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
#include "common/endian.h"
#include "graphics/pixelformat.h"
#include "graphics/segagfx.h"
#include "snatcher/graphics.h"
#include "snatcher/palette.h"
#include "snatcher/render.h"
#include "snatcher/resource.h"
#include "snatcher/staticres.h"
#include "snatcher/transition.h"
#include "snatcher/util.h"
#include "common/system.h"

namespace Snatcher {

//#define		ANIM_DEBUG

struct AnimObject {
	AnimObject(int num) : id(num), enable(0), fadeLevel(0), drawFlags(0), posX(0), posY(0), relSpeedX(0), relSpeedY(0), f16(0), f18(0), f1c(0),
		timeStamp(0), f24(0), controlFlags(0), allowFrameDrop(0), frameSeqCounter(0), frame(0), frameDelay(0), f2c(0), f2d(0), spriteTableLocation(0),
			res(), scriptData(), spriteData(nullptr), absSpeedX(0), absSpeedY(0), parent(0), children(0), next(0), f4e(0), f4f(0), blink(0), blinkCounter(0), blinkDuration(0) {}
	
	void clear() {
		enable = fadeLevel = drawFlags = 0;
		posX = posY = 0;
		relSpeedX = relSpeedY = 0;
		f16 = 0;
		f18 = 0;
		f1c = 0;
		timeStamp = 0;
		f24 = 0;
		controlFlags = allowFrameDrop = frameSeqCounter = 0;
		frame = 0;
		frameDelay = 0;
		f2c = f2d = 0;
		spriteTableLocation = 0;
		res = scriptData = ResourcePointer();
		spriteData = nullptr;
		absSpeedX = absSpeedY = 0;
		parent = children = next = 0;
		f4e = f4f = 0;
		blink = blinkCounter = blinkDuration = 0;

	}

	uint16 enable;
	uint16 fadeLevel;
	uint16 drawFlags;
	int32 posX;
	int32 posY;

	int32 relSpeedX;
	int32 relSpeedY;
	uint8 f16;
	uint16 f18;
	uint8 f1c;
	uint32 timeStamp;
	uint8 f24;
	uint8 controlFlags;
	uint8 allowFrameDrop;
	uint8 frameSeqCounter;
	uint16 frame;
	int16 frameDelay;
	uint8 f2c;
	uint8 f2d;
	uint32 spriteTableLocation;
	ResourcePointer res;
	ResourcePointer scriptData;
	const uint16 *spriteData;
	uint16 blink;
	uint16 blinkCounter;
	uint16 blinkDuration;
	int32 absSpeedX;
	int32 absSpeedY;
	uint16 parent;
	uint16 children;
	uint16 next;
	uint8 f4e;
	uint8 f4f;

	const uint8 id;
};

class GraphicsEngine;
class Renderer_SCD : public Renderer, public Graphics::SegaRenderer::HINTClient {
public:
	Renderer_SCD(const Graphics::PixelFormat *pxf, GraphicsEngine::GfxState &state, Palette *pal, TransitionManager *scr);
	~Renderer_SCD() override;

	bool enqueueDrawCommands(ResourcePointer &res) override;
	void clearDrawCommands() override;
	void initData(ResourcePointer &res, uint8 mode) override;
	void initAnimations(ResourcePointer &res, uint16 len) override;
	void linkAnimations(ResourcePointer &res, uint16 len) override;
	void clearAnimations(int mode = 0) override;
	void setPlaneMode(uint16 mode) override;

	void updateScreen(uint8 *screen) override;
	void updateAnimations() override;

	void anim_setControlFlags(uint8 animObjId, int flags) override;
	void anim_addControlFlags(uint8 animObjId, int flags) override;
	void anim_clearControlFlags(uint8 animObjId, int flags) override;
	void anim_setFrame(uint8 animObjId, uint16 frameNo) override;
	uint16 anim_getCurFrameNo(uint8 animObjId) const override;
	void anim_setPosX(uint8 animObjId, int16 x) override;
	void anim_setPosY(uint8 animObjId, int16 y) override;
	void anim_setSpeedX(uint8 animObjId, int16 speedX) override;
	void anim_setSpeedY(uint8 animObjId, int16 speedY) override;
	void anim_toggleBlink(uint8 animObjId, bool enable) override;
	bool anim_isEnabled(uint8 animObjId) const override;
	void anim_updateBlink() override;

	uint16 screenWidth() const override { return _screenWidth; }
	uint16 screenHeight() const override { return _screenHeight; }

	void createMouseCursor() override;

	void hINTCallback(Graphics::SegaRenderer *sr) override;

private:
	void loadDataFromGfxScript();
	void executeDrawCommands();
	void drawAnimSprites();
	void reconfigPlanes();
	void updateScrollState();

	void generateSpriteData(AnimObject &a, uint16 &spritesCurIndex, uint16 *&spritesBufferCurPos);
	void updateAnim32Spec(AnimObject &a);
	bool reachedAudioTimeStamp(AnimObject &a) const;
	void runAnimScript(AnimObject &a);

	const uint16 _screenWidth, _screenHeight;

	ResourcePointer _transferData;
	uint8 *_tempBuffer;
	uint16 *_spriteBuffer;
	uint8 _transferMode;
	uint16 _transferDelay;
	uint16 _clearFlags;
	uint8 _mode;
	uint8 _modeChange;
	Graphics::SegaRenderer *_sr;
	Palette *_pal;
	TransitionManager *_trs;
	const Graphics::SegaRenderer::HINTHandler _hINTClientProc;

private:
	AnimObject **_animations;

	struct DrawCommand {
		DrawCommand() : cmd(0), progress(0), ptr(0), res() {}
		void clear() {
			cmd = 0;
			progress = 0;
			ptr = 0;
			res = ResourcePointer();
		}
		uint8 cmd;
		uint8 progress;
		const uint8 *ptr;
		ResourcePointer res;
	};

	DrawCommand *_drawCommands;

private:
#ifndef ANIM_DEBUG
	typedef Common::Functor2Mem<AnimObject&, const uint8*, int, Renderer_SCD> GfxAnimFunc;
#else
	class GfxAnimFunc : public Common::Functor2Mem<AnimObject&, const uint8*, int, Renderer_SCD> {
	public:
		typedef int (Renderer_SCD::*AnimFunc)(AnimObject&, const uint8*);
		GfxAnimFunc(Renderer_SCD *gfx, AnimFunc func, const char *desc) : Common::Functor2Mem<AnimObject &, const uint8*, int, Renderer_SCD>(gfx, func), _desc(Common::String("anim_") + desc) {}
		int operator()(AnimObject &a, const uint8 *data) {
			debug("%s():   anim: %02d,   data: [%02x %02x %02x]", _desc.c_str(), a.id, data[0], data[1], data[2]);
			return Functor2Mem<AnimObject &, const uint8*, int, Renderer_SCD>::operator()(a, data);
		}
	private:
		Common::String _desc;
	};
#endif
	Common::Array<GfxAnimFunc*> _animProcs;

	void makeAnimFunctions();

	int anim_terminate(AnimObject &a, const uint8 *data);
	int anim_setFrame(AnimObject &a, const uint8 *data);
	int anim_seqSetFrame(AnimObject &a, const uint8 *data);
	int anim_rndSeqSetFrame(AnimObject &a, const uint8 *data);
	int anim_setX(AnimObject &a, const uint8 *data);
	int anim_setY(AnimObject &a, const uint8 *data);
	int anim_setSpeedX(AnimObject &a, const uint8 *data);
	int anim_setSpeedY(AnimObject &a, const uint8 *data);
	int anim_palEvent(AnimObject &a, const uint8 *data);
	int anim_palReset(AnimObject &a, const uint8 *data);
	int anim_copyCmds(AnimObject &a, const uint8 *data);
	int anim_nop(AnimObject &a, const uint8 *data);
	int anim_scrollSingleStepH(AnimObject &a, const uint8 *data);
	int anim_scrollSingleStepV(AnimObject &a, const uint8 *data);
	int anim_scrollStartH(AnimObject &a, const uint8 *data);
	int anim_scrollStartV(AnimObject &a, const uint8 *data);
	int anim_pause(AnimObject &a, const uint8 *data);
	int anim_resumeAndUnsyncOther(AnimObject &a, const uint8 *data);
	int anim_terminateOther(AnimObject &a, const uint8 *data);
	int anim_pauseAndHideOther(AnimObject &a, const uint8 *data);
	int anim_hide(AnimObject &a, const uint8 *data);
	int anim_24(AnimObject &a, const uint8 *data);
	int anim_audioSync(AnimObject &a, const uint8 *data);
	int anim_27(AnimObject &a, const uint8 *data);
	int anim_28(AnimObject &a, const uint8 *data);
	int anim_29(AnimObject &a, const uint8 *data);
	int anim_30(AnimObject &a, const uint8 *data);
	int anim_31(AnimObject &a, const uint8 *data);
	int anim_setDrawFlags(AnimObject &a, const uint8 *data);
	int anim_34(AnimObject &a, const uint8 *data);
	int anim_35(AnimObject &a, const uint8 *data);
	int anim_36(AnimObject &a, const uint8 *data);
	int anim_37(AnimObject &a, const uint8 *data);
	int anim_38(AnimObject &a, const uint8 *data);
	int anim_39(AnimObject &a, const uint8 *data);
	int anim_40(AnimObject &a, const uint8 *data);
	int anim_41(AnimObject &a, const uint8 *data);
	int anim_42(AnimObject &a, const uint8 *data);
	int anim_43(AnimObject &a, const uint8 *data);
	int anim_allowFrameDrop(AnimObject &a, const uint8 *data);
	int anim_45(AnimObject &a, const uint8 *data);
	int anim_46(AnimObject &a, const uint8 *data);
	int anim_47(AnimObject &a, const uint8 *data);

	int anim_updateFrameDelay(AnimObject &a, const uint8 *data);
	void anim_setGroupParameter(AnimObject &a, int pata, int16 val = 0, bool recursive = false);

public:
	// boot logo animation
	int drawBootLogoFrame(uint8 *screen, int frameNo) override;
private:
	int32 _bootsDelay, _bootsTotalFrames;
	uint16 _bootsHScroll, _bootsVScroll, _bootsCol;
	uint8 *_bootsSprTbl;
};

Renderer_SCD::Renderer_SCD(const Graphics::PixelFormat *pxf, GraphicsEngine::GfxState &state, Palette *pal, TransitionManager *scr) : Renderer(state), _pal(pal), _trs(scr), _mode(1),
	_modeChange(0), _screenWidth(256), _screenHeight(224), _transferData(), _transferMode(0), _transferDelay(0), _clearFlags(0), _tempBuffer(nullptr), _spriteBuffer(nullptr), _sr(nullptr),
		_animations(nullptr), _drawCommands(nullptr), _hINTClientProc(Graphics::SegaRenderer::HINTHandler(this, &Graphics::SegaRenderer::HINTClient::hINTCallback)), _bootsDelay(0), _bootsSprTbl(nullptr),
			_bootsTotalFrames(0), _bootsHScroll(0), _bootsVScroll(0), _bootsCol(0) {

	makeAnimFunctions();

	assert(pxf);
	_sr = new Graphics::SegaRenderer(pxf);
	assert(_sr);

	_tempBuffer = new uint8[0x10000]();
	assert(_tempBuffer);

	_spriteBuffer = new uint16[0x100]();
	assert(_spriteBuffer);

	_drawCommands = new DrawCommand[7]();
	assert(_drawCommands);

	_animations = new AnimObject*[64]();
	assert(_animations);
	for (int i = 0; i < 64; ++i) {
		_animations[i] = new AnimObject(i);
		assert(_animations[i]);
	}

	_sr->setResolution(_screenWidth, _screenHeight);
	_sr->setupWindowPlane(0, 0, Graphics::SegaRenderer::kWinToLeft, Graphics::SegaRenderer::kWinToTop);
	_sr->setVScrollMode(Graphics::SegaRenderer::kVScrollFullScreen);
	_sr->setHScrollMode(Graphics::SegaRenderer::kHScrollFullScreen);
	_sr->hINT_setHandler(&_hINTClientProc);

	reconfigPlanes();
}

Renderer_SCD::~Renderer_SCD() {
	delete _sr;
	delete[] _tempBuffer;
	delete[] _spriteBuffer;
	delete[] _drawCommands;

	if (_animations) {
		for (int i = 0; i < 64; ++i) 
			delete _animations[i];
		delete[] _animations;
	}

	for (uint i = 0; i < _animProcs.size(); ++i) \
		delete _animProcs[i];
}

bool Renderer_SCD::enqueueDrawCommands(ResourcePointer &res) {
	for (DrawCommand *it = _drawCommands; it < &_drawCommands[7]; ++it) {
		if (it->cmd != 0)
			continue;
		it->res = res;
		it->cmd = *res++;
		it->progress = 0;
		it->ptr = ++res;
		return true;
	}
	return false;
}

void Renderer_SCD::clearDrawCommands() {
	for (DrawCommand *it = _drawCommands; it < &_drawCommands[7]; ++it)
		it->clear();
}

void Renderer_SCD::initData(ResourcePointer &res, uint8 mode) {
	_transferData = res;
	_transferMode = mode;
}

void Renderer_SCD::initAnimations(ResourcePointer &res, uint16 len) {
	ResourcePointer next = res + len;
	ResourcePointer in = res;
	while (in < next) {
		assert(in[0] < 64);
		AnimObject *a = _animations[*in++];
		if (a->enable)
			return;

		a->clear();
		a->enable = 1;
		a->res = res;
		a->controlFlags = *in++;
		a->posX = in.readIncrSINT16() << 16;
		a->posY = in.readIncrSINT16() << 16;
		a->spriteTableLocation = in.readIncrUINT32();
		int16 offs = in.readIncrSINT16();
		a->scriptData = in + offs;
		a->frame = 0xFFFF;

		uint8 cmd = a->scriptData[0];
		for (bool l = true; l; ) {
			if (cmd == 0xA1 || cmd == 0xA7) {
				a->f16 = a->scriptData[1];
				cmd = a->scriptData[4];
				assert(cmd != 0xA1 && cmd != 0xA7); // The original code would theoretically allow a deadlock here.
			} else {
				if (!(cmd & 0x80))
					a->spriteData = reinterpret_cast<const uint16*>(res.makeAbsPtr(a->spriteTableLocation).getDataFromTable(cmd)());
				l = false;
			}
		}
	}
}

void Renderer_SCD::linkAnimations(ResourcePointer &res, uint16 len) {
	ResourcePointer next = res + len;
	ResourcePointer in = res;
	while (in < next) {
		uint16 p = *in++;
		uint16 c = *in++;
		assert(p < 64 && c < 64);
		AnimObject *pPr = _animations[p];
		AnimObject *pCh = _animations[c];
		pCh->next = pPr->children;
		pPr->children = c;
		pCh->parent = p;
	}
}

void Renderer_SCD::clearAnimations(int mode) {
	if (mode) {
		for (int i = 1; i < 64; ++i) {
			AnimObject *s = _animations[i];
			if (s->enable && !(s->f4e & 1))
				s->clear();
		}
	} else {
		for (int i = 0; i < 64; ++i)
			_animations[i]->clear();
	}
}
void Renderer_SCD::setPlaneMode(uint16 mode) {
	assert(mode >> 8 == 0);
	_modeChange = mode & 0xFF;;
}

void Renderer_SCD::updateScreen(uint8 *screen) {
	if (_gfxState.getVar(9)) {
		_sr->setPlaneTableLocation(Graphics::SegaRenderer::kPlaneB, _gfxState.getVar(9) == 0xFF ? 0xE000 : 0xC000);
		_gfxState.setVar(9, 0);
	}

	if (_clearFlags & 1)
		_sr->memsetVRAM(0xE000, 0, 0x2000);
	if (_clearFlags & 2)
		_sr->memsetVRAM(0xC000, 0, 0x2000);
	_clearFlags = 0;

	if (_modeChange)
		reconfigPlanes();

	loadDataFromGfxScript();
	executeDrawCommands();
	drawAnimSprites();
	updateScrollState();

	_sr->setRenderColorTable(_pal->getSystemPalette(), 0, 64);
	_sr->render(screen);
}

void Renderer_SCD::updateAnimations() {
	for (int i = 0; i < 64; ++i) {
		AnimObject *s = _animations[i];
		int16 dropFrames = _gfxState.getDropFrames();
		if ((s->f1c || s->allowFrameDrop) && _gfxState.getVar(10))
			continue;
		if (dropFrames)
			debug("%s(): Dropping %d frames for anim %d", __FUNCTION__, dropFrames, s->id);
		do {
			if (!s->enable)
				break;
			if (s->controlFlags & GraphicsEngine::kAnimAudioSync) {
				if (!reachedAudioTimeStamp(*s))
					break;
				s->controlFlags &= ~GraphicsEngine::kAnimAudioSync;
			}
			if (s->controlFlags & GraphicsEngine::kAnimPause)
				break;

			runAnimScript(*s);

		} while (--dropFrames >= 0 && s->allowFrameDrop);
	}

	for (int i = 0; i < 64; ++i) {
		AnimObject *s = _animations[i];
		if (!s->enable || (s->f1c & _gfxState.getVar(10)))
			continue;

		uint8 cmd = s->scriptData[(s->frame == 0xFFFF ? 0 : s->frame) << 2];
		if (!(cmd & 0x80))
			s->spriteData = reinterpret_cast<const uint16*>(s->res.makeAbsPtr(s->spriteTableLocation).getDataFromTable(cmd)());

		if (!(s->controlFlags & GraphicsEngine::kAnimPause)) {
			s->posX += s->absSpeedX;
			s->posY += s->absSpeedY;
		}
	}
}

void Renderer_SCD::anim_setControlFlags(uint8 animObjId, int flags) {
	assert(animObjId < 64);
	flags &= 7;
	_animations[animObjId]->controlFlags = flags;
}

void Renderer_SCD::anim_addControlFlags(uint8 animObjId, int flags) {
	assert(animObjId < 64);
	flags &= 7;
	_animations[animObjId]->controlFlags |= flags;
}

void Renderer_SCD::anim_clearControlFlags(uint8 animObjId, int flags) {
	assert(animObjId < 64);
	_animations[animObjId]->controlFlags &= ~flags;
}

void Renderer_SCD::anim_setFrame(uint8 animObjId, uint16 frameNo) {
	assert(animObjId < 64);
	_animations[animObjId]->frame = frameNo;
}

uint16 Renderer_SCD::anim_getCurFrameNo(uint8 animObjId) const {
	assert(animObjId < 64);
	return _animations[animObjId]->frame;
}

void Renderer_SCD::anim_setPosX(uint8 animObjId, int16 x) {
	assert(animObjId < 64);
	_animations[animObjId]->posX = x << 16;
}

void Renderer_SCD::anim_setPosY(uint8 animObjId, int16 y) {
	assert(animObjId < 64);
	_animations[animObjId]->posY = y << 16;
}

void Renderer_SCD::anim_setSpeedX(uint8 animObjId, int16 speedX) {
	assert(animObjId < 64);
	_animations[animObjId]->absSpeedX = speedX << 16;
}

void Renderer_SCD::anim_setSpeedY(uint8 animObjId, int16 speedY) {
	assert(animObjId < 64);
	_animations[animObjId]->absSpeedY = speedY << 16;
}

void Renderer_SCD::anim_toggleBlink(uint8 animObjId, bool enable) {
	assert(animObjId < 64);
	_animations[animObjId]->blink = enable ? 1 : 0;
}

bool Renderer_SCD::anim_isEnabled(uint8 animObjId) const {
	assert(animObjId < 64);
	return _animations[animObjId]->enable != 0;
}

void Renderer_SCD::anim_updateBlink() {
	for (int i = 0; i < 3; ++i) {
		AnimObject &a = *_animations[17 + i];
		a.controlFlags = GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide;
		if (a.absSpeedX)
			a.controlFlags = GraphicsEngine::kAnimPause;
		if (a.blink == 1) {
			if (++a.blinkCounter >= 16) {
				a.blinkCounter = 0;
				a.absSpeedX ^= (1 << 16);
				if (++a.blinkDuration >= 6)
					a.blink = 0;
			}
		}
	}
}

void Renderer_SCD::hINTCallback(Graphics::SegaRenderer *sr) {
	if (_pal)
		_pal->hINTCallback(sr);
	if (_trs)
		_trs->hINTCallback(sr);
}

void Renderer_SCD::loadDataFromGfxScript() {
	if (_gfxState.getVar(0)) {
		if (_gfxState.getVar(0) == 1) {
			_gfxState.setVar(0, 2);
			_clearFlags = 3;
		} else {
			_gfxState.setVar(0, 0);
		}
		return;
	}

	if (!_gfxState.getVar(8))
		return;
	
	if (_transferDelay) {
		--_transferDelay;
		return;
	}

	if (_transferData.readUINT16() == 0xFFFF) {
		_gfxState.setVar(8, 0);
		return;
	}

	const uint8 *src = _transferData.makeAbsPtr(_transferData.readIncrUINT32() & 0xFFFFFF)();
	_transferDelay = _transferData.readIncrUINT16();
	uint16 addr = _transferData.readIncrUINT16();

	if (!_transferMode) {
		uint32 len = READ_BE_UINT16(src);
		if (len != Util::decodeSCDData(src + 2, _tempBuffer))
			error("%s(): Decode size mismatch", __FUNCTION__);
		_sr->loadToVRAM(_tempBuffer, len, addr);
		return;
	}

	assert(0);

	// multi part transfer: not needed (this is just due to original hardware limitations with the word ram and dma transfer)

	/*int D7 = 0;
	if (_transferOffset) {
		D7 = 0;//processGfxScriptCase1_set_dstA5_to_238000_etc();
		//len = D7;
	}*/

	//_transferSrc = 0x238000;
	//_transferVRAMAddr += _transferOffset;
	//_transferSrc += _transferOffset;

	//if (_transferLen >= 0x800) {
	//	_transferLen -= 0x800;
	//	_transferOffset += 0x800;
	//	D7 = 0x800;
	/*} else {
		D7 = _transferLen;
		_transferOffset = 0;
		_transferCommands += 8;
	}*/
	// code = 2
	// ga_comm_loc_E322_writeWordRam_codeD6_dstA5_srcA6_lenD7
}

void Renderer_SCD::executeDrawCommands() {
	if (_gfxState.getVar(8))
		return;

	static const uint8 offs[] =	{ 0x02, 0x03, 0x01, 0x03, 0x00, 0x02, 0x00, 0x01 };
	static const uint8 mask[] =	{ 0xF0, 0x0F, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F };

	for (DrawCommand *it = _drawCommands; it < &_drawCommands[7]; ++it) {
		if (it->cmd == 0)
			continue;

		bool loop = (it->cmd & 0x40);
		int section = (it->cmd & 0x30) >> 4;
		uint8 *s = _tempBuffer + section * 0x2000;
		uint16 a = 0;
		uint8 b = 0;

		if ((it->cmd & 0x0F) < 5 && (it->cmd & 0x0F) != 2 && !(it->cmd & 0x80)) {
			const uint8 *cmp = it->res.makeAbsPtr(READ_BE_UINT32(it->ptr))();
			uint32 len = READ_BE_UINT16(cmp);
			if (len != Util::decodeSCDData(cmp + 2, s))
				error("%s(): Decode size mismatch", __FUNCTION__);
		}

		switch (it->cmd & 0x0F) {
		case 0:
		case 1:
			_sr->loadToVRAM(s + READ_BE_INT16(it->ptr + 6), READ_BE_UINT16(it->ptr + 8), READ_BE_UINT16(it->ptr + 4));
			it->ptr += 10;
			if (READ_BE_UINT16(it->ptr) == 0xFFFF) {
				it->cmd = 0;
				break;
			}
			break;
		case 2:
			for (bool rept = true; rept; ) {
				memcpy(s + READ_BE_INT16(it->ptr + 6), it->res.makeAbsPtr(READ_BE_UINT32(it->ptr))(), READ_BE_UINT16(it->ptr + 8));
				_sr->loadToVRAM(s + READ_BE_INT16(it->ptr + 6), READ_BE_UINT16(it->ptr + 8), READ_BE_UINT16(it->ptr + 4));
				it->ptr += 10;
				if (READ_BE_UINT16(it->ptr) == 0xFFFF) {
					it->cmd = 0;
					rept = false;
				}
			}
			break;
		case 3:
			it->cmd |= 0x80;
			for (bool rept = true; rept; ) {
				_sr->loadToVRAM(s + READ_BE_INT16(it->ptr + 6), READ_BE_UINT16(it->ptr + 8), READ_BE_UINT16(it->ptr + 4));
				it->ptr += 10;
				if (READ_BE_UINT16(it->ptr) == 0xFFFF) {
					it->cmd = 0;
					rept = false;
				} else {
					it->ptr -= 4;
				}
			}
			break;
		case 4:
			it->cmd |= 0x80;
			a = it->progress & 7;
			b = (a + (((it->progress >> 3) * 5) & 7)) & 7;
			a = (a << 2) + offs[b];
			s += (a + READ_BE_INT16(it->ptr + 6));
			b = mask[b];
			a = READ_BE_UINT16(it->ptr + 8) >> 5;
			while (a--) {
				*s &= b;
				s += 32;
			}
			s = _tempBuffer + section * 0x2000;
			_sr->loadToVRAM(s + READ_BE_INT16(it->ptr + 6), READ_BE_UINT16(it->ptr + 8), READ_BE_UINT16(it->ptr + 4));
			if (++it->progress == 65)
				it->cmd = 0;
			break;
		case 5:
			if (it->progress == 0)
				memset(s, 0, 32);
			a = it->progress & 7;
			b = (a + (((it->progress >> 3) * 5) & 7)) & 7;
			a = (a << 2) + offs[b];
			b = (mask[b] ^ 0xFF) & 0x77;
			s[a] |= b;
			_sr->loadToVRAM(s, 32, 0);
			if (++it->progress == 65)
				it->cmd = 0;
			break;
		default:
			error("%s(): Invalid instruction", __FUNCTION__);
			break;
		}

		if (!loop)
			break;
	}
}

void Renderer_SCD::drawAnimSprites() {
	uint16 *d = _spriteBuffer;
	memset(d, 0, 4 * sizeof(uint16));
	uint16 nxt = 1;
	debug("%s():", __FUNCTION__);
	for (int i = 0; i < 64; ++i) {
		AnimObject &a = *_animations[i];
		if (a.enable && a.spriteData != nullptr && !(a.controlFlags & GraphicsEngine::kAnimHide) && a.fadeLevel != 0xFFFF && !(a.fadeLevel & _gfxState.frameCount())) {
			debug("		anim: %02d at x = %d, y = %d", i, a.posX >> 16, a.posY >> 16);
			generateSpriteData(a, nxt, d);
		} if (i == 31)
			updateAnim32Spec(a);
	}

	if (d - 3 > _spriteBuffer)
		*(reinterpret_cast<uint8*>(d) - 5) = 0;

	_sr->loadToVRAM(_spriteBuffer, MAX<uint>(4, d - _spriteBuffer) * sizeof(uint16), 0xFE00);
}

void Renderer_SCD::reconfigPlanes() {
	if (_modeChange == 0xFF)
		_mode = 1;
	else if (_modeChange)
		_mode = 2;

	static const uint16 dimCfgData[3][2] = {
		{512, 512},
		{256, 512},
		{512, 256}
	};

	static const uint16 memCfgData[3][5] = {
		{0xE000, 0xC000, 0xB000, 0xBE00, 0xB800},
		{0xE000, 0xC000, 0xF000, 0xFE00, 0xF800},
		{0xE000, 0xC000, 0xF000, 0xFE00, 0xF800}
	};

	_sr->setupPlaneAB(dimCfgData[_mode][0], dimCfgData[_mode][1]);

	const uint16 *in = memCfgData[_mode];
	_sr->setPlaneTableLocation(Graphics::SegaRenderer::kPlaneA, *in++);
	_sr->setPlaneTableLocation(Graphics::SegaRenderer::kPlaneB, *in++);
	_sr->setPlaneTableLocation(Graphics::SegaRenderer::kWindowPlane, *in++);
	_sr->setSpriteTableLocation(*in++);
	_sr->setHScrollTableLocation(*in);

	in = memCfgData[_mode];
	_sr->memsetVRAM(*in++, 0, 0x1000);
	_sr->memsetVRAM(*in++, 0, 0x1000);
	in += 2;
	_sr->memsetVRAM(*in, 0, 0x200);
	_sr->memsetVRAM(0, 0, _mode ? 0xC000 : 0xB000);
	
	_modeChange = 0;
}

void Renderer_SCD::updateScrollState() {
	if (!_trs->nextFrame())
		return;

	const TransitionManager::ScrollState &s = _trs->scroll_getState();

	if (s.hInt.needUpdate) {
		_sr->hINT_enable(s.hInt.enable);
		_sr->hINT_setCounter(s.hInt.counter);
		if (!s.hInt.enable)
			_sr->enableDisplay(true);
	}

	if (!s.disableVScroll) {
		_sr->writeUint16VSRAM(0, TO_BE_16(s.realOffsets[TransitionManager::kVertA] & 0x3FF));
		_sr->writeUint16VSRAM(2, TO_BE_16(s.realOffsets[TransitionManager::kVertB] & 0x3FF));
	}

	if (s.lineScrollMode) {
		for (int i = 0; i < s.hScrollTableNumEntries; ++i)
			_sr->writeUint16VRAM(0xF822 + i * 4, TO_BE_16(s.hScrollTable[i] & 0x3FF));
	} else {
		uint16 addr = _mode ? 0xF800 : 0xB800;
		_sr->writeUint16VRAM(addr, TO_BE_16(s.realOffsets[TransitionManager::kHorzA] & 0x3FF));
		_sr->writeUint16VRAM(addr + 2, TO_BE_16(s.realOffsets[TransitionManager::kHorzB] & 0x3FF));
	}
}

void Renderer_SCD::generateSpriteData(AnimObject &a, uint16 &spritesCurIndex, uint16 *&spritesBufferCurPos) {
	const uint16 *in = a.spriteData;
	uint16 num = FROM_BE_16(*in++) + 1;

	int32 curY = a.posY + 0x800000;
	int32 curX = a.posX + 0x800000;

	if (a.drawFlags & 4) {
		const TransitionManager::ScrollState &s = _trs->scroll_getState();
		curX += (s.unmodifiedOffsets[TransitionManager::kHorzA] << 16);
		curY += (s.unmodifiedOffsets[TransitionManager::kVertA] << 16);
	}

	if (a.drawFlags & 8) {
		const TransitionManager::ScrollState &s = _trs->scroll_getState();
		curX += (s.unmodifiedOffsets[TransitionManager::kHorzB] << 16);
		curY += (s.unmodifiedOffsets[TransitionManager::kVertB] << 16);
	}

	uint16 flip = (a.drawFlags & 3) << 11;
	uint16 p = ((a.f18 & 4) << 13) | ((a.f18 & 0xF8) >> 3);

	for (int i = 0; i < num && spritesCurIndex < 65; ++i) {
		int16 origY = FROM_BE_16(*in++);
		uint16 hw = FROM_BE_16(*in++);

		if (a.drawFlags & 2)
			origY = -origY - (((hw >> 5) & 0x18) + 8);

		uint16 tl = FROM_BE_16(*in++);
		int16 origX = FROM_BE_16(*in++);

		if (a.drawFlags & 1)
			origX = -origX - (((hw >> 7) & 0x18) + 8);

		tl = (tl | p) ^ flip;
		if (a.f16 & 0x80) {
			uint16 v = (a.f16 & 3) << 13;
			tl = (tl & 0x9FFF) | v;
		}

		*spritesBufferCurPos++ = TO_BE_16(origY + (curY >> 16));
		*spritesBufferCurPos++ = TO_BE_16(hw + (spritesCurIndex++));
		*spritesBufferCurPos++ = TO_BE_16(tl);
		*spritesBufferCurPos++ = TO_BE_16(origX + (curX >> 16));
	}
}

void Renderer_SCD::updateAnim32Spec(AnimObject &a) {
	int _animSpec_activate = 0;
	switch (_animSpec_activate) {
	case 0:

		break;
	case 1:
	case 2:
		break;
	default:
		break;
	}
}

bool Renderer_SCD::reachedAudioTimeStamp(AnimObject &a) const {
	return _gfxState.getAudioSync() >= a.timeStamp;
}

void Renderer_SCD::runAnimScript(AnimObject &a) {
	if (--a.frameDelay > 0) {
		if (a.scriptData[a.frame << 2] != 0x97)
			a.controlFlags &= ~GraphicsEngine::kAnimHide;
		return;
	}

	int incr = (a.f4f == 0) ? 1 : 0;

	while (incr != -1) {
		a.frame += incr;
		a.controlFlags &= ~GraphicsEngine::kAnimHide;
		const uint8 *in = a.scriptData() + (a.frame << 2);
		uint8 opcode = *in++;
		if (opcode & 0x80) {
			opcode -= 0x80;
			assert(opcode < _animProcs.size());
			if (_animProcs[opcode]->isValid())
				incr = (*_animProcs[opcode])(a, in);
			else
				error("%s(): Invalid opcode %d", __FUNCTION__, opcode);
		} else {
			incr = anim_updateFrameDelay(a, in);
		}
	}
}

void Renderer_SCD::makeAnimFunctions() {
#ifndef ANIM_DEBUG
	#define ANM(x) &Renderer_SCD::anim_##x
	typedef int (Renderer_SCD::*AnimFunc)(AnimObject&, const uint8*);
	static const AnimFunc funcTbl[] = {
#else
	#define ANM(x) {&Renderer_SCD::anim_##x, #x}
	struct FuncTblEntry {
		GfxAnimFunc::AnimFunc func;
		const char *desc;
	};
	static const FuncTblEntry funcTbl[] = {
#endif
		ANM(terminate),
		ANM(setFrame),
		ANM(seqSetFrame),
		ANM(rndSeqSetFrame),
		ANM(setX),
		ANM(setY),
		ANM(setSpeedX),
		ANM(setSpeedY),
		ANM(palEvent),
		ANM(palReset),
		ANM(copyCmds),
		ANM(nop),
		ANM(scrollSingleStepH),
		ANM(scrollSingleStepV),
		ANM(scrollStartH),
		ANM(scrollStartV),
		ANM(pause),
		ANM(pause),
		ANM(pause),
		ANM(pause),
		ANM(resumeAndUnsyncOther),
		ANM(terminateOther),
		ANM(pauseAndHideOther),
		ANM(hide),
		ANM(24),
		ANM(audioSync),
		ANM(audioSync),
		ANM(27),
		ANM(28),
		ANM(29),
		ANM(30),
		ANM(31),
		ANM(setDrawFlags),
		ANM(setDrawFlags),
		ANM(34),
		ANM(35),
		ANM(36),
		ANM(37),
		ANM(38),
		ANM(39),
		ANM(40),
		ANM(41),
		ANM(42),
		ANM(43),
		ANM(allowFrameDrop),
		ANM(45),
		ANM(46),
		ANM(47)
	};

	for (uint i = 0; i < ARRAYSIZE(funcTbl); ++i)
#ifndef ANIM_DEBUG
		_animProcs.push_back(new GfxAnimFunc(this, funcTbl[i]));
#else
		_animProcs.push_back(new GfxAnimFunc(this, funcTbl[i].func, funcTbl[i].desc));
#endif
#undef ANM
}

uint16 animRand(uint16 a, uint16 b) {
	return ((b * Util::rngMakeNumber()) >> 16) + a;
}

int Renderer_SCD::anim_terminate(AnimObject &a, const uint8 *data) {
	a.clear();
	return -1;
}

int Renderer_SCD::anim_setFrame(AnimObject &a, const uint8 *data) {
	a.frame = *data;
	return 0;
}

int Renderer_SCD::anim_seqSetFrame(AnimObject &a, const uint8 *data) {
	if (a.frameSeqCounter == *data++) {
		a.frameSeqCounter = 0;
		return 1;
	}
	++a.frameSeqCounter;
	a.frame = *++data;
	return 0;
}

int Renderer_SCD::anim_rndSeqSetFrame(AnimObject &a, const uint8 *data) {
	if (a.frameSeqCounter == 0)
		a.frameSeqCounter = animRand(2, (uint16)data[1] - data[0]) & 0xFF;
	if (a.frameSeqCounter == data[1]) {
		a.frameSeqCounter = 0;
		return 1;
	}
	++a.frameSeqCounter;
	a.frame = data[2];
	return 0;
}

int Renderer_SCD::anim_setX(AnimObject &a, const uint8 *data) {
	int16 xdiff = READ_BE_UINT16(data + 1) - (a.posX >> 16);
	a.frameDelay = -1;
	anim_setGroupParameter(a, 0, xdiff);
	return 1;
}

int Renderer_SCD::anim_setY(AnimObject &a, const uint8 *data) {
	int16 ydiff = READ_BE_UINT16(data + 1) - (a.posY >> 16);
	a.frameDelay = -1;
	anim_setGroupParameter(a, 1, ydiff);
	return 1;
}

int Renderer_SCD::anim_setSpeedX(AnimObject &a, const uint8 *data) {
	a.relSpeedX = (int32)((READ_BE_UINT16(data + 1) << 16) | (*data << 8));
	anim_setGroupParameter(a, 2);
	return 1;
}

int Renderer_SCD::anim_setSpeedY(AnimObject &a, const uint8 *data) {
	a.relSpeedY = (int32)((READ_BE_UINT16(data + 1) << 16) | (*data << 8));
	anim_setGroupParameter(a, 3);
	return 1;
}

int Renderer_SCD::anim_palEvent(AnimObject &a, const uint8 *data) {
	if (_gfxState.getVar(2))
		return 1;
	ResourcePointer r = a.scriptData + READ_BE_INT16(data + 1);
	_pal->enqueueEvent(r);
	_gfxState.setVar(3, 1);
	return 1;
}

int Renderer_SCD::anim_palReset(AnimObject &a, const uint8 *data) {
	_pal->clearEvents();
	return 1;
}

int Renderer_SCD::anim_copyCmds(AnimObject &a, const uint8 *data) {
	ResourcePointer r = a.scriptData + READ_BE_INT16(data + 1);
	return enqueueDrawCommands(r) ? 1 : -1;
}

int Renderer_SCD::anim_nop(AnimObject &a, const uint8 *data) {
	return 1;
}

int Renderer_SCD::anim_scrollSingleStepH(AnimObject &a, const uint8 *data) {
	_trs->doCommand(1);
	_trs->scroll_setSingleStep(*data ? TransitionManager::kHorzB : TransitionManager::kHorzA, READ_BE_INT16(data + 1));
	return 1;
}

int Renderer_SCD::anim_scrollSingleStepV(AnimObject &a, const uint8 *data) {
	_trs->doCommand(1);
	_trs->scroll_setSingleStep(*data ? TransitionManager::kVertB : TransitionManager::kVertA, READ_BE_INT16(data + 1));
	return 1;
}

int Renderer_SCD::anim_scrollStartH(AnimObject &a, const uint8 *data) {
	_trs->doCommand(2);
	_trs->scroll_setDirectionAndSpeed(*data ? TransitionManager::kHorzB : TransitionManager::kHorzA, READ_BE_INT16(data + 1));
	return 1;
}

int Renderer_SCD::anim_scrollStartV(AnimObject &a, const uint8 *data) {
	_trs->doCommand(2);
	_trs->scroll_setDirectionAndSpeed(*data ? TransitionManager::kVertB : TransitionManager::kVertA, READ_BE_INT16(data + 1));
	return 1;
}

int Renderer_SCD::anim_pause(AnimObject &a, const uint8 *data) {
	anim_setGroupParameter(a, 5);
	return 1;
}

int Renderer_SCD::anim_resumeAndUnsyncOther(AnimObject &a, const uint8 *data) {
	assert(*data < 64);
	anim_setGroupParameter(*_animations[*data], 4);
	return 1;
}

int Renderer_SCD::anim_terminateOther(AnimObject &a, const uint8 *data) {
	assert(*data < 64);
	_animations[*data]->clear();
	return 1;
}

int Renderer_SCD::anim_pauseAndHideOther(AnimObject &a, const uint8 *data) {
	assert(*data < 64);
	_animations[*data]->controlFlags = GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide;
	return 1;
}

int Renderer_SCD::anim_hide(AnimObject &a, const uint8 *data) {
	a.controlFlags |= GraphicsEngine::kAnimHide;
	return anim_updateFrameDelay(a, data);
}

int Renderer_SCD::anim_24(AnimObject &a, const uint8 *data) {
	a.f4e |= 2;
	return 1;
}

int Renderer_SCD::anim_audioSync(AnimObject &a, const uint8 *data) {
	a.timeStamp = data[0] << 24 | data[1] << 16 | data[2] << 8;
	a.controlFlags |= GraphicsEngine::kAnimAudioSync;
	return 1;
}

int Renderer_SCD::anim_27(AnimObject &a, const uint8 *data) {
	a.f4e |= 1;
	_gfxState.setVar(4, *data);
	return 1;
}

int Renderer_SCD::anim_28(AnimObject &a, const uint8 *data) {
	a.f4e |= 0x80;
	_gfxState.setVar(4, a.f4e);
	return 1;
}

int Renderer_SCD::anim_29(AnimObject &a, const uint8 *data) {
	return 1;
}

int Renderer_SCD::anim_30(AnimObject &a, const uint8 *data) {
	return 1;
}

int Renderer_SCD::anim_31(AnimObject &a, const uint8 *data) {
	return 1;
}

int Renderer_SCD::anim_setDrawFlags(AnimObject &a, const uint8 *data) {
	a.drawFlags = READ_BE_UINT16(data + 1);
	return 1;
}

int Renderer_SCD::anim_34(AnimObject &a, const uint8 *data) {
	a.f4f = 0;
	if (_gfxState.getVar(8) == 0)
		return 1;
	a.f4f = 1;
	return -1;
}

int Renderer_SCD::anim_35(AnimObject &a, const uint8 *data) {
	_gfxState.setVar(5, 1);
	return 1;
}

int Renderer_SCD::anim_36(AnimObject &a, const uint8 *data) {
	_gfxState.setFlag(1, 0);
	return 1;
}

int Renderer_SCD::anim_37(AnimObject &a, const uint8 *data) {
	static const uint8 cmd[] = { 0x25, 0x00 };
	ResourcePointer r(cmd, 0);
	enqueueDrawCommands(r);
	return 1;
}

int Renderer_SCD::anim_38(AnimObject &a, const uint8 *data) {
	return 1;
}

int Renderer_SCD::anim_39(AnimObject &a, const uint8 *data) {
	return 1;
}

int Renderer_SCD::anim_40(AnimObject &a, const uint8 *data) {
	//gfxDoCommand_D0(*data);
	return 1;
}

int Renderer_SCD::anim_41(AnimObject &a, const uint8 *data) {
	return 1;
	//return -1;
}

int Renderer_SCD::anim_42(AnimObject &a, const uint8 *data) {
	return 1;
}

int Renderer_SCD::anim_43(AnimObject &a, const uint8 *data) {
	return 1;
}

int Renderer_SCD::anim_allowFrameDrop(AnimObject &a, const uint8 *data) {
	a.allowFrameDrop = 1;
	return 1;
}

int Renderer_SCD::anim_45(AnimObject &a, const uint8 *data) {
	return 1;
}

int Renderer_SCD::anim_46(AnimObject &a, const uint8 *data) {
	_gfxState.setFlag(1, 1);
	return 1;
}

int Renderer_SCD::anim_47(AnimObject &a, const uint8 *data) {
	a.f1c = *data;
	return 1;
}

int Renderer_SCD::anim_updateFrameDelay(AnimObject &a, const uint8 *data) {
	if (*data++) {
		uint16 v1 = *data++;
		uint16 v2 = *data - v1;
		a.frameDelay = animRand(v1, v2);
	} else {
		a.frameDelay = READ_BE_INT16(data);
	}
	return -1;
}

void Renderer_SCD::anim_setGroupParameter(AnimObject &a, int para, int16 val, bool recursive) {
	AnimObject *ta = &a;
	for (int n = ta->children; n || !recursive; n = recursive ? ta->next : 0) {
		if (recursive)
			ta = _animations[n];
		switch (para) {
		case 0:
			ta->posX += (val << 16);
			break;
		case 1:
			ta->posY += (val << 16);
			break;
		case 2:
			a.absSpeedX = a.relSpeedX;
			if (ta->parent)
				a.absSpeedX += _animations[ta->parent]->absSpeedX;
			break;
		case 3:
			a.absSpeedY = a.relSpeedY;
			if (ta->parent)
				a.absSpeedY += _animations[ta->parent]->absSpeedY;
			break;
		case 4:
			ta->controlFlags &= GraphicsEngine::kAnimHide;
			break;
		case 5:
			ta->controlFlags |= GraphicsEngine::kAnimPause;
			break;
		case 6:
			ta->controlFlags = GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide;
			break;
		default:
			error("%s(): unhandled para %d", __FUNCTION__, para);
			break;

		}
		recursive = true;
		anim_setGroupParameter(*ta, para, val, true);
	}
}

void Renderer_SCD::createMouseCursor() {
	// This will obviously only work when the necessary animation has been loaded.
	anim_setControlFlags(16, GraphicsEngine::kAnimPause);
	anim_setFrame(16, 0);
	
	memset(_tempBuffer, 0, 0x10000);
	updateAnimations();
	drawAnimSprites();
	_sr->renderSprites(_tempBuffer, nullptr);
	anim_setControlFlags(16, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);

	for (int i = 1; i < 24; ++i)
		memcpy(_tempBuffer + i * 24, _tempBuffer + i * _screenWidth, 24);

	g_system->setMouseCursor(_tempBuffer, 24, 24, 12, 12, 4);
	g_system->setCursorPalette(_pal->getSystemPalette(), 0, 64);
	g_system->showMouse(true);
}

// Boot logo sequence code. This has nothing to do with the engine graphics code, so I put it at the end, to keep the ugly mess out of sight...
int Renderer_SCD::drawBootLogoFrame(uint8 *screen, int frameNo)  {
	uint16 *tmp = reinterpret_cast<uint16*>(_tempBuffer);
	uint32 len = 0;
	uint16 val = 0;
	uint8 r = 0;
	uint8 g = 0;
	uint8 b = 0;

	static const uint16 colors[] = {
		0x0000, 0x0000, 0x0020, 0x0040, 0x0060, 0x0080, 0x00A0
	};

	switch (frameNo) {
	case 0:
		_mode = 0;
		reconfigPlanes();
		len = READ_BE_UINT16(StaticRes_SCD::_bootSeqData[0]);
		if (len != Util::decodeSCDData(StaticRes_SCD::_bootSeqData[0] + 2, _tempBuffer))
			error("%s(): Decode size mismatch", __FUNCTION__);
		_sr->loadToVRAM(_tempBuffer, len, 0x000);
		len = READ_BE_UINT16(StaticRes_SCD::_bootSeqData[1]);
		if (len != Util::decodeSCDData(StaticRes_SCD::_bootSeqData[1] + 2, _tempBuffer))
			error("%s(): Decode size mismatch", __FUNCTION__);
		_sr->loadToVRAM(_tempBuffer, len, 0x2000);
		_pal->selectPalettes(0x06060606);

		for (int i = 0; i < 107; ++i)
			tmp[i] = TO_BE_16(StaticRes_SCD::_bootSeqData[4][i] | 0x100);

		_sr->loadToVRAM(tmp, 22, 0xE294);
		_sr->loadToVRAM(&tmp[12], 4, 0xE39C);
		_sr->loadToVRAM(&tmp[14], 60, 0xE482);
		_sr->loadToVRAM(&tmp[44], 58, 0xE704);
		_sr->loadToVRAM(&tmp[74], 22, 0xEA94);
		_sr->loadToVRAM(&tmp[86], 42, 0xEB8C);

		if (++_bootsDelay == 180) {
			_bootsDelay = 0;
			++frameNo;
		}
		break;

	case 1:
		++frameNo;
		break;

	case 2:
		_pal->selectPalettes(0x04040405);
		for (int i = 0; i < 7; ++i)
			_pal->setColor(50 + i, (colors[i] >> 8) & 0x0F, (colors[i] >> 4) & 0x0F, colors[i] & 0x0F);
		len = READ_BE_UINT16(StaticRes_SCD::_bootSeqData[3]);
		if (len != Util::decodeSCDData(StaticRes_SCD::_bootSeqData[3] + 2, _tempBuffer))
			error("%s(): Decode size mismatch", __FUNCTION__);
		_sr->loadToVRAM(_tempBuffer, len, 0xC000);
		_sr->enableDisplay(false);
		if (++_bootsDelay == 2) {
			_bootsDelay = 0;
			++frameNo;
		}
		break;

	case 3:
		_bootsSprTbl = new uint8[128]();
		len = READ_BE_UINT16(StaticRes_SCD::_bootSeqData[2]);
		if (len != Util::decodeSCDData(StaticRes_SCD::_bootSeqData[2] + 2, _tempBuffer))
			error("%s(): Decode size mismatch", __FUNCTION__);
		_sr->loadToVRAM(_tempBuffer, len, 0xE000);
		val = TO_BE_16(0x6001);
		for (int i = 0; i < 3072; i += 2)
			_sr->writeUint16VRAM(0xE140 + i, val);
		val = TO_BE_16(0x6002);
		for (int i = 0; i < 64; i += 2)
			_sr->writeUint16VRAM(0xE0C0 + i, val);
		val = TO_BE_16(0x6003);
		for (int i = 0; i < 64; i += 2)
			_sr->writeUint16VRAM(0xE140 + i, val);
		_sr->writeUint16VRAM(0xE080, 0x6002);
		_sr->writeUint16VRAM(0xE100, 0x6003);
		WRITE_BE_UINT16(_bootsSprTbl, 0x0088);
		WRITE_BE_UINT16(_bootsSprTbl + 2, 0x0500);
		WRITE_BE_UINT16(_bootsSprTbl + 4, 0xE004);
		WRITE_BE_UINT16(_bootsSprTbl + 6, 0x0080);
		_sr->enableDisplay(true);
		++frameNo;
		break;

	case 4:
		_bootsSprTbl[5] = ((((_bootsTotalFrames >> 1) & 3) + 1) << 2);
		WRITE_BE_UINT16(_bootsSprTbl + 6, READ_BE_UINT16(_bootsSprTbl + 6) + 6);
		_bootsHScroll = (_bootsHScroll + 6) & 0x1FF;
		if (_bootsHScroll > 0x100) {
			_bootsHScroll = 0x100;
			_bootsDelay = 16;
			_bootsCol = 5;
			++frameNo;
		}
		break;

	case 5:
		if (--_bootsDelay < 0) {
			++frameNo;
		} else if (!(_bootsDelay & 7)) {
			for (int i = _bootsCol; i >= 0; --i)
				_pal->adjustColor(57, 0, 2, 0);
			++_bootsCol;
		}
		break;

	case 6:
		_bootsVScroll = (_bootsVScroll - 2) & 0x3FF;
		if (_bootsVScroll <= 0x318) {
			_bootsCol = 15;
			_pal->selectPalettes(0xFF01FFFF);
			_pal->setColor(16, 0, 0, 0);
			++frameNo;
		}
		break;

	case 7:
		if (_bootsTotalFrames & 1)
			break;

		for (int i = 0; i < 16 - _bootsCol; ++i) {
			_pal->getPresetColor(1, 6 + i, r, g, b);
			_pal->setColor(16 + _bootsCol + i, r, g, b);
			_pal->getPresetColor(2, 6 + i, r, g, b);
			_pal->setColor(32 + _bootsCol + i, r, g, b);
		}

		if (--_bootsCol == 5) {
			_bootsCol = 15;
			_pal->selectPalettes(0x00FFFFFF);
			_pal->setColor(0, 0, 0, 0);
			++frameNo;
		}

		break;

	case 8:
		if (_bootsTotalFrames & 1)
			break;

		for (int i = 0; i < 16 - _bootsCol; ++i) {
			_pal->getPresetColor(0, 6 + i, r, g, b);
			_pal->setColor(_bootsCol + i, r, g, b);
		}

		if (--_bootsCol == 5) {
			_bootsDelay = 3;
			++frameNo;
		}

		break;

	case 9:
		if (--_bootsDelay == 0) {
			_bootsDelay = 3;
			_pal->adjustColor(0, 2, 2, 2);
			_pal->getColor(0, r, g, b);
			if (r == 0x0E && g == 0x0E && b == 0x0E) {
				_pal->selectPalettes(0x00010203);
				_bootsDelay = 512;
				++frameNo;
			}
		}
		break;

	case 10:
		if (--_bootsDelay == 0) {
			_bootsDelay = 40;
			memset(_bootsSprTbl, 0, 8);
			++frameNo;
		}
		break;

	case 11:
		if (--_bootsDelay == 0) {
			setPlaneMode(0xFF);
			delete[] _bootsSprTbl;
			_bootsSprTbl = nullptr;
			frameNo = -1;
		} else if (!(_bootsDelay & 1)) {
			for (int i = 0; i < 64; ++i) {
				_pal->getColor(0, r, g, b);
				if (r >= 2)
					r -= 2;
				if (g >= 2)
					g -= 2;
				if (b >= 2)
					b -= 2;
				_pal->setColor(i, r, g, b);
			}
		}
		break;

	default:
		break;
	}

	++_bootsTotalFrames;

	_sr->writeUint16VSRAM(0, TO_BE_16(_bootsVScroll & 0x3FF));
	_sr->writeUint16VRAM(0xB800, TO_BE_16(_bootsHScroll & 0x3FF));
	if (_bootsSprTbl)
		_sr->loadToVRAM(_bootsSprTbl, 64 * sizeof(uint16), 0xBE00);

	_pal->updateSystemPalette();
	_sr->setRenderColorTable(_pal->getSystemPalette(), 0, 64);
	_sr->render(screen);

	return frameNo;	
}


Renderer *Renderer::createSegaRenderer(const Graphics::PixelFormat *pxf, GraphicsEngine::GfxState &state, Palette *pal, TransitionManager *scr) {
	return new Renderer_SCD(pxf, state, pal, scr);
}

} // End of namespace Snatcher
