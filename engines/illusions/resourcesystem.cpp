/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "illusions/resourcesystem.h"

namespace Illusions {

ResourceSystem::ResourceSystem() {
}

ResourceSystem::~ResourceSystem() {
	// Delete all registered resource loaders
	for (ResourceLoadersMapIterator it = _resourceLoaders.begin(); it != _resourceLoaders.end(); ++it)
		delete (*it)._value;
}

void ResourceSystem::addResourceLoader(uint32 resTypeId, BaseResourceLoader *resourceLoader) {
	_resourceLoaders[resTypeId] = resourceLoader;
}

void ResourceSystem::loadResource(uint32 resId, uint32 tag, uint32 threadId) {
	BaseResourceLoader *resourceLoader = getResourceLoader(resId);

	Resource *resource = new Resource();
	resource->_loaded = 0;
	resource->_resId = resId;
	resource->_tag = tag;
	resource->_threadId = threadId;
	resource->_resourceLoader = resourceLoader;

	resourceLoader->buildFilename(resource);

	if (resourceLoader->isFlag(kRlfLoadFile)) {
		// TODO Move to Resource class?
		Common::File fd;
		if (!fd.open(resource->filename))
			error("ResourceSystem::loadResource() Could not open %s for reading", resource->filename.c_str());
		resource->_dataSize = fd.size();
		resource->_data = (byte*)malloc(resource->_dataSize);
		fd.read(resource->_data, resource->_dataSize);
	}
	
	resourceLoader->load(resource);
	
	if (resourceLoader->isFlag(kRlfFreeDataAfterUse)) {
		// TODO Move to Resource class?
		delete resource->_data;
		resource->_data = 0;
		resource->_dataSize = 0;
	}
	
	resource->_loaded = true;

	_resources.push_back(resource);
	// TODO? Not sure if this is needed krnfileAdd(filenameb, taga);

}

BaseResourceLoader *ResourceSystem::getResourceLoader(uint32 resId) {
	ResourceLoadersMapIterator it = _resourceLoaders.find(ResourceTypeId(resId));
	if (it != _resourceLoaders.end())
		return (*it)._value;
	error("ResourceSystem::getResourceLoader() Could not find resource loader for resource id %08X", resId);
}

} // End of namespace Illusions
