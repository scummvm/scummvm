/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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
 * $Header$
 *
 */
#include "common/stdafx.h"
#include "common/system.h"
#include "common/config-manager.h"

#include "cd_pockettunes.h"

PckTunesCDPlayer::PckTunesCDPlayer(OSystem *sys) {
	_sys = sys;
	StrCopy(gameP, ConfMan.getActiveDomain().c_str());
}

bool PckTunesCDPlayer::init() {
	PocketTunesStart();
	_isInitialized = PocketTunesIsRunning();
	_isPlaying = false;
	_pAction = NULL;
	
	if (_isInitialized) {
		_pAction = (PocketTunesAction*)MemPtrNew(sizeof(PocketTunesAction));
		_volumeLimit = getVolumeLimit();
	}

	_isInitialized = (_isInitialized && _pAction);
	return _isInitialized;
}

void PckTunesCDPlayer::release() {
	PocketTunesStop();
	if (_pAction)
		MemPtrFree(_pAction);

	// self delete
	delete this;
}

UInt32 PckTunesCDPlayer::getVolumeLimit() {
	UInt32 value = 0;

	if (!_pAction)
		return value;

	_pAction->action = kPocketTunesActionGetValue;
	_pAction->data.getValueAction.which = kPtunesValueMaxVolume;
	
	EvtGetEvent(&_eAction, evtNoWait);
	if (PocketTunesCallSynch(_pAction) == errNone)
		value = _pAction->data.getValueAction.value;

	return value;	
}

UInt32 PckTunesCDPlayer::getStatus() {
	UInt32 status = kPtunesStopped;

	if (!_isPlaying || !_pAction)
		return status;

	_pAction->action = kPocketTunesActionGetStatus;

	EvtGetEvent(&_eAction, evtNoWait);
	if (PocketTunesCallSynch(_pAction) == errNone)
		status = _pAction->data.getStatusAction.status;
	
	return status;	
}

UInt32 PckTunesCDPlayer::getPosition(UInt32 deflt) {
	UInt32 value = deflt;

	if (!_isPlaying || !_pAction)
		return value;

	_pAction->action = kPocketTunesActionGetValue;
	_pAction->data.getValueAction.which = kPtunesValueSongPosition;
	
	EvtGetEvent(&_eAction, evtNoWait);
	if (PocketTunesCallSynch(_pAction) == errNone)
		value = _pAction->data.getValueAction.value;

	return value;
}

UInt32 PckTunesCDPlayer::getDuration() {
	UInt32 value = gVars->CD.defaultTrackLength;

	if (!_isPlaying || !_pAction)
		return value;

	_pAction->action = kPocketTunesActionGetValue;
	_pAction->data.getValueAction.which = kPtunesValueSongDuration;
	
	EvtGetEvent(&_eAction, evtNoWait);
	if (PocketTunesCallSynch(_pAction) == errNone)
		value = _pAction->data.getValueAction.value;

	return value;	
}

void PckTunesCDPlayer::setPosition(UInt32 value) {
	if (!_isPlaying)
		return;

	PocketTunesAction *pAction = AllocateAsynchronousActionStruct();
	if (!pAction)
		return;

	pAction->action = kPocketTunesActionSetValue;
	pAction->data.getValueAction.which = kPtunesValueSongPosition;
	pAction->data.getValueAction.value = value;

	EvtGetEvent(&_eAction, evtNoWait);
	PocketTunesCall(pAction);
}

void PckTunesCDPlayer::setVolume(int volume) {
	_volumeLevel = volume;

	PocketTunesAction *pAction = AllocateAsynchronousActionStruct();
	if (!pAction)
		return;

	pAction->action = kPocketTunesActionSetValue;
	pAction->data.getValueAction.which = kPtunesValueVolume;
	pAction->data.getValueAction.value = (_volumeLimit * volume) / 100;

	EvtGetEvent(&_eAction, evtNoWait);
	PocketTunesCall(pAction);
}

bool PckTunesCDPlayer::poll() {
	return (_pckLoops != 0 && (getPosition(_pckTrackEndFrame) < _pckTrackEndFrame || getStatus() != kPtunesStopped));
}

void PckTunesCDPlayer::update() {

	// stop replay upon request of stopCD()
	if (_pckStopTime != 0 && _sys->getMillis() >= _pckStopTime) {
		PocketTunesStop();
		_pckLoops = 0;
		_pckStopTime = 0;
		_pckTrackEndFrame = 0;
		return;
	}

	// not fully played
	if (getPosition(_pckTrackEndFrame) < _pckTrackEndFrame && getStatus() != kPtunesStopped)
		return;

	PocketTunesStop();

	if (_pckLoops == 0)
		return;

	// loop again ?
	if (_pckLoops > 0)
		_pckLoops--;

	// loop if needed
	if (_pckLoops != 0 && _isPlaying) {
		if (_pckTrackStartFrame == 0 && _pckTrackDuration == 0) {
			setPosition(0);
		} else {
			setPosition(_pckTrackStartFrame);
		}

		PocketTunesPlay();
	}
}

void PckTunesCDPlayer::stop() {	/* Stop CD Audio in 1/10th of a second */
	_pckStopTime = _sys->getMillis() + 100;
	_pckLoops = 0;
	return;
}

void PckTunesCDPlayer::play(int track, int num_loops, int start_frame, int duration) {
	if (!num_loops && !start_frame)
		return;

	EventType e;
	Char nameP[256], fileP[100];
	static const Char *ext[] = { "mp3", "ogg" };
	
	_pckTrack = track;
	_pckLoops = num_loops;
	_pckTrackStartFrame = TO_MSECS(start_frame);
	_pckTrackDuration = TO_MSECS(duration);

	VFSVolumeGetLabel(gVars->VFS.volRefNum, nameP, 256);
	
	StrPrintF(fileP, "/Palm/Programs/ScummVM/Audio/%s_%03ld.%s", gameP, (track + gVars->CD.firstTrack - 1), ext[gVars->CD.format]);
	
	if (PocketTunesOpenFile(nameP, fileP, 0) == errNone) {
		EvtGetEvent(&e, evtNoWait);
		PocketTunesPauseIfPlaying();
		_isPlaying = true;

		if (_pckTrackStartFrame == 0 && _pckTrackDuration == 0) {
			_pckTrackDuration = getDuration();
		} else {
			setPosition(_pckTrackStartFrame);
			if (_pckTrackDuration == 0)
				_pckTrackDuration = getDuration() - _pckTrackStartFrame;
		}

		PocketTunesPlay();

	} else {
		_isPlaying = false;
		_pckTrackDuration = gVars->CD.defaultTrackLength * 1000;
	}

	_pckStopTime = 0;
	_pckTrackEndFrame = _pckTrackStartFrame + _pckTrackDuration;
}
