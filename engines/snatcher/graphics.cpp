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


#include "snatcher/graphics.h"
#include "snatcher/palette.h"
#include "snatcher/animator.h"
#include "snatcher/resource.h"
#include "snatcher/text.h"
#include "snatcher/transition.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/pixelformat.h"

namespace Snatcher {

GraphicsEngine::GraphicsEngine(const Graphics::PixelFormat *pxf, OSystem *system, Common::Platform platform, const VMInfo &vmstate, SoundEngine *snd, bool enableAspectRatioCorrection) : _system(system), _state(vmstate),
	_animator(nullptr), _text(nullptr), _dataMode(0), _screen(nullptr), _bpp(pxf ? pxf->bytesPerPixel : 1), _flags(0), _verbAreaType(0) {
	assert(system);
	_palette = Palette::create(pxf, _system->getPaletteManager(), platform, _state);
	assert(_palette);
	_trs = TransitionManager::create(platform, _state);
	_animator = Animator::create(pxf, platform, _state, _palette, _trs, snd, enableAspectRatioCorrection);
	_text = TextRenderer::create(platform, _animator);
	assert(_animator);
	_screen = new uint8[_animator->realScreenWidth() * _animator->realScreenHeight() * (pxf ? pxf->bytesPerPixel : 1)]();
	assert(_screen);
	_animSaveLoadData = new uint8[64]();
	assert(_animSaveLoadData);
}

GraphicsEngine::~GraphicsEngine() {
	delete _animator;
	delete _palette;
	delete _trs;
	delete[] _screen;
	delete _text;
	delete[] _animSaveLoadData;
}

void GraphicsEngine::runScript(ResourcePointer res, int func) {
	ResourcePointer sc = res.getDataFromTable(func);

	for (uint8 cmd = *sc++; cmd != 0xFF; cmd = *sc++) {
		uint8 len = *sc++;
		ResourcePointer next = sc + len;

		switch (cmd) {
		case 0:
			assert(_palette);
			_palette->enqueueEvent(sc);
			break;
		case 1:
			_state.clearFlag(1, 0);
			_animator->initData(sc, _dataMode);
			_state.setVar(8, 1);
			break;
		case 2:
			_animator->initAnimations(sc, len, true);
			break;
		case 3:
		case 4:
			_animator->linkAnimations(sc, len);
			break;

		case 5:
		case 6:
		case 7:
			_verbAreaType = sc[1];
			break;
		case 8:
			_animator->setPlaneMode(sc.readUINT16());
			break;
		case 9:
			_dataMode = sc[1];
			// Multi part data transfer for more than 2048 bytes. This is just due to original hardware limitations
			// (in particular, the dma transfer from word ram to vram). We can just copy the whole thing at once...
			break;
		default:
			error("%s(): Unknown opcode 0x%02x", __FUNCTION__, cmd);
		}

		sc = next;
	}
}

void GraphicsEngine::enqueuePaletteEvent(ResourcePointer res) {
	_palette->enqueueEvent(res);
}

bool GraphicsEngine::enqueueDrawCommands(ResourcePointer res) {
	return _animator->enqueueDrawCommands(res);
}

void GraphicsEngine::initAnimations(ResourcePointer res, uint16 len, bool dontUpdate) {
	_animator->initAnimations(res, len, dontUpdate);
}

void GraphicsEngine::setScrollStep(uint8 mode, int16 step) {
	if (mode < 4)
		_trs->scroll_setDirectionAndSpeed(mode, step);
	else
		_trs->scroll_setSingleStep(mode & 3, step);
}

void GraphicsEngine::transitionCommand(uint8 cmd) {
	_trs->doCommand(cmd);
}

bool GraphicsEngine::transitionStateBusy() const {
	return _trs->scroll_getState().busy;
}

void GraphicsEngine::setTextFont(const uint8 *font, uint32 fontSize, const uint8 *charWidthTable, uint32 charWidthTableSize) {
	_text->setFont(font, fontSize);
	_text->setCharWidthTable(charWidthTable, charWidthTableSize);
}

void GraphicsEngine::printText(const uint8 *text) {
	_text->enqueuePrintJob(text);
}

void GraphicsEngine::setTextPrintDelay(uint16 delay) {
	_text->setPrintDelay(delay);
}

bool GraphicsEngine::isTextInQueue() const {
	return _text->needsPrint();
}

void GraphicsEngine::resetTextFields() {
	_animator->resetTextFields();
	_text->reset();
}

void GraphicsEngine::clearTextInputLine() {
	_animator->clearTextInputLine();
}

uint8 GraphicsEngine::getVerbAreaType() const {
	return _verbAreaType;
}

bool GraphicsEngine::isVerbsTabActive() const {
	return _trs->scroll_getState().verbsTabVisible;
}

void GraphicsEngine::updateSaveLoadDialog(SaveInfo &saveInfo) {
	_animator->updateSaveLoadDialog(saveInfo);
}

void GraphicsEngine::updateAnimations() {
	_animator->updateAnimations();
}

void GraphicsEngine::updateText() {
	if (_text->needsPrint())
		_text->draw();
}

void GraphicsEngine::nextFrame() {
	_palette->processEventQueue();
	_palette->update();
	_animator->updateScreen(_screen);

	_system->copyRectToScreen(_screen, _animator->realScreenWidth() * _bpp, 0, 0, _animator->realScreenWidth(), _animator->realScreenHeight());
	_system->updateScreen();

	_state.nextFrame();
}

void GraphicsEngine::reset(int mode) {
	if (mode & kResetSetDefaultsExt)
		restoreDefaultsExt();
	if (mode & kResetSetDefaults)
		restoreDefaults();
	if (mode & kResetPalEvents)
		_palette->clearEvents();
	if (mode & kResetAnimations)
		_animator->clearAnimations();
	if (mode & kResetCopyCmds)
		_animator->clearDrawCommands();
	if (mode & kResetScrollState)
		_trs->clear();
}

void GraphicsEngine::setVar(uint8 var, uint8 val) {
	_state.setVar(var, val);
}

uint8 GraphicsEngine::getVar(uint8 var) const {
	return _state.getVar(var);
}

void GraphicsEngine::setAnimParameter(uint8 animObjId, int param, int32 value) {
	_animator->setAnimParameter(animObjId, param, value);
}

void GraphicsEngine::setAnimGroupParameter(uint8 animObjId, int groupOp, int32 value) {
	_animator->setAnimGroupParameter(animObjId, groupOp, value);
}

int32 GraphicsEngine::getAnimParameter(uint8 animObjId, int param) const {
	return _animator->getAnimParameter(animObjId, param);
}

uint8 GraphicsEngine::getAnimScriptByte(uint8 animObjId, uint16 offset) const {
	return _animator->getAnimScriptByte(animObjId, offset);
}

void GraphicsEngine::addAnimParameterFlags(uint8 animObjId, int param, int flags) {
	setAnimParameter(animObjId, param, getAnimParameter(animObjId, param) | flags);
}

void GraphicsEngine::clearAnimParameterFlags(uint8 animObjId, int param, int flags) {
	setAnimParameter(animObjId, param, getAnimParameter(animObjId, param) & ~flags);
}

bool GraphicsEngine::testAnimParameterFlags(uint8 animObjId, int param, int flags) const {
	return (getAnimParameter(animObjId, param) & flags);
}

void GraphicsEngine::animCopySpec(uint8 srcAnimObjId, uint8 dstAnimObjId) {
	_animator->animCopySpec(srcAnimObjId, dstAnimObjId);
}

uint16 GraphicsEngine::screenWidth() const {
	return _animator ? _animator->screenWidth() : 0;
}

uint16 GraphicsEngine::screenHeight() const {
	return _animator ? _animator->screenHeight() : 0;
}

uint16 GraphicsEngine::realScreenWidth() const {
	return _animator ? _animator->realScreenWidth() : 0;
}

uint16 GraphicsEngine::realScreenHeight() const {
	return _animator ? _animator->realScreenHeight() : 0;
}

bool GraphicsEngine::busy(int type) const {
	if (type == 0)
		return ((_state.getVar(0) != 0) || (_state.getVar(8) != 0));
	else if (type == 1)
		return _state.getVar(3);
	else if (type == 2)
		return ((_state.getVar(4) & 0x7F) || (_state.testFlag(1, 0)));
	return false;
}

uint16 GraphicsEngine::frameCount() const {
	return _state.frameCount();
}

void GraphicsEngine::loadState(Common::SeekableReadStream *in, bool onlyTempData) {
	if (in->readUint32BE() != MKTAG('S', 'N', 'A', 'T'))
		error("%s(): Save file invalid or corrupt", __FUNCTION__);
	in->read(_animSaveLoadData, 64);

	if (onlyTempData)
		return;

	//for (int i = 0; i < 13; ++i)
	//	_state.setVar(i, in->readByte());
}

void GraphicsEngine::saveState(Common::SeekableWriteStream *out, bool onlyTempData) {
	out->writeUint32BE(MKTAG('S', 'N', 'A', 'T'));
	for (int i = 0; i < 64; ++i) {
		uint8 v = 0;
		if (getAnimParameter(i, kAnimParaEnable)) {
			v |= 0x80;
			uint8 cf = getAnimParameter(i, kAnimParaScriptComFlags);
			if (cf & 0x80) {
				v |= 0x40;
			} else {
				v |= (getAnimParameter(i, kAnimParaControlFlags) & 0x03);
				v |= (getAnimParameter(i, kAnimParaPhase) & 0x0C);
				if (cf & 1)
					v |= 0x10;
				if (getAnimParameter(i, kAnimParaAllowFrameDrop))
					v |= 0x20;
			}
		}
		_animSaveLoadData[i] = v;
	}
	out->write(_animSaveLoadData, 64);

	if (onlyTempData)
		return;

	for (int i = 0; i < 13; ++i)
		out->writeByte(_state.getVar(i));
}

void GraphicsEngine::postLoadProcess() {
	for (int i = 0; i < 64; ++i) {
		if (!getAnimParameter(i, kAnimParaEnable))
			continue;
		uint8 v = _animSaveLoadData[i];
		if (!(v & 0x80)) {
			setAnimParameter(i, kAnimParaEnable, 0);
			continue;
		}
		if (v & 0x40)
			continue;
		uint8 cf = getAnimParameter(i, kAnimParaControlFlags) & 0xFC;
		setAnimParameter(i, kAnimParaControlFlags, cf | (v & 0x03));
		cf = getAnimParameter(i, kAnimParaPhase) & 0xF3;
		setAnimParameter(i, kAnimParaPhase, cf | (v & 0x0C));
		if (v & 0x10)
			setAnimParameter(i, kAnimParaScriptComFlags, getAnimParameter(i, kAnimParaScriptComFlags) | 1);
		else
			setAnimParameter(i, kAnimParaScriptComFlags, getAnimParameter(i, kAnimParaScriptComFlags) & ~1);
		setAnimParameter(i, kAnimParaAllowFrameDrop, (v & 0x20) ? 1 : 0);
	}
}

void GraphicsEngine::createMouseCursor() {
	_animator->createMouseCursor();
}

int GraphicsEngine::displayBootLogoFrame(int frameNo) {
	int res = _animator->drawBootLogoFrame(_screen, frameNo);
	_system->copyRectToScreen(_screen, _animator->realScreenWidth() * _bpp, 0, 0, _animator->realScreenWidth(), _animator->realScreenHeight());
	_system->updateScreen();
	return res;
}

void GraphicsEngine::restoreDefaultsExt() {
	if (!_state.getVar(4))
		restoreDefaults();
	_palette->clearEvents();

	if (_state.getVar(5)) {
		_palette->setDefaults(1);
		_state.setVar(5, 0);
	}

	_animator->clearDrawCommands();
	_trs->doCommand(0xFC);
	_animator->clearAnimations(1);

	if (!(_state.testFlag(4, 1)))
		_state.setVar(0, 1);
	_state.setVar(4, 0);
}

void GraphicsEngine::restoreDefaults() {
	_state.setVar(3, 1);
	if (_state.testFlag(1, 0))
		return;
	_state.setFlag(1, 0);
	_palette->clearEvents();
	_palette->setDefaults(0);
	_state.setVar(2, 1);
}

} // End of namespace Snatcher
