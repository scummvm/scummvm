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

#include "loader3ds.h"
#include "../../base_file_manager.h"
#include "camera3d.h"
#include "light3d.h"
#include "mesh.h"

namespace Wintermute {

bool load3DSObject(byte **buffer, BaseArray<Wintermute::Mesh *>& meshes, BaseArray<Common::String>& meshNames,
				   BaseArray<Wintermute::Light3D *>& lights, BaseArray<Wintermute::Camera3D *>& cameras) {
	uint32 whole_chunk_size = *reinterpret_cast<uint32 *>(*buffer);
	byte *end = *buffer + whole_chunk_size - 2;
	*buffer += 4;

	char* name_begin = reinterpret_cast<char *>(*buffer);
	int name_size = strlen(reinterpret_cast<char *>(*buffer)) + 1;
	*buffer += name_size;

	while (*buffer < end) {
		uint16 chunk_id = *reinterpret_cast<uint16 *>(*buffer);

		Mesh *mesh;
		Light3D *light;
		Camera3D *camera;

		switch (chunk_id) {
		case MESH:
			*buffer += 2;
			mesh = new Mesh;
			if (mesh->loadFrom3DS(buffer)) {
				meshNames.add(Common::String(name_begin));
				meshes.add(mesh);
			} else {
				delete mesh;
			}
			break;

		case LIGHT:
			*buffer += 2;
			light = new Light3D(nullptr);
			if (light->loadFrom3DS(buffer)) {
				light->setName(reinterpret_cast<char *>(name_begin));
				lights.add(light);
			} else {
				delete light;
			}
			break;

		case CAMERA:
			*buffer += 2;
			camera = new Camera3D(nullptr);
			if (camera->loadFrom3DS(buffer)) {
				camera->setName(reinterpret_cast<char *>(name_begin));
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



bool load3DSFile(const char *filename, BaseArray<Wintermute::Mesh *> &meshes, BaseArray<Common::String> &meshNames,
				   BaseArray<Wintermute::Light3D *> &lights, BaseArray<Wintermute::Camera3D *> &cameras) {
	uint32 file_size = 0;
	byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename, &file_size);

	if (buffer == nullptr) {
		return false;
	}

	for (byte *iter = buffer; iter < buffer + file_size; ) {
		uint16 chunk_id = *reinterpret_cast<uint16 *>(iter);
		iter += 2;

		switch (chunk_id) {
		case MAIN:
		case EDITOR:
			iter += 4;
			break;

		case OBJECT:
			load3DSObject(&iter, meshes, meshNames, lights, cameras);
			break;

		default:
			iter += (*reinterpret_cast<uint32 *>(iter) - 2);
			break;
		}
	}

	delete[] buffer;

	return true;
}

}
