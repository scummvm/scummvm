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

FixedWidthFont *FixedWidthFont::Create(const Std::string &iniroot) {
	ConfigFileManager *config = ConfigFileManager::get_instance();
	FileSystem *filesys = FileSystem::get_instance();

	Std::string filename;
	if (!config->get(iniroot + "/font/path", filename)) {
		perr << "Error: 'path' key not found in font ini" << Std::endl;
		return nullptr;
	}

	IDataSource *ds = filesys->ReadFile(filename);

	if (!ds) {
		perr << "Error: Unable to open file " << filename << Std::endl;
		return nullptr;
	}

	Texture *fonttex = Texture::Create(ds, filename.c_str());

	if (!fonttex) {
		perr << "Error: Unable to read texture " << filename << Std::endl;
		// FIXME: This leaks ds
		return nullptr;
	}

	delete ds;

	FixedWidthFont *fwf = new FixedWidthFont;

	fwf->_tex = fonttex;

	if (!config->get(iniroot + "/font/width", fwf->_width)) {
		fwf->_width = fwf->_tex->w / 16;
	}

	if (!config->get(iniroot + "/font/height", fwf->_height)) {
		fwf->_height = fwf->_tex->h / 16;
	}

	if (!config->get(iniroot + "/font/align_x", fwf->_alignX)) {
		for (int i = 0; i < 32; i++) {
			if (fwf->_width <= (1 << i)) {
				fwf->_alignX = 1 << i;
				break;
			}
		}
	}

	if (!config->get(iniroot + "/font/align_y", fwf->_alignY)) {
		for (int i = 0; i < 32; i++) {
			if (fwf->_height <= (1 << i)) {
				fwf->_alignY = 1 << i;
				break;
			}
		}
	}

	return fwf;
}

} // End of namespace Ultima8
} // End of namespace Ultima
