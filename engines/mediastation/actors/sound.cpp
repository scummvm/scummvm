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

#include "mediastation/audio.h"
#include "mediastation/debugchannels.h"
#include "mediastation/actors/sound.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

SoundActor::~SoundActor() {
	unregisterWithStreamManager();
	if (_streamFeed != nullptr) {
		g_engine->getStreamFeedManager()->closeStreamFeed(_streamFeed);
		_streamFeed = nullptr;
	}
}

void SoundActor::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderActorId: {
		// We already have this actor's ID, so we will just verify it is the same
		// as the ID we have already read.
		uint32 duplicateActorId = chunk.readTypedUint16();
		if (duplicateActorId != _id) {
			warning("[%s] %s: Duplicate actor ID %s does not match", debugName(), __func__, g_engine->formatActorName(duplicateActorId).c_str());
		}
		break;
	}

	case kActorHeaderChannelIdent:
		_channelIdent = chunk.readTypedChannelIdent();
		registerWithStreamManager();
		break;

	case kActorHeaderDiscardAfterUse:
		_discardAfterUse = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderSoundInfo:
		_sequence.readParameters(chunk);
		break;

	case kActorHeaderCachingEnabled:
		// This controls some caching behavior in the original, but since that is not currently
		// implemented here, just throw it away.
		chunk.readTypedByte();
		break;

	case kActorHeaderInstallType:
		// In the original, this controls behavior if the files are NOT installed. But since
		// the "installation" is just copying from the CD-ROM, we can treat the game as always
		// installed. So just throw away this value.
		chunk.readTypedByte();
		break;

	default:
		Actor::readParameter(chunk, paramType);
	}
}

void SoundActor::readChunk(Chunk &chunk) {
	_isLoadedFromChunk = true;
	_sequence.readChunk(chunk);
}

void SoundActor::soundPlayStateChanged(SoundPlayState state, SoundStopReason stopReason) {
	switch (state) {
	case kSoundPlayStateStopped: {
		_playState = kSoundPlayStateStopped;
		g_engine->getTimerService()->stopTimer(_timer);

		EventType eventType = kEventTypeInvalid;
		switch (stopReason) {
		case kSoundStopForFailure:
			eventType = kSoundFailureEvent;
			break;

		case kSoundStopForEnd:
			eventType = kSoundEndEvent;
			break;

		case kSoundStopForScriptStop:
			eventType = kSoundStoppedEvent;
			break;

		case kSoundStopForAbort:
			eventType = kSoundAbortEvent;
			break;

		case kSoundStopForNone:
		default:
			break;
		}

		ActorEvent event(_id, eventType);
		g_engine->getEventLoop()->queueEvent(event);
		break;
	}

	case kSoundPlayStatePlaying:
		if (_playState == kSoundPlayStateStopped) {
			ActorEvent event(_id, kSoundBeginEvent);
			g_engine->getEventLoop()->queueEvent(event);
		}
		_playState = kSoundPlayStatePlaying;
		break;

	case kSoundPlayStateSleep:
		_playState = kSoundPlayStatePaused;
		break;

	default:
		// Other cases are explicitly not handled.
		break;
	}
}

ScriptValue SoundActor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;

	switch (methodId) {
	case kSpatialShowMethod:
		// WORKAROUND: No-op to avoid triggering error on Dalmatians
		// timer_6c06_AnsweringMachine, which calls SpatialShow on a sound.
		// Since the engine is currently flagging errors on unimplemented
		// methods for easier debugging, a no-op is used here to avoid the error.
		ARGCOUNTCHECK(0);
		break;

	case kTimePlayMethod:
		ARGCOUNTCHECK(0);
		start();
		break;

	case kTimeStopMethod:
		ARGCOUNTCHECK(0);
		stop();
		break;

	case kTimePauseMethod:
		ARGCOUNTCHECK(0);
		pause();
		break;

	case kTimeResumeMethod: {
		ARGCOUNTRANGE(0, 1);
		bool shouldRestart = false;
		if (args.size() == 1) {
			shouldRestart = args[0].asBool();
		}
		resume(shouldRestart);
		break;
	}

	case kIsPlayingMethod:
		returnValue.setToBool(_playState == kSoundPlayStatePlaying || _playState == kSoundPlayStatePaused);
		break;

	case kIsPausedMethod:
		returnValue.setToBool(_playState == kSoundPlayStatePaused);
		break;

	default:
		returnValue = Actor::callMethod(methodId, args);
	}
	return returnValue;
}

void SoundActor::onEvent(const ActorEvent &event) {
	switch (event.type) {
	case kSoundEndEvent:
		triggerRemainingTimerEvents();
		break;

	case kCachingStartedEvent:
	case kCachingEndedEvent:
	case kCachingFailureEvent:
		// Caching-related events are not implemented, but they can be implemented
		// if the original CD-ROM streaming/caching logic is reimplemented.
		Actor::onEvent(event);
		break;

	case kSoundStoppedEvent:
	case kSoundAbortEvent:
	case kSoundFailureEvent:
		// Currently, these aren't reimplemented. But if original CD-ROM streaming
		// logic is implemented later on, some stream cleanup is needed here.
		break;

	default:
		break;
	}

	runScriptResponseIfExists(event.type);
}

void SoundActor::timerEvent(const TimerEvent &event) {
	Actor::processTimeScriptResponses();

	// Set up the next timer wakeup.
	g_engine->getTimerService()->stopTimer(_timer);
	setupNextScriptResponseTimer();
}

void SoundActor::start() {
	if (_loadIsComplete) {
		if (_playState == kSoundPlayStatePlaying || _playState == kSoundPlayStatePaused) {
			stop();
		}

		openStream();
		_playState = kSoundPlayStatePlaying;
		_startTime = g_engine->getTotalPlayTime();
		_lastProcessedTime = 0;
		setupNextScriptResponseTimer();
		_sequence.start();

		ActorEvent actorEvent(_id, kSoundBeginEvent);
		g_engine->getEventLoop()->queueEvent(actorEvent);
	} else {
		warning("[%s] %s: Attempted to play sound before it was loaded", debugName(), __func__);
	}
}

void SoundActor::stop() {
	if (_playState == kSoundPlayStatePlaying || _playState == kSoundPlayStatePaused) {
		_playState = kSoundPlayStateStopped;
		_sequence.stop();
		g_engine->getTimerService()->stopTimer(_timer);
	}
}

void SoundActor::pause() {
	if (_playState == kSoundPlayStatePlaying) {
		_sequence.pause();
		_sequence.sleep();
		// There don't seem to be script events to trigger in this instance.
	}
}

void SoundActor::resume(bool restart) {
	if (_playState == kSoundPlayStatePaused) {
		_sequence.awake();
		_sequence.resume();
	} else if (restart) {
		start();
	}
	// There don't seem to be script events to trigger in this instance.
}

void SoundActor::openStream() {
	if (_streamFeed == nullptr && !_isLoadedFromChunk) {
		_streamFeed = g_engine->getStreamFeedManager()->openStreamFeed(_id);
		_streamFeed->readData();
	}
}

} // End of namespace MediaStation
