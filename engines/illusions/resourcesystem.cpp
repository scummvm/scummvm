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
#include "illusions/illusions.h"

#include "common/algorithm.h"
#include "common/debug.h"

namespace Illusions {

// ResourceInstance

void ResourceInstance::load(Resource *resource) {
}

void ResourceInstance::unload() {
}

ResourceInstance::~ResourceInstance() {
}

// Resource

void Resource::loadData() {
	debug("Resource::loadData()");
	
	Common::File fd;
	if (!fd.open(_filename))
		error("Resource::loadData() Could not open %s for reading", _filename.c_str());
	_dataSize = fd.size();
	_data = (byte*)malloc(_dataSize);
	fd.read(_data, _dataSize);
	debug("Resource::loadData() OK");
}

void Resource::unloadData() {
	debug("Resource::unloadData()");
	
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
	for (ResourceLoadersMapIterator it = _resourceLoaders.begin(); it != _resourceLoaders.end(); ++it)
		delete (*it)._value;
}

void ResourceSystem::addResourceLoader(uint32 resTypeId, BaseResourceLoader *resourceLoader) {
	_resourceLoaders[resTypeId] = resourceLoader;
}

void ResourceSystem::loadResource(uint32 resId, uint32 tag, uint32 threadId) {
	debug("ResourceSystem::loadResource(%08X, %08X, %08X)", resId, tag, threadId);
	BaseResourceLoader *resourceLoader = getResourceLoader(resId);

	Resource *resource = new Resource();
	resource->_loaded = false;
	resource->_resId = resId;
	resource->_tag = tag;
	resource->_threadId = threadId;
	resource->_gameId = _vm->getGameId();

	resourceLoader->buildFilename(resource);

	if (resourceLoader->isFlag(kRlfLoadFile)) {
		debug("ResourceSystem::loadResource() kRlfLoadFile");
		resource->loadData();
	}
	
	resourceLoader->load(resource);
	
	if (resourceLoader->isFlag(kRlfFreeDataAfterLoad)) {
		debug("ResourceSystem::loadResource() kRlfFreeDataAfterLoad");
		resource->unloadData();
	}
	
	resource->_loaded = true;

	_resources.push_back(resource);
	// TODO? Not sure if this is needed krnfileAdd(filenameb, taga);

}

void ResourceSystem::unloadResourceById(uint32 resId) {
	Resource *resource = getResource(resId);
	if (resource) 
		unloadResource(resource);
}

void ResourceSystem::unloadResourcesByTag(uint32 tag) {
	ResourcesArrayIterator it = Common::find_if(_resources.begin(), _resources.end(), ResourceEqualByTag(tag));
	while (it != _resources.end()) {
		unloadResource(*it);
		it = Common::find_if(it, _resources.end(), ResourceEqualByTag(tag));
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
	debug("Unloading %08X... (tag: %08X)", resource->_resId, resource->_tag);
	ResourcesArrayIterator it = Common::find_if(_resources.begin(), _resources.end(), ResourceEqualByValue(resource));
	if (it != _resources.end())
		_resources.remove_at(it - _resources.begin());
	delete resource;
}

} // End of namespace Illusions
