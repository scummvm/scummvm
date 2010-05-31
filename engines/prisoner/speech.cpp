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
 * $URL$
 * $Id$
 *
 */

#include "prisoner/prisoner.h"
#include "prisoner/resource.h"
#include "prisoner/screen.h"
#include "prisoner/speech.h"

namespace Prisoner {

void PrisonerEngine::startTalkieSpeech(int16 pakSlot, uint32 &finishedTime, int16 resourceCacheSlot) {

	Common::String soundPakName = _res->get<TextResource>(resourceCacheSlot)->getSoundPakName();

	makeLanguageString(soundPakName);

	_talkieDataResourceCacheSlot = -1;
	_talkieSpeechDataPlayNow = false;

	// TODO: Check if sound and speech are enabled

	if (_lipSyncScriptNumber == -1) {
		_talkieDataResourceCacheSlot = _res->load<SoundResource>(soundPakName, pakSlot, 16);
		SoundResource *soundResource = _res->get<SoundResource>(_talkieDataResourceCacheSlot);
		_talkieSpeechData = soundResource->getAudioStream();
		finishedTime = getTicks() + soundResource->getDuration();
		_talkieSpeechDataPlayNow = true;
	} else {
		_talkieDataResourceCacheSlot = _res->load<LipSyncSoundResource>(soundPakName, pakSlot, 16);
		LipSyncSoundResource *lipSyncSoundResource = _res->get<LipSyncSoundResource>(_talkieDataResourceCacheSlot);
		_talkieSpeechData = lipSyncSoundResource->getAudioStream();
		finishedTime = getTicks() + lipSyncSoundResource->getDuration();
		if (_lipSyncChannelStatusRestored) {
			_lipSyncChannelStatusRestored = false;
		} else {
			_lipSyncChannelStatus.clear();
			_lipSyncChannelStatus.reserve(lipSyncSoundResource->getChannelCount());
			for (uint i = 0; i < lipSyncSoundResource->getChannelCount(); i++)
				_lipSyncChannelStatus.push_back(LipSyncChannelStatus());
		}
		_talkieSpeechDataPlayNow = true;
	}

}

void PrisonerEngine::stopTalkieSpeech() {
	_talkieSpeechActive = false;
	if (_mixer->isSoundHandleActive(_talkieSoundHandle)) {
		_mixer->stopHandle(_talkieSoundHandle);
	}
	if (_talkieDataResourceCacheSlot != -1) {
		_res->unload(_talkieDataResourceCacheSlot);
		_talkieDataResourceCacheSlot = -1;
	}
	_lipSyncChannelStatus.clear();
}

void PrisonerEngine::updateTalkieSpeech(int16 &pakSlot, uint32 &finishedTime, int16 resourceCacheSlot) {

	if (pakSlot == -1)
		return;

	if (!_talkieSpeechActive) {
		_talkieSpeechActive = true;
		startTalkieSpeech(pakSlot, finishedTime, resourceCacheSlot);
		pakSlot++;
	}

	if (_lipSyncScriptNumber != -1) {
		LipSyncSoundResource *lipSyncSoundResource = _res->get<LipSyncSoundResource>(_talkieDataResourceCacheSlot);
		_lipSyncAnimationResource = _res->get<AnimationResource>(_lipSyncResourceCacheSlot);
		drawLipSyncFrame(0);
		if (!_loadingSavegame) {
			_lipSyncTime = getTicks() - _lipSyncTicks;
			_lipSyncTicks  = getTicks();
		}
		for (uint channelIndex = 0; channelIndex < lipSyncSoundResource->getChannelCount(); channelIndex++) {
			const LipSyncChannel &channel = lipSyncSoundResource->getChannel(channelIndex);
			LipSyncChannelStatus &channelStatus = _lipSyncChannelStatus[channelIndex];
			if (!_loadingSavegame && (uint)channelStatus.index < channel.items.size()) {
				channelStatus.ticks -= _lipSyncTime;
				if (channelStatus.ticks <= 0) {
					channelStatus.index++;
					channelStatus.ticks = lipSyncSoundResource->getTicksScale() * channel.items[channelStatus.index - 1].duration;
				}
			}
			if (channel.parentChannel != -1) {
				if (!channel.flag) {
					debug("!channel.flag not implemented");
				} else {
					drawLipSyncFrame(channel.items[channelStatus.index - 1].index);
				}
			}
		}
	}

	if (!_loadingSavegame && _talkieSpeechDataPlayNow) {
		_talkieSpeechDataPlayNow = false;
		_talkieSpeechData->rewind();
		_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_talkieSoundHandle,
			_talkieSpeechData, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
	}

}

void PrisonerEngine::startLipSync(Common::String &pakName, int16 pakSlot, int16 scriptIndex, int16 actorIndex,
		int16 lipSyncX, int16 lipSyncY) {
	_lipSyncScriptNumber = scriptIndex;
	_lipSyncActorIndex = actorIndex;
	_lipSyncX = lipSyncX;
	_lipSyncY = lipSyncY;
	_lipSyncResourceCacheSlot = _res->load<AnimationResource>(pakName, pakSlot, 14);
	_lipSyncTicks = getTicks();
}

void PrisonerEngine::stopLipSync() {
	if (_lipSyncScriptNumber != -1) {
		_lipSyncScriptNumber = -1;
		if (_lipSyncResourceCacheSlot != -1) {
			_res->unload(_lipSyncResourceCacheSlot);
			_lipSyncResourceCacheSlot = -1;
		}
	}
	_lipSyncActorIndex = -1;
}

void PrisonerEngine::drawLipSyncFrame(int16 frameListIndex) {

	_screen->setClipRect(0, 82, 639, 397);

	_screen->drawAnimationElement(_lipSyncAnimationResource,
		_lipSyncAnimationResource->_anims[frameListIndex]->frames[0]->elementIndex,
		_lipSyncX, _lipSyncY, 0);

	_screen->setClipRect(0, 0, 639, 479);

}

} // End of namespace Prisoner
