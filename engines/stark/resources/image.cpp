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

#include "engines/stark/resources/image.h"

#include "common/debug.h"
#include "image/png.h"

#include "engines/stark/debug.h"
#include "engines/stark/formats/xrc.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/services/archiveloader.h"
#include "engines/stark/services/settings.h"
#include "engines/stark/services/services.h"
#include "engines/stark/visual/effects/bubbles.h"
#include "engines/stark/visual/effects/fireflies.h"
#include "engines/stark/visual/effects/fish.h"
#include "engines/stark/visual/image.h"
#include "engines/stark/visual/text.h"

#include "math/line2d.h"
#include "math/vector2d.h"

namespace Stark {
namespace Resources {

Object *Image::construct(Object *parent, byte subType, uint16 index, const Common::String &name) {
	switch (subType) {
	case kImageSub2:
	case kImageSub3:
		return new ImageStill(parent, subType, index, name);
	case kImageText:
		return new ImageText(parent, subType, index, name);
	default:
		error("Unknown image subtype %d", subType);
	}
}

Image::~Image() {
	delete _visual;
}

Image::Image(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_transparent(false),
		_transparentColor(0),
		_field_44_ADF(0),
		_field_48_ADF(30),
		_visual(nullptr) {
	_type = TYPE;
}

void Image::readData(Formats::XRCReadStream *stream) {
	_filename = stream->readString();
	_hotspot = stream->readPoint();
	_transparent = stream->readBool();
	_transparentColor = stream->readUint32LE();

	uint32 polygonCount = stream->readUint32LE();
	for (uint32 i = 0; i < polygonCount; i++) {
		Polygon polygon;

		uint32 pointCount = stream->readUint32LE();
		for (uint32 j = 0; j < pointCount; j++) {
			polygon.push_back(stream->readPoint());
		}

		_polygons.push_back(polygon);
	}

	_archiveName = stream->getArchiveName();
}

Visual *Image::getVisual() {
	initVisual();
	return _visual;
}

void Image::printData() {
	debug("filename: %s", _filename.c_str());
	debug("hotspot: x %d, y %d", _hotspot.x, _hotspot.y);
	debug("transparent: %d", _transparent);
	debug("transparentColor: %d", _transparentColor);
	debug("field_44: %d", _field_44_ADF);
	debug("field_48: %d", _field_48_ADF);

	for (uint32 i = 0; i < _polygons.size(); i++) {
		Common::String description;
		for (uint32 j = 0; j < _polygons[i].size(); j++) {
			description += Common::String::format("(x %d, y %d) ", _polygons[i][j].x, _polygons[i][j].y);
		}
		debug("polygon %d: %s", i, description.c_str());
	}
}

int Image::indexForPoint(const Common::Point &point) const {
	int index = -1;
	for (uint32 i = 0; i < _polygons.size(); i++) {
		if (isPointInPolygon(_polygons[i], point)) {
			index = i;
		}
	}

	return index;
}

bool Image::isPointInPolygon(const Polygon &polygon, const Common::Point &point) const {
	if (polygon.size() <= 1) {
		return false; // Empty polygon
	}

	// A ray cast from the point
	Math::Segment2d testLine(Math::Vector2d(point.x, point.y), Math::Vector2d(-100, -100));

	// Special case the line created between the last point and the first
	Math::Vector2d prevPoint = Math::Vector2d(polygon.back().x, polygon.back().y);

	// Count the intersections of the ray with the polygon's edges
	int intersectCount = 0;
	for (uint32 j = 0; j < polygon.size(); j++) {
		Math::Vector2d curPoint = Math::Vector2d(polygon[j].x, polygon[j].y);

		if (Math::Segment2d(prevPoint, curPoint).intersectsSegment(testLine, nullptr)) {
			intersectCount++;
		}

		prevPoint = curPoint;
	}

	// If the ray crosses the polygon an odd number of times, the point is inside the polygon
	return intersectCount % 2 != 0;
}

Common::Point Image::getHotspotPosition(uint index) const {
	if (index >= _polygons.size()) {
		return Common::Point(-1, -1);
	}

	Polygon polygon = _polygons[index];

	// Return the top-middle point as the hotspot
	int right = polygon[0].x, top = polygon[0].y;

	for (uint i = 1; i < polygon.size(); ++i) {
		right += polygon[i].x;
		if (polygon[i].y < top) {
			top = polygon[i].y;
		}
	}

	right /= polygon.size();
	
	if (top < 0) {
		top = 0;
	}

	return Common::Point(right, top);
}

ImageStill::~ImageStill() {
}

ImageStill::ImageStill(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Image(parent, subType, index, name),
		_noName(false) {
}

void ImageStill::readData(Formats::XRCReadStream *stream) {
	Image::readData(stream);

	if (stream->isDataLeft()) {
		_field_44_ADF = stream->readUint32LE();
		_field_44_ADF /= 33;
	}

	if (stream->isDataLeft()) {
		_field_48_ADF = stream->readUint32LE();
	}

	_noName = _filename == "noname" || _filename == "noname.xmg";
}

void ImageStill::onPostRead() {
	initVisual();
}

void ImageStill::initVisual() {
	if (_visual) {
		return; // The visual is already there
	}

	if (_noName) {
		return; // No file to load
	}

	Common::ReadStream *xmgStream = StarkArchiveLoader->getFile(_filename, _archiveName);

	VisualImageXMG *visual = new VisualImageXMG(StarkGfx);

	if (StarkSettings->isAssetsModEnabled() && loadPNGOverride(visual)) {
		visual->readOriginalSize(xmgStream);
	} else {
		visual->load(xmgStream);
	}

	visual->setHotSpot(_hotspot);

	_visual = visual;

	delete xmgStream;
}

bool ImageStill::loadPNGOverride(VisualImageXMG *visual) const {
	if (!_filename.hasSuffixIgnoreCase(".xmg")) {
		return false;
	}

	Common::String pngFilename = Common::String(_filename.c_str(), _filename.size() - 4) + ".png";
	Common::String pngFilePath = StarkArchiveLoader->getExternalFilePath(pngFilename, _archiveName);

	debugC(kDebugModding, "Attempting to load %s", pngFilePath.c_str());

	Common::SeekableReadStream *pngStream = SearchMan.createReadStreamForMember(pngFilePath);
	if (!pngStream) {
		return false;
	}

	if (!visual->loadPNG(pngStream)) {
		warning("Failed to load %s. It is not a valid PNG file.", pngFilePath.c_str());
		delete pngStream;
		return false;
	}

	debugC(kDebugModding, "Loaded %s", pngFilePath.c_str());

	delete pngStream;
	return true;
}

void ImageStill::printData() {
	Image::printData();
}

ImageText::ImageText(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Image(parent, subType, index, name),
		_color(Color(0, 0, 0)),
		_font(0) {
}

ImageText::~ImageText() {
}

void ImageText::readData(Formats::XRCReadStream *stream) {
	Image::readData(stream);

	_size = stream->readPoint();
	_text = stream->readString();
	_color.r = stream->readByte();
	_color.g = stream->readByte();
	_color.b = stream->readByte();
	_color.a = stream->readByte() | 0xFF;
	_font = stream->readUint32LE();

	// WORKAROUND: Give more space to text in the Archives' computer
	// So there are no line breaks in the French version of the game
	// when using a scaled font.
	Location *location = findParent<Location>();
	if (_name == "MAIN" && location && location->getName() == "Archive Database") {
		_size.x = 80;
	}
}

void ImageText::initVisual() {
	if (_visual) {
		return; // The visual is already there
	}

	if (_text.hasPrefix("GFX_Bubbles")) {
		VisualEffectBubbles *bubbles = new VisualEffectBubbles(StarkGfx, _size);
		bubbles->setParams(_text);
		_visual = bubbles;
	} else if (_text.hasPrefix("GFX_FireFlies")) {
		VisualEffectFireFlies *fireFlies = new VisualEffectFireFlies(StarkGfx, _size);
		fireFlies->setParams(_text);
		_visual = fireFlies;
	} else if (_text.hasPrefix("GFX_Fish")) {
		VisualEffectFish *fish = new VisualEffectFish(StarkGfx, _size);
		fish->setParams(_text);
		_visual = fish;
	} else if (_text.hasPrefix("GFX_")) {
		error("Unknown effect '%s'", _text.c_str());
	} else {
		VisualText *text = new VisualText(StarkGfx);
		text->setText(_text);
		text->setColor(_color);
		text->setTargetWidth(_size.x);
		text->setTargetHeight(_size.y);
		text->setFont(FontProvider::kCustomFont, _font);

		// WORKAROUND: Move the "White Cardinal" hotspot in the Archives'
		// computer so it matches the text. Fixes the hotspot being hard to
		// use with scaled fonts in the Spanish version of the game.
		if (_name == "The Church" && _polygons.size() == 2) {
			fixWhiteCardinalHotspot(text);
		}

		_visual = text;
	}
}

void ImageText::fixWhiteCardinalHotspot(VisualText *text) {
	Common::Rect textRect = text->getRect();

	Polygon &hotspotPoly = _polygons.back();
	if (hotspotPoly.size() != 4) {
		return;
	}

	Common::Point &topLeft     = hotspotPoly[0];
	Common::Point &topRight    = hotspotPoly[1];
	Common::Point &bottomRight = hotspotPoly[2];
	Common::Point &bottomLeft  = hotspotPoly[3];

	int16 hotspotHeight = bottomLeft.y - topLeft.y;
	if (hotspotHeight <= 0) {
		return;
	}

	bottomLeft .y = textRect.bottom;
	bottomRight.y = textRect.bottom;
	topLeft    .y = textRect.bottom - hotspotHeight;
	topRight   .y = textRect.bottom - hotspotHeight;
}

void ImageText::resetVisual() {
	if (!_visual) {
		return;
	}

	VisualText *text = _visual->get<VisualText>();
	if (text) {
		text->resetTexture();
	}
}

void ImageText::printData() {
	Image::printData();

	debug("size: x %d, y %d", _size.x, _size.y);
	debug("text: %s", _text.c_str());
	debug("color: (%d, %d, %d, %d)", _color.r, _color.g, _color.b, _color.a);
	debug("font: %d", _font);
}

} // End of namespace Resources
} // End of namespace Stark
