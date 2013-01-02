/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/**
 * @file
 * YUV to RGB conversion used in engines:
 * - mohawk
 * - scumm (he)
 * - sword25
 */

#ifndef GRAPHICS_YUVA_TO_RGBA_H
#define GRAPHICS_YUVA_TO_RGBA_H

#include "common/scummsys.h"
#include "common/singleton.h"
#include "graphics/surface.h"

namespace Graphics {

class YUVAToRGBALookup;

class YUVAToRGBAManager : public Common::Singleton<YUVAToRGBAManager> {
public:
	/** The scale of the luminance values */
	enum LuminanceScale {
		kScaleFull, /** Luminance values range from [0, 255] */
		kScaleITU   /** Luminance values range from [16, 235], the range from ITU-R BT.601 */
	};

	/**
	 * Convert a YUV420 image to an RGB surface
	 *
	 * @param dst     the destination surface
	 * @param scale   the scale of the luminance values
	 * @param ySrc    the source of the y component
	 * @param uSrc    the source of the u component
	 * @param vSrc    the source of the v component
	 * @param aSrc    the source of the a component
	 * @param yWidth  the width of the y surface (must be divisible by 2)
	 * @param yHeight the height of the y surface (must be divisible by 2)
	 * @param yPitch  the pitch of the y surface
	 * @param uvPitch the pitch of the u and v surfaces
	 */
	void convert420(Graphics::Surface *dst, LuminanceScale scale, const byte *ySrc, const byte *uSrc, const byte *vSrc, const byte *aSrc, int yWidth, int yHeight, int yPitch, int uvPitch);

private:
	friend class Common::Singleton<SingletonBaseType>;
	YUVAToRGBAManager();
	~YUVAToRGBAManager();

	const YUVAToRGBALookup *getLookup(Graphics::PixelFormat format, LuminanceScale scale);

	YUVAToRGBALookup *_lookup;
	int16 _colorTab[4 * 256]; // 2048 bytes
};

} // End of namespace Graphics

#define YUVAToRGBAMan (::Graphics::YUVAToRGBAManager::instance())

#endif
