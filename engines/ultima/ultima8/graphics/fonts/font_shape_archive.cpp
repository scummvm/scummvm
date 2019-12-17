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

#include "ultima/ultima8/graphics/fonts/font_shape_archive.h"
#include "ultima/ultima8/misc/util.h"
#include "ultima/ultima8/graphics/fonts/shape_font.h"
#include "ultima/ultima8/conf/config_file_manager.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(FontShapeArchive, ShapeArchive)

ShapeFont *FontShapeArchive::getFont(uint32 fontnum) {
	return p_dynamic_cast<ShapeFont *>(getShape(fontnum));
}

void FontShapeArchive::cache(uint32 shapenum) {
	if (shapenum >= count) return;
	if (shapes.empty()) shapes.resize(count);

	if (shapes[shapenum]) return;

	uint32 shpsize;
	uint8 *data = getRawObject(shapenum, &shpsize);

	if (!data || shpsize == 0) return;

	// Auto detect format
	if (!format) format = Shape::DetectShapeFormat(data, shpsize);

	if (!format) {
		delete [] data;
		perr << "Error: Unable to detect shape format for flex." << std::endl;
		return;
	}

	Shape *shape = new ShapeFont(data, shpsize, format, id, shapenum);
	if (palette) shape->setPalette(palette);

	shapes[shapenum] = shape;
}

void FontShapeArchive::setHVLeads() {
	ConfigFileManager *config = ConfigFileManager::get_instance();

	KeyMap leadkeyvals = config->listKeyValues("game/fontleads");
	KeyMap::iterator iter;

	for (iter = leadkeyvals.begin(); iter != leadkeyvals.end(); ++iter) {
		int fontnum = std::atoi(iter->_key.c_str());
		std::string leaddesc = iter->_value;

		std::vector<std::string> vals;
		Pentagram::SplitString(leaddesc, ',', vals);
		if (vals.size() != 2) {
			perr << "Invalid hlead/vlead description: " << leaddesc
			     << std::endl;
			continue;
		}

		int hlead = std::atoi(vals[0].c_str());
		int vlead = std::atoi(vals[1].c_str());

		ShapeFont *font = getFont(fontnum);
		if (font) {
			font->setHLead(hlead);
			font->setVLead(vlead);
		}
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
