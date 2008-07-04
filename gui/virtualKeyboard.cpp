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
 * $URL$
 * $Id$
 *
 */

#include "gui/virtualKeyboard.h"
#include "gui/virtualKeyboardParser.h"
#include "common/config-manager.h"
#include "graphics/imageman.h"
#include "common/unzip.h"

namespace GUI {

VirtualKeyboard::VirtualKeyboard() : _currentMode(0) {
	assert(g_system);
	_system = g_system;

	_parser = new VirtualKeyboardParser(this);
}

VirtualKeyboard::~VirtualKeyboard() {
	// TODO: clean up event data pointers
	delete _parser;
}

void VirtualKeyboard::reset() {
	// TODO: clean up event data pointers
	_modes.clear();
	_initialMode = _currentMode = 0;
	_pos.x = _pos.y = 0;
	_hAlignment = kAlignCentre;
	_vAlignment = kAlignBottom;
	_keyQueue.clear();
	_displaying = false;

}

bool VirtualKeyboard::loadKeyboardPack(Common::String packName) {
	// reset to default settings
	reset();

	if (ConfMan.hasKey("extrapath"))
		Common::File::addDefaultDirectoryRecursive(ConfMan.get("extrapath"));

	if (Common::File::exists(packName + ".xml")) {
		// uncompressed keyboard pack
		if (!_parser->loadFile(packName + ".xml"))
			return false;
		
	} else if (Common::File::exists(packName + ".zip")) {
		// compressed keyboard pack
#ifdef USE_ZLIB
		unzFile zipFile = unzOpen((packName + ".zip").c_str());
		if (zipFile && unzLocateFile(zipFile, (packName + ".xml").c_str(), 2) == UNZ_OK) {
			unz_file_info fileInfo;
			unzOpenCurrentFile(zipFile);
			unzGetCurrentFileInfo(zipFile, &fileInfo, NULL, 0, NULL, 0, NULL, 0);
			byte *buffer = new byte[fileInfo.uncompressed_size+1];
			assert(buffer);
			memset(buffer, 0, (fileInfo.uncompressed_size+1)*sizeof(byte));
			unzReadCurrentFile(zipFile, buffer, fileInfo.uncompressed_size);
			unzCloseCurrentFile(zipFile);
			if (!_parser->loadBuffer(buffer, fileInfo.uncompressed_size+1, true)) {
				unzClose(zipFile);
				return false;
			}
		} else {
			unzClose(zipFile);
			return false;
		}
		unzClose(zipFile);

		ImageMan.addArchive(packName + ".zip");
#else
		return false;
#endif
	} else {
		warning("Keyboard pack not found");
		return false;
	}

	if (!_parser->parse())
		return false;

	if (!_initialMode)
		warning("Initial mode of keyboard pack not defined");

	ModeMap::iterator it;
	for (it = _modes.begin(); it != _modes.end(); it++) {
		// if no image then it means layout tag for the 
		// required resolution was missing from the mode tag.
		if (!it->_value.image) {
			warning("'%s' layout missing from '%s' mode", it->_value.resolution, it->_value.name);
			return false;
		}
	}

	reposition();

	return true;
}

void VirtualKeyboard::reposition()
{
	// calculate keyboard co-ordinates
	int16 scrW = _system->getOverlayWidth(), scrH = _system->getOverlayHeight();
	int16 keyW = _currentMode->image->w, keyH = _currentMode->image->h;
	if (scrW != keyW) {
		switch (_hAlignment) {
		case kAlignCentre:
			_pos.x = (scrW - keyW) / 2;
			break;
		case kAlignRight:
			_pos.x = scrW - keyW;
			break;
		}
	}
	if (scrH != keyH) {
		switch (_vAlignment) {
		case kAlignMiddle:
			_pos.y = (scrH - keyH) / 2;
			break;
		case kAlignBottom:
			_pos.y = scrH - keyH;
			break;
		}
	}
}

void VirtualKeyboard::processClick(int16 x, int16 y)
{
	x -= _pos.x;
	y -= _pos.y;
	if (x < 0 || x > _currentMode->image->w) return;
	if (y < 0 || y > _currentMode->image->h) return;

	Common::MapArea *area = _currentMode->imageMap.findMapArea(x, y);
	if (!area) return;
	if (!_currentMode->events.contains(area->getTarget())) return;
	Event evt = _currentMode->events[area->getTarget()];
	
	switch (evt.type) {
	case kEventKey:
		// add virtual keypress to queue
		_keyQueue.push_back(*(Common::KeyState*)evt.data);
		break;
	case kEventSwitchMode:
		// switch to new mode
		switchMode(*(Common::String *)evt.data);
		break;
	case kEventClose:
		// close virtual keyboard
		_displaying = false;
		break;
	}
}

void VirtualKeyboard::switchMode(const Common::String& newMode) {
	if (!_modes.contains(newMode)) return;
	_currentMode = &_modes[newMode];
	reposition();
	draw();
}

void VirtualKeyboard::show() {
	_displaying = true;
	runLoop();
}

void VirtualKeyboard::runLoop() {

	while (_displaying) {

	}
}

void VirtualKeyboard::draw() {
	_system->copyRectToOverlay((OverlayColor*)_currentMode->image->pixels, 
		_currentMode->image->pitch, _pos.x, _pos.y, 
		_currentMode->image->w, _currentMode->image->h);
}

} // end of namespace GUI
