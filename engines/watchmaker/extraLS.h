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

#ifndef WATCHMAKER_EXTRALS_H
#define WATCHMAKER_EXTRALS_H

#include "watchmaker/types.h"
#include "watchmaker/renderer.h"

#define MAX_EXTRALS                     55

#define EXTRALS_SAVE                            1
#define EXTRALS_LOAD                            2
#define EXTRALS_OPTIONS                     3
#define EXTRALS_QUIT                            4
#define EXTRALS_CLOSE                           5
#define EXTRALS_NOCONNECTION            6
#define EXTRALS_INVALIDNUMBER           7
#define EXTRALS_DIAL                            8
#define EXTRALS_ENTERUSERNUMBER     9
#define EXTRALS_DOCUMENTS                   10
#define EXTRALS_DOCUMENTVIEWER      11
#define EXTRALS_EMAILVIEWER             12
#define EXTRALS_EMAIL                           13
#define EXTRALS_AVAILABLEEMAIL      14
#define EXTRALS_OK                              15
#define EXTRALS_CANCEL                      16
#define EXTRALS_CLEAR                           17
#define EXTRALS_DIALLING                    18
#define EXTRALS_SEARCH                      19
#define EXTRALS_FINDBY                      20
#define EXTRALS_NAME                            21
#define EXTRALS_SURNAME                     22
#define EXTRALS_SUBJECT                     23
#define EXTRALS_CASENAME                    24
#define EXTRALS_YEAR                            25
#define EXTRALS_NODOCUMENTS             26
#define EXTRALS_NEWMAIL                     27
#define EXTRALS_CIAMAINTERMINAL     28
#define EXTRALS_GAMEOVER                    29
#define EXTRALS_EXIT                            30
#define EXTRALS_PLAYTHEGAME             31
#define EXTRALS_LOADAGAME                   32
#define EXTRALS_MAINMENU                    33
#define EXTRALS_HELP                            34
#define EXTRALS_LOGVIEWER                   35
#define EXTRALS_TONESEQUENCER           36
#define EXTRALS_ACQUIRE                     37
#define EXTRALS_PROCESS                     38
#define EXTRALS_LOG                             39
#define EXTRALS_PREVIEW                     40
#define EXTRALS_SCAN                            41
#define EXTRALS_SELECTAREA              42
#define EXTRALS_SCANPROGRAM             43
#define EXTRALS_MUSIC                           44
#define EXTRALS_SOUND                           45
#define EXTRALS_SPEECH                      46
#define EXTRALS_SUBTITLES                   47
#define EXTRALS_DARRELL                     48
#define EXTRALS_VICTORIA                    49

namespace Watchmaker {

extern char *ExtraLS[MAX_EXTRALS];

void CheckExtraLocalizationStrings(Renderer &renderer, uint32 id);

} // End of namespace Watchmaker

#endif // WATCHMAKER_EXTRALS_H
