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

#ifndef CLOUD_CLOUDMANAGER_H
#define CLOUD_CLOUDMANAGER_H

#include "backends/cloud/storage.h"
#include "common/array.h"
#include "common/singleton.h"

namespace GUI {

class CommandReceiver;

}

namespace Cloud {

class CloudManager : public Common::Singleton<CloudManager> {
	Common::Array<Cloud::Storage *> _storages;
	uint _currentStorageIndex;	

	void printBool(Cloud::Storage::BoolResponse response) const;

public:
	CloudManager();
	virtual ~CloudManager();

	/**
	 * Loads all information from configs and creates current Storage instance.
	 *
	 * @note It's called once on startup in scummvm_main().
	 */
	void init();

	/**
	 * Saves all information into configuration file.
	 */
	void save();

	/**
	 * Adds new Storage into list.	
	 *
	 * @param	storage Cloud::Storage to add.
	 * @param	makeCurrent whether added storage should be the new current storage.
	 * @param	saveConfig whether save() should be called to update configuration file.
	 */
	void addStorage(Cloud::Storage *storage, bool makeCurrent = true, bool saveConfig = true);

	/**
	 * Returns active Storage, which could be used to interact
	 *  with cloud storage.
	 *
	 * @return	active Cloud::Storage or null, if there is no active Storage.
	 */
	Cloud::Storage *getCurrentStorage();

	/**
	 * Starts saves syncing process in currently active storage if there is any.
	 */
	SavesSyncRequest *syncSaves(Cloud::Storage::BoolCallback callback = nullptr, Networking::ErrorCallback errorCallback = nullptr);

	/**
	 * Starts feature testing (the one I'm working on currently). (Temporary)
	 */
	void testFeature();

	/** Returns whether there are any requests running. */
	bool isWorking();

	/** Returns whether there is a SavesSyncRequest running. */
	bool isSyncing();

	/** Returns a number in [0, 1] range which represents current sync progress (1 = complete). */
	double getSyncProgress();

	/** Returns an array of saves names which are not yet synced (thus cannot be used). */
	Common::Array<Common::String> getSyncingFiles();

	/** Cancels running sync. */
	void cancelSync();

	/** Sets SavesSyncRequest's target to given CommandReceiver. */
	void setSyncTarget(GUI::CommandReceiver *target);
};

/** Shortcut for accessing the connection manager. */
#define CloudMan		Cloud::CloudManager::instance()

} // End of namespace Cloud

#endif
