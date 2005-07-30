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
#include <SonyClie.h>
#include "common/stdafx.h"
#include "common/system.h"
#include "cd_msa.h"
#include "start.h"	// for appFileCreat

static void doErr(Err e, const Char *msg) {
	Char err[100];
	StrPrintF(err, "%ld : " , e);
	StrCat(err,msg);
	FrmCustomAlert(1000,err,0,0);
}

MsaCDPlayer::MsaCDPlayer(OSystem *sys) {
	_sys = sys;
	_msaRefNum = sysInvalidRefNum;

	_msaLoops = 0;
	_msaStopTime = 0;
	_msaTrackEndSu = 0;
}

bool MsaCDPlayer::init() {
	SonySysFtrSysInfoP sonySysFtrSysInfoP;
	Err error = errNone;

	if (!(error = FtrGet(sonySysFtrCreator, sonySysFtrNumSysInfoP, (UInt32*)&sonySysFtrSysInfoP))) {
		// not found with audio adapter ?!
		//if (sonySysFtrSysInfoP->libr & sonySysFtrSysInfoLibrMsa) {
			if ((error = SysLibFind(sonySysLibNameMsa, &_msaRefNum)))
				if (error == sysErrLibNotFound)
					error = SysLibLoad(sonySysFileTMsaLib, sonySysFileCMsaLib, &_msaRefNum);

			// FIXME : still don't understand how this lib works, it seems to be very unstable
			// and with the very bad documentation provided by Sony it's difficult to find out why
			// this doesn't work the same way on build-in MP3 device and external MP3 devices
			if (!error) {
				//MsaLibClose(_msaRefNum, msaLibOpenModeAlbum);	// close the lib if we previously let it open (?) Need to add Notify for sonySysNotifyMsaEnforceOpenEvent just in case ...
				error = MsaLibOpen(_msaRefNum, msaLibOpenModeAlbum);

				//if (error == msaErrAlreadyOpen)
				//	error = MsaLibEnforceOpen(_msaRefNum, msaLibOpenModeAlbum, appFileCreator);

				//error = (error != msaErrStillOpen) ? error : errNone;
			}
		//}
	}

//	if (error)
//		_msaRefNum = sysInvalidRefNum;

	_isInitialized = (_msaRefNum != sysInvalidRefNum);
	initInternal();
	return _isInitialized;
}

void MsaCDPlayer::initInternal() {
	if (!_isInitialized)
		return;

	Err e;
	Char nameP[256];
	UInt32 dummy, albumIterater = albumIteratorStart;

	MemSet(&_msaAlbum, sizeof(_msaAlbum), 0);
	_msaAlbum.maskflag = msa_INF_ALBUM;
	_msaAlbum.code = msa_LANG_CODE_ASCII;
	_msaAlbum.nameP = nameP;
	_msaAlbum.fileNameLength = 256;

	e = MsaAlbumEnumerate(_msaRefNum, &albumIterater, &_msaAlbum);
	e = MsaSetAlbum(_msaRefNum, _msaAlbum.albumRefNum, &dummy);

	// TODO : use RMC to control volume
	// move this to setSolume
	MsaOutSetVolume(_msaRefNum, 20, 20);
}

void MsaCDPlayer::release() {
	if (_isInitialized) {
		if (_msaRefNum != sysInvalidRefNum) {
			// stop the current track if any (needed if we use enforce open to prevent the track to play after exit)
			MsaStop(_msaRefNum, true);
			MsaLibClose(_msaRefNum, msaLibOpenModeAlbum);
		}
	}

	// self delete
	delete this;
}

bool MsaCDPlayer::poll() {
	if (!_isInitialized)
		return false;

	MsaPBStatus pb;
	MsaGetPBStatus(_msaRefNum, &pb);
	return (_msaLoops != 0 && (pb.currentSU < _msaTrackEndSu || pb.status != msa_STOPSTATUS));
}

