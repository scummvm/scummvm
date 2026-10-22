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

#ifndef MEDIASTATION_CLIENTS_H
#define MEDIASTATION_CLIENTS_H

#include "common/array.h"
#include "common/hashmap.h"

#include "mediastation/datafile.h"

namespace MediaStation {

class ScreenActor;

class ParameterClient {
public:
	ParameterClient();
	virtual ~ParameterClient();

	virtual bool attemptToReadFromStream(Chunk &chunk, uint sectionType) = 0;
};

enum DeviceOwnerSectionType {
	kDeviceOwnerAllowMultipleSounds = 0x35,
	kDeviceOwnerAllowMultipleStreams = 0x36,
};

enum ImtDeviceType {
	kImtDeviceTypeSound = 2,
	kImtDeviceTypeStream = 3,
};

class ImtDeviceUser {
public:
	virtual ~ImtDeviceUser() {};
	virtual bool giveUpDevice() = 0;

	void grantDevice(ImtDeviceType type);
	void releaseDevice(ImtDeviceType type);
};

// Pooh scripts rely on a sound mutex to stop overlapping sounds/movies.
// Pooh is the only known title that relies on this behavior, and no known
// titles rely on the stream mutex behavior, but it is present in the original
// so is implemnted here.
class ImtDeviceOwner : public ParameterClient {
public:
	ImtDeviceOwner();
	virtual bool attemptToReadFromStream(Chunk &chunk, uint sectionType) override;

	bool hasExclusiveAccessTo(ImtDeviceType type);
	bool grantDevice(ImtDeviceUser *user, ImtDeviceType type);
	void releaseDevice(ImtDeviceUser *user, ImtDeviceType type);
	bool releaseAll();

	void setMultipleSounds(bool allowMultipleSounds);
	void setMultipleStreams(bool allowMultipleStreams);

private:
	bool _exclusiveAccessToSounds = true;
	bool _exclusiveAccessToStreams = true;

	void forceReleaseDevice(ImtDeviceType type);

	Common::HashMap<uint, Common::Array<ImtDeviceUser *>> _deviceUsers;
};

enum DocumentSectionType {
	kDocumentContextLoadComplete = 0x10,
	kDocumentStartupInformation = 0x2e,
	kDocumentEntryScreen = 0x2f,
};

class Document : public ParameterClient, public ImtDeviceUser {
friend class Debugger;

public:
	virtual bool attemptToReadFromStream(Chunk &chunk, uint sectionType) override;
	virtual bool giveUpDevice() override;
	void readStartupInformation(Chunk &chunk);
	void readContextLoadComplete(Chunk &chunk);

	void beginTitle();
	void startContextLoad(uint contextId);
	bool isContextLoadInProgress(uint contextId);
	void branchToScreen(uint screenId, bool disableScreenAutoUpdate);

	void streamDidClose(uint streamId);
	void streamDidFinish(uint streamId);
	// These implementations are left empty because they are empty in the original,
	// but they are kept because they are technically still defined in the original.
	void streamDidOpen(uint streamId) {};
	void streamWillRead(uint streamId) {};

	uint contextIdForScreenActorId(uint screenActorId);
	void addToContextLoadQueue(uint contextId);
	void removeFromContextLoadQueue(uint contextId);
	bool isContextLoadQueued(uint contextId);
	void contextReleaseComplete(uint contextId);
	void contextAlreadyReleased(uint contextId);

	Common::String getDebugString();

private:
	uint _currentScreenActorId = 0;
	StreamFeed *_currentStreamFeed = nullptr;
	Common::Array<uint> _contextLoadQueue;
	bool _entryPointScreenIdWasOverridden = false;
	uint _entryPointScreenId = 0;
	uint _entryPointStreamId = 0;
	uint _loadingContextId = 0;
	uint _loadingScreenActorId = 0;
	uint _disabledScreenAutoUpdateToken = 0;

	void contextLoadDidComplete();
	void screenLoadDidComplete();
	void startFeed(uint streamId);
	// This is named stopFeed in the original, but it is a bit of a misnomer
	// because it also closes the stream feed. In the lower-level stream feed manager
	// and stream feeds themselves, stopping the stream feed and closing it is
	// two separate operations.
	void stopFeed();
	void blowAwayCurrentScreen();
	void preloadParentContexts(uint contextId);
	void checkQueuedContextLoads();
};

} // End of namespace MediaStation

#endif
