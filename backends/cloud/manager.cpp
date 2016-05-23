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

#include "backends/cloud/manager.h"
#include "backends/cloud/dropbox/dropboxstorage.h"
#include "common/config-manager.h"

namespace Cloud {

Manager::Manager(): _currentStorageIndex(0) {}

Manager::~Manager() {
	//TODO: do we have to save storages on manager destruction?	
	for (uint32 i = 0; i < _storages.size(); ++i)
		delete _storages[i];
	_storages.clear();	
}

void Manager::init() {
	bool offerDropbox = false;

	if (ConfMan.hasKey("storages_number", "cloud")) {
		int storages = ConfMan.getInt("storages_number", "cloud");
		for (int i = 1; i <= storages; ++i) {
			Storage *loaded = 0;
			Common::String keyPrefix = Common::String::format("storage%d_", i);
			if (ConfMan.hasKey(keyPrefix + "type", "cloud")) {
				Common::String storageType = ConfMan.get(keyPrefix + "type", "cloud");
				if (storageType == "Dropbox") loaded = Dropbox::DropboxStorage::loadFromConfig(keyPrefix);
				else warning("Unknown cloud storage type '%s' passed", storageType.c_str());
			} else {
				warning("Cloud storage #%d (out of %d) is missing.", i, storages);
			}
			if (loaded) _storages.push_back(loaded);
		}

		uint32 index = 0;
		if (ConfMan.hasKey("current_storage", "cloud")) {
			index = ConfMan.getInt("current_storage", "cloud") - 1; //count from 1, all for UX
		}
		if (index >= _storages.size()) index = 0;
		_currentStorageIndex = index;

		if (_storages.size() == 0) offerDropbox = true;
	} else {
		offerDropbox = true;
	}

	if (offerDropbox) {
		//this is temporary console offer to auth with Dropbox (because there is no other storage type yet anyway)
		Dropbox::DropboxStorage::authThroughConsole();
	}
}

void Manager::save() {
	ConfMan.set("storages_number", Common::String::format("%d", _storages.size()), "cloud");
	ConfMan.set("current_storage", Common::String::format("%d", _currentStorageIndex + 1), "cloud");
	for (uint32 i = 0; i < _storages.size(); ++i)
		_storages[i]->saveConfig(Common::String::format("storage%d_", i+1));
	ConfMan.flushToDisk();
}

Storage *Manager::getCurrentStorage() {
	if (_currentStorageIndex < _storages.size())
		return _storages[_currentStorageIndex];
	return 0;
}

void Manager::syncSaves(Storage::BoolCallback callback) {
	Storage *storage = getCurrentStorage();
	if (storage) storage->syncSaves(callback);
}

} //end of namespace Cloud
