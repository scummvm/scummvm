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

#ifndef STARK_RESOURCES_BONES_MESH_H
#define STARK_RESOURCES_BONES_MESH_H

#include "common/str.h"

#include "engines/stark/resources/object.h"

namespace Stark {

class Model;

namespace Formats {
class XRCReadStream;
}

namespace Resources {

/**
 * Bone mesh resources reference a mesh usable by actor resources
 */
class BonesMesh : public Object {
public:
	static const Type::ResourceType TYPE = Type::kBonesMesh;

	BonesMesh(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~BonesMesh();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;
	void onPostRead() override;

	/** Obtain the mesh object */
	Model *getModel();

protected:
	void printData() override;

	Common::String _filename;
	Common::String _archiveName;

	Model *_model;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_BONES_MESH_H
