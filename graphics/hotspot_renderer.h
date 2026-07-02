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
#include "common/ustr.h"

namespace Graphics {

class Font;
class Surface;
struct PixelFormat;

/**
 * Classification of a hotspot by its role in the game world.
 */
enum HotspotType {
	kHotspotDefault, ///< No specific type (generic / unclassified)
	kHotspotObject,  ///< Interactable object
	kHotspotExit,    ///< Exit or door leading to another area
	kHotspotNPC      ///< Non-player character
};

/**
 * Information about a single hotspot to display.
 */
struct HotspotInfo {
	Common::Point position; ///< Position in game coordinates
	Common::U32String name; ///< Display name
	HotspotType type;       ///< Classification of the hotspot

	HotspotInfo() : type(kHotspotDefault) {}
	HotspotInfo(const Common::Point &pos, const Common::U32String &n,
		HotspotType t = kHotspotDefault)
		: position(pos), name(n), type(t) {}
};

/**
 * Marker shape types.
 */
enum MarkerShape {
	kMarkerCrosshair,
	kMarkerSquare,
	kMarkerPoint
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
		kBaseMarkerSize = 10,  ///< Size of square and crosshair markers at 1x scale
		kBaseGlowSize = 3,     ///< Size of glow effect around markers and text at 1x scale
		kBasePointRadius = 3,  ///< Radius of point marker at 1x scale
		kBaseLineThickness = 1 ///< Thickness of lines for markers and text boxes at 1x scale
	};

	float _sizeScale;   ///< Overlay/game scale factor applied to marker geometry
	int _markerSize;    ///< Scaled size of square and crosshair markers
	int _glowSize;      ///< Scaled size of glow effect around markers and text
	int _pointRadius;   ///< Scaled radius of point marker
	int _lineThickness; ///< Scaled thickness of lines for markers and text boxes

	void drawMarker(Surface *surface, int x, int y, const PixelFormat &format, MarkerShape markerShape);
	void drawCrosshairMarker(Surface *surface, int x, int y, int width, int height, const PixelFormat &format);
	void drawSquareMarker(Surface *surface, int x, int y, int width, int height, const PixelFormat &format);
	void drawPointMarker(Surface *surface, int x, int y, int width, int height, const PixelFormat &format);
	void drawHotspotLabel(Surface *surface, int overlayX, int overlayY, const Common::U32String &label,
		int overlayWidth, int overlayHeight, const PixelFormat &format, const Font *font);
	void drawLabelBox(Surface *surface, int x, int y, int w, int h,
		int overlayWidth, int overlayHeight, const PixelFormat &format);
	void drawRectWithGlow(Surface *surface, int x, int y, int w, int h,
		int overlayWidth, int overlayHeight, const PixelFormat &format);

	void drawLineWithGlow(Surface *surface, int x1, int y1, int x2, int y2,
		int width, int height, const PixelFormat &format, int lineThickness);

	void blendPixelWithGlow(Surface *surface, int px, int py, const PixelFormat &format,
		int distance, int solidSize);
};

} // End of namespace Graphics

#endif // GRAPHICS_HOTSPOT_RENDERER_H
