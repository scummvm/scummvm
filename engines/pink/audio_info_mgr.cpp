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

#include "pink/audio_info_mgr.h"
#include "pink/archive.h"
#include "pink/constants.h"
#include "pink/objects/actors/lead_actor.h"

namespace Pink {

AudioInfoMgr::AudioInfoMgr(LeadActor *lead)
	: _lead(lead) {}

void AudioInfoMgr::loadState(Archive &archive) {
	_aboutWhom = archive.readString();
}

void AudioInfoMgr::saveState(Archive &archive) {
	archive.writeString(_aboutWhom);
}

void AudioInfoMgr::start(Actor *actor) {
	if (!actor->getPDALink().empty()) {
		_aboutWhom = actor->getName();
		playAudio();
		showPDAButton();
	} else
		stop();
}

void AudioInfoMgr::stop() {
	if (!_aboutWhom.empty()) {
		stopAudio();
		hidePDAButton();
		_aboutWhom.clear();
	}
}

void AudioInfoMgr::onLeftClick() {
	Actor *actor = _lead->findActor(_aboutWhom);
	assert(actor);
	_lead->loadPDA(actor->getPDALink());
	stopAudio();
}

void AudioInfoMgr::playAudio() {
	Actor *audioInfo = _lead->findActor(kAudioInfoActor);
	assert(audioInfo);
	audioInfo->setAction(_aboutWhom);
}

void AudioInfoMgr::stopAudio() {
	Actor *audioInfo = _lead->findActor(kAudioInfoActor);
	assert(audioInfo);
	audioInfo->setAction(kIdleAction);
}

void AudioInfoMgr::showPDAButton() {
	Actor *pdaButton = _lead->findActor(kPdaButtonActor);
	assert(pdaButton);
	pdaButton->setAction(kShowAction);
}

void AudioInfoMgr::hidePDAButton() {
	Actor *pdaButton = _lead->findActor(kPdaButtonActor);
	assert(pdaButton);
	pdaButton->setAction(kHideAction);
}

} // End of namespace Pink
