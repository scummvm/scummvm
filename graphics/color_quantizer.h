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

#ifndef GRAPHICS_COLOR_QUANTIZER_H
#define GRAPHICS_COLOR_QUANTIZER_H

namespace Graphics {

class Octree;
class Palette;

/**
 * @brief Class for selecting a good palette from a large number of colors.
 *
 * Colors are added one by one, after which a palette with at most maxColors
 * entries can be retrieved. The caller is responsible for freeing the palette
 * afterwards.
 */

class ColorQuantizer {
private:
	Octree *_octree = nullptr;

public:
	/**
	 * @brief Construct a new ColorQuantizer object
	 *
	 * @param maxColors   the maximum number of colors in the final palette
	 */
	ColorQuantizer(int maxColors);
	~ColorQuantizer();

	/**
	 * @brief Add a single color to the quantizer
	 *
	 * @param r   the R component of the color
	 * @param g   the G component of the color
	 * @param b   the B component of the color
	 */
	void addColor(byte r, byte g, byte b);

	/**
	 * @brief Retrieve the resulting palette from the quantizer.
	 *
	 * @return A Palette object with at most maxColor entries.
	 */
	Palette *getPalette();
};

} // End of namespace Graphics

#endif
