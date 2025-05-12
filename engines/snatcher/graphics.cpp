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
#include "snatcher/transition.h"
#include "common/endian.h"
#include "common/system.h"
#include "graphics/pixelformat.h"

namespace Snatcher {

GraphicsEngine::GraphicsEngine(const Graphics::PixelFormat *pxf, OSystem *system, Common::Platform platform, const VMInfo &vmstate, SoundEngine *snd) : _system(system), _state(vmstate),
	_animator(nullptr), _dataMode(0), _screen(nullptr), _bpp(pxf ? pxf->bytesPerPixel : 1), _flags(0) {
	assert(system);
	_palette = Palette::create(pxf, _system->getPaletteManager(), platform, _state);
	assert(_palette);
	_trs = TransitionManager::create(platform, _state);
	_animator = Animator::create(pxf, platform, _state, _palette, _trs, snd);
	assert(_animator);
	_screen = new uint8[_animator->screenWidth() * _animator->screenHeight() * (pxf ? pxf->bytesPerPixel : 1)]();
	assert(_screen);
}

GraphicsEngine::~GraphicsEngine() {
	delete _animator;
	delete _palette;
	delete _trs;
	delete[] _screen;
}

uint8 _transitionType;

void GraphicsEngine::runScript(ResourcePointer res, int func) {
	ResourcePointer sc = ResourcePointer(res.dataStart, READ_BE_UINT32(res.dataStart + 4)).getDataFromTable(func);

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
			_animator->initAnimations(sc, len);
			break;
		case 3: 
		case 4:
			_animator->linkAnimations(sc, len);
			break;

		case 5:
		case 6:
		case 7:
			_transitionType = sc[1];
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

void GraphicsEngine::setScrollStep(uint8 mode, int16 step) {
	if (mode < 4)
		_trs->scroll_setDirectionAndSpeed(mode, step);
	else
		_trs->scroll_setSingleStep(mode & 3, step);
}

void GraphicsEngine::transitionCommand(uint8 cmd) {
	_trs->doCommand(cmd);
}

void GraphicsEngine::updateAnimations() {
	_animator->updateAnimations();
}

void GraphicsEngine::nextFrame() {
	_palette->processEventQueue();
	_palette->updateSystemPalette();
	_animator->updateScreen(_screen);

	_system->copyRectToScreen(_screen, _animator->screenWidth() * _bpp, 0, 0, _animator->screenWidth(), _animator->screenHeight());
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

void GraphicsEngine::addAnimParameterFlags(uint8 animObjId, int param, int flags) {
	setAnimParameter(animObjId, param, getAnimParameter(animObjId, param) | flags);
}

void GraphicsEngine::clearAnimParameterFlags(uint8 animObjId, int param, int flags) {
	setAnimParameter(animObjId, param, getAnimParameter(animObjId, param) & ~flags);
}

bool GraphicsEngine::testAnimParameterFlags(uint8 animObjId, int param, int flags) const {
	return (getAnimParameter(animObjId, param) & flags);
}

uint16 GraphicsEngine::screenWidth() const {
	return _animator ? _animator->screenWidth() : 0;
}

uint16 GraphicsEngine::screenHeight() const {
	return _animator ? _animator->screenHeight() : 0;
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

void GraphicsEngine::createMouseCursor(bool show) {
	_animator->createMouseCursor();
}

int GraphicsEngine::displayBootLogoFrame(int frameNo) {
	int res = _animator->drawBootLogoFrame(_screen, frameNo);
	_system->copyRectToScreen(_screen, _animator->screenWidth() * _bpp, 0, 0, _animator->screenWidth(), _animator->screenHeight());
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
