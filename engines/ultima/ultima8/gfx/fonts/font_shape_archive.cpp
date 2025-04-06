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

#include "ultima/ultima8/misc/debugger.h"

#include "ultima/ultima8/gfx/fonts/font_shape_archive.h"
#include "ultima/ultima8/misc/util.h"
#include "ultima/ultima8/gfx/fonts/shape_font.h"
#include "ultima/ultima8/conf/config_file_manager.h"

namespace Ultima {
namespace Ultima8 {

ShapeFont *FontShapeArchive::getFont(uint32 fontnum) {
	return dynamic_cast<ShapeFont *>(getShape(fontnum));
}

void FontShapeArchive::cache(uint32 shapenum) {
	if (shapenum >= _count) return;
	if (_shapes.empty()) _shapes.resize(_count);

	if (_shapes[shapenum]) return;

	uint32 shpsize;
	uint8 *data = getRawObject(shapenum, &shpsize);

	if (!data || shpsize == 0) return;

	// Auto detect format
	if (!_format)
		_format = Shape::DetectShapeFormat(data, shpsize);

	if (!_format) {
		delete [] data;
		warning("Unable to detect shape format for flex.");
		return;
	}

	Shape *shape = new ShapeFont(data, shpsize, _format, _id, shapenum);
	if (_palette) shape->setPalette(_palette);

	_shapes[shapenum] = shape;
}

void FontShapeArchive::setHVLeads() {
	ConfigFileManager *config = ConfigFileManager::get_instance();

	KeyMap leadkeyvals = config->listKeyValues("game", "fontleads");
	for (const auto &i : leadkeyvals) {
		int fontnum = atoi(i._key.c_str());
		Std::string leaddesc = i._value;

		Std::vector<Std::string> vals;
		SplitString(leaddesc, ',', vals);
		if (vals.size() != 2) {
			warning("Invalid hlead/vlead description: %s", leaddesc.c_str());
			continue;
		}

		int hlead = atoi(vals[0].c_str());
		int vlead = atoi(vals[1].c_str());

		ShapeFont *font = getFont(fontnum);
		if (font) {
			font->setHLead(hlead);
			font->setVLead(vlead);
		}
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
