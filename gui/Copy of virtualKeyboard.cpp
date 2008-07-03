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

VirtualKeyboard::VirtualKeyboard() {
	assert(g_system);
	_system = g_system;

	_parser = new VirtualKeyboardParser(this);

}

VirtualKeyboard::~VirtualKeyboard() {

}

bool VirtualKeyboard::loadKeyboardPack(Common::String packName) {
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
			memset(buffer, 0, (fileInfo.uncompressed_size+1)*sizeof(uint8));
			unzReadCurrentFile(zipFile, buffer, fileInfo.uncompressed_size);
			unzCloseCurrentFile(zipFile);
			if (!_parser->loadBuffer(buffer, true)) {
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

	return _parser->parse();
}

void VirtualKeyboard::show() {

}

void VirtualKeyboard::runLoop() {

}

void VirtualKeyboard::draw() {

}

} // end of namespace GUI
