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

#ifndef BAGEL_METAGAME_GTL_ENCOUNT_H
#define BAGEL_METAGAME_GTL_ENCOUNT_H

#include "bagel/hodjnpodj/metagame/bgen/mgstat.h"
#include "bagel/hodjnpodj/metagame/bgen/invent.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

#define TRAP_PER_SECTOR       2
#define NARR_PER_SECTOR       8
#define MG_TRAP_COUNT        60
#define MG_NARR_COUNT       180
#define MG_ENC_COUNT        (MG_TRAP_COUNT + MG_NARR_COUNT)

#define TRAP_PROB            20         // 20% of the time, set a trap  ( 2 out of 10 times )
#define NARR_PROB             3         // 3 * number of steps
#define PROB_FACTOR          10         // raise percentages by a factor of ten
#define RAND_FACTOR         100         // 100%

#define MG_ACT_HODJ         288
#define MG_ACT_PODJ         289

// Value of Two objects, MISH and MOSH
#define ENC_MISHMOSH        2

// Input whether they want a boobytrap or narration
#define ENC_BOOBYTRAP       0
#define ENC_NARRATION       1

// RETURN values for the DoEncounter function
#define ENC_DO_NOTHING      0
#define ENC_LOSE_TURN       1
#define ENC_GAIN_TURN       2

#define MAX_ACTIONS         5


// CEncounterTable definition, used for both Booby Traps and Colour Narrations

class CEncounterTable {
public:
	int     m_iSectorCode ;         // MG_SECTOR_xxxx: sector code
	const char *m_lpszWavEFile;        // name of 8-bit sound file (*m_pItemSoundPath)
	int     m_Actions[MAX_ACTIONS]; // string of action codes, set to nullptr for Colour Narrations
	const char *m_lpszText ;           // text of booby trap
} ;


int DoEncounter(CWnd *pWnd, CPalette *pPalette, bool bHodj, CInventory *pInventory,
                CInventory *pPawn, CInventory *pGeneral,
                int EncType, int nNumSteps,
                int nPSector, int nOSector, bool *pArray);

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
