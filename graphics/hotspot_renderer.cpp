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

#include "graphics/hotspot_renderer.h"
#include "common/util.h"
#include "common/system.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

namespace Graphics {

HotspotRenderer::HotspotRenderer() :
		_sizeScale(1.0f), _markerSize(_style.markerSize),
		_pointRadius(_style.pointRadius), _lineThickness(_style.lineThickness),
		_markerBlendColor(0) {
}

HotspotRenderer::~HotspotRenderer() {
	releaseResources();
}

void HotspotRenderer::releaseResources() {
	_overlay.free();
	clearLabels();
	_positions.clear();
	_labelLayouts.clear();
	_labelRects.clear();
	_markerRects.clear();
	_previousBounds = Common::Rect();
	_drawBounds = Common::Rect();
	_dirty = true;
}

void HotspotRenderer::invalidate() {
	_dirty = true;
}

void HotspotRenderer::clearLabels() {
	_labels.clear();
	_labelBytes = 0;
	_labelClock = 0;
}

static Common::Rect unionRects(Common::Rect first, const Common::Rect &second) {
	if (first.isEmpty())
		return second;
	if (!second.isEmpty())
		first.extend(second);
	return first;
}

void HotspotRenderer::addDrawRect(Common::Rect rect) {
	_drawBounds = unionRects(_drawBounds, rect);
}

void HotspotRenderer::packLabelMask(const Surface &scaled, LabelMask &mask) {
	mask.width = scaled.w;
	mask.height = scaled.h;
	mask.stride = (scaled.w + 7) / 8;
	mask.bits.resize(mask.stride * mask.height);
	for (int y = 0; y < mask.height; ++y) {
		byte *row = &mask.bits[y * mask.stride];
		for (int x = 0; x < mask.width; ++x)
			if (scaled.getPixel(x, y) != 0)
				row[x >> 3] |= (1 << (x & 7));
	}
}

const HotspotRenderer::LabelMask &HotspotRenderer::getLabelMask(const Common::U32String &text, const Font *font,
		int width, int height, int baseWidth, int baseHeight, const PixelFormat &format, LabelMask &scratch) {
	for (uint i = 0; i < _labels.size(); ++i) {
		CachedLabel &label = _labels[i];
		if (label.mask.width == width && label.mask.height == height &&
				label.font == font && label.format == format && label.text == text) {
			label.lastUse = ++_labelClock;
			return label.mask;
		}
	}
	Surface original;
	original.create(baseWidth, baseHeight, format);
	font->drawString(&original, text, 0, 0, baseWidth, format.RGBToColor(255, 255, 255), kTextAlignLeft);
	Surface *scaled = original.scale(width, height, false);
	original.free();
	LabelMask built;
	packLabelMask(*scaled, built);
	scaled->free();
	delete scaled;

	const uint bytes = (uint)built.bits.size();
	const uint budget = 4 * 1024 * 1024;
	if (bytes > budget) {
		scratch = Common::move(built);
		return scratch;
	}
	while (!_labels.empty() && (_labelBytes + bytes > budget || _labels.size() >= 512)) {
		uint oldest = 0;
		for (uint i = 1; i < _labels.size(); ++i)
			if (_labels[i].lastUse < _labels[oldest].lastUse)
				oldest = i;
		_labelBytes -= (uint)_labels[oldest].mask.bits.size();
		_labels.erase(_labels.begin() + oldest);
	}
	CachedLabel label;
	label.text = text;
	label.mask = Common::move(built);
	label.font = font;
	label.format = format;
	label.lastUse = ++_labelClock;
	_labelBytes += bytes;
	_labels.push_back(Common::move(label));
	return _labels.back().mask;
}

void HotspotRenderer::setStyle(const HotspotStyle &style) {
	HotspotStyle next = style;
	next.markerSize = CLIP(next.markerSize, 2, 256);
	next.pointRadius = CLIP(next.pointRadius, 1, 256);
	next.lineThickness = CLIP(next.lineThickness, 1, 256);
	next.borderThickness = CLIP(next.borderThickness, 1, 256);
	next.labelGap = CLIP(next.labelGap, 0, 256);
	next.paddingX = CLIP(next.paddingX, 0, 256);
	next.paddingY = CLIP(next.paddingY, 0, 256);
	next.markerColor &= 0xffffff;
	next.textColor &= 0xffffff;
	next.backgroundColor &= 0xffffff;
	next.borderColor &= 0xffffff;
	_style = next;
	invalidate();
}

void HotspotRenderer::setHotspots(Common::Array<HotspotInfo> hotspots) {
	exchangeHotspots(hotspots);
}

void HotspotRenderer::exchangeHotspots(Common::Array<HotspotInfo> &hotspots) {
	bool changed = hotspots.size() != _hotspots.size();
	for (uint i = 0; !changed && i < hotspots.size(); ++i) {
		changed = hotspots[i].position != _hotspots[i].position ||
			hotspots[i].name != _hotspots[i].name || hotspots[i].type != _hotspots[i].type;
	}
	if (changed) {
		_hotspots.swap(hotspots);
		_dirty = true;
	}
}

void HotspotRenderer::renderOverlay(OSystem &system, MarkerShape markerShape, bool showText, bool forceRedraw) {
	if (_hotspots.empty()) {
		if (system.isOverlayVisible())
			system.hideOverlay();
		_overlay.free();
		_previousBounds = Common::Rect();
		_drawBounds = Common::Rect();
		_dirty = true;
		return;
	}
	const int width = system.getOverlayWidth();
	const int height = system.getOverlayHeight();
	const int gameWidth = system.getWidth();
	const int gameHeight = system.getHeight();
	const PixelFormat format = system.getOverlayFormat();
	const int screenChangeID = system.getScreenChangeID();
	const Font *font = showText ? (_style.font ? _style.font : FontMan.getFontByUsage(FontManager::kGUIFont)) : nullptr;
	if (width <= 0 || height <= 0 || gameWidth <= 0 || gameHeight <= 0 || format.bytesPerPixel < 2)
		return;
	const bool visible = system.isOverlayVisible();
	const bool resized = _overlay.w != width || _overlay.h != height || _overlay.format != format;
	if (!_dirty && !forceRedraw && !resized && visible && format.aBits() &&
		_gameSize.x == gameWidth && _gameSize.y == gameHeight && _screenChangeID == screenChangeID &&
		_shape == markerShape && _font == font)
		return;
	if (!visible)
		system.showOverlay(false);
	if (resized)
		_overlay.create(width, height, format);
	if (format.aBits()) {
		if (!resized)
			_overlay.fillRect(_previousBounds, 0);
	} else {
		system.clearOverlay();
		system.grabOverlay(_overlay);
	}
	renderFrame(&_overlay, _hotspots, gameWidth, gameHeight, width, height, format, markerShape, font);
	Common::Rect upload = _drawBounds;
	if (format.aBits())
		upload = unionRects(upload, _previousBounds);
	if (resized || !visible || forceRedraw || screenChangeID != _screenChangeID)
		upload = Common::Rect(width, height);
	if (!upload.isEmpty())
		system.copyRectToOverlay(_overlay.getBasePtr(upload.left, upload.top), _overlay.pitch,
			upload.left, upload.top, upload.width(), upload.height());
	_previousBounds = _drawBounds;
	_gameSize = Common::Point(gameWidth, gameHeight);
	_screenChangeID = screenChangeID;
	_shape = markerShape;
	_font = font;
	_dirty = false;
}

void HotspotRenderer::render(Surface *surface,
		const Common::Array<HotspotInfo> &hotspots,
		int gameWidth, int gameHeight,
		int overlayWidth, int overlayHeight,
		const PixelFormat &format,
		MarkerShape markerShape,
		bool showText) {
	const Font *font = showText ? (_style.font ? _style.font : FontMan.getFontByUsage(FontManager::kGUIFont)) : nullptr;
	renderFrame(surface, hotspots, gameWidth, gameHeight, overlayWidth, overlayHeight, format, markerShape, font);
}

void HotspotRenderer::renderFrame(Surface *surface, const Common::Array<HotspotInfo> &hotspots,
		int gameWidth, int gameHeight, int overlayWidth, int overlayHeight,
		const PixelFormat &format, MarkerShape markerShape, const Font *font) {
	if (!surface || hotspots.empty())
		return;

	if (format.bytesPerPixel < 2 || gameWidth <= 0 || gameHeight <= 0 || overlayWidth <= 0 || overlayHeight <= 0)
		return;

	const float rawScaleX = (float)overlayWidth / (float)gameWidth;
	const float rawScaleY = (float)overlayHeight / (float)gameHeight;

	_sizeScale = _style.scaleWithGame ? MIN(rawScaleX, rawScaleY) : 1.0f;
	_markerSize = MAX(2, (int)(_style.markerSize * _sizeScale));
	_pointRadius = MAX(1, (int)(_style.pointRadius * _sizeScale));
	_lineThickness = MAX(1, (int)(_style.lineThickness * _sizeScale));
	_markerBlendColor = format.aBits() ? format.ARGBToColor(_style.markerOpacity,
		_style.markerColor >> 16, _style.markerColor >> 8, _style.markerColor) : 0;

	_drawBounds = Common::Rect();
	prepareRender(hotspots, gameWidth, gameHeight, overlayWidth, overlayHeight);

	_positions.resize(hotspots.size());
	_labelLayouts.resize(hotspots.size());
	for (uint i = 0; i < hotspots.size(); ++i) {
		_positions[i] = placeHotspot(hotspots[i], Common::Point(
			(int)(hotspots[i].position.x * rawScaleX), (int)(hotspots[i].position.y * rawScaleY)));
	}

	prepareCollisionRects(hotspots);

	for (uint i = 0; i < hotspots.size(); ++i) {
		_labelLayouts[i] = font && !hotspots[i].name.empty() ?
			layoutHotspotLabel(hotspots[i], _positions[i], overlayWidth, overlayHeight, font) : LabelLayout();
	}
	if (font) {
		for (uint i = 0; i < hotspots.size(); ++i)
			if (!hotspots[i].name.empty())
				drawLeaderLine(surface, _labelLayouts[i], hotspots[i], _positions[i],
					overlayWidth, overlayHeight, format);
		for (uint i = 0; i < hotspots.size(); ++i)
			if (!hotspots[i].name.empty())
				drawHotspotLabelBackground(surface, _labelLayouts[i], hotspots[i],
					overlayWidth, overlayHeight, format, font);
	}
	for (uint i = 0; i < hotspots.size(); ++i)
		drawMarker(surface, hotspots[i], _positions[i].x, _positions[i].y, format, markerShape);
	if (font) {
		for (uint i = 0; i < hotspots.size(); ++i)
			if (!hotspots[i].name.empty())
				drawHotspotLabel(surface, _labelLayouts[i], hotspots[i],
					overlayWidth, overlayHeight, format, font);
	}
	_drawBounds.clip(Common::Rect(overlayWidth, overlayHeight));
}

void HotspotRenderer::prepareCollisionRects(const Common::Array<HotspotInfo> &hotspots) {
	_labelRects.resize(0);
	_markerRects.resize(0);
	const int radius = MAX<int>(_markerSize / 2, _pointRadius) + _lineThickness + 2;
	for (uint i = 0; i < hotspots.size(); ++i) {
		const int x = _positions[i].x;
		const int y = _positions[i].y;
		_markerRects.push_back(Common::Rect(x - radius, y - radius, x + radius + 1, y + radius + 1));
	}
}

void HotspotRenderer::drawMarker(Surface *surface, const HotspotInfo &hotspot, int x, int y, const PixelFormat &format, MarkerShape markerShape) {
	int width = surface->w;
	int height = surface->h;
	const int extent = markerShape == kMarkerPoint ? _pointRadius : _markerSize / 2 + _lineThickness;
	Common::Rect bounds(x - extent, y - extent, x + extent + 1, y + extent + 1);
	bounds.clip(Common::Rect(width, height));
	addDrawRect(bounds);

	switch (markerShape) {
	case kMarkerDefault:
	case kMarkerCrosshair:
		drawCrosshairMarker(surface, x, y, format);
		break;
	case kMarkerSquare:
		drawSquareMarker(surface, x, y, format);
		break;
	case kMarkerPoint:
		drawPointMarker(surface, x, y, width, height, format);
		break;
	}
}

void HotspotRenderer::drawLeaderLine(Surface *surface, const LabelLayout &layout,
		const HotspotInfo &hotspot, Common::Point markerPos, int overlayWidth, int overlayHeight,
		const PixelFormat &format) {
	if (layout.boxRect.isEmpty())
		return;
	const int x = markerPos.x;
	const int y = markerPos.y;
	const Common::Rect &box = layout.boxRect;
	if (box.top != y - box.height() / 2)
		drawLine(surface, x, y, CLIP(x, (int)box.left, (int)box.right - 1),
			CLIP(y, (int)box.top, (int)box.bottom - 1), overlayWidth, overlayHeight, format, _lineThickness);
}

void HotspotRenderer::drawHotspotLabelBackground(Surface *surface, const LabelLayout &layout,
		const HotspotInfo &hotspot, int overlayWidth, int overlayHeight,
		const PixelFormat &format, const Font *font) {
	drawLabel(surface, layout, hotspot, overlayWidth, overlayHeight, format, font, true);
}

void HotspotRenderer::drawHotspotLabel(Surface *surface, const LabelLayout &layout,
		const HotspotInfo &hotspot, int overlayWidth, int overlayHeight,
		const PixelFormat &format, const Font *font) {
	drawLabel(surface, layout, hotspot, overlayWidth, overlayHeight, format, font, false);
}

bool HotspotRenderer::isLabelSlotFree(const Common::Rect &candidate, int borderGrow) const {
	Common::Rect spaced(candidate);
	spaced.grow(borderGrow + 2);
	for (uint i = 0; i < _labelRects.size(); ++i)
		if (spaced.intersects(_labelRects[i]))
			return false;
	for (uint i = 0; i < _markerRects.size(); ++i)
		if (spaced.intersects(_markerRects[i]))
			return false;
	return true;
}

static int labelDistanceScore(const Common::Rect &candidate, Common::Point markerPos) {
	const int markerX = markerPos.x, markerY = markerPos.y;
	const int dx = CLIP(markerX, (int)candidate.left, (int)candidate.right - 1) - markerX;
	const int dy = CLIP(markerY, (int)candidate.top, (int)candidate.bottom - 1) - markerY;
	return dx * dx + dy * dy;
}

Common::Rect HotspotRenderer::findInlineLabelSlot(Common::Point markerPos, int width, int height,
		int overlayWidth, int overlayHeight, int gap, int borderGrow) const {
	const int preferredY = markerPos.y - height / 2;
	const int markerRadius = MAX<int>(_markerSize / 2, _pointRadius) + _lineThickness + 2;
	const int rightX = markerPos.x + markerRadius + gap;
	const int leftX = markerPos.x - markerRadius - gap - width;
	const int candidateX[] = {rightX, leftX};
	for (int side = 0; side < 2; ++side) {
		const int x = candidateX[side];
		if (x < 2 || x + width > overlayWidth - 2 || preferredY < 2 || preferredY + height > overlayHeight - 2)
			continue;
		Common::Rect candidate(x, preferredY, x + width, preferredY + height);
		if (isLabelSlotFree(candidate, borderGrow))
			return candidate;
	}
	return Common::Rect();
}

Common::Rect HotspotRenderer::findGridLabelSlot(Common::Point markerPos, int width, int height,
		int overlayWidth, int overlayHeight, int gap, int borderGrow) const {
	const int xStep = MAX(2, width / 4);
	const int rightmostX = overlayWidth - width - 2;
	Common::Rect bestCandidate;
	bool found = false;
	int bestScore = 0;
	for (int row = 0; row <= (overlayHeight - height - 4) / (height + gap); ++row) {
		const int y = 2 + row * (height + gap);
		if (y < 2 || y + height > overlayHeight - 2)
			continue;
		for (int x = 2; x <= rightmostX; x += xStep) {
			Common::Rect candidate(x, y, x + width, y + height);
			if (!isLabelSlotFree(candidate, borderGrow))
				continue;
			const int score = labelDistanceScore(candidate, markerPos);
			if (!found || score < bestScore) {
				found = true;
				bestScore = score;
				bestCandidate = candidate;
			}
		}
		if (rightmostX >= 2 && (rightmostX - 2) % xStep != 0) {
			Common::Rect candidate(rightmostX, y, rightmostX + width, y + height);
			if (isLabelSlotFree(candidate, borderGrow)) {
				const int score = labelDistanceScore(candidate, markerPos);
				if (!found || score < bestScore) {
					found = true;
					bestScore = score;
					bestCandidate = candidate;
				}
			}
		}
	}
	return found ? bestCandidate : Common::Rect();
}

Common::Rect HotspotRenderer::findLabelPlacement(int width, int height, Common::Point markerPos,
		int overlayWidth, int overlayHeight, int gap) {
	if (width > overlayWidth - 4 || height > overlayHeight - 4)
		return Common::Rect();
	const int borderGrow = _style.labelBorder ? MAX(1, (int)(_style.borderThickness * _sizeScale)) - 1 : 0;
	Common::Rect boxRect = findInlineLabelSlot(markerPos, width, height, overlayWidth, overlayHeight, gap, borderGrow);
	if (boxRect.isEmpty())
		boxRect = findGridLabelSlot(markerPos, width, height, overlayWidth, overlayHeight, gap, borderGrow);
	if (boxRect.isEmpty())
		return Common::Rect();
	Common::Rect grown(boxRect);
	grown.grow(borderGrow);
	_labelRects.push_back(grown);
	return boxRect;
}

Common::U32String HotspotRenderer::truncateToWidth(const Common::U32String &text, const Font *font, int maxWidth) {
	if (maxWidth <= 0 || font->getStringWidth(text) <= maxWidth)
		return text;

	Common::U32String ellipsis("...");
	while (!ellipsis.empty() && font->getStringWidth(ellipsis) > maxWidth)
		ellipsis = ellipsis.substr(0, ellipsis.size() - 1);
	if (ellipsis.empty())
		return ellipsis;

	uint lo = 0, hi = text.size();
	while (lo < hi) {
		const uint mid = lo + (hi - lo + 1) / 2;
		if (font->getStringWidth(text.substr(0, mid) + ellipsis) <= maxWidth)
			lo = mid;
		else
			hi = mid - 1;
	}
	return lo > 0 ? text.substr(0, lo) + ellipsis : ellipsis;
}

HotspotRenderer::LabelLayout HotspotRenderer::layoutHotspotLabel(const HotspotInfo &hotspot, Common::Point position,
		int overlayWidth, int overlayHeight, const Font *font) {
	LabelLayout layout;
	const int gap = MAX(0, (int)(_style.labelGap * _sizeScale));
	const int padX = MAX(0, (int)(_style.paddingX * _sizeScale));
	const int padY = MAX(0, (int)(_style.paddingY * _sizeScale));
	const int borderGrow = _style.labelBorder ? MAX(1, (int)(_style.borderThickness * _sizeScale)) - 1 : 0;

	// Leave room for padding/border so the truncated text still fits within the overlay.
	const int maxTextWidth = MAX(1, overlayWidth - 4 - 2 * padX - 2 * borderGrow);
	const int maxBaseTextWidth = MAX(1, (int)(maxTextWidth / MAX(_sizeScale, 0.0001f)));
	layout.displayText = truncateToWidth(hotspot.name, font, maxBaseTextWidth);

	layout.baseTextWidth = font->getStringWidth(layout.displayText);
	const int baseTextWidth = layout.baseTextWidth;
	layout.baseTextHeight = font->getFontHeight();
	const int baseTextHeight = layout.baseTextHeight;
	if (baseTextWidth <= 0 || baseTextHeight <= 0)
		return layout;

	const int width = MAX(1, (int)(baseTextWidth * _sizeScale)) + 2 * padX;
	const int height = MAX(1, (int)(baseTextHeight * _sizeScale)) + 2 * padY;

	Common::Rect boxRect = findLabelPlacement(width, height, position, overlayWidth, overlayHeight, gap);
	if (boxRect.isEmpty())
		return layout;

	layout.boxRect = boxRect;
	layout.textRect = Common::Rect(boxRect.left + padX, boxRect.top + padY, boxRect.right - padX, boxRect.bottom - padY);
	return layout;
}

void HotspotRenderer::drawLabel(Surface *surface, const LabelLayout &layout,
		const HotspotInfo &hotspot, int overlayWidth, int overlayHeight,
		const PixelFormat &format, const Font *font, bool background) {
	if (layout.textRect.isEmpty())
		return;
	const Common::U32String &label = layout.displayText;
	const int baseTextWidth = layout.baseTextWidth;
	const int baseTextHeight = layout.baseTextHeight;
	const Common::Rect &textRect = layout.textRect;
	Common::Rect boxRect = layout.boxRect;
	const Common::Rect overlayBounds(overlayWidth, overlayHeight);

	if (background) {
		drawLabelBox(surface, boxRect, overlayWidth, overlayHeight, format);
		if (_style.labelBorder)
			boxRect.grow(MAX(1, (int)(_style.borderThickness * _sizeScale)) - 1);
		boxRect.clip(overlayBounds);
		addDrawRect(boxRect);
		return;
	}
	Common::Rect bounds = textRect;
	bounds.clip(overlayBounds);
	addDrawRect(bounds);
	const uint32 textColor = format.RGBToColor(_style.textColor >> 16, _style.textColor >> 8, _style.textColor);
	if (textRect.width() == baseTextWidth && textRect.height() == baseTextHeight &&
		textRect.left >= 0 && textRect.top >= 0 && textRect.right <= overlayWidth && textRect.bottom <= overlayHeight) {
		Surface textArea = surface->getSubArea(textRect);
		font->drawString(&textArea, label, 0, 0, baseTextWidth, textColor, kTextAlignLeft);
		return;
	}
	Common::Rect clipped = textRect;
	clipped.clip(overlayBounds);
	if (clipped.isEmpty())
		return;
	LabelMask scratch;
	const LabelMask &mask = getLabelMask(label, font, textRect.width(), textRect.height(), baseTextWidth, baseTextHeight, format, scratch);
	const int offsetX = clipped.left - textRect.left;
	const int offsetY = clipped.top - textRect.top;
	const int rowWidth = clipped.width();
	for (int row = 0; row < clipped.height(); ++row) {
		const byte *bits = &mask.bits[(offsetY + row) * mask.stride];
		const int py = clipped.top + row;
		int x = 0;
		while (x < rowWidth) {
			const int bit = offsetX + x;
			const byte b = bits[bit >> 3];
			if (!b) {
				// whole byte is empty glyph coverage; skip 8 pixels at once
				x += 8 - (bit & 7);
				continue;
			}
			if ((b >> (bit & 7)) & 1) {
				const int px = clipped.left + x;
				if (format.bytesPerPixel == 2)
					*(uint16 *)surface->getBasePtr(px, py) = (uint16)textColor;
				else if (format.bytesPerPixel == 4)
					*(uint32 *)surface->getBasePtr(px, py) = textColor;
				else
					surface->setPixel(px, py, textColor);
			}
			++x;
		}
	}
}

void HotspotRenderer::drawCrosshairMarker(Surface *surface, int x, int y, const PixelFormat &format) {
	const int lineLength = (_markerSize / 2) - 1;
	const int t = _lineThickness;
	fillBlendRect(surface, Common::Rect(x - lineLength, y - t + 1, x + lineLength + 1, y + t),
		format, _style.markerColor, _style.markerOpacity);
	fillBlendRect(surface, Common::Rect(x - t + 1, y - lineLength, x + t, y + lineLength + 1),
		format, _style.markerColor, _style.markerOpacity);
}

void HotspotRenderer::drawSquareMarker(Surface *surface, int x, int y, const PixelFormat &format) {
	Common::Rect rect((int16)_markerSize, (int16)_markerSize);
	rect.moveTo(x - _markerSize / 2, y - _markerSize / 2);
	const int t = _lineThickness;
	fillBlendRect(surface, Common::Rect(rect.left, rect.top - t + 1, rect.right, rect.top + t),
		format, _style.markerColor, _style.markerOpacity);
	fillBlendRect(surface, Common::Rect(rect.left, rect.bottom - t, rect.right, rect.bottom + t - 1),
		format, _style.markerColor, _style.markerOpacity);
	fillBlendRect(surface, Common::Rect(rect.left - t + 1, rect.top + 1, rect.left + t, rect.bottom - 1),
		format, _style.markerColor, _style.markerOpacity);
	fillBlendRect(surface, Common::Rect(rect.right - t, rect.top + 1, rect.right + t - 1, rect.bottom - 1),
		format, _style.markerColor, _style.markerOpacity);
}

void HotspotRenderer::drawPointMarker(Surface *surface, int x, int y,
		int width, int height, const PixelFormat &format) {
	int maxRadius = _pointRadius;

	for (int dy = -maxRadius; dy <= maxRadius; dy++) {
		for (int dx = -maxRadius; dx <= maxRadius; dx++) {
			int px = x + dx;
			int py = y + dy;

			if (px < 0 || px >= width || py < 0 || py >= height)
				continue;

			int distSq = dx * dx + dy * dy;
			if (distSq < _pointRadius * _pointRadius - _pointRadius + 1)
				blendPixel(surface, px, py, format, 0, _pointRadius);
		}
	}
}

void HotspotRenderer::blendPixel(Surface *surface, int px, int py,
		const PixelFormat &format, int distance, int solidSize) {
	// Convert solidSize to maximum solid distance (solidSize=1 means center pixel only, distance=0)
	int maxSolidDistance = solidSize - 1;

	if (!_style.markerOpacity || distance > maxSolidDistance)
		return;

	if (format.aBits()) {
		surface->setPixel(px, py, _markerBlendColor);
		return;
	}

	byte bgR, bgG, bgB, bgA;
	format.colorToARGB(surface->getPixel(px, py), bgA, bgR, bgG, bgB);

	const int solidBeta = _style.markerOpacity;
	const int solidAlpha = 255 - solidBeta;
	byte r = ((((_style.markerColor >> 16) & 255) * solidBeta) + (bgR * solidAlpha)) / 255;
	byte g = ((((_style.markerColor >> 8) & 255) * solidBeta) + (bgG * solidAlpha)) / 255;
	byte b = ((((_style.markerColor >> 0) & 255) * solidBeta) + (bgB * solidAlpha)) / 255;
	surface->setPixel(px, py, format.RGBToColor(r, g, b));
}

void HotspotRenderer::fillBlendRect(Surface *surface, Common::Rect rect,
		const PixelFormat &format, uint32 color, byte opacity) {
	if (!opacity)
		return;
	rect.clip(Common::Rect(surface->w, surface->h));
	if (format.aBits()) {
		surface->fillRect(rect, format.ARGBToColor(opacity, color >> 16, color >> 8, color));
		return;
	}
	const int alpha = 255 - opacity;
	const int red = ((color >> 16) & 255) * opacity;
	const int green = ((color >> 8) & 255) * opacity;
	const int blue = (color & 255) * opacity;
	for (int py = rect.top; py < rect.bottom; ++py) {
		for (int px = rect.left; px < rect.right; ++px) {
			byte r, g, b;
			format.colorToRGB(surface->getPixel(px, py), r, g, b);
			surface->setPixel(px, py, format.RGBToColor((r * alpha + red) / 255, (g * alpha + green) / 255, (b * alpha + blue) / 255));
		}
	}
}

void HotspotRenderer::drawLine(Surface *surface, int x1, int y1, int x2, int y2,
		int width, int height, const PixelFormat &format, int lineThickness) {
	addDrawRect(Common::Rect(MIN(x1, x2), MIN(y1, y2), MAX(x1, x2) + 1, MAX(y1, y2) + 1));
	const int dx = ABS(x2 - x1), dy = ABS(y2 - y1);
	const int sx = x1 < x2 ? 1 : -1, sy = y1 < y2 ? 1 : -1;
	const bool horizontal = dx >= dy;
	int error = dx - dy;
	for (;;) {
		for (int offset = 1 - lineThickness; offset < lineThickness; ++offset) {
			const int px = x1 + (horizontal ? 0 : offset);
			const int py = y1 + (horizontal ? offset : 0);
			if (px >= 0 && px < width && py >= 0 && py < height)
				blendPixel(surface, px, py, format, ABS(offset), lineThickness);
		}
		if (x1 == x2 && y1 == y2)
			break;
		const int twiceError = error * 2;
		if (twiceError > -dy) {
			error -= dy;
			x1 += sx;
		}
		if (twiceError < dx) {
			error += dx;
			y1 += sy;
		}
	}
}

void HotspotRenderer::drawLabelBox(Surface *surface, const Common::Rect &rect,
		int overlayWidth, int overlayHeight, const PixelFormat &format) {
	if (rect.width() > 2 && rect.height() > 2)
		fillBlendRect(surface, Common::Rect(rect.left + 1, rect.top + 1, rect.right - 1, rect.bottom - 1),
			format, _style.backgroundColor, _style.backgroundOpacity);
	if (!_style.labelBorder || rect.isEmpty())
		return;
	const int thickness = MAX(1, (int)(_style.borderThickness * _sizeScale));
	fillBlendRect(surface, Common::Rect(rect.left, rect.top - thickness + 1, rect.right, rect.top + thickness), format, _style.borderColor, _style.borderOpacity);
	fillBlendRect(surface, Common::Rect(rect.left, rect.bottom - thickness, rect.right, rect.bottom + thickness - 1), format, _style.borderColor, _style.borderOpacity);
	if (rect.height() > 2) {
		fillBlendRect(surface, Common::Rect(rect.left - thickness + 1, rect.top + 1, rect.left + thickness, rect.bottom - 1), format, _style.borderColor, _style.borderOpacity);
		fillBlendRect(surface, Common::Rect(rect.right - thickness, rect.top + 1, rect.right + thickness - 1, rect.bottom - 1), format, _style.borderColor, _style.borderOpacity);
	}
}

} // End of namespace Graphics
