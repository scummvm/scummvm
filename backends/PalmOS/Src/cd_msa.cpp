/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
#include "stdafx.h"
#include "scumm.h"
#include "common/system.h"
#include "cd_msa.h"
#include "start.h"	// for appFileCreator

MsaCDPlayer::MsaCDPlayer(OSystem *sys) {
	_sys = sys;
	_msaRefNum = sysInvalidRefNum;
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

			if (!error) {
//				Char buf[100];
//				StrPrintF(buf,"MSA refNum %ld, Try to open lib ...", refNum);
//				FrmCustomAlert(1000,buf,0,0);
				MsaLibClose(_msaRefNum, msaLibOpenModeAlbum);	// close the lib if we previously let it open (?) Need to add Notify for sonySysNotifyMsaEnforceOpenEvent just in case ...
				error = MsaLibOpen(_msaRefNum, msaLibOpenModeAlbum);			
/*				switch (error) {
				case msaErrAlreadyOpen:
					FrmCustomAlert(1000,"msaErrAlreadyOpen",0,0);
					break;
				case msaErrMemory:
					FrmCustomAlert(1000,"msaErrMemory",0,0);
					break;
				case msaErrDifferentMode:
					FrmCustomAlert(1000,"msaErrDifferentMode",0,0);
					break;
				case expErrCardNotPresent:
					FrmCustomAlert(1000,"expErrCardNotPresent",0,0);
					break;
				}
*/
				if (error == msaErrAlreadyOpen)
					error = MsaLibEnforceOpen(_msaRefNum, msaLibOpenModeAlbum, appFileCreator);
			
				error = (error != msaErrStillOpen) ? error : errNone;
			}
		//}
	}

	if (error)
		_msaRefNum = sysInvalidRefNum;

	_isInitialized = (_msaRefNum != sysInvalidRefNum);
	initInternal();
	return _isInitialized;
}

void MsaCDPlayer::initInternal() {
	if (!_isInitialized)
		return;

	// Beeeeeep !!!
//	MsaOutCapabilityType capability;
//	MsaOutInit(_msaRefNum);
//	MsaOutGetCapability(_msaRefNum, &capability);
//	MsaOutSetBeepLevel(_msaRefNum, capability.beepMaxLebel);
//	MsaOutStartBeep(_msaRefNum, 1000, msaOutBeepPatternOK);

	Err e;
	UInt32 dummy, albumIterater = albumIteratorStart;
	Char nameP[256];
	MemSet(&_msaAlbum, sizeof(_msaAlbum), 0);
	_msaAlbum.maskflag = msa_INF_ALBUM;
	_msaAlbum.code = msa_LANG_CODE_ASCII;
	_msaAlbum.nameP = nameP;
	_msaAlbum.fileNameLength = 256;

	e = MsaAlbumEnumerate(_msaRefNum, &albumIterater, &_msaAlbum);
//	if (e) doErr(e, "MsaAlbumEnumerate");
	e = MsaSetAlbum(_msaRefNum, _msaAlbum.albumRefNum, &dummy);
//	if (e) doErr(e, "MsaSetAlbum");
//	e = MsaGetPBRate(_msaRefNum, &_msaPBRate);
//	if (e) doErr(e, "MsaGetPBRate");

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
	return (_msaLoops != 0 && (_sys->get_msecs() < _msaEndTime || pb.status != msa_STOPSTATUS));
}

void MsaCDPlayer::update() {
	if (!_isInitialized)
		return;

	// stop replay upon request of stop_cdrom()
	if (_msaStopTime != 0 && _sys->get_msecs() >= _msaStopTime) {
		MsaStop(_msaRefNum, true);
		_msaLoops = 0;
		_msaStopTime = 0;
		_msaEndTime = 0;
		return;
	}

	// not fully played
	if (_sys->get_msecs() < _msaEndTime)
		return;
		
	if (_msaLoops == 0) {
		MsaStop(_msaRefNum, true);
		return;
	}

	// track ends and last play, force stop if still playing
	if (_msaLoops != 1) {
		MsaPBStatus pb;
		MsaGetPBStatus(_msaRefNum, &pb);
	 	if (pb.status != msa_STOPSTATUS) {
//			debug(0,"Stop It now");
			MsaStop(_msaRefNum, true);
			return;
		}
	}

	// loop again ?
	if (_msaLoops > 0) {
		MsaStop(_msaRefNum, true); // stop if loop=0
		_msaLoops--;
	}

	// loop if needed
	if (_msaLoops != 0) {
		MsaStop(_msaRefNum, true);
//		debug(0,"Next loop : %d", _msaLoops);

		_msaEndTime = _sys->get_msecs() + _msaTrackLength;
		if (_msaStartFrame == 0 && _msaEndFrame == 0)
			MsaPlay(_msaRefNum, _msaTrack, 0, msa_PBRATE_SP);
		else
			MsaPlay(_msaRefNum, _msaTrack, _msaTrackStart, msa_PBRATE_SP);
	}
}

