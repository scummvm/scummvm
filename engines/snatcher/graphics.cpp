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
#include "snatcher/render.h"
#include "snatcher/resource.h"
#include "snatcher/transition.h"
#include "common/endian.h"
#include "common/system.h"
#include "graphics/pixelformat.h"

namespace Snatcher {

GraphicsEngine::GraphicsEngine(const Graphics::PixelFormat *pxf, OSystem *system, Common::Platform platform, const VMInfo &vmstate) : _system(system), _state(vmstate),
	_renderer(nullptr), _dataMode(0), _screen(nullptr), _bpp(pxf ? pxf->bytesPerPixel : 1), _flags(0) {
	assert(system);
	_palette = Palette::create(pxf, _system->getPaletteManager(), platform, _state);
	assert(_palette);
	_trs = TransitionManager::create(platform, _state);
	_renderer = Renderer::create(pxf, platform, _state, _palette, _trs);
	assert(_renderer);
	_screen = new uint8[_renderer->screenWidth() * _renderer->screenHeight() * (pxf ? pxf->bytesPerPixel : 1)]();
	assert(_screen);
}

GraphicsEngine::~GraphicsEngine() {
	delete _renderer;
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
			_renderer->initData(sc, _dataMode);
			_state.setVar(8, 1);
			break;
		case 2:
			_renderer->initAnimations(sc, len);
			break;
		case 3: 
		case 4:
			_renderer->linkAnimations(sc, len);
			break;

		case 5:
		case 6:
		case 7:
			_transitionType = sc[1];
			break;
		case 8:
			_renderer->setPlaneMode(sc.readUINT16());
			break;
		case 9:
			_dataMode = sc[1];
			// Multi part data transfer for more than 2048 bytes. This is just due to original hardware limitations
			// (in particular, the dma transfer from word ram to vram). We can just copy the whole thing at once...
			break;
		default:
			error("Unknown opcode 0x%02x", cmd);
		}

		sc = next;
	}
}

void GraphicsEngine::enqueuePaletteEvent(ResourcePointer res) {
	_palette->enqueueEvent(res);
}

bool GraphicsEngine::enqueueDrawCommands(ResourcePointer res) {
	return _renderer->enqueueDrawCommands(res);
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
	_renderer->updateAnimations();
}

void GraphicsEngine::nextFrame() {
	_palette->processEventQueue();
	_palette->updateSystemPalette();
	_renderer->updateScreen(_screen);

	_system->copyRectToScreen(_screen, _renderer->screenWidth() * _bpp, 0, 0, _renderer->screenWidth(), _renderer->screenHeight());
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
		_renderer->clearAnimations();
	if (mode & kResetCopyCmds)
		_renderer->clearDrawCommands();
	if (mode & kResetScrollState)
		_trs->clear();
}

void GraphicsEngine::setVar(uint8 var, uint8 val) {
	_state.setVar(var, val);
}

void GraphicsEngine::setAnimControlFlags(uint8 animObjId, int flags) {
	_renderer->anim_setControlFlags(animObjId, flags);
}

void GraphicsEngine::addAnimControlFlags(uint8 animObjId, int flags) {
	_renderer->anim_addControlFlags(animObjId, flags);
}

void GraphicsEngine::clearAnimControlFlags(uint8 animObjId, int flags) {
	_renderer->anim_clearControlFlags(animObjId, flags);
}

void GraphicsEngine::setAnimFrame(uint8 animObjId, uint16 frameNo) {
	_renderer->anim_setFrame(animObjId, frameNo);
}

uint16 GraphicsEngine::getAnimCurFrame(uint8 animObjId) const {
	return _renderer->anim_getCurFrameNo(animObjId);
}

bool GraphicsEngine::isAnimEnabled(uint8 animObjId) const {
	return _renderer->anim_isEnabled(animObjId);
}

void GraphicsEngine::gunTestAnimUpdate() {
	_renderer->anim_gunTestUpdate();
}

uint16 GraphicsEngine::screenWidth() const {
	return _renderer ? _renderer->screenWidth() : 0;
}

uint16 GraphicsEngine::screenHeight() const {
	return _renderer ? _renderer->screenHeight() : 0;
}

bool GraphicsEngine::busy(int type) const {
	if (type == 0)
		return ((_state.getVar(0) != 0) || (_state.getVar(8) != 0));
	if (type == 1)
		return _state.getVar(3);
	return false;
}

uint16 GraphicsEngine::frameCount() const {
	return _state.frameCount();
}

void GraphicsEngine::createMouseCursor(bool show) {
	_renderer->createMouseCursor();
}

int GraphicsEngine::displayBootSequenceFrame(int frameNo) {
	int res = _renderer->drawBootSequenceFrame(_screen, frameNo);
	_system->copyRectToScreen(_screen, _renderer->screenWidth() * _bpp, 0, 0, _renderer->screenWidth(), _renderer->screenHeight());
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

	_renderer->clearDrawCommands();
	_trs->doCommand(0xFC);
	_renderer->clearAnimations(1);

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
