/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/memstream.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/base_renderer3d.h"
#include "engines/wintermute/base/gfx/3ds/camera3d.h"
#include "engines/wintermute/base/gfx/3ds/light3d.h"
#include "engines/wintermute/base/gfx/3ds/loader3ds.h"
#include "engines/wintermute/base/gfx/3ds/mesh3ds.h"

namespace Wintermute {

bool load3DSObject(Common::MemoryReadStream &fileStream, BaseArray<Wintermute::Mesh3DS *> &meshes, BaseArray<Common::String> &meshNames,
                   BaseArray<Wintermute::Light3D *> &lights, BaseArray<Wintermute::Camera3D *> &cameras, BaseGame *gameRef) {
	uint32 wholeChunkSize = fileStream.readUint32LE();
	int32 end = fileStream.pos() + wholeChunkSize - 6;

	Common::String name;
	for (int8 current = fileStream.readByte(); current != 0; current = fileStream.readByte()) {
		name.insertChar(current, name.size());
	}

	while (fileStream.pos() < end) {
		uint16 chunkId = fileStream.readUint16LE();

		Mesh3DS *mesh;
		Light3D *light;
		Camera3D *camera;

		switch (chunkId) {
		case MESH:
			mesh = gameRef->_renderer3D->createMesh3DS();
			if (mesh->loadFrom3DS(fileStream)) {
				meshNames.add(name);
				meshes.add(mesh);
			} else {
				delete mesh;
			}
			break;

		case LIGHT:
			light = new Light3D(gameRef);
			if (light->loadFrom3DS(fileStream)) {
				light->setName(name.c_str());
				lights.add(light);
			} else {
				delete light;
			}
			break;

		case CAMERA:
			camera = new Camera3D(gameRef);
			if (camera->loadFrom3DS(fileStream)) {
				camera->setName(name.c_str());
				cameras.add(camera);
			} else {
				delete camera;
			}
			break;

		default:
			break;
		}
	}

	return true;
}

bool load3DSFile(const char *filename, BaseArray<Wintermute::Mesh3DS *> &meshes, BaseArray<Common::String> &meshNames,
                 BaseArray<Wintermute::Light3D *> &lights, BaseArray<Wintermute::Camera3D *> &cameras, BaseGame *gameRef) {
	uint32 fileSize = 0;
	byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename, &fileSize);

	if (buffer == nullptr) {
		return false;
	}

	Common::MemoryReadStream fileStream(buffer, fileSize);

	while (fileStream.pos() < fileStream.size()) {
		uint16 chunkId = fileStream.readUint16LE();
		uint32 chunkSize = 0;

		switch (chunkId) {
		case MAIN:
		case EDITOR:
			chunkSize = fileStream.readUint32LE();
			break;

		case OBJECT:
			load3DSObject(fileStream, meshes, meshNames, lights, cameras, gameRef);
			break;

		default:
			chunkSize = fileStream.readUint32LE();
			fileStream.seek(chunkSize - 6, SEEK_CUR);
			break;
		}
	}

	delete[] buffer;

	return true;
}

} // namespace Wintermute
