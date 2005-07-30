/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
	return _isInitialized;
}

void PckTunesCDPlayer::release() {
	// self delete
	PocketTunesStop();
	delete this;
}

UInt32 PckTunesCDPlayer::getStatus() {
	if (!_isPlaying)
		return kPtunesStopped;

	EventType e;
	UInt32 status;

	PocketTunesAction *pAction = (PocketTunesAction*)MemPtrNew (sizeof(PocketTunesAction));
	if (!pAction)
		return kPtunesStopped;

	pAction->action = kPocketTunesActionGetStatus;
	EvtGetEvent(&e, evtNoWait);
	if (PocketTunesCallSynch(pAction) == errNone)
		status = pAction->data.getStatusAction.status;
	else
		status = kPtunesStopped;

	MemPtrFree(pAction);
	return status;
}

UInt32 PckTunesCDPlayer::getPosition(UInt32 deflt) {
	if (!_isPlaying)
		return deflt;

	EventType e;
	UInt32 value;

	PocketTunesAction *pAction = (PocketTunesAction*)MemPtrNew (sizeof(PocketTunesAction));
	if (!pAction)
		return deflt;

	pAction->action = kPocketTunesActionGetValue;
	pAction->data.getValueAction.which = kPtunesValueSongPosition;

	EvtGetEvent(&e, evtNoWait);
	if (PocketTunesCallSynch(pAction) == errNone)
		value = pAction->data.getValueAction.value;
	else
		value = deflt;

	MemPtrFree(pAction);
	return value;
}

void PckTunesCDPlayer::setPosition(UInt32 value) {
	if (!_isPlaying)
		return;

	EventType e;
	PocketTunesAction *pAction = AllocateAsynchronousActionStruct();
	if (!pAction)
		return;

	pAction->action = kPocketTunesActionSetValue;
	pAction->data.getValueAction.which = kPtunesValueSongPosition;
	pAction->data.getValueAction.value = value;

	EvtGetEvent(&e, evtNoWait);
	PocketTunesCall(pAction);
}

UInt32 PckTunesCDPlayer::getDuration() {
	if (!_isPlaying)
		return gVars->CD.defaultTrackLength;

	EventType e;
	UInt32 value;

	PocketTunesAction *pAction = (PocketTunesAction*)MemPtrNew (sizeof(PocketTunesAction));
	if (!pAction)
		return gVars->CD.defaultTrackLength;

	pAction->action = kPocketTunesActionGetValue;
	pAction->data.getValueAction.which = kPtunesValueSongDuration;

	EvtGetEvent(&e, evtNoWait);
	if (PocketTunesCallSynch(pAction) == errNone)
		value = pAction->data.getValueAction.value;
	else
		value = gVars->CD.defaultTrackLength;

	MemPtrFree(pAction);
	return value;
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
//	if (_sys->getMillis() < _pckTrackEndFrame)
//		return;
	if (getPosition(_pckTrackEndFrame) < _pckTrackEndFrame)
		return;

	PocketTunesStop();

	if (_pckLoops == 0)
		return;

	// loop again ?
	if (_pckLoops > 0)
		_pckLoops--;

	// loop if needed
	if (_pckLoops != 0 && _isPlaying) {
		PocketTunesPlay();

		if (_pckTrackStartFrame == 0 && _pckTrackDuration == 0) {
			setPosition(0);
		} else {
			setPosition(_pckTrackStartFrame);
		}
//		_pckTrackEndFrame = _pckTrackStartFrame + _pckTrackDuration;
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
	Char *ext[]	= { "mp3", "ogg" };

//	if (duration > 0)
//		duration += 5;

	_pckTrack = track;
	_pckLoops = num_loops;
	_pckTrackStartFrame = TO_MSECS(start_frame);
	_pckTrackDuration = TO_MSECS(duration);

	// stop current play if any
	VFSVolumeGetLabel(gVars->volRefNum, nameP, 256);

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