static void doErr(Err e, const Char *msg) {
	Char err[100];
	StrPrintF(err, "%ld : " , e);
	StrCat(err,msg);
	FrmCustomAlert(1000,err,0,0);
}

void MsaCDPlayer::stop() {	/* Stop CD Audio in 1/10th of a second */
	if (!_isInitialized)
		return;

	_msaStopTime = _sys->get_msecs() + 100;
	_msaLoops = 0;
	return;
}

// frames are 1/75 sec
#define CD_FPS 75
#define TO_MSECS(frame)	((UInt32)((frame) * 1000 / CD_FPS))
// consider frame at 1/1000 sec
#define TO_SEC(msecs)	((UInt16)((msecs) / 1000))
#define TO_MIN(msecs)	((UInt16)(TO_SEC((msecs)) / 60))
#define FROM_MIN(mins)	((UInt32)((mins) * 60 * 1000))
#define FROM_SEC(secs)	((UInt32)((secs) * 1000))

#define FRAMES_TO_MSF(f, M,S,F) {                                       \
        int value = f;                                                  \
        *(F) = value%CD_FPS;                                            \
        value /= CD_FPS;                                                \
        *(S) = value%60;                                                \
        value /= 60;                                                    \
        *(M) = value;                                                   \
}

void MsaCDPlayer::play(int track, int num_loops, int start_frame, int duration) {
	if (!_isInitialized)
		return;

	if (!num_loops && !start_frame)
		return;
	
	Err e;
	
/*
	if (start_frame > 0)
		start_frame += CD_FPS >> 1;
*/
	if (duration > 0)
		duration += 5;

//	debug(0, ">> Request : track %d / loops : %d / start : %d / duration : %d", track, num_loops, start_frame, duration);
	
	_msaLoops = num_loops;
	_msaTrack = track + gVars->music.firstTrack - 1;	// first track >= 1 ?, not 0 (0=album)
	_msaStartFrame = TO_MSECS(start_frame);
	_msaEndFrame = TO_MSECS(duration);

//	debug(0, ">> To MSECS : start : %d / duration : %d", _msaStartFrame, _msaEndFrame);

	// if gVars->MP3 audio track
//	Err e;
	MemHandle trackH;

	// stop current play if any
	MsaStop(_msaRefNum, true);
	// retreive track infos
	e = MsaGetTrackInfo(_msaRefNum, _msaTrack, 0, msa_LANG_CODE_ASCII, &trackH);
//	if (e) doErr(e, "MsaGetTrackInfo");
	// track exists
	if (!e && trackH) {
		MsaTime tTime;
		UInt32 SU, fullLength;
		MsaTrackInfo *tiP;
	
		// FIXME : this enable MsaSuToTime to return the right value
		MsaPlay(_msaRefNum, _msaTrack, 0, msa_PBRATE_SP);
		MsaStop(_msaRefNum, true);
		
		tiP = (MsaTrackInfo *)MemHandleLock(trackH);	
		MsaSuToTime(_msaRefNum, tiP->totalsu, &tTime);
		SU = tiP->totalsu;
		MemPtrUnlock(tiP);
		MemHandleFree(trackH);
		
//		debug(0, ">> SU of track : %d (%d%:%d.%d)", SU, tTime.minute, tTime.second, tTime.frame);

//		Char buf[200];
//		StrPrintF(buf,"Track : %ld - %ld%:%ld.%ld (%ld)", track, tTime.minute, tTime.second, tTime.frame, SU);
//		FrmCustomAlert(1000,buf,0,0);

		_msaStopTime = 0;
		fullLength = FROM_MIN(tTime.minute) + FROM_SEC(tTime.second) + tTime.frame;

//		debug(0, ">> Full length : %d", fullLength);
		
		if (_msaEndFrame > 0) {
			_msaTrackLength = _msaEndFrame;
		} else if (_msaStartFrame > 0) {
			_msaTrackLength = fullLength;
			_msaTrackLength -= _msaStartFrame;
		} else {
			_msaTrackLength = fullLength;
		}
		
		// try to play the track
		if (start_frame == 0 && duration == 0) {
			MsaPlay(_msaRefNum, _msaTrack, 0, msa_PBRATE_SP);
		} else {
			// FIXME : MsaTimeToSu doesn't work ... (may work with previous FIXME)
			_msaTrackStart = (UInt32) ((float)(_msaStartFrame) / ((float)fullLength / (float)SU));
//			debug(0, ">> start at (float) : %d", _msaTrackStart);
/*
UInt32 f;
			FRAMES_TO_MSF(start_frame, &tTime.minute, &tTime.second, &f)
			tTime.frame = 0;
			MsaTimeToSu(_msaRefNum, &tTime, &SU);
			debug(0, ">> start at (MsaTimeToSu) : %d", SU);
*/
			MsaPlay(_msaRefNum, _msaTrack, _msaTrackStart, msa_PBRATE_SP);
		}

		_msaEndTime = _sys->get_msecs() + _msaTrackLength;
//		debug(0, ">> track length : %d / end : %d", _msaTrackLength, _msaEndTime);
	}
}
