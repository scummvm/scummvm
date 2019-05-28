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
#include "illusions/resourcereader.h"
#include "illusions/illusions.h"

#include "common/algorithm.h"
#include "common/debug.h"

namespace Illusions {

// ResourceInstance

void ResourceInstance::load(Resource *resource) {
}

void ResourceInstance::unload() {
}

void ResourceInstance::pause() {
}

void ResourceInstance::unpause() {
}

ResourceInstance::~ResourceInstance() {
}

// Resource

void Resource::loadData(BaseResourceReader *resReader) {
	_data = resReader->readResource(_sceneId, _resId, _dataSize);
}

void Resource::unloadData() {
	free(_data);
	_data = 0;
	_dataSize = 0;
}

// ResourceSystem

ResourceSystem::ResourceSystem(IllusionsEngine *vm)
	: _vm(vm) {
}

ResourceSystem::~ResourceSystem() {
	// Delete all registered resource loaders
	for (ResourceLoadersMapIterator it = _resourceLoaders.begin(); it != _resourceLoaders.end(); ++it) {
		delete (*it)._value;
	}
}

void ResourceSystem::addResourceLoader(uint32 resTypeId, BaseResourceLoader *resourceLoader) {
	_resourceLoaders[resTypeId] = resourceLoader;
}

void ResourceSystem::loadResource(uint32 resId, uint32 sceneId, uint32 threadId) {
	debug(1, "ResourceSystem::loadResource(%08X, %08X, %08X)", resId, sceneId, threadId);
	BaseResourceLoader *resourceLoader = getResourceLoader(resId);

	Resource *resource = new Resource();
	resource->_loaded = false;
	resource->_resId = resId;
	resource->_sceneId = sceneId;
	resource->_threadId = threadId;
	resource->_gameId = _vm->getGameId();

	if (resourceLoader->isFlag(kRlfLoadFile)) {
		debug(1, "ResourceSystem::loadResource() kRlfLoadFile");
		resource->loadData(_vm->_resReader);
	}

	resourceLoader->load(resource);

	if (resourceLoader->isFlag(kRlfFreeDataAfterLoad)) {
		debug(1, "ResourceSystem::loadResource() kRlfFreeDataAfterLoad");
		resource->unloadData();
	}

	resource->_loaded = true;

	_resources.push_back(resource);

}

void ResourceSystem::unloadResourceById(uint32 resId) {
	Resource *resource = getResource(resId);
	if (resource)
		unloadResource(resource);
}

void ResourceSystem::unloadResourcesBySceneId(uint32 sceneId) {
	ResourcesArrayIterator it = Common::find_if(_resources.begin(), _resources.end(), ResourceEqualBySceneId(sceneId));
	while (it != _resources.end()) {
		unloadResource(*it);
		it = Common::find_if(it, _resources.end(), ResourceEqualBySceneId(sceneId));
	}
}

void ResourceSystem::unloadSceneResources(uint32 sceneId1, uint32 sceneId2) {
	ResourcesArrayIterator it = Common::find_if(_resources.begin(), _resources.end(), ResourceNotEqualByScenes(sceneId1, sceneId2));
	while (it != _resources.end()) {
		unloadResource(*it);
		it = Common::find_if(it, _resources.end(), ResourceNotEqualByScenes(sceneId1, sceneId2));
	}
}

BaseResourceLoader *ResourceSystem::getResourceLoader(uint32 resId) {
	ResourceLoadersMapIterator it = _resourceLoaders.find(ResourceTypeId(resId));
	if (it != _resourceLoaders.end())
		return (*it)._value;
	error("ResourceSystem::getResourceLoader() Could not find resource loader for resource id %08X", resId);
}

Resource *ResourceSystem::getResource(uint32 resId) {
	ResourcesArrayIterator it = Common::find_if(_resources.begin(), _resources.end(), ResourceEqualById(resId));
	return it != _resources.end() ? *it : 0;
}

void ResourceSystem::unloadResource(Resource *resource) {
	debug(1, "Unloading %08X... (sceneId: %08X)", resource->_resId, resource->_sceneId);
	ResourcesArrayIterator it = Common::find_if(_resources.begin(), _resources.end(), ResourceEqualByValue(resource));
	if (it != _resources.end())
		_resources.remove_at(it - _resources.begin());
	delete resource;
}

} // End of namespace Illusions
