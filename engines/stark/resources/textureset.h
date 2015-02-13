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

#ifndef STARK_RESOURCES_TEXTURE_SET_H
#define STARK_RESOURCES_TEXTURE_SET_H

#include "common/str.h"

#include "engines/stark/resources/resource.h"

namespace Stark {

class Texture;
class XRCReadStream;

namespace Resources {

/**
 * A texture resource
 *
 * Used by items to provide textures to meshes
 */
class TextureSet : public Resource {
public:
	static const ResourceType::Type TYPE = ResourceType::kTextureSet;

	enum SubType {
		kTextureNormal = 1,
		kTextureFace = 2
	};

	TextureSet(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~TextureSet();

	// Resource API
	void readData(XRCReadStream *stream) override;
	void onPostRead() override;

	/** Obtain the texture to be rendered */
	Texture *getTexture();

protected:
	void printData() override;

	Common::String _filename;
	Common::String _archiveName;

	Texture *_texture;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_TEXTURE_SET_H
