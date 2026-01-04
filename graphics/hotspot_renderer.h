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

#ifndef GRAPHICS_HOTSPOT_RENDERER_H
#define GRAPHICS_HOTSPOT_RENDERER_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

namespace Graphics {

class Font;
class Surface;
struct PixelFormat;

/**
 * Information about a single hotspot to display.
 */
struct HotspotInfo {
	Common::Point position; ///< Position in game coordinates
	Common::String name;    ///< Display name

	HotspotInfo() {}
	HotspotInfo(const Common::Point &pos, const Common::String &n)
		: position(pos), name(n) {}
};

/**
 * Marker shape types.
 */
enum MarkerShape {
	MARKER_CROSSHAIR,
	MARKER_SQUARE,
	MARKER_POINT
};

/**
 * Renderer for drawing hotspot markers on overlay surface.
 */
class HotspotRenderer {
public:
	HotspotRenderer();
	~HotspotRenderer();

	/**
	 * Render hotspot markers on an overlay surface.
	 *
	 * @param surface       The overlay surface to draw on
	 * @param hotspots      Array of hotspot information
	 * @param gameWidth     Width of game screen
	 * @param gameHeight    Height of game screen
	 * @param overlayWidth  Width of overlay
	 * @param overlayHeight Height of overlay
	 * @param format        Pixel format of overlay
	 * @param markerShape   Type of marker to display
	 * @param showText      Whether to display text labels
	 */
	void render(Surface *surface,
				const Common::Array<HotspotInfo> &hotspots,
				int gameWidth, int gameHeight,
				int overlayWidth, int overlayHeight,
				const PixelFormat &format,
				MarkerShape markerShape,
				bool showText);

private:
	enum {
		kMarkerSize = 10,  ///< Size of square and crosshair markers
		kGlowSize = 3,     ///< Size of glow effect around markers and text
		kPointRadius = 3,  ///< Radius of point marker
		kLineThickness = 1 ///< Thickness of lines for markers and text boxes
	};

	void drawMarker(Surface *surface, int x, int y, const PixelFormat &format, MarkerShape markerShape);
	void drawCrosshairMarker(Surface *surface, int x, int y, int width, int height, const PixelFormat &format);
	void drawSquareMarker(Surface *surface, int x, int y, int width, int height, const PixelFormat &format);
	void drawPointMarker(Surface *surface, int x, int y, int width, int height, const PixelFormat &format);
	void drawHotspotLabel(Surface *surface, int overlayX, int overlayY, const Common::String &label,
						  int overlayWidth, int overlayHeight, const PixelFormat &format, const Font *font);
	void drawLabelBox(Surface *surface, int x, int y, int w, int h,
					  int overlayWidth, int overlayHeight, const PixelFormat &format);
	void drawRectWithGlow(Surface *surface, int x, int y, int w, int h,
						  int overlayWidth, int overlayHeight, const PixelFormat &format);

	void drawLineWithGlow(Surface *surface, int x1, int y1, int x2, int y2,
						  int width, int height, const PixelFormat &format,
						  int lineThickness);

	void blendPixelWithGlow(Surface *surface, int px, int py, const PixelFormat &format,
							int distance, int solidSize);
};

} // End of namespace Graphics

#endif
