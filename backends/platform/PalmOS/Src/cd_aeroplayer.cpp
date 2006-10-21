/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2006 Chris Apers - PalmOS Backend
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

#include "stdafx.h"
#include "common/system.h"
#include "common/config-manager.h"

#include "cd_aeroplayer.h"

AeroCDPlayer::AeroCDPlayer(OSystem *sys) {
	_sys = sys;
	StrCopy(gameP, ConfMan.getActiveDomainName().c_str());
}

bool AeroCDPlayer::init() {
	aeroplayer_Startup();
	_isInitialized = aeroplayer_PlayerIsActive();
	
	if (_isInitialized)
		_volumeLimit = aeroplayer_GetMaxVolumeIndex();

	return _isInitialized;
}

void AeroCDPlayer::release() {
	aeroplayer_Shutdown();

	// self delete
	delete this;
}

UInt32 AeroCDPlayer::getStatus() {
	return aeroplayer_GetPlaybackStatus();
}

UInt32 AeroCDPlayer::getPosition() {
	return aeroplayer_GetPosition_msec();
}

UInt32 AeroCDPlayer::getDuration() {
	return aeroplayer_GetDuration_msec();
}

void AeroCDPlayer::setPosition(UInt32 value) {
	aeroplayer_Seek_abs_msec(value);
}

void AeroCDPlayer::setVolume(int volume) {
	_volumeLevel = volume;
	aeroplayer_SetVolumeIndex((_volumeLimit * volume) / 100);
}

bool AeroCDPlayer::poll() {
	return
	(	_pckLoops != 0 && 
		(	getPosition() < _pckTrackEndFrame &&
			getStatus() == AEROPLAYER_STATUS_PLAY
		)
	);
}

void AeroCDPlayer::update() {

	// stop replay upon request of stopCD()
	if (_pckStopTime != 0 && _sys->getMillis() >= _pckStopTime) {
		forceStop();
		_pckLoops = 0;
		_pckStopTime = 0;
		_pckTrackEndFrame = 0;
		return;
	}

	// not fully played
	if (getPosition() < _pckTrackEndFrame && getStatus() == AEROPLAYER_STATUS_PLAY)
		return;

	aeroplayer_Pause();
//	_pckStopTime = _sys->getMillis();

	// loop again ?
	if (_pckLoops > 0)
		_pckLoops--;

	// loop if needed
	if (_pckLoops == 0)
		forceStop();
	else {
		//_pckStopTime = 0;
			
		if (_pckTrackStartFrame == 0 && _pckTrackDuration == 0) {
			setPosition(0);
		} else {
			setPosition(_pckTrackStartFrame);
		}

		aeroplayer_Play();
		_pckEndTime = _sys->getMillis() + _pckTrackDuration;
	}
}

void AeroCDPlayer::stop() {
	_pckStopTime = _sys->getMillis();
	_pckLoops = 0;
	return;
}

void AeroCDPlayer::forceStop() {
	if (getStatus() != AEROPLAYER_STATUS_STOP)
		aeroplayer_PlayTrack(vfsInvalidVolRef, NULL);
}

void AeroCDPlayer::play(int track, int num_loops, int start_frame, int duration) {
	if (!num_loops && !start_frame)
		return;

	Char fileP[100];
	static const Char *ext[] = { "mp3", "ogg" };
	
//	if (duration > 0)
//		duration += 5;
	
	_pckTrack = track;
	_pckLoops = num_loops;
	_pckTrackStartFrame = TO_MSECS(start_frame);
	_pckTrackDuration = TO_MSECS(duration);

	StrPrintF(fileP, "/Palm/Programs/ScummVM/Audio/%s_%03ld.%s", gameP, (track + gVars->CD.firstTrack - 1), ext[gVars->CD.format]);
	aeroplayer_Pause();
	aeroplayer_PlayTrack(gVars->VFS.volRefNum, fileP);
	aeroplayer_Pause();
	
	if (_pckTrackStartFrame == 0 && _pckTrackDuration == 0) {
		_pckTrackDuration = getDuration();
	} else {
		setPosition(_pckTrackStartFrame);
		if (_pckTrackDuration == 0)
			_pckTrackDuration = getDuration() - _pckTrackStartFrame;
	}
	
	aeroplayer_Play();

	_pckStopTime = 0;
	_pckTrackEndFrame = _pckTrackStartFrame + _pckTrackDuration;
	_pckEndTime = _sys->getMillis() + _pckTrackDuration;
}
