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

#ifndef STARK_RESOURCES_IMAGE_H
#define STARK_RESOURCES_IMAGE_H

#include "common/rect.h"
#include "common/str.h"

#include "engines/stark/resources/object.h"
#include "engines/stark/visual/text.h"

namespace Stark {

class Visual;
class VisualImageXMG;
namespace Formats {
class XRCReadStream;
}

namespace Resources {

/**
 * A still image resource
 */
class Image : public Object {
public:
	static const Type::ResourceType TYPE = Type::kImage;

	enum SubType {
		kImageSub2 = 2,
		kImageSub3 = 3,
		kImageText = 4
	};

	/** Image factory */
	static Object *construct(Object *parent, byte subType, uint16 index, const Common::String &name);

	typedef Common::Array<Common::Point> Polygon;

	Image(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Image();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;

	/** Initialize the renderable for the image */
	virtual Visual *getVisual();

	/** Get the pat-table index for a given point */
	int indexForPoint(const Common::Point &point) const;

	/** Get the hotspot position for a given index of a pat-table */
	Common::Point getHotspotPosition(uint index) const;

protected:
	void printData() override;
	bool isPointInPolygon(const Polygon &polygon, const Common::Point &point) const;

	virtual void initVisual() = 0;

	Common::String _filename;
	Common::String _archiveName;

	Visual *_visual;

	bool _transparent;
	uint32 _transparentColor;
	uint32 _field_44_ADF;
	uint32 _field_48_ADF;

	Common::Point _hotspot;
	Common::Array<Polygon> _polygons;
};

/**
 * A still image resource loading its data from an XMG file
 */
class ImageStill : public Image {
public:
	ImageStill(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~ImageStill();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;
	void onPostRead() override;

protected:
	// Resource API
	void printData() override;

	// Image API
	void initVisual() override;

	bool loadPNGOverride(Stark::VisualImageXMG *visual) const;

	bool _noName;
};

/**
 * Text image rendered from a TTF font
 */
class ImageText : public Image {
public:
	ImageText(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~ImageText();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;

	/** Reset the text visual so it is recomputed the next frame it is rendered */
	void resetVisual();

protected:
	// Resource API
	void printData() override;

	// Image API
	void initVisual() override;

	void fixWhiteCardinalHotspot(VisualText *text);

	Common::Point _size;
	Common::String _text;
	Color _color;
	uint32 _font;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_IMAGE_H
