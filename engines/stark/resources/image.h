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

#include "engines/stark/resources/resource.h"

namespace Stark {

class SceneElement;
class XRCReadStream;

class Image : public Resource {
public:
	static const ResourceType::Type TYPE = ResourceType::kImage;

	enum SubType {
		kImageSub2 = 2,
		kImageSub3 = 3,
		kImageSub4 = 4
	};

	/** Image factory */
	static Resource *construct(Resource *parent, byte subType, uint16 index, const Common::String &name);

	typedef Common::Array<Common::Point> Polygon;

	Image(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Image();

	// Resource API
	void readData(XRCReadStream *stream) override;

	virtual SceneElement *getVisual();

protected:
	void printData() override;

	Common::String _filename;
	Common::String _archiveName;

	SceneElement *_visual;

	bool _transparent;
	uint32 _transparency;
	uint32 _field_44_ADF;
	uint32 _field_48_ADF;

	Common::Point _hotspot;
	Common::Array<Polygon> _polygons;
};

class ImageSub23 : public Image {
public:
	ImageSub23(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~ImageSub23();

	// Resource API
	void readData(XRCReadStream *stream) override;
	void onPostRead() override;

	// Image API
	SceneElement *getVisual() override;

protected:
	void printData() override;

	void initVisual();

	bool _noName;
};

} // End of namespace Stark

#endif // STARK_RESOURCES_IMAGE_H
