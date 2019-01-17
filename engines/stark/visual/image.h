/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_VISUAL_IMAGE_H
#define STARK_VISUAL_IMAGE_H

#include "engines/stark/visual/visual.h"

#include "common/rect.h"
#include "common/stream.h"

namespace Graphics {
struct Surface;
}

namespace Stark {

namespace Gfx {
class Driver;
class SurfaceRenderer;
class Texture;
}

/**
 * XMG (still image) renderer
 */
class VisualImageXMG : public Visual {
public:
	static const VisualType TYPE = Visual::kImageXMG;

	explicit VisualImageXMG(Gfx::Driver *gfx);
	~VisualImageXMG() override;

	/**
	 * Load the pixel data from a XMG image
	 */
	void load(Common::ReadStream *stream);

	/**
	 * Load the size from an XMG image
	 */
	void readOriginalSize(Common::ReadStream *stream);

	/**
	 * Load the pixel data from a PNG image
	 */
	bool loadPNG(Common::SeekableReadStream *stream);

	void render(const Common::Point &position, bool useOffset);
	void render(const Common::Point &position, bool useOffset, bool unscaled);

	/** Set an offset used when rendering */
	void setHotSpot(const Common::Point &hotspot);
	Common::Point getHotspot() const { return _hotspot; }

	/**
	 * The fade level is added to the color value of each pixel
	 *
	 * It is a value between -1 and 1
	 */
	void setFadeLevel(float fadeLevel);

	/** Perform a transparency hit test on an image point */
	bool isPointSolid(const Common::Point &point) const;

	/** Get the width in pixels */
	int getWidth() const;

	/** Get the height in pixels */
	int getHeight() const;

	/** Get a read only pointer to the surface backing the image */
	const Graphics::Surface *getSurface() const;

private:
	Graphics::Surface *multiplyColorWithAlpha(const Graphics::Surface *source);

	Gfx::Driver *_gfx;
	Gfx::SurfaceRenderer *_surfaceRenderer;
	Gfx::Texture *_texture;
	Graphics::Surface *_surface;
	Common::Point _hotspot;
	uint _originalWidth;
	uint _originalHeight;
};

} // End of namespace Stark

#endif // STARK_VISUAL_IMAGE_H
