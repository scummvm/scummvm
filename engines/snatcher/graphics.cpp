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


#include "snatcher/palette.h"
#include "snatcher/render.h"
#include "snatcher/resource.h"
#include "snatcher/graphics.h"
#include "common/endian.h"
#include "common/system.h"

namespace Snatcher {

GraphicsEngine::GraphicsEngine(OSystem *system, Common::Platform platform) : _system(system), _renderer(0), _dataMode(0), _screen(nullptr), _flags(0) {
	_renderer = Renderer::create(platform, _state);
	_palette = Palette::create(_system->getPaletteManager(), platform, _state);
	_screen = new uint8[_renderer->screenWidth() * _renderer->screenHeight()]();
	assert(_renderer);
	assert(_palette);
	assert(_screen);
}

GraphicsEngine::~GraphicsEngine() {
	delete _renderer;
	delete _palette;
	delete[] _screen;
}

//uint8 _vc1_var_788A_flags;
//bool _var_7886;
uint8 _gfxScript_byte_3;

//int16 _wordRAM__TABLE48__03;
//int16 _gg_A__, _gg_A__cntDwn;


//uint8 _gg_B;
//uint32 _gg_dword_787A;
//bool gfx1_sub1() { return true; }
//void gfx1_sub2() {}
//

void GraphicsEngine::runScript(ResourcePointer res, int func) {
	ResourcePointer sc = ResourcePointer(res.dataStart, READ_BE_UINT32(res.dataStart + 4)).getDataFromTable(func);

	for (uint8 cmd = *sc++; cmd != 0xFF; cmd = *sc++) {
		uint8 len = *sc++;
		const uint8 *next = sc + len;

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
			_renderer->initSprites(sc, len);
			break;
		case 3: 
		case 4:
			_renderer->linkSprites(sc, len);
			break;

		case 5:
		case 6:
		case 7:
			_gfxScript_byte_3 = sc[1];
			break;
		case 8:
			_renderer->setPlaneMode(READ_BE_UINT16(sc()));
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

void GraphicsEngine::enqueueCopyCommands(ResourcePointer res) {
	_renderer->enqueueCopyCommands(res);
}

void GraphicsEngine::doCommand(uint8 cmd) {
	_renderer->doCommand(cmd);
}

void GraphicsEngine::updateAnimations() {
	_renderer->updateAnimations();
}

void GraphicsEngine::nextFrame() {
	//if (!_skipManyEvents) {
		//_renderer->processEventQueue();
	//}

	//if (!_skipManyEvents) {
		//_blockPalEvent = false;

	// update_vint_tables_8F00_
	//vram_updt1();
	_palette->processEventQueue();
	//vintUpdateGfxStructs6();
	_renderer->updateScreen(_screen);
	_system->copyRectToScreen(_screen, _renderer->screenWidth(), 0, 0, _renderer->screenWidth(), _renderer->screenHeight());
	//if (_doSCriptBuff)
	//
		//}
	_palette->updateSystemPalette();
	_system->updateScreen();

	_state.nextFrame();
}

void GraphicsEngine::reset(int mode) {
	if (mode & kRestoreDefaultsExt)
		restoreDefaultsExt();
	if (mode & kRestoreDefaults)
		restoreDefaults();
	if (mode & kClearPalEvents)
		_palette->clearEvents();
	if (mode & kClearSprites)
		_renderer->clearSprites();
	if (mode & kResetGfxStructs6)
		_renderer->clearCopyCommands();
}

void GraphicsEngine::setVar(uint8 var, uint8 val) {
	_state.setVar(var, val);
}

uint16 GraphicsEngine::screenWidth() const {
	return _renderer ? _renderer->screenWidth() : 0;
}

uint16 GraphicsEngine::screenHeight() const {
	return _renderer ? _renderer->screenHeight() : 0;
}

bool GraphicsEngine::busy() const {
	return ((_state.getVar(0) != 0) || (_state.getVar(8) != 0));
}

void GraphicsEngine::restoreDefaultsExt() {
	if (!_state.getVar(4))
		restoreDefaults();
	_palette->clearEvents();

	if (_state.getVar(5)) {
		_palette->setDefaults(1);
		_state.setVar(5, 0);
	}

	_renderer->clearCopyCommands();
	//withD0_FF_FD_FC(0xFC);
	_renderer->clearSprites();

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
