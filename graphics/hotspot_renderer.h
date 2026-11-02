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
#include "graphics/surface.h"

class OSystem;

namespace Graphics {

class Font;
struct Surface;
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
	kMarkerDefault = -1,
	kMarkerCrosshair,
	kMarkerSquare,
	kMarkerPoint
};


struct HotspotStyle {
	int markerSize = 10;
	int pointRadius = 3;
	int lineThickness = 1;
	int borderThickness = 1;
	int labelGap = 8;
	int paddingX = 4;
	int paddingY = 2;
	uint32 markerColor = 0xffffff;
	uint32 textColor = 0xffffff;
	uint32 backgroundColor = 0;
	uint32 borderColor = 0xffffff;
	byte markerOpacity = 180;
	byte backgroundOpacity = 153;
	byte borderOpacity = 180;
	bool labelBorder = true;
	bool scaleWithGame = true;
	const Font *font = nullptr;
};

/**
 * Renderer for drawing hotspot markers on overlay surface.
 */
class HotspotRenderer {
public:
	HotspotRenderer();
	virtual ~HotspotRenderer();

	const HotspotStyle &getStyle() const { return _style; }
	void setStyle(const HotspotStyle &style);
	void invalidate();
	void releaseResources();
	bool empty() const { return _hotspots.empty(); }
	void setHotspots(Common::Array<HotspotInfo> hotspots);
	void exchangeHotspots(Common::Array<HotspotInfo> &hotspots);
	void renderOverlay(OSystem &system, MarkerShape markerShape, bool showText, bool forceRedraw = false);

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

protected:
	struct LabelLayout {
		Common::Rect textRect, boxRect;
		int baseTextWidth = 0, baseTextHeight = 0;
	};
	virtual LabelLayout layoutHotspotLabel(const HotspotInfo &hotspot, Common::Point position,
		int overlayWidth, int overlayHeight, const Font *font) const;
	void addDrawRect(Common::Rect rect);
	virtual void prepareRender(const Common::Array<HotspotInfo> &hotspots,
		int gameWidth, int gameHeight, int overlayWidth, int overlayHeight) {}
	virtual Common::Point placeHotspot(const HotspotInfo &hotspot, Common::Point position) { return position; }
	virtual void drawMarker(Surface *surface, const HotspotInfo &hotspot, int x, int y, const PixelFormat &format, MarkerShape markerShape);
	virtual void drawHotspotLabelBackground(Surface *surface, const LabelLayout &layout, const HotspotInfo &hotspot,
		int overlayWidth, int overlayHeight, const PixelFormat &format, const Font *font);
	virtual void drawHotspotLabel(Surface *surface, const LabelLayout &layout, const HotspotInfo &hotspot,
		int overlayWidth, int overlayHeight, const PixelFormat &format, const Font *font);
	void blendPixel(Surface *surface, int px, int py, const PixelFormat &format,
		int distance, int solidSize);

private:
	HotspotRenderer(const HotspotRenderer &) = delete;
	HotspotRenderer &operator=(const HotspotRenderer &) = delete;
	/** 1-bit-per-pixel glyph coverage mask, packed 8 pixels per byte (LSB first) per row. */
	struct LabelMask {
		Common::Array<byte> bits;
		int width = 0, height = 0, stride = 0;
	};
	struct CachedLabel {
		Common::U32String text;
		LabelMask mask;
		const Font *font;
		PixelFormat format;
		uint64 lastUse;
	};
	Common::Array<CachedLabel> _labels;
	uint _labelBytes = 0;
	uint64 _labelClock = 0;
	Common::Array<Common::Point> _positions;
	Common::Array<LabelLayout> _labelLayouts;
	void clearLabels();
	static void packLabelMask(const Surface &scaled, LabelMask &mask);
	const LabelMask &getLabelMask(const Common::U32String &text, const Font *font,
		int width, int height, int baseWidth, int baseHeight, const PixelFormat &format, LabelMask &scratch);
	Common::Rect _drawBounds, _previousBounds;
	Surface _overlay;
	Common::Array<HotspotInfo> _hotspots;
	bool _dirty = true;
	Common::Point _gameSize;
	int _screenChangeID = -1;
	MarkerShape _shape = kMarkerCrosshair;
	const Font *_font = nullptr;
	void drawLabel(Surface *surface, const LabelLayout &layout, const HotspotInfo &hotspot,
		int overlayWidth, int overlayHeight, const PixelFormat &format, const Font *font, bool background);
	void renderFrame(Surface *surface, const Common::Array<HotspotInfo> &hotspots,
		int gameWidth, int gameHeight, int overlayWidth, int overlayHeight,
		const PixelFormat &format, MarkerShape markerShape, const Font *font);

protected:
	HotspotStyle _style;
	float _sizeScale;   ///< Overlay/game scale factor applied to marker geometry
	int _markerSize;    ///< Scaled size of square and crosshair markers
	int _pointRadius;   ///< Scaled radius of point marker
	int _lineThickness; ///< Scaled thickness of lines for markers and text boxes
	uint32 _markerBlendColor; ///< Precomputed ARGBToColor(markerOpacity, markerColor) for alpha-blended overlays, recomputed once per renderFrame()

	void drawCrosshairMarker(Surface *surface, int x, int y, const PixelFormat &format);
	void drawSquareMarker(Surface *surface, int x, int y, const PixelFormat &format);
	void drawPointMarker(Surface *surface, int x, int y, int width, int height, const PixelFormat &format);
	void fillBlendRect(Surface *surface, Common::Rect rect, const PixelFormat &format, uint32 color, byte opacity);
	void drawLabelBox(Surface *surface, const Common::Rect &rect,
		int overlayWidth, int overlayHeight, const PixelFormat &format);

};

} // End of namespace Graphics

#endif // GRAPHICS_HOTSPOT_RENDERER_H
