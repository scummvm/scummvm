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

#include "dgds/head.h"
#include "dgds/dgds.h"
#include "dgds/image.h"
#include "dgds/includes.h"
#include "dgds/sound_raw.h"
#include "dgds/drawing.h"
#include "dgds/scene.h"
#include "dgds/dialog.h"

#include "graphics/cursorman.h"

namespace Dgds {

void TalkDataHead::drawHead(Graphics::ManagedSurface *dst, const TalkData &data) const {
	uint drawtype = _drawType ? _drawType : 1;
	// Use specific head shape if available (eg, in Willy Beamish), if not use talk data shape
	Common::SharedPtr<Image> img = _shape;
	if (!img)
		img = data._shape;
	if (!img)
		return;
	switch (drawtype) {
	case 1:
		drawHeadType1(dst, *img);
		break;
	case 2:
		drawHeadType2(dst, *img);
		break;
	case 3:
		if (DgdsEngine::getInstance()->getGameId() == GID_WILLY)
			drawHeadType3Beamish(dst, data);
		else
			drawHeadType3(dst, *img);
		break;
	default:
		error("Unsupported head draw type %d", drawtype);
	}
}

void TalkDataHead::drawHeadType1(Graphics::ManagedSurface *dst, const Image &img) const {
	Common::Rect r = _rect.toCommonRect();
	dst->fillRect(r, _drawCol);
	r.grow(-1);
	dst->fillRect(r, _drawCol == 0 ? 15 : 0);
	r.left += 2;
	r.top += 2;
	const int x = _rect.x;
	const int y = _rect.y;
	if (img.isLoaded()) {
		for (const auto &frame : _headFrames) {
			img.drawBitmap(frame._frameNo & 0xff, x + frame._xoff, y + frame._yoff, r, *dst);
		}
	}
}

void TalkDataHead::drawHeadType2(Graphics::ManagedSurface *dst, const Image &img) const {
	if (!img.isLoaded())
		return;
	const Common::Rect r = _rect.toCommonRect();
	for (const auto &frame : _headFrames) {
		img.drawBitmap(frame._frameNo & 0xff, r.left + frame._xoff, r.top + frame._yoff, r, *dst);
	}
}

void TalkDataHead::drawHeadType3Beamish(Graphics::ManagedSurface *dst, const TalkData &data) const {
	const Common::Rect r = _rect.toCommonRect();

	// Note: only really need the 1px border here but just fill the box.
	dst->fillRect(r, 8);

	Common::Rect fillRect(r);
	fillRect.grow(-1);
	dst->fillRect(fillRect, _drawCol);

	for (const auto &frame : _headFrames) {
		int frameNo = frame._frameNo & 0x7fff;
		bool useHeadShape = frame._frameNo & 0x8000;

		Common::SharedPtr<Image> img = useHeadShape ? _shape : data._shape;
		if (!img || !img->isLoaded() || frameNo >= img->loadedFrameCount())
			continue;

		ImageFlipMode flip = kImageFlipNone;
		// Yes, the numerical values are reversed here (1 -> 2 and 2 -> 1).
		// The head flip flags are reversed from the image draw flags.
		if (frame._flipFlags & 1)
			flip = static_cast<ImageFlipMode>(flip & kImageFlipH);
		if (frame._flipFlags & 2)
			flip = static_cast<ImageFlipMode>(flip & kImageFlipV);

		img->drawBitmap(frameNo, r.left + frame._xoff, r.top + frame._yoff, fillRect, *dst);
	}
}

void TalkDataHead::drawHeadType3(Graphics::ManagedSurface *dst, const Image &img) const {
	Common::Rect r = _rect.toCommonRect();
	dst->fillRect(r, 0);
	if (!img.isLoaded())
		return;
	for (const auto &frame : _headFrames) {
		int frameNo = frame._frameNo;
		if (frameNo < img.loadedFrameCount())
			img.drawBitmap(frameNo, r.left + frame._xoff, r.top + frame._yoff, r, *dst);
		else
			dst->fillRect(r, 4);
	}
}

void TalkDataHead::updateHead() {
	warning("TODO: Update head");
	_flags = static_cast<HeadFlags>(_flags & ~(kHeadFlag1 | kHeadFlag8 | kHeadFlag10 | kHeadFlagVisible));

	/* This seems to just be a "needs redraw" flag, but we always redraw
	for (auto tds : _talkData) {
		for (auto h : tds._heads) {
			if ((h._flags & kHeadFlagVisible) && !(h._flags & (kHeadFlag8 | kHeadFlag10 | kHeadFlag80))) {
				if (h._rect.toCommonRect().intersects(head._rect.toCommonRect())) {
					h._flags = static_cast<HeadFlags>(h._flags | kHeadFlag4);
				}
			}
		}
	}
	*/
}


void TalkData::updateVisibleHeads() {
	for (auto &head : _heads) {
		if (head._flags & kHeadFlagVisible)
			head.updateHead();
	}
}

void TalkData::drawVisibleHeads(Graphics::ManagedSurface *dst) const {
	for (const auto &h : _heads) {
		if ((h._flags & kHeadFlagVisible) && !(h._flags & kHeadFlag40)) {
			h.drawHead(dst, *this);
		}
	}
}

bool TalkData::hasVisibleHead() const {
	for (const auto &h : _heads) {
		if (h._flags & kHeadFlagVisible)
			return true;
	}
	return false;
}

CDSTTMInterpreter::CDSTTMInterpreter(DgdsEngine *vm) : TTMInterpreter(vm) {
	_storedAreaBuffer.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
}


void CDSTTMInterpreter::handleOperation(TTMEnviro &env_, TTMSeq &seq, uint16 op, byte count, const int16 *ivals, const Common::String &sval, const Common::Array<Common::Point> &pts) {
	CDSTTMEnviro &env = static_cast<CDSTTMEnviro &>(env_);

	switch (op) {
	case 0x0080: // FREE SHAPE
		env._scriptShapes[0].reset();
		break;
	case 0x0110: // PURGE void
		break;
	case 0x0FF0: // REFRESH
		break;
	case 0x1020: { // SET DELAY:	    i:int   [0..n]
		// TODO: Probably should do this accounting (as well as timeCut and dialogs)
		// 		 in game frames, not millis.
		int delayMillis = (int)round(ivals[0] * MS_PER_FRAME);
		env._cdsDelay = delayMillis;
		break;
	}
	case 0x1050: // SELECT BMP:  id:int [0]
		seq._currentBmpId = ivals[0];
		break;
	case 0x1060: // SELECT PAL:  id:int [0]
		seq._currentPalId = ivals[0];
		break;
	case 0x1100: // SET_SCENE:  i:int   [1..n]
	case 0x1110: // SET_SCENE:  i:int   [1..n]
		break;
	case 0x2000: // SET (DRAW) COLORS: fgcol,bgcol:int [0..255]
		seq._drawColFG = static_cast<byte>(ivals[0]); // aka Line Color
		seq._drawColBG = static_cast<byte>(ivals[1]); // aka Fill Color
		break;
	case 0x3200: // CDS FIND GOTO TARGET frameno
		seq._gotoFrame = findGOTOTarget(env, seq, ivals[0]);
		break;
	case 0x3300: // CDS GOSUB - first 2 args are ignored by original
			if (!env._cdsJumped && seq._gotoFrame + ivals[2] != seq._currentFrame && seq._gotoFrame >= 0) {
			env._cdsJumped = true;
			int64 prevPos = env.scr->pos();
			int16 currentFrame = seq._currentFrame;
			env.scr->seek(env._frameOffsets[seq._gotoFrame + ivals[2]]);
			seq._currentFrame = seq._gotoFrame;
			run(env, seq);
			seq._currentFrame = currentFrame;
			env.scr->seek(prevPos);
			env._cdsJumped = false;
		}
		break;
	case 0x4200: { // STORE AREA: x,y,w,h:int [0..n]  ; makes this area of foreground persist in the next frames.
		if (env._cdsDidStoreArea) // this is a one-shot op
			break;

		if (env._storedAreaRect.width * 2 < (int)ivals[2]) {
			env._storedAreaRect = DgdsRect(ivals[0] + env._xOff, ivals[1] + env._yOff, ivals[2], ivals[3]);
		}
		const Common::Rect rect = env._storedAreaRect.toCommonRect();
		_storedAreaBuffer.blitFrom(_vm->_compositionBuffer, rect, rect);
		env._cdsDidStoreArea = true;
		break;
	}
	case 0xa500: // DRAW SPRITE: x,y,frameno,bmpno:int [-n,+n]
	case 0xa510: // DRAW SPRITE FLIP V x,y:int
	case 0xa520: // DRAW SPRITE FLIP H: x,y:int
	case 0xa530: // DRAW SPRITE FLIP HV: x,y,frameno,bmpno:int	[-n,+n]
		doDrawSpriteOp(env, seq, op, count, ivals, env._xOff, env._yOff);
		break;
	case 0xc220: // PLAY RAW SFX
		if (env._cdsPlayedSound) // this is a one-shot op
			break;
		if (!env._soundRaw) {
			warning("TODO: Trying to play raw SFX but nothing loaded");
		} else {
			env._soundRaw->play();
			env._cdsPlayedSound = true;
		}
		break;
	case 0xc250: {	// SYNC RAW SFX
		uint16 hi = (uint16)ivals[1];
		uint16 lo = (uint16)ivals[0];
		uint32 offset = ((uint32)hi << 16) + lo;
		debug("TODO: 0xC250 Sync raw sfx?? offset %d", offset);
		/*if (env._soundRaw->playedOffset() < offset) {
			// Not played to this point yet.
			env.scr->seek(-6, SEEK_CUR);
			return false;
		}*/
		break;
	}
	case 0xa100: // DRAW FILLED RECT
	case 0xa110: // DRAW EMPTY RECT  x1,y1,x2,y2:int
		// Appear in the scripts but not implemented in the original.
		break;
	case 0xc200: // ??? SFX: ??,?? - not implemented in willy, ignore?
	case 0xc210: // LOAD RAW SFX: filename:str
	case 0xf010: // LOAD SCR:	filename:str
	case 0xf020: // LOAD BMP:	filename:str
	case 0xf050: // LOAD PAL:	filename:str
		// Ignore all these in CDS scripts.  The original implements them but only to load
		// patch data outside of the CDS files.  That data isn't present in the final
		// games so we can just ignore the opcode.
		break;
	default:
		if (count < 15)
			warning("Unimplemented CDS TTM opcode: 0x%04X (%s, %d args) (ivals: %d %d %d %d)",
					op, ttmOpName(op), count, ivals[0], ivals[1], ivals[2], ivals[3]);
		else
			warning("Unimplemented CDS TTM opcode: 0x%04X (%s, sval: %s)", op,
					ttmOpName(op), sval.c_str());
		break;
	}
}


Conversation::~Conversation() {
	unloadData();
}

void Conversation::unloadData() {
	debug(10, "CDS: Unloading data (%d,%d)", _dlgNum, _dlgFileNum);
	_ttmScript.reset();

	_ttmEnv._scriptShapes[0].reset();
	if (_ttmEnv._soundRaw)
		_ttmEnv._soundRaw->stop();
	_ttmEnv = CDSTTMEnviro();
	_loadState = 0;
}

void Conversation::clear() {
	_dlgNum = -1;
	_dlgFileNum = -1;
	_subNum = -1;
	_finished = false;
	_haveHeadData = false;
	_stopScript = false;
}

bool Conversation::isForDlg(const Dialog *dlg) const {
	return dlg && dlg->_num == _dlgNum && dlg->_fileNum == _dlgFileNum;
}

void Conversation::loadData(uint16 dlgFileNum, uint16 dlgNum, int16 sub, bool haveHeadData) {
	unloadData();
	clear();

	DgdsEngine *engine = DgdsEngine::getInstance();

	// These files are only present in Willy Beamish CD version
	if (engine->getGameId() != GID_WILLY)
		return;

	_dlgNum = dlgNum;
	_dlgFileNum = dlgFileNum;
	_subNum = sub;
	_nextExecMs = 0;
	_runTempFrame = 0;
	_tempFrameNum = 0;
	_thisFrameMs = 0;
	_stopScript = false;
	_haveHeadData = haveHeadData;

	if (!haveHeadData)
		_drawRect = DgdsRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	ResourceManager *resourceManager = engine->getResourceManager();
	Decompressor *decompressor = engine->getDecompressor();

	Common::String fname;
	if (sub >= 0) {
		assert(sub < 26);
		fname = Common::String::format("F%dB%d%c.CDS", dlgFileNum, dlgNum, 'A' + sub);
	} else {
		fname = Common::String::format("F%dB%d.CDS", dlgFileNum, dlgNum);
	}

	if (!resourceManager->hasResource(fname))
		return;

	debug(10, "CDS: Load CDS resource %s", fname.c_str());

	// The scripts are designed so the resources are patchable, but by default
	// they use the sound and image data from the CDS file.
	_ttmEnv._soundRaw.reset(new SoundRaw(resourceManager, decompressor));
	_ttmEnv._soundRaw->load(fname);
	_ttmEnv._scriptShapes[0].reset(new Image(resourceManager, decompressor));
	_ttmEnv._scriptShapes[0]->loadBitmap(fname);
	_ttmScript.reset(new CDSTTMInterpreter(engine));
	_ttmScript->load(fname, _ttmEnv);
	_ttmScript->findAndAddSequences(_ttmEnv, _ttmSeqs);

	_loadState = 1;

	// Then run START
	for (const auto &tag : _ttmEnv._tags) {
		if (tag._value.equalsIgnoreCase("START"))
			_ttmEnv._cdsFrame = tag._key;
	}
	_tempFrameNum = _ttmScript->findGOTOTarget(_ttmEnv, *_ttmSeqs[0], _ttmEnv._cdsFrame);
	_ttmEnv._cdsFrame = _tempFrameNum;

	// Always run frame 1 on init.
	runScriptFrame(1);
}

bool Conversation::runScriptFrame(int16 frameNum) {
	if (!_ttmScript || frameNum >= _ttmEnv._totalFrames)
		return false;

	_ttmEnv.scr->seek(_ttmEnv._frameOffsets[frameNum]);
	TTMSeq *seq = _ttmSeqs[0].get();
	seq->_currentFrame = frameNum;

	debug(10, "CDS: Running TTM sequence %d frame %d", seq->_seqNum, seq->_currentFrame);

	seq->_drawWin = _drawRect.toCommonRect();
	return _ttmScript->run(_ttmEnv, *seq);
}

void Conversation::checkAndRunScript() {
	if (!_ttmScript || _finished)
		return;

	DgdsEngine *engine = DgdsEngine::getInstance();

	// Always add the stored buffer first.  Copy the whole thing to overwrite other state.
	engine->_compositionBuffer.transBlitFrom(_ttmScript->getStoredAreaBuffer());

	if (_runTempFrame) {
		runScriptFrame(_tempFrameNum);
	}
	runScriptFrame(_ttmEnv._cdsFrame);
	if (_ttmEnv._cdsDelay > 0) {
		_nextExecMs = _thisFrameMs + _ttmEnv._cdsDelay;
		_ttmEnv._cdsDelay = -1;
	} else {
		_nextExecMs = 0;
	}
}

void Conversation::incrementFrame() {
	if (!_ttmScript)
		return;

	// TODO: check load type 2 here?

	if (_loadState == 1 && _ttmEnv._scriptShapes[0] && _ttmEnv._scriptShapes[0]->loadedFrameCount() > 0) {
		_ttmEnv._storedAreaRect.x = _drawRect.x;
		_ttmEnv._storedAreaRect.y = _drawRect.y;
		_ttmEnv._storedAreaRect.width = _ttmEnv._scriptShapes[0]->getFrames()[0]->w + 8;
		_ttmEnv._storedAreaRect.height = _ttmEnv._scriptShapes[0]->getFrames()[0]->h;
		_loadState = 2;
	}

	if (_runTempFrame)
		_runTempFrame--;

	if (!_nextExecMs || _thisFrameMs >= _nextExecMs) {
		debug(10, "CDS: Increment frame %d -> %d", _ttmEnv._cdsFrame, _ttmEnv._cdsFrame + 1);
		_ttmEnv._cdsFrame++;
	}
}

bool Conversation::isScriptRunning() {
	return (_ttmScript &&
		((_ttmEnv._soundRaw && _ttmEnv._soundRaw->isPlaying())
		||
		(_ttmEnv._cdsFrame < _ttmEnv._totalFrames)
		));
}

void Conversation::pumpMessages() {
	Common::Event ev;
	DgdsEngine *engine = DgdsEngine::getInstance();

	while (engine->getEventManager()->pollEvent(ev)) {
		if (ev.type == Common::EVENT_LBUTTONDOWN ||
			ev.type == Common::EVENT_RBUTTONDOWN ||
			ev.type == Common::EVENT_KEYDOWN) {
			_stopScript = true;
			engine->getScene()->setIgnoreMouseUp();
		}
	}
}

void Conversation::runScript() {
	if (!_ttmScript)
		return;

	DgdsEngine *engine = DgdsEngine::getInstance();
	engine->disableKeymapper();

	_nextExecMs = 0;
	_ttmEnv._xOff = _drawRect.x;
	_ttmEnv._yOff = _drawRect.y;
	_runTempFrame = 2;

	if (_drawRect.width == 0) {
		_drawRect.width = SCREEN_WIDTH;
		_drawRect.height = SCREEN_HEIGHT;
	}

	int frameCount = 0;
	uint32 startMillis = g_system->getMillis();

	DgdsEngine::dumpFrame(engine->_compositionBuffer, "cds-comp-before-script");

	engine->getScene()->checkDialogActive();
	// update here to make sure the dialog gets drawn.
	engine->getScene()->drawAndUpdateDialogs(&engine->_compositionBuffer);

	DgdsEngine::dumpFrame(engine->_compositionBuffer, "cds-comp-before-script-with-dlg");

	_ttmScript->getStoredAreaBuffer().blitFrom(engine->_compositionBuffer);

	CursorMan.showMouse(false);

	g_system->copyRectToScreen(engine->_compositionBuffer.getPixels(), SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_system->updateScreen();

	// If we have head data we should dim the palette
	engine->dimPalForWillyDialog(_haveHeadData);

	debug(10, "CDS: Start execution of (%d,%d) - frame %d maxFrame %d", _dlgNum, _dlgFileNum,
		_ttmEnv._cdsFrame, _ttmEnv._totalFrames);

	while (isScriptRunning() && !engine->shouldQuit()) {
		_thisFrameMs = g_system->getMillis();
		pumpMessages();
		if (_stopScript)
			break;

		if (!_nextExecMs || _nextExecMs <= _thisFrameMs) {
			incrementFrame();

			const Common::Rect r = _drawRect.toCommonRect();

			checkAndRunScript();

			const byte *srcPtr = (const byte *)engine->_compositionBuffer.getPixels() + r.top * SCREEN_WIDTH + r.left;
			g_system->copyRectToScreen(srcPtr, SCREEN_WIDTH, r.left, r.top, r.width(), r.height());
		}

		g_system->updateScreen();

		frameCount++;
		// Limit to 15 FPS
		static const int framesPerSecond = 15;
		uint32 thisFrameEndMillis = g_system->getMillis();
		uint32 elapsedMillis = thisFrameEndMillis - startMillis;
		const uint32 targetMillis = (frameCount * 1000 / framesPerSecond);
		if (targetMillis > elapsedMillis) {
			while (targetMillis > elapsedMillis) {
				pumpMessages();
				g_system->updateScreen();
				g_system->delayMillis(5);
				elapsedMillis = g_system->getMillis() - startMillis;
			}
		} else if (targetMillis < elapsedMillis) {
			startMillis = thisFrameEndMillis;
			frameCount = 0;
		}
	}

	debug(10, "CDS: Finished execution of (%d,%d) - stop %s frame %d maxFrame %d", _dlgNum, _dlgFileNum,
		_stopScript ? "true" : "false", _ttmEnv._cdsFrame, _ttmEnv._totalFrames);

	CursorMan.showMouse(true);
	engine->enableKeymapper();
	if (_ttmEnv._soundRaw)
		_ttmEnv._soundRaw->stop();

	// If the dialog was cleared, set the force-clear flag in the scene.  Otherwise, just set
	// a flag the scene can check when updating dialogs.
	if (_stopScript)
		engine->getScene()->setShouldClearDlg();
	else
		_finished = true;

	unloadData();
}

} // end namespace Dgds
