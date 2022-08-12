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

#include "scumm/imuse_digi/dimuse_engine.h"
#include "scumm/imuse_digi/dimuse_defs.h"

namespace Scumm {

int IMuseDigital::dispatchInit() {
	_dispatchBuffer = (uint8 *)malloc(DIMUSE_SMALL_FADES * DIMUSE_SMALL_FADE_DIM + DIMUSE_LARGE_FADE_DIM * DIMUSE_LARGE_FADES);

	if (_dispatchBuffer) {
		_dispatchLargeFadeBufs = _dispatchBuffer;
		_dispatchSmallFadeBufs = _dispatchBuffer + (DIMUSE_LARGE_FADE_DIM * DIMUSE_LARGE_FADES);

		for (int i = 0; i < DIMUSE_LARGE_FADES; i++) {
			_dispatchLargeFadeFlags[i] = 0;
		}

		for (int i = 0; i < DIMUSE_SMALL_FADES; i++) {
			_dispatchSmallFadeFlags[i] = 0;
		}

		for (int i = 0; i < DIMUSE_MAX_STREAMZONES; i++) {
			_streamZones[i].useFlag = 0;
			_streamZones[i].fadeFlag = 0;
			_streamZones[i].prev = nullptr;
			_streamZones[i].next = nullptr;
			_streamZones[i].size = 0;
			_streamZones[i].offset = 0;
		}

		for (int i = 0; i < DIMUSE_MAX_DISPATCHES; i++) {
			_dispatches[i].trackPtr = nullptr;
			_dispatches[i].wordSize = 0;
			_dispatches[i].sampleRate = 0;
			_dispatches[i].channelCount = 0;
			_dispatches[i].currentOffset = 0;
			_dispatches[i].audioRemaining = 0;
			memset(_dispatches[i].map, 0, sizeof(_dispatches[i].map));
			_dispatches[i].streamPtr = nullptr;
			_dispatches[i].streamBufID = 0;
			_dispatches[i].streamZoneList = nullptr;
			_dispatches[i].streamErrFlag = 0;
			_dispatches[i].fadeBuf = nullptr;
			_dispatches[i].fadeOffset = 0;
			_dispatches[i].fadeRemaining = 0;
			_dispatches[i].fadeWordSize = 0;
			_dispatches[i].fadeSampleRate = 0;
			_dispatches[i].fadeChannelCount	= 0;
			_dispatches[i].fadeSyncFlag = 0;
			_dispatches[i].fadeSyncDelta = 0;
			_dispatches[i].fadeVol = 0;
			_dispatches[i].fadeSlope = 0;
			_dispatches[i].vocLoopStartingPoint = 0;
		}

	} else {
		debug(5, "IMuseDigital::dispatchInit(): ERROR: couldn't allocate buffers\n");
		return -1;
	}

	return 0;
}

IMuseDigiDispatch *IMuseDigital::dispatchGetDispatchByTrackId(int trackId) {
	return &_dispatches[trackId];
}

void IMuseDigital::dispatchSaveLoad(Common::Serializer &ser) {

	for (int l = 0; l < DIMUSE_MAX_DISPATCHES; l++) {
		ser.syncAsSint32LE(_dispatches[l].wordSize, VER(103));
		ser.syncAsSint32LE(_dispatches[l].sampleRate, VER(103));
		ser.syncAsSint32LE(_dispatches[l].channelCount, VER(103));
		ser.syncAsSint32LE(_dispatches[l].currentOffset, VER(103));
		ser.syncAsSint32LE(_dispatches[l].audioRemaining, VER(103));
		ser.syncArray(_dispatches[l].map, 2048, Common::Serializer::Sint32LE, VER(103));

		// This is only needed to signal if the sound originally had a stream associated with it
		int hasStream = 0;
		if (ser.isSaving()) {
			if (_dispatches[l].streamPtr)
				hasStream = 1;
			ser.syncAsSint32LE(hasStream, VER(103));
		} else {
			ser.syncAsSint32LE(hasStream, VER(103));
			_dispatches[l].streamPtr = hasStream ? (IMuseDigiStream *)1 : nullptr;
		}

		ser.syncAsSint32LE(_dispatches[l].streamBufID, VER(103));
		ser.syncAsSint32LE(_dispatches[l].streamErrFlag, VER(103));
		ser.syncAsSint32LE(_dispatches[l].fadeOffset, VER(103));
		ser.syncAsSint32LE(_dispatches[l].fadeRemaining, VER(103));
		ser.syncAsSint32LE(_dispatches[l].fadeWordSize, VER(103));
		ser.syncAsSint32LE(_dispatches[l].fadeSampleRate, VER(103));
		ser.syncAsSint32LE(_dispatches[l].fadeChannelCount, VER(103));
		ser.syncAsSint32LE(_dispatches[l].fadeSyncFlag, VER(103));
		ser.syncAsSint32LE(_dispatches[l].fadeSyncDelta, VER(103));
		ser.syncAsSint32LE(_dispatches[l].fadeVol, VER(103));
		ser.syncAsSint32LE(_dispatches[l].fadeSlope, VER(103));
		ser.syncAsSint32LE(_dispatches[l].vocLoopStartingPoint, VER(103));
	}

	if (ser.isLoading()) {
		for (int i = 0; i < DIMUSE_LARGE_FADES; i++) {
			_dispatchLargeFadeFlags[i] = 0;
		}

		for (int i = 0; i < DIMUSE_SMALL_FADES; i++) {
			_dispatchSmallFadeFlags[i] = 0;
		}

		for (int i = 0; i < DIMUSE_MAX_STREAMZONES; i++) {
			_streamZones[i].useFlag = 0;
		}
	}
}

int IMuseDigital::dispatchRestoreStreamZones() {
	IMuseDigiDispatch *curDispatchPtr;
	IMuseDigiStreamZone *curStreamZone;
	int32 sizeToFeed = _isEarlyDiMUSE ? 0x800 : 0x4000;

	curDispatchPtr = _dispatches;
	for (int i = 0; i < _trackCount; i++) {
		curDispatchPtr = &_dispatches[i];
		curDispatchPtr->fadeBuf = nullptr;

		if (curDispatchPtr->trackPtr->soundId && curDispatchPtr->streamPtr) {
			// Try allocating the stream
			curDispatchPtr->streamPtr = streamerAllocateSound(curDispatchPtr->trackPtr->soundId, curDispatchPtr->streamBufID, sizeToFeed);

			if (curDispatchPtr->streamPtr) {
				streamerSetSoundToStreamFromOffset(curDispatchPtr->streamPtr, curDispatchPtr->trackPtr->soundId, curDispatchPtr->currentOffset);

				if (_isEarlyDiMUSE) {
					if (curDispatchPtr->vocLoopStartingPoint)
						streamerSetLoopFlag(curDispatchPtr->streamPtr, curDispatchPtr->currentOffset + curDispatchPtr->audioRemaining);
				} else if (curDispatchPtr->audioRemaining) {
					// Try allocating the first streamZone of the dispatch
					curStreamZone = dispatchAllocateStreamZone();
					curDispatchPtr->streamZoneList = curStreamZone;

					if (curStreamZone) {
						curStreamZone->offset = curDispatchPtr->currentOffset;
						curStreamZone->size = 0;
						curStreamZone->fadeFlag = 0;
					} else {
						debug(5, "IMuseDigital::dispatchRestoreStreamZones(): unable to allocate streamZone during restore");
					}
				}
			} else {
				debug(5, "IMuseDigital::dispatchRestoreStreamZones(): unable to start stream during restore");
			}
		}
	}
	return 0;
}

int IMuseDigital::dispatchAllocateSound(IMuseDigiTrack *trackPtr, int groupId) {
	IMuseDigiDispatch *trackDispatch;
	int navigateMapResult;
	int32 sizeToFeed = _isEarlyDiMUSE ? 0x800 : 0x4000;

	trackDispatch = trackPtr->dispatchPtr;
	trackDispatch->currentOffset = 0;
	trackDispatch->audioRemaining = 0;
	trackDispatch->fadeBuf = nullptr;

	if (_isEarlyDiMUSE) {
		trackDispatch->vocLoopStartingPoint = 0;
	} else {
		memset(trackDispatch->map, 0, sizeof(trackDispatch->map));
	}

	if (groupId) {
		trackDispatch->streamPtr = streamerAllocateSound(trackPtr->soundId, groupId, sizeToFeed);
		trackDispatch->streamBufID = groupId;

		if (!trackDispatch->streamPtr) {
			debug(5, "IMuseDigital::dispatchAllocateSound(): unable to allocate stream for sound %d", trackPtr->soundId);
			return -1;
		}

		if (_isEarlyDiMUSE)
			return 0;

		trackDispatch->streamZoneList = 0;
		trackDispatch->streamErrFlag = 0;
	} else {
		trackDispatch->streamPtr = nullptr;
		if (_isEarlyDiMUSE)
			return dispatchSeekToNextChunk(trackDispatch);
	}

	navigateMapResult = dispatchNavigateMap(trackDispatch);
	if (navigateMapResult && navigateMapResult != -3) {
		// At this point, something went wrong, so let's release the dispatch
		debug(5, "IMuseDigital::dispatchAllocateSound(): problem starting sound (%d) in dispatch", trackPtr->soundId);
		dispatchRelease(trackPtr);
		return -1;
	}

	return 0;
}

int IMuseDigital::dispatchRelease(IMuseDigiTrack *trackPtr) {
	IMuseDigiDispatch *dispatchToDeallocate;
	IMuseDigiStreamZone *streamZoneList;

	dispatchToDeallocate = trackPtr->dispatchPtr;

	// Remove streamZones from list
	if (dispatchToDeallocate->streamPtr) {
		streamerClearSoundInStream(dispatchToDeallocate->streamPtr);

		if (_isEarlyDiMUSE)
			return 0;

		streamZoneList = dispatchToDeallocate->streamZoneList;
		if (dispatchToDeallocate->streamZoneList) {
			do {
				streamZoneList->useFlag = 0;
				removeStreamZoneFromList(&dispatchToDeallocate->streamZoneList, streamZoneList);
				streamZoneList = dispatchToDeallocate->streamZoneList;
			} while (dispatchToDeallocate->streamZoneList);
		}
	}

	// Mark the fade corresponding to our fadeBuf as unused
	if (dispatchToDeallocate->fadeBuf)
		dispatchDeallocateFade(dispatchToDeallocate, "dispatchRelease");

	return 0;
}

int IMuseDigital::dispatchSwitchStream(int oldSoundId, int newSoundId, int fadeLength, int unusedFadeSyncFlag, int offsetFadeSyncFlag) {
	int32 effFadeLen, effFadeSize, strZnSize;
	int getMapResult, i;
	IMuseDigiDispatch *curDispatch = _dispatches;

	effFadeLen = fadeLength;

	if (fadeLength > 2000)
		effFadeLen = 2000;

	for (i = 0; i < _trackCount; i++) {
		curDispatch = &_dispatches[i];
		if (oldSoundId && curDispatch->trackPtr->soundId == oldSoundId && curDispatch->streamPtr) {
			break;
		}
	}

	if (i >= _trackCount) {
		debug(5, "IMuseDigital::dispatchSwitchStream(): couldn't find sound, index went past _trackCount (%d)", _trackCount);
		return -1;
	}

	if (curDispatch->streamZoneList) {
		if (!curDispatch->wordSize) {
			debug(5, "IMuseDigital::dispatchSwitchStream(): found streamZoneList but NULL wordSize");
			return -1;
		}

		if (curDispatch->fadeBuf) {
			// Mark the fade corresponding to our fadeBuf as unused
			dispatchDeallocateFade(curDispatch, "dispatchSwitchStream");
		}

		_dispatchFadeSize = dispatchGetFadeSize(curDispatch, effFadeLen);

		strZnSize = curDispatch->streamZoneList->size;
		if (strZnSize >= _dispatchFadeSize)
			strZnSize = _dispatchFadeSize;
		_dispatchFadeSize = strZnSize;

		// Validate and adjust the fade dispatch size further;
		// this should correctly align the dispatch size to avoid starting a fade without
		// inverting the stereo image by mistake
		dispatchValidateFadeSize(curDispatch, _dispatchFadeSize, "dispatchSwitchStream");

		curDispatch->fadeBuf = dispatchAllocateFade(_dispatchFadeSize, "dispatchSwitchStream");

		// If we were able to allocate a fade, set up a fade out for the old sound.
		// We'll copy data from the stream buffer to the fade buffer
		if (curDispatch->fadeBuf) {
			curDispatch->fadeOffset = 0;
			curDispatch->fadeRemaining = 0;
			curDispatch->fadeWordSize = curDispatch->wordSize;
			curDispatch->fadeSampleRate = curDispatch->sampleRate;
			curDispatch->fadeChannelCount = curDispatch->channelCount;
			curDispatch->fadeSyncFlag = unusedFadeSyncFlag | offsetFadeSyncFlag;
			curDispatch->fadeSyncDelta = 0;
			curDispatch->fadeVol = DIMUSE_MAX_FADE_VOLUME;
			curDispatch->fadeSlope = 0;

			while (curDispatch->fadeRemaining < _dispatchFadeSize) {
				effFadeSize = _dispatchFadeSize - curDispatch->fadeRemaining;
				if ((_dispatchFadeSize - curDispatch->fadeRemaining) >= 0x4000)
					effFadeSize = 0x4000;

				memcpy(&curDispatch->fadeBuf[curDispatch->fadeRemaining], streamerGetStreamBuffer(curDispatch->streamPtr, effFadeSize), effFadeSize);

				curDispatch->fadeRemaining += effFadeSize;
			}
		} else {
			debug(5, "IMuseDigital::dispatchSwitchStream(): WARNING: couldn't allocate fade buffer (from sound %d to sound %d)", oldSoundId, newSoundId);
		}
	}

	// Clear fades and triggers for the old soundId
	char emptyMarker[1] = "";
	_fadesHandler->clearFadeStatus(curDispatch->trackPtr->soundId, -1);
	_triggersHandler->clearTrigger(curDispatch->trackPtr->soundId, (char *)emptyMarker, -1);

	// Setup the new soundId
	curDispatch->trackPtr->soundId = newSoundId;

	streamerSetReadIndex(curDispatch->streamPtr, streamerGetFreeBufferAmount(curDispatch->streamPtr));

	if (offsetFadeSyncFlag && curDispatch->streamZoneList) {
		// Start the soundId from an offset
		streamerSetSoundToStreamFromOffset(curDispatch->streamPtr, newSoundId, curDispatch->currentOffset);
		while (curDispatch->streamZoneList->next) {
			curDispatch->streamZoneList->next->useFlag = 0;
			removeStreamZoneFromList(&curDispatch->streamZoneList->next, curDispatch->streamZoneList->next);
		}
		curDispatch->streamZoneList->size = 0;

		return 0;
	} else {
		// Start the soundId from the beginning
		streamerSetSoundToStreamFromOffset(curDispatch->streamPtr, newSoundId, 0);

		while (curDispatch->streamZoneList) {
			curDispatch->streamZoneList->useFlag = 0;
			removeStreamZoneFromList(&curDispatch->streamZoneList, curDispatch->streamZoneList);
		}

		curDispatch->currentOffset = 0;
		curDispatch->audioRemaining = 0;
		memset(curDispatch->map, 0, sizeof(curDispatch->map));

		getMapResult = dispatchNavigateMap(curDispatch);
		if (!getMapResult || getMapResult == -3) {
			return 0;
		} else {
			debug(5, "IMuseDigital::dispatchSwitchStream(): problem switching stream in dispatch (from sound %d to sound %d)", oldSoundId, newSoundId);
			tracksClear(curDispatch->trackPtr);
			return -1;
		}
	}
}

int IMuseDigital::dispatchSwitchStream(int oldSoundId, int newSoundId, uint8 *crossfadeBuffer, int crossfadeBufferSize, int vocLoopFlag) {
	IMuseDigiDispatch *dispatchPtr = nullptr;
	uint8 *streamBuf;
	int i;
	int32 effAudioRemaining, audioRemaining, offset;

	for (i = 0; i < _trackCount; i++) {
		dispatchPtr = &_dispatches[i];
		if (oldSoundId && dispatchPtr->trackPtr->soundId == oldSoundId && dispatchPtr->streamPtr) {
			break;
		}
	}

	if (i >= _trackCount) {
		debug(5, "IMuseDigital::dispatchSwitchStream(): couldn't find sound, index went past _trackCount (%d)", _trackCount);
		return -1;
	}

	offset = dispatchPtr->currentOffset;
	audioRemaining = dispatchPtr->audioRemaining;
	dispatchPtr->trackPtr->soundId = newSoundId;
	dispatchPtr->fadeBuf = crossfadeBuffer;
	dispatchPtr->fadeRemaining = 0;
	dispatchPtr->fadeSyncDelta = 0;
	dispatchPtr->fadeVol = DIMUSE_MAX_FADE_VOLUME;
	dispatchPtr->fadeSlope = 0;

	if (crossfadeBufferSize) {
		do {
			if (!streamerGetFreeBufferAmount(dispatchPtr->streamPtr)
				|| (!dispatchPtr->audioRemaining && dispatchSeekToNextChunk(dispatchPtr))) {
				break;
			}

			effAudioRemaining = dispatchPtr->audioRemaining;
			if (crossfadeBufferSize - dispatchPtr->fadeRemaining < effAudioRemaining)
				effAudioRemaining = crossfadeBufferSize - dispatchPtr->fadeRemaining;

			if (effAudioRemaining >= streamerGetFreeBufferAmount(dispatchPtr->streamPtr))
				effAudioRemaining = streamerGetFreeBufferAmount(dispatchPtr->streamPtr);

			if (effAudioRemaining >= 0x800)
				effAudioRemaining = 0x800;

			streamBuf = streamerGetStreamBuffer(dispatchPtr->streamPtr, effAudioRemaining);
			memcpy(&crossfadeBuffer[dispatchPtr->fadeRemaining], streamBuf, effAudioRemaining);

			dispatchPtr->fadeRemaining += effAudioRemaining;
			dispatchPtr->currentOffset += effAudioRemaining;
			dispatchPtr->audioRemaining -= effAudioRemaining;
		} while (dispatchPtr->fadeRemaining < crossfadeBufferSize);
	}

	streamerSetReadIndex(dispatchPtr->streamPtr, streamerGetFreeBufferAmount(dispatchPtr->streamPtr));
	streamerSetSoundToStreamFromOffset(dispatchPtr->streamPtr, newSoundId, vocLoopFlag ? offset : 0);

	if (vocLoopFlag) {
		if (dispatchPtr->vocLoopStartingPoint)
			streamerSetLoopFlag(dispatchPtr->streamPtr, dispatchPtr->audioRemaining + dispatchPtr->currentOffset);
	} else {
		streamerRemoveLoopFlag(dispatchPtr->streamPtr);
	}

	dispatchPtr->currentOffset = vocLoopFlag ? offset : 0;
	dispatchPtr->audioRemaining = vocLoopFlag ? audioRemaining : 0;

	if (!vocLoopFlag) {
		dispatchPtr->vocLoopStartingPoint = 0;
	}

	return 0;
}

void IMuseDigital::dispatchProcessDispatches(IMuseDigiTrack *trackPtr, int feedSize, int sampleRate) {
	IMuseDigiDispatch *dispatchPtr;
	int32 effFeedSize, effWordSize, effRemainingAudio, effRemainingFade, effSampleRate;
	int32 inFrameCount, mixVolume, mixStartingPoint, elapsedFadeDelta;
	int navigateMapResult;
	uint8 *srcBuf, *soundAddrData;

	dispatchPtr = trackPtr->dispatchPtr;
	if (dispatchPtr->streamPtr && dispatchPtr->streamZoneList)
		dispatchPredictStream(dispatchPtr);

	// If a fade has previously been allocated
	if (dispatchPtr->fadeBuf) {
		inFrameCount = 8 * dispatchPtr->fadeRemaining / (dispatchPtr->fadeWordSize * dispatchPtr->fadeChannelCount);

		if (_vm->_game.id == GID_DIG) {
			effSampleRate = dispatchPtr->fadeSampleRate;
		} else {
			effSampleRate = (trackPtr->pitchShift * dispatchPtr->fadeSampleRate) >> 8;
		}

		if (inFrameCount >= effSampleRate * feedSize / sampleRate) {
			inFrameCount = effSampleRate * feedSize / sampleRate;
			effFeedSize = feedSize;
		} else {
			effFeedSize = sampleRate * inFrameCount / effSampleRate;
		}

		if (dispatchPtr->fadeWordSize == 12 && dispatchPtr->fadeChannelCount == 1)
			inFrameCount &= 0xFFFFFFFE;

		// If the fade is still going on
		if (inFrameCount) {
			// Update the fade volume
			effRemainingFade = ((dispatchPtr->fadeWordSize * dispatchPtr->fadeChannelCount) * inFrameCount) / 8;
			mixVolume = dispatchUpdateFadeMixVolume(dispatchPtr, effRemainingFade);

			// Send it all to the mixer
			srcBuf = &dispatchPtr->fadeBuf[dispatchPtr->fadeOffset];

			_internalMixer->mix(
				srcBuf,
				inFrameCount,
				dispatchPtr->fadeWordSize,
				dispatchPtr->fadeChannelCount,
				effFeedSize,
				0,
				mixVolume,
				trackPtr->pan,
				false);

			dispatchPtr->fadeOffset += effRemainingFade;
			dispatchPtr->fadeRemaining -= effRemainingFade;

			// Deallocate fade if it ended
			if (!dispatchPtr->fadeRemaining) {
				dispatchDeallocateFade(dispatchPtr, "dispatchProcessDispatches");
			}
		} else {
			debug(5, "IMuseDigital::dispatchProcessDispatches(): WARNING: fade for sound %d ends with incomplete frame (or odd 12-bit mono frame)", trackPtr->soundId);

			// Fade ended, deallocate it
			dispatchDeallocateFade(dispatchPtr, "dispatchProcessDispatches");
		}

		if (!dispatchPtr->fadeRemaining)
			dispatchPtr->fadeBuf = nullptr;
	}

	// This index keeps track of the offset position until which we have
	// filled the buffer; with each update it is incremented by the effective
	// feed size.
	mixStartingPoint = 0;

	while (1) {
		// If the current region is finished playing
		// go check for any event on the map for the current offset
		if (!dispatchPtr->audioRemaining) {
			_dispatchFadeStartedFlag = 0;
			navigateMapResult = dispatchNavigateMap(dispatchPtr);

			if (navigateMapResult)
				break;

			if (_dispatchFadeStartedFlag) {
				// We reached a JUMP, therefore we have to crossfade to
				// the destination region: start a fade-out
				if (_vm->_game.id == GID_DIG) {
					effSampleRate = dispatchPtr->fadeSampleRate;
				} else {
					effSampleRate = (trackPtr->pitchShift * dispatchPtr->fadeSampleRate) >> 8;
				}

				inFrameCount = 8 * dispatchPtr->fadeRemaining / (dispatchPtr->fadeWordSize * dispatchPtr->fadeChannelCount);
				if (inFrameCount >= effSampleRate * feedSize / sampleRate) {
					inFrameCount = effSampleRate * feedSize / sampleRate;
					effFeedSize = feedSize;
				} else {
					effFeedSize = sampleRate * inFrameCount / effSampleRate;
				}

				if (dispatchPtr->fadeWordSize == 12 && dispatchPtr->fadeChannelCount == 1)
					inFrameCount &= 0xFFFFFFFE;

				if (!inFrameCount)
					debug(5, "IMuseDigital::dispatchProcessDispatches(): WARNING: fade for sound %d ends with incomplete frame (or odd 12-bit mono frame)", trackPtr->soundId);

				// Update the fade volume
				effRemainingFade = (inFrameCount * dispatchPtr->fadeWordSize * dispatchPtr->fadeChannelCount) / 8;
				mixVolume = dispatchUpdateFadeMixVolume(dispatchPtr, effRemainingFade);

				// Send it all to the mixer
				srcBuf = &dispatchPtr->fadeBuf[dispatchPtr->fadeOffset];

				_internalMixer->mix(
					srcBuf,
					inFrameCount,
					dispatchPtr->fadeWordSize,
					dispatchPtr->fadeChannelCount,
					effFeedSize,
					mixStartingPoint,
					mixVolume,
					trackPtr->pan,
					false);

				dispatchPtr->fadeOffset += effRemainingFade;
				dispatchPtr->fadeRemaining -= effRemainingFade;

				if (!dispatchPtr->fadeRemaining) {
					// Fade ended, deallocate it
					dispatchDeallocateFade(dispatchPtr, "dispatchProcessDispatches");
				}
			}
		}

		if (!feedSize)
			return;

		if (_vm->_game.id == GID_DIG) {
			effSampleRate = dispatchPtr->sampleRate;
		} else {
			effSampleRate = (trackPtr->pitchShift * dispatchPtr->sampleRate) >> 8;
		}

		effWordSize = dispatchPtr->channelCount * dispatchPtr->wordSize;
		inFrameCount = effSampleRate * feedSize / sampleRate;

		if (inFrameCount <= (8 * dispatchPtr->audioRemaining / effWordSize)) {
			effFeedSize = feedSize;
		} else {
			inFrameCount = 8 * dispatchPtr->audioRemaining / effWordSize;
			effFeedSize = sampleRate * (8 * dispatchPtr->audioRemaining / effWordSize) / effSampleRate;
		}

		if (dispatchPtr->wordSize == 12 && dispatchPtr->channelCount == 1)
			inFrameCount &= 0xFFFFFFFE;

		if (!inFrameCount) {
			if (_vm->_game.id == GID_DIG || dispatchPtr->wordSize == 12)
				debug(5, "IMuseDigital::dispatchProcessDispatches(): WARNING: region in sound %d ends with incomplete frame (or odd 12-bit mono frame)", trackPtr->soundId);
			tracksClear(trackPtr);
			return;
		}

		// Play the audio of the current region
		effRemainingAudio = (effWordSize * inFrameCount) / 8;

		if (dispatchPtr->streamPtr) {
			srcBuf = streamerGetStreamBuffer(dispatchPtr->streamPtr, effRemainingAudio);
			if (!srcBuf) {
				dispatchPtr->streamErrFlag = 1;
				if (dispatchPtr->fadeBuf && dispatchPtr->fadeSyncFlag)
					dispatchPtr->fadeSyncDelta += feedSize;

				streamerQueryStream(
					dispatchPtr->streamPtr,
					_dispatchCurStreamBufSize,
					_dispatchCurStreamCriticalSize,
					_dispatchCurStreamFreeSpace,
					_dispatchCurStreamPaused);

				if (_dispatchCurStreamPaused) {
					debug(5, "IMuseDigital::dispatchProcessDispatches(): WARNING: stopping starving paused stream for sound %d", dispatchPtr->trackPtr->soundId);
					tracksClear(trackPtr);
				}

				return;
			}
			dispatchPtr->streamZoneList->offset += effRemainingAudio;
			dispatchPtr->streamZoneList->size -= effRemainingAudio;
			dispatchPtr->streamErrFlag = 0;
		} else {
			soundAddrData = _filesHandler->getSoundAddrData(trackPtr->soundId);
			if (!soundAddrData) {
				debug(5, "IMuseDigital::dispatchProcessDispatches(): ERROR: soundAddrData for sound %d is NULL", trackPtr->soundId);
				// Try to gracefully play nothing instead of getting stuck on an infinite loop
				dispatchPtr->currentOffset += effRemainingAudio;
				dispatchPtr->audioRemaining -= effRemainingAudio;
				return;
			}

			srcBuf = &soundAddrData[dispatchPtr->currentOffset];
		}

		if (dispatchPtr->fadeBuf) {
			// If the fadeSyncFlag is active (e.g. we are crossfading
			// to another version of the same music piece), do this... thing,
			// and update the fadeSyncDelta
			if (dispatchPtr->fadeSyncFlag) {
				if (dispatchPtr->fadeSyncDelta) {
					elapsedFadeDelta = effFeedSize;
					if (effFeedSize >= dispatchPtr->fadeSyncDelta)
						elapsedFadeDelta = dispatchPtr->fadeSyncDelta;

					dispatchPtr->fadeSyncDelta -= elapsedFadeDelta;
					effFeedSize -= elapsedFadeDelta;

					if (_vm->_game.id == GID_DIG) {
						effSampleRate = dispatchPtr->sampleRate;
					} else {
						effSampleRate = (trackPtr->pitchShift * dispatchPtr->sampleRate) >> 8;
					}

					inFrameCount = effFeedSize * effSampleRate / sampleRate;

					if (dispatchPtr->wordSize == 12 && dispatchPtr->channelCount == 1)
						inFrameCount &= 0xFFFFFFFE;

					srcBuf = &srcBuf[effRemainingAudio - ((dispatchPtr->wordSize * inFrameCount * dispatchPtr->channelCount) / 8)];
				}
			}

			// If there's still a fadeBuffer active in our dispatch
			// we balance the volume of the considered track with
			// the fade volume, effectively creating a crossfade
			if (dispatchPtr->fadeBuf) {
				// Fade-in
				mixVolume = dispatchUpdateFadeSlope(dispatchPtr);
			} else {
				mixVolume = trackPtr->effVol;
			}
		} else {
			mixVolume = trackPtr->effVol;
		}

		// Real-time lo-fi Radio voice effect
		if (trackPtr->mailbox)
			_internalMixer->setRadioChatter();

		_internalMixer->mix(
			srcBuf,
			inFrameCount,
			dispatchPtr->wordSize,
			dispatchPtr->channelCount,
			effFeedSize,
			mixStartingPoint,
			mixVolume,
			trackPtr->pan,
			false);

		_internalMixer->clearRadioChatter();
		mixStartingPoint += effFeedSize;
		feedSize -= effFeedSize;

		dispatchPtr->currentOffset += effRemainingAudio;
		dispatchPtr->audioRemaining -= effRemainingAudio;
	}

	// Behavior of errors and STOP marker
	if (navigateMapResult == -1)
		tracksClear(trackPtr);

	if (dispatchPtr->fadeBuf && dispatchPtr->fadeSyncFlag)
		dispatchPtr->fadeSyncDelta += feedSize;
}

void IMuseDigital::dispatchProcessDispatches(IMuseDigiTrack *trackPtr, int feedSize) {
	IMuseDigiDispatch *dispatchPtr = nullptr;
	IMuseDigiStream *streamPtr;
	uint8 *buffer, *srcBuf;
	int32 fadeChunkSize = 0;
	int32 tentativeFeedSize, inFrameCount, fadeSyncDelta, mixStartingPoint, seekResult;
	int mixVolume;

	dispatchPtr = trackPtr->dispatchPtr;
	tentativeFeedSize = (dispatchPtr->sampleRate == 22050) ? feedSize : feedSize / 2;

	if (dispatchPtr->fadeBuf) {
		if (tentativeFeedSize >= dispatchPtr->fadeRemaining) {
			fadeChunkSize = dispatchPtr->fadeRemaining;
		} else {
			fadeChunkSize = tentativeFeedSize;
		}

		mixVolume = dispatchUpdateFadeMixVolume(dispatchPtr, fadeChunkSize);
		_internalMixer->mix(dispatchPtr->fadeBuf, fadeChunkSize, 8, 1, feedSize, 0, mixVolume, trackPtr->pan, (dispatchPtr->sampleRate == 11025));
		dispatchPtr->fadeRemaining -= fadeChunkSize;
		dispatchPtr->fadeBuf += fadeChunkSize;
		if (dispatchPtr->fadeRemaining == fadeChunkSize)
			dispatchPtr->fadeBuf = nullptr;
	}

	mixStartingPoint = 0;
	while (1) {
		if (!dispatchPtr->audioRemaining) {
			seekResult = dispatchSeekToNextChunk(dispatchPtr);
			if (seekResult)
				break;
		}

		if (!tentativeFeedSize)
			return;

		inFrameCount = dispatchPtr->audioRemaining;
		if (tentativeFeedSize < inFrameCount)
			inFrameCount = tentativeFeedSize;

		streamPtr = dispatchPtr->streamPtr;
		if (streamPtr) {
			buffer = streamerGetStreamBuffer(streamPtr, inFrameCount);
			if (!buffer) {
				if (dispatchPtr->fadeBuf)
					dispatchPtr->fadeSyncDelta += fadeChunkSize;
				return;
			}
		} else {
			srcBuf = _filesHandler->getSoundAddrData(trackPtr->soundId);
			if (!srcBuf)
				return;
			buffer = &srcBuf[dispatchPtr->currentOffset];
		}

		if (dispatchPtr->fadeBuf) {
			if (dispatchPtr->fadeSyncDelta) {
				fadeSyncDelta = dispatchPtr->fadeSyncDelta;
				if (dispatchPtr->fadeSyncDelta >= inFrameCount)
					fadeSyncDelta = inFrameCount;
				inFrameCount -= fadeSyncDelta;
				dispatchPtr->fadeSyncDelta -= fadeSyncDelta;
				buffer += fadeSyncDelta;
				dispatchPtr->currentOffset += fadeSyncDelta;
				dispatchPtr->audioRemaining -= fadeSyncDelta;
			}
		}

		if (inFrameCount) {
			if (dispatchPtr->fadeBuf) {
				mixVolume = dispatchUpdateFadeSlope(dispatchPtr);
			} else {
				mixVolume = trackPtr->effVol;
			}

			_internalMixer->mix(buffer, inFrameCount, 8, 1, feedSize, mixStartingPoint, mixVolume, trackPtr->pan, (dispatchPtr->sampleRate == 11025));
			mixStartingPoint += inFrameCount;
			tentativeFeedSize -= inFrameCount;
			dispatchPtr->currentOffset += inFrameCount;
			dispatchPtr->audioRemaining -= inFrameCount;
		}
	}

	if (seekResult == -1)
		tracksClear(trackPtr);

	if (dispatchPtr->fadeBuf)
		dispatchPtr->fadeSyncDelta += fadeChunkSize;
}

void IMuseDigital::dispatchPredictFirstStream() {
	Common::StackLock lock(_mutex);

	for (int i = 0; i < _trackCount; i++) {
		if (_dispatches[i].trackPtr->soundId && _dispatches[i].streamPtr && _dispatches[i].streamZoneList)
			dispatchPredictStream(&_dispatches[i]);
	}
}

int IMuseDigital::dispatchNavigateMap(IMuseDigiDispatch *dispatchPtr) {
	uint8 *mapCurEvent;
	int32 blockTag, effFadeSize, elapsedFadeSize, regionOffset;
	char *marker = NULL;

	int getMapResult = dispatchGetMap(dispatchPtr);
	if (getMapResult)
		return getMapResult;

	if (dispatchPtr->audioRemaining
		|| (dispatchPtr->streamPtr && dispatchPtr->streamZoneList->offset != dispatchPtr->currentOffset)) {
		debug(5, "IMuseDigital::dispatchNavigateMap(): ERROR: navigation error in dispatch");
		return -1;
	}

	mapCurEvent = NULL;
	while (1) {
		mapCurEvent = dispatchGetNextMapEvent(dispatchPtr->map, dispatchPtr->currentOffset, mapCurEvent);

		if (!mapCurEvent) {
			debug(5, "IMuseDigital::dispatchNavigateMap(): ERROR: no more map events at offset %dx", dispatchPtr->currentOffset);
			return -1;
		}

		blockTag = READ_UINT32(mapCurEvent);
		switch (blockTag) {
		case MKTAG('J', 'U', 'M', 'P'):
			// Handle any event found at this offset
			// Jump block (fixed size: 28 bytes)
			// - The tag 'JUMP' (4 bytes)
			// - Block size in bytes minus 8 (4 bytes)
			// - Block offset (hook position) (4 bytes)
			// - Jump destination offset (4 bytes)
			// - Hook ID (4 bytes)
			// - Fade time in ms (4 bytes)
			if (!checkHookId(dispatchPtr->trackPtr->jumpHook, READ_UINT32(mapCurEvent + 16))) {
				// This is the right hookId, let's jump
				dispatchPtr->currentOffset = READ_UINT32(mapCurEvent + 12);
				if (dispatchPtr->streamPtr) {
					if (dispatchPtr->streamZoneList->size || !dispatchPtr->streamZoneList->next) {
						debug(5, "IMuseDigital::dispatchNavigateMap(): next streamZone is unallocated, calling dispatchPrepareToJump()");
						dispatchPrepareToJump(dispatchPtr, dispatchPtr->streamZoneList, mapCurEvent, 1);
					}

					debug(5, "IMuseDigital::dispatchNavigateMap(): \n"
							 "\tJUMP found for sound %d with valid candidateHookId (%d), \n"
							 "\tgoing to offset %d with a crossfade of %d ms",
						  dispatchPtr->trackPtr->soundId, (int)READ_UINT32(mapCurEvent + 16),
						  (int)READ_UINT32(mapCurEvent + 12), (int)READ_UINT32(mapCurEvent + 20));

					dispatchPtr->streamZoneList->useFlag = 0;
					removeStreamZoneFromList(&dispatchPtr->streamZoneList, dispatchPtr->streamZoneList);

					if (dispatchPtr->streamZoneList->fadeFlag) {
						if (dispatchPtr->fadeBuf) {
							// Mark the fade corresponding to our fadeBuf as unused
							dispatchDeallocateFade(dispatchPtr, "dispatchNavigateMap");
						}

						_dispatchJumpFadeSize = dispatchPtr->streamZoneList->size;
						dispatchPtr->fadeBuf = dispatchAllocateFade(_dispatchJumpFadeSize, "dispatchNavigateMap");

						// If the fade buffer is allocated
						// set up the fade
						if (dispatchPtr->fadeBuf) {
							dispatchPtr->fadeWordSize = dispatchPtr->wordSize;
							dispatchPtr->fadeSampleRate = dispatchPtr->sampleRate;
							dispatchPtr->fadeChannelCount = dispatchPtr->channelCount;
							dispatchPtr->fadeOffset = 0;
							dispatchPtr->fadeRemaining = 0;
							dispatchPtr->fadeSyncFlag = 0;
							dispatchPtr->fadeSyncDelta = 0;
							dispatchPtr->fadeVol = DIMUSE_MAX_FADE_VOLUME;
							dispatchPtr->fadeSlope = 0;

							// Clone the old sound in the fade buffer for just the duration of the fade
							if (_dispatchJumpFadeSize) {
								do {
									effFadeSize = _dispatchJumpFadeSize - dispatchPtr->fadeRemaining;
									if ((_dispatchJumpFadeSize - dispatchPtr->fadeRemaining) >= 0x4000)
										effFadeSize = 0x4000;

									memcpy(&dispatchPtr->fadeBuf[dispatchPtr->fadeRemaining],
										   streamerGetStreamBuffer(dispatchPtr->streamPtr, effFadeSize),
										   effFadeSize);

									elapsedFadeSize = effFadeSize + dispatchPtr->fadeRemaining;
									dispatchPtr->fadeRemaining = elapsedFadeSize;
								} while (_dispatchJumpFadeSize > elapsedFadeSize);
							}
							_dispatchFadeStartedFlag = 1;
						}
						dispatchPtr->streamZoneList->useFlag = 0;
						removeStreamZoneFromList(&dispatchPtr->streamZoneList, dispatchPtr->streamZoneList);
					}
				}
				mapCurEvent = nullptr;
			}

			continue;
		case MKTAG('S', 'Y', 'N', 'C'):
			// SYNC block (fixed size: x bytes)
			// - The tag 'SYNC' (4 bytes)
			// - SYNC size in bytes (4 bytes)
			// - SYNC data (variable length)

			// It is possible to gather a total maximum of 4 SYNCs for a single track;
			// this is not a problem however, as speech files only have one SYNC block,
			// and the most we get is four (one for each character) in
			// A Pirate I Was Meant To Be, in Part 3 of COMI

			// Curiously we skip the first four bytes of data, ending up having the first
			// four bytes of the next block in our syncPtr; but this is exactly what happens
			// within the interpreter, so I'm not going to argue with it

			if (!dispatchPtr->trackPtr->syncPtr_0) {
				dispatchPtr->trackPtr->syncPtr_0 = (byte *)malloc(READ_UINT32(mapCurEvent + 4));
				memcpy(dispatchPtr->trackPtr->syncPtr_0, mapCurEvent + 3 * 4, READ_UINT32(mapCurEvent + 4));
				dispatchPtr->trackPtr->syncSize_0 = READ_UINT32(mapCurEvent + 4);

			} else if (!dispatchPtr->trackPtr->syncPtr_1) {
				dispatchPtr->trackPtr->syncPtr_1 = (byte *)malloc(READ_UINT32(mapCurEvent + 4));
				memcpy(dispatchPtr->trackPtr->syncPtr_1, mapCurEvent + 3 * 4, READ_UINT32(mapCurEvent + 4));
				dispatchPtr->trackPtr->syncSize_1 = READ_UINT32(mapCurEvent + 4);

			} else if (!dispatchPtr->trackPtr->syncPtr_2) {
				dispatchPtr->trackPtr->syncPtr_2 = (byte *)malloc(READ_UINT32(mapCurEvent + 4));
				memcpy(dispatchPtr->trackPtr->syncPtr_2, mapCurEvent + 3 * 4, READ_UINT32(mapCurEvent + 4));
				dispatchPtr->trackPtr->syncSize_2 = READ_UINT32(mapCurEvent + 4);

			} else if (!dispatchPtr->trackPtr->syncPtr_3) {
				dispatchPtr->trackPtr->syncPtr_3 = (byte *)malloc(READ_UINT32(mapCurEvent + 4));
				memcpy(dispatchPtr->trackPtr->syncPtr_3, mapCurEvent + 3 * 4, READ_UINT32(mapCurEvent + 4));
				dispatchPtr->trackPtr->syncSize_3 = READ_UINT32(mapCurEvent + 4);
			}

			continue;
		case MKTAG('F', 'R', 'M', 'T'):
			// Format block (fixed size: 28 bytes)
			// - The tag 'FRMT' (4 bytes)
			// - Block size in bytes minus 8 (4 bytes)
			// - Block offset (4 bytes)
			// - Empty field (4 bytes) (which is set to 1 in Grim Fandango, I suspect this is the endianness)
			// - Word size between 8, 12 and 16 (4 bytes)
			// - Sample rate (4 bytes)
			// - Number of channels (4 bytes)
			dispatchPtr->wordSize = READ_UINT32(mapCurEvent + 16);
			dispatchPtr->sampleRate = READ_UINT32(mapCurEvent + 20);
			dispatchPtr->channelCount = READ_UINT32(mapCurEvent + 24);

			continue;
		case MKTAG('R', 'E', 'G', 'N'):
			// Region block (fixed size: 16 bytes)
			// - The tag 'REGN' (4 bytes)
			// - Block size in bytes minus 8 (4 bytes)
			// - Block offset (4 bytes)
			// - Region length (4 bytes)
			regionOffset = READ_UINT32(mapCurEvent + 8);
			if (regionOffset == dispatchPtr->currentOffset) {
				dispatchPtr->audioRemaining = READ_UINT32(mapCurEvent + 12);
				return 0;
			} else {
				debug(5, "IMuseDigital::dispatchNavigateMap(): ERROR: region offset %d != currentOffset %d", regionOffset, dispatchPtr->currentOffset);
				return -1;
			}
		case MKTAG('S', 'T', 'O', 'P'):
			// Stop block (fixed size: 12 bytes)
			// Contains:
			// - The tag 'STOP' (4 bytes)
			// - Block size in bytes minus 8 (4 bytes)
			// - Block offset (4 bytes)
			return -1;
		case MKTAG('T', 'E', 'X', 'T'):
			// Marker block (variable size)
			// Contains:
			// - The tag 'TEXT' (4 bytes)
			// - Block size in bytes minus 8 (4 bytes)
			// - Block offset (4 bytes)
			// - A string of characters ending with '\0' (variable length)
			marker = (char *)mapCurEvent + 12;
			_triggersHandler->processTriggers(dispatchPtr->trackPtr->soundId, marker);
			if (dispatchPtr->audioRemaining)
				return 0;

			continue;
		default:
			debug(5, "IMuseDigital::dispatchNavigateMap(): ERROR: Unrecognized map event at offset %dx", dispatchPtr->currentOffset);
			break;
		};
	}
	return -1;
}

int IMuseDigital::dispatchGetMap(IMuseDigiDispatch *dispatchPtr) {
	int32 *dstMap;
	uint8 *rawMap, *copiedBuf, *soundAddrData;
	int32 size;

	dstMap = dispatchPtr->map;

	// If there's no map, try to fetch it
	if (dispatchPtr->map[0] != MKTAG('M', 'A', 'P', ' ')) {
		if (dispatchPtr->currentOffset) {
			debug(5, "IMuseDigital::dispatchNavigateMap(): found offset but no map");
			return -1;
		}

		// If there's a streamPtr it means that this is a sound loaded
		// from a bundle (either music or speech)
		if (dispatchPtr->streamPtr) {
			copiedBuf = (uint8 *)streamerGetStreamBufferAtOffset(dispatchPtr->streamPtr, 0, 0x10);

			if (!copiedBuf) {
				return -3;
			}

			if (READ_BE_UINT32(copiedBuf) == MKTAG('i', 'M', 'U', 'S') && READ_BE_UINT32(copiedBuf + 8) == MKTAG('M', 'A', 'P', ' ')) {
				size = READ_BE_UINT32(copiedBuf + 12) + 24;
				if (!streamerGetStreamBufferAtOffset(dispatchPtr->streamPtr, 0, size)) {
					return -3;
				}
				rawMap = (uint8 *)streamerGetStreamBuffer(dispatchPtr->streamPtr, size);
				if (!rawMap) {
					debug(5, "IMuseDigital::dispatchGetMap(): ERROR: stream read failed after view succeeded");
					return -1;
				}

				dispatchPtr->currentOffset = size;
				if (dispatchConvertMap(rawMap + 8, dstMap)) {
					debug(5, "IMuseDigital::dispatchGetMap(): ERROR: dispatchConvertMap() failed");
					return -1;
				}

				if (dispatchPtr->map[2] != MKTAG('F', 'R', 'M', 'T')) {
					debug(5, "IMuseDigital::dispatchGetMap(): ERROR: expected 'FRMT' at start of map");
					return -1;
				}

				if (dispatchPtr->map[4] != dispatchPtr->currentOffset) {
					debug(5, "IMuseDigital::dispatchGetMap(): ERROR: expected data to follow map");
					return -1;
				} else {
					if (dispatchPtr->streamZoneList) {
						debug(5, "IMuseDigital::dispatchGetMap(): ERROR: expected NULL streamZoneList");
						return -1;
					}

					dispatchPtr->streamZoneList = dispatchAllocateStreamZone();
					if (!dispatchPtr->streamZoneList) {
						debug(5, "IMuseDigital::dispatchGetMap(): ERROR: couldn't allocate zone");
						return -1;
					}

					dispatchPtr->streamZoneList->offset = dispatchPtr->currentOffset;
					dispatchPtr->streamZoneList->size = streamerGetFreeBufferAmount(dispatchPtr->streamPtr);
					dispatchPtr->streamZoneList->fadeFlag = 0;
				}
			} else {
				debug(5, "IMuseDigital::dispatchGetMap(): ERROR: unrecognized file format in stream buffer");
				return -1;
			}

		} else {
			// Otherwise, this is a SFX and we must load it using its resource pointer
			soundAddrData = _filesHandler->getSoundAddrData(dispatchPtr->trackPtr->soundId);

			if (!soundAddrData) {
				debug(5, "IMuseDigital::dispatchGetMap(): ERROR: couldn't get sound address");
				return -1;
			}

			if (READ_BE_UINT32(soundAddrData) == MKTAG('i', 'M', 'U', 'S') && READ_BE_UINT32(soundAddrData + 8) == MKTAG('M', 'A', 'P', ' ')) {
				dispatchPtr->currentOffset = READ_BE_UINT32(soundAddrData + 12) + 24;
				if (dispatchConvertMap((soundAddrData + 8), dstMap)) {
					debug(5, "IMuseDigital::dispatchGetMap(): ERROR: dispatchConvertMap() failure");
					return -1;
				}

				if (dispatchPtr->map[2] != MKTAG('F', 'R', 'M', 'T')) {
					debug(5, "IMuseDigital::dispatchGetMap(): ERROR: expected 'FRMT' at start of map");
					return -1;
				}

				if (dispatchPtr->map[4] != dispatchPtr->currentOffset) {
					debug(5, "IMuseDigital::dispatchGetMap(): ERROR: expected data to follow map");
					return -1;
				}
			} else {
				debug(5, "IMuseDigital::dispatchGetMap(): ERROR: unrecognized file format in stream buffer");
				return -1;
			}
		}
	}

	return 0;
}

int IMuseDigital::dispatchConvertMap(uint8 *rawMap, int32 *destMap) {
	int32 effMapSize;
	uint8 *mapCurPos;
	int32 blockName;
	uint8 *blockSizePtr;
	uint32 blockSizeMin8;
	uint8 *firstChar;
	uint8 *otherChars;
	uint32 remainingFieldsNum;
	int32 bytesUntilEndOfMap;
	uint8 *endOfMapPtr;

	if (READ_BE_UINT32(rawMap) == MKTAG('M', 'A', 'P', ' ')) {
		bytesUntilEndOfMap = READ_BE_UINT32(rawMap + 4);
		effMapSize = bytesUntilEndOfMap + 8;
		if (((_vm->_game.id == GID_DIG
			|| (_vm->_game.id == GID_CMI && _vm->_game.features & GF_DEMO)) && effMapSize <= 0x400)
			|| (_vm->_game.id == GID_CMI && effMapSize <= 0x2000)) {
			memcpy(destMap, rawMap, effMapSize);

			// Fill (or rather, swap32) the fields:
			// - The 4 bytes string 'MAP '
			// - Size of the map
			destMap[0] = READ_BE_UINT32(destMap);
			destMap[1] = READ_BE_UINT32(destMap + 4);

			mapCurPos = (uint8 *)destMap + 8;
			endOfMapPtr = (uint8 *)destMap + effMapSize;

			// Swap32 the rest of the map
			while (mapCurPos < endOfMapPtr) {
				// Swap32 the 4 characters block name
				int32 swapped = READ_BE_UINT32(mapCurPos);
				memcpy(mapCurPos, &swapped, 4);
				blockName = swapped;

				// Advance and Swap32 the block size (minus 8) field
				blockSizePtr = mapCurPos + 4;
				blockSizeMin8 = READ_BE_UINT32(blockSizePtr);
				memcpy(blockSizePtr, &blockSizeMin8, 4);
				mapCurPos = blockSizePtr + 4;

				// Swapping32 a TEXT block is different:
				// it also contains single characters, so we skip them
				// since they're already good like this
				if (blockName == MKTAG('T', 'E', 'X', 'T')) {
					// Swap32 the block offset position
					swapped = READ_BE_UINT32(mapCurPos);
					memcpy(mapCurPos, &swapped, 4);

					// Skip the single characters
					firstChar = mapCurPos + 4;
					mapCurPos += 5;
					if (*firstChar) {
						do {
							otherChars = mapCurPos++;
						} while (*otherChars);
					}
				} else if ((blockSizeMin8 & 0xFFFFFFFC) != 0) {
					// Basically divide by 4 to retrieve the number
					// of fields to swap
					remainingFieldsNum = blockSizeMin8 >> 2;

					// ...and swap them of course
					do {
						swapped = READ_BE_UINT32(mapCurPos);
						memcpy(mapCurPos, &swapped, 4);
						mapCurPos += 4;
						--remainingFieldsNum;
					} while (remainingFieldsNum);
				}
			}

			// Just a sanity check to see if we've parsed the whole map
			if ((uint8 *)destMap + bytesUntilEndOfMap - mapCurPos == -8) {
				return 0;
			} else {
				debug(5, "IMuseDigital::dispatchConvertMap(): ERROR: converted wrong number of bytes");
				return -1;
			}
		} else {
			debug(5, "IMuseDigital::dispatchConvertMap(): ERROR: map is too big (%d)", effMapSize);
			return -1;
		}
	} else {
		debug(5, "IMuseDigital::dispatchConvertMap(): ERROR: got bogus map");
		return -1;
	}

	return 0;
}

uint8 *IMuseDigital::dispatchGetNextMapEvent(int32 *mapPtr, int32 soundOffset, uint8 *mapEvent) {
	if (mapEvent) {
		// Advance the map to the next block (READ_UINT32(mapEvent + 4) + 8 is the size of the block)
		mapEvent = mapEvent + READ_UINT32(mapEvent + 4) + 8;

		if ((uint8 *)&mapPtr[2] + mapPtr[1] > mapEvent) {
			if ((int32)READ_UINT32(mapEvent + 8) != soundOffset) {
				debug(5, "IMuseDigital::dispatchGetNextMapEvent(): ERROR: no more events at offset %d", soundOffset);
				return nullptr;
			}
		} else {
			debug(5, "IMuseDigital::dispatchGetNextMapEvent(): ERROR: map overrun");
			return nullptr;
		}

	} else {
		// Init the current map position starting from the first block
		// (cells 0 and 1 are the tag 'MAP ' and the map size respectively)
		mapEvent = (uint8 *)&mapPtr[2];

		// Search for the block with the same offset as ours
		while ((int32)READ_UINT32(mapEvent + 8) != soundOffset) {
			// Check if we've overrun the offset, to make sure
			// that there actually is an event at our offset
			mapEvent = mapEvent + READ_UINT32(mapEvent + 4) + 8;

			if ((uint8 *)&mapPtr[2] + mapPtr[1] <= mapEvent) {
				debug(5, "IMuseDigital::dispatchGetNextMapEvent(): ERROR: couldn't find event at offset %d", soundOffset);
				return nullptr;
			}
		}
	}

	return mapEvent;
}

void IMuseDigital::dispatchPredictStream(IMuseDigiDispatch *dispatchPtr) {
	IMuseDigiStreamZone *szTmp, *lastStreamInList, *curStrZn;
	int32 cumulativeStreamOffset;
	uint8 *jumpParameters;

	if (!dispatchPtr->streamPtr || !dispatchPtr->streamZoneList) {
		debug(5, "IMuseDigital::dispatchPredictStream(): ERROR: NULL streamId or streamZoneList");
		return;
	}

	szTmp = dispatchPtr->streamZoneList;

	// Get the offset which our stream is currently at
	cumulativeStreamOffset = 0;
	do {
		cumulativeStreamOffset += szTmp->size;
		lastStreamInList = szTmp;
		szTmp = szTmp->next;
	} while (szTmp);

	lastStreamInList->size += streamerGetFreeBufferAmount(dispatchPtr->streamPtr) - cumulativeStreamOffset;
	curStrZn = dispatchPtr->streamZoneList;

	for (_dispatchBufferedHookId = dispatchPtr->trackPtr->jumpHook; curStrZn; curStrZn = curStrZn->next) {
		if (!curStrZn->fadeFlag) {
			jumpParameters = dispatchCheckForJump(dispatchPtr->map, curStrZn, _dispatchBufferedHookId);
			if (jumpParameters) {
				// If we've reached a JUMP and it's successful, allocate the streamZone of the destination
				dispatchPrepareToJump(dispatchPtr, curStrZn, jumpParameters, 0);
			} else {
				// If we don't have to jump, just play the next streamZone available
				dispatchStreamNextZone(dispatchPtr, curStrZn);
			}
		}
	}
}

uint8 *IMuseDigital::dispatchCheckForJump(int32 *mapPtr, IMuseDigiStreamZone *strZnPtr, int &candidateHookId) {
	uint8 *curMapPlace = (uint8 *)&mapPtr[2];
	uint8 *endOfMap = (uint8 *)&mapPtr[2] + mapPtr[1];
	int32 mapPlaceTag, jumpHookPos, jumpHookId, bytesUntilNextPlace;

	while (curMapPlace < endOfMap) {
		mapPlaceTag = READ_UINT32(curMapPlace);
		bytesUntilNextPlace = READ_UINT32(curMapPlace + 4) + 8;

		if (mapPlaceTag == MKTAG('J', 'U', 'M', 'P')) {
			jumpHookPos = READ_UINT32(curMapPlace + 8);
			jumpHookId = READ_UINT32(curMapPlace + 16);

			if (jumpHookPos > strZnPtr->offset && jumpHookPos <= strZnPtr->size + strZnPtr->offset) {
				if (!checkHookId(candidateHookId, jumpHookId))
					return curMapPlace;
			}
		}
		// Advance the map to the next place
		curMapPlace = curMapPlace + bytesUntilNextPlace;
	}

	return nullptr;
}

void IMuseDigital::dispatchPrepareToJump(IMuseDigiDispatch *dispatchPtr, IMuseDigiStreamZone *strZnPtr, uint8 *jumpParams, int calledFromNavigateMap) {
	int32 hookPosition, jumpDestination, fadeTime;
	IMuseDigiStreamZone *nextStreamZone;
	IMuseDigiStreamZone *zoneForJump = nullptr;
	IMuseDigiStreamZone *zoneAfterJump;
	uint32 streamOffset;
	IMuseDigiStreamZone *zoneCycle;

	// jumpParams format (assuming jumpParams is int32*):
	// jumpParams[0]: four bytes which form the string 'JUMP'
	// jumpParams[1]: block size in bytes minus 8 (16 for a JUMP block like this one; total == 24 bytes)
	// jumpParams[2]: hook position
	// jumpParams[3]: jump destination
	// jumpParams[4]: hook ID
	// jumpParams[5]: fade time in milliseconds

	hookPosition = READ_UINT32(jumpParams + 8);
	jumpDestination = READ_UINT32(jumpParams + 12);
	fadeTime = READ_UINT32(jumpParams + 20);

	// Edge cases handling
	if (strZnPtr->size + strZnPtr->offset == hookPosition) {
		nextStreamZone = strZnPtr->next;
		if (nextStreamZone) {
			if (nextStreamZone->fadeFlag) {
				// Avoid jumping if the next stream zone is already fading
				// and its ending position is our jump destination.
				// Basically: cancel the jump if there's already a fade in progress
				if (nextStreamZone->offset == hookPosition) {
					if (nextStreamZone->next) {
						if (nextStreamZone->next->offset == jumpDestination)
							return;
					}
				}
			} else {
				// Avoid jumping if we're trying to jump to the next stream zone
				if (nextStreamZone->offset == jumpDestination)
					return;
			}
		}
	}

	// Maximum size of the dispatch for the fade (in bytes)
	_dispatchSize = dispatchGetFadeSize(dispatchPtr, fadeTime);

	// If this function is being called from dispatchPredictStream,
	// avoid accepting an oversized dispatch
	if (!calledFromNavigateMap) {
		if (_dispatchSize > strZnPtr->size + strZnPtr->offset - hookPosition)
			return;
	}

	// Cap the dispatch size, if oversized
	if (_dispatchSize > strZnPtr->size + strZnPtr->offset - hookPosition)
		_dispatchSize = strZnPtr->size + strZnPtr->offset - hookPosition;

	// This prevents starting a fade with an inverted stereo image
	dispatchValidateFadeSize(dispatchPtr, _dispatchSize, "dispatchPrepareToJump");

	if (_vm->_game.id == GID_DIG) {
		if (hookPosition < jumpDestination)
			_dispatchSize = 0;
	} else {
		if (dispatchPtr->fadeRemaining)
			_dispatchSize = 0;
	}

	// Try allocating the two streamZones needed for the jump
	if (_dispatchSize) {
		zoneForJump = dispatchAllocateStreamZone();
		if (!zoneForJump) {
			debug(5, "IMuseDigital::dispatchPrepareToJump(): ERROR: couldn't allocate streamZone");
			return;
		}
	}

	zoneAfterJump = dispatchAllocateStreamZone();
	if (!zoneAfterJump) {
		debug(5, "IMuseDigital::dispatchPrepareToJump(): ERROR: couldn't allocate streamZone");
		return;
	}

	strZnPtr->size = hookPosition - strZnPtr->offset;
	streamOffset = hookPosition - strZnPtr->offset + _dispatchSize;

	// Go to the interested stream zone to calculate the stream offset,
	// and schedule the sound to stream with that offset
	zoneCycle = dispatchPtr->streamZoneList;
	while (zoneCycle != strZnPtr) {
		streamOffset += zoneCycle->size;
		zoneCycle = zoneCycle->next;
	}

	streamerSetLoadIndex(dispatchPtr->streamPtr, streamOffset);

	while (strZnPtr->next) {
		strZnPtr->next->useFlag = 0;
		removeStreamZoneFromList(&strZnPtr->next, strZnPtr->next);
	}

	streamerSetSoundToStreamFromOffset(dispatchPtr->streamPtr,
		dispatchPtr->trackPtr->soundId, jumpDestination);

	// Prepare the fading zone for the jump
	// and also a subsequent empty dummy zone
	if (_dispatchSize) {
		strZnPtr->next = zoneForJump;
		zoneForJump->prev = strZnPtr;
		strZnPtr = zoneForJump;
		zoneForJump->next = 0;
		zoneForJump->offset = hookPosition;
		zoneForJump->size = _dispatchSize;
		zoneForJump->fadeFlag = 1;
	}

	strZnPtr->next = zoneAfterJump;
	zoneAfterJump->prev = strZnPtr;
	zoneAfterJump->next = nullptr;
	zoneAfterJump->offset = jumpDestination;
	zoneAfterJump->size = 0;
	zoneAfterJump->fadeFlag = 0;
}

void IMuseDigital::dispatchStreamNextZone(IMuseDigiDispatch *dispatchPtr, IMuseDigiStreamZone *strZnPtr) {
	int32 cumulativeStreamOffset;
	IMuseDigiStreamZone *szTmp;

	if (strZnPtr->next) {
		cumulativeStreamOffset = strZnPtr->size;
		szTmp = dispatchPtr->streamZoneList;
		while (szTmp != strZnPtr) {
			cumulativeStreamOffset += szTmp->size;
			szTmp = szTmp->next;
		}

		// Set the stream load index of the sound to the new streamZone
		streamerSetLoadIndex(dispatchPtr->streamPtr, cumulativeStreamOffset);

		// Remove che previous streamZone from the list, we don't need it anymore
		while (strZnPtr->next->prev) {
			strZnPtr->next->prev->useFlag = 0;
			removeStreamZoneFromList(&strZnPtr->next, strZnPtr->next->prev);
		}

		streamerSetSoundToStreamFromOffset(
			dispatchPtr->streamPtr,
			dispatchPtr->trackPtr->soundId,
			strZnPtr->size + strZnPtr->offset);
	}
}

IMuseDigiStreamZone *IMuseDigital::dispatchAllocateStreamZone() {
	for (int i = 0; i < DIMUSE_MAX_STREAMZONES; i++) {
		if (_streamZones[i].useFlag == 0) {
			_streamZones[i].prev = 0;
			_streamZones[i].next = 0;
			_streamZones[i].useFlag = 1;
			_streamZones[i].offset = 0;
			_streamZones[i].size = 0;
			_streamZones[i].fadeFlag = 0;

			return &_streamZones[i];
		}
	}
	debug(5, "IMuseDigital::dispatchAllocateStreamZone(): ERROR: out of streamZones");
	return nullptr;
}

uint8 *IMuseDigital::dispatchAllocateFade(int32 &fadeSize, const char *function) {
	uint8 *allocatedFadeBuf = nullptr;
	if (fadeSize > DIMUSE_LARGE_FADE_DIM) {
		debug(5, "IMuseDigital::dispatchAllocateFade(): WARNING: requested fade too large (%d) in %s()", fadeSize, function);
		fadeSize = DIMUSE_LARGE_FADE_DIM;
	}

	if (fadeSize <= DIMUSE_SMALL_FADE_DIM) { // Small fade
		for (int i = 0; i <= DIMUSE_SMALL_FADES; i++) {
			if (i == DIMUSE_SMALL_FADES) {
				debug(5, "IMuseDigital::dispatchAllocateFade(): couldn't allocate small fade buffer in %s()", function);
				allocatedFadeBuf = nullptr;
				break;
			}

			if (!_dispatchSmallFadeFlags[i]) {
				_dispatchSmallFadeFlags[i] = 1;
				allocatedFadeBuf = &_dispatchSmallFadeBufs[DIMUSE_SMALL_FADE_DIM * i];
				break;
			}
		}
	} else { // Large fade
		for (int i = 0; i <= DIMUSE_LARGE_FADES; i++) {
			if (i == DIMUSE_LARGE_FADES) {
				debug(5, "IMuseDigital::dispatchAllocateFade(): couldn't allocate large fade buffer in %s()", function);
				allocatedFadeBuf = nullptr;
				break;
			}

			if (!_dispatchLargeFadeFlags[i]) {
				_dispatchLargeFadeFlags[i] = 1;
				allocatedFadeBuf = &_dispatchLargeFadeBufs[DIMUSE_LARGE_FADE_DIM * i];
				break;
			}
		}

		// Fallback to a small fade if large fades are unavailable
		if (!allocatedFadeBuf) {
			for (int i = 0; i <= DIMUSE_SMALL_FADES; i++) {
				if (i == DIMUSE_SMALL_FADES) {
					debug(5, "IMuseDigital::dispatchAllocateFade(): couldn't allocate small fade buffer in %s()", function);
					allocatedFadeBuf = nullptr;
					break;
				}

				if (!_dispatchSmallFadeFlags[i]) {
					_dispatchSmallFadeFlags[i] = 1;
					allocatedFadeBuf = &_dispatchSmallFadeBufs[DIMUSE_SMALL_FADE_DIM * i];
					break;
				}
			}
		}
	}

	return allocatedFadeBuf;
}

void IMuseDigital::dispatchDeallocateFade(IMuseDigiDispatch *dispatchPtr, const char *function) {
	// This function flags the fade corresponding to our fadeBuf as unused

	// First, check if our fade buffer is one of the large fade buffers
	for (int i = 0; i < DIMUSE_LARGE_FADES; i++) {
		if (_dispatchLargeFadeBufs + (DIMUSE_LARGE_FADE_DIM * i) == dispatchPtr->fadeBuf) { // Found it!
			if (_dispatchLargeFadeFlags[i] == 0) {
				debug(5, "IMuseDigital::dispatchDeallocateFade(): redundant large fade buf de-allocation in %s()", function);
			}
			_dispatchLargeFadeFlags[i] = 0;
			return;
		}
	}

	// If not, check between the small fade buffers
	for (int j = 0; j < DIMUSE_SMALL_FADES; j++) {
		if (_dispatchSmallFadeBufs + (DIMUSE_SMALL_FADE_DIM * j) == dispatchPtr->fadeBuf) { // Found it!
			if (_dispatchSmallFadeFlags[j] == 0) {
				debug(5, "IMuseDigital::dispatchDeallocateFade(): redundant small fade buf de-allocation in %s()", function);
			}
			_dispatchSmallFadeFlags[j] = 0;
			return;
		}
	}

	debug(5, "IMuseDigital::dispatchDeallocateFade(): couldn't find fade buf to de-allocate in %s()", function);
}

int IMuseDigital::dispatchGetFadeSize(IMuseDigiDispatch *dispatchPtr, int fadeLength) {
	return (dispatchPtr->wordSize * dispatchPtr->channelCount * ((dispatchPtr->sampleRate * fadeLength / 1000) & 0xFFFFFFFE)) / 8;
}

void IMuseDigital::dispatchValidateFadeSize(IMuseDigiDispatch *dispatchPtr, int32 &dispatchSize, const char *function) {
	int alignmentModDividend;
	if (_vm->_game.id == GID_DIG || (_vm->_game.id == GID_CMI && _vm->_game.features & GF_DEMO)) {
		alignmentModDividend = dispatchPtr->channelCount * (dispatchPtr->wordSize == 8 ? 1 : 3);
	} else {
		if (dispatchPtr->wordSize == 8) {
			alignmentModDividend = dispatchPtr->channelCount * 1;
		} else {
			alignmentModDividend = dispatchPtr->channelCount * ((dispatchPtr->wordSize == 12) + 2);
		}
	}

	if (alignmentModDividend) {
		dispatchSize -= dispatchSize % alignmentModDividend;
	} else {
		debug(5, "IMuseDigital::dispatchValidateFadeSize(): WARNING: tried mod by 0 while validating fade size in %s(), ignored", function);
	}
}

int IMuseDigital::dispatchUpdateFadeMixVolume(IMuseDigiDispatch *dispatchPtr, int32 remainingFade) {
	int mixVolume = (((dispatchPtr->fadeVol / 65536) + 1) * dispatchPtr->trackPtr->effVol) / 128;
	dispatchPtr->fadeVol += remainingFade * dispatchPtr->fadeSlope;

	if (dispatchPtr->fadeVol < 0)
		dispatchPtr->fadeVol = 0;
	if (dispatchPtr->fadeVol > DIMUSE_MAX_FADE_VOLUME)
		dispatchPtr->fadeVol = DIMUSE_MAX_FADE_VOLUME;

	return mixVolume;
}

int IMuseDigital::dispatchUpdateFadeSlope(IMuseDigiDispatch *dispatchPtr) {
	int32 updatedVolume, effRemainingFade;

	updatedVolume = (dispatchPtr->trackPtr->effVol * (128 - (dispatchPtr->fadeVol / 65536))) / 128;
	if (!dispatchPtr->fadeSlope) {
		effRemainingFade = dispatchPtr->fadeRemaining;
		if (effRemainingFade <= 1)
			effRemainingFade = 2;
		dispatchPtr->fadeSlope = -(DIMUSE_MAX_FADE_VOLUME / effRemainingFade);
	}

	return updatedVolume;
}

void IMuseDigital::dispatchVOCLoopCallback(int soundId) {
	IMuseDigiDispatch *curDispatchPtr;
	uint8 *dataBlockTag;

	if (!soundId)
		return;

	for (int i = 0; i < _trackCount; i++) {
		curDispatchPtr = &_dispatches[i];
		if (curDispatchPtr->trackPtr->soundId == soundId) {
			dataBlockTag = streamerGetStreamBufferAtOffset(curDispatchPtr->streamPtr, curDispatchPtr->audioRemaining, 1);
			if (dataBlockTag && dataBlockTag[0] == 7) { // End of loop
				streamerSetLoadIndex(curDispatchPtr->streamPtr, curDispatchPtr->audioRemaining + 1);
				streamerSetSoundToStreamFromOffset(curDispatchPtr->streamPtr, curDispatchPtr->trackPtr->soundId, curDispatchPtr->vocLoopStartingPoint);
			}
		}
	}
}

int IMuseDigital::dispatchSeekToNextChunk(IMuseDigiDispatch *dispatchPtr) {
	uint8 *headerBuf;
	uint8 *soundAddrData;

	while (1) {
		if (dispatchPtr->streamPtr) {
			headerBuf = streamerGetStreamBufferAtOffset(dispatchPtr->streamPtr, 0, 0x30);
			if (headerBuf || (headerBuf = streamerGetStreamBufferAtOffset(dispatchPtr->streamPtr, 0, 1)) != 0) {
				memcpy(_currentVOCHeader, headerBuf, 0x30);
			} else {
				return -3;
			}
		} else {
			soundAddrData = _filesHandler->getSoundAddrData(dispatchPtr->trackPtr->soundId);
			uint32 soundAddrSize = _filesHandler->getSoundAddrDataSize(dispatchPtr->trackPtr->soundId, dispatchPtr->streamPtr != 0);
			uint32 fetchSize = (soundAddrSize - dispatchPtr->currentOffset) >= 0x30 ? 0x30 : (soundAddrSize - dispatchPtr->currentOffset);

			if (soundAddrData && soundAddrSize > 0) {
				memcpy(_currentVOCHeader, &soundAddrData[dispatchPtr->currentOffset], fetchSize);
			} else {
				return -1;
			}
		}

		if (READ_BE_UINT32(_currentVOCHeader) == MKTAG('C', 'r', 'e', 'a')) {
			// We expect to find (everything in Little Endian except where noted):
			// - The string "Creative Voice File" stored in Big Endian format;
			// - 0x1A, which the interpreter doesn't check, so we don't either
			// - Total size of the header, which has to be 0x001A (0x1A00 in LE)
			// - Version tags: 0x0A (minor), 0x01 (major), this corresponds to version 1.10
			if (_currentVOCHeader[20] == 0x1A && _currentVOCHeader[21] == 0x0 &&
				_currentVOCHeader[22] == 0xA && _currentVOCHeader[23] == 0x1) {
				dispatchPtr->currentOffset += 26;
				if (dispatchPtr->streamPtr)
					streamerGetStreamBuffer(dispatchPtr->streamPtr, 26);
				continue;
			}
			return -1;
		} else {
			uint8 *headerTag = _currentVOCHeader;

			switch (headerTag[0]) {
			case 1:
				dispatchPtr->sampleRate = headerTag[4] > 196 ? 22050 : 11025;
				dispatchPtr->audioRemaining = (READ_LE_UINT32(headerTag) >> 8) - 2;
				dispatchPtr->currentOffset += 6;

				if (dispatchPtr->streamPtr) {
					streamerGetStreamBuffer(dispatchPtr->streamPtr, 6);
					if (dispatchPtr->vocLoopStartingPoint)
						streamerSetLoopFlag(dispatchPtr->streamPtr, dispatchPtr->audioRemaining + dispatchPtr->currentOffset);
				}
				return 0;
			case 4:
				// Marker, 2 bytes, theoretically used for triggers, but never actually found in the game;
				// I am keeping this case here, in order to correctly keep track of the offset
				dispatchPtr->currentOffset += 6;
				continue;
			case 6:
				dispatchPtr->vocLoopStartingPoint = dispatchPtr->currentOffset;
				dispatchPtr->currentOffset += 6;
				if (dispatchPtr->streamPtr)
					streamerGetStreamBuffer(dispatchPtr->streamPtr, 6);
				continue;
			case 7:
				dispatchPtr->currentOffset = dispatchPtr->vocLoopStartingPoint;
				if (dispatchPtr->streamPtr)
					streamerGetStreamBuffer(dispatchPtr->streamPtr, 1);
				continue;
			default:
				return -1;
			}
		}
		return -1;
	}
}

} // End of namespace Scumm
