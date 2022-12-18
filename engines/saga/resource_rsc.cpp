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

// RSC Resource file management module (SAGA 1, used in ITE)

#include "saga/saga.h"
#include "saga/resource.h"

namespace Saga {

void ResourceContext_RSC::processPatches(Resource *resource, const GamePatchDescription *patchFiles) {
	const GamePatchDescription *patchDescription;
	ResourceData *resourceData;

	// Process external patch files
	for (patchDescription = patchFiles; patchDescription && patchDescription->fileName; ++patchDescription) {
		if ((patchDescription->fileType & _fileType) != 0) {
			if (patchDescription->resourceId < _table.size()) {
				resourceData = &_table[patchDescription->resourceId];
				// Check if we've already found a patch for this resource. One is enough.
				if (!resourceData->patchData) {
					resourceData->patchData = new PatchData(patchDescription->fileName);
					Common::SeekableReadStream *s = resourceData->patchData->getStream();
					if (s) {
						resourceData->offset = 0;
						resourceData->size = s->size();
						// The patched ITE file is in memory, so close the patch file
						resourceData->patchData->closeStream();
					} else {
						delete resourceData->patchData;
						resourceData->patchData = nullptr;
					}
				}
			}
		}
	}
}

} // End of namespace Saga
