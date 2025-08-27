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

#ifdef USE_PRINTING

#include "common/config-manager.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"

#include "graphics/paletteman.h"

#include "printman.h"

#include "gui/printing-dialog.h"

#ifdef USE_PNG
#include "image/png.h"
#else
#include "image/bmp.h"
#endif

namespace Common {

PrintingManager::~PrintingManager() {}

void PrintingManager::printImage(const Graphics::ManagedSurface &surf) {
	GUI::PrintingDialog dialog(surf);
	dialog.runModal();
}

void PrintingManager::saveAsImage(const Graphics::ManagedSurface &surf, const Common::String &fileName) {
	Common::String saveName = fileName;
	Common::String currentTarget = ConfMan.getActiveDomainName();

#ifdef USE_PNG
	const char *extension = "png";
#else
	const char *extension = "bmp";
#endif

	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();

	if (!saveName.size()) {
		for (uint n = 0; ; n++) {
			saveName = Common::String::format("%s-%s-%05d.%s", currentTarget.c_str(), "printout", n, extension);

			if (!saveFileMan->listSavefiles(saveName).size()) {
				break;
			}
		}
	}

	Common::OutSaveFile *saveFile = saveFileMan->openForSaving(saveName, false);
	if (!saveFile) {
		warning("PrintingManager::saveAsImage: Could not open file to save");
		return;
	}

	byte palette[256 * 3];
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);

#ifdef USE_PNG
	Image::writePNG(*saveFile, surf, palette);
#else
	Image::writeBMP(*saveFile, surf, palette);
#endif

	saveFile->finalize();
	delete saveFile;
}

} // End of namespace Common

#endif