void MsaCDPlayer::update() {
	if (!_isInitialized)
		return;

	// get playback status
	MsaPBStatus pb;
	MsaGetPBStatus(_msaRefNum, &pb);

	// stop replay upon request of stopCD()
	if (_msaStopTime != 0 && _sys->getMillis() >= _msaStopTime) {
		MsaStop(_msaRefNum, true);
		_msaLoops = 0;
		_msaStopTime = 0;
		_msaTrackEndSu = 0;
		return;
	}

	// not fully played nad still playing the correct track
	// (when playing a full track the return SU is not correct
	// and so we need to check if we are still playing the correct track)
	if (pb.currentSU < _msaTrackEndSu) {
		UInt16 trackNo;
		MsaPBListIndexToTrackNo(_msaRefNum, pb.currentpblistindex, &trackNo);
		if (trackNo == _msaTrack)
			return;
	}

	MsaStop(_msaRefNum, true);

	if (_msaLoops == 0)
		return;

	// track ends and last play, force stop if still playing
/*	if (_msaLoops != 1 && pb.status != msa_STOPSTATUS) {
		MsaStop(_msaRefNum, true);
		return;
	}
*/
	// loop again ?
	if (_msaLoops > 0)
		_msaLoops--;

	// loop if needed
	if (_msaLoops != 0) {
		if (_msaStartFrame == 0 && _msaDuration == 0)
			MsaPlay(_msaRefNum, _msaTrack, 0, msa_PBRATE_SP);
		else
			MsaPlay(_msaRefNum, _msaTrack, _msaTrackStartSu, msa_PBRATE_SP);
	}
}

void MsaCDPlayer::stop() {	/* Stop CD Audio in 1/10th of a second */
	if (!_isInitialized)
		return;

	_msaStopTime = _sys->getMillis() + 100;
	_msaLoops = 0;
	return;
}

void MsaCDPlayer::play(int track, int num_loops, int start_frame, int duration) {
	if (!_isInitialized)
		return;

	if (!num_loops && !start_frame)
		return;

	_msaTrack = track + gVars->CD.firstTrack - 1;	// first track >= 1 ?, not 0 (0=album)
	_msaLoops = num_loops;
	_msaStartFrame = TO_MSECS(start_frame);
	_msaDuration = TO_MSECS(duration);

	Err e;
	MemHandle trackH;

	// stop current play if any
	MsaStop(_msaRefNum, true);
	_msaStopTime = 0;

	// retreive track infos
	e = MsaGetTrackInfo(_msaRefNum, _msaTrack, 0, msa_LANG_CODE_ASCII, &trackH);

	// track exists
	if (!e && trackH) {
		MsaTime msaTime;
		MsaTrackInfo *trackP;
		UInt32 SU, fullLength;

		// FIXME (?) : this enable MsaSuToTime to return the right value in some cases
		MsaPlay(_msaRefNum, _msaTrack, 0, msa_PBRATE_SP);
		MsaStop(_msaRefNum, true);

		// get the msa time
		trackP = (MsaTrackInfo *)MemHandleLock(trackH);
		MsaSuToTime(_msaRefNum, trackP->totalsu, &msaTime);
		SU = trackP->totalsu;
		MemPtrUnlock(trackP);
		MemHandleFree(trackH);

		// MSA frame in milli-seconds
		fullLength  = FROM_MIN(msaTime.minute);
		fullLength += FROM_SEC(msaTime.second);
		fullLength += msaTime.frame;

		if (_msaDuration > 0) {
			_msaTrackLength = _msaDuration;
		} else if (_msaStartFrame > 0) {
			_msaTrackLength = fullLength;
			_msaTrackLength -= _msaStartFrame;
		} else {
			_msaTrackLength = fullLength;
		}

		// try to play the track
		if (start_frame == 0 && duration == 0) {
			MsaPlay(_msaRefNum, _msaTrack, 0, msa_PBRATE_SP);
			_msaTrackEndSu = SU;
		} else {
			// FIXME : MsaTimeToSu doesn't work ... (may work with previous FIXME)
			_msaTrackStartSu = (UInt32) ((float)(_msaStartFrame)  / ((float)fullLength / (float)SU));
			_msaTrackEndSu	 = (UInt32) ((float)(_msaTrackLength) / ((float)fullLength / (float)SU));
			_msaTrackEndSu	+= _msaTrackStartSu;

			if (_msaTrackEndSu > SU)
				_msaTrackEndSu = SU;

			MsaPlay(_msaRefNum, _msaTrack, _msaTrackStartSu, msa_PBRATE_SP);
		}
	}
	// TODO : use default track length if track not found
}
