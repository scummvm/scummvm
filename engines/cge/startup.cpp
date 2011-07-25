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
 */

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge/startup.h"
#include "cge/text.h"
#include "cge/sound.h"
#include "cge/cfile.h"
#include "cge/snddrv.h"

namespace CGE {

extern char _copr[];

#define id (*(Ident*)_copr)

// static Startup _startUp;

int    Startup::_mode = 0;
int    Startup::_soundOk = 0;
uint16 Startup::_summa;


void quitNow(int ref) {
	error("%s", _text->getText(ref));
}


bool Startup::getParms() {
	_summa = 0;

	/*
	int i = _argc;
	while (i > 1) {
		static char *PrmTab[] = { "NEW", "MK0SVG", "QUIET", "SB", "GUS", "MIDI", "P", "D", "I", "M" };
		int n = takeEnum(PrmTab, strtok(_argv[--i], " =:("));
		uint16 p = xtow(strtok(NULL, " h,)"));
		switch (n) {
		case 0 : 
			if (Mode != 2)
			Mode = 1;
			break;
		case 1 :
			Mode = 2;
			break;
		case 2 :
			SNDDrvInfo.DDEV = DEV_QUIET;
			break;
		case 3 :
			SNDDrvInfo.DDEV = DEV_SB;
			break;
		case 4 :
			SNDDrvInfo.DDEV = DEV_GUS;
			break;
		case 5 :
			SNDDrvInfo.MDEV = DEV_GM;
			break;
		case 6 :
			SNDDrvInfo.DBASE = p;
			break;
		case 7 :
			SNDDrvInfo.DDMA = p;
			break;
		case 8 :
			SNDDrvInfo.DIRQ = p;
			break;
		case 9 :
			SNDDrvInfo.MBASE = p;
			SNDDrvInfo.MDEV = DEV_GM;
			break;
		default:
			return false;
		}

		if (n >= 2)
			SoundOk = 2;
	}
	if (_vm->_isDemo)
		// protection disabled
		Summa = 0;
	else {
#ifdef EVA
		union { dosdate_t d; uint32 n; } today;
		_dos_getdate(&today.d);
		id.disk += (id.disk < today.n);
#endif
#ifdef CD
		Summa = 0;
#else
	// disk signature checksum
		Summa = ChkSum(Copr, sizeof(Ident));
#endif
	}
	
	if (SNDDrvInfo.MDEV != DEV_GM)
		SNDDrvInfo.MDEV = SNDDrvInfo.DDEV;
	return true;
	  */
	warning("STUB: Startup::get_parms");
	return true;
}


Startup::Startup() {
	/*
	uint32 m = farcoreleft() >> 10;
	if (m < 0x7FFF)
		Core = (int) m; 
	else
		Core = 0x7FFF;

	if (! IsVga())
		quit_now(NOT_VGA_TEXT);
	if (Cpu() < _80286)
		quit_now(BAD_CHIP_TEXT);
	if (100 * _osmajor + _osminor < 330)
		quit_now(BAD_DOS_TEXT);
	if (! get_parms())
		quit_now(BAD_ARG_TEXT);
	//--- load sound configuration
	const char * fn = usrPath(ProgName(CFG_EXT));
	if (!Startup::_soundOk && CFILE::Exist(fn)) {
		CFILE cfg(fn, REA);
		if (! cfg.Error) {
			cfg.Read(&SNDDrvInfo, sizeof(SNDDrvInfo)-sizeof(SNDDrvInfo.VOL2));
			if (! cfg.Error)
				Startup::_soundOk = 1;
		}
	}
	*/
	warning("STUB: Startup::Startup");
}


const char *usrPath(const char *nam) {
	static char buf[kPathMax] = ".\\", *p = buf + 2;
#if defined(CD)
	if (DriveCD(0)) {
		bool ok = false;
		CFILE ini = Text[CDINI_FNAME];
		if (!ini.Error) {
			char *key = Text[GAME_ID];
			int i = strlen(key);
			while (ini.Read(buf) && !ok) {
				int j = strlen(buf);
				if (j)
					if (buf[--j] == '\n')
						buf[j] = '\0';
				if (scumm_strnicmp((const char *) buf, (const char*) key, i) == 0)
					ok = true;
			}
			if (ok) {
				strcpy(buf, buf + i);
				p = buf + strlen(buf);
				if (*(p - 1) != '\\')
					*(p++) = '\\';
				strcpy(p, "NUL");
				if (_dos_open(buf, 0, &i) == 0)
					_dos_close(i);
				else
					ok = false;
			}
		}
		if (!ok)
			quit_now(BADCD_TEXT);
	}
#endif
	strcpy(p, nam);
	return buf;
}

} // End of namespace CGE
