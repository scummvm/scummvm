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

#include "mediastation/datafile.h"

namespace MediaStation {

class ParameterClient {
public:
	ParameterClient() {};
	virtual ~ParameterClient() {};

	virtual bool attemptToReadFromStream(Chunk &chunk, uint sectionType) = 0;
};

enum DeviceOwnerSectionType {
	kDeviceOwnerAllowMultipleSounds = 0x35,
	kDeviceOwnerAllowMultipleStreams = 0x36,
};

class DeviceOwner : public ParameterClient {
public:
	virtual bool attemptToReadFromStream(Chunk &chunk, uint sectionType) override;

	bool _allowMultipleSounds = false;
	bool _allowMultipleStreams = false;
};

enum DocumentSectionType {
	kDocumentContextLoadComplete = 0x10,
	kDocumentStartupInformation = 0x2e,
	kDocumentEntryScreen = 0x2f,
};

class Document : public ParameterClient {
public:
	virtual bool attemptToReadFromStream(Chunk &chunk, uint sectionType) override;
	void readStartupInformation(Chunk &chunk);
	void readContextLoadComplete(Chunk &chunk);

	void beginTitle(uint overriddenEntryPointScreenId = 0);
	void startContextLoad(uint contextId);
	bool isContextLoadInProgress(uint contextId);
	void branchToScreen();
	void scheduleScreenBranch(uint screenActorId);
	void scheduleContextRelease(uint contextId);

	void streamDidClose(uint streamId);
	void streamDidFinish(uint streamId);
	// These implementations are left empty because they are empty in the original,
	// but they are kept because they are technically still defined in the original.
	void streamDidOpen(uint streamId) {};
	void streamWillRead(uint streamId) {};

	void process();
	uint contextIdForScreenActorId(uint screenActorId);

private:
	uint _currentScreenActorId = 0;
	StreamFeed *_currentStreamFeed = nullptr;
	Common::Array<uint> _requestedContextReleaseId;
	Common::Array<uint> _contextLoadQueue;
	uint _requestedScreenBranchId = 0;
	bool _entryPointScreenIdWasOverridden = false;
	uint _entryPointScreenId = 0;
	uint _entryPointStreamId = 0;
	uint _loadingContextId = 0;
	uint _loadingScreenActorId = 0;

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
	void addToContextLoadQueue(uint contextId);
	bool isContextLoadQueued(uint contextId);
	void checkQueuedContextLoads();
};

} // End of namespace MediaStation

#endif
