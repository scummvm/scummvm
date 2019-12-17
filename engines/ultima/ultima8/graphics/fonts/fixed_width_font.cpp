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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/graphics/fonts/fixed_width_font.h"

#include "ultima/ultima8/graphics/texture.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/conf/config_file_manager.h"
#include "ultima/ultima8/filesys/file_system.h"

namespace Ultima {
namespace Ultima8 {

FixedWidthFont *FixedWidthFont::Create(std::string iniroot) {
	ConfigFileManager *config = ConfigFileManager::get_instance();
	FileSystem *filesys = FileSystem::get_instance();

	std::string filename;
	if (!config->get(iniroot + "/font/path", filename)) {
		perr << "Error: 'path' key not found in font ini" << std::endl;
		return 0;
	}

	IDataSource *ds = filesys->ReadFile(filename);

	if (!ds) {
		perr << "Error: Unable to open file " << filename << std::endl;
		return 0;
	}

	Texture *fonttex = Texture::Create(ds, filename.c_str());

	if (!fonttex) {
		perr << "Error: Unable to read texture " << filename << std::endl;
		return 0;
	}

	delete ds;

	FixedWidthFont *fwf = new FixedWidthFont;

	fwf->tex = fonttex;

	if (!config->get(iniroot + "/font/width", fwf->width)) {
		fwf->width = fwf->tex->width / 16;
	}

	if (!config->get(iniroot + "/font/height", fwf->height)) {
		fwf->height = fwf->tex->height / 16;
	}

	if (!config->get(iniroot + "/font/align_x", fwf->align_x)) {
		for (int i = 0; i < 32; i++) {
			if (fwf->width <= (1 << i)) {
				fwf->align_x = 1 << i;
				break;
			}
		}
	}

	if (!config->get(iniroot + "/font/align_y", fwf->align_y)) {
		for (int i = 0; i < 32; i++) {
			if (fwf->height <= (1 << i)) {
				fwf->align_y = 1 << i;
				break;
			}
		}
	}

	return fwf;
}

} // End of namespace Ultima8
} // End of namespace Ultima
