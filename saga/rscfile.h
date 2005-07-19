/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// RSC Resource file management header file

#ifndef SAGA_RSCFILE_H__
#define SAGA_RSCFILE_H__

#include "backends/fs/fs.h"
#include "common/file.h"

namespace Saga {

#define RSC_TABLEINFO_SIZE 8
#define RSC_TABLEENTRY_SIZE 8

#define RSC_MIN_FILESIZE (RSC_TABLEINFO_SIZE + RSC_TABLEENTRY_SIZE + 1)

//TODO: good PATCH.RE_ support

struct ResourceData {
	size_t offset;
	size_t size;
	Common::File *patchFile;
};

struct ResourceContext {
	const char *fileName;
	uint16 fileType;
	Common::File *file;

	bool isBigEndian;
	ResourceData *table;
	size_t count;

	Common::File *getFile(ResourceData *resourceData) const {
		if (resourceData->patchFile != NULL) {
			return resourceData->patchFile;
		} else {
			return file;
		}
	}
};

class Resource {
public:
	Resource(SagaEngine *vm);
	~Resource();
	bool createContexts();
	void clearContexts();
	void loadResource(ResourceContext *context, uint32 resourceId, byte*&resourceBuffer, size_t &resourceSize);
	size_t getResourceSize(ResourceContext *context, uint32 resourceId);
	uint32 convertResourceId(uint32 resourceId);

	ResourceContext *getContext(uint16 fileType) {
		int i;
		for (i = 0; i < _contextsCount; i++) {
			if (_contexts[i].fileType & fileType) {
				return &_contexts[i];
			}
		}
		return NULL;
	}

	bool validResourceId(ResourceContext *context, uint32 resourceId) const {
		return (resourceId < context->count);
	}

	size_t getResourceSize(ResourceContext *context, uint32 resourceId) const {
		return getResourceData(context, resourceId)->size;
	}

	size_t getResourceOffset(ResourceContext *context, uint32 resourceId) const {
		return getResourceData(context, resourceId)->offset;
	}

	ResourceData *getResourceData(ResourceContext *context, uint32 resourceId) const {
		if (!validResourceId(context, resourceId)) {
			error("Resource::getResourceData() wrong resourceId %d", resourceId);
		}
		return &context->table[resourceId];
	}

private:
	SagaEngine *_vm;
	ResourceContext *_contexts;
	int _contextsCount;
	
	bool loadContext(ResourceContext *context);

};

} // End of namespace Saga

#endif
