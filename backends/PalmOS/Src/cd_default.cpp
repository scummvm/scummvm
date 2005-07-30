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
#include "cd_default.h"

DefaultCDPlayer::DefaultCDPlayer(OSystem *sys) {
	_sys = sys;
}

bool DefaultCDPlayer::init() {
	_isInitialized = true;
	return _isInitialized;
}

void DefaultCDPlayer::release() {
	// self delete
	delete this;
}

bool DefaultCDPlayer::poll() {
	return (_defLoops != 0 && _sys->getMillis() < _defTrackEndFrame);
}

void DefaultCDPlayer::update() {

	// stop replay upon request of stopCD()
	if (_defStopTime != 0 && _sys->getMillis() >= _defStopTime) {
		_defLoops = 0;
		_defStopTime = 0;
		_defTrackEndFrame = 0;
		return;
	}

	// not fully played
	if (_sys->getMillis() < _defTrackEndFrame)
		return;

	if (_defLoops == 0)
		return;

	// loop again ?
	if (_defLoops > 0)
		_defLoops--;

	// loop if needed
	if (_defLoops != 0) {
		_defTrackEndFrame = _sys->getMillis() + _defTrackLength;
	}
}

void DefaultCDPlayer::stop() {	/* Stop CD Audio in 1/10th of a second */
	_defStopTime = _sys->getMillis() + 100;
	_defLoops = 0;
	return;
}

void DefaultCDPlayer::play(int track, int num_loops, int start_frame, int duration) {
	if (!num_loops && !start_frame)
		return;

	UInt32 fullLength;

	_defLoops = num_loops;
	start_frame = TO_MSECS(start_frame);
	duration = TO_MSECS(duration);

	// frame in milli-seconds
	_defStopTime = 0;
	fullLength  = start_frame + gVars->CD.defaultTrackLength * 1000;

	if (duration > 0) {
		_defTrackLength = duration;
	} else if (start_frame > 0) {
		_defTrackLength = fullLength;
		_defTrackLength -= start_frame;
	} else {
		_defTrackLength = fullLength;
	}

	// try to play the track
	_defTrackEndFrame = _sys->getMillis() + _defTrackLength;
}
