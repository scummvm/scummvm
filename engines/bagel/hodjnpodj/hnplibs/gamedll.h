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

#ifndef HODJNPODJ_HNPLIBS_GAMEDLL_H
#define HODJNPODJ_HNPLIBS_GAMEDLL_H

#include "bagel/afxwin.h"

namespace Bagel {
namespace HodjNPodj {

#define PATHSPECSIZE        256

#define NOPLAY             -1
#define SKILLLEVEL_LOW      0
#define SKILLLEVEL_MEDIUM   1
#define SKILLLEVEL_HIGH     2

#define INSTALL_NONE        0
#define INSTALL_MINIMAL     1
#define INSTALL_BASIC       2
#define INSTALL_EXTRA       3
#define INSTALL_FULL        4


struct GAMESTRUCT {
	long            lCrowns;
	long            lScore;
	int             nSkillLevel;
	bool            bSoundEffectsEnabled;
	bool            bMusicEnabled;
	bool            bPlayingMetagame;
	bool            bPlayingHodj;
	/*  For use later in Beta Test
	        int             nInstallationCode;
	        char            chHomePath[PATHSPECSIZE];
	        char            chCDPath[PATHSPECSIZE];
	        char            chMiniPath[PATHSPECSIZE];
	*/
};

typedef GAMESTRUCT FAR *LPGAMESTRUCT;

} // namespace HodjNPodj
} // namespace Bagel

#endif
